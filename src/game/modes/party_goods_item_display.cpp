#include <decomp/static_init.h>
// Party Goods selection/presentation plus stream-to-texture creation and font ownership.
// Mod lookup (texture, model resources, frame rate, UI coordinates):
// - Encoded GSR metadata/palette: gtl_Image* in gtl_image_decode_and_tiling.cpp/gtl_image_format_dispatch.cpp; this unit's
//   stream path always requests format 14 and uploads RGBA8, NOT indexed palettes.
//   Palette creation/upload is Texture_LoadResourceWithOptionalPalette (engine_render_cache_and_texture_load.cpp).
// - Texture dimensions: Texture_CreateFromStream gets imageInfo[1]/[2] in pixels;
//   Texture_CreateFromEncodedRgbaImage forwards them to Texture_Create2D (gx_renderer_state_and_resources.cpp).
//   kUploadSwapRedBlueFormat=14, platform selector=7, pixel bytes=4 must agree.
//   Preserve complete four-pixel runs and sufficient padded destination storage.
// - Stream buffer alignment: kImageStreamAlignmentBytes=32 below rounds the read
//   SIZE; it does not establish the allocation address alignment. Read/open/decode
//   statuses are unchecked, and malformed streams have no safe fallback here.
// - Appearance timing: kEffectApproachFrames/kEffectExitFrames=30 update calls;
//   kEffectFadeStep=17 alpha units/call; kPartyOpacityPulseFrames=15 calls.
//   These do not use the engine delta. See UpdatePartyEffectExitPose's zero divisor.
// - Appearance placement: u800A38AC_data.cpp supplies target X=580, approach Y=150
//   and exit Y=kPartyAppearanceSecondTargetY=50; layouts/BP scripts supply origin/sizes.
//   Coordinates are UI layout units; no resolution/aspect/viewport is set here.
// - Owned font pairs: CAppFontResource_dtor releases both resource references,
//   then backingResource, then pair storage. Keep that destruction order.
// Source range 0x800A38AC-0x800A7CA4; ABI names and packed pool addresses are retained.
// Pointer/selection domains: players 0..3, item types 0..11 (or -1 where stated),
// category=itemType/4. Public slot APIs document their negative-index fallback.
// UI methods use constructed receivers on the existing game/UI update sequence;
// no thread synchronization is supplied here. Do not confuse item ownership,
// borrowed inventory records, texture handles and transient pixel addresses.
#include "MSL/vector.h"
// Containers whose deleting destructors this unit keeps out of line under plain names. The element
// types are placeholders: the destructor depends only on whether elements are scalars or records.
typedef std::vector<void*, app::app_allocator<void*> > MSLPointerVector;
#include "classes/PartyItemModifiers.h"
#include "game/golfball.h"

// Byte offsets into the existing string pool; retain its storage and encoding.
// PartyItemType preserves constructor/internal names; the glossary calls the
// Fujin fan Wind God Fan. Unverified labels are not promoted to US UI names.
enum {
    kPartyItemLayoutName = 0x13,
    kPartyItemScriptResource = 0x30,
    kPartyAnimateInScript = 0x4D,
    kPartyDefaultScript = 0x50,
    kPartyAnimateOutScript = 0x58,
    kPartyVectorLengthError = 0x5C,
    kPartyLengthErrorType = 0x70,
    kPartyVectorRangeError = 0xA7,
    kPartyRangeErrorType = 0xC5,
    kPartyEquipSound = 0xFC,
};


enum PartyItemType {
    kFireBandage = 0, kSilentMisanga = 1, kLuckyBracelet = 2, kTitanWristband = 3,
    kFujinFan = 4, kRakugakiCrayon = 5, kPululuJarashi = 6, kStampHammer = 7,
    kHurricaneBadge = 8, kFeatherBadge = 9, kTimebombBadge = 10, kUnidentifiedPartyItem = 11
};
enum PartyDisplayState {
    kPartyDisplayLoading = 0, kPartyDisplayReady = 1, kPartyDisplayStartIn = 2,
    kPartyDisplayAnimatingIn = 3, kPartyDisplayActive = 4, kPartyDisplayStartOut = 5,
    kPartyDisplayAnimatingOut = 6, kPartyDisplayHide = 7
};
enum PartyEffectPhase { kEffectApproach = 0, kEffectWait = 1, kEffectPause = 2, kEffectExit = 3 };
enum { kPartyPlayerCount = 4, kPartyItemTypeCount = 12, kPartyItemCategoryCount = 3,
    kPartyItemsPerCategory = 4, kNoPartyItem = -1, kEffectFadeStep = 17,
    kEffectApproachFrames = 30, kEffectExitFrames = 30, kPartyOpacityPulseFrames = 15 };
// Concrete party items share this virtual layout: item type is the second word.
struct PartyItemHeader { void* vtable; int itemType; };

#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "classes/ItemSlotsA522C.h"
#define AsItemSlotsA522C(p) ((ItemSlotsA522C*)(p))
namespace exception_runtime {
extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12length_errorFv(void*, int);
extern const char partyBalloonItemStrings[];
}
}

// Keep the non-returning annotation on the first declaration, before the SDK header.
extern "C" void __throw(const char*, void*, void*) __attribute__((noreturn));
#include "sdk_protos.h"
#include "game/wvector.h"
#include "game/types.h"
#include "game/singletons.h"

// Original source range: 0x800A38AC-0x800A7CA4.
// Behavior entry points: PartyItem_Create selects item classes; PartyItemDisplay_Update
// drives presentation; CBpItemVector_SetItemAtSlot replaces player items. Private
// *_storage.inc fragments hold compiler-sensitive vector/exception support.
// Construct in caller-supplied storage without allocating.
inline void* operator new(size_t, void* p) { return p; }

extern "C" {
void* InputPlayerGuard_dtor(void*, int);

extern void __dl__FPv(void*);
extern void __dla__FPv(void*);
extern void appFreeUni__FPv(void*);
extern void Layout_Destroy(void*, int);
extern void CBpCtrl_DeleteEntryById(void*, int);
extern void CPartyItemFireBantage_Ctor(void*, void*);
extern void CPartyItemSilentMisanga_Ctor(void*, void*);
extern void CPartyItemLuckyBracelet_Ctor(void*, void*);
extern void CPartyItemTitanWristband_Ctor(void*, void*);
extern void CPartyItemFujinUchiwa_Ctor(void*, void*);
extern void CPartyItemRakugakiCrayon_Ctor(void*, void*, void*);
extern void CPartyItemPululuJarashi_Ctor(void*, void*);
extern void CPartyItemStampHammer_Ctor(void*, void*);
extern void CPartyItemHurricaneBadge_Ctor(void*, void*);
extern void CPartyItemFeatherBadge_Ctor(void*, void*);
extern void CPartyItemTimebombBadge_Ctor(void*, void*);
extern void CPartyItemRandomClub_Ctor(void*, void*);

void* party_IItem_dtor(void* p, int deleting);

struct ObjA4028 { void* data; int count; ~ObjA4028(); };
extern "C" void* __dt__8ObjA4028Fv(ObjA4028* p, int deleting);

// Read the number of appearance-effect pointers in the storage view.
inline unsigned int A4028_size(ObjA4028* p) { return *(unsigned int*)&p->count; }
// Reduce the appearance-effect pointer count by the requested amount.
inline void A4028_erase(ObjA4028* p, unsigned int n) { p->count -= n; }
// Clear the appearance-effect count while preserving the original subtraction form.
inline void A4028_clear(ObjA4028* p) { A4028_erase(p, A4028_size(p)); }
// Clear and free appearance-effect pointer storage when allocated.
inline void A4028_destroy4(ObjA4028* p) {
    if (p) {
        if (p->data) {
            A4028_clear(p);
            appFreeUni__FPv(p->data);
        }
    }
}
// Forward the third nested vector-destruction layer, preserving its null check.
inline void A4028_destroy3(ObjA4028* p) { if (p) A4028_destroy4(p); }
// Forward the second nested vector-destruction layer, preserving its null check.
inline void A4028_destroy2(ObjA4028* p) { if (p) A4028_destroy3(p); }
// Forward the outer vector-destruction layer, preserving its null check.
inline void A4028_destroy1(ObjA4028* p) { if (p) A4028_destroy2(p); }

void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);
extern void Layout_GetElementSize(void*, void*, float*);
extern void* __nw__FUl(unsigned int);
extern void CBpTex_Init(void*);
extern void* GolfBall__Fv(void*);
extern void Layout_Init(void*, int);
extern void Layout_InitStandard(void*, int, const char*, int, const char*);
struct ResourceHeader { char storage[0x5c]; inline
    // Initialize the fixed-size layout storage for the requested mode.
    ResourceHeader(int mode) { Layout_Init(this, mode); } };
void __ct__7CBpCtrlFv(void*);
void CBpCtrl_dtor(void*);
extern void CBpCtrl_CreateEntry(void*, const char*, void*, int);
extern signed char s_bpCtrlInitialized;
extern unsigned int g_partyItemAppearanceLayout;
extern const char s_partyItemLayoutDescriptors[];
extern const char s_partyCategoryLayoutDescriptors[];
extern char s_bpCtrlDestructorRegistrationA38AC[];
extern char s_bpCtrl[];
extern unsigned int g_soundManager;
extern const float kFloat_0_5_805DDB48; // Original scalar value: 0.5.
extern const float kFloat_200_805DDB4C; // Original scalar value: 200.
extern float kFloat_1_ItemSlots;
extern const char partyBalloonItemStrings[];
extern const char* partyItemTextureNames[];
extern const char* partyItemCategoryTextureNames[];

void* PartyItem_Create(void* a1, unsigned int type, void* arg, void* extra);

void* PartyItemDisplay_dtor(void* ptr, int deleting);

struct ObjA3B30 {
    int framesRemaining;
    int itemType;
    int playerSlot;
    int animationPhase;
    float x, y, z;
    int primaryTexture, secondaryTexture;
    int opacityByte;
};

extern float kFloat_580_PartyGoodsEffect;
extern const double kDouble_4503601774854144_805DDB60; // Original scalar value: 4503601774854144.
extern float kFloat_150_PartyGoodsEffect;
extern float kPartyAppearanceSecondTargetY;

struct ObjA3CA0 {
    int framesRemaining;
    unsigned char selectionStorage[8];
    int animationPhase;
    float x, y;
    int renderingStorage[3];
    int opacityByte;
};

int PartyGoodsEffect_UpdateAnimation(ObjA3CA0* self, int arg);

struct Obj68_A6218 { void* itemLayout; void* categoryLayout; void* selectionOverlay; int bpEntryId; int displayState; ObjA4028 effects; int effectCapacity; int selectedItems[kPartyPlayerCount]; Obj68_A6218(); };

struct RawObjA3B30 { char storage[0x28]; inline
    // Initialize one BP texture object in its fixed-size storage.
    RawObjA3B30() { CBpTex_Init(this); } };
struct VObjA3B30 { 
virtual void v0(); virtual void load(const char*);
};

ObjA3B30* PartyGoodsEffect_InitAppearance(ObjA3B30* self, int index, int value, void* source);

void party_IItem_vfunc_6();

int party_IItem_vfunc_5();

int party_IItem_vfunc_2();

int party_IItem_vfunc_1();

extern const float kFloat_6_2831855;
extern float s_initializedTwoPi_PartyItemInterface;
extern float s_partyItemAppearanceTwoPi;

void PartyItemInterface_InitAngleConstant();

void PartyItemAppearance_InitAngleConstant();

struct EntryA4B20 {
    // Initialize one appearance effect from item type, player slot and origin.
    EntryA4B20(int index, int value, void* source) {
        PartyGoodsEffect_InitAppearance((ObjA3B30*)this, index, value, source);
    }
    unsigned char data[0x28];
};

struct VecA4B20 {
    unsigned char unk[0x14];
    int* volatile data;
    unsigned int size;
    unsigned int capacity;
};

extern void* appMallocDummy__FUl(unsigned int);
extern void* memcpy(void*, const void*, unsigned int);
extern void* memset(void*, int, unsigned int);
void* __nwa__FUl(unsigned long);
extern void __dt__Q23std12length_errorFv(void*);
extern "C" void __dt__Q23std9exceptionFv(void*, int = 0);
extern void dtor_80046988(void*, int);
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];

extern void Layout_Update(void*);
extern int Layout_PollLoadComplete(void*);
void* IdMap_Lookup(void*, int);
extern int CBpCtrl_PollAllEntriesLoaded(void*);
extern void BpObject_SetEnabled(void*, int);
extern void* BpObject_StartScriptByName(void*, const char*, int);
void* BpObject_GetOrCreateNamedGroup(void*, const char*);
void UIGroup_SetEnabled(void*, int);
extern void BpObject_TryInvokeCurrentCallback(void*, int);
extern int BpObject_IsCurrentScriptComplete(void*);
extern void UIGroup_SetTintColor(void*, int);
extern void PartyItemDisplay_UpdateSlotSelection(void*, int, int);
extern void PartyItemDisplay_UpdateElementPositions(void*);
int PartySelectionOverlay_Update(unsigned char*);
extern int partyItemCategoryTintColors[];

