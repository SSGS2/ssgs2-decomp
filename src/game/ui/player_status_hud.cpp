// Player status panel: the equipment icons and names, the stat numbers, and the sprite and product-ring
// helpers the panel is drawn with.

#include <decomp/static_init.h>
#include "ppc_double_words.h"
// Shared ABI views for data not fully described by the class headers.
struct PlayerStatusNameView {
    char reserved[0x6F];
    char equipmentName[40];
};
// One character's 0x34-byte equipment selection block in the save payload.
struct CharacterEquipmentSelections {
    short partIndices[24];
    char reserved30;
    signed char auxiliaryPartIndex;
    char reserved32[2];
};
struct SaveCharacterEquipmentView {
    char prefix[0x2CE];
    CharacterEquipmentSelections equipped;
};
struct SaveClubUpgradeView {
    char prefix[0x4AD];
    signed char upgradeLevel;
};
enum SaveEquipmentOffsets {
    kCharacterEquipmentBytes = sizeof(CharacterEquipmentSelections),
    kCharacterPartIndicesOffset = (int)&((SaveCharacterEquipmentView*)0)->equipped.partIndices
};
struct ClubUpgradeStatsView {
    short level, power, control, spin;
};
struct NumberDrawEnableView {
    char reserved[0x28];
    void* enabled;
};
struct QuadBufferView {
    void* buffer;
    int primitiveCount;
};
enum PlayerStatusConstants {
    kStatusNameOffset = (int)&((PlayerStatusNameView*)0)->equipmentName,
    kTextureEntryBytes = 0x28,
    kNoCaddieTextureOffset = 8 * kTextureEntryBytes,
    kStatStorageLimit = 50,
    kControlSpinDisplayLimit = 30,
    kStatPower = 0,
    kStatControl = 1,
    kStatSpin = 3,
    kStatusPowerText = 0x10,
    kStatusControlText = 0x11,
    kStatusSpinText = 0x12
};
enum PlayerStatusStringOffsets {
    kStatusRefreshScriptOffset = 0x1E,
    kStatusNumberGroupOffset = 0x28,
    kStatusEmptyNameMarkerOffset = 0x38,
    kStatusNameGroupOffset = 0x39
};
enum StatusLayoutId {
    kPowerBar = 0,
    kControlBar = 1,
    kSpinBar = 2,
    kFirstEquipmentRect = 3,
    kEquipmentSlotCount = 4
};
enum StatusNameMode {
    kCharacterName = 0,
    kLeaveNameUnchanged = 1,
    kCaddieName = 2
};
enum QuadRenderingConstants {
    kQuadVertexBytes = 0x18,
    kQuadBufferBytes = 0x60,
    kStatusQuadFormat = 0x11F005,
    kDartsQuadFormat = 0x11F001,
    kGxTriangleStrip = 0x98
};
// Format words above are opaque engine vertex-format codes, not GX primitive IDs.
// Status layout: bars 0/1/2 = power/control/spin; rects 3/4/5/6 = character,
// caddie, club set, ball. Text entries 0x10/0x11/0x12 display the same three stats.
// Name mode 1 (and out-of-range values) succeeds without copying a name.

#include "iff/IffStruct.h"
#include "classes/SavePlayerRecord.h" // Slot layout: SaveData_GetCurrentRecord, 0x80073E54
#include "classes/CPlayerStatus.h"
#define AsCPlayerStatus(p) ((CPlayerStatus*)(p))
// Stat bytes at 0x6C..0x6E keep their original char view to preserve signed loads.
// These accessors preserve the byte representation; callers choose signed UI
// interpretation or unsigned modulo-256 accumulation explicitly.
// Borrow the mutable power byte in the status object.
static inline char& StatusPowerByte(void* status) {
    return *(char*)&AsCPlayerStatus(status)->power;
}
// Borrow the mutable control byte in the status object.
static inline char& StatusControlByte(void* status) {
    return *(char*)&AsCPlayerStatus(status)->control;
}
// Borrow the mutable spin byte in the status object.
static inline char& StatusSpinByte(void* status) {
    return *(char*)&AsCPlayerStatus(status)->spin;
}
#include "classes/CSimpleSpd.h"

#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))

#include "game/singletons.h"
#include "sdk_protos.h"
#include "classes/CBpCtrl.h"
#include "classes/CShopProductRing.h"
#include "classes/CSharedFileHandle.h"
#include "classes/CClubCustomTbl.h"
#include "classes/CClubHouseMsg.h"
#include "classes/CFontNumDraw.h"
#include "classes/CBpTex.h"
#include "classes/CSpdCtrl.h"

