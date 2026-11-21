#include "sdk_protos.h"
#include "classes/CSharedDoc.h"
#include "iff/IffStruct.h"
#include "game/wvector.h"
// Translation unit 0x801C54BC-0x801C6B14 (6 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

enum SurfaceMappingFormat {
    kCourseVariantMask = 15, kNoSourceTriangle = 0xFFFF,
    kExistingVertexIndex = 0x8000, kSurfaceGroupCount = 2,
    kSeekFromCurrent = 1, kSurfaceNamePatternCount = 0x51,
    kSurfaceTriangleBytes = 8,      // sizeof(SurfaceTriangle): source id + three vertex codes
    kMeshCountArrays = 4,           // per-mesh int arrays in one allocation (offset, base, variant, group counts)
    kCourseRenderState23 = 0x17, kCourseRenderState24 = 0x18
};


// Compare the leading unsigned halfword, used to sort mappings by source triangle id.
int Compare_U16(unsigned short* a, unsigned short* b) {
    if (*a < *b)
        return -1;
    return *a > *b;
}

void WView_InvalidateViewAndProjection(void*);
void WView_DefaultProjectionHook(void*, int, const char*);
extern void* g_view;
extern const char s_sceneLighting[];

// Invalidate cached camera matrices and restore the default scene-lighting projection hook.
// (The symbol name WPolySoup_CStaticPetGrp_ResetViewProjection does not describe this body.)
void WPolySoup_CStaticPetGrp_ResetViewProjection() {
    WView_InvalidateViewAndProjection(g_view);
    WView_DefaultProjectionHook(g_view, 0, s_sceneLighting);
}

void Engine_SetCachedRenderState(int, int);
extern const char* s_courseObjectNamePatterns[];

// Return whether an object name contains any entry in the 81-pattern filter table.
int CourseObjectName_MatchesFilterPattern(void*, void* objectName) {
    int found = 0;
    for (int i = 0; i < kSurfaceNamePatternCount; ++i) {
        if (strstr((const char*)objectName, s_courseObjectNamePatterns[i])) {
            found = 1;
            break;
        }
    }
    return found;
}

struct CourseRenderStatePairs {
    char pad[0x493c];
    int state23Values[2];
    int state24Values[2];
};

// Clamp the pair index and apply the two course render-state values.
void CourseRenderStates_ApplyPair(CourseRenderStatePairs* self, int index) {
    if (index < 0)
        index = 0;
    else if (index > 1)
        index = 1;
    Engine_SetCachedRenderState(kCourseRenderState23, self->state23Values[index]);
    Engine_SetCachedRenderState(kCourseRenderState24, self->state24Values[index]);
}

void* __nwa__FUl(unsigned int);
void __dla__FPv(void*);
struct Vec3 {
    // Leave array elements uninitialized; every live generated vertex is written before use.
    Vec3() {}
    // Allocate through the game's array allocator (CodeWarrior adds the array cookie).
    static void* operator new[](size_t n) { return __nwa__FUl(n); }
    // Release the array allocation supplied by the compiler's delete[] path.
    static void operator delete[](void* p) { __dla__FPv(p); }
    float x, y, z;
};
// Four packed halfwords: the original triangle id followed by three replacement vertices.
// A four-word view keeps cyclic indexing in bounds; named fields document each word's role.
union SurfaceTriangle {
    unsigned short words[4];
    struct { unsigned short sourceTriangle; unsigned short vertices[3]; };
};
struct CourseSurfaceMesh {
    int vertexCount;
    char header[0x0C];
    Vec3* vertices;
    Vec3* normals;
    int* colorStorage;
    char renderState[8];
    void* copiedColors;
    char indexState[8];
    int indexCount;
    void* indices;
    char materialState[0x1C];
    unsigned long long* textureCoordinates;
    char textureState[0x1C];
    unsigned int* colors;
};
typedef char CourseSurfaceMesh_size[(sizeof(CourseSurfaceMesh) == 0x78) ? 1 : -1];
void* __nwa__FUl(unsigned int);
void __dla__FPv(void*);
int Vec3_NearlyEqualWithTolerance(void*, void*, float);
void __as__7WVectorFRC7WVector(void*, void*);
void Triangle_ComputeBarycentricCoordinates(float*, float*, void*, void*, void*, void*);
void* memcpy(void*, const void*, unsigned int);
void* memset(void*, int, unsigned int);
struct CourseSurfaceMesh;
void CourseMesh_ReplaceMappedSurfaceTriangles(void*, CourseSurfaceMesh*, SurfaceTriangle*, int, int*);

