// Translation unit 0x80430240-0x804311B4 (30 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

// GX's write-gather FIFO consumes each volatile write in source order.
static const unsigned int kGxFifoAddress = 0xCC008000;
enum { kSkinWeightCount = 2, kUnusedBoneIndex = 0x20, kMatrixByteShift = 6 };

// Counts measure records, not bytes. Inputs/palettes are borrowed; CPU outputs
// are caller-owned and must fit every record. Submit routines emit attributes
// only: the caller begins GX primitives and sets a descriptor matching the exact
// write order and width. Repeated-index routines emit one u16 index N times for
// N enabled indexed attributes; A/B/... do not identify the descriptor's fields.
// Direct routines forward positions in the caller's GX input space. Skinning
// maps source positions into the palette's destination space; this file alone
// does not establish whether that space is world, view, or projected space.

// Interleaved CPU skinning formats; the two byte bone indices select palette rows.
struct WeightedPositionColor {
    float position[3]; unsigned int packedColor;
    float firstWeight; unsigned char boneIndices[2]; char pad_16[2];
};
struct WeightedPositionNormal {
    float position[3]; float firstWeight;
    float normal[3]; unsigned char boneIndices[2]; char pad_1E[2];
};
struct WeightedPositionNormalTex {
    WeightedPositionNormal skin; float u, v;
};
typedef char WeightedPositionColor_size[(sizeof(WeightedPositionColor) == 0x18) ? 1 : -1];
typedef char WeightedPositionNormal_size[(sizeof(WeightedPositionNormal) == 0x20) ? 1 : -1];
typedef char WeightedPositionNormalTex_size[(sizeof(WeightedPositionNormalTex) == 0x28) ? 1 : -1];
static const int kColorWeightOffset = (int)&((WeightedPositionColor*)0)->firstWeight;
static const int kColorBoneIndicesOffset = (int)&((WeightedPositionColor*)0)->boneIndices;
static const int kPackedColorOffset = (int)&((WeightedPositionColor*)0)->packedColor;
static const int kNormalWeightOffset = (int)&((WeightedPositionNormal*)0)->firstWeight;
static const int kNormalBoneIndicesOffset = (int)&((WeightedPositionNormal*)0)->boneIndices;
static const int kNormalOffset = (int)&((WeightedPositionNormal*)0)->normal;
static const int kSkinUOffset = (int)&((WeightedPositionNormalTex*)0)->u;
static const int kSkinVOffset = (int)&((WeightedPositionNormalTex*)0)->v;