struct PartyItemDisplay { void* layout0; void* layout1; void* stateData; int bpEntryId; int state; void** entries; unsigned int count; unsigned int capacity; int values[4]; };
// Object/view map (all bases are distinct unless stated):
// ItemSlotsA522C / Obj800A6638 / EntryVector_A59E0 overlay the SAME slot manager:
// +0 vector.data,+4 size,+8 capacity,+C selected player,+10 modifiers,+68 display,
// +6C four saved controller words,+7C four owned accessory histories. The
// included *_storage.inc machinery only manages this vector's pointer storage.
// +68 points to a SEPARATE PartyItemDisplay: +0 item layout,+4 category layout,
// +8 owned display state,+C BP id,+10 phase,+14 appearance vector,+20 values[4].
// PartyItemSlots_RefreshDisplay copies item types to display.values. Positioning
// receives the DISPLAY base despite its PartyItemSlots_ name. Its byte cursor
// is display+4*player: reading cursor+20 selects values[player], with no new
// object/lifetime at the shifted address. Layout indices are 12*player+item and
// 3*player+item/4, not indices into the slot manager's vector.
struct PartyItemEntry { void* a; int value; int slot; char pad0[0x10]; void* obj1; void* obj2; };
struct PartyItemDisplayState { unsigned char flag; char pad0[0x07]; int bpEntryId; char pad1[0x18]; int arr[4]; };

// Complete overlay allocation, shared by category display and opacity animation.
// No vtable: +0 is an enable byte; the pulse view's pointer-shaped first word
// is merely an overlapping view of that byte and its padding.
struct PartySelectionStorage {
    signed char enabled; char padding01[3];
    int selectedPlayer, bpEntryId, pulseFrames, fadingOut;
    int unresolved14[4];
    int playerCategories[kPartyPlayerCount];
};
typedef char PartySelectionStorage_size[sizeof(PartySelectionStorage) == 0x34 ? 1 : -1];

// Read the party display's queued appearance-effect count.
inline unsigned int PartyItemDisplay_EffectCount(PartyItemDisplay* p) { return *(unsigned int*)&p->count; }
// Reduce the effect count through the original volatile access.
inline void PartyItemDisplay_EraseEffects(PartyItemDisplay* p, unsigned int n) { *(volatile unsigned int*)&p->count -= n; }
// Clear the party display's queued effect count.
inline void PartyItemDisplay_ClearEffects(PartyItemDisplay* p) { PartyItemDisplay_EraseEffects(p, PartyItemDisplay_EffectCount(p)); }

// Lazily initialize and return the party display's BP controller.
static inline void* PartyItemDisplay_GetBpCtrl() {
    if (!((unsigned int&)Sing_805DC5F0())) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, (void*)s_bpCtrlDestructorRegistrationA38AC);
            s_bpCtrlInitialized = 1;
        }
        ((unsigned int&)Sing_805DC5F0()) = (unsigned int)s_bpCtrl;
    }
    return (void*)((unsigned int&)Sing_805DC5F0());
}

int PartyItemDisplay_Update(PartyItemDisplay* self, void* arg);

// Appearance-effect pointer-vector allocation, growth guards and exception text.
#include "game/u800A38AC_appearance_storage.inc"

void PartyGoodsEffectList_AddAppearance(VecA4B20* v, int index, int value, void* source);

} // extern "C"

extern "C" {

extern char __vt__Q23std12out_of_range[];
extern void __dt__Q23std12out_of_rangeFv(void*);

extern int g_partyItemSlots;
extern signed char s_inputManagerInitialized;
extern char s_gameInputManager[0x100];
extern char s_inputManagerDtorRegistration[0x100];
extern int partyItemPlayerGroupNames[];
extern int partyItemOffsetGroupNames[];
void* PartyItemSlots_dtor(void*, int);
void* PartyItemSlots_ctor(void*);
void InputManager_Init(void*);
void InputManager_Dtor();
void InputManager_SetSelectedPlayer(void*, void*);
void* InputManager_GetSelectedPlayer(void*);
void Layout_SetElementEnabled(void*, int, int);
void UIGroup_GetPosition(void*, float*);
void Layout_SetElementPosition(void*, int, float*);
void CAccState_Ctor(void*, int);
void __dt__Q33std6detail16char_array_guardFv(void*, int = 0);
void* dtor_800A5798(void*, int);

void PartyItemSlots_RebuildEffectParameters(void* self);

struct A5958Type {
    char data[0x8c];
    // Initialize the party-item slot manager in its singleton storage.
    A5958Type() { PartyItemSlots_ctor(this); }
};

void* PartyItemSlots_GetInstance();

// Local standard-exception representations and diagnostic-buffer ownership.
#include "game/u800A38AC_exception_storage.inc"


}

// Party-slot pointer-vector growth, replacement ownership and per-player accessory storage.
#include "game/u800A38AC_slot_storage.inc"


extern "C" {

struct Entry_A59E0 {
    virtual void f8(int);
};

struct A59RangeError_A59E0 : ErrorBase_A59E0 {
    // Construct the range-error identity around copied text.
    A59RangeError_A59E0(const char* p) : ErrorBase_A59E0(p) {
        table = (void*)__vt__Q23std12out_of_range;
        __throw(partyBalloonItemStrings + kPartyRangeErrorType, this, (void*)__dt__Q23std12out_of_rangeFv);
    }
    // Release range-error text through the runtime helper.
    ~A59RangeError_A59E0() { __dt__Q33std6detail16char_array_guardFv(this, 0); }
};

typedef void* volatile A59Elem_A59E0;
// Construct and throw the item-slot vector bounds error.
static inline void A59ThrowRange_A59E0() {
    const char* text;
    A59RangeError_A59E0 error(text = partyBalloonItemStrings + kPartyVectorRangeError);
}

struct EntryVector_A59E0 {
    A59Elem_A59E0* volatile data;
    volatile unsigned int size;
    unsigned int capacity;
    int defIndex;
    // Return a mutable element after checking the recovered vector bounds.
    A59Elem_A59E0& operator[](int i) {
        if ((unsigned int)i >= size) {
            A59ThrowRange_A59E0();
        }
        return data[i];
    }
};

int CBpItemVector_SetItemAtSlot(EntryVector_A59E0* vec, int index, int slot, int oldIndex);
int PartyItemSlots_GetItemType(void* self, int index);

int PartyItemSlots_RefreshDisplay(void* self);

void* InputPlayerGuard_dtor(void* self, int shouldDelete);

void* Object_GetSlotEntry(void* self, int index);

void PartyItemSlots_DestroyInstance();

int PartyItemSlots_SetSelectedPlayer(void* self, int value);

struct BaseCall_A6038 { 
virtual void unk0(); virtual void unk1(); virtual void unk2(); virtual void unk3();
};

int PartyItemSlots_RunLifecycleHook(void* self);

} // extern "C"

extern "C" {

extern const float kFloat_0_805DDB6C; // Original scalar value: 0.
extern char texturePathStrings[];
extern void* CWind_GetInstance(void);
extern void __as__7WVectorFRC7WVector(void*, void*);
extern void PadSampleHistory_RecordFrame(void*);

extern float s_twoPi_800A38AC;
extern char __vt__16CAppFontResource[];
extern const float kFloat_255_805DDB70; // Original scalar value: 255.
extern const float kFloat_15_805DDB74; // Original scalar value: 15.
extern const char partyAppearanceQuadTemplate[];
extern const float kFloat_64_805DDB68; // Original scalar value: 64.
extern int partyPlayerIndicatorNames[];

struct Obj800A6638 {
    void** objects;              /* 0x00 */
    unsigned int count;          /* 0x04 */
    void* capacity;                 /* 0x08 */
    int index;                   /* 0x0C */
    PartyItemModifiers effectParameters; /* 0x10: complete 0x58-byte shared item-effect block */
    char* overlays;              /* 0x68 */
    int savedItemArguments[4];                /* 0x6C */
    void* resources[4];          /* 0x7C */
    void*& at(unsigned int i);
};

// Bounds-check a player slot and return its mutable party-item pointer.
inline void*& Obj800A6638::at(unsigned int i) {
    if (i >= count) {
        const char* msg = partyBalloonItemStrings + kPartyVectorRangeError;
        throw std::out_of_range(msg);
    }
    return objects[i];
}
// Forward the checked player-slot lookup without changing its reference result.
inline void*& PartyItems_SlotRef(Obj800A6638* self, unsigned int i) {
    return self->at(i);
}

int PartyItems_ApplySelection(Obj800A6638* self, void* arg);
void BpObject_Draw(void*);
void Layout_DrawWithOpacity(void*, float);
void* GX_GetPrimitiveCount(int, int, void*);
void DrawPrimitiveUP(int, void*, void*, int);
int gtl_ImageFormat_ToPlatformFormat(int, int);
void Texture_Create2D(int, int, int, int, int, int, int, int);
int gtl_Image_GetImageData(void*, int, int);
void Texture_LockLevel(void*, int, void*, int, int);
void Texture_ConvertRGBA8ToTiledARGBPlanes(void*, void*, int, int, int, int, int, int);
void Texture_UnlockLevel(void*, int);
void* GolfPlayer_GetEntryByIndex(void*, unsigned char);
int Input_IsActionButtonDown(void*, int, void*);
extern int g_sharedDoc;
void UIGroup_SetOpacity(void*, int);
void CubeResourceStorage_dtor(void*, int);
void FontDataView_dtor(void*, int);

struct Obj800A6EB0 { void** items; int count; };
struct Obj800A71C4;
void PartySelectionOverlay_UpdateOpacityPulse(Obj800A71C4*);

struct Obj800A6B9C {
    void** items;
    int count;
    char slotAndEffectStorage[0x60];
    void* overlay;
};

struct Overlay800A6B9C {
    void* itemLayout;
    void* categoryLayout;
    void* selectionOverlay;
    void* key;
    int type;
    void** entries;
    int entryCount;
};

// Immediate quad vertex: position, packed RGBA color, then texture coordinates.
struct Point800A6B9C { float x, y, z; unsigned int color; float u, v; };
// The original draw path reuses the final vertex word for primitive setup.
enum { kQuadScratchWordOffset = sizeof(Point800A6B9C) * 4 - sizeof(float) };

int PartyItemSlots_Draw(Obj800A6B9C* self, void* arg);

int Texture_CreateFromEncodedRgbaImage(int input, int unused, int width, int height, int mipLevels, int layoutFlags, int format, int allocationMode, int p9, int p10, int p11, int p12, int p13, void** result);
void PartyItemSlots_RunFinalLifecycleHook(Obj800A6EB0* self);

struct CAppFontResource { void* vtable; int reserved04; void* backingResource; void** storage; int count; int capacity; };
void* CAppFontResource_dtor(CAppFontResource* self, int shouldDelete);

struct Obj800A71C4 { void* vtable; int selectedPlayer; int bpEntryId; int pulseFrames; int fadingOut; };
typedef char Obj800A71C4_off_bpEntryId[((int)&((Obj800A71C4*)0)->bpEntryId == 0x8) ? 1 : -1];
typedef char Obj68_A6218_off_displayState[((int)&((Obj68_A6218*)0)->displayState == 0x10) ? 1 : -1];

void AppFontTexture_InitAngleConstant();

void CAppFontResource_ctor(CAppFontResource* self);

// Clear and free the image-byte vector through its matching storage view.
inline void ByteVector_ClearAndFree(void* self) {
    if (self) {
        void** fields = (void**)self;
        if (fields[0] != 0) {
            unsigned int* count = (unsigned int*)((char*)self + 4);
            unsigned int n = *count;
            *count = n - *(volatile unsigned int*)&n;
            appFreeUni__FPv(fields[0]);
        }
    }
}
// Forward the nested image-storage destructor, preserving its null check.
inline void ByteVector_DestroyStorage(void* self) { if (self) ByteVector_ClearAndFree(self); }
// Forward the nested image-buffer destructor, preserving its null check.
inline void ByteVector_DestroyBuffer(void* self) { if (self) ByteVector_DestroyStorage(self); }
// Forward the outer image-vector destructor, preserving its null check.
inline void ByteVector_DestroyVector(void* self) { if (self) ByteVector_DestroyBuffer(self); }

void* ByteVector_dtor(void* self, int shouldDelete);

struct ImageInputFile { char data[0x18]; ~ImageInputFile(); };
extern void gtl_CFile_Ctor(ImageInputFile*);
extern void gtl_CFile_Open(ImageInputFile*, void*, int);
extern int gtl_CFile_GetFileSize(ImageInputFile*);
extern void gtl_CFile_Read(ImageInputFile*, void*, unsigned int);
extern void gtl_CFile_dtor(ImageInputFile*, int);
// Close the temporary image-input file without deleting its stack storage.
inline ImageInputFile::~ImageInputFile() { gtl_CFile_dtor(this, -1); }
extern void gtl_Image_GetDescription(void*, void*, void*, unsigned int);

// Return the larger image-buffer growth candidate by reference.
inline const unsigned int& ByteVector_Max(const unsigned int& a, const unsigned int& b) {
    return a < b ? b : a;
}
// Recommend byte-buffer capacity with the original piecewise growth policy.
// Growth: below 0x55555555 elements grow by 3/5 (x1.6); below 0xAAAAAAAA by 1/2 (x1.5); otherwise clamp to the maximum.
inline unsigned int ByteVector_RecommendCapacity(unsigned int cap, unsigned int extra) {
    if (cap < 0x55555555u) return cap + ByteVector_Max(((cap + 1) * 3) / 5, extra);
    if (cap < 0xAAAAAAAAu) return cap + ByteVector_Max((cap + 1) / 2, extra);
    return 0xFFFFFFFFu;
}

// Construct and throw an image-buffer length error in aligned unwind storage.
inline void ByteVector_ThrowLengthError() {
    const char* msg = texturePathStrings;
    char allocator[4];
    {
        union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
        std::length_error* error = (std::length_error*)storage.bytes;
        error->__ct(msg, allocator);
        exception_runtime::__throw(exception_runtime::partyBalloonItemStrings + kPartyLengthErrorType, error,
            (void*)exception_runtime::__dt__Q23std12length_errorFv);
    }
}
// Forward the outer image-buffer length-error throw.
inline void ByteVector_ThrowLengthErrorOuter() {
    ByteVector_ThrowLengthError();
}

void Texture_CreateFromStream(void* self, void* stream, int textureOutputAddress);

} // extern "C"