#define AsCBpCtrl(p) ((CBpCtrl*)(p))
extern "C" {

extern float kFloat_6_2831855;
extern float s_shopProductRingTwoPi;
extern float s_simpleSpdTwoPi;
extern float s_dartsQuadTwoPi;

extern void Layout_Update(void*);
extern void* SpdManager_GetSpriteAt(void*, unsigned int);
extern int Sprite_GetPosition2D(void*, void*);
extern void BpObject_SetEnabled(void*, int);
extern void ShopRingRecordList_Initialize(void*, void*);
extern void ShopRingRecordList_dtor(void*, int);
extern void __dl__FPv(void*);
extern char __vt__16CShopProductRing[];
extern signed char s_bpCtrlInitialized;
extern char s_bpCtrl[];
extern char s_playerStatusBpCtrlDtorRegistration[];
extern char __vt__10CSimpleSpd[];
extern void* Layout_PollLoadComplete(void*);
extern void CBpCtrl_dtor(void);
extern void* Sprite_GetPackedColor(void*, void*);
extern int Sprite_SetPackedColor(void*, unsigned int);
extern void* SimpleSpd_DrawBpSprite(void*);
extern void* g_sharedDoc;
extern int ItemInventory_GetCharacterIdAt(void*, int);
extern void* ItemInventory_FindCharacterData(void*, void*);
extern int ItemInventory_GetCaddieIdAt(void*, int);
extern void* ItemInventory_FindCaddieData(void*, void*);
extern void* SharedFile_Create(void*, int);
extern void* __nw__FUl(unsigned int);
extern int CSharedFilePtr_GetLoadedBuffer(void*);
extern void SpdManager_BuildItemsFromResource(void*, int, void*);
extern int ItemInventory_GetPartIdAt(void*, int);
extern void* ItemInventory_FindPartData(void*, void*);
extern int ItemInventory_GetAuxPartIdAt(void*, int);
extern void* ItemInventory_FindAuxPartData(void*, void*);
extern void UITextTable_SetInteger(void*, int, int);
extern void* BpObject_GetOrCreateNamedGroup(void*, void*);
extern void Layout_SetElementScale(void*, int, float*);
extern void Layout_ReadElementRect(void*, int, void*);
extern void Layout_SetModeByte(void*, int);
extern void BpObject_StartScriptByName(void*, void*, int);
extern char playerStatusResourceStrings[256];
extern char s_playerStatusNumberEntries[];
extern int s_playerStatusScaleXBits;
extern int s_playerStatusScaleYBits;
extern double kPlayerStatusControlSpinBarMaximum;
extern double kPlayerStatusControlSpinBarUnitsPerStat;
extern double kPlayerStatusSignedStatConversionBias;
extern unsigned int kDartsQuadDefaultUMaxBits;
extern unsigned int kDartsQuadDefaultVMaxBits;
extern float kDartsQuadAtlasHalfSpan;
extern float kFloat_0_darts;
extern float kDartsQuadAtlasVSpan;
extern float kFloat_1_0_MenuModel;
extern float kDartsQuadNegativeHalfAxisMultiplier;
extern char kUnitXAxisVector[];
extern char kUnitYAxis[];
extern void Vec3_Scale(void*, void*, float);
extern void Vec3_Add(void*, void*, void*);
extern void Mtx44_TransformPointToVec4(void*, void*, void*);
extern void Mtx44_SetRotationY(void*, float);
extern void gtl_PixelCopy_CopyPackedColor(void*, void*);
extern void VertexBuffer_Create(int, int, int, int, void*, int);
extern void Resource_GetOffsetPtr(void*, int, int, void*, int);
extern void GraphicsBuffer_Flush(void*);
extern void* memcpy(void*, const void*, unsigned int);
extern void BpObject_TryInvokeCurrentCallback(void*, int);
extern void BpObject_Draw(void*);
extern void Layout_DrawWithOpacity(void*, float);
extern void Engine_SetFlexibleVertexFormat(int);
extern int GX_GetPrimitiveCount(int, int);
extern void DrawPrimitiveUP(int, int, void*, int);
extern void UITextTable_Draw(void*);
extern unsigned int s_playerStatusQuadVertexTemplate[];
extern float kPlayerStatusLayoutOpacity;

// Local dispatch views retain the original virtual slot order.
struct ShopProductRingView {
    virtual void reservedSlot1() {} // Preserve unresolved slot 1; local body is a placeholder.
    virtual void reservedSlot2() {} // Preserve unresolved slot 2; local body is a placeholder.
    virtual void draw() {}          // Describe the draw dispatch slot; local body performs no work.
    virtual void isReadyToDraw() {} // Preserve the readiness slot's position/signature.
    virtual void reservedSlot5() {} // Preserve unresolved slot 5; local body is a placeholder.
    virtual void setInitialSelection(void*) {}
    // Describe selection dispatch; actual object supplies implementation.
};
typedef ShopProductRingView ShopRingVirtual;

// Update the status layout and reveal the current equipment-name message.
int CPlayerStatus::Update() {
    Layout_Update(layout);
    ((CClubHouseMsg*)clubHouseMessage)->UpdateReveal();
    return 1;
}

// Local dispatch views retain the original virtual slot order.
struct Fn802E06ECBase {
    virtual void reservedSlot0() {} // Preserve unresolved slot 0 with a no-op placeholder.
    virtual void reservedSlot1() {} // Preserve unresolved slot 1 with a no-op placeholder.
    virtual void reservedSlot2() {} // Preserve unresolved slot 2 with a no-op placeholder.
    virtual void draw() {}          // Describe equipment-texture draw dispatch; actual object supplies the body.
    virtual int isReadyToDraw() {
        return 0;
    }
    // Describe readiness dispatch; placeholder returns false.
};
typedef Fn802E06ECBase EquipmentTextureVirtual;

struct StatusQuadWords {
    unsigned int words[24];
};

// Status keeps four borrowed selected-texture pointers at +0x5c+slot*4 and four
// pixel rectangles {left,top,right,bottom} at +0x0c+slot*16, in the slot order above.
// Immediate vertices are four 24-byte records (six words each): X/Y at words 0/1,
// remaining words copied from the template. Corner order is TL,TR,BL,BR for a strip.

#pragma opt_strength_reduction off
// Draw the script, four selected equipment textures, stat numbers and equipment-name text.
int CPlayerStatus::Draw() {
    float quadVertices[24];
    char* vertexTemplate;
    int i;
    char* textureCursor;
    char* rectCursor;

    BpObject_TryInvokeCurrentCallback(*(void**)&scriptObject, 0);
    BpObject_Draw(*(void**)&scriptObject);
    Layout_DrawWithOpacity(layout, kPlayerStatusLayoutOpacity);
    vertexTemplate = (char*)s_playerStatusQuadVertexTemplate;
    textureCursor = (char*)this;
    rectCursor = (char*)this;
    for (i = 0; i < kEquipmentSlotCount; i++) {
        EquipmentTextureVirtual* obj = *(EquipmentTextureVirtual**)(textureCursor + 0x5C);
        if (obj != 0 && obj->isReadyToDraw()) {
            ((EquipmentTextureVirtual*)*(void**)(textureCursor + 0x5C))->draw();
            *(StatusQuadWords*)quadVertices = *(StatusQuadWords*)vertexTemplate;
            quadVertices[0] = (float)*(int*)(rectCursor + 0xC);
            quadVertices[1] = (float)*(int*)(rectCursor + 0x10);
            quadVertices[6] = (float)*(int*)(rectCursor + 0x14);
            quadVertices[7] = (float)*(int*)(rectCursor + 0x10);
            quadVertices[12] = (float)*(int*)(rectCursor + 0xC);
            quadVertices[13] = (float)*(int*)(rectCursor + 0x18);
            quadVertices[18] = (float)*(int*)(rectCursor + 0x14);
            quadVertices[19] = (float)*(int*)(rectCursor + 0x18);
            Engine_SetFlexibleVertexFormat(kStatusQuadFormat);
            DrawPrimitiveUP(kGxTriangleStrip, GX_GetPrimitiveCount(4, kGxTriangleStrip), quadVertices,
                            kQuadVertexBytes);
        }
        textureCursor += 4;
        rectCursor += 0x10;
    }
    UITextTable_Draw(numberDraw);
    ((CClubHouseMsg*)clubHouseMessage)->DrawText();
    return 1;
}
#pragma opt_strength_reduction reset

// Copy a loaded layout sprite's position; fail if that index has no sprite.
int CSimpleSpd::GetSpritePosition2D(unsigned int index, void* position) {
    void* obj = SpdManager_GetSpriteAt(layout, index);
    if (obj)
        return Sprite_GetPosition2D(obj, position);
    return 0;
}

// Set visibility for the script, numeric stats and message together.
int CPlayerStatus::SetEnabled(int enabled) {
    BpObject_SetEnabled((void*)scriptObject, enabled);
    ((NumberDrawEnableView*)numberDraw)->enabled = (void*)enabled;
    ((CClubHouseMsg*)clubHouseMessage)->SetDrawEnabled(enabled);
    return 1;
}

// Initialize the simple-sprite module's cached full-turn angle.
void SimpleSpd_InitTwoPiCache() {
    s_simpleSpdTwoPi = kFloat_6_2831855;
}

// Initialize the textured-quad module's cached full-turn angle.
void Math_InitTwoPiCache() {
    s_dartsQuadTwoPi = kFloat_6_2831855;
}

// Advance only the status layout animation.
int CPlayerStatus::UpdateLayout() {
    Layout_Update(layout);
    return 1;
}

// Poll the layout, shared B2D entries and message resource before reporting readiness.
int CPlayerStatus::PollResourcesLoaded() {
    void* layout = this->layout;
    if (!Layout_PollLoadComplete(layout))
        return 0;
    if (!((int&)Sing_805DC5F0())) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)&s_bpCtrl)->__ct();
            __register_global_object(&s_bpCtrl, CBpCtrl_dtor, &s_playerStatusBpCtrlDtorRegistration);
            s_bpCtrlInitialized = 1;
        }
        ((int&)Sing_805DC5F0()) = (int)&s_bpCtrl;
    }
    if (!((CBpCtrl*)(((int&)Sing_805DC5F0())))->PollAllEntriesLoaded())
        return 0;
    return ((CClubHouseMsg*)clubHouseMessage)->IsResourceLoaded();
}

