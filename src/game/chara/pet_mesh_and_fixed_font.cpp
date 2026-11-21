#include <decomp/static_init.h>
typedef unsigned long new_size_t;
#include "classes/PetMeshDescriptor.h"
#include "classes/PetMeshGeometry.h"
#include "classes/PetMeshVertexBlock.h"
#include "classes/WFixedFont.h"
#include "classes/IPetMesh.h"
#include "classes/FontCacheNode.h"
#include "classes/CSerifuEcb.h"

#define GAME_NSTRING_INLINE
#include "game/string.h"

// Stack-local narrow-string view; the canonical string ABI remains external.
class LocalNString_u8021C59C {
public:
    unsigned int flags;
    unsigned int length;
    char* data;

    // Initialize local engine-string storage using this constructor variant.
    LocalNString_u8021C59C() : flags(0), length(0), data(0) {}
#if defined(GAME_NSTRING_TEXT_CONSTRUCTOR)
    // Initialize local engine-string storage using this constructor variant.
    LocalNString_u8021C59C(const char* text) { MWString_ConstructCString(this, text); }
#endif
#if defined(GAME_NSTRING_RANGE_CONSTRUCTOR)
    // Initialize local engine-string storage using this constructor variant.
    LocalNString_u8021C59C(const char* text) {
        LocalNString_u8021C59C* object = this;
        unsigned int size;
        const char* source = text;
        object->flags = 0;
        object->length = 0;
        object->data = 0;
        size = nstring_detail::strlen(source);
        nstring_detail::String_ReserveCapacity(object, size);
        int allocator;
        char copy = *(char*)&allocator;
        nstring_detail::String_ReplaceRange(object, 0, 0, source, source + size, &copy);
    }
#endif
    // Free the string buffer only when the representation owns heap storage.
    ~LocalNString_u8021C59C() { if (flags >> 31) appFreeUni__FPv(data); }
    // Decode the inline or heap string length without modifying storage.
    unsigned int size() const {
        return (flags >> 31) == 0 ? (*(const unsigned char*)this & 0x7f) : length;
    }
    // Return borrowed null-terminated string data from the active representation.
    const char* c_str() const {
        return (flags >> 31) == 0 ? (const char*)this + 1 : data;
    }
};

// Translation unit 0x8021C59C-0x8021DA94 (21 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
#include <decomp/cw_bitfields.h>
extern "C" { void* __nw__FUl(unsigned long); void __dl__FPv(void*); }
/* Allocate wrapper storage through the existing engine allocator. */
inline void* operator new(new_size_t size) { return __nw__FUl(size); }
void operator delete(void*) throw();

extern "C" {
void OSLockMutex(void*);
void OSUnlockMutex(void*);
void Object_DeleteVirtual(void*, void*);
}

class InstanceLock_u8021C59C {
public:
    void* mLock;
    // Acquire the supplied OS mutex for this scope.
    explicit InstanceLock_u8021C59C(void* lock) { mLock = lock; OSLockMutex(lock); }
    // Release the OS mutex when this scope ends.
    ~InstanceLock_u8021C59C() { OSUnlockMutex(mLock); }
};

class WDeletable {
public:
    virtual ~WDeletable();
};

struct RCBase { int count; };
class RCObj : public RCBase {
public:
    virtual ~RCObj();
};

// Packed modeling chunks use numeric identifiers through the legacy pointer-typed API.
enum PetMeshChunk {
    kModelChunk = 0x6C646D, kFileNamesChunk = 0x646273,
    kMeshGroupsChunk = 0x68736D, kPrimitivesChunk = 0x6D7270,
    kMaterialChunk = 0x6C72746D, kTextureNamesChunk = 0x6E786574,
    kVertexComponentsChunk = 0x766366, kComponentDataChunk = 0x646B,
    kVertexHeadersChunk = 0x686276, kVertexDataChunk = 0x646276,
    kRigidBoundsChunk = 0x76646272, kMeshBoundsChunk = 0x6D707662,
    kTransformChunk = 0x6674, kTransformNodesChunk = 0x736E7274,
    kOmatrixChunk = 0x78746D6F, kLightDataChunk = 0x646C
};
// Serialized record legend (byte offsets, not native pointer layouts):
// model u16[0]=group count, u16[1]=first group; mesh-group 12 bytes:
// u32[0]=first primitive, u32[1]=primitive count, u32[2]=unread here.
// Primitive records are 32 bytes; u16[2] selects a material. Material +0C
// selects texture name, whose u32[1] is a byte offset into the file-name blob.
// Component records: kind byte +02, encoded type u32 +04, count u16 +0A,
// byte offset u32 +0C into the component-data blob.
// Group ownership: each group owns an array of 0x10-byte geometry records.
// Each geometry owns one allocation: vertexCount float3 positions followed by float2 UVs;
// texcoords aliases that allocation and must not be freed separately. Texture handles
// are resolved through the resource manager. The destructor below visits only group 0.
enum PetMeshRecordSizes {
    kMeshGroupRecordBytes = 12, kPrimitiveRecordBytes = 32,
    kRuntimeGroupBytes = 8, kRuntimeGeometryBytes = 16,
    kGroupFirstPrimitiveWord = 0, kGroupPrimitiveCountWord = 1,
    kPrimitiveMaterialHalfword = 2, kComponentTypeWord = 1, kComponentOffsetWord = 3
};
struct PetMeshGroup { unsigned int count; void* items; };
struct PetMeshComponentChunk {
    unsigned char header[2];
    unsigned char kind;
    unsigned char reserved;
    unsigned int dataType;
    unsigned short reservedCount;
    unsigned short elementCount;
    unsigned int dataOffset;
};
struct PetMeshBoundsFrame {
    float axisX[4], axisY[4], axisZ[4], translation[4];
};
struct PetMeshTextureName { unsigned int header; unsigned int fileNameOffset; };
struct PetMeshMaterial { char prefix[0xC]; unsigned char textureNameIndex; };
struct ResourceByteSpan { void* vtable; unsigned int byteSize; };
struct ResourceModelHandle { char prefix[0x14]; unsigned int handle; };
struct FontMarkupPool { char markupAndErrors[0x83]; char messageArchive[12]; };

