// Retry menu background draw, and the render packets that batch course geometry read from SBIN
// course files. A packet owns its batches and the shared resources they reference.

#include <decomp/static_init.h>
#pragma ipa file

#include "classes/CRetryMenu.h"
#include "classes/RenderPacketFields.h"
#include "classes/ArchiveEntryInfo.h"
#include <decomp/cw_bitfields.h>
// Normal destruction remains explicit. This owner handles exceptional exits.
struct EH5Cleanup {
    void* object;
    void (*destroy)(void*, int);
    bool active;
    // Arm cleanup of a constructed local owner without taking ownership of its object storage.
    EH5Cleanup(void* p, void (*d)(void*, int)) : object(p), destroy(d), active(true) {}
    // Disarm immediately before the matching explicit normal-path destructor.
    void release() {
        active = false;
    }
    // On unwinding only, destroy an owner whose normal-path destructor has not run.
    ~EH5Cleanup() {
        if (active)
            destroy(object, -1);
    }
};

#pragma auto_inline off

extern "C" {

extern float kFloat_6_2831855;
extern float s_courseRenderPacketFullTurn;
extern float kFloat_1_RetryMenu;

extern void* memcpy(void*, const void*, unsigned int);
extern void* memset(void*, int, unsigned int);

extern void UIRender_SetDefaultState(void*);
extern void Engine_SetTextureStageResource(int, int);
extern void Engine_SetTexturePalette(int, int);
extern void ResultMenu_DrawSharedQuad();
extern void Layout_DrawWithOpacity(void*, float);
extern void PSMTX44Identity(void*);
extern void appFreeUni__FPv(void*);
extern void CubeResourceStorage_dtor(void*, int);
extern void __dl__FPv(void*);
extern void sprintf(void*, const char*, ...);
extern void* g_resourceManager;
extern const char packedResourcePathStrings[];
// Packed names shared with the course archive/vector code (global typing round 0).
struct CourseArchiveStringPool {
    char fileNameFormat[6];
    char archiveExtension[5];
    char packetExtension[5];
    char textureNameFormat[7];
    char vectorLengthError[20];
    char lengthErrorType[55];
};
#define COURSE_ARCHIVE_STRING(member) (packedResourcePathStrings + (int)&((CourseArchiveStringPool*)0)->member)
enum CourseArchiveAllocation {
    kReadBufferPadding = 0x20
};

extern void* Resource_OpenResolvedPath(void*, const char*, int, int, int, int);
extern void ResourceHolder_SetResourceAddress(void*, void*);
extern void* CourseFile_GetByteLength(void*);
extern void Resource_Release(void*);
extern void CourseModelByteBuffer_Init(void*);
extern void CourseModelByteBuffer_ResizeZeroFilled(void*, void*);
extern void* CourseModelByteBuffer_GetElement(void*, int);
extern void* Archive_Open(void*, void*, int);
extern void Archive_GetEntryInfo(void*, int, void*);
extern void Archive_ReadEntry(void*, int, void*, void*, int);
extern void Archive_Close(void*);
extern void* Texture_CreateFromImageDescription(void*, void*, void*);
extern void* VertexBuffer_Create(unsigned int, int, unsigned int, void*, void*, int);
extern void* Resource_GetOffsetPtr(void*, int, int, void**, int);
extern void GraphicsBuffer_Flush(void*);
extern void* GX_GetPrimitiveCount(int, int);
extern void CourseRenderPacket_AppendBatch(void*, void*);
extern void Triple_Clear(void*);
extern void CoursePacketPositionVector_Resize(void*, int);
extern void* CoursePacketPositionVector_GetElement(void*, int);
extern void CoursePacketTexCoordVector_Init(void*);
extern void CoursePacketTexCoordVector_Resize(void*, int);
extern void* CoursePacketTexCoordVector_GetElement(void*, int);
extern void CoursePacketVertexVector_Init(void*);
extern void CoursePacketVertexVector_Resize(void*, int);
extern void* CoursePacketVertexVector_GetElement(void*, int);
extern int CoursePacketVertexVector_GetCount(void*);
extern void gtl_PixelCopy_AssignPackedColor(void*, void*);
extern void ResourceHandle_dtor(void*, int);
extern void ByteVector_dtor(void*, int);
extern void dtor_80263084(void*, int);
extern void dtor_80262A30(void*, int);
extern void dtor_802636C0(void*, int);

// Initialize this unit's angle conversion constant.
void CourseRenderPacket_InitializeConstants() {
    s_courseRenderPacketFullTurn = kFloat_6_2831855;
}

// Copy size bytes and advance the borrowed cursor; caller guarantees both ranges (no bounds check).
void Buffer_ReadAndAdvance(void* dest, void** srcPtr, int size) {
    if (size != 0) {
        memcpy(dest, *srcPtr, size);
        *srcPtr = (char*)*srcPtr + size;
    }
}

} // extern "C"