// Local dispatch views retain the original virtual slot order.
struct Fn802E13F4Obj {
    virtual void u0() {} // Preserve unresolved slot 0 with a no-op placeholder.
    virtual int method() {
        return 0;
    }
    // Describe sprite readiness dispatch; placeholder returns false.
};
typedef Fn802E13F4Obj SpriteDrawableVirtual;

// Replace only the packed alpha byte and draw the sprite when its virtual readiness check passes.
int CSimpleSpd::ApplyElementOpacity(unsigned int index) {
    void* obj = SpdManager_GetSpriteAt(layout, index);
    if (!obj)
        return 0;
    unsigned int color;
    if (!Sprite_GetPackedColor(obj, &color))
        return 0;
    unsigned int oldColor = color;
    color = (oldColor & 0xFFFFFF00) | opacity;
    if (!Sprite_SetPackedColor(obj, color))
        return 0;
    if (((SpriteDrawableVirtual*)obj)->method())
        return (int)SimpleSpd_DrawBpSprite(obj);
    return 0;
}

typedef CSimpleSpd SimpleSpdDestructionView;
// Local dispatch views retain the original virtual slot order.
struct Fn802E1318Obj {
    virtual void destroy(int) {}
    // Describe owned-object destruction dispatch; local body is a placeholder.
};
typedef Fn802E1318Obj OwnedObjectVirtual;

// Destroy the owned sprite layout, texture and shared file, then optionally free this object.
CSimpleSpd::~CSimpleSpd() {
    SimpleSpdDestructionView* self = (SimpleSpdDestructionView*)this;
    self->vtbl = &__vt__10CSimpleSpd;
    if (self->layout) {
        if (self->layout)
            ((OwnedObjectVirtual*)self->layout)->destroy(1);
        self->layout = 0;
    }
    if (self->texture) {
        if (self->texture)
            ((OwnedObjectVirtual*)self->texture)->destroy(1);
        self->texture = 0;
    }
    if (self->resource) {
        CSharedFileHandle::Destroy(self->resource, 1);
        self->resource = 0;
    }
}

// Set the sprite wrapper's byte opacity for subsequent draws.
void CSimpleSpd::SetOpacity(unsigned char opacity) {
    this->opacity = opacity;
}

// Initialize the product-ring module's cached full-turn angle.
void InitTwoPiCache() {
    s_shopProductRingTwoPi = kFloat_6_2831855;
}

extern int SaveData_HasAuxPart(void*, void*);

// Query whether the save data owns the auxiliary item identified by its IFF header.
int CShopProductRing::IsOwned(void* itemHeader) {
    return SaveData_HasAuxPart(saveData, (void*)((IffItemCommon*)itemHeader)->typeId);
}

// Destroy the product-ring base and optionally free its allocation.
CShopProductRing::~CShopProductRing() {
    ShopRingRecordList_dtor(this, 0);
}

// Initialize product records, install the ring vtable, and dispatch the initial selection.
CShopProductRing::CShopProductRing(void* value, void* initialSelection) {
    ShopRingRecordList_Initialize(this, value);
    vtbl = &__vt__16CShopProductRing;
    ((ShopRingVirtual*)this)->setInitialSelection(initialSelection);
}

// Select character, caddie, club-set and ball icons; absent caddies use the ninth icon.
int CPlayerStatus::UpdateEquipmentTextures() {
    void* data = playerRecord;
    *(char**)&selectedEquipmentTexture =
        *(char**)&equipmentTextures + (AsSavePlayerRecord(data)->characterIndex) * kTextureEntryBytes;
    signed char idx = AsSavePlayerRecord(data)->caddieIndex;
    if (idx != -1) {
        *(char**)&selectedCaddieTexture = *(char**)&caddieTextures + idx * kTextureEntryBytes;
    } else {
        *(char**)&selectedCaddieTexture = *(char**)&caddieTextures + kNoCaddieTextureOffset;
    }
    data = playerRecord;
    *(char**)&selectedClubSetTexture =
        *(char**)&clubSetTextures + (AsSavePlayerRecord(data)->clubSetIndex) * kTextureEntryBytes;
    *(char**)&selectedBallTexture = *(char**)&ballTextures + (AsSavePlayerRecord(data)->ballIndex) * kTextureEntryBytes;
    return 1;
}

// Copy the selected character or caddie IFF name into the fixed status-name buffer.
// Return 0 if the selected ID is missing, 1 otherwise (including no-op modes).
// Copies at most 40 bytes; strncpy does not promise a terminator for longer names.
int CPlayerStatus_CopyCharacterOrCaddieName(void* status, int mode) {
    if (mode == kLeaveNameUnchanged || mode < 0 || mode >= 3)
        return 1;
    if (mode == kCharacterName) {
        void* data = AsCPlayerStatus(status)->playerRecord;
        signed char index = AsSavePlayerRecord(data)->characterIndex;
        void* resource = (void*)ItemInventory_GetCharacterIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, index);
        if (!resource)
            return 0;
        void* text = ItemInventory_FindCharacterData(AsCSharedDoc(g_sharedDoc)->itemInventory, resource);
        strncpy(((PlayerStatusNameView*)status)->equipmentName, (const char*)text, 0x28);
    } else {
        void* data = AsCPlayerStatus(status)->playerRecord;
        signed char index = AsSavePlayerRecord(data)->caddieIndex;
        void* resource = (void*)ItemInventory_GetCaddieIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, index);
        if (!resource)
            return 0;
        void* text = ItemInventory_FindCaddieData(AsCSharedDoc(g_sharedDoc)->itemInventory, resource);
        strncpy(((PlayerStatusNameView*)status)->equipmentName, (const char*)text, 0x28);
    }
    return 1;
}

// Local dispatch views retain the original virtual slot order.
struct BpTexView {
    virtual void reservedSlot1() {} // Preserve unresolved slot 0 with a no-op placeholder.
    virtual void init(void*) {}     // Describe texture initialization dispatch; actual object supplies the body.
    virtual void reservedSlot2() {} // Preserve unresolved slot 2 with a no-op placeholder.
    virtual void draw() {}          // Describe texture draw dispatch; actual object supplies the body.
    virtual bool ready() {
        return true;
    }
    // Describe readiness dispatch; local placeholder returns true.
};
typedef BpTexView TextureResourceVirtual;

struct TextureResourceStorage {
    // Construct a texture wrapper in its complete 0x28-byte storage.
    TextureResourceStorage() {
        ((CBpTex*)this)->__ct();
    }
    char pad[0x28];
};

struct SpriteLayoutStorage {
    // Construct a sprite-layout controller in its complete 0x30-byte storage.
    SpriteLayoutStorage() {
        ((CSpdCtrl*)this)->__ct();
    }
    char pad[0x30];
};

struct SimpleSpdConstructionView {
    void* resource;
    TextureResourceVirtual* texture;
    void* layout;
    unsigned char opacity;
    char pad[3];
    void* vtable;
};

// Load the shared sprite file and texture, build its layout, and start with zero opacity.
// resource/textureResource are borrowed load inputs; self owns the created file
// handle, texture wrapper and layout controller. This constructor busy-waits for
// texture readiness, as in the original. opacity is an unsigned 0..255 byte.
CSimpleSpd::CSimpleSpd(void* resource, void* textureResource) {
    SimpleSpdConstructionView* self = (SimpleSpdConstructionView*)this;
    self->resource = (void*)SharedFile_Create(resource, (self->vtable = &__vt__10CSimpleSpd, 1));
    TextureResourceVirtual* texture = (TextureResourceVirtual*)new TextureResourceStorage;
    self->texture = texture;
    texture->init(textureResource);
    while (!self->texture->ready()) {
    }
    void* layout = new SpriteLayoutStorage;
    self->layout = layout;
    int value = CSharedFilePtr_GetLoadedBuffer(self->resource);
    SpdManager_BuildItemsFromResource(self->layout, value, self->texture);
    self->opacity = 0;
}

