// Shadow model cache and shared document: the cache that finds or loads shadow models, the pointer
// vector used for them, and the shared document's mode and round defaults.
// It also holds the render-state copy helpers and the string helpers the document uses.

#include <decomp/static_init.h>
// Projected shadow model cache and the shared document's round defaults.
//
// The shadow functions bind the projector textures and transforms, draw the
// model, then reset the temporary texture stages; loaded shadow models are
// cached by resource name. CSharedDoc's constructor installs the round defaults.
extern "C" {
extern unsigned char kProjectedShadowCameraRenderStates[];
}
#include "game/string.h"
#include "game/exception.h"
#include "classes/CSharedDoc.h"
#include "classes/TextureListNode.h"
#include "classes/TextureRenderSetup801F.h"
#include "classes/WPuppet.h"
#include "classes/ShadowList801EF.h"
#include "classes/CHashPath.h"
#include "classes/CItemManager.h"
#include "classes/CSharedDocSingleton.h"

// Private views name address-passed subobjects without changing shared headers.
struct ShadowModelView {
    WPuppet* puppet;
    unsigned int baseTextureStageCount;
};
enum {
    kProjectedTextureDisabled = 0x20,
    kCurrentViewTransform = 0,
    kTextureTransformDimensionsState = 0xD,
    kTextureCoordinateSourceState = 0xA,
    kStageOwnTextureCoordinates = 0x80000000,
    kProjectorTransformSlotCount = 8,
    kBaseStageStatePairCount = 6,
    kBaseStageParameterPairCount = 5,
    kProjectorStatePairCount = 8,
    kProjectorParameterPairCount = 5
};
// State 0x0D is texture-transform dimensions, 0x0A coordinate source, consumed
// by Engine_ApplyStateBeforeDraw (gx_renderer_state_and_resources.cpp, TextureStageBindingView):
// low byte dimensions==3 selects GX_MTX3x4/GX_TG_MTX3x4, otherwise 2x4;
// source 0x80020000 generates from position, 0x80010000/0x80030000 from normal;
// 0x80000000 uses the stage's own UV, other values use low-nibble UV index.
// Cleanup restores dimensions=0, source=0x80000000 (ordinary stage UV).
// Exact preset map, native (engine ID,word) pairs in
// kProjectedShadowCameraRenderStates (.rodata 0x804715B0):
// +D8 base states: (0,0),(1,2),(2,0),(3,0),(4,2),(5,0).
//   IDs0/3=color/alpha multiply; argument1 IDs1/4=2 selects texture;
//   argument2 IDs2/5=0 selects previous result. Thus texture*previous RGBA.
// +108 base sampler parameters: (4,1),(3,1),(5,1),(0,1),(1,1).
//   Parameter4=min,3=mag,5=mip,0=wrapS,1=wrapT; min/mip=(1,1)
//   resolves trilinear, mag=linear, wraps=GX_REPEAT. A one-level texture
//   forces mip=0 in Render_ApplyTextureStage, so mip count matters too.
// +130 projector / +170 extra states (identical):
//   (3,0),(4,2),(5,1),(0,0),(1,2),(2,1),(13,0x103),(10,0x80020000).
//   Also texture*previous; previous selector1 is equivalent to0 in the TEV
//   consumers (gtl_texture_stage_tev_setup.cpp/gx_fifo_vertex_and_tev_helpers.cpp). 0x103's low byte=3 selects three-coordinate
//   projection; its 0x100 bit is preserved but not tested by this GX consumer.
// +1B0 projector sampler: (4,1),(3,1),(5,1),(0,0),(1,0): same filters,
//   GX_CLAMP on both axes. Wrap domain=0 clamp/1 repeat/2 mirror; mag=0 nearest,
//   1 linear. Min/mip is a PAIR lookup, not a raw GX filter enum: consult
//   s_textureMinMipFilterTable in gx_renderer_state_and_resources.cpp before choosing another pair.
// No implicit state restore: only the extra bindings/transforms/IDs13/10 reset.
// A future projector mod must keep sampler, texgen source/dimensions, texture
// resource and matrix slots together; changing the camera-capture prefix alone
// does not change these later presets. These are engine IDs, not GX registers.
enum {
    kSharedPlayerCount = 4,
    kRoundHoleCount = 18,
    kUnusedMaterialSlot = 0xFF,
    kDefaultPangPercent = 100,
    kDefaultGameModeId = 0,
    kNoPartyGame = 0,
    kSharedAllBitsSet = -1,
    kHoleVariantCount = 3,
    kHoleOrderEndValue = kRoundHoleCount + 1
};
// Matrices passed to the renderer are contiguous 16-float, row-vector 4x4
// transforms. Keep four-byte alignment: these are not GX's 3x4 Mtx arrays.
typedef float ShadowMatrix4x4[16];
struct RenderSetting {
    unsigned int parameter, value;
};
struct RenderState12 {
    RenderSetting pairs[6];
};
struct RenderState10 {
    RenderSetting pairs[5];
};
struct RenderState16 {
    RenderSetting pairs[8];
};
struct ShadowRenderSetup {
    char prefix[0x1A8];
    ShadowMatrix4x4 textureBias;
    unsigned int transformSlots[kProjectorTransformSlotCount];
    RenderState12 baseStates;
    RenderState10 baseParameters;
    RenderState16 projectorStates, extraStates;
    RenderState10 projectorParameters;
};
// Each 8-byte descriptor holds a pooled name and three parameter bytes. The
// names are the Stroke Play, Match Play and Caddie Practice labels; the
// parameters are installed as the stored defaults {4,2,3}, {2,2,9}, {1,1,1}.
struct SharedModeDescriptor {
    void* name;
    unsigned char parameter0, parameter1, parameter2, reserved;
};
struct SharedModeDescriptors {
    unsigned int reserved760;
    SharedModeDescriptor stroke, match, practice;
    unsigned char reserved77C[8];
};
enum ModeDescriptorDefaults {
    kStrokeParameter0 = 4,
    kStrokeParameter1 = 2,
    kStrokeParameter2 = 3,
    kMatchParameter0 = 2,
    kMatchParameter1 = 2,
    kMatchParameter2 = 9,
    kPracticeParameter0 = 1,
    kPracticeParameter1 = 1,
    kPracticeParameter2 = 1
};
// The seven owned lists share one header shape: a count followed by the two
// links of an embedded sentinel. ListHeader_InitSentinel leaves them empty, so
// the constructor allocates no nodes; their destructors free nodes only.
struct SharedOwnedListHeader {
    unsigned int count;
    void* sentinelPrevious;
    void* sentinelNext;
};
// Each list has its own count and sentinel at 0x7A4 + 12*i.
// The bonus-item vector at 0x7F8 is a separate allocation, not a list.
enum SharedOwnedListIndex {
    kSharedListAt7A4,
    kSharedListAt7B0,
    kSharedListAt7BC,
    kSharedListAt7C8,
    kSharedListAt7D4,
    kSharedListAt7E0,
    kSharedListAt7EC,
    kSharedOwnedListCount
};
// Pointer vectors own their allocation, not the pointed-to resources: shrinking
// drops slots without releasing payloads. Growth transfers pointer words, clears
// the old words, then frees only the old allocation.
// Shadow cache/projection lists use {count, sentinel links} headers. Each node
// has previous/next at +0/+4 and a resource at +8; sentinel address is header+4.
// Cache nodes additionally store the original model name at +0xC (64 bytes).
struct ProjectedTextureView {
    char prefix[0xF8];
    ShadowMatrix4x4 textureView;
    ShadowMatrix4x4 textureProjection;
    unsigned int textureResource;
    unsigned int flags;
};
struct ExtraProjectedTexture {
    unsigned int textureResource;
    ShadowMatrix4x4 view, projection;
};
struct ShadowCacheNode {
    void* previous;
    void* next;
    void* resource;
    char resourceName[0x40];
};
struct SharedDocInitStorage {
    unsigned char prefix[0x10];
    unsigned char playerStates[4][0x1D4];
    SharedModeDescriptors modeDescriptors;
    unsigned char firstTree[0x10], secondTree[0x10];
    // Seven independent {count, prev, next} headers; the constructor leaves them empty.
    SharedOwnedListHeader ownedLists[kSharedOwnedListCount];
    unsigned char bonusItems[0xC];
    unsigned char gameSettings[0x14];
    unsigned char randomVariants[18];
    unsigned char holeOrder[19];
    unsigned char reservedToSeeds[0x13];
    unsigned int randomSeeds[18];
    unsigned char reservedToCalendar[0xE];
    unsigned char calendarFields[0x12];
    unsigned char courseSettings[0x12C];
    unsigned char roundFlags[0x20];
    unsigned char itemInventory[0xBC];
    unsigned char options[0x1C];
    unsigned char partyPerHole[0x12];
    unsigned char reservedToItemSelection[6];
    unsigned char itemSelection[0x68];
    unsigned char roundCounters[0x14];
};
struct PuppetMaterialSlots {
    unsigned char prefix[12];
    unsigned char slots[8];
    unsigned char tail[8];
};
struct PuppetBoneStringPool {
    char bound[6], center[7], vectorLengthError[20], lengthErrorType[55];
    char shadowSuffix[5];
};
struct PlayModeStrings {
    unsigned char strokePlay[11], matchPlay[7], caddiePractice[17];
};