struct SurfaceTriangleMapping {
    unsigned char mesh, alignment;
    unsigned short id;
    unsigned short v[3];
    char remainingMappingData[6];
};
struct SurfaceTriangleGroup { int count; struct Sub { int count; SurfaceTriangleMapping* tris; }* subs; };
struct SurfaceMappingStream {
    virtual void v0(); virtual void v1();
    virtual int read(void*, int); virtual void v5(); virtual void v6();
    virtual int seek(int,int);
};
extern CSharedDoc* g_sharedDoc;
static const float kVertexMergeTolerance = 9.999999747378752e-06f;
static const unsigned int kFloatSignBit = 0x80000000U;
// Select which optional surface mapping block belongs to the current course variant.
inline unsigned char GetCourseVariant() {
    IFF_STRUCT::sCourse* course = (IFF_STRUCT::sCourse*)g_sharedDoc->selectedCourseData;
    if (course == 0) return 0;
    return course->courseFlags & kCourseVariantMask;
}

#pragma push
#pragma use_lmw_stmw on
// Merge base, variant and group mappings, discard cyclic duplicates, and rebuild each affected mesh.
// stream is borrowed and advanced; meshOutputs has meshCount mesh pointers represented as words.
// generatedVertices is a borrowed XYZ table in the same coordinate space as the destination meshes.
// groups contains two surface groups. Mapping mesh IDs/vertex codes must already be in range;
// this routine trusts the data and does not check stream read/seek results.
void CourseData_ReadSurfaceTriangleMappings(void* self, SurfaceMappingStream* stream, int meshCount,
    int* meshOutputs, void* generatedVertices, SurfaceTriangleGroup* groups) {
    int baseTriangleCount;
    SurfaceTriangleGroup* group = (SurfaceTriangleGroup*)groups;
    int triangles = 0;

    // Count enabled mappings in both surface groups before allocating merged triangle storage.
    for (int groupIndex = 0; kSurfaceGroupCount > groupIndex; groupIndex++, group++) {
        if (group->subs) {
            for (int i = 0; i < group->count; i++) {
                if (group->subs) {
                    SurfaceTriangleGroup::Sub* subgroup = &group->subs[i];
                    for (int j = 0; subgroup->count > j; j++) {
                        if (subgroup->tris[j].id != kNoSourceTriangle) triangles++;
                    }
                }
            }
        }
    }
    int variantTriangleCount;
    stream->read(&baseTriangleCount, 4);
    variantTriangleCount = 0;
    int skippedVariantACount;
    int skippedVariantBCount;
    skippedVariantBCount = 0;
    skippedVariantACount = 0;

    // The two optional count blocks describe course variants 1 and 2 respectively.
    switch (GetCourseVariant()) {
    case 0:
        stream->read(&skippedVariantACount, 4);
        stream->read(&skippedVariantBCount, 4);
        break;
    case 1:
        stream->read(&variantTriangleCount, 4);
        stream->read(&skippedVariantBCount, 4);
        break;
    case 2:
        stream->read(&skippedVariantACount, 4);
        stream->read(&variantTriangleCount, 4);
        break;
    }

    // Stream phase: 3 global counts, per-mesh count blocks for each present source,
    // then packed 8-byte triangle blocks. Unselected variant blocks are skipped, not decoded.
    // One allocation contains four arrays, each meshCount ints:
    // [0*N] start offsets, [1*N] base counts, [2*N] selected variant counts,
    // [3*N] group-mapping counts (later reset and reused as append cursors).
    // All offsets/counts are in SurfaceTriangle RECORDS, not bytes or vertex indices.
    int* meshOffsets = (int*)__nwa__FUl(meshCount * kMeshCountArrays * 4);
    memset(meshOffsets, 0, (meshCount * kMeshCountArrays * 4));
    SurfaceTriangle* trianglesByMesh = (SurfaceTriangle*)__nwa__FUl(
        (variantTriangleCount + (triangles + baseTriangleCount)) * kSurfaceTriangleBytes);
    if (baseTriangleCount > 0) stream->read(meshOffsets + meshCount, 4 * meshCount);
    if (skippedVariantACount > 0) stream->seek(4 * meshCount, kSeekFromCurrent);
    if (variantTriangleCount > 0) stream->read(meshOffsets + meshCount * 2, meshCount * 4);
    if (0 < skippedVariantBCount) stream->seek(meshCount * 4, kSeekFromCurrent);
    group = (SurfaceTriangleGroup*)groups;
    int* mappedTriangleCounts = meshOffsets + meshCount * 3;
    for (int groupIndex = 0; kSurfaceGroupCount > groupIndex; groupIndex++, group++) {
        if (group->subs) {
            for (int i = 0; i < group->count; i++) {
                if (group->subs) {
                    SurfaceTriangleGroup::Sub* subgroup = &group->subs[i];
                    for (int j = 0; j < subgroup->count; j++) {
                        SurfaceTriangleMapping* mapping = &subgroup->tris[j];
                        if (mapping->id != kNoSourceTriangle) mappedTriangleCounts[mapping->mesh]++;
                    }
                }
            }
        }
    }

    // Prefix-sum the three contributing triangle counts for each output mesh.
    meshOffsets[0] = 0;
    for (int i = 1; i < meshCount; i++) {
        meshOffsets[i] = (meshOffsets[meshCount * 2 + i - 1] + mappedTriangleCounts[i - 1])
            + (meshOffsets[meshCount + i - 1] + meshOffsets[i - 1]);
    }
    // Starts are now fixed; reuse the group counts as per-mesh write cursors.
    memset(mappedTriangleCounts, 0, 4 * meshCount);
    // This array starts with base counts and later holds total merged counts.
    int* triangleCounts = (meshOffsets + meshCount);
    {
        int* meshOffset = meshOffsets;
        int* triangleCount = triangleCounts;
        for (int i = 0; i < meshCount; triangleCount++, meshOffset++, i++) {
            if (*triangleCount > 0) stream->read(trianglesByMesh + *meshOffset, *triangleCount * kSurfaceTriangleBytes);
        }
    }
    if (skippedVariantACount) stream->seek(skippedVariantACount * kSurfaceTriangleBytes, kSeekFromCurrent);
    if (variantTriangleCount > 0) {
        int* triangleCount = triangleCounts;
        int* meshOffset = meshOffsets;
        int* variantCounts = meshOffsets + meshCount * 2;
        for (int i = 0; i < meshCount; variantCounts++, triangleCount++, meshOffset++, i++) {
            if (*variantCounts > 0) {
                stream->read((char*)(trianglesByMesh + *meshOffset) + *triangleCount * kSurfaceTriangleBytes, *variantCounts * kSurfaceTriangleBytes);
            }
        }
    }
    // Append group mappings after each mesh's reserved base + selected-variant range.
    group = (SurfaceTriangleGroup*)groups;
    int* variantCounts = meshOffsets + meshCount * 2;
    for (int groupIndex = 0; kSurfaceGroupCount > groupIndex; groupIndex++, group++) {
        if (group->subs) {
            for (int i = 0; i < group->count; i++) {
                if (group->subs) {
                    SurfaceTriangleGroup::Sub* subgroup = &group->subs[i];
                    for (int j = 0; j < subgroup->count; j++) {
                        SurfaceTriangleMapping* mapping = &subgroup->tris[j];
                        if (mapping->id != kNoSourceTriangle) {
                            int meshIndex = mapping->mesh;
                            SurfaceTriangle* destination = trianglesByMesh +
                                ((variantCounts[meshIndex] + mappedTriangleCounts[meshIndex]) +
                                 (meshOffsets[meshIndex] + triangleCounts[meshIndex]));
                            destination->sourceTriangle = mapping->id;
                            memcpy(&destination->vertices[0], mapping->v, 6);
                            mappedTriangleCounts[meshIndex]++;
                        }
                    }
                }
            }
        }
    }
    // Skip the remaining unselected variant-B block before reading selected-variant records.
    // It is part of the original stream traversal; removing it changes the final stream position.
    if (skippedVariantBCount) stream->seek(kSurfaceTriangleBytes * skippedVariantBCount, kSeekFromCurrent);
    {
        int* meshOffset = meshOffsets;
        int* triangleCount = triangleCounts;
        for (int i = 0; i < meshCount; variantCounts++, meshOffset++, mappedTriangleCounts++, triangleCount++, i++) {
            if (*variantCounts > 0) {
                stream->read((char*)(trianglesByMesh + *meshOffset) + *triangleCount * kSurfaceTriangleBytes, *variantCounts * kSurfaceTriangleBytes);
            }
            *triangleCount = *mappedTriangleCounts + (*triangleCount + *variantCounts);
        }
    }

    // Dedup phase (duplicate test is run twice: once to count survivors, once to copy them): same cyclic vertex order counts as a duplicate even with a different source ID;
    // reversed winding does not. Keep the last occurrence, then sort by source triangle ID.
    int* meshOffset = meshOffsets;
    int* triangleCount = triangleCounts;
    for (int i = 0; i < meshCount; i++) {
        int count = *triangleCount;
        if (count > 0) {
            int uniqueCount = 0;
            for (int j = 0; j < count - 1; j++) {
                int k;
                for (k = j + 1; count > k; k++) {
                    int rotation;
                    for (rotation = 0; rotation < 3; rotation++) {
                        SurfaceTriangle* candidate = trianglesByMesh + *meshOffset + j;
                        SurfaceTriangle* laterTriangle = trianglesByMesh + *meshOffset + k;
                        if (laterTriangle->words[1 + rotation] == candidate->vertices[0]
                            && candidate->vertices[1] == laterTriangle->words[1 + (rotation + 1) % 3]
                            && candidate->vertices[2] == laterTriangle->words[1 + (rotation + 2) % 3]) break;
                    }
                    if (rotation < 3) break;
                }
                if (k == count) uniqueCount++;
            }
            if (1 + uniqueCount != count) {
                int j;
                SurfaceTriangle* uniqueTriangles = (SurfaceTriangle*)__nwa__FUl((1 + uniqueCount) * kSurfaceTriangleBytes);
                unsigned int writtenCount = 0;
                int byteOffset = 0;
                for (j = 0; j < *triangleCount - 1; j++) {
                    int k;
                    for (k = j + 1; k < *triangleCount; k++) {
                        int rotation;
                        for (rotation = 0; rotation < 3; rotation++) {
                            SurfaceTriangle* laterTriangle = trianglesByMesh + *meshOffset + k;
                            SurfaceTriangle* candidate = trianglesByMesh + (*meshOffset + j);
                            if (laterTriangle->words[1 + rotation] == candidate->vertices[0]
                                && candidate->vertices[1] == laterTriangle->words[1 + (1 + rotation) % 3]
                                && candidate->vertices[2] == laterTriangle->words[1 + (rotation + 2) % 3]) break;
                        }
                        if (3 > rotation) break;
                    }
                    if (k == *triangleCount) {
                        writtenCount += 1;
                        void* destination = (char*)uniqueTriangles + byteOffset;
                        // Advance the write cursor by one record. The three-step form is needed for identical code.
                        int nextByteOffset = byteOffset + kSurfaceTriangleBytes;
                        int savedByteOffset = nextByteOffset;
                        byteOffset = savedByteOffset;
                        memcpy(destination, trianglesByMesh + (j + *meshOffset), kSurfaceTriangleBytes);
                    }
                }
                memcpy(uniqueTriangles + writtenCount++, trianglesByMesh + (j + *meshOffset), kSurfaceTriangleBytes);
                qsort(uniqueTriangles, writtenCount, 8, (int (*)(const void*, const void*))Compare_U16);
                CourseMesh_ReplaceMappedSurfaceTriangles(self, (CourseSurfaceMesh*)*meshOutputs, uniqueTriangles, writtenCount, (int*)generatedVertices);
                __dla__FPv(uniqueTriangles);
            } else {
                qsort(trianglesByMesh + *meshOffset, count, 8, (int (*)(const void*, const void*))Compare_U16);
                CourseMesh_ReplaceMappedSurfaceTriangles(self, (CourseSurfaceMesh*)*meshOutputs, trianglesByMesh + *meshOffset, *triangleCount, (int*)generatedVertices);
            }
        }
        ++meshOutputs;
        meshOffset++;
        triangleCount = (triangleCount + 1);
    }
    __dla__FPv(meshOffsets);
    __dla__FPv(trianglesByMesh);
}
#pragma pop