extern "C" {

void Render_WriteFifo3f(float x, float y, float z);
void Render_WriteTexCoord2f(float x, float y);
void Render_WriteNormal3f(float a, float b, float c);
float* Mtx44_BlendIndexed2(float* out, unsigned int arg, const void* a, const void* b);
void Skin_TransformPositionAndNormalProjective(float* out, float* out2, const void* a, const void* b, const float* c);
void Mtx34_TransformPointAndDirection(void* out, void* out2, const void* a, const void* b, const float* c);
void Mtx44_TransformPointPerspective(float* out, const void* a, const float* b);

typedef __vec2x32float__ Vec2;

#pragma push
#pragma optimization_level 2
#pragma peephole on
#pragma optimize_for_size on
#pragma use_lmw_stmw on
// Blend up to two palette matrices; the second weight is one minus the accumulated first weight.
// out holds 16 floats; matrixPaletteAddress addresses 64-byte 4x4 matrices.
// boneIndices has two bytes: 0x20 terminates, other values select a matrix.
// Only weightValues[0] is read. Two indices yield w0*M0 + (1-w0)*M1; an early
// terminator leaves the partial sum (zero if first). Weights are not clamped or
// normalized on the one-influence path. Paired arithmetic order is intentional.
float* Mtx44_BlendIndexed2(float* out, unsigned int matrixPaletteAddress, const void* weightValues, const void* boneIndices) {
    struct BlendScratch {
        unsigned int matrixPalette;
        float currentWeight;
        float accumulatedWeight;
    } frame;
    float zero = 0.0f;
    register Vec2 sum0 = zero;
    register Vec2 sum1 = sum0;
    register Vec2 sum2 = sum0;
    register Vec2 sum3 = sum0;
    register Vec2 sum4 = sum0;
    register Vec2 sum5 = sum0;
    register Vec2 sum6 = sum0;
    register Vec2 sum7 = sum0;
    *(volatile unsigned int*)&frame.matrixPalette = matrixPaletteAddress;
    *(volatile float*)&frame.accumulatedWeight = 0.0f;
    __alloca(0);
    register Vec2* outputPairs = (Vec2*)out;
    register unsigned char* indices = (unsigned char*)boneIndices;
    register float* weights = (float*)weightValues;
    register unsigned int influenceIndex;
    influenceIndex = 0;
    // Accumulate the eight paired matrix lanes before publishing any output.
    while (influenceIndex < kSkinWeightCount) {
        register unsigned int boneIndex = indices[influenceIndex];
        if (boneIndex == kUnusedBoneIndex) break;
        if (influenceIndex == 1) frame.currentWeight = 1.0f - frame.accumulatedWeight;
        else frame.currentWeight = weights[influenceIndex];
        register Vec2* palettePairs = (Vec2*)((char*)frame.matrixPalette + (boneIndex << kMatrixByteShift));
        ++influenceIndex;
        float scalarWeight = frame.currentWeight;
        Vec2 v0 = palettePairs[0], v1 = palettePairs[1];
        float total = frame.accumulatedWeight + scalarWeight;
        Vec2 v2 = palettePairs[2];
        register Vec2 pairedWeight = scalarWeight;
        sum0 = sum0 + v0 * pairedWeight;
        v0 = palettePairs[3];
        sum1 = sum1 + v1 * pairedWeight;
        v1 = palettePairs[4];
        sum2 = sum2 + v2 * pairedWeight;
        v2 = palettePairs[5];
        sum3 = sum3 + v0 * pairedWeight;
        v0 = palettePairs[6];
        sum4 = sum4 + v1 * pairedWeight;
        v1 = palettePairs[7];
        sum5 = sum5 + v2 * pairedWeight;
        sum6 = sum6 + v0 * pairedWeight;
        sum7 = sum7 + v1 * pairedWeight;
        frame.accumulatedWeight = total;
    }
    outputPairs[0] = sum0; outputPairs[1] = sum1; outputPairs[2] = sum2; outputPairs[3] = sum3;
    outputPairs[4] = sum4; outputPairs[5] = sum5; outputPairs[6] = sum6; outputPairs[7] = sum7;
    return out;
}
#pragma pop

#pragma push
#pragma optimization_level 0
#pragma peephole on
#pragma optimize_for_size on
#pragma use_lmw_stmw on
// Skin interleaved position/color vertices into a packed output buffer.
// Input stride 24 bytes, output stride 16: projected xyz plus unchanged color.
void Skin_Transform2WeightPositionColor(float* out, const void* records, unsigned int vertexCount, unsigned int matrixPalette) {
    register unsigned int vertexIndex = 0;
    register unsigned char* dst = (unsigned char*)out;
    register const unsigned char* sourceBytes = (const unsigned char*)records;
    while (vertexIndex < vertexCount) {
        float blendedMatrix[16];
        Mtx44_BlendIndexed2(blendedMatrix, matrixPalette, sourceBytes + kColorWeightOffset, sourceBytes + kColorBoneIndicesOffset);
        Mtx44_TransformPointPerspective((float*)dst, sourceBytes, blendedMatrix);
        dst[12] = sourceBytes[12]; dst[13] = sourceBytes[13]; dst[14] = sourceBytes[14]; dst[15] = sourceBytes[15];
        dst += 0x10;
        sourceBytes += sizeof(WeightedPositionColor);
        ++vertexIndex;
    }
}
#pragma pop

#pragma push
#pragma optimization_level 0
#pragma peephole on
#pragma optimize_for_size on
// Skin position/normal pairs and submit them with one UV set to GX.
// Input stride 40 bytes; UV and normal follow the transformed position in the FIFO.
void Skin_Submit2WeightPositionNormalTex1(const void* data, unsigned int vertexCount, unsigned int matrixPalette) {
    register unsigned int vertexIndex = 0;
    register const unsigned char* vertex = (const unsigned char*)data;
    while (vertexIndex < vertexCount) {
        float blendedMatrix[16], skinnedPosition[3], skinnedNormal[3];
        Mtx44_BlendIndexed2(blendedMatrix, matrixPalette, vertex + kNormalWeightOffset, vertex + kNormalBoneIndicesOffset);
        Skin_TransformPositionAndNormalProjective(skinnedPosition, skinnedNormal, vertex, vertex + kNormalOffset, blendedMatrix);
        Render_WriteFifo3f(skinnedPosition[0], skinnedPosition[1], skinnedPosition[2]);
        Render_WriteFifo3f(skinnedNormal[0], skinnedNormal[1], skinnedNormal[2]);
        Render_WriteTexCoord2f(*(float*)(vertex + kSkinUOffset), *(float*)(vertex + kSkinVOffset));
        vertex += sizeof(WeightedPositionNormalTex);
        ++vertexIndex;
    }
}
#pragma pop

struct PositionNormalColorTexVertex {
    float x, y, z;
    float normalX, normalY, normalZ;
    unsigned int packedColor;
    float u, v;
};

#pragma push
#pragma optimization_level 0
#pragma peephole on
#pragma optimize_for_size on
// Submit position, normal, packed color, and one UV set for each vertex.
void Render_WriteVerticesPositionNormalColorTex1(PositionNormalColorTexVertex* data, volatile unsigned int vertexCount) {
    register unsigned int vertexIndex = 0;
    register PositionNormalColorTexVertex* vertex = data;
    while (vertexIndex < vertexCount) {
        Render_WriteFifo3f(vertex->x, vertex->y, vertex->z);
        Render_WriteNormal3f(vertex->normalX, vertex->normalY, vertex->normalZ);
        register unsigned int t = vertex->packedColor;
        *(volatile unsigned int*)kGxFifoAddress = t;
        Render_WriteTexCoord2f(vertex->u, vertex->v);
        vertex++;
        vertexIndex++;
    }
}
#pragma pop

struct PositionColorThreeTexVertex {
    float x, y, z;
    unsigned int packedColor;
    float u0, v0, u1, v1, u2, v2;
};

#pragma push
#pragma optimization_level 0
#pragma peephole on
#pragma optimize_for_size on
#pragma use_lmw_stmw on
// Submit position, packed color, and three UV sets for each vertex.
void Render_WriteVerticesWithThreeTexCoords(volatile PositionColorThreeTexVertex* data, volatile unsigned int vertexCount) {
    register volatile PositionColorThreeTexVertex* vertex = data;
    register unsigned int vertexIndex = 0;
    while (vertexIndex < vertexCount) {
        Render_WriteFifo3f(vertex->x, vertex->y, vertex->z);
        volatile unsigned int packedColor;
        packedColor = *(volatile unsigned int*)kGxFifoAddress = vertex->packedColor;
        Render_WriteTexCoord2f(vertex->u0, vertex->v0);
        Render_WriteTexCoord2f(vertex->u1, vertex->v1);
        Render_WriteTexCoord2f(vertex->u2, vertex->v2);
        vertex++;
        vertexIndex++;
    }
}
#pragma pop

#pragma push
#pragma optimization_level 0
#pragma peephole on
#pragma optimize_for_size on
// Skin each position and submit its original packed color to GX.
void Skin_Submit2WeightPositionColor(const void* data, volatile unsigned int vertexCount, unsigned int matrixPalette) {
    register const volatile unsigned char* vertex = (const volatile unsigned char*)data;
    register unsigned int vertexIndex = 0;
    while (vertexIndex < vertexCount) {
        float blendedMatrix[16];
        float skinnedPosition[3];
        Mtx44_BlendIndexed2(blendedMatrix, matrixPalette, (const void*)(vertex + kColorWeightOffset), (const void*)(vertex + kColorBoneIndicesOffset));
        Mtx44_TransformPointPerspective(skinnedPosition, (const void*)vertex, blendedMatrix);
        Render_WriteFifo3f(skinnedPosition[0], skinnedPosition[1], skinnedPosition[2]);
        register unsigned int t = *(const volatile unsigned int*)(vertex + kPackedColorOffset);
        vertex += sizeof(WeightedPositionColor);
        vertexIndex++;
        *(volatile unsigned int*)kGxFifoAddress = t;
    }
}
#pragma pop

struct PositionColorThreeTexLayout {
    float x, y, z;
    unsigned int packedColor;
    float u0, v0, u1, v1, u2, v2;
};

struct PositionColorFourTexVertex {
    float x, y, z;
    unsigned int packedColor;
    float u0, v0, u1, v1, u2, v2, u3, v3;
};

 #pragma push
#pragma optimization_level 0
#pragma peephole on
#pragma optimize_for_size on
#pragma use_lmw_stmw on
// Submit position, packed color, and four UV sets for each vertex.
void Render_WriteVerticesWithFourTexCoords(volatile PositionColorFourTexVertex* data, volatile unsigned int vertexCount) {
    register volatile PositionColorFourTexVertex* vertex = data;
    register unsigned int vertexIndex = 0;
    while (vertexIndex < vertexCount) {
        Render_WriteFifo3f(vertex->x, vertex->y, vertex->z);
        volatile unsigned int packedColor;
        packedColor = *(volatile unsigned int*)kGxFifoAddress = vertex->packedColor;
        Render_WriteTexCoord2f(vertex->u0, vertex->v0);
        Render_WriteTexCoord2f(vertex->u1, vertex->v1);
        Render_WriteTexCoord2f(vertex->u2, vertex->v2);
        Render_WriteTexCoord2f(vertex->u3, vertex->v3);
        vertex++;
        vertexIndex++;
    }
}
#pragma pop

#pragma push
#pragma optimization_level 0
#pragma peephole on
#pragma optimize_for_size on
#pragma use_lmw_stmw on
// Skin and submit each position and normal pair to GX.
void Skin_Submit2WeightPositionNormal(const void* data, volatile unsigned int vertexCount, unsigned int matrixPalette) {
    register const volatile unsigned char* vertex = (const volatile unsigned char*)data;
    register unsigned int vertexIndex = 0;
    while (vertexIndex < vertexCount) {
        float blendedMatrix[16];
        float skinnedPosition[3];
        float skinnedNormal[3];
        Mtx44_BlendIndexed2(blendedMatrix, matrixPalette, (const void*)(vertex + kNormalWeightOffset), (const void*)(vertex + kNormalBoneIndicesOffset));
        Skin_TransformPositionAndNormalProjective(skinnedPosition, skinnedNormal, (const void*)vertex, (const void*)(vertex + kNormalOffset), blendedMatrix);
        Render_WriteFifo3f(skinnedPosition[0], skinnedPosition[1], skinnedPosition[2]);
        Render_WriteNormal3f(skinnedNormal[0], skinnedNormal[1], skinnedNormal[2]);
        vertex += sizeof(WeightedPositionNormal);
        vertexIndex++;
    }
}
#pragma pop

struct PositionColorTexVertex {
    float x;
    float y;
    float z;
    unsigned int packedColor;
    float u;
    float v;
};

struct PositionNormalVertex {
    float x;
    float y;
    float z;
    float normalX;
    float normalY;
    float normalZ;
};

struct PositionNormalTexVertex {
    float x, y, z;
    float normalX, normalY, normalZ;
    float u, v;
};

struct PositionNormalColorVertex {
    float x, y, z;
    float normalX, normalY, normalZ;
    unsigned int packedColor;
};

#pragma push
#pragma optimization_level 0
#pragma peephole on
#pragma optimize_for_size on
#pragma use_lmw_stmw on
// Submit each shared 16-bit vertex index 6 times for the enabled indexed attributes.
void Render_WriteSixSharedIndices(const volatile unsigned short* data, unsigned int vertexCount) {
    volatile unsigned int n = vertexCount;
    register unsigned int vertexIndex = 0;
    register const volatile unsigned short* vertex = data;
    while (vertexIndex < n) {
        *(volatile unsigned short*)kGxFifoAddress = vertex[0];
        *(volatile unsigned short*)kGxFifoAddress = vertex[0];
        *(volatile unsigned short*)kGxFifoAddress = vertex[0];
        *(volatile unsigned short*)kGxFifoAddress = vertex[0];
        *(volatile unsigned short*)kGxFifoAddress = vertex[0];
        *(volatile unsigned short*)kGxFifoAddress = vertex[0];
        vertexIndex++;
        vertex++;
    }
}
#pragma pop

#pragma push
#pragma optimization_level 0
// Submit each position/normal pair and its UV coordinates.
void Render_WriteVerticesPositionNormalTex1(PositionNormalTexVertex* data, volatile unsigned int vertexCount) {
    register unsigned int vertexIndex = 0;
    register PositionNormalTexVertex* vertex = data;
    while (vertexIndex < vertexCount) {
        Render_WriteFifo3f(vertex->x, vertex->y, vertex->z);
        Render_WriteNormal3f(vertex->normalX, vertex->normalY, vertex->normalZ);
        Render_WriteTexCoord2f(vertex->u, vertex->v);
        vertex++;
        vertexIndex++;
    }
}
#pragma pop

#pragma push
#pragma optimization_level 0
#pragma peephole on
#pragma optimize_for_size on
// Submit each position/normal pair and its packed color.
void Render_WriteVerticesPositionNormalColor(volatile PositionNormalColorVertex* data, volatile unsigned int vertexCount) {
    register unsigned int vertexIndex = 0;
    register volatile PositionNormalColorVertex* vertex = data;
    while (vertexIndex < vertexCount) {
        Render_WriteFifo3f(vertex->x, vertex->y, vertex->z);
        Render_WriteNormal3f(vertex->normalX, vertex->normalY, vertex->normalZ);
        register unsigned int packedColor = vertex->packedColor;
        vertex++;
        vertexIndex++;
        *(volatile unsigned int*)kGxFifoAddress = packedColor;
    }
}
#pragma pop

struct PositionColorTwoTexVertex {
    float x, y, z;
    unsigned int packedColor;
    float s0, t0, s1, t1;
};

#pragma push
#pragma optimization_level 0
#pragma peephole on
#pragma optimize_for_size on
#pragma use_lmw_stmw on
// Skin interleaved positions/normals into consecutive six-float output records.
// Input stride 32, output stride 24 bytes. This CPU path uses the affine point/
// direction helper, unlike the projective submit paths; no normalization occurs
// in this loop. unused is an unused ABI slot; matrixPalette selects the palette.
void Skin_Transform2WeightPositionNormal(float* out, volatile unsigned char* data, unsigned int count,
                 unsigned int unused, volatile unsigned int matrixPalette) {
    register unsigned int vertexIndex = 0;
    register float* outputRecord = out;
    register const unsigned char* inputRecord = (const unsigned char*)data;
    while (vertexIndex < count) {
        float blendedMatrix[16];
        Mtx44_BlendIndexed2(blendedMatrix, matrixPalette, (const void*)(inputRecord + kNormalWeightOffset), (const void*)(inputRecord + kNormalBoneIndicesOffset));
        Mtx34_TransformPointAndDirection(outputRecord, outputRecord + 3, (const void*)inputRecord, (const void*)(inputRecord + kNormalOffset), blendedMatrix);
        inputRecord += sizeof(WeightedPositionNormal);
        outputRecord += 6;
        vertexIndex++;
    }
}
#pragma pop

 #pragma push
#pragma optimization_level 0
#pragma peephole on
#pragma optimize_for_size on
#pragma use_lmw_stmw on
// Submit position, packed color, and two UV sets for each vertex.
void Render_WriteVerticesPositionAttributeTwoTexCoords(volatile PositionColorTwoTexVertex* data, volatile unsigned int vertexCount) {
    register volatile PositionColorTwoTexVertex* vertex = data;
    register unsigned int vertexIndex = 0;
    while (vertexIndex < vertexCount) {
        Render_WriteFifo3f(vertex->x, vertex->y, vertex->z);
        register unsigned int packedColor = vertex->packedColor;
        *(volatile unsigned int*)kGxFifoAddress = packedColor;
        Render_WriteTexCoord2f(vertex->s0, vertex->t0);
        Render_WriteTexCoord2f(vertex->s1, vertex->t1);
        vertex++;
        vertexIndex++;
    }
}
#pragma pop

#pragma push
#pragma optimization_level 0
#pragma peephole on
#pragma optimize_for_size on
#pragma use_lmw_stmw on
// Submit each position, packed color, and UV pair.
void Render_WritePositionColorTexcoordVertices(PositionColorTexVertex* data, volatile unsigned int vertexCount) {
    register unsigned int vertexIndex = 0;
    register PositionColorTexVertex* vertex = data;
    while (vertexIndex < vertexCount) {
        Render_WriteFifo3f(vertex->x, vertex->y, vertex->z);
        register unsigned int packedColor = vertex->packedColor;
        *(volatile unsigned int*)kGxFifoAddress = packedColor;
        Render_WriteTexCoord2f(vertex->u, vertex->v);
        vertex++;
        vertexIndex++;
    }
}
#pragma pop

#pragma push
#pragma optimization_level 0
#pragma peephole on
#pragma optimize_for_size on
#pragma use_lmw_stmw on
// Submit each position and normal pair to GX.
void Render_WriteVerticesPositionNormal(PositionNormalVertex* data, volatile unsigned int vertexCount) {
    register unsigned int vertexIndex = 0;
    register PositionNormalVertex* vertex = data;
    while (vertexIndex < vertexCount) {
        Render_WriteFifo3f(vertex->x, vertex->y, vertex->z);
        Render_WriteNormal3f(vertex->normalX, vertex->normalY, vertex->normalZ);
        vertex++;
        vertexIndex++;
    }
}
#pragma pop

#pragma push
#pragma optimization_level 1
// Write a normal's three float components to the GX FIFO in order.
void Render_WriteNormal3f(float x, float y, float z) {
    volatile float* fifoXAndZ = (volatile float*)kGxFifoAddress;
    volatile float* fifoY = (volatile float*)kGxFifoAddress;
    *fifoXAndZ = x;
    fifoXAndZ = (volatile float*)kGxFifoAddress;
    *fifoY = y;
    *fifoXAndZ = z;
}
#pragma pop

struct PositionColorVertex {
    float x;
    float y;
    float z;
    unsigned int packedColor;
};

#pragma push
#pragma optimization_level 0
#pragma peephole on
#pragma optimize_for_size on
#pragma use_lmw_stmw on
// Submit each position followed by its packed color word.
void Render_WritePositionColorVertices(PositionColorVertex* data, volatile unsigned int vertexCount) {
    register unsigned int vertexIndex = 0;
    register PositionColorVertex* vertex = data;
    while (vertexIndex < vertexCount) {
        Render_WriteFifo3f(vertex->x, vertex->y, vertex->z);
        register unsigned int t = vertex->packedColor;
        vertex++;
        vertexIndex++;
        *(volatile unsigned int*)kGxFifoAddress = t;
    }
}
#pragma pop

#pragma push
#pragma optimization_level 0
#pragma peephole on
#pragma optimize_for_size on
// Submit each position followed by its packed attribute word.
void Render_WritePositionAttributeVertices(PositionColorVertex* data, volatile unsigned int vertexCount) {
    register unsigned int vertexIndex = 0;
    register PositionColorVertex* vertex = data;
    while (vertexIndex < vertexCount) {
        Render_WriteFifo3f(vertex->x, vertex->y, vertex->z);
        register unsigned int t = vertex->packedColor;
        vertex++;
        vertexIndex++;
        *(volatile unsigned int*)kGxFifoAddress = t;
    }
}
#pragma pop

#pragma push
#pragma optimization_level 0
#pragma peephole on
#pragma optimize_for_size on
#pragma use_lmw_stmw on
// Submit each shared 16-bit vertex index 2 times for the enabled indexed attributes.
void Render_WriteTwoSharedIndicesA(const volatile unsigned short* data, unsigned int vertexCount) {
    volatile unsigned int n = vertexCount;
    register unsigned int vertexIndex = 0;
    register const volatile unsigned short* vertex = data;
    while (vertexIndex < n) {
        *(volatile unsigned short*)kGxFifoAddress = vertex[0];
        vertexIndex++;
        *(volatile unsigned short*)kGxFifoAddress = vertex[0];
        vertex++;
    }
}
#pragma pop

#pragma push
#pragma optimization_level 0
#pragma peephole on
#pragma optimize_for_size on
#pragma use_lmw_stmw on
// Submit each shared 16-bit vertex index 2 times for the enabled indexed attributes.
void Render_WriteTwoSharedIndicesB(const volatile unsigned short* data, unsigned int vertexCount) {
    volatile unsigned int n = vertexCount;
    register const volatile unsigned short* vertex = data;
    register unsigned int vertexIndex = 0;
    while (vertexIndex < n) {
        *(volatile unsigned short*)kGxFifoAddress = vertex[0];
        vertexIndex++;
        *(volatile unsigned short*)kGxFifoAddress = vertex[0];
        vertex++;
    }
}

#pragma push
#pragma optimization_level 0
#pragma peephole on
#pragma optimize_for_size on
#pragma use_lmw_stmw on
// Submit each shared 16-bit vertex index 2 times for the enabled indexed attributes.
void Render_WriteTwoSharedIndicesC(const volatile unsigned short* data, unsigned int vertexCount) {
    volatile unsigned int n = vertexCount;
    register unsigned int vertexIndex = 0;
    register const volatile unsigned short* vertex = data;
    while (vertexIndex < vertexCount) {
        *(volatile unsigned short*)kGxFifoAddress = vertex[0];
        vertexIndex++;
        *(volatile unsigned short*)kGxFifoAddress = vertex[0];
        vertex++;
    }
}
#pragma pop
#pragma pop

#pragma push
#pragma optimization_level 0
#pragma peephole on
#pragma optimize_for_size on
#pragma use_lmw_stmw on
// Submit each shared 16-bit vertex index 2 times for the enabled indexed attributes.
void Render_WriteTwoSharedIndicesD(const volatile unsigned short* data, unsigned int vertexCount) {
    volatile unsigned int n = vertexCount;
    register unsigned int vertexIndex = 0;
    register const volatile unsigned short* vertex = data;
    while (vertexIndex < n) {
        *(volatile unsigned short*)kGxFifoAddress = vertex[0];
        vertexIndex++;
        *(volatile unsigned short*)kGxFifoAddress = vertex[0];
        vertex++;
    }
}
#pragma pop

#pragma push
#pragma optimization_level 0
#pragma peephole on
#pragma optimize_for_size on
#pragma use_lmw_stmw on
// Submit each shared 16-bit vertex index 2 times for the enabled indexed attributes.
void Render_WriteTwoSharedIndicesE(const volatile unsigned short* data, unsigned int vertexCount) {
    volatile unsigned int n = vertexCount;
    register unsigned int vertexIndex = 0;
    register const volatile unsigned short* vertex = data;
    while (vertexIndex < n) {
        *(volatile unsigned short*)kGxFifoAddress = vertex[0];
        vertexIndex++;
        *(volatile unsigned short*)kGxFifoAddress = vertex[0];
        vertex++;
    }
}
#pragma pop

#pragma push
#pragma optimization_level 0
#pragma peephole on
#pragma optimize_for_size on
// Submit each shared 16-bit vertex index 3 times for the enabled indexed attributes.
void Render_WriteThreeSharedIndicesB(const volatile unsigned short* data, volatile unsigned int vertexCount) {
    volatile unsigned int n = vertexCount;
    register unsigned int vertexIndex = 0;
    register const volatile unsigned short* vertex = data;
    while (vertexIndex < n) {
        *(volatile unsigned short*)kGxFifoAddress = vertex[0];
        vertexIndex++;
        *(volatile unsigned short*)kGxFifoAddress = vertex[0];
        *(volatile unsigned short*)kGxFifoAddress = vertex[0];
        vertex++;
    }
}
#pragma pop

#pragma push
#pragma optimization_level 0
#pragma peephole on
#pragma optimize_for_size on
#pragma use_lmw_stmw on
// Submit each shared 16-bit vertex index 3 times for the enabled indexed attributes.
void Render_WriteThreeSharedIndicesC(const volatile unsigned short* data, unsigned int vertexCount) {
    volatile unsigned int n = vertexCount;
    register unsigned int vertexIndex = 0;
    register const volatile unsigned short* vertex = data;
    while (vertexIndex < n) {
        *(volatile unsigned short*)kGxFifoAddress = vertex[0];
        vertexIndex++;
        *(volatile unsigned short*)kGxFifoAddress = vertex[0];
        *(volatile unsigned short*)kGxFifoAddress = vertex[0];
        vertex++;
    }
}
#pragma pop

#pragma push
#pragma optimization_level 0
#pragma peephole on
#pragma optimize_for_size on
#pragma use_lmw_stmw on
// Submit each shared 16-bit vertex index 3 times for the enabled indexed attributes.
void Render_WriteThreeSharedIndicesD(const volatile unsigned short* data, unsigned int vertexCount) {
    volatile unsigned int n = vertexCount;
    register unsigned int vertexIndex = 0;
    register const volatile unsigned short* vertex = data;
    while (vertexIndex < n) {
        *(volatile unsigned short*)kGxFifoAddress = vertex[0];
        vertexIndex++;
        *(volatile unsigned short*)kGxFifoAddress = vertex[0];
        *(volatile unsigned short*)kGxFifoAddress = vertex[0];
        vertex++;
    }
}
#pragma pop

#pragma push
#pragma optimization_level 0
#pragma peephole on
#pragma optimize_for_size on
#pragma use_lmw_stmw on
// Submit each shared 16-bit vertex index 3 times for the enabled indexed attributes.
void Render_WriteThreeSharedIndicesA(volatile unsigned short* data, unsigned int vertexCount) {
    volatile unsigned int n = vertexCount;
    register unsigned int vertexIndex = 0;
    register volatile unsigned short* vertex = data;
    while (vertexIndex < n) {
        *(volatile unsigned short*)kGxFifoAddress = vertex[0];
        vertexIndex++;
        *(volatile unsigned short*)kGxFifoAddress = vertex[0];
        *(volatile unsigned short*)kGxFifoAddress = vertex[0];
        vertex++;
    }
}
#pragma pop

#pragma push
#pragma optimization_level 0
#pragma peephole on
#pragma optimize_for_size on
#pragma use_lmw_stmw on
// Submit each shared 16-bit vertex index 4 times for the enabled indexed attributes.
void Render_WriteFourSharedIndicesA(volatile unsigned short* data, volatile unsigned int vertexCount) {
    register unsigned int vertexIndex = 0;
    register volatile unsigned short* d = data;
    while (vertexIndex < vertexCount) {
        *(volatile unsigned short*)kGxFifoAddress = d[0];
        vertexIndex++;
        *(volatile unsigned short*)kGxFifoAddress = d[0];
        *(volatile unsigned short*)kGxFifoAddress = d[0];
        *(volatile unsigned short*)kGxFifoAddress = d[0];
        d++;
    }
}
#pragma pop

#pragma push
#pragma optimization_level 0
#pragma peephole on
#pragma optimize_for_size on
// Submit each shared 16-bit vertex index 4 times for the enabled indexed attributes.
void Render_WriteFourSharedIndicesB(volatile unsigned short* data, unsigned int vertexCount) {
    volatile unsigned int n = vertexCount;
    register unsigned int vertexIndex = 0;
    register volatile unsigned short* d = data;
    while (vertexIndex < n) {
        *(volatile unsigned short*)kGxFifoAddress = d[0];
        vertexIndex++;
        *(volatile unsigned short*)kGxFifoAddress = d[0];
        *(volatile unsigned short*)kGxFifoAddress = d[0];
        *(volatile unsigned short*)kGxFifoAddress = d[0];
        d++;
    }
}
#pragma pop

#pragma push
#pragma optimization_level 0
#pragma peephole on
#pragma optimize_for_size on
#pragma use_lmw_stmw on
// Submit each shared 16-bit vertex index 5 times for the enabled indexed attributes.
void Render_WriteFiveSharedIndices(const volatile unsigned short* data, unsigned int vertexCount) {
    volatile unsigned int n = vertexCount;
    register unsigned int vertexIndex = 0;
    register const volatile unsigned short* vertex = data;
    while (vertexIndex < n) {
        *(volatile unsigned short*)kGxFifoAddress = vertex[0];
        vertexIndex++;
        *(volatile unsigned short*)kGxFifoAddress = vertex[0];
        *(volatile unsigned short*)kGxFifoAddress = vertex[0];
        *(volatile unsigned short*)kGxFifoAddress = vertex[0];
        *(volatile unsigned short*)kGxFifoAddress = vertex[0];
        vertex++;
    }
}
#pragma pop

} // extern "C"
