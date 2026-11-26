// WGeneric3DR renderer core: construction and initialization, render-entry storage, resource
// cleanup, and the render submission sequence (activate, reset, complete). It also holds the
// base WGeneric3D hooks that the renderer uses.

#include <decomp/static_init.h>
typedef unsigned long new_size_t;

#include "classes/WGeneric3DR.h"
#include "classes/WGeneric3DRFields.h"
#include "classes/WGeneric3D.h"
#include <decomp/cw_bitfields.h>
extern "C" {
void* __nw__FUl(unsigned long);
void __dl__FPv(void*);
}
// Route C++ allocations through the game's allocator.
inline void* operator new(new_size_t size) {
    return __nw__FUl(size);
}
void operator delete(void*) throw();

enum {
    kRenderEntryBytes = 0x2C,
    kRendererResourceBlockBytes = 0x16000,
    kDefaultDisplayWidth = 640,
    kDefaultDisplayHeight = 480,
    kRequiredWindowFormatBit = 0x80000,
    kDefaultTextureDimension = 1024,
    kRenderStatePairCount = 20,
    kTextureStageDefaultCount = 22,
    kCachedTextureStageCount = 2,
    kCachedStatesPerStage = 6,
    kSourceBlendRenderState = 13,
    kDestinationBlendRenderState = 14,
    kRenderCacheInvalid = -1,
    kDefaultPackedDrawFlags = 0x1004,
    kRenderSubmissionIdle = 0,
    kRenderSubmissionPending = 1,
    kRenderSubmissionActive = 2
};
enum RendererVertexFormat {
    kVertexLegacyDefault = 0,
    kVertexXYZColor = 0xF001,
    kVertexXYZColorAlt = 0xF005,
    kVertexXYZUv = 0x110001,
    kVertexXYZUvAlt = 0x110005,
    kVertexXYZColorUv = 0x11F001,
    kVertexXYZColorUvAlt = 0x11F005,
    kVertexXYZNormalUv = 0x110401,
    kVertexXYZNormalColorUv = 0x11F401,
    kVertexLayout12F005 = 0x12F005,
    kVertexLayout120401 = 0x120401,
    kVertexLayout510521 = 0x510521,
    kVertexLayout510531 = 0x510531,
    kVertexLayout510541 = 0x510541
};
// Format keys below are engine-packed IDs; return values are vertex sizes in bytes.
static const unsigned int kVertexFormatKeyMask = 0x7FFFFFFF;

// Properties exposed by WGeneric3DR::GetProperty (wgeneric3dr_immediate_renderer.cpp). Numbered names
// identify the public query IDs; their physical meaning/units are still unknown.
// property1Flags is tested for BOTH 0x80000 and 0x10000. Property 4 returns
// texture dimensions, property 3 returns windowFormatFlags bit 29.
struct RendererPropertyStorage {
    char prefix[0x486C];
    int unresolvedModeWord; // Written as 1 here; no reader established.
    char pad70[0x18];
    unsigned int property1Flags;
    char pad8C[0x38];
    int textureWidth, textureHeight;
    char padCC[0x34];
    int property5, property6;
    char pad08[0xC];
    int property7, property8;
    float property9;
    char pad20[0x10];
    int property10;
    char pad34[4];
    int property11;
};
typedef char RendererPropertyStorage_5[((int)&((RendererPropertyStorage*)0)->property5 == 0x4900) ? 1 : -1];
typedef char RendererPropertyStorage_11[((int)&((RendererPropertyStorage*)0)->property11 == 0x4938) ? 1 : -1];
// Borrow query-property storage; setters below neither allocate nor transfer it.
static inline RendererPropertyStorage* RendererProperties(void* renderer) {
    return (RendererPropertyStorage*)renderer;
}

// Eleven words per entry; unresolved words retain their relative positions.
struct RendererEntryStorage {
    void* helper;
    void* texture;
    int unresolved08;
    void* retainedResource;
    int unresolved10, unresolved14;
    int externallyOwned;
    int unresolved1C, unresolved20, unresolved24, unresolved28;
};
typedef char RendererEntryStorage_size[(sizeof(RendererEntryStorage) == kRenderEntryBytes) ? 1 : -1];
struct RendererEntriesView {
    char rendererPrefix[0x49A4];
    RendererEntryStorage entries[2048];
};
struct RendererScalarDefaults {
    char prefix[0x1A9F0];
    float values[8]; // No getter or consumer identifies these eight parameters yet.
};