extern "C" {

// Delete the party-item interface wrapper when requested.
void* party_IItem_dtor(void* object, int deleting) {
    if (object && deleting > 0) __dl__FPv(object);
    return object;
}

// Default party-item apply hook; concrete items supply the effect.
void party_IItem_vfunc_6() {}

// Default party-item readiness hook reports success.
int party_IItem_vfunc_5() { return 1; }

// Default party-item lifecycle hook reports success.
int party_IItem_vfunc_2() { return 1; }

// Default party-item initialization hook reports success.
int party_IItem_vfunc_1() { return 1; }

// Initialize the party-item interface angle constant.
void PartyItemInterface_InitAngleConstant() { s_initializedTwoPi_PartyItemInterface = kFloat_6_2831855; }

// Item factory: the literal sizes passed to operator new are sizeof() of each CPartyItem* class (not verified per class).
// Allocate the selected party-item class, preserving constructor cleanup for the crayon item.
// unusedOwner is unused. playerSlot is an integer 0..3 in a pointer-shaped ABI
// word. Only crayon (5) consumes itemArgument: integer controller index 0..3,
// passed to TrailRibbon_Ctor (party_item_gameplay_hooks.cpp -> trail_ribbon_pointer.cpp) and stored in the misleading
// pointer-shaped controllerIndex ABI field. gtl_InputEx_GetScreenPointer (gtl_input_ex_devices_part2.cpp)
// indexes its controller table with this word on every sample: it is NOT an
// object address or owned device. The controller table must remain initialized;
// disconnect yields a null query, not an out-of-range fallback. Other item types
// ignore this word. CGolfRule's player-list controllerIndex is its producer.
// Unknown type (including unsigned -1) returns null; allocation can return null
// or propagate an exception. Destroy a result via IItem deleting destructor (1).
void* PartyItem_Create(void* unusedOwner, unsigned int itemType, void* playerSlot, void* itemArgument) {
    switch (itemType) {
    case kFireBandage: { void* item = __nw__FUl(0x1c); if (item) CPartyItemFireBantage_Ctor(item, playerSlot); return item; }
    case kSilentMisanga: { void* item = __nw__FUl(0x24); if (item) CPartyItemSilentMisanga_Ctor(item, playerSlot); return item; }
    case kLuckyBracelet: { void* item = __nw__FUl(0x1c); if (item) CPartyItemLuckyBracelet_Ctor(item, playerSlot); return item; }
    case kTitanWristband: { void* item = __nw__FUl(0x20); if (item) CPartyItemTitanWristband_Ctor(item, playerSlot); return item; }
    case kFujinFan: { void* item = __nw__FUl(0xa8); if (item) CPartyItemFujinUchiwa_Ctor(item, playerSlot); return item; }
    case kRakugakiCrayon: { void* item = __nw__FUl(0x14); if (item) {
        try {
            CPartyItemRakugakiCrayon_Ctor(item, playerSlot, itemArgument);
        } catch (...) {
            __dl__FPv(item);
            throw;
        }
    } return item; }
    case kPululuJarashi: { void* item = __nw__FUl(0x18); if (item) CPartyItemPululuJarashi_Ctor(item, playerSlot); return item; }
    case kStampHammer: { void* item = __nw__FUl(0x28); if (item) CPartyItemStampHammer_Ctor(item, playerSlot); return item; }
    case kHurricaneBadge: { void* item = __nw__FUl(0x24); if (item) CPartyItemHurricaneBadge_Ctor(item, playerSlot); return item; }
    case kFeatherBadge: { void* item = __nw__FUl(0x24); if (item) CPartyItemFeatherBadge_Ctor(item, playerSlot); return item; }
    case kTimebombBadge: { void* item = __nw__FUl(0x18); if (item) CPartyItemTimebombBadge_Ctor(item, playerSlot); return item; }
    case kUnidentifiedPartyItem: { void* item = __nw__FUl(0x14); if (item) CPartyItemRandomClub_Ctor(item, playerSlot); return item; }
    }
    return 0;
}

// Initialize the party-item appearance angle constant.
void PartyItemAppearance_InitAngleConstant() { s_partyItemAppearanceTwoPi = kFloat_6_2831855; }

// Create the item/category textures, center their quad and play the appearance sound.
// itemType 0..11 and playerSlot 0..3 are unchecked; originPosition borrows three
// float words copied into the record. Both 0x28-byte BP textures become owned by
// the effect and are deleted with it; load receives borrowed pool paths.
// Returns self. Allocation failures are not handled before virtual texture loading.
ObjA3B30* PartyGoodsEffect_InitAppearance(ObjA3B30* self, int itemType, int playerSlot, void* originPosition) {
    void* unusedItem = 0;
    float soundVolume = kFloat_0_5_805DDB48;
    float soundParameter200 = kFloat_200_805DDB4C;
    float soundPitch = kFloat_1_ItemSlots;
    self->playerSlot = playerSlot;
    self->framesRemaining = kEffectApproachFrames; // Same +0 countdown read through ObjA3CA0::framesRemaining.
    self->itemType = itemType;
    self->animationPhase = 0;
    ((int*)self)[4] = ((int*)originPosition)[0];
    ((int*)self)[5] = ((int*)originPosition)[1];
    ((int*)self)[6] = ((int*)originPosition)[2];
    self->primaryTexture = 0;
    self->secondaryTexture = 0;
    self->opacityByte = 255;
    Sound_PlayByName((void*)g_soundManager, partyBalloonItemStrings, 0, 0, 0, 0, soundVolume, soundParameter200, soundPitch);
    float v[2];
    Layout_GetElementSize((void*)g_partyItemAppearanceLayout, (char*)self->itemType + self->playerSlot * 12, v);
    float vx = v[0];
    float half = kFloat_0_5_805DDB48;
    self->x = self->x - half * vx;
    self->y = self->y - v[1] * half;
    VObjA3B30* q = (VObjA3B30*)new RawObjA3B30;
    self->primaryTexture = (int)q;
    q->load(partyItemTextureNames[self->itemType]);
    q = (VObjA3B30*)new RawObjA3B30;
    self->secondaryTexture = (int)q;
    q->load(partyItemCategoryTextureNames[self->itemType / 4]);
    return self;
}

// Fade and move toward the exit UI target using the already decremented frame count.
// PRECONDITION for finite arithmetic: framesRemaining > 0. The dispatcher only
// rejects negatives; it does not enforce this. Starting an exit update at 1 reaches
// zero and divides both integer opacity and float displacement by zero. Preserve
// this baseline behavior here; callers must end/reset the phase before that update
// if they require defined arithmetic. The 30-frame countdown alone is insufficient.
static inline void UpdatePartyEffectExitPose(ObjA3CA0* self, float& currentX, float& xStepDivisor) {
    int remainingFrames = self->framesRemaining;
    int opacityBeforeStep = self->opacityByte;
// The PowerPC divw gives 0 for a zero divisor.
    int opacityStep = opacityBeforeStep / remainingFrames;
    self->opacityByte = opacityBeforeStep - opacityStep;
    currentX = self->x;
    float remainingX = kFloat_580_PartyGoodsEffect - currentX;
    xStepDivisor = (float)remainingFrames;
    float remainingY = kPartyAppearanceSecondTargetY - self->y;
    float currentY = self->y;
    float yStepDivisor = (float)remainingFrames;
    float stepX = remainingX / xStepDivisor;
    float stepY = remainingY / yStepDivisor;
    self->x = currentX + stepX;
    self->y = currentY + stepY;
}

// Advance appearance/wait/pause/exit; older effects lose 17 opacity units per update.
// self is an owned appearance record viewed through the same 0x28-byte ABI as
// ObjA3B30. isNewestEffect is Boolean: older entries fade but still advance state.
// Return 1 only when Exit's decremented countdown becomes negative; otherwise 0.
// This function takes no delta: increasing update frequency speeds all phases.
// Preserve Approach's strict >0 gate, Wait's ball-state==0 gate, Pause's one-call
// delay, and Exit's distinct <0 gate (including the documented zero-divisor case).
int PartyGoodsEffect_UpdateAnimation(ObjA3CA0* self, int isNewestEffect) {
    if (isNewestEffect == 0) {
        if (self->opacityByte > 0) self->opacityByte -= kEffectFadeStep;
        if (self->opacityByte < 0) self->opacityByte = 0;
    }
    switch (self->animationPhase) {
    case kEffectApproach: { // Move the appearance quad toward its first UI target.
        if (--self->framesRemaining > 0) {
            int t = self->framesRemaining;
            float x = self->x;
            float numX = kFloat_580_PartyGoodsEffect - x;
            float convX = (float)t;
            float numY = kFloat_150_PartyGoodsEffect - self->y;
            float y = self->y;
            float convY = (float)t;
            float divX = numX / convX;
            float divY = numY / convY;
            self->x = x + divX;
            self->y = y + divY;
        } else {
            self->animationPhase = kEffectWait;
        }
        break;
    }
    case kEffectWait: { // Wait for the ball state to return to its reset value (0).
        // CGolfBall_Reset writes m_state=0; no stronger readiness meaning is assumed.
        if (((CGolfBall*)GolfBall__Fv(self))->m_state == 0) {
            self->framesRemaining = 0;
            self->animationPhase = kEffectPause;
        }
        break;
    }
    case kEffectPause: { // Wait one update after ball reset before starting exit.
        int t = --self->framesRemaining;
        if (t < 0) {
            self->framesRemaining = kEffectExitFrames;
            self->animationPhase = kEffectExit;
        }
        break;
    }
    case kEffectExit: { // Fade/move on nonnegative ticks; report complete once negative.
        if (--self->framesRemaining < 0) return 1;
        float x, convX;
        UpdatePartyEffectExitPose(self, x, convX);
        break;
    }
    }
    return 0;
}

// Create the two party-item layouts, script entry and per-player selection overlay.
Obj68_A6218::Obj68_A6218() {
    itemLayout = 0;
    selectionOverlay = 0;
    bpEntryId = -1;
    displayState = 0; effects.data = 0; effects.count = 0; effectCapacity = 0;
    selectedItems[0] = -1; selectedItems[1] = -1; selectedItems[2] = -1; selectedItems[3] = -1;
    void* newObject = new ResourceHeader(5);
    itemLayout = newObject;
    Layout_InitStandard(newObject, 0, s_partyItemLayoutDescriptors, 0x30, partyBalloonItemStrings + kPartyItemLayoutName);
    newObject = new ResourceHeader(5);
    categoryLayout = newObject;
    Layout_InitStandard(newObject, 0, s_partyCategoryLayoutDescriptors, 0x0c, partyBalloonItemStrings + kPartyItemLayoutName);
    g_partyItemAppearanceLayout = (unsigned int)itemLayout;
    if (!((unsigned int&)Sing_805DC5F0())) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, (void*)s_bpCtrlDestructorRegistrationA38AC);
            s_bpCtrlInitialized = 1;
        }
        ((unsigned int&)Sing_805DC5F0()) = (unsigned int)s_bpCtrl;
    }
    CBpCtrl_CreateEntry((void*)((unsigned int&)Sing_805DC5F0()), partyBalloonItemStrings + kPartyItemScriptResource, &bpEntryId, 0);
    newObject = __nw__FUl(0x34);
    if (newObject) {
        PartySelectionStorage* overlay = (PartySelectionStorage*)newObject;
        int scriptEntryId = bpEntryId;
        overlay->enabled = 0; overlay->selectedPlayer = -1;
        overlay->bpEntryId = scriptEntryId; overlay->pulseFrames = kPartyOpacityPulseFrames;
        overlay->fadingOut = 0; overlay->unresolved14[0] = 0;
        overlay->playerCategories[0] = kNoPartyItem; overlay->unresolved14[1] = 0;
        overlay->playerCategories[1] = kNoPartyItem; overlay->unresolved14[2] = 0;
        overlay->playerCategories[2] = kNoPartyItem; overlay->unresolved14[3] = 0;
        overlay->playerCategories[3] = kNoPartyItem;
    }
    selectionOverlay = newObject;
    displayState = 0;
}

// Release the appearance-effect pointer buffer before optional deletion.
extern "C" void* __dt__8ObjA4028Fv(ObjA4028* effects, int deleting) {
    if (effects) {
        A4028_destroy1(effects);
        if (deleting > 0) __dl__FPv(effects);
    }
    return effects;
}