extern "C" {

extern const float kFloat_6_2831855;
extern float s_sharedDocFullTurn;
extern unsigned char textureCaptureRenderStates[];
extern const unsigned char kIdentityMatrix4x4[];
extern const float kFloat_1_TextureRender;
void* WPuppet_GetSkinMatrices(void*);
void Matrix_ExpandAffineTo4x4(void*, void*);
void Engine_SetTextureStageStateList(unsigned int, void*, unsigned int);
void Engine_SetTextureStageParameterList(unsigned int, void*, unsigned int);
void Engine_SetTextureStageResource(unsigned int, unsigned int);
void Engine_GetTransform(unsigned int, void*);
void PSMTX44Concat(void*, const void*, const void*);
void Mtx44_InvertLegacyCofactors(void*, int, const void*);
void PSMTX44Identity(void*);
void SetTransform(unsigned int, const void*);
void Engine_SetTextureStageState(unsigned int, unsigned int, unsigned int);

unsigned char* WPuppet_GetMaterialMetadata(unsigned char*, unsigned int*);
void String_ReserveCapacity(void*, unsigned int);
void* String_ReplaceRange(void*, unsigned int, unsigned int, const unsigned char*, const unsigned char*,
                          unsigned char*);
void SharedPlayer_ClearRuntimeWords(void*);
void TreeHeader_InitEmptyWithSentinel(void*);
void TreeHeader_InitSentinel(void*);
void ListHeader_InitSentinel(void*);
void PointerVector_InitEmptyStorage(void*);
void RoundCourseSettings_Init(void*);
void ItemSelection_Init(void*);
void SharedDoc_FreeTreesAndLists(void*);
void SharedDoc_InvalidatePlayerState(void*);
unsigned int rand();
void Calendar_GetCurrentShortFields(void*);
void* memset(void*, int, unsigned int);
void* memcpy(void*, const void*, unsigned int);
extern const float kAuxPartDefaultRateModifier;
extern unsigned char __vt__10CSharedDoc[];
extern unsigned char playModeAndCourseAliasStrings[];
extern unsigned char s_shadowResourceLookupEnabled;
extern void* g_resourceManager;
extern const unsigned char puppetBoneExceptionStrings[];
// Runtime thunk to the bounded, case-insensitive string comparison.
int Runtime_StrnicmpThunk(const char*, const char*, unsigned int);
unsigned int strlen(const char*);
char* strrchr(char*, int);
int String_FindLastOf(void*, const char*, int, unsigned int);
void String_ConstructSubstring(void*, void*, int, unsigned int);
void String_ConstructConcatCStr(void*, void*, const char*);

void appFreeUni__FPv(void*);
void* ResourceManager_LoadByExtension(void*, const char*, int, int, int);
void strncpy(void*, const char*, unsigned int);
void* appMallocDummy__FUl(unsigned int);
void* __nwa__FUl(unsigned int);
void __throw(const char*, void*, void (*)(void*));
void __dt__Q23std12length_errorFv(void*);
void* __nw__FUl(unsigned int);
void __dla__FPv(void*);
void strcpy(char*, const char*);

enum {
    kMaxPointerCount = 0x3FFFFFFF,
    kSmallPointerGrowthLimit = 0x15555555,
    kMediumPointerGrowthLimit = 0x2AAAAAAA
};

struct PtrVector {
    void** data;
    unsigned int size;
    unsigned int capacity;
};

extern unsigned char __vt__Q23std11logic_error[], s_sharedCharDeleterVtable[], __vt__Q23std12length_error[];
void dtor_80046988(void*);
struct ErrorBuffer {
    char* ptr;
    unsigned char* allocator;
    // Own exception text until a shared control block adopts it.
    ErrorBuffer(char* p, unsigned char* a) : ptr(p), allocator(a) {}
    // Release text only while ownership has not been transferred.
    ~ErrorBuffer() {
        if (ptr)
            __dla__FPv(ptr);
    }
};
struct ErrorControl {
    void* table;
    unsigned int refs, owners;
    char* ptr;
    // Adopt exception text with initial shared/weak ownership counts of one.
    ErrorControl(char* p) {
        refs = 1;
        owners = 1;
        table = s_sharedCharDeleterVtable;
        ptr = p;
    }
};
struct ErrorAllocator {
    unsigned char tag;
    // Initialize the allocator tag used by exception-string construction.
    ErrorAllocator() : tag(0) {}
    unsigned char pad[3];
};
namespace std {
struct logic_error : exception {
    // Install the runtime exception vtable without changing the ABI layout.
    void SetTable(const void* vt) {
        *(const void**)this = vt;
    }

    char* message;
    ErrorControl* control;
    // Deep-copy the exception message and transfer it to a shared control block.
    logic_error(const char* text, const ErrorAllocator& allocator = ErrorAllocator()) {
        SetTable(__vt__Q23std11logic_error);
        unsigned int length = strlen(text) + 1;
        char* p = (char*)__nwa__FUl(length * 2 - length);
        message = p;
        {
            ErrorBuffer buffer(p, (unsigned char*)&allocator);
            control = new ErrorControl(p);
            buffer.ptr = 0;
        }
        strcpy(message, text);
    }

    ~logic_error();
};
}
} // extern C
namespace std {
struct length_error : logic_error {
    // Construct a logic-error payload with the length-error runtime type.
    length_error(const char* text) : logic_error(text) {
        SetTable(__vt__Q23std12length_error);
    }
    ~length_error();
};
}
extern "C" {
// Throw the pointer-vector's length_error using its original string-pool entry.
inline void VectorLengthError() {
    throw std::length_error((char*)puppetBoneExceptionStrings + (int)&((PuppetBoneStringPool*)0)->vectorLengthError);
}
struct VectorTransaction {
    void** data;
    unsigned int size, capacity;
    // Borrow the destination capacity word.
    // It records the association only; this unit never dereferences it.
    unsigned int* destinationCapacity;
    unsigned int offset;
    // Start an empty transaction associated with the destination capacity word.
    VectorTransaction(unsigned int* a) : data(0), size(0), capacity(0), destinationCapacity(a), offset(0) {}
    // Free uncommitted storage, or the old allocation after a successful swap.
    ~VectorTransaction() {
        if (data) {
            size = 0;
            appFreeUni__FPv(data);
        }
    }
};
// Select the larger growth request while preserving its reference identity.
inline const unsigned int& VectorMax(const unsigned int& a, const unsigned int& b) {
    return a < b ? b : a;
}
#pragma push
#pragma use_lmw_stmw on
// Append a resource pointer, growing storage transactionally while preserving old pointers.
// Required valid vector (size<=capacity) and value pointing to one borrowed
// pointer word, which may be in the old array. Tail is copied BEFORE old slots
// are zeroed/freed, preserving aliasing. Owns allocations only, never retains/
// releases resource payloads. Excess length throws; transaction frees uncommitted
// storage on unwind. Keep growth tiers and commit ordering, not a gameplay knob.
void ShadowModelPointerVector_Append(PtrVector* self, void** value) {
    int prefix;
    unsigned int size = self->size;
    unsigned int capacity = self->capacity;
    if (capacity > size) {
        ++self->size;
        self->data[self->size - 1] = *value;
        return;
    }

    // Reserve a guarded replacement allocation, constructing the new tail first.
    if (kMaxPointerCount - self->capacity < 1)
        throw std::length_error((char*)puppetBoneExceptionStrings +
                                (int)&((PuppetBoneStringPool*)0)->vectorLengthError);
    VectorTransaction transaction(&self->capacity);
    unsigned int needed = self->size + 1 - self->capacity;
    unsigned int newCapacity = self->capacity;
    if (needed > kMaxPointerCount - newCapacity)
        throw std::length_error((char*)puppetBoneExceptionStrings +
                                (int)&((PuppetBoneStringPool*)0)->vectorLengthError);

    unsigned int largeGrowth;
    if (newCapacity < kSmallPointerGrowthLimit) {
        unsigned int smallGrowth;
        smallGrowth = (newCapacity + 1) * 3 / 5;
        newCapacity += VectorMax(smallGrowth, needed);
    } else if (newCapacity < kMediumPointerGrowthLimit) {
        largeGrowth = (newCapacity + 1) / 2;
        newCapacity += VectorMax(largeGrowth, needed);
    } else {
        newCapacity = kMaxPointerCount;
    }
    if (newCapacity > kMaxPointerCount)
        throw std::length_error((char*)puppetBoneExceptionStrings +
                                (int)&((PuppetBoneStringPool*)0)->vectorLengthError);
    void** fresh = (void**)appMallocDummy__FUl(newCapacity * 4);
    transaction.offset = self->size;
    (fresh + transaction.offset)[transaction.size] = *value;
    transaction.data = fresh;
    transaction.capacity = newCapacity;
    ++transaction.size;

    // Transfer the old prefix as pointer words; the resources are not copied/retained.
    void** first = self->data;
    prefix = ((char*)(self->data + self->size) - (char*)first) / 4;
    transaction.offset -= prefix;
    memcpy(transaction.offset + transaction.data, first, prefix * 4);
    memset(first, 0, prefix * 4);
    transaction.size += prefix;
    // Commit the new vector. The transaction now owns only the old allocation.
    void** oldData = self->data;
    self->size = 0;
    unsigned int oldCapacity = self->capacity;
    self->capacity = transaction.capacity;
    transaction.capacity = oldCapacity;
    self->data = transaction.data;
    transaction.data = oldData;
    self->size = transaction.size;
    transaction.size = 0;
}
#pragma pop

// Initialize the shared-document full-turn angle cache.
void SharedDoc_InitFullTurnConstant() {
    s_sharedDocFullTurn = kFloat_6_2831855;
}

// Copy six state/value pairs into the local GX-state staging block.
inline void CopyRender12(RenderState12* d, const RenderState12* s) {
    *d = *s;
}
// Copy five parameter/value pairs into the local staging block.
inline void CopyRender10(RenderState10* d, const RenderState10* s) {
    *d = *s;
}
// Copy eight state/value pairs into the local projection staging block.
inline void CopyRender16(RenderState16* d, const RenderState16* s) {
    *d = *s;
}
struct RenderInterface {
    virtual void slot2();
    virtual void slot3();
    virtual void slot4();
    virtual void slot5();
    virtual void slot6();
    virtual void slot7();
    virtual void slot8();
    virtual void slot9();
    virtual void draw(void*, int, int, int, float, int, void*, int);
};

// Compose texture-projection transforms, draw the puppet, then reset all temporary texture stages.
// Inputs are borrowed. The combined base/projection/extra stage count must fit
// the renderer's transform-slot table. Returns 1 after drawing; restores added
// stages to defaults rather than saving/restoring arbitrary caller state.
// drawContext is forwarded unchanged to virtual draw +0x28; required puppet
// and skin matrix storage are borrowed. projectionList may be null; otherwise
// it is a sentinel list of TextureListNode values (no null payload guard).
// extraProjection is nullable. No status propagates from draw/matrix helpers.
// Base stages remain configured; extra stages reset transform, texture, states
// 0x0D/0x0A. There is no exception guard to restore them if the draw throws.
// Virtual draw boundary: self+0 is the borrowed WPuppet pointer, so the apparent
// (*(RenderInterface**)self) receiver is that puppet, not the wrapper. Its +28
// slot is WPuppet_QueueModelDraw (phase1_class_tables.cpp / wpuppet_model_bone_animation.cpp).
// Call shape: (context,1,1,1,1.0f,0,skinMatrices,0). The current source consumer
// uses the first integer flag to choose world matrix versus identity; uses the
// explicit skinMatrices (null there would select puppet skinMatrices); and uses
// the final zero as optional draw-payload word. Other context/options/float are
// unused by that implementation: no material/opacity semantics are inferred.
// Its eight-word queue payload is {skinMatrices,0,materials,0,optionalWord,
// modelLayerVisibilityWords,0,0}, borrowed model arrays; matrices are expanded to 4x4.
// Queueing requires those resources through render consumption; this function
// acquires no new references. Preserve the complete ABI tuple for overrides and
// the queue's copy/lifetime contract; zero modelResource makes QueueModelDraw
// submit nothing, yet this function still resets stages and returns 1.
int ShadowModel_DrawWithProjectedTextureStages(void* self, void* drawContext, void* projectionList,
                                               void* extraProjection) {
    unsigned int stageIndex;
    unsigned int textureStage;
    RenderState12 baseStageStates;
    RenderState10 baseStageParameters;
    RenderState16 projectorStageStates, extraStageStates;
    RenderState10 projectorStageParameters;
    ShadowMatrix4x4 worldMatrix4x4;
    ShadowMatrix4x4 worldSkinProduct;
    ShadowMatrix4x4 inverseCurrentView;
    ShadowMatrix4x4 unusedInverseWorldSkin;
    ShadowMatrix4x4 unusedInverseStorage;
    ShadowMatrix4x4 viewToTexture;
    ShadowMatrix4x4 unusedIdentityStorage;
    ShadowMatrix4x4 extraWorldSkinProduct, extraInverseCurrentView, extraUnusedInverseWorldSkin, extraViewToTexture;
    ShadowRenderSetup* base = (ShadowRenderSetup*)textureCaptureRenderStates;
    void* skinMatrices = WPuppet_GetSkinMatrices(((ShadowModelView*)self)->puppet);

    // Install base stages, then reserve successive stages for enabled projectors.
    CopyRender12(&baseStageStates, (RenderState12*)((char*)(&base->baseStates)));
    CopyRender10(&baseStageParameters, (RenderState10*)((char*)(&base->baseParameters)));
    for (stageIndex = 0; stageIndex < ((ShadowModelView*)self)->baseTextureStageCount; ++stageIndex) {
        Engine_SetTextureStageStateList(stageIndex, &baseStageStates, kBaseStageStatePairCount);
        Engine_SetTextureStageParameterList(stageIndex, &baseStageParameters, kBaseStageParameterPairCount);
    }

    CopyRender16(&projectorStageStates, (RenderState16*)((char*)(&base->projectorStates)));
    CopyRender16(&extraStageStates, (RenderState16*)((char*)(&base->extraStates)));
    CopyRender10(&projectorStageParameters, (RenderState10*)((char*)(&base->projectorParameters)));
    Matrix_ExpandAffineTo4x4(((ShadowModelView*)self)->puppet->worldMatrix, worldMatrix4x4);

    textureStage = ((ShadowModelView*)self)->baseTextureStageCount;
    if (projectionList) {
        void* sentinel = &AsShadowList801EF(projectionList)->sentinelNext;
        unsigned int* transforms = ((unsigned int*)((char*)(base->transformSlots)));
        for (void* node = AsShadowList801EF(projectionList)->sentinelPrevious; node != sentinel;
             node = ((TextureListNode*)node)->next) {
            void* object = ((TextureListNode*)node)->value; // 0x801F23C0: list node +8
            if (((ProjectedTextureView*)object)->flags & kProjectedTextureDisabled)
                continue;
            Engine_SetTextureStageResource(textureStage, ((ProjectedTextureView*)object)->textureResource);
            Engine_SetTextureStageStateList(textureStage, &projectorStageStates, kProjectorStatePairCount);
            Engine_SetTextureStageParameterList(textureStage, &projectorStageParameters, kProjectorParameterPairCount);
            object = ((TextureListNode*)node)->value;
            // Engine concat is (dst,left,right) with row-vector matrices:
            // texture = inverse(current view) * projector view * projector
            // projection * texture bias. This maps view coordinates to texture
            // coordinates. The world*skin inverse is computed but not consumed.
            Engine_GetTransform(kCurrentViewTransform, inverseCurrentView);
            PSMTX44Concat(worldSkinProduct, worldMatrix4x4, skinMatrices);
            Mtx44_InvertLegacyCofactors(unusedInverseWorldSkin, 0, worldSkinProduct);
            Mtx44_InvertLegacyCofactors(inverseCurrentView, 0, inverseCurrentView);
            PSMTX44Identity(viewToTexture);
            PSMTX44Concat(viewToTexture, viewToTexture, inverseCurrentView);
            PSMTX44Concat(viewToTexture, viewToTexture, ((ProjectedTextureView*)object)->textureView);
            PSMTX44Concat(viewToTexture, viewToTexture, ((ProjectedTextureView*)object)->textureProjection);
            PSMTX44Concat(viewToTexture, viewToTexture, ((char*)(base->textureBias)));
            SetTransform(transforms[textureStage], viewToTexture);
            ++textureStage;
        }
    }
    // Optional final projector follows the same matrix chain with its own state.
    if (extraProjection) {
        Engine_SetTextureStageResource(textureStage, ((ExtraProjectedTexture*)extraProjection)->textureResource);
        Engine_SetTextureStageStateList(textureStage, &extraStageStates, kProjectorStatePairCount);
        Engine_SetTextureStageParameterList(textureStage, &projectorStageParameters, kProjectorParameterPairCount);
        Engine_GetTransform(kCurrentViewTransform, extraInverseCurrentView);
        PSMTX44Concat(extraWorldSkinProduct, worldMatrix4x4, skinMatrices);
        Mtx44_InvertLegacyCofactors(extraUnusedInverseWorldSkin, 0, extraWorldSkinProduct);
        Mtx44_InvertLegacyCofactors(extraInverseCurrentView, 0, extraInverseCurrentView);
        PSMTX44Identity(extraViewToTexture);
        PSMTX44Concat(extraViewToTexture, extraViewToTexture, extraInverseCurrentView);
        PSMTX44Concat(extraViewToTexture, extraViewToTexture, ((ExtraProjectedTexture*)extraProjection)->view);
        PSMTX44Concat(extraViewToTexture, extraViewToTexture, ((ExtraProjectedTexture*)extraProjection)->projection);
        PSMTX44Concat(extraViewToTexture, extraViewToTexture, ((char*)(base->textureBias)));
        SetTransform(((unsigned int*)((char*)(base->transformSlots)))[textureStage], extraViewToTexture);
        ++textureStage;
    }
    // Invoke the renderer's draw method with the selected state.
    (*(RenderInterface**)self)->draw(drawContext, 1, 1, 1, kFloat_1_TextureRender, 0, skinMatrices, 0);
    // Unbind only the extra stages, resetting their texture transforms to identity.
    unsigned int first = ((ShadowModelView*)self)->baseTextureStageCount;
    for (stageIndex = first; stageIndex < textureStage; ++stageIndex) {
        Engine_SetTextureStageResource(stageIndex, 0);
        // Preserve the separate table-base calculation used by the original
        // cleanup iterator; the typed table above defines its eight-slot extent.
        SetTransform(
            ((unsigned int*)(((char*)((char*)base + (int)&((ShadowRenderSetup*)0)->transformSlots))))[stageIndex],
            kIdentityMatrix4x4);
        Engine_SetTextureStageState(stageIndex, kTextureTransformDimensionsState, 0);
        Engine_SetTextureStageState(stageIndex, kTextureCoordinateSourceState, kStageOwnTextureCoordinates);
    }
    return 1;
}

// Resize the pointer list, zero new slots, and grow capacity with the original allocator policy.
// Required valid vector; requested is a count of four-byte pointers, <=0x3FFFFFFF.
// Shrink changes size only, leaving capacity/payloads intact. Growth appends null
// words and transfers old pointer values without retaining their resources.
// Length overflow throws; no return status. Preserve count/byte distinction and
// old-slot zeroing before old allocation release; ownership stays with vector.
void ShadowModelPointerVector_Resize(PtrVector* self, unsigned int requested) {
    unsigned int oldSize = self->size;
    if (requested <= oldSize) {
        self->size = requested;
        return;
    }

    unsigned int count = requested - oldSize;
    unsigned int oldCapacity = self->capacity;
    if (count <= oldCapacity) {
        unsigned int remain = oldCapacity - count;
        if (oldSize <= remain) {
            memset(self->data + oldSize, 0, count * 4);
            self->size = requested;
            return;
        }
    }

    // Existing capacity is insufficient; choose a tier and transfer into fresh storage.
    unsigned int capacity = self->capacity;
    unsigned int max = kMaxPointerCount;
    unsigned int newCapacity;
    if (requested - capacity > max - capacity) {
        VectorLengthError();
        return;
    } else if (capacity < kSmallPointerGrowthLimit) {
        unsigned int grown = capacity + (capacity + 1) * 3 / 5;
        newCapacity = grown < requested ? requested : grown;
    } else if (capacity < kMediumPointerGrowthLimit) {
        unsigned int grown = capacity + (capacity + 1) / 2;
        newCapacity = grown < requested ? requested : grown;
    } else {
        newCapacity = max;
    }
    if (newCapacity > max)
        VectorLengthError();

    void** fresh = (void**)appMallocDummy__FUl(newCapacity * 4);
    unsigned int prefix = oldSize;
    memset(fresh + oldSize, 0, count * 4);
    if (prefix)
        memcpy(fresh, self->data, prefix * 4);
    void** oldData = self->data;
    if (prefix)
        memset(oldData, 0, prefix * 4);
    self->data = fresh;
    self->size = requested;
    self->capacity = newCapacity;
    if (oldData)
        appFreeUni__FPv(oldData);
}

// Write the three mode-name/parameter records without touching reserved bytes.
static inline void initializeSharedModeDescriptors(SharedDocInitStorage* doc) {
    SharedModeDescriptors* modes = &doc->modeDescriptors;
    modes->stroke.parameter0 = kStrokeParameter0;
    modes->stroke.parameter1 = kStrokeParameter1;
    modes->stroke.parameter2 = kStrokeParameter2;
    modes->stroke.name = playModeAndCourseAliasStrings + (int)&((PlayModeStrings*)0)->strokePlay;
    modes->match.parameter0 = kMatchParameter0;
    modes->match.parameter1 = kMatchParameter1;
    modes->match.parameter2 = kMatchParameter2;
    modes->match.name = playModeAndCourseAliasStrings + (int)&((PlayModeStrings*)0)->matchPlay;
    modes->practice.parameter0 = kPracticeParameter0;
    modes->practice.parameter1 = kPracticeParameter1;
    modes->practice.parameter2 = kPracticeParameter2;
    modes->practice.name = playModeAndCourseAliasStrings + (int)&((PlayModeStrings*)0)->caddiePractice;
}

} // extern "C"