extern "C" {

extern char __vt__11WGeneric3DR[0x10000];
extern unsigned int strlen(const char*);
extern char* strcpy(char*, const char*);

extern void WGeneric3D_ctor(void*, void*, void*);
extern void* __nwa__FUl(unsigned int);
extern void strcat(void*, const char*);
extern char gtlWindowDiagnosticStrings[];
extern void* memset(void*, int, unsigned int);
extern void WGeneric3D_dtor(void*, int);
extern float kFloat_1_Generic3DR;
} // extern "C"

// Initialize renderer resources, per-entry state, the resource name and default parameter values.
// self is caller-allocated; returns self. name is borrowed and copied into an
// owned prefixed resourceName; arg is retained as an opaque factory context.
// Layout map (WGeneric3DRFields supplies offsets, not a full object model):
//  0x49A4..0x1A9A3: 2048 entries, 0x2C bytes each. First entry is spelled out,
//  remaining 2047 are the q loop. Entry +0 is the virtually destroyed helper,
//  +0xC the reference-counted entry resource, +0x18 its external-ownership flag.
//  Entry words +08/+10/+14/+1C/+20/+24/+28 have no proven consumer here;
//  RendererEntryStorage records these positions without assigning resource roles.
//  0x1A9A8: owned name; 0x1A9AC/B0: factory index/context.
//  0x1A9F0..0x1AA0C: RendererScalarDefaults::values[0..7], each 1.
//  No recovered getter/consumer identifies these parameters; they are unrelated
//  to GetProperty IDs 1..11. Physical order is explicit for later identification.
void* WGeneric3DR::Init(const char* name, void* arg) {
    // Phase 1: construct the base and clear every entry before acquiring resources.
    WGeneric3D_ctor(this, (void*)name, arg);
    char* p = (char*)this;
    char* qEnd = (char*)(p + 0x1a9a4);
    *(void**)p = __vt__11WGeneric3DR;
    ((RendererEntriesView*)p)->entries[0].helper = 0;
    AsWGeneric3DRFields(p)->textureResource = 0;
    AsWGeneric3DRFields(p)->unk_49AC = 0;
    *(int*)&AsWGeneric3DRFields(p)->entryResource = 0;
    AsWGeneric3DRFields(p)->unk_49B4 = 0;
    AsWGeneric3DRFields(p)->unk_49B8 = 0;
    AsWGeneric3DRFields(p)->entryResourceExternallyOwned = 0;
    AsWGeneric3DRFields(p)->unk_49C0 = 0;
    AsWGeneric3DRFields(p)->unk_49C4 = 0;
    AsWGeneric3DRFields(p)->unk_49C8 = 0;
    AsWGeneric3DRFields(p)->unk_49CC = 0;
    for (char* q = (char*)(p + 0x49d0); q <= qEnd - (kRenderEntryBytes - 1); q += kRenderEntryBytes) {
        ((RendererEntryStorage*)q)->helper = 0;
        ((RendererEntryStorage*)q)->texture = 0;
        ((RendererEntryStorage*)q)->unresolved08 = 0;
        ((RendererEntryStorage*)q)->retainedResource = 0;
        ((RendererEntryStorage*)q)->unresolved10 = 0;
        ((RendererEntryStorage*)q)->unresolved14 = 0;
        ((RendererEntryStorage*)q)->externallyOwned = 0;
        ((RendererEntryStorage*)q)->unresolved1C = 0;
        ((RendererEntryStorage*)q)->unresolved20 = 0;
        ((RendererEntryStorage*)q)->unresolved24 = 0;
        ((RendererEntryStorage*)q)->unresolved28 = 0;
    }
    AsWGeneric3DRFields(p)->unk_1A9A4 = 0; /* original's second fill loop (13 x 0x2c from 0x1a770) runs to 0x1a9ac */
    *(int*)&AsWGeneric3DRFields(p)->resourceName = 0;
    // Phase 2: allocate the copied name; unwind the base if allocation throws.
    if (name) {
        unsigned int len = strlen((const char*)name);
        unsigned int t = len + 6;
        unsigned int n = t * 2 - t;
        try {
            *(void**)(p + 0x1a9a8) = __nwa__FUl(n);
        } catch (...) {
            WGeneric3D_dtor(this, -1);
            throw;
        }
        strcpy(*(char**)(p + 0x1a9a8), gtlWindowDiagnosticStrings);
        strcat(*(void**)(p + 0x1a9a8), (const char*)name);
    }
    // Phase 3: reset resource/draw defaults. The block memset intentionally
    // clears the same entries again; preserve its range and initialization order.
    AsWGeneric3DRFields(p)->factoryIndex = 0;
    AsWGeneric3DRFields(p)->initialized = 0;
    AsWGeneric3DRFields(p)->factoryContext = (int)arg;
    *(int*)(p + 0x18) = 0;
    *(int*)(p + 0x14) = 0;
    AsWGeneric3DRFields(p)->retainedResource3 = 0;
    memset(p + 0x49a4, 0, kRendererResourceBlockBytes);
    for (int i = 0; i < 5; ++i)
        *(int*)(p + 0x1a9ec - i * 4) = 0;
    memset(p + 0x1a9f0, 0, 0x44);
    float one = kFloat_1_Generic3DR;
    ((RendererScalarDefaults*)p)->values[6] = one;
    ((RendererScalarDefaults*)p)->values[2] = one;
    ((RendererScalarDefaults*)p)->values[5] = one;
    ((RendererScalarDefaults*)p)->values[1] = one;
    ((RendererScalarDefaults*)p)->values[4] = one;
    ((RendererScalarDefaults*)p)->values[0] = one;
    ((RendererScalarDefaults*)p)->values[7] = one;
    ((RendererScalarDefaults*)p)->values[3] = one;
    AsWGeneric3DRFields(p)->unk_1AA34 = -1;
    AsWGeneric3DRFields(p)->unk_1A9D4 = -1;
    AsWGeneric3DRFields(p)->unk_1AA38 = 0;
    AsWGeneric3DRFields(p)->unk_1AA3C = 0;
    AsWGeneric3DRFields(p)->unk_1AA40 = 1;
    AsWGeneric3DRFields(p)->unk_1AA44 = 2;
    AsWGeneric3DRFields(p)->unk_1AA48 = 0;
    AsWGeneric3DRFields(p)->retainedResource1 = 0;
    AsWGeneric3DRFields(p)->retainedResource0 = 0;
    AsWGeneric3DRFields(p)->retainedResource2 = 0;
    return this;
}