extern char s_playerStatusEquipmentTextureNames[];

// Local dispatch views retain the original virtual slot order.
struct Fn802E0EECChild {
    virtual void reservedSlot0() {} // Preserve unresolved slot 0 with a no-op placeholder.
    virtual void init(void*) {}     // Describe equipment texture initialization dispatch.
    virtual void reservedSlot2() {} // Preserve unresolved slot 2 with a no-op placeholder.
    virtual void draw() {}          // Describe equipment texture draw dispatch.
    virtual bool ready() {
        return true;
    }
    // Describe readiness dispatch; local placeholder returns true.
};
typedef Fn802E0EECChild EquipmentTextureLoaderVirtual;

#pragma use_lmw_stmw on
// Initialize each character, caddie, club-set and ball texture from its resource-name table.
int CPlayerStatus::InitEquipmentTextures(void* status) {
    char* self = (char*)status;

    char* base = (char*)s_playerStatusEquipmentTextureNames;
    {
        char* labels = base;
        unsigned int idx = 0;
        unsigned int i = 0;
        for (; i < 8; ++i) {
            ((EquipmentTextureLoaderVirtual*)(*(char**)&AsCPlayerStatus(self)->equipmentTextures + idx))
                ->init(*(void**)labels);
            ((EquipmentTextureLoaderVirtual*)(*(char**)&AsCPlayerStatus(self)->equipmentTextures + idx))->ready();
            idx += kTextureEntryBytes;
            labels += 4;
        }
    }
    {
        char* labels = base + 0x20;
        unsigned int idx = 0;
        unsigned int i = 0;
        for (; i < 9; ++i) {
            ((EquipmentTextureLoaderVirtual*)(*(char**)&AsCPlayerStatus(self)->caddieTextures + idx))
                ->init(*(void**)labels);
            ((EquipmentTextureLoaderVirtual*)(*(char**)&AsCPlayerStatus(self)->caddieTextures + idx))->ready();
            idx += kTextureEntryBytes;
            labels += 4;
        }
    }
    {
        char* labels = base + 0x44;
        unsigned int idx = 0;
        unsigned int i = 0;
        for (; i < 15; ++i) {
            ((EquipmentTextureLoaderVirtual*)(*(char**)&AsCPlayerStatus(self)->clubSetTextures + idx))
                ->init(*(void**)labels);
            ((EquipmentTextureLoaderVirtual*)(*(char**)&AsCPlayerStatus(self)->clubSetTextures + idx))->ready();
            idx += kTextureEntryBytes;
            labels += 4;
        }
    }
    {
        char* labels = base + 0x80;
        unsigned int idx = 0;
        unsigned int i = 0;
        for (; i < 11; ++i) {
            ((EquipmentTextureLoaderVirtual*)(*(char**)&AsCPlayerStatus(self)->ballTextures + idx))
                ->init(*(void**)labels);
            ((EquipmentTextureLoaderVirtual*)(*(char**)&AsCPlayerStatus(self)->ballTextures + idx))->ready();
            idx += kTextureEntryBytes;
            labels += 4;
        }
    }

    return 1;
}
#pragma use_lmw_stmw reset