// Destroy layouts, script entry, queued appearance textures and their pointer buffer.
void* PartyItemDisplay_dtor(void* ptr, int deleting) {
    if (ptr) {
        Obj68_A6218* owner = (Obj68_A6218*)ptr;
        g_partyItemAppearanceLayout = 0;
        // Phase 1: release owned layouts and overlay, then unregister the script ID.
        if (owner->itemLayout) { Layout_Destroy(owner->itemLayout, 1); owner->itemLayout = 0; }
        if (owner->categoryLayout) { Layout_Destroy(owner->categoryLayout, 1); owner->categoryLayout = 0; }
        if (owner->selectionOverlay) { __dl__FPv(owner->selectionOverlay); owner->selectionOverlay = 0; }
        if (owner->bpEntryId != -1) {
            if (!((unsigned int&)Sing_805DC5F0())) {
                if (!s_bpCtrlInitialized) {
                    __ct__7CBpCtrlFv((void*)s_bpCtrl);
                    __register_global_object((void*)s_bpCtrl, (void*)CBpCtrl_dtor, (void*)s_bpCtrlDestructorRegistrationA38AC);
                    s_bpCtrlInitialized = 1;
                }
                ((unsigned int&)Sing_805DC5F0()) = (unsigned int)s_bpCtrl;
            }
            CBpCtrl_DeleteEntryById((void*)((unsigned int&)Sing_805DC5F0()), owner->bpEntryId); owner->bpEntryId = -1;
        }
        // Phase 2: textures belong to each effect; effects belong to this list.
        if (owner->effects.count) {
            unsigned int* effectWords = (unsigned int*)owner->effects.data;
            for (unsigned int* cur = effectWords; cur < effectWords + (unsigned int)owner->effects.count; ++cur) {
                ObjA3B30* effect = (ObjA3B30*)*cur;
                if (effect) {
                    if (effect->primaryTexture) { ((void (**)(void*, int))(*(void***)effect->primaryTexture))[2]((void*)effect->primaryTexture, 1); effect->primaryTexture = 0; }
                    if (effect->secondaryTexture) { ((void (**)(void*, int))(*(void***)effect->secondaryTexture))[2]((void*)effect->secondaryTexture, 1); effect->secondaryTexture = 0; }
                    __dl__FPv(effect);
                    *cur = 0;
                }
            }
        }
        // Phase 3: release the pointer array independently of its former elements.
        owner->effects.count = 0;
        if (owner->effects.data) { owner->effects.count = 0; appFreeUni__FPv(owner->effects.data); }
        if (deleting > 0) __dl__FPv(ptr);
    }
    return ptr;
}

#pragma push
#pragma use_lmw_stmw on
enum PartyItemNameGroup { kPartyNamesTintGroup = 16, kPartyNamesGroupB = 20, kPartyNamesGroupC = 24 };
#pragma push
#pragma always_inline on
// Destroy every queued effect and its two owned sprites, retaining pointer capacity.
// Cursor/effect references borrow caller temporaries to preserve register lifetimes.
static inline void DestroyPartyAppearanceBatch(PartyItemDisplay* self, void**& effectCursor, PartyItemEntry*& effect) {
    if (*(volatile unsigned int*)&self->count != 0) {
        effectCursor = self->entries;
        while (effectCursor != self->entries + self->count) {
            effect = (PartyItemEntry*)*effectCursor;
            if (effect) {
                if (effect) {
                    if (effect->obj1) {
                        void* primarySprite = effect->obj1;
                        if (primarySprite) { void** vtable = *(void***)primarySprite; ((void (**)(void*, int))((char*)vtable + 8))[0](primarySprite, 1); }
                        effect->obj1 = 0;
                    }
                    if (effect->obj2) {
                        void* categorySprite = effect->obj2;
                        if (categorySprite) { void** vtable = *(void***)categorySprite; ((void (**)(void*, int))((char*)vtable + 8))[0](categorySprite, 1); }
                        effect->obj2 = 0;
                    }
                    __dl__FPv(effect);
                }
                *effectCursor = 0;
            }
            ++effectCursor;
        }
        PartyItemDisplay_ClearEffects(self);
    }
}
#pragma pop