// Return the current render-entry count.
unsigned int WGeneric3D::GetRenderEntryCount() {
    return renderEntryCount;
}

extern "C" {

extern float kFloat_6_2831855;
extern float s_twoPi_80086CE0;
// Initialize the generic-renderer angle constant to one full turn in radians.
void Generic3DR_InitAngleConstant() {
    s_twoPi_80086CE0 = kFloat_6_2831855;
}

} // extern "C"

// Base renderer capability hook: unsupported.
int WGeneric3D::vfunc_26() {
    return 0;
}
// Base renderer capability hook: unsupported.
int WGeneric3D::vfunc_27() {
    return 0;
}

extern "C" {

void WGeneric3D_vfunc_40() {}                   // Optional renderer hook; base implementation has no work.
void WGeneric3D_NoOpTextureStorageCreation() {} // Optional renderer hook; base implementation has no work.
void WGeneric3D_NoOpTextureDataUpdate() {}      // Optional renderer hook; base implementation has no work.
void WGeneric3D_vfunc_25() {}                   // Optional renderer hook; base implementation has no work.
void WGeneric3D_vfunc_24() {}                   // Optional renderer hook; base implementation has no work.
void WGeneric3D_vfunc_23() {}                   // Optional renderer hook; base implementation has no work.

// Map an engine vertex-format key to its byte stride, ignoring its metadata flag.
// arg0 is unused. Established layouts (w3d_sprite_and_billboard.cpp extraction and gtl_texture_vertex_formats.cpp):
// XYZ = 3 floats/12 bytes; color = one packed word/4; normal = 3 floats/12;
// UV = two floats/8. Thus XYZ+color=16, XYZ+UV=20, XYZ+color+UV=24,
// XYZ+normal+UV=32, XYZ+normal+color+UV=36 bytes. 001/005 spellings
// share these storage sizes; their behavioral distinction is not decoded here.
// For larger opaque layouts the first UV is at float lane 4 (12F005),
// 6 (120401), 8 (510521), 9 (510531), or 10 (510541); do not infer
// the intervening fields from stride alone. Key 0 is a legacy 20-byte fallback.
int VertexFormat_GetStride(void* arg0, unsigned int type) {
    switch (type & kVertexFormatKeyMask) {
    case kVertexLegacyDefault:
        return 0x14;
    case kVertexXYZColorAlt:
        return 0x10;
    case kVertexXYZColorUvAlt:
        return 0x18;
    case kVertexXYZUvAlt:
        return 0x14;
    case kVertexLayout12F005:
        return 0x20;
    case kVertexXYZColor:
        return 0x10;
    case kVertexXYZColorUv:
        return 0x18;
    case kVertexXYZUv:
        return 0x14;
    case kVertexXYZNormalUv:
        return 0x20;
    case kVertexXYZNormalColorUv:
        return 0x24;
    case kVertexLayout510541:
        return 0x30;
    case kVertexLayout120401:
        return 0x28;
    case kVertexLayout510521:
        return 0x28;
    case kVertexLayout510531:
        return 0x2c;
    default:
        return 0;
    }
}

} // extern "C"