// Install the round defaults: four empty player records, the owned containers,
// the mode names, and one random seed and three-way variant per hole.
CSharedDoc::CSharedDoc() {
    ((CSharedDocSingleton*)this)->__ct();
    *(void**)this = __vt__10CSharedDoc;

    SharedDocInitStorage* doc = (SharedDocInitStorage*)this;
    unsigned char* player = doc->playerStates[0];
    do {
        SharedPlayer_ClearRuntimeWords(player);
        player += sizeof(doc->playerStates[0]);
    } while (player < (unsigned char*)&doc->modeDescriptors);

    TreeHeader_InitEmptyWithSentinel(doc->firstTree);
    TreeHeader_InitSentinel(doc->secondTree);
    ListHeader_InitSentinel(&doc->ownedLists[kSharedListAt7A4]);
    ListHeader_InitSentinel(&doc->ownedLists[kSharedListAt7B0]);
    ListHeader_InitSentinel(&doc->ownedLists[kSharedListAt7BC]);
    ListHeader_InitSentinel(&doc->ownedLists[kSharedListAt7C8]);
    ListHeader_InitSentinel(&doc->ownedLists[kSharedListAt7D4]);
    ListHeader_InitSentinel(&doc->ownedLists[kSharedListAt7E0]);
    ListHeader_InitSentinel(&doc->ownedLists[kSharedListAt7EC]);
    PointerVector_InitEmptyStorage(doc->bonusItems);
    RoundCourseSettings_Init(doc->courseSettings);
    ((CItemManager*)itemInventory)->__ct();
    ItemSelection_Init(doc->itemSelection);

    golfDoc = 0;
    SharedDoc_FreeTreesAndLists(this);
    SharedDoc_InvalidatePlayerState(this);
    ClearPlayerStateStorage();

    initializeSharedModeDescriptors(doc);
    courseRecordUpdatesEnabled = 0;
    roundHoleOrder = 0;
    gameMode = kDefaultGameModeId;
    unk_80C = 0;
    unk_814 = 0;
    pangRewardMultiplier = kAuxPartDefaultRateModifier;
    pangRewardPercent = kDefaultPangPercent;
    randomizePlayerStats = 0;
    unk_AC4 = 0;
    unk_AD0 = 0;
    unk_AD4 = 0;
    partyGameMode = kNoPartyGame;
    memset(doc->partyPerHole, 0, kRoundHoleCount);
    partyResultTier = 0;
    unk_ACC = kSharedAllBitsSet;

    // One random word per hole is both its saved seed and its three-way variant;
    // holeOrder is one-based and ends with a nineteenth value.
    unsigned char i;
    for (i = 0; i < kRoundHoleCount; ++i) {
        unsigned int value = rand();
        doc->randomSeeds[i] = value;
        doc->randomVariants[i] = (unsigned char)(value % kHoleVariantCount);
        doc->holeOrder[i] = (unsigned char)(i + 1);
    }
    doc->holeOrder[i] = kHoleOrderEndValue;

    Calendar_GetCurrentShortFields(doc->calendarFields);
    memset(doc->roundCounters, 0, sizeof(doc->roundCounters));
    unk_AC8 = 0;
    unk_AC9 = 0;
    memset(doc->roundFlags, 0, sizeof(doc->roundFlags));
    courseRecordIndex = 0;
    enemyLevelIndex = 0;
    partyCourseTableIndex = 0;
    pendingCourseClearNotification = 0;
    menuTextureIndex = 0;
}