extern "C" {

void* __nw__FUl(unsigned long);
void __dl__FPv(void*);
void CMsgEcbResource_ctor(void*);
void MessageContext_SetTablesAndFormat(void*, void*, void*, void*);
void CMsgDraw_Ctor(void*, void*);
void PairVector_InitReserved(void*);
void PointerPairList_RemoveByKey(void*, void*);
void MessageDrawPairList_Dtor();
void __register_global_object(void*, void*, void*);
void PointerIntVector_AppendPair(void*, void*, void*);
void appFreeUni__FPv(void*);
void* gtl_ResourceFile_GetChunkElementCount(const void*, void*, const char*);
void* gtl_ResourceFile_GetChunkElement(const void*, void*, const char*, unsigned int);
unsigned int gtl_Resource_GetDataTypeByteStride(void*);
void PetMesh_ScaleVertexChunkByKind(void*, void*, float);
unsigned int FlexibleVertexFormat_GetStride(void*);
void* FlexibleVertexFormat_GetPositionAddress(void*, void*);
void* Resource_OpenResolvedPath(void*, void*, int, int, int, int);
unsigned int __nwa__FUl(void*);
void FontAtlas_LoadFromMemory(void*, void*, void*, void*);
void NString_AssignCString(void*, const char*);
void __ct__8WOverlayFv(void*);
void WOverlay_dtor(void*, int);
void OSInitMutex(void*);
void OSLockMutex(void*);
void OSUnlockMutex(void*);
void gtl_IGenericResourceFile_GetChunkElementCount(void*, const char*);
void* gtl_IGenericResourceFile_GetChunkElement(void*, const char*, unsigned int);
void* ResourceManager_FindByNormalizedBasename(void*, void*);
extern void* g_resourceManager;
void IPetMesh_BuildGeometry(void*, void*, const void*, void*);
int IPetMesh_ScaleVertexPositions(void*, float);
int IPetMesh_ScaleVertexChunks(const void*, float);
void Vec3_Scale(void*, void*, float);
extern float kFloat_4_PetMesh;
void* memcpy(void*, const void*, unsigned long);
void* appMallocUni__FUl(unsigned long);
void gtl_IGenericModelingResource_GetResource(void*, int, unsigned short, unsigned int*);
void gtl_IGenericModelingResource_GetDescription(void*, void*);
void VertexBuffer_GetDescription(void*, void*);
void Resource_GetOffsetPtr(void*, int, void*, void*, int);
void IndexBuffer_GetDescription(void*, void*);
void IndexBuffer_GetOffsetPtr(void*, int, void*, void*, int);
void IndexBuffer_Flush(void*);
void GraphicsBuffer_Flush(void*);
void PetMesh_ExtractVertexPositionAndTexCoords(void*, void*, void*, void*);
int IPetMesh_ExpandTriangleStrip(void*, void*, void*, int, unsigned int, unsigned int, void*);
void CubeResourceStorage_dtor(void*, int);
extern FontMarkupPool fontMarkupStrings;
void __dla__FPv(void*);
extern signed char s_pointerPairListInitialized;
extern char s_messageDrawPairList[];
extern char s_messageDrawListDtorRegistration[];
extern unsigned int petMeshTriangleWindingOrder[];

extern float kFloat_0_Font;
extern float kFloat_6_2831855;
extern float s_fixedFontFullTurn;
extern float s_petMeshFullTurn;
extern char __vt__8IPetMesh[];
extern char __vt__5WFont[];
extern char __vt__10WFixedFont[];
extern float kFloat_1_FixedFont;

#pragma push
#pragma use_lmw_stmw on

// Release owned message resources, drain font-cache lists under their mutex, and optionally delete the font.
void* WFixedFont_dtor(void* fontObject, int deleting) {
    // Original 0x8021C59C: the null receiver shares the final return.
    if (fontObject != 0) {
        // Phase 1: only owning fonts unregister/delete message resources. Clones skip this.
        unsigned char borrowedResources = AsWFixedFont(fontObject)->borrowedResources;
        *(void**)fontObject = (void*)__vt__10WFixedFont;
        if (!borrowedResources) {
            if (0 == ((volatile int&)Sing_805DC650())) {
                if (!s_pointerPairListInitialized) {
                    PairVector_InitReserved(s_messageDrawPairList);
                    __register_global_object(s_messageDrawPairList, (void*)MessageDrawPairList_Dtor, s_messageDrawListDtorRegistration);
                    s_pointerPairListInitialized = 1;
                }
                ((volatile int&)Sing_805DC650()) = (int)s_messageDrawPairList;
            }
            PointerPairList_RemoveByKey((void*)((volatile int&)Sing_805DC650()), AsWFixedFont(fontObject)->messageDraw);
            void* p = AsWFixedFont(fontObject)->messageResource;
            if (p) {
                delete (WDeletable*)p;
                AsWFixedFont(fontObject)->messageResource = 0;
            }
            p = AsWFixedFont(fontObject)->messageDraw;
            if (p) {
                delete (WDeletable*)p;
                AsWFixedFont(fontObject)->messageDraw = 0;
            }
        }
        // Cache invariants: null means empty; active/recycled/allocation heads
        // each describe circular rings. Iterator becomes null when it wraps to head.
        // allocationHead owns backing blocks; recycling nodes does not free them.
        // OS mutexes are recursive: RAII scopes hold depth 1, nested explicit
        // lock/unlock pairs reach depth 2 and return to 1. Keep every pair balanced.
        char* self = (char*)fontObject;
        if (self) {
            *(void**)self = (void*)__vt__5WFont;
            char* p;
            void* nextNode;
            void* node;
            void* next;
            {
                InstanceLock_u8021C59C firstLock(self + (int)&((WFixedFont*)0)->mutex);
                AsWFixedFont(self)->activeIterator = AsWFixedFont(self)->activeHead;
                p = self + (int)&((WFixedFont*)0)->mutex;
                {
                    OSLockMutex(p);
                    node = AsWFixedFont(self)->activeIterator;
                    if (node) {
                        next = AsFontCacheNode(node)->object;
                        void* t = AsFontCacheNode(node)->next;
                        void* h = AsWFixedFont(self)->activeHead;
                        if (t == h) t = 0;
                        AsWFixedFont(self)->activeIterator = t;
                        OSUnlockMutex(p);
                    } else {
                        OSUnlockMutex(p);
                        next = 0;
                    }
                }
            }
            // Phase 2: destroy cached objects through their owner before recycling node links.
            // No outer lock is held while calling Object_DeleteVirtual; only
            // iterator advancement is locked. Do not extend the lock over callbacks.
            while (next) {
                Object_DeleteVirtual(AsWFixedFont(self)->resourceOwner, next);
                p = self + (int)&((WFixedFont*)0)->mutex;
                OSLockMutex(p);
                {
                    node = AsWFixedFont(self)->activeIterator;
                    if (node) {
                        void* h = AsWFixedFont(self)->activeHead;
                        next = AsFontCacheNode(node)->object;
                        void* t = AsFontCacheNode(node)->next;
                        if (t == h) t = 0;
                        AsWFixedFont(self)->activeIterator = t;
                        OSUnlockMutex(p);
                    } else {
                        OSUnlockMutex(p);
                        next = 0;
                    }
                }
            }
            {
                InstanceLock_u8021C59C secondLock(self + (int)&((WFixedFont*)0)->mutex);
                // Phase 3: splice active nodes into the recycled ring and clear lookup slots.
                // 0x8021C59C: splice circular cache lists through +0xC/+0x10 links.
                // Depth 1 remains held across the splice and slot reset.
                // Each link update must preserve next->previous / previous->next.
                if (AsWFixedFont(self)->activeHead && AsWFixedFont(self)->recycledHead) {
                    AsFontCacheNode(AsFontCacheNode(AsWFixedFont(self)->activeHead)->previous)->next =
                        AsFontCacheNode(AsWFixedFont(self)->recycledHead)->next;
                    AsFontCacheNode(AsFontCacheNode(AsWFixedFont(self)->recycledHead)->next)->previous =
                        AsFontCacheNode(AsWFixedFont(self)->activeHead)->previous;
                    AsFontCacheNode(AsWFixedFont(self)->recycledHead)->next = AsWFixedFont(self)->activeHead;
                    AsFontCacheNode(AsWFixedFont(self)->activeHead)->previous = AsWFixedFont(self)->recycledHead;
                    AsWFixedFont(self)->activeHead = 0;
                } else if (AsWFixedFont(self)->activeHead) {
                    p = self + (int)&((WFixedFont*)0)->mutex;
                    while (AsWFixedFont(self)->activeHead) {
                        node = AsWFixedFont(self)->activeHead;
                        OSLockMutex(p);
                        AsFontCacheNode(AsFontCacheNode(node)->next)->previous =
                            AsFontCacheNode(node)->previous;
                        AsFontCacheNode(AsFontCacheNode(node)->previous)->next =
                            AsFontCacheNode(node)->next;
                        nextNode = AsFontCacheNode(node)->next;
                        if (nextNode == node) nextNode = 0;
                        OSUnlockMutex(p);
                        {
                            void* m = AsWFixedFont(self)->activeHead;
                            void* recycledHead = AsWFixedFont(self)->recycledHead;
                            OSLockMutex(p);
                            if (!recycledHead) {
                                AsFontCacheNode(m)->previous = m;
                                AsFontCacheNode(m)->next = m;
                                OSUnlockMutex(p);
                            } else {
                                AsFontCacheNode(m)->next = recycledHead;
                                AsFontCacheNode(m)->previous = AsFontCacheNode(recycledHead)->previous;
                                AsFontCacheNode(recycledHead)->previous = m;
                                AsFontCacheNode(AsFontCacheNode(m)->previous)->next = m;
                                OSUnlockMutex(p);
                                m = recycledHead;
                            }
                            AsWFixedFont(self)->recycledHead = m;
                        }
                        AsWFixedFont(self)->activeHead = nextNode;
                    }
                }
                {
                    int i = 0;
                    int off = 0;
                    while (i < AsWFixedFont(self)->slotCount) {

                        i = (i + 1);
                        *(void**)((char*)AsWFixedFont(self)->slots + off) = 0; off = (off + 4);
                    }
                }
            }
            // Phase 4: free whole allocation blocks and the slot table, then destroy overlay base.
            if (self + 0x68) {
                InstanceLock_u8021C59C thirdLock(self + (int)&((WFixedFont*)0)->mutex);
                if (AsWFixedFont(self)->allocationHead) {
                    p = self + (int)&((WFixedFont*)0)->mutex;
                    while ((node = AsWFixedFont(self)->allocationHead) != 0) {
                        OSLockMutex(p);
                        AsFontCacheNode(AsFontCacheNode(node)->next)->previous =
                            AsFontCacheNode(node)->previous;
                        AsFontCacheNode(AsFontCacheNode(node)->previous)->next =
                            AsFontCacheNode(node)->next;
                        nextNode = AsFontCacheNode(node)->next;
                        if (nextNode == node) nextNode = 0;
                        OSUnlockMutex(p);
                        AsWFixedFont(self)->allocationHead = nextNode;
                        appFreeUni__FPv(node);
                    }
                }
                {
                    void* arr = AsWFixedFont(self)->slots;
                    if (arr) {
                        appFreeUni__FPv(arr);
                        AsWFixedFont(self)->slots = 0;
                    }
                }
            }
            WOverlay_dtor(self, 0);
        }
        if (deleting > 0) __dl__FPv(fontObject);
    }
    return fontObject;
}

#pragma pop

#pragma push
#pragma opt_common_subs off
#pragma opt_propagation off
class OverlayCall {
public:
    virtual void v0();
    virtual void Set(int);
};
// Construct a borrowed-resource font with a four-slot cache and its own mutex.
enum FontCloneDefaults {
    kFontCloneCacheSlots = 4, kFontCloneOverlayFlags = 0x2200,
    kFontRedChannelMask = 0x00FF0000, kFontGreenChannelMask = 0x0000FF00,
    kFontBlueChannelMask = 0x000000FF
};
struct CloneOverlay {
    char data[0xB8];
    // Construct a borrowed-resource font with its own four-slot cache and mutex.
    CloneOverlay(void* resource) {
        void* self = this;
        __ct__8WOverlayFv(self);
        void* vtablePtr = __vt__5WFont;
        *(void**)self = vtablePtr;
        unsigned int zero = 0;
        // Unresolved font state: +30=0, +34=1, +64=0, +A0/+A1=0.
        // These are interface defaults, not evidence of particular metric names.
        AsWFixedFont(self)->unk_30 = zero;
        (*(unsigned int*)&AsWFixedFont(self)->slots) = zero;
        OSInitMutex(AsWFixedFont(self)->mutex);
        {
        InstanceLock_u8021C59C local(AsWFixedFont(self)->mutex);
        (*(unsigned int*)&AsWFixedFont(self)->activeHead) = zero;
        int count = kFontCloneCacheSlots;
        (*(unsigned int*)&AsWFixedFont(self)->recycledHead) = 0;
        (*(unsigned int*)&AsWFixedFont(self)->allocationHead) = 0;
        // +78 receives the initial cache slot count; resizing semantics are unknown.
        AsWFixedFont(self)->unk_78 = count;
        (*(unsigned int*)&AsWFixedFont(self)->activeIterator) = 0;
        (*(unsigned int*)&AsWFixedFont(self)->slotCount) = count;
        if (count > 0) {
            void* array = appMallocUni__FUl(0x10);
            AsWFixedFont(self)->slots = array;
            AsWFixedFont(self)->slotMask = AsWFixedFont(self)->slotCount - 1;
            int off, i;
            i = 0;
            off = 0;
            while (i < AsWFixedFont(self)->slotCount) {
                ++i;
                *(unsigned int*)((char*)AsWFixedFont(self)->slots + off) = 0;
                off += 4;
            }
        } else {
            (*(unsigned int*)&AsWFixedFont(self)->slots) = 0;
        }
        }
        ((OverlayCall*)self)->Set(kFontCloneOverlayFlags);
        AsWFixedFont(self)->glyphScale = kFloat_1_FixedFont;
        unsigned int lateZero = 0;
        unsigned int redChannelMask = kFontRedChannelMask;
        unsigned int greenChannelMask = kFontGreenChannelMask;
        unsigned int blueChannelMask = kFontBlueChannelMask;
        AsWFixedFont(self)->unk_64 = lateZero;
        AsWFixedFont(self)->redChannelMask = redChannelMask;
        AsWFixedFont(self)->greenChannelMask = greenChannelMask;
        AsWFixedFont(self)->blueChannelMask = blueChannelMask;
        AsWFixedFont(self)->unk_34 = 1;
        *(void**)self = __vt__10WFixedFont;
        AsWFixedFont(self)->unk_30 = lateZero;
        AsWFixedFont(self)->unk_A1 = lateZero;
        AsWFixedFont(self)->unk_A0 = lateZero;
        (*(unsigned int*)&AsWFixedFont(self)->messageBuffer) = lateZero;
        AsWFixedFont(self)->resource = resource;
        AsWFixedFont(self)->borrowedResources = 1;
        (*(unsigned int*)&AsWFixedFont(self)->messageResource) = lateZero;
    }
};

// Clone the font while borrowing its atlas, message draw, and message-buffer resources.
// The source must outlive the clone's borrowed message/atlas storage; no retain occurs here.
void* WFixedFont_CloneFont(void* sourceFont) {
    void* self = new CloneOverlay(AsWFixedFont(sourceFont)->resource);
    unsigned int messageResource, messageDraw, messageBuffer;
    messageBuffer = (*(unsigned int*)&AsWFixedFont(sourceFont)->messageBuffer);
    messageDraw = (*(unsigned int*)&AsWFixedFont(sourceFont)->messageDraw);
    messageResource = (*(unsigned int*)&AsWFixedFont(sourceFont)->messageResource);
    (*(unsigned int*)&AsWFixedFont(self)->messageResource) = messageResource;
    (*(unsigned int*)&AsWFixedFont(self)->messageDraw) = messageDraw;
    (*(unsigned int*)&AsWFixedFont(self)->messageBuffer) = messageBuffer;
    return self;
}

#pragma pop

#pragma push
#pragma use_lmw_stmw on

// Resolve model groups, materials, and texture handles, then build each primitive geometry.
// Returns 0 with no resource, otherwise 1; requires valid model/material/texture chunks.
// Call on an empty mesh: replacing groups here does not release any previous allocation.
int IPetMesh_LoadGroups(void* meshObject, unsigned int modelIndex) {
    struct Grp { unsigned int count; char* items; };
    struct Self { unsigned char pad[0xC]; void* res; Grp* groups; int ngroups; };
    Self* self = (Self*)meshObject;
    if (!self->res) return 0;
    gtl_IGenericResourceFile_GetChunkElementCount(self->res, (const char*)kModelChunk);
    char* fileNames = (char*)gtl_IGenericResourceFile_GetChunkElement(self->res, (const char*)kFileNamesChunk, 0);
    unsigned short* modelGroups = (unsigned short*)gtl_IGenericResourceFile_GetChunkElement(self->res, (const char*)kModelChunk, modelIndex);
    int groupCount = modelGroups[0];
    self->ngroups = groupCount;
    if (!groupCount) return 1;
    // Resolve the model's contiguous group-descriptor range and allocate the group table.
    unsigned short firstGroup = modelGroups[1];
    char* groupDescriptors = (char*)gtl_IGenericResourceFile_GetChunkElement(self->res, (const char*)kMeshGroupsChunk, 0);
    self->groups = (Grp*)__nwa__FUl((void*)(kRuntimeGroupBytes * self->ngroups));
    unsigned int* groupDescriptor = (unsigned int*)(groupDescriptors + firstGroup * kMeshGroupRecordBytes);
    for (unsigned int i = 0; i < self->ngroups; ++i) { int primitiveCount = groupDescriptor[kGroupPrimitiveCountWord];
        Grp* g = &self->groups[i];

         g->count = primitiveCount;
        g->items = (char*)__nwa__FUl((void*)(primitiveCount * kRuntimeGeometryBytes));
        char* primitiveDescriptors = (char*)gtl_IGenericResourceFile_GetChunkElement(self->res, (const char*)kPrimitivesChunk, 0);
        // Each primitive resolves its texture handle before allocating expanded geometry.
        for (unsigned int j = 0; j < g->count; ++j) {
            unsigned int* it;
            it = (unsigned int*)(g->items + j * kRuntimeGeometryBytes);
            char* primitiveDescriptor = (kPrimitiveRecordBytes * groupDescriptor[kGroupFirstPrimitiveWord]) + (primitiveDescriptors + j * kPrimitiveRecordBytes);
            void* material = gtl_IGenericResourceFile_GetChunkElement(self->res, (const char*)kMaterialChunk, ((unsigned short*)primitiveDescriptor)[kPrimitiveMaterialHalfword]);
            void* textureName = gtl_IGenericResourceFile_GetChunkElement(self->res, (const char*)kTextureNamesChunk, ((PetMeshMaterial*)material)->textureNameIndex);
            void* textureResource = ResourceManager_FindByNormalizedBasename(g_resourceManager, fileNames + ((PetMeshTextureName*)textureName)->fileNameOffset);
            *it = ((ResourceModelHandle*)textureResource)->handle;
            IPetMesh_BuildGeometry(meshObject, it, primitiveDescriptor, self->res);
        }
        groupDescriptor += 3;
    }
    return 1;
}

#pragma pop

// This font interface hook requires no work.
void WFixedFont_vfunc_7() {}

// Return zero for vtable metric slot 9; the metric identity/units are unconfirmed.
float WFixedFont_PrintInside() { return kFloat_0_Font; }

// This font interface hook requires no work.
void WFixedFont_vfunc_8() {}

// Return zero for vtable metric slot 10; the metric identity/units are unconfirmed.
float WFixedFont_GetTextWidthInside() { return kFloat_0_Font; }

// Initialize the font full-turn angle cache.
void FixedFont_InitFullTurnConstant() { s_fixedFontFullTurn = kFloat_6_2831855; }

#pragma push
#pragma use_lmw_stmw on

// Scale position-bearing component streams according to each encoded data type.
int IPetMesh_ScaleVertexChunks(const void* meshObject, float scale) {
    unsigned int chunkCount = (unsigned int)gtl_ResourceFile_GetChunkElementCount(meshObject, AsIPetMesh(meshObject)->resource, (const char*)kVertexComponentsChunk);
    char* componentData = (char*)gtl_ResourceFile_GetChunkElement(meshObject, AsIPetMesh(meshObject)->resource, (const char*)kComponentDataChunk, 0);
    char* chunkData;
    for (unsigned int i = 0; i < chunkCount; ++i) {
        unsigned int* componentDescriptor = (unsigned int*)gtl_ResourceFile_GetChunkElement(meshObject, AsIPetMesh(meshObject)->resource, (const char*)kVertexComponentsChunk, i);
        int j;
        chunkData = componentData + componentDescriptor[kComponentOffsetWord];
        for (j = 0; j < ((PetMeshComponentChunk*)componentDescriptor)->elementCount; ++j) {
            switch (((PetMeshComponentChunk*)componentDescriptor)->kind) {
            // Observed scalable kind families: 0..3, 7..10 and 26..29.
            // Individual kind semantics are unconfirmed; all other streams stay unchanged.
            case 0: case 1: case 2: case 3:
            case 7: case 8: case 9: case 10:
            case 26: case 27: case 28: case 29: {
                // The resource type size is a byte stride: every caller advances bytes.
                // Do not divide this stride by eight.
                unsigned int stride = gtl_Resource_GetDataTypeByteStride((void*)componentDescriptor[kComponentTypeWord]);
                PetMesh_ScaleVertexChunkByKind((char*)chunkData + j * stride, (void*)componentDescriptor[kComponentTypeWord], scale);
                break;
            }
            }
        }
    }
    return 1;
}

#pragma pop

#pragma push
#pragma use_lmw_stmw on

// Expand a strip into alternating-winding triangles, skipping degenerate index triples.
// Returns emitted VERTICES (three per nondegenerate triangle), not triangle count.
// outputCapacity is unused. Both caller buffers must fit descriptor->triangleCount * 3
// vertices; the strip must provide two more indices than that triangle count.
// Positions stay in resource/model space; this step changes topology and winding only.
int IPetMesh_ExpandTriangleStrip(void* meshObject, void* outputPositions, void* outputTexcoords, int outputCapacity,
                unsigned int vertexBuffer, unsigned int indexBuffer, void* primitiveDescriptor) {
    unsigned int outputTriangle;
    struct Keys { int k[2][3]; };
    Keys keys;
    unsigned int vertexDescription[4];
    unsigned int indexDescription[4];
    char* vertexData;
    unsigned short* indexData;
    VertexBuffer_GetDescription((void*)vertexBuffer, vertexDescription);
    vertexData = 0;
    Resource_GetOffsetPtr((void*)vertexBuffer, 0, (void*)vertexDescription[2], &vertexData, 0);
    IndexBuffer_GetDescription((void*)indexBuffer, indexDescription);
    indexData = 0;
    IndexBuffer_GetOffsetPtr((void*)indexBuffer, 0, (void*)indexDescription[2], &indexData, 0);
    // Copy the 16-bit strip indices using the source index stride.
    unsigned int count = indexDescription[2] >> 1;
    unsigned short* stripIndices = (unsigned short*)__nwa__FUl((void*)(indexDescription[2] & ~1u));
    unsigned short* dst = stripIndices;
    unsigned int i = 0;
    while (i < count) {
        *dst++ = indexData[i++ * indexDescription[3]];
    }
    keys = *(Keys*)petMeshTriangleWindingOrder;
    int windingParity = 0;
    int emittedVertexCount = 0;
    int sourceTriangle;
    // Parity flips for every source triple, including degenerate ones; output is compacted.
    for (sourceTriangle = 0, outputTriangle = 0; sourceTriangle < AsPetMeshDescriptor(primitiveDescriptor)->triangleCount; ++sourceTriangle) {
        unsigned short* v = stripIndices + sourceTriangle;
        windingParity ^= 1;
        if (v[0] != v[1] && v[0] != v[2] && v[1] != v[2]) {
            int* windingOrder = keys.k[windingParity];
            for (int j = 0; j < 3; ++j) {
                int at = outputTriangle * 3 + windingOrder[j];
                PetMesh_ExtractVertexPositionAndTexCoords((char*)outputPositions + at * 12,
                            (char*)outputTexcoords + at * 8,
                            vertexData + AsPetMeshDescriptor(primitiveDescriptor)->vertexStride * stripIndices[sourceTriangle + j],
                            (void*)vertexDescription[3]);
                ++emittedVertexCount;
            }
            ++outputTriangle;
        }
    }
    if (stripIndices) __dla__FPv(stripIndices);
    IndexBuffer_Flush((void*)indexBuffer);
    GraphicsBuffer_Flush((void*)vertexBuffer);
    return emittedVertexCount;
}

#pragma pop

#pragma push
#pragma opt_strength_reduction off
// Scale every position vector in each flexible-format vertex block.
int IPetMesh_ScaleVertexPositions(void* meshObject, float scale) {
    unsigned int count = (unsigned int)gtl_ResourceFile_GetChunkElementCount(meshObject, AsIPetMesh(meshObject)->resource, (const char*)kVertexHeadersChunk);
    void* vertexData = gtl_ResourceFile_GetChunkElement(meshObject, AsIPetMesh(meshObject)->resource, (const char*)kVertexDataChunk, 0);
    for (unsigned int i = 0; i < count; ++i) {
        void* vertexBlock = gtl_ResourceFile_GetChunkElement(meshObject, AsIPetMesh(meshObject)->resource, (const char*)kVertexHeadersChunk, i);
        char* base = (char*)vertexData + AsPetMeshVertexBlock(vertexBlock)->dataOffset;
        unsigned int stride = FlexibleVertexFormat_GetStride(*(void**)vertexBlock);
        unsigned int vertexCount = AsPetMeshVertexBlock(vertexBlock)->byteSize / stride;
        for (unsigned int j = 0; j < vertexCount; ++j) {
            unsigned int byteOffset = j * stride;
            void* vector = FlexibleVertexFormat_GetPositionAddress(base + byteOffset, *(void**)vertexBlock);
            float position[3];
            memcpy(position, vector, 12);
            position[0] *= scale; position[1] *= scale; position[2] *= scale;
            memcpy(vector, position, 12);
        }
    }
    return 1;
}
#pragma pop

// Initialize the pet-mesh full-turn angle cache.
void PetMesh_InitFullTurnConstant() { s_petMeshFullTurn = kFloat_6_2831855; }

// Scale vertex positions, bounds axes/translations, and resource transform translations.
// Mutates retained resource storage, including any other mesh sharing it. scale is unitless;
// bounds axes receive 4 * scale, while positions/translations receive scale. Returns 0
// only for a null mesh. Transform/light chunk layouts remain format-specific below.
int IPetMesh_ScaleResource(void* meshObject, float scale) {
    if (!meshObject)
        return 0;

    // Phase 1: scale flexible-format positions and both kinds of bounds frames.
    IPetMesh_ScaleVertexPositions(meshObject, scale);
    void* entry;
    unsigned int i;
    unsigned int count = (unsigned int)gtl_ResourceFile_GetChunkElementCount(meshObject, AsIPetMesh(meshObject)->resource, (const char*)kRigidBoundsChunk);
    float axisScaleFactor = kFloat_4_PetMesh;
    for (i = 0; i < count; ++i) {
        entry = gtl_ResourceFile_GetChunkElement(meshObject, AsIPetMesh(meshObject)->resource, (const char*)kRigidBoundsChunk, i);
        Vec3_Scale(entry, entry, axisScaleFactor * scale);
        Vec3_Scale(((PetMeshBoundsFrame*)entry)->axisY, ((PetMeshBoundsFrame*)entry)->axisY, axisScaleFactor * scale);
        Vec3_Scale(((PetMeshBoundsFrame*)entry)->axisZ, ((PetMeshBoundsFrame*)entry)->axisZ, axisScaleFactor * scale);
        Vec3_Scale(((PetMeshBoundsFrame*)entry)->translation, ((PetMeshBoundsFrame*)entry)->translation, scale);
    }
    count = (unsigned int)gtl_ResourceFile_GetChunkElementCount(meshObject, AsIPetMesh(meshObject)->resource, (const char*)kMeshBoundsChunk);
    axisScaleFactor = kFloat_4_PetMesh;
    for (unsigned int i = 0; i < count; ++i) {
        void* entry = gtl_ResourceFile_GetChunkElement(meshObject, AsIPetMesh(meshObject)->resource, (const char*)kMeshBoundsChunk, i);
        Vec3_Scale(entry, entry, axisScaleFactor * scale);
        Vec3_Scale(((PetMeshBoundsFrame*)entry)->axisY, ((PetMeshBoundsFrame*)entry)->axisY, axisScaleFactor * scale);
        Vec3_Scale(((PetMeshBoundsFrame*)entry)->axisZ, ((PetMeshBoundsFrame*)entry)->axisZ, axisScaleFactor * scale);
        Vec3_Scale(((PetMeshBoundsFrame*)entry)->translation, ((PetMeshBoundsFrame*)entry)->translation, scale);
    }
    // Phase 2: scale translation slots in each serialized transform format.
    count = (unsigned int)gtl_ResourceFile_GetChunkElementCount(meshObject, AsIPetMesh(meshObject)->resource, (const char*)kTransformChunk);
    for (unsigned int i = 0; i < count; ++i) {
        float* v = (float*)gtl_ResourceFile_GetChunkElement(meshObject, AsIPetMesh(meshObject)->resource, (const char*)kTransformChunk, i);
        v[8] *= scale; v[9] *= scale; v[10] *= scale;
    }
    count = (unsigned int)gtl_ResourceFile_GetChunkElementCount(meshObject, AsIPetMesh(meshObject)->resource, (const char*)kTransformNodesChunk);
    for (unsigned int i = 0; i < count; ++i) {
        float* v = (float*)gtl_ResourceFile_GetChunkElement(meshObject, AsIPetMesh(meshObject)->resource, (const char*)kTransformNodesChunk, i);
        v[12] *= scale; v[13] *= scale; v[14] *= scale;
    }
    count = (unsigned int)gtl_ResourceFile_GetChunkElementCount(meshObject, AsIPetMesh(meshObject)->resource, (const char*)kOmatrixChunk);
    for (unsigned int i = 0; i < count; ++i) {
        float* v = (float*)gtl_ResourceFile_GetChunkElement(meshObject, AsIPetMesh(meshObject)->resource, (const char*)kOmatrixChunk, i);
        v[12] *= scale; v[13] *= scale; v[14] *= scale;
    }
    // Phase 3: scale component streams and the three light-data coordinates.
    IPetMesh_ScaleVertexChunks(meshObject, scale);
    count = (unsigned int)gtl_ResourceFile_GetChunkElementCount(meshObject, AsIPetMesh(meshObject)->resource, (const char*)kLightDataChunk);
    for (unsigned int i = 0; i < count; ++i) {
        float* v = (float*)gtl_ResourceFile_GetChunkElement(meshObject, AsIPetMesh(meshObject)->resource, (const char*)kLightDataChunk, i);
        v[13] *= scale; v[14] *= scale; v[15] *= scale;
    }
    return 1;
}

#pragma push
#pragma use_lmw_stmw on
// Expand indexed geometry and pack positions followed by UVs into one owned allocation.
// result receives ownership of positions; texcoords is an interior pointer.
// Resource vertex/index handles are retained temporarily and released after extraction.
void IPetMesh_BuildGeometry(void* meshObject, void* result, const void* descriptor, void* context) {
    unsigned int count = AsPetMeshDescriptor(descriptor)->triangleCount;
    unsigned int maxVertexCount = count * 3;
    void* trianglePositions = (void*)__nwa__FUl((void*)(maxVertexCount * 12));
    void* triangleTexcoords = (void*)__nwa__FUl((void*)(maxVertexCount * 8));
    unsigned int vertexBuffer = 0;
    gtl_IGenericModelingResource_GetResource(context, 0, AsPetMeshDescriptor(descriptor)->vertexResource, &vertexBuffer);
    unsigned char resourceDescription[0x20];
    gtl_IGenericModelingResource_GetDescription(context, resourceDescription);
    unsigned int indexBuffer = 0;
    gtl_IGenericModelingResource_GetResource(context, 1, AsPetMeshDescriptor(descriptor)->indexResource, &indexBuffer);
    AsPetMeshGeometry(result)->vertexCount = IPetMesh_ExpandTriangleStrip(meshObject, trianglePositions, triangleTexcoords, maxVertexCount, vertexBuffer, indexBuffer, (void*)descriptor);
    if (indexBuffer) {
        struct Ref { CW_BITS2( unsigned int count:16, unsigned int other:16) };
        Ref* ref = (Ref*)indexBuffer;
        if (--ref->count == 0) CubeResourceStorage_dtor(ref, 1);
        indexBuffer = 0;
    }
    // Pack only emitted vertices: contiguous XYZ data, followed by contiguous UV data.
    AsPetMeshGeometry(result)->positions = appMallocUni__FUl(AsPetMeshGeometry(result)->vertexCount * 0x14);
    memcpy(AsPetMeshGeometry(result)->positions, trianglePositions, AsPetMeshGeometry(result)->vertexCount * 12);
    AsPetMeshGeometry(result)->texcoords = (char*)AsPetMeshGeometry(result)->positions + AsPetMeshGeometry(result)->vertexCount * 12;
    memcpy(AsPetMeshGeometry(result)->texcoords, triangleTexcoords, AsPetMeshGeometry(result)->vertexCount * 8);
    if (vertexBuffer) {
        struct Ref { CW_BITS2( unsigned int count:16, unsigned int other:16) };
        Ref* ref = (Ref*)vertexBuffer;
        if (--ref->count == 0) CubeResourceStorage_dtor(ref, 1);
        vertexBuffer = 0;
    }
    __dla__FPv(trianglePositions);
    __dla__FPv(triangleTexcoords);
}

#pragma pop

#pragma push
#pragma use_lmw_stmw on

class CBRes {
public:
    virtual void v0();
    virtual void v1();
    virtual void Read(void*, unsigned int);
};

typedef NString CBStr;

// Load the requested font atlas and the shared AcLabel message archive into memory.
// Always returns 0; it is not a success flag. Atlas staging memory is freed after load;
// the message archive buffer is retained on the font for later contexts.
int WFixedFont_LoadMessageResources(void* fontObject, void* atlasPath) {
    unsigned int buffer = 0;
    void* resource;
    unsigned int length = 0;
    resource =Resource_OpenResolvedPath(AsWFixedFont(fontObject)->resourceOwner, atlasPath, 0xFFFF, 0, 0, 0);
    if (resource) {
        length = ((ResourceByteSpan*)resource)->byteSize;
        buffer = (unsigned int)__nwa__FUl((void*)length);
        ((CBRes*)resource)->Read((void*)buffer, length);
    }
    FontAtlas_LoadFromMemory(AsWFixedFont(fontObject)->messageResource, atlasPath, (void*)buffer, (void*)length);
    __dl__FPv((void*)buffer);
    CBStr archivePath;

    NString_AssignCString(&archivePath, fontMarkupStrings.messageArchive);
    const char* path = ((archivePath.flags >> 31) == 0) ? (const char*)&archivePath + 1 : (const char*)archivePath.data;
    resource = Resource_OpenResolvedPath(AsWFixedFont(fontObject)->resourceOwner, (void*)path, 0xFFFF, 0, 0, 0);
    if (resource) {
        length = ((ResourceByteSpan*)resource)->byteSize;
        buffer = (unsigned int)__nwa__FUl((void*)length);
        AsWFixedFont(fontObject)->messageBuffer = (void*)buffer;
        ((CBRes*)resource)->Read((void*)buffer, length);
// The message tables (AcLabel.eap) are big-endian on disc.
    }
    return 0;
}

#pragma pop

// Return the requested group, or null when its index is out of range.
void* IPetMesh_GetGroupByIndex(void* meshObject, unsigned short index) {
    if (index >= AsIPetMesh(meshObject)->groupCount) return 0;
    return (char*)AsIPetMesh(meshObject)->groups + index * 8;
}

// Initialize mesh ownership and retain its resource reference.
void* IPetMesh_Ctor(void* meshObject, unsigned int* resource, unsigned int meshFlags) {
    AsIPetMesh(meshObject)->referenceCount = 1;
    (*(unsigned int*)&AsIPetMesh(meshObject)->vtbl) = (unsigned int)__vt__8IPetMesh;
    AsIPetMesh(meshObject)->unk_8 = meshFlags;
    meshFlags = 0;
    (*(unsigned int*)&AsIPetMesh(meshObject)->groups) = 0;
    AsIPetMesh(meshObject)->groupCount = 0;
    (*(unsigned int*)&AsIPetMesh(meshObject)->resource) = (unsigned int)resource;
    register unsigned int value = *(volatile unsigned int*)resource;
    *(volatile unsigned int*)resource = value + 1 + (unsigned int)meshObject * 0;
    return meshObject;
}

#pragma push
#pragma use_lmw_stmw on

struct AF800Obj {
    CSerifuEcb context;
    // Construct a message context in the allocation wrapper.
    AF800Obj() { CMsgEcbResource_ctor(this); }
};

// Create a message context using this font's loaded archive and supplied formatting tables.
// *outContext is newly allocated and belongs to the caller; formatting inputs are borrowed.
int WFixedFont_CreateMsgEcbResource(void* fontObject, void** outContext, void* tables, void* formatContext) {
    AF800Obj* item = new AF800Obj();
    *outContext = item;
    MessageContext_SetTablesAndFormat(item, tables, AsWFixedFont(fontObject)->messageBuffer, formatContext);
    return 1;
}

#pragma pop

struct MessageItemCCC4 {
    char data[0x4BC];
    // Construct a message draw object from the source font atlas.
    explicit MessageItemCCC4(void* source) {
        CMsgDraw_Ctor(this, AsWFixedFont(source)->messageResource);
    }
};

// Create a message draw object and register it with the shared draw list.
// *outMessageDraw is newly allocated; the shared list registers it with drawTag.
int WFixedFont_CreateAndRegisterMsgDraw(void* fontObject, void** outMessageDraw, void* drawTag) {
    void* item = new MessageItemCCC4(fontObject);
    *outMessageDraw = item;
    if (!((volatile int&)Sing_805DC650())) {
        if (!s_pointerPairListInitialized) {
            PairVector_InitReserved(s_messageDrawPairList);
            __register_global_object(s_messageDrawPairList, (void*)MessageDrawPairList_Dtor, s_messageDrawListDtorRegistration);
            s_pointerPairListInitialized = 1;
        }
        ((volatile int&)Sing_805DC650()) = (int)s_messageDrawPairList;
    }
    PointerIntVector_AppendPair((void*)((volatile int&)Sing_805DC650()), *outMessageDraw, drawTag);
    return 1;
}

// Release the first group's geometry allocation and mesh resource, then optionally delete the mesh.
// Requires a non-null group table even when its first group has no geometry. Only group 0
// is traversed; LoadGroups can create more groups, but their geometry is not freed here.
void* IPetMesh_dtor(void* meshObject, int deleting) {
    if (meshObject) {
        AsIPetMesh(meshObject)->vtbl = __vt__8IPetMesh;
        unsigned int i = 0;
        unsigned int geometryByteOffset = 0;
        while (i < ((PetMeshGroup*)AsIPetMesh(meshObject)->groups)->count) {
            appFreeUni__FPv(AsPetMeshGeometry((char*)((PetMeshGroup*)AsIPetMesh(meshObject)->groups)->items + geometryByteOffset)->positions);
            geometryByteOffset += 0x10;
            ++i;
        }
        if (((PetMeshGroup*)AsIPetMesh(meshObject)->groups)->items) {
            __dla__FPv(((PetMeshGroup*)AsIPetMesh(meshObject)->groups)->items);
            ((PetMeshGroup*)AsIPetMesh(meshObject)->groups)->items = 0;
        }
        if (AsIPetMesh(meshObject)->groups) {
            void* data = AsIPetMesh(meshObject)->groups;
            __dla__FPv(data);
            AsIPetMesh(meshObject)->groups = 0;
        }
        RCObj* ref = (*(RCObj**)&AsIPetMesh(meshObject)->resource);
        if (ref && --ref->count == 0) {
            delete ref;
        }
        if (deleting > 0) __dl__FPv(meshObject);
    }
    return meshObject;
}

// Return the mesh group count truncated to the interface's 16-bit result.
unsigned short IPetMesh_GetGroupCount(void* meshObject) { unsigned int value = AsIPetMesh(meshObject)->groupCount; return (unsigned short)value; }

// Compile-time guards for private ABI views used above.
#define LOCAL_FIELD(t, f, o) typedef char local_##t##_##f[((int)&((t*)0)->f == (o)) ? 1 : -1]
LOCAL_FIELD(PetMeshComponentChunk, kind, 2);
LOCAL_FIELD(PetMeshComponentChunk, elementCount, 0xA);
LOCAL_FIELD(PetMeshComponentChunk, dataOffset, 0xC);
LOCAL_FIELD(PetMeshBoundsFrame, translation, 0x30);
LOCAL_FIELD(FontMarkupPool, messageArchive, 0x83);
#undef LOCAL_FIELD

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462204", _ctors_u8021C59C_0, FixedFont_InitFullTurnConstant);
STATIC_INIT_ENTRY(".CRT$XCU80462204", _ctors_u8021C59C_1, PetMesh_InitFullTurnConstant);
#pragma pop