// Base renderer readiness hook: ready.
int WGeneric3D::vfunc_21() {
    return 1;
}

extern "C" {

extern void WGeneric3D_ReleaseItemsAndBuffer(void*);
extern void WGeneric3D_dtor(void*, int);
extern void __dl__FPv(void*);
extern int g_generic3DRInitializationErrorText;
extern char __vt__11WGeneric3DR[0x10000];
} // extern "C"

// Release queued items and renderer resources, then destroy the base renderer.
void* WGeneric3DR::Destroy(int shouldDelete) {
    if (this != 0) {
        *(void**)this = (void*)__vt__11WGeneric3DR;
        WGeneric3D_ReleaseItemsAndBuffer(this);
        ReleaseResources();
        if (g_generic3DRInitializationErrorText != 0)
            g_generic3DRInitializationErrorText = 0;
        WGeneric3D_dtor(this, 0);
        if (shouldDelete > 0)
            __dl__FPv(this);
    }
    return this;
}
extern "C" {

struct RendererFactoryInterface {
#define OBJ877C8_SLOT(n) virtual void slot##n() = 0
    OBJ877C8_SLOT(0);
    OBJ877C8_SLOT(1);
    OBJ877C8_SLOT(2);
    OBJ877C8_SLOT(3);
    OBJ877C8_SLOT(4);
    OBJ877C8_SLOT(5);
    OBJ877C8_SLOT(6);
    OBJ877C8_SLOT(7);
    OBJ877C8_SLOT(8);
    OBJ877C8_SLOT(9);
    OBJ877C8_SLOT(10);
    OBJ877C8_SLOT(11);
    OBJ877C8_SLOT(12);
    OBJ877C8_SLOT(13);
    OBJ877C8_SLOT(14);
    OBJ877C8_SLOT(15);
    OBJ877C8_SLOT(16);
    OBJ877C8_SLOT(17);
    OBJ877C8_SLOT(18);
    OBJ877C8_SLOT(19);
    OBJ877C8_SLOT(20);
    OBJ877C8_SLOT(21);
    OBJ877C8_SLOT(22);
    OBJ877C8_SLOT(23);
    OBJ877C8_SLOT(24);
    OBJ877C8_SLOT(25);
    OBJ877C8_SLOT(26);
    OBJ877C8_SLOT(27);
    OBJ877C8_SLOT(28);
    OBJ877C8_SLOT(29);
    OBJ877C8_SLOT(30);
    OBJ877C8_SLOT(31);
    OBJ877C8_SLOT(32);
    OBJ877C8_SLOT(33);
    OBJ877C8_SLOT(34);
    OBJ877C8_SLOT(35);
    OBJ877C8_SLOT(36);
    OBJ877C8_SLOT(37);
    OBJ877C8_SLOT(38);
    OBJ877C8_SLOT(39);
    OBJ877C8_SLOT(40);
    OBJ877C8_SLOT(41);
    virtual void* method(void*, int) = 0;
#undef OBJ877C8_SLOT
};
struct RendererFactoryCleanup {
    virtual void cleanup(int);
};
} // extern "C"

// Create a renderer through the factory; destroy it and return null if initialization fails.
// The factory returns an owned renderer pointer. The fourth initialization word
// is forwarded by this call site but unused by this implementation of Initialize.
void* WGeneric3DR::CreateInitializedRenderer(const char* name, void* presentationTarget, void* factoryArgument) {
    int index = AsWGeneric3DRFields(this)->factoryContext;
    if (index >= 0) {
        void* renderer = ((RendererFactoryInterface*)this)->method(AsWGeneric3DRFields(this)->resourceName, index);
        if (((WGeneric3DR*)renderer)->Initialize(name, presentationTarget, factoryArgument) == 0)
            return renderer;
        ((WGeneric3DR*)renderer)->ReleaseResources();
        if (renderer != 0) {
            if (renderer != 0) {
                ((RendererFactoryCleanup*)renderer)->cleanup(1);
            }
        }
    }
    return 0;
}
extern "C" {

extern void CubeResourceStorage_dtor(void*, int);
extern void WGeneric3D_DestroyTexture(void*, int);
struct Obj87908 {
    char rendererPrefix[0x49A4];
    void* value49A4;
    char padding49A8[8];
    void* value49B0;
    char padding49B4[8];
    int value49BC;
};
struct RenderEntryOwnedObject {
    virtual void slot0();
    virtual void destroy();
};
} // extern "C"