// Replace mapped triangles, interpolate generated vertex attributes, and retain unaffected triangles.
// triangles is borrowed, nonempty and sorted by sourceTriangle; IDs address valid mesh triangles.
// Vertex codes <0x8000 index generatedVertices; codes >=0x8000 reference existing mesh vertices.
// mesh owns all replaced arrays. Input positions and normals use the mesh's coordinate space;
// barycentric u/v are dimensionless weights, and UVs are texture coordinates (no normalization).
void CourseMesh_ReplaceMappedSurfaceTriangles(void* unused, CourseSurfaceMesh* mesh, SurfaceTriangle* triangles, int triangleCount, int* generatedVertices) {
    int samePositionAndNormal;
    int sameTextureV;
    int sameTextureU;
    unsigned short* cornerIndex;
    unsigned short* outIndex;
    Vec3* normal2;
    Vec3* normal1;
    Vec3* normal0;
    Vec3* searchVertex;
    Vec3* searchNormal;
    float* searchTexcoords;
    Vec3* vertex;
    unsigned int* colors;
    int addedCount;
    int corner;
    int match;
    int copied;
    int i;
    SurfaceTriangle* tri;
    Vec3* outVertex;
    Vec3* outNormal;
    float* outTexcoords;
    unsigned short* outBase;
    Vec3* vertex2;
    Vec3* vertex1;
    Vec3* vertex0;
    float* texcoord2;
    float* texcoord1;
    float* texcoord0;
    int triIndex;
    unsigned short* oldIndices;
    Vec3* vertices;
    Vec3* normals;
    float* texcoords;
    float* addedTexcoords;
    unsigned int* addedColors;
    unsigned short* newIndices;
    int newIndexCount;
    if (triangleCount == 0) return;
    // Phase 1: count unique replaced source triangles and allocate worst-case temporary output.
    oldIndices = (unsigned short*)mesh->indices;
    vertices = mesh->vertices;
    normals = mesh->normals;
    texcoords = (float*)mesh->textureCoordinates;
    colors = mesh->colors;
    int replacedTriangleCount = 1;
    for (int i = 1; i < triangleCount; ++i)
        if (triangles[i].sourceTriangle != triangles[i-1].sourceTriangle) ++replacedTriangleCount;
    int vertexCapacity;
    int retainedIndexCount;
    retainedIndexCount = mesh->indexCount - replacedTriangleCount * 3;
    vertexCapacity = triangleCount * 3;
    newIndexCount = retainedIndexCount + vertexCapacity;
    newIndices = (unsigned short*)__nwa__FUl(newIndexCount * 2);
    Vec3* addedVertices = new Vec3[vertexCapacity];
    Vec3* addedNormals = new Vec3[vertexCapacity];
    addedTexcoords = (float*)__nwa__FUl(vertexCapacity * 8);
    addedColors = (unsigned int*)__nwa__FUl(vertexCapacity * 4);
    addedCount = 0;
    float epsilon = kVertexMergeTolerance;
    tri = triangles;
    outVertex = addedVertices;
    outNormal = addedNormals;
    outTexcoords = addedTexcoords;
    unsigned int* outColor = addedColors;
    outBase = newIndices + retainedIndexCount;
    // Phase 2: append replacement indices; interpolate attributes for generated corners only.
    // New indices occupy the tail; untouched source indices will be copied into the prefix.
    for (triIndex = 0; triIndex < triangleCount; ++tri, ++triIndex) {
        unsigned short* outIndices = outBase + triIndex * 3;
        unsigned short* indices = oldIndices + tri->sourceTriangle * 3;
        unsigned short i0 = *indices;
        unsigned short i2 = indices[2];
        unsigned short i1 = indices[1];
        vertex0 = vertices + i0;
        vertex2 = vertices + i2;
        vertex1 = vertices + i1;
        normal0 = normals + i0;
        normal2 = normals + i2;
        normal1 = normals + i1;
        texcoord2 = texcoords + i2 * 2;
        texcoord1 = texcoords + i1 * 2;
        texcoord0 = texcoords + i0 * 2;
        cornerIndex = tri->words;
        outIndex = outIndices;
        corner = 0;
        do {
            int vertexCode = cornerIndex[1];
            if (vertexCode < kExistingVertexIndex) {
                vertex = (Vec3*)generatedVertices + vertexCode;
                float u, v;
                Triangle_ComputeBarycentricCoordinates(&u, &v, vertex, vertex0, vertex1, vertex2);
                WVector normal;
                normal.x = normal0->x + u * (normal1->x - normal0->x) + v * (normal2->x - normal0->x);
                normal.y = normal0->y + u * (normal1->y - normal0->y) + v * (normal2->y - normal0->y);
                normal.z = normal0->z + u * (normal1->z - normal0->z) + v * (normal2->z - normal0->z);
                unsigned int c2 = colors[i2], c1 = colors[i1], c0 = colors[i0];
                float red, green, blue, alpha;
                float r2 = (float)(c2 >> 24), r1 = (float)(c1 >> 24), r0 = (float)(c0 >> 24);
                float g2 = (float)((c2 >> 16) & 255), g1 = (float)((c1 >> 16) & 255), g0 = (float)((c0 >> 16) & 255);
                red = (u * (r1 - r0) + r0) + v * (r2 - r0);
                green = (u * (g1 - g0) + g0) + v * (g2 - g0);
                unsigned int r = (int)red;
                unsigned int g = (int)green;
                float b2 = (float)((c2 >> 8) & 255), b1 = (float)((c1 >> 8) & 255), b0 = (float)((c0 >> 8) & 255);
                float a2 = (float)(c2 & 255), a1 = (float)(c1 & 255), a0 = (float)(c0 & 255);
                blue = (u * (b1 - b0) + b0) + v * (b2 - b0);
                alpha = (u * (a1 - a0) + a0) + v * (a2 - a0);
                unsigned int b = (int)blue;
                unsigned int a = (int)alpha;
                float tex[2];
                tex[0] = (u * (texcoord1[0] - texcoord0[0]) + texcoord0[0]) + v * (texcoord2[0] - texcoord0[0]);
                tex[1] = (u * (texcoord1[1] - texcoord0[1]) + texcoord0[1]) + v * (texcoord2[1] - texcoord0[1]);
                unsigned int color = ((r << 24) | ((g & 255) << 16)) | (((b & 255) << 8) | (a & 255));
                // Preserve the original equivalence rule: compare position, normal and UV only.
                // Color is deliberately NOT part of this code's comparison; a reused vertex keeps
                // the first generated color. Intent beyond this observed behavior is unresolved.
                match = 0;
                searchVertex = addedVertices;
                searchNormal = addedNormals;
                searchTexcoords = addedTexcoords;
                while (match < addedCount) {
                    sameTextureV = 0; sameTextureU = 0; samePositionAndNormal = 0;
                    if (Vec3_NearlyEqualWithTolerance(searchVertex, vertex, kVertexMergeTolerance) && Vec3_NearlyEqualWithTolerance(searchNormal, &normal, kVertexMergeTolerance)) samePositionAndNormal = 1;
                    if (samePositionAndNormal) {
                        float diff = searchTexcoords[0] - tex[0];
                        float mag = diff;
                        if (*(unsigned int*)&diff & kFloatSignBit) mag = -diff;
                        int lt = mag < epsilon;
                        if (lt) sameTextureU = 1;
                    }
                    if (sameTextureU) {
                        float diff = searchTexcoords[1] - tex[1];
                        float mag = diff;
                        if (*(unsigned int*)&diff & kFloatSignBit) mag = -diff;
                        int lt = mag < epsilon;
                        if (lt) sameTextureV = 1;
                    }
                    if (sameTextureV) break;
                    ++searchVertex;
                    ++searchNormal;
                    searchTexcoords += 2;
                    ++match;
                }
                if (match == addedCount) {
                    *(WVector*)outVertex = *(WVector*)vertex;
                    *(WVector*)outNormal = normal;
                    memcpy(outTexcoords, tex, 8);
                    ++outVertex;
                    *outColor++ = color;
                    ++outNormal;
                    *outIndex = mesh->vertexCount + addedCount;
                    ++addedCount;
                    outTexcoords += 2;
                } else {
                    *outIndex = mesh->vertexCount + match;
                }
            } else {
                *outIndex = vertexCode - kExistingVertexIndex;
            }
            ++corner;
            ++outIndex;
            ++cornerIndex;
        } while (corner < 3);
    }
    // Phase 3: preserve untouched triangles in original order (prefix, gaps, then suffix).
    SurfaceTriangle* copyTri;
    copied = 0;
    if (triangles->sourceTriangle != 0) {
        memcpy(newIndices, oldIndices, triangles->sourceTriangle * 6);
        copied = triangles->sourceTriangle * 3;
    }
    i = 1;
    copyTri = triangles + 1;
    for (; i < triangleCount; ++copyTri, ++i) {
        int previous = copyTri[-1].words[0] * 3;
        int current = copyTri->sourceTriangle * 3;
        int gap = current - previous;
        if (gap > 3) {
            int length = gap - 3;
            memcpy(newIndices + copied, oldIndices + (previous + 3), length * 2);
            copied += length;
        }
    }
    int last = triangles[i-1].sourceTriangle * 3;
    int remaining = mesh->indexCount - last;
    if (remaining > 3) memcpy(newIndices + copied, oldIndices + (last + 3), (remaining - 3) * 2);

    // Phase 4: copy old attributes plus the deduplicated generated suffix into final arrays.
    Vec3* newVertices = new Vec3[mesh->vertexCount + addedCount];
    memcpy(newVertices, mesh->vertices, mesh->vertexCount * 12);
    int addedBytes = addedCount * 12;
    memcpy(newVertices + mesh->vertexCount, addedVertices, addedBytes);
    Vec3* newNormals = new Vec3[mesh->vertexCount + addedCount];
    memcpy(newNormals, mesh->normals, mesh->vertexCount * 12);
    memcpy(newNormals + mesh->vertexCount, addedNormals, addedBytes);
    float* newTexcoords;
    unsigned int* copiedColors;
    unsigned int* newColors = (unsigned int*)__nwa__FUl((mesh->vertexCount + addedCount) * 4);
    memcpy(newColors, mesh->colors, mesh->vertexCount * 4);
    memcpy(newColors + mesh->vertexCount, addedColors, addedCount * 4);
    copiedColors = (unsigned int*)__nwa__FUl((mesh->vertexCount + addedCount) * 4);
    memcpy(copiedColors, newColors, (mesh->vertexCount + addedCount) * 4);
    newTexcoords = (float*)__nwa__FUl((mesh->vertexCount + addedCount) * 8);
    memcpy(newTexcoords, mesh->textureCoordinates, mesh->vertexCount * 8);
    memcpy(newTexcoords + mesh->vertexCount * 2, addedTexcoords, addedCount * 8);
    // Phase 5: release scratch and old mesh arrays, then publish the final buffers/counts.
    // Vec3 new[] results have a 16-byte compiler cookie; raw attribute/index arrays do not.
    if (addedVertices) __dla__FPv((char*)addedVertices - 16);
    if (addedNormals) __dla__FPv((char*)addedNormals - 16);
    __dla__FPv(addedTexcoords);
    __dla__FPv(addedColors);
    if (mesh->vertices) __dla__FPv((char*)mesh->vertices - 16);
    if (mesh->normals) __dla__FPv((char*)mesh->normals - 16);
    __dla__FPv(mesh->colors);
    __dla__FPv(mesh->copiedColors);
    __dla__FPv(mesh->textureCoordinates);
    __dla__FPv(mesh->indices);
    mesh->indexCount = newIndexCount;
    mesh->vertexCount += addedCount;
    mesh->normals = newNormals;
    mesh->vertices = newVertices;
    mesh->colors = newColors;
    mesh->copiedColors = copiedColors;
    mesh->textureCoordinates = (unsigned long long*)newTexcoords;
    mesh->indices = newIndices;
}

} // extern "C"