// Recompute equipped stats, clamp their display, refresh icons and show the selected item name.
// Contributions: character base + 24 parts + auxiliary part + optional caddie
// + club upgrade. Each add truncates to 8 bits; reads for caps/UI sign-extend that byte.
// Part/caddie/upgrade additions use shorts; auxiliary stats use signed bytes.
// Cap is only an upper bound of 50 (no lower clamp). Control/spin numbers cap at
// 30 and their bars scale by 50/30; power uses its raw signed byte for both.
// Pipeline comparison (intentional differences):
//   Refresh: recompute -> name -> cap -> select icons -> scale -> numbers/message.
//   Init: load/register UI -> recompute -> name -> cap -> icons -> scale -> message -> enable.
// Both preserve per-add byte truncation and skip missing table lookups.
// Refresh gates parts/aux on a valid character ID; Init below computes them even
// without that ID. Preserve this difference when editing the duplicated arithmetic.
// nameMode follows StatusNameMode. Missing lookups are skipped; return is always 1.
int CPlayerStatus::RefreshEquipmentDisplay(void* status, int nameMode) {
    float barScale[2];
    struct {
        int reservedWord;
        char scratchPrefix[0x0A];
        char equipmentName[0x28];
        char scratchSuffix[0x28];
    } name; // Only equipmentName is passed to the message API; other scratch roles are unresolved.
    char* self = (char*)status;
    StatusPowerByte(self) = 0;
    StatusControlByte(self) = 0;
    StatusSpinByte(self) = 0;
    Memory_Clear((self + kStatusNameOffset), 0x28);

    char* data = *(char**)&AsCPlayerStatus(self)->playerRecord;

    // Phase 1: seed character stats, then add this character's part/aux contributions.
    {
        void* res = (void*)ItemInventory_GetCharacterIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory,
                                                          AsSavePlayerRecord(data)->characterIndex);
        if (res) {
            IFF_STRUCT::sChar* text =
                (IFF_STRUCT::sChar*)ItemInventory_FindCharacterData(AsCSharedDoc(g_sharedDoc)->itemInventory, res);
            if (text) {
                StatusPowerByte(self) = (char)(int)text->stats[kStatPower];
                StatusControlByte(self) = (char)(int)text->stats[kStatControl];
                StatusSpinByte(self) = (char)(int)text->stats[kStatSpin];
            }
            int partIndex = 0;
            int partIndexBytes = 0;
            for (; partIndex < 24; ++partIndex) {
                char* d = *(char**)&AsCPlayerStatus(self)->playerRecord;
                void* res = (void*)ItemInventory_GetPartIdAt(
                    AsCSharedDoc(g_sharedDoc)->itemInventory,
                    *(short*)((char*)d + (AsSavePlayerRecord(d)->characterIndex) * kCharacterEquipmentBytes +
                              partIndexBytes + kCharacterPartIndicesOffset));
                if (res) {
                    IFF_STRUCT::sPart* text =
                        (IFF_STRUCT::sPart*)ItemInventory_FindPartData(AsCSharedDoc(g_sharedDoc)->itemInventory, res);
                    if (text) {
                        unsigned char powerByte = (unsigned char)StatusPowerByte(self);
                        unsigned char controlByte = (unsigned char)StatusControlByte(self);
                        unsigned char spinByte = (unsigned char)StatusSpinByte(self);
                        powerByte = (unsigned char)(powerByte + (short)text->stats[kStatPower]);
                        StatusPowerByte(self) = (char)powerByte;
                        controlByte = (unsigned char)(controlByte + (short)text->stats[kStatControl]);
                        StatusControlByte(self) = (char)controlByte;
                        spinByte = (unsigned char)(spinByte + (short)text->stats[kStatSpin]);
                        StatusSpinByte(self) = (char)spinByte;
                    }
                }
                partIndexBytes += 2;
            }

            // Add the auxiliary part's signed-byte contributions, retaining byte wraparound.
            {
                char* d = *(char**)&AsCPlayerStatus(self)->playerRecord;
                signed char idx = AsSavePlayerRecord(d)->characterIndex;
                char* entry = d + idx * kCharacterEquipmentBytes;
                signed char auxiliaryPartIndex = ((SaveCharacterEquipmentView*)entry)->equipped.auxiliaryPartIndex;
                void* res =
                    (void*)ItemInventory_GetAuxPartIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, auxiliaryPartIndex);
                if (res) {
                    IFF_STRUCT::sAuxPart* text = (IFF_STRUCT::sAuxPart*)ItemInventory_FindAuxPartData(
                        AsCSharedDoc(g_sharedDoc)->itemInventory, res);
                    if (text) {
                        unsigned int auxiliaryPowerByte = *(volatile unsigned char*)&text->stats[kStatPower];
                        unsigned char powerByte = (unsigned char)StatusPowerByte(self);
                        unsigned char controlByte = (unsigned char)StatusControlByte(self);
                        unsigned char spinByte = (unsigned char)StatusSpinByte(self);
                        powerByte = (unsigned char)(powerByte + (signed char)auxiliaryPowerByte);
                        StatusPowerByte(self) = (char)powerByte;
                        signed char auxiliaryControl = *(volatile signed char*)&text->stats[kStatControl];
                        controlByte = (unsigned char)(controlByte + auxiliaryControl);
                        StatusControlByte(self) = (char)controlByte;
                        signed char auxiliarySpin = *(volatile signed char*)&text->stats[kStatSpin];
                        spinByte = (unsigned char)(spinByte + auxiliarySpin);
                        StatusSpinByte(self) = (char)spinByte;
                    }
                }
            }
        }
    }

    // Phase 2: optional caddie contribution; signed index -1 means no caddie.
    {
        char* d = *(char**)&AsCPlayerStatus(self)->playerRecord;
        signed char idx = AsSavePlayerRecord(d)->caddieIndex;
        if (idx != -1) {
            void* res = (void*)ItemInventory_GetCaddieIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, idx);
            if (res) {
                IFF_STRUCT::sCaddie* text =
                    (IFF_STRUCT::sCaddie*)ItemInventory_FindCaddieData(AsCSharedDoc(g_sharedDoc)->itemInventory, res);
                if (text) {
                    unsigned char powerByte = (unsigned char)StatusPowerByte(self);
                    powerByte = (unsigned char)(powerByte + (short)text->stats[kStatPower]);
                    StatusPowerByte(self) = (char)powerByte;
                    unsigned char controlByte = (unsigned char)StatusControlByte(self);
                    controlByte = (unsigned char)(controlByte + (short)text->stats[kStatControl]);
                    StatusControlByte(self) = (char)controlByte;
                    unsigned char spinByte = (unsigned char)StatusSpinByte(self);
                    spinByte = (unsigned char)(spinByte + (short)text->stats[kStatSpin]);
                    StatusSpinByte(self) = (char)spinByte;
                }
            }
        }
    }

    // Phase 3: club-set upgrade contributes three short stats; upgrade level is signed.
    {
        char* d = *(char**)&AsCPlayerStatus(self)->playerRecord;
        signed char clubSetIndex = AsSavePlayerRecord(d)->clubSetIndex;
        char* entry = d + clubSetIndex * 2;
        unsigned int upgradeLevel = (unsigned char)((SaveClubUpgradeView*)entry)->upgradeLevel;
        void* res = (void*)CClubCustomTbl::FindUpgradeEntry(clubSetIndex, (signed char)upgradeLevel);
        if (res) {
            unsigned char powerByte = (unsigned char)StatusPowerByte(self);
            powerByte = (unsigned char)(powerByte + ((ClubUpgradeStatsView*)res)->power);
            StatusPowerByte(self) = (char)powerByte;
            unsigned char controlByte = (unsigned char)StatusControlByte(self);
            controlByte = (unsigned char)(controlByte + ((ClubUpgradeStatsView*)res)->control);
            StatusControlByte(self) = (char)controlByte;
            unsigned char spinByte = (unsigned char)StatusSpinByte(self);
            spinByte = (unsigned char)(spinByte + ((ClubUpgradeStatsView*)res)->spin);
            StatusSpinByte(self) = (char)spinByte;
        }
    }

#pragma inline_depth(0)
    {
        int (*copySelectedName)(void*, int) = CPlayerStatus_CopyCharacterOrCaddieName;
        copySelectedName(status, nameMode);
    }
#pragma inline_depth(8)

    // Phase 4: upper-cap the signed results; negative wrapped bytes remain negative.
    {
        signed char v = (signed char)StatusPowerByte(self);
        if (v > kStatStorageLimit)
            StatusPowerByte(self) = kStatStorageLimit;
        v = (signed char)StatusControlByte(self);
        if (v > kStatStorageLimit)
            StatusControlByte(self) = kStatStorageLimit;
        v = (signed char)StatusSpinByte(self);
        if (v > kStatStorageLimit)
            StatusSpinByte(self) = kStatStorageLimit;
    }

    // Phase 5: borrow the four selected icons; the ninth caddie texture means absent.
    {
        // Use the shared save-record selections at +0x2C..0x2F.
        *(unsigned int*)&AsCPlayerStatus(self)->selectedEquipmentTexture =
            *(unsigned int*)&AsCPlayerStatus(self)->equipmentTextures +
            (*(SavePlayerRecord**)&AsCPlayerStatus(self)->playerRecord)->characterIndex * kTextureEntryBytes;
        signed char idx = (*(SavePlayerRecord**)&AsCPlayerStatus(self)->playerRecord)->caddieIndex;
        if (idx != -1)
            *(char**)&AsCPlayerStatus(self)->selectedCaddieTexture =
                *(char**)&AsCPlayerStatus(self)->caddieTextures + idx * kTextureEntryBytes;
        else
            *(char**)&AsCPlayerStatus(self)->selectedCaddieTexture =
                *(char**)&AsCPlayerStatus(self)->caddieTextures + kNoCaddieTextureOffset;
    }
    {
        // Use the shared save-record selections at +0x2C..0x2F.
        *(unsigned int*)&AsCPlayerStatus(self)->selectedClubSetTexture =
            *(unsigned int*)&AsCPlayerStatus(self)->clubSetTextures +
            (*(SavePlayerRecord**)&AsCPlayerStatus(self)->playerRecord)->clubSetIndex * kTextureEntryBytes;
        *(unsigned int*)&AsCPlayerStatus(self)->selectedBallTexture =
            *(unsigned int*)&AsCPlayerStatus(self)->ballTextures +
            (*(SavePlayerRecord**)&AsCPlayerStatus(self)->playerRecord)->ballIndex * kTextureEntryBytes;
    }

    // Phase 6: update bar scales. Y scale remains the cached constant; only X changes.
    {
        ((int*)barScale)[0] = s_playerStatusScaleXBits;
        ((int*)barScale)[1] = s_playerStatusScaleYBits;

        // Power bar: signed stat directly controls X scale.
        {
            signed char v = (signed char)StatusPowerByte(self);
            float f = (float)v;
            *(float*)&barScale[0] = f;
            Layout_SetElementScale(AsCPlayerStatus(self)->layout, kPowerBar, barScale);
        }

        // Control bar: values >=30 fill 50 units; smaller signed values multiply by 50/30.
        {
            signed char v = (signed char)StatusControlByte(self);
            double f;
            if (v >= kControlSpinDisplayLimit) {
                f = kPlayerStatusControlSpinBarMaximum;
            } else {
                f = kPlayerStatusControlSpinBarUnitsPerStat * (double)v;
            }
            *(float*)&barScale[0] = f;
            Layout_SetElementScale(AsCPlayerStatus(self)->layout, kControlBar, barScale);
        }

        // Spin bar uses the same 30-to-50 display mapping.
        {
            signed char v = (signed char)StatusSpinByte(self);
            double f;
            if (v >= kControlSpinDisplayLimit) {
                f = kPlayerStatusControlSpinBarMaximum;
            } else {
                f = kPlayerStatusControlSpinBarUnitsPerStat * (double)v;
            }
            *(float*)&barScale[0] = f;
            Layout_SetElementScale(AsCPlayerStatus(self)->layout, kSpinBar, barScale);
        }
    }

    // Phase 7: publish signed numeric values, then reset/rebuild the selected-name message.
    {
        signed char v = (signed char)StatusPowerByte(self);
        UITextTable_SetInteger(AsCPlayerStatus(self)->numberDraw, kStatusPowerText, v);
    }
    {
        UITextTable_SetInteger(AsCPlayerStatus(self)->numberDraw, kStatusControlText,
                               (signed char)StatusControlByte(self) < kControlSpinDisplayLimit
                                   ? (signed char)StatusControlByte(self)
                                   : kControlSpinDisplayLimit);
    }
    {
        UITextTable_SetInteger(AsCPlayerStatus(self)->numberDraw, kStatusSpinText,
                               (signed char)StatusSpinByte(self) < kControlSpinDisplayLimit
                                   ? (signed char)StatusSpinByte(self)
                                   : kControlSpinDisplayLimit);
    }

    ((CClubHouseMsg*)(AsCPlayerStatus(self)->clubHouseMessage))->ResetTextState();

    // Compare one byte against the pooled empty-name marker before creating message text.
    {
        char* selectedName = (self + kStatusNameOffset);
        char* resourceStrings = (char*)&playerStatusResourceStrings;
        char* emptyNameMarker = resourceStrings + kStatusEmptyNameMarkerOffset;
        int nameIsNonempty = strncmp(selectedName, emptyNameMarker, 1);
        if (nameIsNonempty) {
            name.reservedWord = 0;
            Memory_Clear(name.scratchPrefix, 0x0A);
            Memory_Clear(name.equipmentName, 0x28);
            Memory_Clear(name.scratchSuffix, 0x28);
            strncpy(name.equipmentName, (self + kStatusNameOffset), 0x28);
            void* nameGroup = BpObject_GetOrCreateNamedGroup(*(void**)&AsCPlayerStatus(self)->scriptObject,
                                                             resourceStrings + kStatusNameGroupOffset);
            ((CClubHouseMsg*)(AsCPlayerStatus(self)->clubHouseMessage))
                ->SetTextFromGroup(name.equipmentName, nameGroup, 1);
        }
    }

    return 1;
}