#pragma opt_common_subs off
// Release one entry resource unless externally owned, destroy its helper and clear the base entry.
void WGeneric3DR::ReleaseRenderEntry(int index) {
    char* base;
    unsigned int offset;
    offset = index * kRenderEntryBytes;
    {
        base = (char*)this + offset;
        // +0x49BC marks external ownership; +0x49B0 is the retained graphics resource.
        if (AsWGeneric3DRFields(base)->entryResourceExternallyOwned == 0) {
            void* p = AsWGeneric3DRFields(base)->entryResource;
            if (p) {
                struct RefCount {
                    CW_BITS2(unsigned int count : 16, unsigned int flags : 16)
                };
                RefCount* ref = (RefCount*)p;
                --ref->count;
                if (ref->count == 0)
                    CubeResourceStorage_dtor(p, 1);
                AsWGeneric3DRFields(base)->entryResource = 0;
            }
        }
    }
    void* p = ((RendererEntriesView*)((char*)this + offset))->entries[0].helper;
    if (p) {
        ((RenderEntryOwnedObject*)p)->destroy();
        ((RendererEntriesView*)((char*)this + offset))->entries[0].helper = 0;
    }
    WGeneric3D_DestroyTexture(this, index);
}
extern "C" {

#pragma opt_common_subs reset

extern const float kFloat_0_Generic3DR;
extern int strstr(const char*, const char*);
extern int Engine_PresentFrame(int, int, void*, int);
extern void WGeneric3D_AllocateVertexBuffer(void*);
extern char gtlWindowDiagnosticStrings[];
extern int g_generic3DRInitializationErrorText;
// Install the query-property defaults before checking window compatibility.
static inline void InstallRendererPropertyDefaults(char* rendererBytes, int defaultTextureDimension,
                                                   int requiredWindowFormat, int defaultProperty5, int enabledWord) {
    // Phase 1: install the property-query defaults before validating the name.
    *(int*)(rendererBytes + 4) = 0;
    RendererProperties(rendererBytes)->unresolvedModeWord = enabledWord;
    AsWGeneric3DRFields(rendererBytes)->windowFormatFlags = requiredWindowFormat;
    RendererProperties(rendererBytes)->property1Flags = 0;
    AsWGeneric3DRFields(rendererBytes)->defaultTextureWidth = defaultTextureDimension;
    AsWGeneric3DRFields(rendererBytes)->defaultTextureHeight = defaultTextureDimension;
    RendererProperties(rendererBytes)->property5 = defaultProperty5;
    RendererProperties(rendererBytes)->property7 = 0;
    RendererProperties(rendererBytes)->property8 = 0;
    RendererProperties(rendererBytes)->property6 = 0;
    RendererProperties(rendererBytes)->property9 = kFloat_0_Generic3DR;
    RendererProperties(rendererBytes)->property10 = 0;
    RendererProperties(rendererBytes)->property11 = 0;
    ((WGeneric3D*)rendererBytes)->separateTexturePassesEnabled = 0;
}

} // extern "C"