extern "C" {

// Find the largest consecutive material-slot prefix before the 0xFF unused-slot marker.
// Required out[2 words] and valid borrowed WPuppet material data. Writes data
// pointer plus max prefix length 0..8 across count 0x1C-byte records. Later
// populated bytes after the FIRST 0xFF do not contribute. No allocation/status.
void PuppetMaterial_InitMaxUsedSlotCount(void* out, unsigned char* data) {
    unsigned int i;
    unsigned int count;
    unsigned char* entry;
    unsigned char usedMaterialSlots;
    ((void**)out)[0] = data;
    ((unsigned int*)out)[1] = 0;
    entry = WPuppet_GetMaterialMetadata(data, &count);
    i = 0;
    while (i < count) {
        // 0x801F2978: stop at the first unused material slot.
        do {
            usedMaterialSlots = 0;
            if (((PuppetMaterialSlots*)entry)->slots[0] == kUnusedMaterialSlot)
                break;
            usedMaterialSlots = 1;
            if (((PuppetMaterialSlots*)entry)->slots[1] == kUnusedMaterialSlot)
                break;
            usedMaterialSlots = 2;
            if (((PuppetMaterialSlots*)entry)->slots[2] == kUnusedMaterialSlot)
                break;
            usedMaterialSlots = 3;
            if (((PuppetMaterialSlots*)entry)->slots[3] == kUnusedMaterialSlot)
                break;
            usedMaterialSlots = 4;
            if (((PuppetMaterialSlots*)entry)->slots[4] == kUnusedMaterialSlot)
                break;
            usedMaterialSlots = 5;
            if (((PuppetMaterialSlots*)entry)->slots[5] == kUnusedMaterialSlot)
                break;
            usedMaterialSlots = 6;
            if (((PuppetMaterialSlots*)entry)->slots[6] == kUnusedMaterialSlot)
                break;
            usedMaterialSlots = 7;
            if (((PuppetMaterialSlots*)entry)->slots[7] == kUnusedMaterialSlot)
                break;
            usedMaterialSlots = 8;
        } while (0);
        if (((unsigned int*)out)[1] < usedMaterialSlots)
            ((unsigned int*)out)[1] = usedMaterialSlots;
        entry += sizeof(PuppetMaterialSlots);
        ++i;
    }
}

typedef NString ConcatResult;
#pragma push
#pragma use_lmw_stmw on
// Build an owned narrow string from two inline-or-heap strings, with exception-safe temporary storage.
// Inputs remain borrowed and unchanged; the returned string owns any heap payload.
// The result is built straight into the caller's storage (the hidden return pointer, passed first as r3).
ConcatResult String_ConcatTwoStrings(const void* leftString, void* rightString) {
    unsigned int leftFlags = ((const unsigned int*)leftString)[0];
    ConcatResult result;
    result.flags = 0;
    result.length = 0;
    result.data = 0;
    unsigned int leftLength;
    unsigned int rightLength;
    const unsigned char* leftData;
    unsigned char* rightData;
    if (!(leftFlags >> 31)) {
        leftData = (const unsigned char*)leftString + 1;
        leftLength = *(const unsigned char*)leftString & 0x7F;
    } else {
        leftData = (const unsigned char*)((const unsigned int*)leftString)[2];
        leftLength = ((const unsigned int*)leftString)[1];
    }
    if (!(((unsigned int*)rightString)[0] >> 31)) {
        rightData = (unsigned char*)rightString + 1;
        rightLength = *(unsigned char*)rightString & 0x7F;
    } else {
        rightData = (unsigned char*)((unsigned int*)rightString)[2];
        rightLength = ((unsigned int*)rightString)[1];
    }
    String_ReserveCapacity(&result, leftLength + rightLength);
    union Tag {
        unsigned int align;
        unsigned char byte;
    };
    volatile Tag srcA;
    Tag first;
    first.byte = srcA.byte;
    void* dest = String_ReplaceRange(&result, 0, 0, leftData, leftData + leftLength, &first.byte);
    volatile Tag srcB;
    Tag second;
    second.byte = srcB.byte;
    String_ReplaceRange(dest, leftLength, 0, rightData, rightData + rightLength, &second.byte);
    return result;
}
#pragma pop

struct MapEntry {
    void* owner;
    void* value;
    char name[0x40];
};
struct Str {
    unsigned flags;
    unsigned len;
    char* ptr;
    // Initialize an empty tagged string with inline storage.
    Str() {
        flags = 0;
        len = 0;
        ptr = 0;
    }
    // Release only an owned heap representation (tag bit 31).
    ~Str() {
        if (flags >> 31)
            appFreeUni__FPv(ptr);
    }
};
struct StrD {
    unsigned flags;
    unsigned len;
    char* ptr;
    // Release a substring/concatenation result's owned heap representation.
    ~StrD() {
        if (flags >> 31)
            appFreeUni__FPv(ptr);
    }
};
} // extern "C"
// Borrow the character span from either tagged-string storage view.
template <class Storage> inline char* ShadowStringData(Storage& s) {
    return !(s.flags >> 31) ? (char*)&s + 1 : s.ptr;
}
// Decode byte length from the inline tag or the heap representation's length word.
template <class Storage> inline unsigned ShadowStringLength(Storage& s) {
    return !(s.flags >> 31) ? (unsigned)(*(unsigned char*)&s & 0x7f) : s.len;
}
extern "C" {
#pragma push
#pragma use_lmw_stmw on
// Find a cached model name ignoring case, or load/cache its shadow-suffixed variant.
// object is a borrowed WPuppet with a name containing an extension. list owns
// cache nodes and tracks loaded resources for later cleanup; the result is a
// borrowed cached instance, with no extra reference acquired on a cache hit.
// Required list/object and NUL-terminated resourceName containing '.'; name
// construction trusts the extension. Disabled lookup/missing suffix path/failed
// load returns null without inserting a node; there is no fallback to original.
// Successful load is stored in one 0x4C-byte node, owned by list; temporaries
// release their own strings. Preserve bounded key comparison and insertion order.
void* ShadowModel_FindOrLoadCachedResource(void* list, void* object, int useOriginalModelName) {
    // Phase 1: cache identity is the ORIGINAL resource name, folded case,
    // compared for at most 64 bytes. Naming mode is absent from the key: the
    // first successful load chooses the resource for subsequent calls in either mode.
    if (!s_shadowResourceLookupEnabled)
        return 0;
    void* resource = 0;
    if (*(void**)list) {
        void* sentinel = &AsShadowList801EF(list)->sentinelNext;
        void* node = ((void**)list)[2];
        while (sentinel != node) {
            if (Runtime_StrnicmpThunk(((ShadowCacheNode*)node)->resourceName, ((WPuppet*)object)->resourceName,
                                      sizeof(((ShadowCacheNode*)0)->resourceName)) == 0) {
                resource = ((TextureListNode*)node)->value;
                break;
            }
            node = ((TextureListNode*)node)->next;
        }
    }
    if (resource)
        return resource;

    // Phase 2: build name-with-shadow-suffix; the original name must have an extension.
    // Cache misses build the shadow suffix before the extension, even when the
    // useOriginalModelName flag later chooses to load the original path.
    Str name;
    const char* objName = ((WPuppet*)object)->resourceName;
    unsigned int nameLen = strlen(objName);
    volatile unsigned char srcA;
    String_ReserveCapacity(&name, nameLen);
    unsigned char tagA = srcA;
    String_ReplaceRange(&name, 0, 0, (const unsigned char*)objName, (const unsigned char*)objName + nameLen, &tagA);
    char* extension = strrchr(ShadowStringData(name), '.');
    Str pattern;
    const char* shadowSuffix = (char*)puppetBoneExceptionStrings + (int)&((PuppetBoneStringPool*)0)->shadowSuffix;
    unsigned int suffixLength = strlen(shadowSuffix);
    volatile unsigned char srcB;
    String_ReserveCapacity(&pattern, suffixLength);
    unsigned char tagB = srcB;
    String_ReplaceRange(&pattern, 0, 0, (const unsigned char*)shadowSuffix,
                        (const unsigned char*)shadowSuffix + suffixLength, &tagB);
    {
        StrD path;
        int pos = String_FindLastOf(&name, extension, -1, strlen(extension));
        String_ConstructSubstring(&path, &name, 0, pos - (strlen(extension) - 1));
        StrD combined;
        ((void (*)(void*, void*, void*))String_ConcatTwoStrings)(&combined, &path, &pattern);
        StrD inserted;
        String_ConstructConcatCStr(&inserted, &combined, extension);
        if (!(name.flags >> 31) && !(inserted.flags >> 31)) {
            name.flags = inserted.flags;
            name.len = inserted.len;
            name.ptr = inserted.ptr;
        } else {
            unsigned int nlen = ShadowStringLength(name);
            const unsigned char* ip = (const unsigned char*)ShadowStringData(inserted);
            unsigned int ilen = ShadowStringLength(inserted);
            volatile unsigned char srcC;
            unsigned char tagC = srcC;
            String_ReplaceRange(&name, 0, nlen, ip, ip + ilen, &tagC);
        }
    }
    // Phase 3: choose the load path; there is no original-path fallback on a suffix miss.
    if (!useOriginalModelName) {
        const char* key = ShadowStringData(name);
        if (((CHashPath*)g_resourceManager)->FindPathForwarder(key))
            resource = ResourceManager_LoadByExtension(g_resourceManager, key, 0, 1, 0);
    } else {
        resource = ResourceManager_LoadByExtension(g_resourceManager, objName, 0, 1, 0);
    }
    if (!resource)
        return 0;
    // Phase 4: publish only a successfully loaded resource under the original key.
    // Cache under the original name so subsequent requests share this loaded model.
    // strncpy copies at most 64 bytes and need not append a terminator.
    unsigned char* newNode = (unsigned char*)appMallocDummy__FUl(0x4C);
    ((TextureListNode*)newNode)->value = resource;
    strncpy(((ShadowCacheNode*)newNode)->resourceName, objName, 0x40);
    void* sentinel = &AsShadowList801EF(list)->sentinelNext;
    void* prev = *(void**)sentinel;
    *(void**)(newNode) = prev;
    ((TextureListNode*)newNode)->next = sentinel;
    *(void**)sentinel = newNode;
    ((TextureListNode*)prev)->next = newNode;
    ++*(unsigned int*)list;
    return resource;
}
#pragma pop

// Compile-time guards for private ABI views used above.
#define LOCAL_FIELD(t, f, o) typedef char local_##t##_##f[((int)&((t*)0)->f == (o)) ? 1 : -1]
LOCAL_FIELD(ProjectedTextureView, textureView, 0xF8);
LOCAL_FIELD(ProjectedTextureView, textureProjection, 0x138);
LOCAL_FIELD(ProjectedTextureView, textureResource, 0x178);
LOCAL_FIELD(ExtraProjectedTexture, projection, 0x44);
LOCAL_FIELD(ShadowCacheNode, resourceName, 0xC);
LOCAL_FIELD(ShadowRenderSetup, textureBias, 0x1A8);
LOCAL_FIELD(ShadowRenderSetup, transformSlots, 0x1E8);
LOCAL_FIELD(ShadowRenderSetup, baseStates, 0x208);
LOCAL_FIELD(ShadowRenderSetup, baseParameters, 0x238);
LOCAL_FIELD(ShadowRenderSetup, projectorStates, 0x260);
LOCAL_FIELD(ShadowRenderSetup, extraStates, 0x2A0);
LOCAL_FIELD(ShadowRenderSetup, projectorParameters, 0x2E0);
LOCAL_FIELD(SharedDocInitStorage, modeDescriptors, 0x760);
LOCAL_FIELD(SharedModeDescriptors, stroke, 4);
LOCAL_FIELD(SharedModeDescriptors, match, 12);
LOCAL_FIELD(SharedModeDescriptors, practice, 20);
LOCAL_FIELD(SharedDocInitStorage, firstTree, 0x784);
LOCAL_FIELD(SharedDocInitStorage, ownedLists, 0x7A4);
LOCAL_FIELD(SharedDocInitStorage, bonusItems, 0x7F8);
LOCAL_FIELD(SharedDocInitStorage, randomVariants, 0x818);
LOCAL_FIELD(SharedDocInitStorage, holeOrder, 0x82A);
LOCAL_FIELD(SharedDocInitStorage, randomSeeds, 0x850);
LOCAL_FIELD(SharedDocInitStorage, calendarFields, 0x8A6);
LOCAL_FIELD(SharedDocInitStorage, courseSettings, 0x8B8);
LOCAL_FIELD(SharedDocInitStorage, itemInventory, 0xA04);
LOCAL_FIELD(SharedDocInitStorage, partyPerHole, 0xADC);
LOCAL_FIELD(SharedDocInitStorage, itemSelection, 0xAF4);
LOCAL_FIELD(SharedDocInitStorage, roundCounters, 0xB5C);
LOCAL_FIELD(PuppetBoneStringPool, shadowSuffix, 0x58);
#undef LOCAL_FIELD

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804621B8", _ctors_u801F1A5C_0, SharedDoc_InitFullTurnConstant);
#pragma pop