// Initialize equipment stats and icons, then scale the three stat bars and populate the name message.
// Uses the contribution/wraparound/UI contract documented on Refresh above, but parts
// and auxiliary contributions are independent of character lookup success here.
int CPlayerStatus::InitEquipmentDisplay(void* status, int nameMode) {
    char* self = (char*)status;

    // Phase 1: start script/layout mode 0, register the three numbers, load icons/message.
    Layout_SetModeByte(AsCPlayerStatus(self)->layout, 0);

    char* resourceStrings = (char*)&playerStatusResourceStrings;
    BpObject_StartScriptByName(*(void**)&AsCPlayerStatus(self)->scriptObject,
                               resourceStrings + kStatusRefreshScriptOffset, 1);
    ((CFontNumDraw*)(AsCPlayerStatus(self)->numberDraw))
        ->RegisterEntries((void*)s_playerStatusNumberEntries, 3, *(void**)&AsCPlayerStatus(self)->scriptObject,
                          resourceStrings + kStatusNumberGroupOffset);

    ((CClubHouseMsg*)(AsCPlayerStatus(self)->clubHouseMessage))->CreateTextFromResource();
    CPlayerStatus::InitEquipmentTextures(status);

    int equipmentSlot = 0;
    char* equipmentRect = self + 0x0C;
    for (; equipmentSlot < kEquipmentSlotCount; ++equipmentSlot) {
        Layout_ReadElementRect(AsCPlayerStatus(self)->layout, equipmentSlot + kFirstEquipmentRect, equipmentRect);
        equipmentRect += 0x10;
    }

    // Phase 2: accumulate base/parts/aux/caddie/upgrade into wrapping byte storage.
    StatusPowerByte(self) = 0;
    StatusControlByte(self) = 0;
    StatusSpinByte(self) = 0;
    Memory_Clear((self + kStatusNameOffset), 0x28);

    {
        char* data = *(char**)&AsCPlayerStatus(self)->playerRecord;
        void* res = (void*)ItemInventory_GetCharacterIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory,
                                                          AsSavePlayerRecord(data)->characterIndex);
        if (res) {
            IFF_STRUCT::sChar* text =
                (IFF_STRUCT::sChar*)ItemInventory_FindCharacterData(AsCSharedDoc(g_sharedDoc)->itemInventory, res);
            if (text) {
                StatusPowerByte(self) = (char)(int)text->stats[kStatPower];
                StatusControlByte(self) = (char)(int)text->stats[kStatControl];
                StatusSpinByte(self) = (char)(int)text->stats[kStatSpin];
            }
        }
    }

    // Add up to 24 equipped parts independently of whether the base character was found.
    {
        int partIndex = 0;
        int partIndexBytes = 0;
        for (; partIndex < 24; ++partIndex) {
            char* d = *(char**)&AsCPlayerStatus(self)->playerRecord;
            void* res = (void*)ItemInventory_GetPartIdAt(
                AsCSharedDoc(g_sharedDoc)->itemInventory,
                *(short*)((char*)d + (AsSavePlayerRecord(d)->characterIndex) * kCharacterEquipmentBytes +
                          partIndexBytes + kCharacterPartIndicesOffset));
            if (res) {
                IFF_STRUCT::sPart* text =
                    (IFF_STRUCT::sPart*)ItemInventory_FindPartData(AsCSharedDoc(g_sharedDoc)->itemInventory, res);
                if (text) {
                    unsigned char powerByte = (unsigned char)StatusPowerByte(self);
                    powerByte = (unsigned char)(powerByte + (short)text->stats[kStatPower]);
                    StatusPowerByte(self) = (char)powerByte;
                    unsigned char controlByte = (unsigned char)StatusControlByte(self);
                    controlByte = (unsigned char)(controlByte + (short)text->stats[kStatControl]);
                    StatusControlByte(self) = (char)controlByte;
                    unsigned char spinByte = (unsigned char)StatusSpinByte(self);
                    spinByte = (unsigned char)(spinByte + (short)text->stats[kStatSpin]);
                    StatusSpinByte(self) = (char)spinByte;
                }
            }
            partIndexBytes += 2;
        }
    }

    // Add auxiliary part stats as signed bytes, with the same per-add byte truncation.
    {
        char* d = *(char**)&AsCPlayerStatus(self)->playerRecord;
        signed char idx = AsSavePlayerRecord(d)->characterIndex;
        char* entry = d + idx * kCharacterEquipmentBytes;
        signed char auxiliaryPartIndex = ((SaveCharacterEquipmentView*)entry)->equipped.auxiliaryPartIndex;
        void* res = (void*)ItemInventory_GetAuxPartIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, auxiliaryPartIndex);
        if (res) {
            IFF_STRUCT::sAuxPart* text =
                (IFF_STRUCT::sAuxPart*)ItemInventory_FindAuxPartData(AsCSharedDoc(g_sharedDoc)->itemInventory, res);
            if (text) {
                unsigned char powerByte = (unsigned char)StatusPowerByte(self);
                powerByte = (unsigned char)(powerByte + *(volatile signed char*)&text->stats[kStatPower]);
                StatusPowerByte(self) = (char)powerByte;
                unsigned char controlByte = (unsigned char)StatusControlByte(self);
                controlByte = (unsigned char)(controlByte + *(volatile signed char*)&text->stats[kStatControl]);
                StatusControlByte(self) = (char)controlByte;
                unsigned char spinByte = (unsigned char)StatusSpinByte(self);
                spinByte = (unsigned char)(spinByte + *(volatile signed char*)&text->stats[kStatSpin]);
                StatusSpinByte(self) = (char)spinByte;
            }
        }
    }

    // Add caddie short stats only when the signed selection index is not -1.
    {
        char* d = *(char**)&AsCPlayerStatus(self)->playerRecord;
        signed char idx = AsSavePlayerRecord(d)->caddieIndex;
        if (idx != -1) {
            void* res = (void*)ItemInventory_GetCaddieIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, idx);
            if (res) {
                IFF_STRUCT::sCaddie* text =
                    (IFF_STRUCT::sCaddie*)ItemInventory_FindCaddieData(AsCSharedDoc(g_sharedDoc)->itemInventory, res);
                if (text) {
                    unsigned char powerByte = (unsigned char)StatusPowerByte(self);
                    powerByte = (unsigned char)(powerByte + (short)text->stats[kStatPower]);
                    StatusPowerByte(self) = (char)powerByte;
                    unsigned char controlByte = (unsigned char)StatusControlByte(self);
                    controlByte = (unsigned char)(controlByte + (short)text->stats[kStatControl]);
                    StatusControlByte(self) = (char)controlByte;
                    unsigned char spinByte = (unsigned char)StatusSpinByte(self);
                    spinByte = (unsigned char)(spinByte + (short)text->stats[kStatSpin]);
                    StatusSpinByte(self) = (char)spinByte;
                }
            }
        }
    }

    // Add club-upgrade short stats using the selected set and signed upgrade level.
    {
        char* d = *(char**)&AsCPlayerStatus(self)->playerRecord;
        signed char clubSetIndex = AsSavePlayerRecord(d)->clubSetIndex;
        char* entry = d + clubSetIndex * 2;
        signed char upgradeLevel = ((SaveClubUpgradeView*)entry)->upgradeLevel;
        void* res = (void*)CClubCustomTbl::FindUpgradeEntry(clubSetIndex, upgradeLevel);
        if (res) {
            unsigned char powerByte = (unsigned char)StatusPowerByte(self);
            powerByte = (unsigned char)(powerByte + ((ClubUpgradeStatsView*)res)->power);
            StatusPowerByte(self) = (char)powerByte;
            unsigned char controlByte = (unsigned char)StatusControlByte(self);
            controlByte = (unsigned char)(controlByte + ((ClubUpgradeStatsView*)res)->control);
            StatusControlByte(self) = (char)controlByte;
            unsigned char spinByte = (unsigned char)StatusSpinByte(self);
            spinByte = (unsigned char)(spinByte + ((ClubUpgradeStatsView*)res)->spin);
            StatusSpinByte(self) = (char)spinByte;
        }
    }