// Install queried property defaults, validate window compatibility and allocate the vertex buffer.
// name and presentationTarget are borrowed; the fourth factory argument is unused.
// Return 0 on
// success, 3 for an incompatible matching window name (with diagnostic text).
// GetProperty mappings are beside RendererPropertyStorage; unnamed physical
// units are intentionally left unresolved.
int WGeneric3DR::Initialize(const char* name, void* presentationTarget, void* unusedFactoryArgument) {
    char* p = (char*)this;
    int defaultTextureDimension = kDefaultTextureDimension;
    int requiredWindowFormat = kRequiredWindowFormatBit;
    int defaultProperty5 = 4;
    int one = 1;
    InstallRendererPropertyDefaults(p, defaultTextureDimension, requiredWindowFormat, defaultProperty5, one);
    // Phase 2: window-name matches require the format bit and default 640x480.
    int ok = strstr(name, gtlWindowDiagnosticStrings + 6);
    if (ok != 0) {
        if ((AsWGeneric3DRFields(p)->windowFormatFlags & requiredWindowFormat) == 0) {
            g_generic3DRInitializationErrorText = (int)(gtlWindowDiagnosticStrings + 0xd);
            return 3;
        }
        char* q = p + 0x20000;
        *(int*)(p + 0x14) = kDefaultDisplayWidth;
        *(int*)(p + 0x18) = kDefaultDisplayHeight;
        *(char*)(q - 0x565c) = (char)one;
    }
    // Phase 3: bind presentation target, install backend states and allocate vertices.
    AsWGeneric3DRFields(p)->presentationTarget = presentationTarget;
    AsWGeneric3DRFields(p)->unk_1A9B8 = -1;
    *(int*)(p + 0x4850) = 0;
    AsWGeneric3DRFields(p)->unk_1A9BC = 0x20;
    AsWGeneric3DRFields(p)->initialized = one;
    InitializeRenderStates();
    Engine_PresentFrame(0, 0, *(void**)(p + 0x1a9b4), 0);
    WGeneric3D_AllocateVertexBuffer(this);
    return 0;
}
extern "C" {

extern unsigned int s_generic3DRDefaultRenderStates[];
extern void Engine_SetRenderStateList(void*, int);
extern void Engine_GetTextureStageState(void*, void*, void**);
extern void Engine_SetTextureStageState(void*, void*, unsigned int);
extern void Engine_GetRenderState(int, void*);
extern int Engine_PresentFrame(int, int, void*, int);
extern unsigned int s_generic3DRDefaultTextureStageStates[];
extern unsigned int s_textureStageStateIds[];
struct DefaultRenderStatePairs {
    unsigned int words[40];
};
struct RendererStateCacheView {
#define INIT_SLOT(n) virtual void slot##n() = 0
    INIT_SLOT(0);
    INIT_SLOT(1);
    INIT_SLOT(2);
    INIT_SLOT(3);
    INIT_SLOT(4);
    INIT_SLOT(5);
    INIT_SLOT(6);
    INIT_SLOT(7);
    INIT_SLOT(8);
    INIT_SLOT(9);
    INIT_SLOT(10);
    INIT_SLOT(11);
    INIT_SLOT(12);
    INIT_SLOT(13);
    INIT_SLOT(14);
    INIT_SLOT(15);
    INIT_SLOT(16);
    INIT_SLOT(17);
    INIT_SLOT(18);
    INIT_SLOT(19);
    INIT_SLOT(20);
    INIT_SLOT(21);
    INIT_SLOT(22);
    INIT_SLOT(23);
    INIT_SLOT(24);
    INIT_SLOT(25);
    INIT_SLOT(26);
    INIT_SLOT(27);
    INIT_SLOT(28);
    INIT_SLOT(29);
    INIT_SLOT(30);
    INIT_SLOT(31);
    virtual void invoke() = 0;
#undef INIT_SLOT
    char pad[0x4964 - 4];
    unsigned int values[2][6];
};
struct RendererBlendStateCache {
    char rendererPrefix[0x4994];
    unsigned int sourceBlend;
    unsigned int destinationBlend;
};
// Borrow a cached backend word. The row displacement remains explicit to preserve
// CodeWarrior's stage-index arithmetic; callers always pass the real renderer.
static inline unsigned int* RendererTextureCacheSlot(void* renderer, int stage, int slot) {
    enum {
        kCacheStageRowBytes = kCachedStatesPerStage * (int)sizeof(unsigned int)
    };
    return &((RendererStateCacheView*)((char*)renderer + stage * kCacheStageRowBytes))->values[0][slot];
}
} // extern "C"

// Apply default render/texture states, cache the affected values and reset draw state.
// Default table contracts: render states = 20 (state,value) word pairs;
// texture defaults = 22 (stage,state,value) word triples. Engine_Get/Set use
// pointer-shaped parameters for integer stage/state/value words; no pointers
// from those casts are dereferenced here. Cache layout is values[2][6] at
// +0x4964: stage = k/6, state slot = k%6, row stride = 0x18 bytes.
// s_textureStageStateIds maps each slot to the backend state ID, not an address.
void WGeneric3DR::InitializeRenderStates() {
    // Phase 1: submit render defaults, then avoid redundant texture-state writes.
    DefaultRenderStatePairs local = *(DefaultRenderStatePairs*)s_generic3DRDefaultRenderStates;
    Engine_SetRenderStateList(&local, kRenderStatePairCount);
    int k;
    unsigned int* p = (unsigned int*)s_generic3DRDefaultTextureStageStates;
    for (k = 0; (unsigned int)k < kTextureStageDefaultCount; ++k) {
        unsigned int* d;
        Engine_GetTextureStageState((void*)p[0], (void*)p[1], (void**)&d);
        if (d != (unsigned int*)p[2])
            Engine_SetTextureStageState((void*)p[0], (void*)p[1], p[2]);
        p += 3;
    }
    // Phase 2: snapshot blend values and the two-stage/six-state texture cache.
    Engine_GetRenderState(kSourceBlendRenderState, &((RendererBlendStateCache*)this)->sourceBlend);
    Engine_GetRenderState(kDestinationBlendRenderState, &((RendererBlendStateCache*)this)->destinationBlend);
    for (k = 0; k < kCachedTextureStageCount * kCachedStatesPerStage; ++k) {
        int idx = k - k / kCachedStatesPerStage * kCachedStatesPerStage;
        Engine_GetTextureStageState((void*)(k / kCachedStatesPerStage), (void*)s_textureStageStateIds[idx],
                                    (void**)RendererTextureCacheSlot(this, k / kCachedStatesPerStage, idx));
    }
    // Phase 3: invalidate per-draw bindings through the renderer's reset hook.
    ((RendererStateCacheView*)this)->invoke();
}
extern "C" {

extern void CubeResourceStorage_dtor(void*, int);
extern void __dla__FPv(void*);
struct RendererResourceRef {
    CW_BITS2(unsigned int count : 16, unsigned int flags : 16)
};
struct RendererResourceOwner {
    char pad0[0x4868];
    int state;
    char pad486c[0x4954 - 0x486c];
    RendererResourceRef* ref4954;
    RendererResourceRef* ref4958;
    RendererResourceRef* ref495c;
    RendererResourceRef* ref4960;
    char pad4964[0x1a9a8 - 0x4964];
    void* array1;
    void* array2;
    int index;
};
} // extern "C"