// Advance the party-display load/show/hide state and commit completed item appearances to player slots.
// Return 1 when any appearance completes and changes a slot, otherwise 0.
// Phase 1 below advances loading/show/hide only. Phase 2 always polls appearance
// effects, even while loading. One completion commits its slot and triggers
// deletion of the ENTIRE appearance list, including unfinished entries.
int PartyItemDisplay_Update(PartyItemDisplay* self, void* unusedContext) {
    const char** names = partyItemTextureNames;
    IdMap_Lookup(PartyItemDisplay_GetBpCtrl(), self->bpEntryId);
    switch (self->state) {
    case kPartyDisplayLoading: {
            Layout_Update(self->layout0);
        Layout_Update(self->layout1);
        int ready = 1;
        if (!Layout_PollLoadComplete(self->layout0)) ready = 0;
        if (!Layout_PollLoadComplete(self->layout1)) ready = 0;
        if (!CBpCtrl_PollAllEntriesLoaded(PartyItemDisplay_GetBpCtrl())) ready = 0;
        if (ready) {
            for (int playerIndex = 0; playerIndex < 4; ++playerIndex) PartyItemDisplay_UpdateSlotSelection(self, self->values[playerIndex], playerIndex);
            self->state = kPartyDisplayReady;
        }
        break;
    }
    case kPartyDisplayReady:
        break;
    // StartIn/StartOut start a BP script and fall through to its first animation tick.
    case kPartyDisplayStartIn: {
        void* scriptObject = IdMap_Lookup(PartyItemDisplay_GetBpCtrl(), self->bpEntryId);
        BpObject_SetEnabled(scriptObject, 1);
        BpObject_StartScriptByName(scriptObject, partyBalloonItemStrings + kPartyAnimateInScript, 1);
            const char** playerGroupNames = (const char**)((kPartyNamesGroupB + names));
        const char** offsetGroupNames = (const char**)(names + kPartyNamesGroupC);
        for (int playerIndex = 0; playerIndex < 4; ++playerIndex) {
            void* playerGroup = BpObject_GetOrCreateNamedGroup(scriptObject, *playerGroupNames);
            UIGroup_SetEnabled(playerGroup, 0);
            if (self->values[playerIndex] == -1) {
                playerGroup = BpObject_GetOrCreateNamedGroup(scriptObject, *offsetGroupNames);
                UIGroup_SetEnabled(playerGroup, 0);
            }
            playerGroupNames++;
            ++offsetGroupNames;
        }
        self->state = kPartyDisplayAnimatingIn;
        /* fall through */
    }
    case kPartyDisplayAnimatingIn: {
        Layout_Update(self->layout0);
        Layout_Update(self->layout1);
        void* scriptObject = IdMap_Lookup(PartyItemDisplay_GetBpCtrl(), self->bpEntryId);
        BpObject_TryInvokeCurrentCallback(scriptObject, 0);
        PartyItemDisplay_UpdateElementPositions(self);
        if (BpObject_IsCurrentScriptComplete(scriptObject)) {
            PartyItemDisplayState* overlayState = (PartyItemDisplayState*)self->stateData;
            void* overlayObject = IdMap_Lookup(PartyItemDisplay_GetBpCtrl(), overlayState->bpEntryId);
            BpObject_StartScriptByName(overlayObject, partyBalloonItemStrings + kPartyDefaultScript, 1);
            int categoryTints[3] = { partyItemCategoryTintColors[0], partyItemCategoryTintColors[1], partyItemCategoryTintColors[2] };
            for (int playerIndex = 0; kPartyPlayerCount > playerIndex; ++playerIndex) {
                void* playerGroup = BpObject_GetOrCreateNamedGroup(overlayObject, names[kPartyNamesTintGroup + playerIndex]);
                if (playerGroup) {
                    UIGroup_SetTintColor(playerGroup, categoryTints[overlayState->arr[playerIndex]]);
                    UIGroup_SetEnabled(playerGroup, 0);
                }
            }
            overlayState->flag = 1;
            self->state = kPartyDisplayActive;
        }
        break;
    }
    case kPartyDisplayActive: {
        Layout_Update(self->layout0);
            Layout_Update(self->layout1);
        void* scriptObject = IdMap_Lookup(PartyItemDisplay_GetBpCtrl(), self->bpEntryId);
        PartySelectionOverlay_Update((unsigned char*)self->stateData);
        BpObject_TryInvokeCurrentCallback(scriptObject, 0);
        break;
    }
    case kPartyDisplayStartOut: {
        void* scriptObject = IdMap_Lookup(PartyItemDisplay_GetBpCtrl(), self->bpEntryId);
        BpObject_SetEnabled(scriptObject, 1);
        BpObject_StartScriptByName(scriptObject, partyBalloonItemStrings + kPartyAnimateOutScript, 1);
            const char** offsetGroupNames = (const char**)(names + kPartyNamesGroupC);
            const char** playerGroupNames = (const char**)((names + kPartyNamesGroupB));
        for (int playerIndex = 0; playerIndex < 4; ++playerIndex) {
            void* playerGroup = BpObject_GetOrCreateNamedGroup(scriptObject, *playerGroupNames);
            UIGroup_SetEnabled(playerGroup, 0);
            if (self->values[playerIndex] == -1) {
                playerGroup = BpObject_GetOrCreateNamedGroup(scriptObject, *offsetGroupNames);
                UIGroup_SetEnabled(playerGroup, 0);
            }
            ++playerGroupNames;
                offsetGroupNames = (1 + offsetGroupNames);
        }
        PartyItemDisplayState* overlayState = (PartyItemDisplayState*)self->stateData;
        void* overlayObject = IdMap_Lookup(PartyItemDisplay_GetBpCtrl(), overlayState->bpEntryId);
            const char** indicatorNames = (const char**)((names + kPartyNamesTintGroup));
        for (int playerIndex = 0; 4 > playerIndex; ++playerIndex) {
            void* playerGroup = BpObject_GetOrCreateNamedGroup(overlayObject, *indicatorNames);
            if (playerGroup) UIGroup_SetEnabled(playerGroup, 0);
            indicatorNames++;
        }
        overlayState->flag = 0;
        self->state = kPartyDisplayAnimatingOut;
        /* fall through */
    }
    // Script completion returns to Active; Hide disables BP then also returns Active.
    case kPartyDisplayAnimatingOut: {
        Layout_Update(self->layout0);
        Layout_Update(self->layout1);
        void* scriptObject = IdMap_Lookup(PartyItemDisplay_GetBpCtrl(), self->bpEntryId);
        BpObject_TryInvokeCurrentCallback(scriptObject, 0);
        PartyItemDisplay_UpdateElementPositions(self);
        if (BpObject_IsCurrentScriptComplete(scriptObject)) self->state = kPartyDisplayActive;
        break;
    }
    case kPartyDisplayHide: {
        void* scriptObject = IdMap_Lookup(PartyItemDisplay_GetBpCtrl(), self->bpEntryId);
        BpObject_SetEnabled(scriptObject, 0);
        self->state = kPartyDisplayActive;
        break;
    }
    default:
        break;
    }

    // Phase 2: poll effects in order; only the final list entry gets isNewestEffect.
    // A completed entry updates type and category for its player slot.
    unsigned int slotsChanged = 0;
    void** entryIt = self->entries;
    while (entryIt != self->entries + self->count) {
        if (*entryIt) { if (PartyGoodsEffect_UpdateAnimation((ObjA3CA0*)*entryIt, entryIt == self->entries + self->count - 1)) {
            PartyItemEntry* effect = (PartyItemEntry*)*entryIt;

            int itemType = effect->value;
                int playerSlot = effect->slot;
                self->values[playerSlot] = itemType;
                PartyItemDisplay_UpdateSlotSelection(self, itemType, playerSlot);
            if (0 != self->state && 1 != self->state) PartyItemDisplay_UpdateElementPositions(self);
            if (itemType == -1) ((PartyItemDisplayState*)self->stateData)->arr[playerSlot] = -1;
            else ((PartyItemDisplayState*)self->stateData)->arr[playerSlot] = itemType / 4;
            slotsChanged = 1;
        } }
        ++entryIt;
    }
    // Phase 3: on any completion, delete both owned sprites of every effect,
    // delete each effect wrapper, and clear count while retaining pointer storage.
    if (slotsChanged) {
        void** effectCursor;
        PartyItemEntry* effect;
        DestroyPartyAppearanceBatch(self, effectCursor, effect);
    }
    return slotsChanged;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Append an appearance effect; required v has size<=capacity, itemType=0..11,
// playerSlot=0..3, originPosition=float[3] in layout space (borrowed/copied).
// Success transfers the wrapper AND its two sprites to the display's batch;
// DestroyPartyAppearanceBatch deletes sprites then wrappers, retaining the array.
// Caller CGolfRule_OnProcess (golf_rule_round_flow_and_shot_sim.cpp) receives no status and supplies
// no recovery owner. Sound/texture construction happens BEFORE capacity checks.
// Failure ledger (baseline behavior, not a strong exception guarantee):
// - If effect construction throws, the new-expression reclaims wrapper storage;
//   EntryA4B20 has no sprite-owning destructor, so already published sprites are
//   not reclaimed here. Sound already played is not rolled back.
// - After construction, entry is a raw local, not an unwind guard. A length-error
//   throw (or a throwing allocator) leaves v unchanged but orphans entry/sprites:
//   neither the caller nor A4B20Buffer can discover/release that unpublished effect.
// - __nw__FUl (heap_alloc_and_operator_new.cpp) may return null. A null effect can be appended; the update
//   loop skips it. Sprite or replacement-buffer null results are used unchecked;
//   they provide no defined failure return, retry, or all-or-nothing guarantee.
// Do not move construction after reserve or add cleanup as a readability edit:
// that would change sound/load side effects and the existing failure behavior.
void PartyGoodsEffectList_AddAppearance(VecA4B20* v, int itemType, int playerSlot, void* originPosition) {
    // Phases: construct the owned effect, append in existing capacity or reserve
    // a new pointer buffer, copy old pointer words, then publish and release old storage.
    EntryA4B20* entry = new EntryA4B20(itemType, playerSlot, originPosition);
    unsigned int size = v->size;

    if (size < v->capacity) {
        unsigned int next = (size + 1);

        v->size = next;
        v->data[next - 1] = (int)entry;
    } else {
        if (0x3FFFFFFFu - v->capacity < 1) { A4B20LengthError error; }
        unsigned int t1, t2;
        unsigned int needed;









        unsigned int observedSize = *(volatile unsigned int*)&v->size;
        unsigned int requestedSize = observedSize + 1;
        needed = requestedSize - (*(volatile unsigned int*)&v->capacity);
        unsigned capacity = v->capacity;
        A4B20Buffer temp(&v->capacity);
        // temp owns ONLY its array allocation; size/offset never confer ownership
        // of effect pointers. On unwind its destructor frees storage, not sprites.
        if (needed > 0x3FFFFFFFu - capacity) A4B20Throw();
        if (capacity < 0x15555555u) {
            t1 = (3 * (capacity + 1)) / 5;
            capacity += A4B20Max(t1, needed);
        } else if (capacity < 0x2AAAAAAAu) {
            t2 = (capacity + 1) / 2;
            capacity += A4B20Max(t2, needed);
        } else capacity = 0x3FFFFFFF;
        if (capacity > 0x3FFFFFFFu) { A4B20LengthError error; }
        int* data = (int*)appMallocDummy__FUl(4 * capacity);

        temp.data = data;
        unsigned int oldSize = v->size; temp.offset = oldSize;

        temp.capacity = capacity;
        ((oldSize + data))[temp.size++] = (int)entry;
        unsigned int previousSize = v->size; int* oldData = v->data;
        int* oldEnd = previousSize + oldData;

        unsigned int count;
        count = ((char*)(oldEnd) - (char*)oldData) / 4;

        unsigned int offset = oldSize - count;
        temp.offset = offset;

        int bytes;
        bytes = (4 * count);
        // Ownership ledger at destructive move: v still owns oldData and all old
        // effects; temp owns data containing the new tail. memcpy transfers words,
        // then memset removes old pointer copies. No throwing C++ work intervenes
        // before publication; memcpy/memset require valid allocations and do not
        // implement rollback. This is serial UI code; observers cannot inspect the
        // temporarily cleared old array or v->size=0 during the swap below.
        memcpy((offset + data), oldData, bytes);
        memset(oldData, 0, bytes);

        oldData = v->data;
        unsigned int oldCapacity = v->capacity;

        int newSize = (temp.size + count);
        temp.size = newSize;
        v->size = 0;
        data = temp.data;
        unsigned int newCapacity = temp.capacity;
        temp.data = oldData;
        v->capacity = newCapacity;
        temp.capacity = (oldCapacity);
        v->data = data;

        v->size = newSize;
        temp.size = 0;
        // Commit complete: v owns data and every effect; temp now owns only the
        // old, zeroed pointer array. Scope exit frees that array exactly once.

    }
}
#pragma pop

// ---- Party-item display and player-slot behavior. ----
// Hide a player's item/category elements, then enable the selected item and category.
void PartyItemDisplay_UpdateSlotSelection(void* self, int itemType, int playerSlot) {
    unsigned int baseOffset = kPartyItemTypeCount * playerSlot;
    int cursor = baseOffset;
    int i = 0;
    for (; i < kPartyItemTypeCount; ++i) {
        Layout_SetElementEnabled(*(void**)self, cursor, 0);
        ++cursor;
    }
    i = 0;
    int base = kPartyItemCategoryCount * playerSlot;
    cursor = base;
    for (; kPartyItemCategoryCount > i; ++i) {
        Layout_SetElementEnabled(*((void**)self + 1), cursor, 0);
        ++cursor;
    }
    if (itemType != -1) {
        int* selectedItem = &((PartyItemDisplay*)self)->values[playerSlot];
        int entryOffset = *selectedItem;
        Layout_SetElementEnabled(*(void**)self, baseOffset + entryOffset, 1);
        entryOffset = *selectedItem;
        Layout_SetElementEnabled(*((void**)self + 1), (entryOffset / kPartyItemsPerCategory) + base, 1);
        Layout_Update(*(void**)self);
        Layout_Update(*((void**)self + 1));
    }
}

// Position each selected item relative to its player slot and script-provided offset.
// Read values[player] through a byte cursor advanced by sizeof(int) per player.
// The cursor retains the display-header bias; it is never a whole PartyItemDisplay.
static inline int PartyDisplayItemAtPlayerCursor(void* cursor) {
    return ((PartyItemDisplay*)cursor)->values[0];
}


// Position each selected item relative to its player slot and the script-provided offset.
void PartyItemDisplay_UpdateElementPositions(void* self) { void** playerItemCursor = (void**)self;
    int* cursorB = partyItemOffsetGroupNames;
    int* cursorA = partyItemPlayerGroupNames;
    unsigned int offset = 0;
    unsigned int y = 0;
    for (int i = 0; kPartyPlayerCount > i; ++i, ++cursorB, ++cursorA, offset += 0x0c, y += 3, ++playerItemCursor) { float b[2];
        if (PartyDisplayItemAtPlayerCursor(playerItemCursor) == -1) continue;

        if (((int&)Sing_805DC5F0()) == 0) {
            if (0 == s_bpCtrlInitialized) {
                __ct__7CBpCtrlFv(&s_bpCtrl);
                __register_global_object(&s_bpCtrl, (void*)CBpCtrl_dtor, &s_bpCtrlDestructorRegistrationA38AC);
                s_bpCtrlInitialized = 1;
            }
            ((int&)Sing_805DC5F0()) = (int)&s_bpCtrl;
        }
        void* node = IdMap_Lookup((void*)((int&)Sing_805DC5F0()), ((PartyItemDisplay*)self)->bpEntryId);
        node = BpObject_GetOrCreateNamedGroup(node, (const char*)*cursorB);
        if (((int&)Sing_805DC5F0()) == 0) {
            if (s_bpCtrlInitialized == 0) {
                __ct__7CBpCtrlFv(&s_bpCtrl);
                __register_global_object(&s_bpCtrl, (void*)CBpCtrl_dtor, &s_bpCtrlDestructorRegistrationA38AC);
                s_bpCtrlInitialized = 1;
            }
            ((int&)Sing_805DC5F0()) = (int)&s_bpCtrl;
        }

        void* other = IdMap_Lookup((void*)((int&)Sing_805DC5F0()), ((PartyItemDisplay*)self)->bpEntryId);
        other = BpObject_GetOrCreateNamedGroup(other, (const char*)*cursorA); float a[2]; UIGroup_GetPosition(node, a);
        UIGroup_GetPosition(other, b);
        b[0] = (b[0] + a[0]);
        b[1] = (b[1] + a[1]);
        Layout_SetElementPosition(*(void**)self, offset + PartyDisplayItemAtPlayerCursor(playerItemCursor), b);
        int categoryElement = PartyDisplayItemAtPlayerCursor(playerItemCursor) / kPartyItemsPerCategory;
        Layout_SetElementPosition(*((void**)self + 1), categoryElement + y, b);
    }
}

#pragma push
#pragma use_lmw_stmw on
// Initialize four player item slots, effect storage and per-player accessory-state objects.
// Storage map: +0x00 vector of four owned item pointers; +0x10..+0x67 effect
// modifier block; +0x6C four player-index words; +0x7C four owned accessory
// objects. display is a separate lazily allocated UI object, not a slot record.
void* PartyItemSlots_ctor(void* self) {
    A522CVec* v = (A522CVec*)self;
    v->data = 0;
    v->size = 0;
    v->capacity = 0;
    AsItemSlotsA522C(self)->selectedIndex = 0;
    (*(unsigned int*)&AsItemSlotsA522C(self)->display) = 0;

    // Phase 1: resize the item-pointer vector to four null entries; growth guard
    // owns replacement storage until existing pointer words have been transferred.
    if (v->size < 4) {
        A522CSize n = 4 - v->size;
        do {
            if (n > v->capacity - v->size || v->size > -n) {
                A522CRecommend(v, n);
            } else {
                memset(v->data + v->size, 0, n * 4);
                v->size += n;
                break;
            }
            {
                A522CSize needed;
                A522CSize size = *(volatile A522CSize*)&v->size;
                A522CSize cap = *(volatile A522CSize*)&v->capacity;
                needed = (size + n) - cap;
                A522CTemp temp = {0, 0, 0, &v->capacity, 0};
                A522CSize capacity = v->capacity;
                A522CSize halfGrowth;
                A522CSize smallGrowth;
                if (needed > 0x3FFFFFFFu - capacity) {
                    const char* text = partyBalloonItemStrings + kPartyVectorLengthError;
                    A522CLengthError error(text);
                }
                if (capacity < 0x15555555u) {
                    smallGrowth = (3 * (capacity + 1)) / 5;
                    capacity += A522CMax(smallGrowth, needed);
                } else if (capacity < 0x2AAAAAAAu) {
                    halfGrowth = (capacity + 1) / 2;
                    capacity += A522CMax(halfGrowth, needed);
                } else {
                    capacity = 0x3FFFFFFF;
                }
                if (capacity > 0x3FFFFFFFu) {
                    const char* text = partyBalloonItemStrings + kPartyVectorLengthError;
                    A522CLengthError error(text);
                }
                int* newData = (int*)appMallocDummy__FUl(capacity * 4);
                int fillSize;
                unsigned oldSize = v->size;
                fillSize = temp.size;
                temp.data = newData;
                temp.capacity = capacity;
                temp.offset = oldSize;
                memset(newData + oldSize + fillSize, 0, n * 4);
                unsigned int count;
                int* oldData = v->data;
                unsigned int byteCount;
                count = ((char*)(oldData + v->size) - (char*)oldData) / 4;
                unsigned insertedSize = temp.size;
                int oldOffset = temp.offset;
                temp.size = insertedSize + n;
                unsigned int offset = oldOffset - count;
                temp.offset = offset;
                byteCount = 4 * count;
                memcpy(temp.data + offset, oldData, byteCount);
                memset(oldData, 0, byteCount);
                unsigned data;
                unsigned newCapacity;
                int zero;
                unsigned newSize;
                unsigned oldCapacity;
                newSize = temp.size + count;
                temp.size = newSize;
                int* old = v->data;
                data = (unsigned)temp.data;
                oldCapacity = v->capacity;
                newCapacity = temp.capacity;
                zero = 0;
                v->size = zero;
                temp.data = old;
                v->capacity = newCapacity;
                temp.capacity = oldCapacity;
                v->data = (int*)data;
                v->size = newSize;
                temp.size = zero;
            }
        } while (false);
    } else if (v->size > 4) {
        v->size = 4;
    }
    for (unsigned int* slot = (unsigned int*)v->data; slot != (unsigned int*)v->data + v->size; ++slot) {
        *slot = 0;
    }
    // Phase 2: zero shared modifiers, then allocate accessory state for each player.
    Memory_Clear(&((Obj800A6638*)self)->effectParameters, sizeof(PartyItemModifiers));
    int off;
    int i = 0;
    off = 0;
    for (; i < 4; ++i) {
        A522CItem* item = new A522CItem(i);
        char* entry = (char*)((unsigned)self + off);
        off += 4;
        *(void**)(entry + 0x7c) = item;
        *(int*)(entry + 0x6c) = i;
    }
    return self;
}
#pragma pop

// Release the item-pointer vector storage before optional deletion.
void* dtor_800A5798(void* self, int shouldDelete) {
    return ::std::__vector_deleting_dtor((MSLPointerVector*)self, shouldDelete);
}

// Destroy active party items, per-player state objects, display and item-pointer storage.
void* PartyItemSlots_dtor(void* self, int shouldDelete) {
    if (self != 0) { int zero = 0; Obj800A6638* owner = (Obj800A6638*)self;

        // Phase 1: destroy owned item objects before releasing the pointer vector.
        if (owner->count != 0) {
            int* slotIt = (int*)owner->objects;
            while (slotIt != (int*)owner->objects + owner->count) {
                void* item = (void*)*slotIt;
                if (item != 0) {
                    if (item != 0) {
                        (*(void (**)(void*, int))((char*)*(void**)item + 8))(item, 1);
                    }
                    *slotIt = zero;
                }
                ++slotIt;
            }
        }
        // Phase 2: each player owns one accessory-state object at +0x7C+4*player.
        // Retain the original cursor bias so load offsets and scheduling stay exact.
        int* playerStateCursor = (int*)self;
        unsigned int playerIndex = 0;
        enum { kPlayerStateWord = (int)&((Obj800A6638*)0)->resources / sizeof(int) };
        owner->count = owner->count - owner->count;
        while (playerIndex < kPartyPlayerCount) {
            void* slot = (void*)playerStateCursor[kPlayerStateWord];
            if (slot != 0) (*(void (**)(void*, int))((char*)*(void**)slot + 8))(slot, 1);
            playerStateCursor[kPlayerStateWord] = zero;
            playerIndex += 1;
            playerStateCursor = (int*)((char*)playerStateCursor + sizeof(void*));
        }
        // Phase 3: destroy the display and then the now-empty item-pointer storage.
        if (owner->overlays != 0) {
            PartyItemDisplay_dtor(owner->overlays, 1);
            owner->overlays = 0;
        }
        // Expanded library destructor guards: preserve their nested address tests.
        if (self != 0) {
            if (self != 0) {
                if (self != 0) {
                    if (self != 0) {
                        if (self != 0) {
                            if (owner->objects != 0) {
                                appFreeUni__FPv(owner->objects);
                                owner->count = owner->count - owner->count;
                            }
                        }
                    }
                }
            }
        }
        if (shouldDelete > 0) __dl__FPv(self);
    }
    return self;
}

// Lazily create the four-player party-item slot manager (legacy CAccState name).
// This differs from the per-player CAccState_Ctor objects allocated by PartyItemSlots_ctor.
void* PartyItemSlots_GetInstance() {
    if (g_partyItemSlots == 0) {
        A5958Type* object = new A5958Type();
        g_partyItemSlots = (int)object;
    }
    return (void*)g_partyItemSlots;
}

// Destroy and clear the shared party-item slot manager.
void PartyItemSlots_DestroyInstance() {
    if (g_partyItemSlots != 0) {
        PartyItemSlots_dtor((void*)g_partyItemSlots, 1);
        g_partyItemSlots = 0;
    }
}

#pragma push
#pragma use_lmw_stmw on
// Destroy/replace a player's Party Goods item, retaining its raw factory argument.
// itemType==-1 returns 0 without replacing; otherwise returns 1 even if the
// factory returns null/unknown type. Old item is destroyed before creation, with
// no rollback. Interface slot 0 receives delete flag 1, then its pointer is cleared.
// requestedPlayerSlot must be 0..3 whenever savedItemArguments is accessed:
// this raw array uses the supplied index even if the item-pointer branch resolves
// a negative index to defIndex. In particular a nonempty replacement still reads
// that raw array when retainedItemArgument==-1. Negative fallback alone supplies
// no array-safety guarantee. The argument word is forwarded opaque to the factory.
// Complete accepted domain: requestedPlayerSlot 0..3 AND <vec->count for a
// replacement; itemType -1 (no replacement) or 0..11. retainedItemArgument=-1
// preserves the prior saved word, otherwise stores it even when itemType==-1.
// For crayon that stored word must be controller index 0..3 with the input
// registry live; for every other type it is ignored by construction. A
// negative requested slot is safe only for the no-op pair itemType=-1 AND
// retainedItemArgument=-1 (neither array access occurs). There is no safe
// negative-index replacement contract, regardless of valid defIndex.
int CBpItemVector_SetItemAtSlot(EntryVector_A59E0* vec, int itemType, int requestedPlayerSlot, int retainedItemArgument) {
    void* factoryScratch;
    void* item;
    // Argument storage uses the raw slot, including negative values; only the item
    // pointer vector resolves negative slots to defIndex. Keep these indices distinct.
    if (retainedItemArgument != kNoPartyItem) {
        ((Obj800A6638*)vec)->savedItemArguments[requestedPlayerSlot] = retainedItemArgument;
    }
    if (itemType == kNoPartyItem) {
        return 0;
    }
    int resolvedPlayerSlot;
    if (requestedPlayerSlot < 0) {
        resolvedPlayerSlot = vec->defIndex;
    } else {
        resolvedPlayerSlot = requestedPlayerSlot;
    }
    if ((*vec)[resolvedPlayerSlot] != 0) {
        item = (*vec)[resolvedPlayerSlot];
        if (item != 0) {
            ((Entry_A59E0*)item)->f8(1);
        }
        (*vec)[resolvedPlayerSlot] = 0;
    }
    (*vec)[resolvedPlayerSlot] = PartyItem_Create(&factoryScratch, itemType, (void*)resolvedPlayerSlot, (void*)((Obj800A6638*)vec)->savedItemArguments[requestedPlayerSlot]);
    return 1;
}
#pragma pop

// Validate and propagate the selected player index to the party display.
int PartyItemSlots_SetSelectedPlayer(void* self, int value) {
    if (value < 0 || value >= 4) {
        return 0;
    }
    int* object = (int*)self;
    object[3] = value;
    int* child = (int*)object[26];
    if (child != 0) {
        int* nested = (int*)child[2];
        if (nested != 0) {
            nested[1] = value;
        }
    }
    return 1;
}

// Return a slot's party-item type, using the selected player for negative indices.
int PartyItemSlots_GetItemType(void* self, int index) {
    unsigned int actual = index < 0 ? AsItemSlotsA522C(self)->selectedIndex : (unsigned int)index;
    if (actual >= AsItemSlotsA522C(self)->entryCount) {
        throw std::out_of_range((const char*)(partyBalloonItemStrings + kPartyVectorRangeError));
    }
    unsigned int offset = actual * 4;
    void* item = *(void**)((char*)AsItemSlotsA522C(self)->entries + offset);
    if (item) {
        if (actual >= AsItemSlotsA522C(self)->entryCount) {
            throw std::out_of_range((const char*)(partyBalloonItemStrings + kPartyVectorRangeError));
        }
        return ((PartyItemHeader*)item)->itemType;
    }
    return -1;
}

// Run each party item's lifecycle hook while restoring the previous input player on exit or exception.
int PartyItemSlots_RunLifecycleHook(void* self) {
    if (0 == ((int&)Sing_805DC5C8())) {
        if (s_inputManagerInitialized == 0) {
            InputManager_Init(&s_gameInputManager);
            __register_global_object(&s_gameInputManager, (void*)InputManager_Dtor, &s_inputManagerDtorRegistration);
            s_inputManagerInitialized = 1;
        }
        ((int&)Sing_805DC5C8()) = (int)&s_gameInputManager;
    }
    void* t = InputManager_GetSelectedPlayer(Sing_805DC5C8());
    void** slotIt = *(void** volatile*)self;
    void* volatile temp = t;
    try {
        for (; slotIt != *(void***)self + *((int*)self + 1); ++slotIt) {
            void* item = *slotIt;
            if (item) ((BaseCall_A6038*)item)->unk1();
        }
    } catch (...) {
        InputPlayerGuard_dtor((void*)&temp, -1);
        throw;
    }
    if (((int&)Sing_805DC5C8()) == 0) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(&s_gameInputManager);
            __register_global_object(&s_gameInputManager, (void*)InputManager_Dtor, &s_inputManagerDtorRegistration);
            s_inputManagerInitialized = 1;
        }
        ((int&)Sing_805DC5C8()) = (int)&s_gameInputManager;
    }
    InputManager_SetSelectedPlayer((void*)((int&)Sing_805DC5C8()), (void*)temp);
    return 1;
}