#pragma inline_depth(0)
    {
        int (*copySelectedName)(void*, int) = CPlayerStatus_CopyCharacterOrCaddieName;
        copySelectedName(status, nameMode);
    }
#pragma inline_depth(8)

    // Phase 3: apply only the signed upper cap; byte wraparound/negative values persist.
    {
        signed char v = (signed char)StatusPowerByte(self);
        if (v > kStatStorageLimit)
            StatusPowerByte(self) = kStatStorageLimit;
        v = (signed char)StatusControlByte(self);
        if (v > kStatStorageLimit)
            StatusControlByte(self) = kStatStorageLimit;
        v = (signed char)StatusSpinByte(self);
        if (v > kStatStorageLimit)
            StatusSpinByte(self) = kStatStorageLimit;
    }

    // Phase 4: select borrowed icon pointers in character/caddie/club-set/ball order.
    {
        char* d = *(char**)&AsCPlayerStatus(self)->playerRecord;
        char* characterTextures = *(char**)&AsCPlayerStatus(self)->equipmentTextures;
        *(char**)&AsCPlayerStatus(self)->selectedEquipmentTexture =
            characterTextures + (AsSavePlayerRecord(d)->characterIndex) * kTextureEntryBytes;
        char* caddieTextures = *(char**)&AsCPlayerStatus(self)->caddieTextures;
        signed char idx = AsSavePlayerRecord(d)->caddieIndex;
        *(char**)&AsCPlayerStatus(self)->selectedCaddieTexture =
            (idx != -1) ? caddieTextures + idx * kTextureEntryBytes : caddieTextures + kNoCaddieTextureOffset;
        char* clubSetTextures = *(char**)&AsCPlayerStatus(self)->clubSetTextures;
        signed char clubSetIndex = AsSavePlayerRecord(d)->clubSetIndex;
        *(char**)&AsCPlayerStatus(self)->selectedClubSetTexture = clubSetTextures + clubSetIndex * kTextureEntryBytes;
        char* ballTextures = *(char**)&AsCPlayerStatus(self)->ballTextures;
        signed char ballIndex = AsSavePlayerRecord(d)->ballIndex;
        *(char**)&AsCPlayerStatus(self)->selectedBallTexture = ballTextures + ballIndex * kTextureEntryBytes;
    }

    // Phase 5: bar scales use signed byte values; control/spin saturate at 30 -> 50.
    // The 0x43300000/0x80000000 double construction implements signed-int conversion.
    {
        int barScale[2];
        barScale[0] = s_playerStatusScaleXBits;
        barScale[1] = s_playerStatusScaleYBits;

        void* obj = AsCPlayerStatus(self)->layout;

        {
            signed char v = (signed char)StatusPowerByte(self);
            union {
                unsigned int i[2];
                double d;
            } u;
            u.i[PPC_DW_HI] = 0x43300000u;
            u.i[PPC_DW_LO] = (unsigned int)v ^ 0x80000000u;
            float f = (float)(u.d - kPlayerStatusSignedStatConversionBias);
            *(float*)&barScale[0] = f;
            Layout_SetElementScale(obj, kPowerBar, (float*)barScale);
        }

        {
            signed char v = (signed char)StatusControlByte(self);
            float f;
            if (v >= kControlSpinDisplayLimit) {
                f = (float)kPlayerStatusControlSpinBarMaximum;
            } else {
                union {
                    unsigned int i[2];
                    double d;
                } u;
                u.i[PPC_DW_HI] = 0x43300000u;
                u.i[PPC_DW_LO] = (unsigned int)v ^ 0x80000000u;
                double d = u.d - kPlayerStatusSignedStatConversionBias;
                f = (float)(kPlayerStatusControlSpinBarUnitsPerStat * d);
            }
            *(float*)&barScale[0] = f;
            Layout_SetElementScale(obj, kControlBar, (float*)barScale);
        }

        {
            signed char v = (signed char)StatusSpinByte(self);
            float f;
            if (v >= kControlSpinDisplayLimit) {
                f = (float)kPlayerStatusControlSpinBarMaximum;
            } else {
                union {
                    unsigned int i[2];
                    double d;
                } u;
                u.i[PPC_DW_HI] = 0x43300000u;
                u.i[PPC_DW_LO] = (unsigned int)v ^ 0x80000000u;
                double d = u.d - kPlayerStatusSignedStatConversionBias;
                f = (float)(kPlayerStatusControlSpinBarUnitsPerStat * d);
            }
            *(float*)&barScale[0] = f;
            Layout_SetElementScale(obj, kSpinBar, (float*)barScale);
        }
    }

    // Phase 6: publish numeric stats, name message, and enable numeric drawing.
    {
        signed char v = (signed char)StatusPowerByte(self);
        UITextTable_SetInteger(AsCPlayerStatus(self)->numberDraw, kStatusPowerText, v);
    }
    {
        int vv = kControlSpinDisplayLimit;
        signed char v = (signed char)StatusControlByte(self);
        vv = (v < kControlSpinDisplayLimit) ? v : vv;
        UITextTable_SetInteger(AsCPlayerStatus(self)->numberDraw, kStatusControlText, vv);
    }
    {
        int vv = kControlSpinDisplayLimit;
        signed char v = (signed char)StatusSpinByte(self);
        vv = (v < kControlSpinDisplayLimit) ? v : vv;
        UITextTable_SetInteger(AsCPlayerStatus(self)->numberDraw, kStatusSpinText, vv);
    }

    ((CClubHouseMsg*)(AsCPlayerStatus(self)->clubHouseMessage))->ResetTextState();

    {
        char* selectedName = (self + kStatusNameOffset);
        char* resourceStrings = (char*)&playerStatusResourceStrings;
        char* emptyNameMarker = resourceStrings + kStatusEmptyNameMarkerOffset;
        int nameIsNonempty = strncmp(selectedName, emptyNameMarker, 1);
        if (nameIsNonempty) {
            char scratchPrefix[0x0A];
            char equipmentName[0x28];
            char scratchSuffix[0x28];
            int reservedWord = 0;
            (void)reservedWord;
            Memory_Clear(scratchPrefix, 0x0A);
            Memory_Clear(equipmentName, 0x28);
            Memory_Clear(scratchSuffix, 0x28);
            strncpy(equipmentName, (self + kStatusNameOffset), 0x28);
            void* nameGroup = BpObject_GetOrCreateNamedGroup(*(void**)&AsCPlayerStatus(self)->scriptObject,
                                                             resourceStrings + kStatusNameGroupOffset);
            ((CClubHouseMsg*)(AsCPlayerStatus(self)->clubHouseMessage))->SetTextFromGroup(equipmentName, nameGroup, 1);
        }
    }

    *(int*)&((NumberDrawEnableView*)AsCPlayerStatus(self)->numberDraw)->enabled = 1;

    return 1;
}