// Release four retained graphics resources and both owned arrays; mark the renderer uninitialized.
void WGeneric3DR::ReleaseResources() {
    RendererResourceOwner* p = (RendererResourceOwner*)this;
    {
        RendererResourceRef* q = p->ref4960;
        if (q != 0) {
            if (--q->count == 0)
                CubeResourceStorage_dtor(q, 1);
            p->ref4960 = 0;
        }
    }
    {
        RendererResourceRef* q = p->ref4958;
        if (q != 0) {
            if (--q->count == 0)
                CubeResourceStorage_dtor(q, 1);
            p->ref4958 = 0;
        }
    }
    {
        RendererResourceRef* q = p->ref4954;
        if (q != 0) {
            if (--q->count == 0)
                CubeResourceStorage_dtor(q, 1);
            p->ref4954 = 0;
        }
    }
    {
        RendererResourceRef* q = p->ref495c;
        if (q != 0) {
            if (--q->count == 0)
                CubeResourceStorage_dtor(q, 1);
            p->ref495c = 0;
        }
    }
    if (p->array1 != 0) {
        __dla__FPv(p->array1);
        p->array1 = 0;
    }
    if (p->array2 != 0) {
        __dla__FPv(p->array2);
        p->array2 = 0;
    }
    p->state = 0;
    p->index = -1;
}
extern "C" {

extern float kFloat_1_Generic3DR;
// Reset fields in the renderer tail. Packed defaults are opaque engine flags,
// not floating-point values. Bytes at +1A9C1 and +1A9D1 share their parent words.
struct RendererPassResetView {
    char rendererPrefix[0x1A9C1];
    unsigned char unresolvedDrawFlag;
    char padC2[2];
    unsigned int packedDrawDefaults[2];
    unsigned int unresolvedDrawWord;
    union {
        unsigned int word;
        struct {
            unsigned char firstByte, flagByte;
        } bytes;
    } drawWordAndFlag;
    char padD4[4];
    unsigned char useStage0ColorOperationZero; // +1A9D8: selects op 0 vs 0x11 for vertex key 0x110001
    unsigned char unresolvedPassFlag;
    char padDA[0x72];
    float unresolvedPassRange[2];
};
typedef char RendererPassReset_range[((int)&((RendererPassResetView*)0)->unresolvedPassRange == 0x1AA4C) ? 1 : -1];
typedef char RendererPassReset_packed[((int)&((RendererPassResetView*)0)->packedDrawDefaults == 0x1A9C4) ? 1 : -1];
} // extern "C"

#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_strength_reduction off
// Invalidate cached bindings and reset fog, draw flags and per-pass parameters.
void WGeneric3DR::ResetRenderState() {
    char* p = (char*)this;
    float f0 = kFloat_1_Generic3DR;
    float f1 = kFloat_0_Generic3DR;
    RendererPassResetView* pass = (RendererPassResetView*)p;
    int val;
    int neg1;
    int zero = 0;
    neg1 = -1;
    val = kDefaultPackedDrawFlags;
    // Phase 1: invalidate cached bindings and clear base fog/color state.
    AsWGeneric3DRFields(p)->cachedDrawSettings = zero;
    AsWGeneric3DRFields(p)->boundTextureIndices = zero;
    AsWGeneric3DRFields(p)->cachedRenderFlags = zero;
    AsWGeneric3DRFields(p)->cachedBlendMode = neg1;
    AsWGeneric3DRFields(p)->cachedVertexFormat = neg1;
    AsWGeneric3DRFields(p)->drawMode = neg1;
    ((WGeneric3D*)p)->fogEnabled = (unsigned char)zero;
    ((WGeneric3D*)p)->colorRampRgb = zero;
    // Phase 2: restore tail defaults; 0x1004 is an engine-packed draw flag word
    // also ORed into immediate-draw flags by Execute. Individual bits unresolved.
    pass->unresolvedPassRange[0] = f1;
    pass->unresolvedPassRange[1] = f0;
    pass->packedDrawDefaults[0] = val;
    pass->packedDrawDefaults[1] = val;
    pass->unresolvedDrawWord = zero;
    pass->drawWordAndFlag.word = zero;
    pass->unresolvedDrawFlag = (unsigned char)zero;
    pass->useStage0ColorOperationZero = (unsigned char)zero;
    pass->unresolvedPassFlag = (unsigned char)zero;
}
extern "C" {

#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset

extern void* __nw__FUl(unsigned long);
extern void __dl__FPv(void*);
struct AllocatedRenderer {
    char data[0x1AA54];
    // Construct the renderer in the factory allocation using borrowed name/context.
    inline AllocatedRenderer(void* name, void* arg) {
        ((WGeneric3DR*)this)->Init((const char*)name, arg);
    }
};
} // extern "C"