// Restore the saved input player, including during exception unwinding.
void* InputPlayerGuard_dtor(void* self, int shouldDelete) {
    if (self != 0) {
        if (((int&)Sing_805DC5C8()) == 0) {
            if (!s_inputManagerInitialized) {
                InputManager_Init(&s_gameInputManager);
                __register_global_object(&s_gameInputManager, (void*)InputManager_Dtor, &s_inputManagerDtorRegistration);
                s_inputManagerInitialized = 1;
            }
            ((int&)Sing_805DC5C8()) = (int)&s_gameInputManager;
        }
        InputManager_SetSelectedPlayer((void*)((int&)Sing_805DC5C8()), *(void**)self);
        if (shouldDelete > 0) __dl__FPv(self);
    }
    return self;
}

// Lazily create the display and copy all four selected item types into its UI state.
// Requires four valid vector entries (null means no item). Display values at
// +0x20 are item types; display->stateData +0x24 holds categories type/4, or -1.
// Writes preserve these separate UI representations and do not transfer item ownership.
int PartyItemSlots_RefreshDisplay(void* self) {
    if (!AsItemSlotsA522C(self)->display) {
        Obj68_A6218* newDisplayState = new Obj68_A6218();
        AsItemSlotsA522C(self)->display = newDisplayState;
    }
    int result = 1;
    int byteOffset = 0;
    for (int i = 0; i < 4; ++i, byteOffset += 4) {
        unsigned int index = i < 0 ? AsItemSlotsA522C(self)->selectedIndex : (unsigned int)i;
        if (index >= AsItemSlotsA522C(self)->entryCount) {
            throw std::out_of_range((const char*)(partyBalloonItemStrings + kPartyVectorRangeError));
        }
        void* item = ((void**)AsItemSlotsA522C(self)->entries)[index];
        int itemIndex = item ? ((PartyItemHeader*)item)->itemType : -1;
        void* display = AsItemSlotsA522C(self)->display;
        *(int*)((char*)display + byteOffset + 0x20) = itemIndex;
        PartyItemDisplay_UpdateSlotSelection(display, itemIndex, i);
        int state = ((Obj68_A6218*)display)->displayState;
        if (state != 0 && state != 1) PartyItemDisplay_UpdateElementPositions(display);
        if (itemIndex == -1) *(int*)(*(char**)((char*)display + 8) + byteOffset + 0x24) = -1;
        else *(int*)(*(char**)((char*)display + 8) + byteOffset + 0x24) = (itemIndex + (itemIndex < 0 ? 3 : 0)) >> 2;
    }
    return result;
}

// Rebuild shared item-effect parameters, marking repeated item types and restoring the input player.
void PartyItemSlots_RebuildEffectParameters(void* self) {
    if (((int&)Sing_805DC5C8()) == 0) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(&s_gameInputManager);
            __register_global_object(&s_gameInputManager, (void*)InputManager_Dtor, &s_inputManagerDtorRegistration);
            s_inputManagerInitialized = 1;
        }
        ((int&)Sing_805DC5C8()) = (int)&s_gameInputManager;
    }
    void* volatile previous = InputManager_GetSelectedPlayer(Sing_805DC5C8());
    try {
        Memory_Clear(&((Obj800A6638*)self)->effectParameters, sizeof(PartyItemModifiers));
        ((Obj800A6638*)self)->effectParameters.impactZoneWidthDivisor = kFloat_1_ItemSlots;
        for (void** cur = *(void***)self; cur != *(void***)self + *((int*)(((char*)&AsItemSlotsA522C(self)->entryCount))); ++cur) {
            if (*cur != 0) {
                int found = 0;
                void** c = *(void***)self;
                while (c != cur) {
                    void* candidate = *c;
                    if (candidate != 0 && ((PartyItemHeader*)candidate)->itemType == ((PartyItemHeader*)*cur)->itemType) {
                        found = 1;
                        break;
                    }
                    ++c;
                }
                (*(void (**)(void*, void*, void*, int))(*(int*)*(void**)cur + 0x14))(*cur, &((Obj800A6638*)self)->effectParameters, (*(void**)&AsItemSlotsA522C(self)->selectedIndex), found); // original r6 = found (li r6,0 at 0x800A6530 / li r6,1 at 0x800A6558)
            }
        }
    } catch (...) {
        InputPlayerGuard_dtor((void*)&previous, -1);
        throw;
    }
    if (((int&)Sing_805DC5C8()) == 0) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(&s_gameInputManager);
            __register_global_object(&s_gameInputManager, (void*)InputManager_Dtor, &s_inputManagerDtorRegistration);
            s_inputManagerInitialized = 1;
        }
        ((int&)Sing_805DC5C8()) = (int)&s_gameInputManager;
    }
    InputManager_SetSelectedPlayer((void*)((int&)Sing_805DC5C8()), (void*)previous);
}

// Return the indexed per-player accessory-state object when its array address is valid.
void* Object_GetSlotEntry(void* self, int index) {
    if (((Obj800A6638*)self)->resources != 0) {
        return ((Obj800A6638*)self)->resources[index];
    }
    else {
        return 0;
    }
}