struct DartsQuadVertex {
    float x;
    float y;
    float z;
    int color;
    float u;
    float v;
};

#pragma opt_common_subs off
// Build a Y-rotated four-vertex quad, optionally selecting one quadrant of a 2x2 texture atlas.
// output receives an owned vertex-buffer handle plus primitiveCount=2. Offsets are
// borrowed 3-vectors. halfExtent is in the offsets' length units; yaw is passed
// unchanged to Mtx44_SetRotationY (angle units follow that function's contract).
// Corner equation: worldOffset + RotateY(yaw)*(localOffset + (+/-X +/-Y)*halfExtent).
// Thus localOffset rotates with the quad, while worldOffset remains in world space.
// atlasQuarter 0..3 chooses U half by bit0 and V row by /2; other values use default UVs.
void Darts_CreateRotatedTexturedQuad(void* output, int color, int atlasQuarter, void* worldOffset, void* localOffset,
                                     float halfExtent, float yaw) {
    DartsQuadVertex vertices[4];
    float rotation[4][4];
    float halfWidth[3];
    float halfHeight[3];
    float cornerX[3];
    float cornerY[3];
    float rotatedCorner[4];
    float vMin, uMin, vMax, uMax;
    void* bufferData;

    // Phase 1: local half axes and UV range (V runs max to min along increasing local Y).
    Vec3_Scale(halfWidth, kUnitXAxisVector, halfExtent);
    Vec3_Scale(halfHeight, kUnitYAxis, halfExtent);
    *(unsigned int*)&uMin = 0;
    *(unsigned int*)&vMin = 0;
    *(unsigned int*)&uMax = kDartsQuadDefaultUMaxBits;
    *(unsigned int*)&vMax = kDartsQuadDefaultVMaxBits;
    if (atlasQuarter >= 0 && atlasQuarter < 4) {
        unsigned int sign = (unsigned int)atlasQuarter >> 31;
        float f1;
        float f2;
        if ((((atlasQuarter & 1) ^ sign) - sign) != 0) {
            f1 = kDartsQuadAtlasHalfSpan;
        } else {
            f1 = kFloat_0_darts;
        }
        f2 = kDartsQuadAtlasHalfSpan;
        uMin = f1;
        uMax = f2 + uMin;
        f2 = (atlasQuarter / 2 != 0) ? f2 : kFloat_0_darts;
        vMin = f2;
        vMax = kDartsQuadAtlasVSpan + vMin;
    }
    // Phase 2: build corners in strip order (-X,-Y),(+X,-Y),(-X,+Y),(+X,+Y).
    Mtx44_SetRotationY(rotation, yaw);
    DartsQuadVertex* dst = vertices;
    DartsQuadVertex* src = dst;
    unsigned int half;
    for (unsigned int i = 0; i < 4; ++i) {
        float f0;
        float f1;
        unsigned int odd = i & 1;
        if (odd) {
            f0 = uMax;
        } else {
            f0 = uMin;
        }
        half = i >> 1;
        dst->u = f0;
        if (half) {
            f0 = vMin;
        } else {
            f0 = vMax;
        }
        dst->v = f0;
        if (odd) {
            f1 = kFloat_1_0_MenuModel;
        } else {
            f1 = kDartsQuadNegativeHalfAxisMultiplier;
        }
        Vec3_Scale(cornerX, halfWidth, f1);
        if (half) {
            f1 = kFloat_1_0_MenuModel;
        } else {
            f1 = kDartsQuadNegativeHalfAxisMultiplier;
        }
        Vec3_Scale(cornerY, halfHeight, f1);
        Vec3_Add(dst, cornerX, cornerY);
        Vec3_Add(dst, src, localOffset);
        Mtx44_TransformPointToVec4(rotatedCorner, src, rotation);
        dst->x = rotatedCorner[0];
        dst->y = rotatedCorner[1];
        dst->z = rotatedCorner[2];
        Vec3_Add(dst, src, worldOffset);
        gtl_PixelCopy_CopyPackedColor(&dst->color, &color);
        ++dst;
        ++src;
    }
    // Phase 3: allocate/upload four {XYZ,packed color,UV} vertices, flush and publish two triangles.
    VertexBuffer_Create(kQuadBufferBytes, 0, kDartsQuadFormat, 0, output, 0);
    Resource_GetOffsetPtr(*(void**)output, 0, 0, &bufferData, 0);
    memcpy(bufferData, vertices, kQuadBufferBytes);
    GraphicsBuffer_Flush(*(void**)output);
    ((QuadBufferView*)output)->primitiveCount = 2;
}
#pragma opt_common_subs reset

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462564", _ctors_u802E001C_0, InitTwoPiCache);
STATIC_INIT_ENTRY(".CRT$XCU80462564", _ctors_u802E001C_1, SimpleSpd_InitTwoPiCache);
STATIC_INIT_ENTRY(".CRT$XCU80462564", _ctors_u802E001C_2, Math_InitTwoPiCache);
#pragma pop