// Draw the retry background quad and its layout at full opacity.
void CRetryMenu::Draw() {
    UIRender_SetDefaultState(this);
    Engine_SetTextureStageResource(0, 0);
    Engine_SetTexturePalette(0, 0);
    ResultMenu_DrawSharedQuad();
    Layout_DrawWithOpacity(layout, kFloat_1_RetryMenu);
}

extern "C" {

// Zero the packet's auxiliary state without changing its batch storage or transform.
static inline void ClearPacketAuxiliaryState(RenderPacketFields* packet) {
    memset(packet->auxiliaryState, 0, sizeof(packet->auxiliaryState));
}

// Initialize an empty packet, identity transform and cleared auxiliary state.
void* RenderPacket_Init(void* self) {
    AsRenderPacketFields(self)->resetMarker = 0;
    AsRenderPacketFields(self)->loaded = 0;
    AsRenderPacketFields(self)->capacity = 0;
    AsRenderPacketFields(self)->entries = 0;
    AsRenderPacketFields(self)->size = 0;
    if (self == 0)
        appFreeUni__FPv(self);
    PSMTX44Identity(AsRenderPacketFields(self)->matrix); // RenderPacket_Init: +0xC
    ClearPacketAuxiliaryState(AsRenderPacketFields(self));
    return self;
}

struct PacketBatch {
    void* vertexBuffer;
    void* texture;
    void* primitiveCount;
};
struct RefCountWord {
    CW_BITS2(unsigned hi : 16, unsigned lo : 16)
};

// Release both shared resources in every batch, then clear packet storage and state.
void RenderPacket_ClearResources(void* self) {
    int zero = 0;
    char* p = (char*)self;
    PacketBatch* entry = (PacketBatch*)*((void**)p);
    while (entry != (PacketBatch*)*((void**)p) + AsRenderPacketFields(p)->size) {
        void* vertexBuffer = entry->vertexBuffer;
        if (vertexBuffer != 0) {
            RefCountWord* rc = (RefCountWord*)vertexBuffer;
            rc->hi = rc->hi - 1;
            if (rc->hi == 0)
                CubeResourceStorage_dtor(vertexBuffer, 1);
            entry->vertexBuffer = 0;
        }
        void* texture = entry->texture;
        if (texture != 0) {
            RefCountWord* rc = (RefCountWord*)texture;
            rc->hi = rc->hi - 1;
            if (rc->hi == 0)
                CubeResourceStorage_dtor(texture, 1);
            entry->texture = 0;
        }
        ++entry;
    }
    void* allocation = *((void**)p);
    AsRenderPacketFields(p)->loaded = zero;
    AsRenderPacketFields(p)->capacity = zero;
    *((void**)p) = (void*)zero;
    AsRenderPacketFields(p)->size = zero;
    if (allocation != 0)
        appFreeUni__FPv(allocation);
    PSMTX44Identity(AsRenderPacketFields(p)->matrix);
    ClearPacketAuxiliaryState(AsRenderPacketFields(p));
    AsRenderPacketFields(p)->resetMarker = 0;
}

struct DtorSelf80261DF4 {
    void* p;
    volatile int n;
};
// Free the owned vector allocation and optionally the vector object.
void* dtor_80261DF4(void* self, int flag) {
    if (self != 0) {
        // Original 0x80261DF4: leave the deleting-destructor tail shared.
        do {
            if (self == 0)
                break;
            if (self == 0)
                break;
            if (self == 0)
                break;
            DtorSelf80261DF4* s = (DtorSelf80261DF4*)self;
            void* member = s->p;
            if (member != 0) {
                int t = s->n;
                s->n = t - t;
                appFreeUni__FPv(member);
            }
        } while (0);
    }
    if (flag > 0) {
        __dl__FPv(self);
    }
    return self;
}

// Discard batch storage and mark the packet reset; callers must release resource references first.
void RenderPacket_Reset(void* self) {
    char* p = (char*)self;
    void* member = *((void**)p);
    int zero = 0;
    AsRenderPacketFields(p)->loaded = zero;
    if (member != 0) {
        // handled below
    }
    AsRenderPacketFields(p)->capacity = zero;
    AsRenderPacketFields(p)->entries = zero;
    AsRenderPacketFields(p)->size = zero;
    if (member != 0) {
        appFreeUni__FPv(member);
    }
    PSMTX44Identity(AsRenderPacketFields(p)->matrix);
    ClearPacketAuxiliaryState(AsRenderPacketFields(p));
    AsRenderPacketFields(p)->resetMarker = 1;
}

enum SbinConstants {
    kSbinTag = 0x5342494E,     // "SBIN"
    kSbinVersion = 0x302E3130, // "0.10"
    kArchiveReadMode = 3,
    kTriangleVertexCount = 3,
    kGXTriangles = 0x90,
    kTrianglePositionWordOffset = 2,
    kTriangleTexCoordWordOffset = 5,
    kPacketVertexFormat = 0x120000 - 0xFFF
};
struct SbinHeader {
    unsigned int tag;
    unsigned int version;
};
#define PACKET_OFFSET(member) ((int)&((PacketBatch*)0)->member)
// Triangle records contain two unused words, then three position and three UV indices.
struct SbinTriangle {
    unsigned short reserved[2];
    unsigned short positions[3];
    unsigned short texCoords[3];
};
struct PacketVertex {
    float position[3];
    unsigned int color;
    float uv[2];
};
// ABI storage for callee-managed local owners, not raw payload arrays. Keep char alignment:
// byte buffers: Init/Resize/GetElement, ByteVector_dtor; positions: Triple_Clear, dtor_80262A30;
// UVs: TexCoordVector_Init, dtor_80263084; vertices: VertexVector_Init, dtor_802636C0.
// Pointers returned by GetElement are borrowed and invalidated by resize/destruction.
typedef char ByteBufferOwnerStorage[0x0C];
typedef char PositionVectorOwnerStorage[0x0C];
typedef char TexCoordVectorOwnerStorage[0x0C];
typedef char VertexVectorOwnerStorage[0x0C];
typedef char PacketBatchStorage[sizeof(PacketBatch)];

#pragma opt_common_subs off
struct ReadableFile {
    virtual void reserved0();
    virtual void reserved1();
    virtual void read(void*, void*);
};
// Read SBIN 0.10 positions, UVs and textured triangles from an archive into GPU batches.
// name is a base resource name; packedColor is copied to every output vertex. Returns 0 on
// null name/open failure/bad tag/version, 1 after appending batches; does not clear old batches.
// Trusted native-endian input only: no cursor end checks, count-overflow checks or index checks.
// Entry 0 schema, in order (u32 unless specified): tag, version, positionCount, texCoordCount,
// positions[positionCount] (3 floats each), UVs[texCoordCount] (2 floats each), ignored word,
// batchCount; then each batch: textureName[64], triangleCount, SbinTriangle[triangleCount].
// Each index must address the corresponding vector; each 64-byte name must contain a terminator.
// Archive texture entry batchIndex+1 must exist. Position space/units are passed through unchanged.
// Local vectors own their allocations; EH5Cleanup covers unwinding and explicit dtors cover returns.
// The archive handle is explicitly closed on the shown normal/error paths, with no EH archive guard.
int CourseRenderPacket_LoadSBINArchive(void* self, const char* name, int unusedArgument3, int packedColor,
                                       int unusedArgument5, int unusedArgument6) {
    void* file;
    void* holder;
    unsigned char* cursor;
    char token[4];
    int positionCount;
    int texCoordCount;
    int unusedHeaderWord;
    unsigned int batchCount;
    unsigned int triangleCount;
    void* mappedVertexData;
    int vertexColor = packedColor;
    struct SbinHeader header;
    ByteBufferOwnerStorage archiveBytes;
    ByteBufferOwnerStorage packetBytes;
    PositionVectorOwnerStorage positions;
    TexCoordVectorOwnerStorage texCoords;
    PacketBatchStorage batch;
    ByteBufferOwnerStorage textureBytes;
    VertexVectorOwnerStorage vertices;
    char triangleRecord[sizeof(SbinTriangle)];
    char textureName[0x40];
    char path[0x100];
    struct ArchiveEntryInfo entry;
    unsigned int firstVertex;
    unsigned short* triangleIndices;
    unsigned int batchIndex;
    unsigned int triangleIndex;

    // Phase 1: read the resource into owned bytes; the archive borrows those bytes until close.
    if (!name)
        return 0;
    sprintf(path, packedResourcePathStrings, name, COURSE_ARCHIVE_STRING(archiveExtension));
    file = Resource_OpenResolvedPath(g_resourceManager, path, -1, 0, 0, 0);
    ResourceHolder_SetResourceAddress(&holder, &file);
    EH5Cleanup holderCleanup(&holder, (void (*)(void*, int))ResourceHandle_dtor);
    if (file == 0) {
        {
            holderCleanup.release();
            ResourceHandle_dtor(&holder, -1);
        }
        return 0;
    }
    void* fileByteLength = CourseFile_GetByteLength(
        file); // original 0x80262060-0x80262080: r3 = file (lwz r3,0x30(r1)); returns its size (+4)
    CourseModelByteBuffer_Init(archiveBytes);
    EH5Cleanup archiveBytesCleanup(archiveBytes, (void (*)(void*, int))ByteVector_dtor);
    CourseModelByteBuffer_ResizeZeroFilled(archiveBytes, (char*)fileByteLength + kReadBufferPadding);
    void* archiveBuffer = CourseModelByteBuffer_GetElement(archiveBytes, 0);
    ((ReadableFile*)file)->read(archiveBuffer, fileByteLength);
    if (file)
        Resource_Release(file);
    file = 0;
    cursor = (unsigned char*)CourseModelByteBuffer_GetElement(archiveBytes, 0);
    void* archive = Archive_Open(cursor, fileByteLength, kArchiveReadMode);
    if (!archive) {
        {
            archiveBytesCleanup.release();
            ByteVector_dtor(archiveBytes, -1);
        }
        {
            holderCleanup.release();
            ResourceHandle_dtor(&holder, -1);
        }
        return 0;
    }
    // Phase 2: extract entry 0 and validate its two-word SBIN signature.
    Archive_GetEntryInfo(archive, -1, &entry);
    sprintf(path, packedResourcePathStrings, name, COURSE_ARCHIVE_STRING(packetExtension));
    Archive_GetEntryInfo(archive, 0, &entry);
    CourseModelByteBuffer_Init(packetBytes);
    EH5Cleanup packetBytesCleanup(packetBytes, (void (*)(void*, int))ByteVector_dtor);
    CourseModelByteBuffer_ResizeZeroFilled(packetBytes, (void*)entry.uncompressedSize);
    Archive_ReadEntry(archive, 0, CourseModelByteBuffer_GetElement(packetBytes, 0), (void*)entry.uncompressedSize,
                      kArchiveReadMode);
    cursor = (unsigned char*)CourseModelByteBuffer_GetElement(packetBytes, 0);
    Buffer_ReadAndAdvance(&header, (void**)&cursor, 8);
    if (header.tag != kSbinTag) {
        memcpy(token, &header.tag, 4);
        Archive_Close(archive);
        {
            packetBytesCleanup.release();
            ByteVector_dtor(packetBytes, -1);
        }
        {
            archiveBytesCleanup.release();
            ByteVector_dtor(archiveBytes, -1);
        }
        {
            holderCleanup.release();
            ResourceHandle_dtor(&holder, -1);
        }
        return 0;
    }
    if (header.version != kSbinVersion) {
        memcpy(token, &header.version, 4);
        Archive_Close(archive);
        {
            packetBytesCleanup.release();
            ByteVector_dtor(packetBytes, -1);
        }
        {
            archiveBytesCleanup.release();
            ByteVector_dtor(archiveBytes, -1);
        }
        {
            holderCleanup.release();
            ResourceHandle_dtor(&holder, -1);
        }
        return 0;
    }
    // Phase 3: decode shared position/UV arrays; they outlive every batch's index expansion.
    positionCount = 0;
    Buffer_ReadAndAdvance(&positionCount, (void**)&cursor, 4);
    texCoordCount = 0;
    Buffer_ReadAndAdvance(&texCoordCount, (void**)&cursor, 4);
    Triple_Clear(positions);
    EH5Cleanup positionsCleanup(positions, (void (*)(void*, int))dtor_80262A30);
    CoursePacketPositionVector_Resize(positions, positionCount);
    Buffer_ReadAndAdvance(CoursePacketPositionVector_GetElement(positions, 0), (void**)&cursor, positionCount * 12);
    CoursePacketTexCoordVector_Init(texCoords);
    EH5Cleanup texCoordsCleanup(texCoords, (void (*)(void*, int))dtor_80263084);
    CoursePacketTexCoordVector_Resize(texCoords, texCoordCount);
    Buffer_ReadAndAdvance(CoursePacketTexCoordVector_GetElement(texCoords, 0), (void**)&cursor, texCoordCount * 8);
    unusedHeaderWord = 0;
    Buffer_ReadAndAdvance(&unusedHeaderWord, (void**)&cursor, 4);
    batchCount = 0;
    Buffer_ReadAndAdvance(&batchCount, (void**)&cursor, 4);
    // Phase 4: load one texture and expand indexed triangles into independent GPU vertices.
    for (batchIndex = 0; batchIndex < batchCount; batchIndex++) {
        memset(batch, 0, 12);
        Buffer_ReadAndAdvance(textureName, (void**)&cursor, 0x40);
        sprintf(path, COURSE_ARCHIVE_STRING(textureNameFormat), textureName);
        unsigned int archiveTextureIndex = batchIndex + 1;
        Archive_GetEntryInfo(archive, archiveTextureIndex, &entry);
        void* data = (void*)entry.uncompressedSize;
        CourseModelByteBuffer_Init(textureBytes);
        EH5Cleanup textureBytesCleanup(textureBytes, (void (*)(void*, int))ByteVector_dtor);
        CourseModelByteBuffer_ResizeZeroFilled(textureBytes, data);
        Archive_ReadEntry(archive, archiveTextureIndex, CourseModelByteBuffer_GetElement(textureBytes, 0), data,
                          kArchiveReadMode);
        Texture_CreateFromImageDescription(CourseModelByteBuffer_GetElement(textureBytes, 0), data,
                                           batch + PACKET_OFFSET(texture));
        {
            textureBytesCleanup.release();
            ByteVector_dtor(textureBytes, -1);
        }
        triangleCount = 0;
        Buffer_ReadAndAdvance(&triangleCount, (void**)&cursor, 4);
        CoursePacketVertexVector_Init(vertices);
        EH5Cleanup verticesCleanup(vertices, (void (*)(void*, int))dtor_802636C0);
        CoursePacketVertexVector_Resize(vertices, triangleCount * kTriangleVertexCount);
        for (triangleIndex = 0, firstVertex = 0; triangleIndex < triangleCount; triangleIndex++) {
            Buffer_ReadAndAdvance(triangleRecord, (void**)&cursor, sizeof(SbinTriangle));
            triangleIndices = (unsigned short*)triangleRecord;
            for (unsigned int k = 0; k < kTriangleVertexCount; k++) {
                unsigned short texCoordIndex = triangleIndices[kTriangleTexCoordWordOffset];
                float* position = (float*)CoursePacketPositionVector_GetElement(
                    positions, triangleIndices[kTrianglePositionWordOffset]);
                float* uv = (float*)CoursePacketTexCoordVector_GetElement(texCoords, texCoordIndex);
                PacketVertex* vertex = (PacketVertex*)CoursePacketVertexVector_GetElement(vertices, k + firstVertex);
                vertex->position[0] = position[0];
                vertex->position[1] = position[1];
                vertex->position[2] = position[2];
                gtl_PixelCopy_AssignPackedColor(&vertex->color, &vertexColor);
                vertex->uv[0] = uv[0];
                vertex->uv[1] = uv[1];
                triangleIndices++;
            }
            firstVertex += kTriangleVertexCount;
        }
        // Upload the expanded buffer; AppendBatch copies the resource handles into the packet.
        int count = CoursePacketVertexVector_GetCount(vertices);
        if (count != 0) {
            int vertexBytes = count * sizeof(PacketVertex);
            VertexBuffer_Create(vertexBytes, 0, kPacketVertexFormat, 0, batch, 0);
            mappedVertexData = 0;
            Resource_GetOffsetPtr(((PacketBatch*)batch)->vertexBuffer, 0, 0, &mappedVertexData, 0);
            if (mappedVertexData) {
                memcpy(mappedVertexData, CoursePacketVertexVector_GetElement(vertices, 0), vertexBytes);
            }
            GraphicsBuffer_Flush(((PacketBatch*)batch)->vertexBuffer);
        }
        ((PacketBatch*)batch)->primitiveCount = GX_GetPrimitiveCount(count, kGXTriangles);
        CourseRenderPacket_AppendBatch(self, batch);
        {
            verticesCleanup.release();
            dtor_802636C0(vertices, -1);
        }
    }
    // Phase 5: close the borrowing archive before freeing byte owners; unwind locals in reverse order.
    Archive_Close(archive);
    AsRenderPacketFields(self)->loaded = 1;
    {
        texCoordsCleanup.release();
        dtor_80263084(texCoords, -1);
    }
    {
        positionsCleanup.release();
        dtor_80262A30(positions, -1);
    }
    {
        packetBytesCleanup.release();
        ByteVector_dtor(packetBytes, -1);
    }
    {
        archiveBytesCleanup.release();
        ByteVector_dtor(archiveBytes, -1);
    }
    {
        holderCleanup.release();
        ResourceHandle_dtor(&holder, -1);
    }
    return 1;
}

#pragma opt_common_subs reset

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. With "#pragma ipa file" the code is generated at
   the end of the file, so force_active must still be on there: a push/pop around the entry drops the flag, the linker
   dead-strips the entry and the initialiser never runs. */
#pragma force_active on
STATIC_INIT_ENTRY_NC(".CRT$XCU8046231C", _ctors_u80261D30_0, CourseRenderPacket_InitializeConstants);