#pragma push
#pragma opt_lifetimes off
#pragma opt_propagation off
// Invoke EVERY active item's apply hook, advance appearances and replace selected items.
// arg is CGolfDoc::gameStateFlags (CGolfRule_OnProcess, golf_rule_round_flow_and_shot_sim.cpp), an integer
// bitmask transported as void*, not borrowed payload memory. Forward unchanged
// to hooks/display. Hook ABI: (item,stateFlags,PartyItemModifiers*,selectedPlayer,
// earlierSameType). Vtable word 6 includes the two-word CW header: it dispatches
// the concrete vfunc_4 implementations in party_item_gameplay_hooks.cpp, not their vfunc_6 stubs.
// Duplicate items ARE invoked: earlierSameType=1 suppresses shared hammer-history
// release, not the whole hook (timebomb/gesture/local state still advances).
// Modifier receiver is this manager+10 (schema PartyItemModifiers.h, 0x58 bytes).
// Per-call reset is ONLY divisor=1, windSpeed=0, tickleActive=0 plus CWind's
// additive XYZ=0. Other fields persist until overwritten or RebuildEffectParameters
// clears the block on player changes. Do not clear it here or skip duplicates.
// Consumer map: golf_rule_round_flow_and_shot_sim.cpp impact classification/assistance uses +08 as fractional
// width gain (1+gain) and +2C as divisor (>0 required); launch/replay velocity Y
// uses +14 as (1+bonus); button gauge reads +0C. ingame_hud_screen.cpp mirrors the impact
// width/divisor and timebomb ticks/period/trigger for HUD/sound. wind_regions_and_vectors.cpp's
// ShotStatOverrides consumes wind magnitude/enable; CSpin_OnProcess (tail_effect_spin_and_bubble.cpp)
// reads +24/+28 when +48 is set (spin/curve ellipse radii in panel units).
// Remaining consumer map, offsets RELATIVE TO modifiers, not self:
// +00 modifier0: GolfParameter_ResolveFloat (club_selection_and_shot_params.cpp) adds distanceYards*rate
//   using FP_FMADDS; +04 affects good/poor impact half-widths in the same unit.
// +10 unk_10: GolfRule_ApplyShotOutcomeModifier (golf_rule_round_flow_and_shot_sim.cpp) multiplies
//   positive awarded delta by 1+rate; it does not multiply Pang or negative delta.
// +18/+44 override wind magnitude/enable and +40 forces unit-length wind
//   (wind_system.cpp/wind_regions_and_vectors.cpp); +1C/+20 are fan-produced speed/heading values, while
//   that hook separately writes CWind additive XYZ (party_item_gameplay_hooks.cpp).
// +30 is ball DISPLAY scale gain (1+gain), CBall_OnDisplay (ball_actor_and_ambient_sound.cpp), not radius
//   or collision geometry. +34 is copied by Titan's owner hook; no reader found.
// +38/+3C/+50 are elapsed ticks/period/trigger: period*18 bounds badge ticks;
//   trigger suspends CClock (club_selection_and_shot_params.cpp), cancels swing (pad_swing_ctrl_and_com_think.cpp) and drives HUD.
// +54 tickleActive selects HUD enter/loop/exit scripts (ingame_hud_screen.cpp).
// All fractional gains are dimensionless. Safe tuning ranges are unestablished;
// do not let a width divisor reach zero or reinterpret a tick count as seconds.
// Input player is saved/restored on normal exit; unlike RebuildEffectParameters
// this routine has no exception restoration guard. All item/history/display
// objects must be live and the initialized manager must contain four slots.
int PartyItems_ApplySelection(Obj800A6638* self, void* arg) {
    // Phases: borrow/save the selected input player, sample all four accessory
    // states, update completed appearances and item effects, then restore input selection.
    float   origin[3];               /* 0x70 */
    void*   volatile lock;           /* 0x1C */
    int     scratch;
    int     i;
    int     slot;
    int     resourceIndex;

    if (!((int&)Sing_805DC5C8())) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(&s_gameInputManager);
            __register_global_object(&s_gameInputManager, (void*)InputManager_Dtor, &s_inputManagerDtorRegistration);
            s_inputManagerInitialized = 1;
        }
        ((int&)Sing_805DC5C8()) = (int)&s_gameInputManager;
    }
    lock = InputManager_GetSelectedPlayer((void*)((int&)Sing_805DC5C8()));
    for (int i = 0; i < 4; ++i) PadSampleHistory_RecordFrame(self->resources[i]);
    {
        float zero = kFloat_0_805DDB6C;
        float one = kFloat_1_ItemSlots;
        self->effectParameters.impactZoneWidthDivisor = one;
        self->effectParameters.windSpeed = zero;
        origin[0] = zero;
        origin[1] = zero;
        origin[2] = zero;
    }
    *(WVector*)((char*)CWind_GetInstance() + 4) = *(WVector*)origin;
    {
        void** it = self->objects;
        self->effectParameters.tickleActive = 0;
        while (it != self->objects + self->count) {
            if (*it != 0) {
                int found = 0;
                void** prev = self->objects;
                while (prev != it) {
                    void* p = *prev;
                    if (p != 0 && ((PartyItemHeader*)p)->itemType == ((PartyItemHeader*)*it)->itemType) {
                        found = 1;
                        break;
                    }
                    prev++;
                }
                void* obj = *it;
                // CW vtable word 6 = concrete vfunc_4 apply hook; duplicate flag is an argument.
                ((void (*)(void*, void*, void*, int, int))(*(void***)obj)[6])
                    (obj, arg, (void*)&self->effectParameters, self->index, found);
            }
            it++;
        }
    }
    if (self->overlays != 0 && PartyItemDisplay_Update((PartyItemDisplay*)self->overlays, arg) != 0) {
        Sound_PlayByName((void*)g_soundManager, (char*)&partyBalloonItemStrings + kPartyEquipSound,
                    0, 0, 0, 0, kFloat_0_5_805DDB48, kFloat_200_805DDB4C, kFloat_1_ItemSlots);
        for (i = 0; i < 4; ++i) {
            resourceIndex = *(int*)(self->overlays + i * 4 + 0x20);
            if (resourceIndex != -1) {
                slot = i < 0 ? self->index : i;
                // Replace the player's item: destroy the old one (IItem vtable slot 2 = deleting destructor), then create the new.
                if (PartyItems_SlotRef(self, slot) != 0) {
                    void* p = PartyItems_SlotRef(self, slot);
                    if (p) ((void (*)(void*, int))(*(void***)p)[2])(p, 1);
                    PartyItems_SlotRef(self, slot) = 0;
                }
                PartyItems_SlotRef(self, slot) = PartyItem_Create(&scratch, resourceIndex, (void*)slot, (void*)self->savedItemArguments[i]);
            }
        }
    }
    if (!((int&)Sing_805DC5C8())) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(&s_gameInputManager);
            __register_global_object(&s_gameInputManager, (void*)InputManager_Dtor, &s_inputManagerDtorRegistration);
            s_inputManagerInitialized = 1;
        }
        ((int&)Sing_805DC5C8()) = (int)&s_gameInputManager;
    }
    InputManager_SetSelectedPlayer((void*)((int&)Sing_805DC5C8()), (void*)lock);
    return 1;
}
#pragma pop

// Draw item effects, visible selection layouts and textured appearance quads, preserving input-player state.
int PartyItemSlots_Draw(Obj800A6B9C* self, void* arg) {
    // Phases: save input selection, draw slot resources and optional display,
    // then restore selection. Temporary player routing does not transfer resources.
    void* volatile lock;
    if (!((int&)Sing_805DC5C8())) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(&s_gameInputManager);
            __register_global_object(&s_gameInputManager, (void*)InputManager_Dtor, &s_inputManagerDtorRegistration);
            s_inputManagerInitialized = 1;
        }
        ((int&)Sing_805DC5C8()) = (int)&s_gameInputManager;
    }
    lock = InputManager_GetSelectedPlayer((void*)((int&)Sing_805DC5C8()));
    try {
        for (void** it = self->items; it != self->items + self->count; ++it)
            if (*it) {
                void** vtable = *(void***)*it;
                ((void (*)(void*, void*))vtable[7])(*it, arg);
            }
        Overlay800A6B9C* overlay = (Overlay800A6B9C*)self->overlay;
        if (overlay) {
            if (overlay->type == 4 || overlay->type == 3 || overlay->type == 6) {
                if (!((int&)Sing_805DC5F0())) {
                    if (!s_bpCtrlInitialized) {
                        __ct__7CBpCtrlFv(&s_bpCtrl);
                        __register_global_object(&s_bpCtrl, (void*)CBpCtrl_dtor, &s_bpCtrlDestructorRegistrationA38AC);
                        s_bpCtrlInitialized = 1;
                    }
                    ((int&)Sing_805DC5F0()) = (int)&s_bpCtrl;
                }
                void* item = IdMap_Lookup((void*)((int&)Sing_805DC5F0()), (int)overlay->key);
                BpObject_Draw(item);
                Layout_DrawWithOpacity(overlay->categoryLayout, kFloat_1_ItemSlots);
                Layout_DrawWithOpacity(overlay->itemLayout, kFloat_1_ItemSlots);
            }
            float f31 = kFloat_64_805DDB68;
            for (void** it = overlay->entries; it != overlay->entries + overlay->entryCount; ++it) {
                unsigned char* data = (unsigned char*)*it;
                if (data) {
                    Point800A6B9C points[4];
                    memcpy(points, partyAppearanceQuadTemplate, sizeof(points));
                    points[0].x = *(float*)(data + 0x10);
                    points[0].y = *(float*)(data + 0x14);
                    ((unsigned char*)&points[0].color)[3] = (unsigned char)*(unsigned int*)(data + 0x24);
                    points[1].x = f31 + *(float*)(data + 0x10);
                    points[1].y = *(float*)(data + 0x14);
                    ((unsigned char*)&points[1].color)[3] = (unsigned char)*(unsigned int*)(data + 0x24);
                    points[2].x = *(float*)(data + 0x10);
                    points[2].y = f31 + *(float*)(data + 0x14);
                    ((unsigned char*)&points[2].color)[3] = (unsigned char)*(unsigned int*)(data + 0x24);
                    points[3].x = f31 + *(float*)(data + 0x10);
                    points[3].y = f31 + *(float*)(data + 0x14);
                    ((unsigned char*)&points[3].color)[3] = (unsigned char)*(unsigned int*)(data + 0x24);
                    void* handle = GX_GetPrimitiveCount(4, 0x98, (void*)((char*)points + kQuadScratchWordOffset));
                    ((void (*)(void*))(*(void***)*(void**)(data + 0x20))[5])(*(void**)(data + 0x20));
                    DrawPrimitiveUP(0x98, handle, &points[0].x, 0x18);
                    ((void (*)(void*))(*(void***)*(void**)(data + 0x1C))[5])(*(void**)(data + 0x1C));
                    DrawPrimitiveUP(0x98, handle, &points[0].x, 0x18);
                }
            }
        }
    } catch (...) {
        InputPlayerGuard_dtor((void*)&lock, -1);
        throw;
    }
    if (!((int&)Sing_805DC5C8())) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(&s_gameInputManager);
            __register_global_object(&s_gameInputManager, (void*)InputManager_Dtor, &s_inputManagerDtorRegistration);
            s_inputManagerInitialized = 1;
        }
        ((int&)Sing_805DC5C8()) = (int)&s_gameInputManager;
    }
    InputManager_SetSelectedPlayer((void*)((int&)Sing_805DC5C8()), (void*)lock);
    return 1;
}

// Run every active item's final lifecycle hook while preserving the input-player state.
void PartyItemSlots_RunFinalLifecycleHook(Obj800A6EB0* self) {
    void* volatile lock;
    if (((int&)Sing_805DC5C8()) == 0) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(&s_gameInputManager);
            __register_global_object(&s_gameInputManager, (void*)InputManager_Dtor, &s_inputManagerDtorRegistration);
            s_inputManagerInitialized = 1;
        }
        ((int&)Sing_805DC5C8()) = (int)&s_gameInputManager;
    }
    void* __r = InputManager_GetSelectedPlayer((void*)((int&)Sing_805DC5C8()));
    void** it = self->items;
    lock = __r;
    try {
        for (; it != self->items + self->count; ++it)
            if (*it) {
                void** vt = *(void***)*it;
                ((void (*)(void*))vt[8])(*it);
            }
    } catch (...) {
        InputPlayerGuard_dtor((void*)&lock, -1);
        throw;
    }
    if (((int&)Sing_805DC5C8()) == 0) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(&s_gameInputManager);
            __register_global_object(&s_gameInputManager, (void*)InputManager_Dtor, &s_inputManagerDtorRegistration);
            s_inputManagerInitialized = 1;
        }
        ((int&)Sing_805DC5C8()) = (int)&s_gameInputManager;
    }
    InputManager_SetSelectedPlayer((void*)((int&)Sing_805DC5C8()), (void*)lock);
}

#pragma push
#pragma use_lmw_stmw on
// Update per-player category visibility and pulse the party-selection indicator opacity.
int PartySelectionOverlay_Update(unsigned char* self) {
    unsigned char* entry;
    if (!*self) return 1;
    if (!((int&)Sing_805DC5F0())) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(&s_bpCtrl);
            __register_global_object(&s_bpCtrl, (void*)CBpCtrl_dtor, &s_bpCtrlDestructorRegistrationA38AC);
            s_bpCtrlInitialized = 1;
        }
        ((int&)Sing_805DC5F0()) = (int)&s_bpCtrl;
    }
    void* list = IdMap_Lookup((void*)((int&)Sing_805DC5F0()), *(int*)(self + 8));
    entry = self;
    void* item;
    int i;
    for (i = 0; i < 4; ++i, entry += 4) {
        item = (void*)BpObject_GetOrCreateNamedGroup(list, (const char*)partyPlayerIndicatorNames[i]);
        if (item) {
            int mode = *(int*)(entry + 0x24);
            switch (mode) {
            case 0:
                if (i == *(int*)(self + 4)) UIGroup_SetEnabled(item, 1);
                else UIGroup_SetEnabled(item, 0);
                break;
            case 1:
                if (i == *(int*)(self + 4)) {
                    UIGroup_SetEnabled(item, 0);
                } else {
                    if (!((int&)Sing_805DC5C8())) {
                        if (!s_inputManagerInitialized) {
                            InputManager_Init(&s_gameInputManager);
                            __register_global_object(&s_gameInputManager, (void*)InputManager_Dtor, &s_inputManagerDtorRegistration);
                            s_inputManagerInitialized = 1;
                        }
                        ((int&)Sing_805DC5C8()) = (int)&s_gameInputManager;
                    }
                    void* cur = (void*)((int&)Sing_805DC5C8());
                    void* p = GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc), (unsigned char)i);
                    if (Input_IsActionButtonDown(cur, 0x15, *(void**)p)) UIGroup_SetEnabled(item, 1);
                    else UIGroup_SetEnabled(item, 0);
                }
                break;
            case 2:
                UIGroup_SetEnabled(item, 1);
                break;
            }
        }
    }
    PartySelectionOverlay_UpdateOpacityPulse((Obj800A71C4*)self);
    return 1;
}
#pragma pop

// Alternate the selection indicator opacity across a 15-frame fade cycle.
void PartySelectionOverlay_UpdateOpacityPulse(Obj800A71C4* self) {
    int result;
    if (self->fadingOut == 0) result = (int)(kFloat_255_805DDB70 - (kFloat_255_805DDB70 * (float)self->pulseFrames) / kFloat_15_805DDB74);
    else result = (int)((kFloat_255_805DDB70 * (float)self->pulseFrames) / kFloat_15_805DDB74);
    self->pulseFrames = self->pulseFrames - 1;
    if (self->pulseFrames <= 0) {
        self->pulseFrames = kPartyOpacityPulseFrames;
        if (!self->fadingOut) self->fadingOut = 1;
        else self->fadingOut = 0;
    }
    if (((int&)Sing_805DC5F0()) == 0) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(&s_bpCtrl);
            __register_global_object(&s_bpCtrl, (void*)CBpCtrl_dtor, &s_bpCtrlDestructorRegistrationA38AC);
            s_bpCtrlInitialized = 1;
        }
        ((int&)Sing_805DC5F0()) = (int)&s_bpCtrl;
    }
    void* item = IdMap_Lookup((void*)((int&)Sing_805DC5F0()), self->bpEntryId);
    for (int i = 0; i < 4; ++i) {
        void* found = (void*)BpObject_GetOrCreateNamedGroup(item, (const char*)partyPlayerIndicatorNames[i]);
        if (found) UIGroup_SetOpacity(found, result);
    }
}