// Allocate and construct a renderer for the requested resource name and argument.
void* WGeneric3DR::CreateRenderer(void* arg0, void* name, void* arg2) {
    AllocatedRenderer* result = new AllocatedRenderer(name, arg2);
    return (void*)result;
}
extern "C" {

struct Vtable8788 {
    char precedingVirtualSlots[0x88];
    void (*method)();
};
struct Obj8788 {
    Vtable8788* vtable;
    char rendererPrefix[0x4840];
    int state;
};
struct VirtualCallTrial_8008788C {
    virtual void unused_0();
    virtual void unused_1();
    virtual void unused_2();
    virtual void unused_3();
    virtual void unused_4();
    virtual void unused_5();
    virtual void unused_6();
    virtual void unused_7();
    virtual void unused_8();
    virtual void unused_9();
    virtual void unused_10();
    virtual void unused_11();
    virtual void unused_12();
    virtual void unused_13();
    virtual void unused_14();
    virtual void unused_15();
    virtual void unused_16();
    virtual void unused_17();
    virtual void unused_18();
    virtual void unused_19();
    virtual void unused_20();
    virtual void unused_21();
    virtual void unused_22();
    virtual void unused_23();
    virtual void unused_24();
    virtual void unused_25();
    virtual void unused_26();
    virtual void unused_27();
    virtual void unused_28();
    virtual void unused_29();
    virtual void unused_30();
    virtual void unused_31();
    virtual void invoke();
};
} // extern "C"

// Promote pending submission to active and invoke the renderer state reset hook.
void WGeneric3DR::ActivateRenderSubmission() {
    int s = submissionState;
    if (s == kRenderSubmissionPending)
        submissionState = kRenderSubmissionActive;
    reinterpret_cast<VirtualCallTrial_8008788C*>(this)->invoke();
}
extern "C" {

struct Vtable8788b {
    char precedingVirtualSlots[0x8c];
    void (*method)(void*, int);
};
struct Obj8788b {
    Vtable8788b* vtable;
    char rendererPrefix[0x4840];
    int state;
};
struct VirtualCallTrial_800878B0 {
    virtual void unused_0();
    virtual void unused_1();
    virtual void unused_2();
    virtual void unused_3();
    virtual void unused_4();
    virtual void unused_5();
    virtual void unused_6();
    virtual void unused_7();
    virtual void unused_8();
    virtual void unused_9();
    virtual void unused_10();
    virtual void unused_11();
    virtual void unused_12();
    virtual void unused_13();
    virtual void unused_14();
    virtual void unused_15();
    virtual void unused_16();
    virtual void unused_17();
    virtual void unused_18();
    virtual void unused_19();
    virtual void unused_20();
    virtual void unused_21();
    virtual void unused_22();
    virtual void unused_23();
    virtual void unused_24();
    virtual void unused_25();
    virtual void unused_26();
    virtual void unused_27();
    virtual void unused_28();
    virtual void unused_29();
    virtual void unused_30();
    virtual void unused_31();
    virtual void unused_32();
    virtual void invoke(int);
};
} // extern "C"

// Flush through the renderer hook and return active submission state to idle.
void WGeneric3DR::CompleteRenderSubmission() {
    reinterpret_cast<VirtualCallTrial_800878B0*>(this)->invoke(0);
    if (submissionState == kRenderSubmissionPending) {
    } else if (submissionState == kRenderSubmissionActive) {
        submissionState = kRenderSubmissionIdle;
    }
}
extern "C" {} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462038", _ctors_u80086CE0_0, Generic3DR_InitAngleConstant);
#pragma pop