// Initialize the font/texture helper angle constant.
void AppFontTexture_InitAngleConstant() {
    s_twoPi_800A38AC = kFloat_6_2831855;
}

#pragma push
#pragma use_lmw_stmw on
// Borrowed lock result: word +0 is flush pitch in bytes, +4 is destination address.
// Texture_LockLevel (gtl_graphics_texture_buffer_resources.cpp) and the converter implementation (gtl_texture_vertex_formats.cpp)
// agree: converter argument 2 is pitch, argument 4 is destination.
// Words remain integer carriers to preserve this caller's ABI declarations.
// This caller's void*-shaped second converter argument still carries pitch bits.
struct TextureUploadLockWords { int flushPitchBytes, destinationAddress; };
enum TextureUploadConstants {
    kUploadImagePlatformWii = 7, // platform format-table selector, not a GX format
    kUploadSwapRedBlueFormat = 14, // engine RGBA-family ID; swap borrowed R/B bytes
    kUploadPixelBytes = 4, kImageStreamAlignmentBytes = 32,
    kImageStreamOpenMode = 0x10000,
    kUploadSingleMipLevel = 1, kUploadDefaultLayoutFlags = 0,
    kUploadDefaultAllocationMode = 0
};

// Swap R/B bytes in each borrowed four-byte pixel; dimensions are pixels, not a byte pitch.
static inline void SwapDecodedImageRedBlue(unsigned char*& data, int width, int height,
    unsigned int& pixelCount, unsigned int& pixelIndex, unsigned int& pixelByteOffset, unsigned char& savedRed) {
    pixelCount = (unsigned int)width * (unsigned int)height;
    for (pixelIndex = 0; pixelIndex < pixelCount; ++pixelIndex) {
        pixelByteOffset = pixelIndex * kUploadPixelBytes;
        savedRed = data[pixelByteOffset];
        data[pixelByteOffset] = data[pixelByteOffset + 2];
        data[pixelByteOffset + 2] = savedRed;
    }
}

// Create a texture, swap red/blue for format 14, then upload tiled ARGB planes.
// input carries a borrowed ENCODED image-buffer address in the 32-bit ABI;
// gtl_Image_GetImageData probes/locates its pixel data. width/height are pixels. Creation domains
// follow Texture_Create2D (gx_renderer_state_and_resources.cpp): mipLevels is a mip count (0 uses its default),
// layoutFlags/format are engine encodings rather than GX enums, and allocationMode
// is forwarded to the graphics allocator. This unit's stream loader uses 1/0/0
// for mipLevels/layoutFlags/allocationMode; other flag bits are not recovered here.
// unused and p9..p13 are ignored. result is required and receives the texture
// handle for caller management. Return 0 clears result when image data is absent;
// no release of the just-created texture is visible on that failure path.
// Format 14 swaps R/B IN PLACE in borrowed image data before uploading level zero.
// Upload uses four bytes per pixel and tiled AR/GB planes. The converter receives
// lock word 0 as flush pitch and word 1 as destination; neither owns storage.
// Flush range is pitch*height; actual pixel addressing uses width, not pitch.
// Pair lock/convert/unlock even though lock here is cache management, not a mutex.
// Creation/lock failures are unchecked. Texture_Create2D rejects dimensions <4
// without writing result; callers must supply valid dimensions and a writable output.
int Texture_CreateFromEncodedRgbaImage(int input, int unused, int width, int height, int mipLevels, int layoutFlags, int format, int allocationMode, int p9, int p10, int p11, int p12, int p13, void** result) {
    Texture_Create2D(width, height, mipLevels, layoutFlags, gtl_ImageFormat_ToPlatformFormat(format, kUploadImagePlatformWii), allocationMode, (int)result, 0);
    unsigned char* data;
    if (!gtl_Image_GetImageData(&data, input, 0)) {
        *result = 0;
        return 0;
    }
    if (format == kUploadSwapRedBlueFormat) {
        unsigned int pixelCount, pixelIndex, pixelByteOffset;
        unsigned char savedRed;
        SwapDecodedImageRedBlue(data, width, height, pixelCount, pixelIndex, pixelByteOffset, savedRed);
    }
    TextureUploadLockWords lockWords;
    Texture_LockLevel(*result, 0, &lockWords, 0, 0);
    Texture_ConvertRGBA8ToTiledARGBPlanes(data, (void*)lockWords.flushPitchBytes, 0, lockWords.destinationAddress, width, height, 0, kUploadPixelBytes);
    Texture_UnlockLevel(*result, 0);
    return 1;
}
#pragma pop

// Initialize backing-resource and paired font-resource storage.
// self must be fresh storage; clearing a live instance here would lose ownership.
// No allocation or font load occurs; reserved04 deliberately remains untouched.
void CAppFontResource_ctor(CAppFontResource* self) {
    int zero = 0;
    void* vtable = &__vt__16CAppFontResource;
    self->backingResource = 0;
    self->vtable = vtable;
    self->storage = 0;
    self->count = zero;
    self->capacity = zero;
}

// Release paired font references, backing resource and storage before optional deletion.
// count counts PAIRS, not pointer words. High 16 bits are resource references;
// low 16 bits are preserved. A reference reaching zero calls CubeResourceStorage_dtor.
// Nullable self; positive shouldDelete also frees it. The returned address can
// therefore be dangling, and neither it nor the pair buffer should be reused.
void* CAppFontResource_dtor(CAppFontResource* self, int shouldDelete) {
    if (self) {
        self->vtable = &__vt__16CAppFontResource;
        for (void** p = self->storage; p != self->storage + self->count * 2; p += 2) {
            void* obj0 = p[0];
            if (obj0) {
                unsigned int* ref = (unsigned int*)obj0;
                unsigned int val = *ref;
                int n = (int)(val >> 16) - 1;
                *ref = (val & 0xFFFF) | (unsigned int)(n << 16);
                if ((*ref >> 16) == 0) CubeResourceStorage_dtor(obj0, 1);
            }
            void* obj1 = p[1];
            if (obj1) {
                unsigned int* ref = (unsigned int*)obj1;
                unsigned int val = *ref;
                int n = (int)(val >> 16) - 1;
                *ref = (val & 0xFFFF) | (unsigned int)(n << 16);
                if ((*ref >> 16) == 0) CubeResourceStorage_dtor(obj1, 1);
            }
        }
        self->count = 0;
        if (self->backingResource) FontDataView_dtor(self->backingResource, 1);
        if (self->storage) {
            self->count = 0;
            appFreeUni__FPv(self->storage);
        }
        if (shouldDelete > 0) __dl__FPv(self);
    }
    return self;
}

#pragma push
#pragma use_lmw_stmw on
// Read a stream into a 32-byte-rounded buffer, decode its image and create the destination texture.
// textureOutputAddress carries void** in the existing 32-bit ABI, not an image format.
// self is unused; stream is borrowed. The output texture survives temporary-buffer cleanup.
// Output must be a writable void** address in the target ABI. imageInfo is scratch:
// [0] engine format, [1]/[2] dimensions in pixels, [3] mip count, [4] source
// decoder-specific word (GSR writes its source platform selector). This path
// requests one output mip regardless of source count; it does not use word [4].
// scratch[2] receives palette description, which this RGBA-only path does not use.
// Required valid input: open, read and description status are ignored. This function
// cannot report decode failure; the callee may clear output when pixel lookup fails.
// Pixel swapping mutates only the owned temporary stream buffer. Cleanup frees it
// with appFreeUni, then ImageInputFile closes its borrowed input via its destructor.
// Keep rounding/read length, conversion argument order, cache flush and unlock together.
void Texture_CreateFromStream(void* self, void* stream, int textureOutputAddress) {
    // Phases: round the borrowed stream size to 32 bytes, fill an owned buffer,
    // decode image metadata, create/upload the texture, then release the buffer.
    int scratch[2];
    ImageInputFile input;
    int imageInfo[6];
    struct Buffer {
        char* volatile data;
        unsigned int volatile size, cap;
        // Append zero-filled bytes, growing and transferring the owned stream buffer if required.
        void resize(unsigned int oldSize, unsigned int needed) {
            // Phases: zero-fill in available capacity, or reserve replacement bytes,
            // fill the new suffix, transfer old bytes, then swap/free old allocation.
            unsigned int count;
            char* first;
            unsigned int newCap;
            struct Temp { char* data; unsigned int size; unsigned int cap; unsigned int* owner; unsigned int origin; };
            volatile Temp temp;
            unsigned int grow0;
            unsigned int grow1;
            unsigned int extra;
            unsigned int oldCap;
            unsigned int capacity = cap;
            if (needed > capacity || oldSize > capacity - needed) {
                unsigned int total = size + needed;
                oldCap = cap;
                if (total - capacity > 0xFFFFFFFFu - oldCap) {
                    ByteVector_ThrowLengthErrorOuter();
                }
                ByteVector_RecommendCapacity(oldCap, total - capacity);
            } else {
                memset(oldSize + data, 0, needed);
                size += needed;
                return;
            }
            {
                unsigned int total = size + needed;
                oldCap = cap;
                temp.data = 0;
                temp.size = 0;
                temp.cap = 0;
                temp.owner = (unsigned int*)&cap;
                temp.origin = 0;
                extra = total - oldCap;
                if (extra > 0xFFFFFFFFu - oldCap) {
                    ByteVector_ThrowLengthError();
                }

                if (oldCap < 0x55555555u) {
                    grow0 = (((oldCap + 1)) * 3) / 5;
                    newCap = oldCap + *((grow0 < extra) ? &extra : &grow0);
                } else if (oldCap < 0xAAAAAAAAu) {
                    grow1 = ((oldCap + 1)) / 2;
                    newCap = oldCap + *(grow1 < extra ? &extra : &grow1);
                } else newCap = 0xFFFFFFFFu;
                char* storage = (char*)appMallocDummy__FUl(newCap);
                unsigned int origin = size;
                unsigned int current = temp.size;
                temp.data = storage;
                char* base = origin + storage;

                char* fill = current + base;
                temp.origin = origin;
                temp.cap = newCap;
                memset(fill, 0, needed);
                first = data;
                char* last = size + first;

                unsigned int filled = temp.size + needed;
                count = last - first; unsigned int start = temp.origin - count;
                char* dest = temp.data;
                temp.origin = start;
                temp.size = filled;
                memcpy((dest + start), first, count);
                memset(first, 0, count);
                unsigned int newSize = temp.size + count;
                char* old = data;
                char* replacement = temp.data;
                unsigned int previousCap = cap;
                unsigned int replacementCap = temp.cap;
                data = replacement;
                temp.cap = previousCap;
                cap = replacementCap;
                temp.data = old;
                size = newSize;
                temp.size = 0;
                if (old) {
                    temp.size = 0;
                    appFreeUni__FPv(old);
                }
            }
        }
    } buffer;

    unsigned int needed;
    gtl_CFile_Ctor(&input);

    gtl_CFile_Open(&input, stream, kImageStreamOpenMode);
    buffer.data = 0;
    buffer.size = 0;
    buffer.cap = 0;
    int amount = gtl_CFile_GetFileSize(&input);
    int remainder = amount % kImageStreamAlignmentBytes;
    if (0 == remainder) amount = amount; else amount += kImageStreamAlignmentBytes - remainder;
    unsigned int oldSize = *(volatile unsigned int*)&buffer.size;
    if ((unsigned int)amount > oldSize) {
        needed = amount - oldSize;
        buffer.resize(oldSize, needed);
    } else if ((unsigned int)amount < oldSize) {
        unsigned int diff = oldSize - (unsigned int)amount;
        buffer.size = oldSize - *(volatile unsigned int*)&diff;
    }
    gtl_CFile_Read(&input, buffer.data, buffer.size);
    char* data = buffer.data;
    unsigned int size = buffer.size;
    gtl_Image_GetDescription(imageInfo, scratch, data, size);
    ((int (*)(void*, unsigned int, int, int, int, int, int, int, int, int, int, int, int, int))Texture_CreateFromEncodedRgbaImage)
        (data, size, imageInfo[1], imageInfo[2], kUploadSingleMipLevel, kUploadDefaultLayoutFlags, kUploadSwapRedBlueFormat, kUploadDefaultAllocationMode, 0, 0, 0, 0, 0, textureOutputAddress);
    char* cleanup = buffer.data;
    if (cleanup) {
        int n = buffer.size;
        buffer.size = n - *(volatile unsigned int*)&n;
        appFreeUni__FPv(cleanup);
    }
}
#pragma pop

// Release the image byte-vector storage before optional deletion.
void* ByteVector_dtor(void* self, int shouldDelete) {
    if (self) {
        ByteVector_DestroyVector(self);
        if (shouldDelete > 0) __dl__FPv(self);
    }
    return self;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462068", _ctors_u800A38AC_0, PartyItemInterface_InitAngleConstant);
STATIC_INIT_ENTRY(".CRT$XCU80462068", _ctors_u800A38AC_1, PartyItemAppearance_InitAngleConstant);
STATIC_INIT_ENTRY(".CRT$XCU80462068", _ctors_u800A38AC_2, AppFontTexture_InitAngleConstant);
#pragma pop
