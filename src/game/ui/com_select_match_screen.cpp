#include <decomp/static_init.h>
// Byte offsets preserve the pooled-string storage and original relocation addends.
// Byte offset of the "R_MSG_title_K13_LC" group name inside computerVersusMenuStrings.
enum ComputerVersusMenuStringsOffset {
    kTitleMessageNameOffset = 0xAE, // R_MSG_title_K13_LC
};

// Byte offsets within the match resource/error pool, decoded in phase1_program_strings.cpp.
enum ComputerMatchStringOffset { kMatchMessageResource = 0x15, kMatchVectorLengthError = 0x21,
    kMatchLengthErrorTypeName = 0x35 };

// Eight-byte match table entry: firstHole is zero-based; UI hole numbers add one.
struct ComputerMatchEntry {
    unsigned short tableKey;
    short opponentId;
    unsigned char courseId;
    signed char firstHole;
    signed char holeCount;
    char reserved;
};
struct ComputerOpponentEntry { int tableKey; int portraitIndex; };
enum ComputerMatchEntryOffsets {
    kMatchCourseOffset = (int)&((ComputerMatchEntry*)0)->courseId,
    kMatchFirstHoleOffset = (int)&((ComputerMatchEntry*)0)->firstHole,
    kMatchHoleCountOffset = (int)&((ComputerMatchEntry*)0)->holeCount
};
// Saved categories: 0..2 select the three category layouts and count as complete;
// 8 means no category layout. Other values >2 make a page incomplete, but cannot
// safely index categoryLayouts. Their individual meanings are not established.
enum ComputerMatchConstants {
    kMatchRowsPerPage = 4, kMatchPageLimit = 8,
    kNoResultCategory = 8, kLastCompletedCategory = 2,
    kCategoryLayoutSaveOffset = 0x238, kPageCompletionSaveOffset = 0x234,
    kComputerMatchRuleCode = 2,
    kEntryIndexVtableSlot = 13, kEntryDescriptionVtableSlot = 27,
    kDescriptionAllocatorBackstep = 0x28
};
#include "classes/SavePlayerRecord.h"
#include "classes/CEnemyRenderRecord.h"
#include "classes/CMsgDraw.h"
#include "game/types.h"
namespace SpreadLayout {
#include "classes/CComSelectMatch.h"
}
#define AsCComSelectMatch(p) ((SpreadLayout::CComSelectMatch*)(p))
extern "C" void appFreeUni__FPv(void*);

// Own a completed string only while its enclosing operation can unwind.
struct EH5StringCleanup {
    const unsigned int* words;
    bool active;
    // Arm unwind cleanup for a completed temporary string.
    explicit EH5StringCleanup(const void* p) : words((const unsigned int*)p), active(true) {}
    // Disarm the guard before explicit normal-path string cleanup.
    void release() { active = false; }
    // Free a heap string if its guarded operation unwinds before release.
    ~EH5StringCleanup() {
        if (active && (words[0] & 0x80000000u)) appFreeUni__FPv((void*)words[2]);
    }
};

// Translation unit 0x80322AA4-0x8032413C (15 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
#include "sdk_protos.h"
#include "classes/CComSelectBase.h"
#include "classes/CComMatchTbl.h"

#define AsCComMatchTbl(p) ((CComMatchTbl*)(p))

#define AsCComSelectBase(p) ((CComSelectBase*)(p))
extern "C" {
void appFreeUni__FPv(void*);

// Narrow and wide strings share the heap flag and pointer words.
struct StringUnwindCleanup_u80322AA4 {
    const unsigned int* words;
    bool active;
    // Arm heap cleanup for a temporary narrow or wide packed string.
    explicit StringUnwindCleanup_u80322AA4(const void* string)
        : words((const unsigned int*)string), active(true) {}
    // Disarm the guard before explicit normal-path string cleanup.
    void release() { active = false; }
    // Free heap string storage if the guard was not explicitly released.
    ~StringUnwindCleanup_u80322AA4() {
        if (active && (words[0] & 0x80000000u)) {
            appFreeUni__FPv((void*)words[2]);
        }
    }
};

void __throw(const char*, void*, void*) __attribute__((noreturn));

extern const float kFloat_6_2831855;
extern float s_computerMatchFullTurnAngle; // s_computerMatchFullTurnAngle
extern char __vt__15CComSelectMatch[];
extern char computerMatchCenteredTextPrefix[]; // computerMatchCenteredTextPrefix
extern char computerMatchPortraitResourceNames[]; // computerMatchPortraitResourceNames
extern char computerMatchResultCategoryResourceNames[]; // computerMatchResultCategoryResourceNames
extern char computerVersusMenuStrings[];
extern char computerMatchResourceAndErrorStrings[]; // computerMatchResourceAndErrorStrings
extern char __vt__Q23std12length_error[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char s_saveData[];

void* SaveData_AcquireInitializedInstance();
unsigned char* SaveData_GetCurrentRecord(unsigned char*);
void WString_ReserveCapacity(void*, int);
void WString_ReplaceRange(void*, int, int, void*, void*, void*, void*, unsigned char*);
int CSharedFilePtr_GetLoadedBuffer(void*);
void WFixedFont_CreateMsgEcbResource(void*, void**, int, int);
void CComSelectBase_StartMenuScripts(void*);
void CBpCtrl_dtor(void*);
void __ct__7CBpCtrlFv(void*);
void* IdMap_Lookup(void*, void*);
void UIGroup_GetPosition(void*, int);
void UIGroup_GetSourceSize(void*, void*);
void* BpObject_GetOrCreateNamedGroup(void*, void*);
void CMsgDraw_SetColor(void*, int);
void CMsgDraw_SetGlyphScale(void*, void*);
void Text_SetPosition(void*, void*);
void CMsgDraw_SetLayoutSize(void*, void*);
void CMsgDraw_ResetTextState(void*);
void Text_SetContent(void*, void*);
void CMsgDraw_ClearFormattingAndRevealAll(void*);
void Text_DrawGlyphs(void*);
void appFreeUni__FPv(void*);
void* __nwa__FUl(unsigned long);
void* appMallocDummy__FUl(unsigned long);
void __dla__FPv(void*);

void SaveData_Ctor(void*);
void CCharacterPreviewRecord_FillFromSaveData(void*, int, void*);
void EnemyData_BuildCharacterSlot(void*, void*, int, int);
void CPangyaEnemyData_Init(void*);
int Layout_HitTest(void*, int, int);
void* Table_GetEntry(void*, void*);
void* CComSelectBase_Ctor(void*);
int CComSelectBase_PollLoadComplete(void*);
void* SharedFile_Create(void*, int);
void CSharedFileHandle_Destroy(void*, int);
void* __nw__FUl(unsigned long);
void __dl__FPv(void*);
void CBpTex_Init(void*);
void __dt__15SelectMatchBaseFv(void*, int);
void* MessageTable_FindTextByName(void*, void*);
void CPangyaEnemyData_Init(void*);
void CPangyaEnemyData_dtor(void*);
void CComMatchTbl_dtor(void*, int);
extern signed char s_saveDataInitialized;
extern signed char s_pangyaEnemyDataInitialized;
extern char __vt__12CComMatchTbl[];
extern char computerMatchCourseDescriptionIds[]; // computerMatchCourseDescriptionIds
extern char s_pangyaEnemyData[];
extern char s_enemyDataDtorRegistration_ComputerMatch[]; // s_enemyDataDtorRegistration_ComputerMatch
extern void* computerMatchCategoryPositionGroupNames[4]; // computerMatchCategoryPositionGroupNames
extern int g_chatMsg;
extern signed char s_bpCtrlInitialized;
extern int kFloat_1_2_ComputerMenuGlyphScaleX; // kFloat_1_2_ComputerMenuGlyphScaleX
extern int kFloat_1_2_ComputerMenuGlyphScaleY; // kFloat_1_2_ComputerMenuGlyphScaleY
extern int s_matchEntryLabelGroupNames[]; // Four UI group-name addresses, indexed by match-entry row.
extern float kFloat_10_ComputerMenuLabelInset; // kFloat_10_ComputerMenuLabelInset
extern char s_bpCtrlRegistration_8052FED4[];
extern char computerMatchRowPositionGroupNames[]; // computerMatchRowPositionGroupNames
extern char s_bpCtrl[];
extern char s_bpCtrlDtorRegistration_ComputerMatch[]; // s_bpCtrlDtorRegistration_ComputerMatch

// Lazy singleton set-up (construct on first use, register its destructor) for the BP controller shared by
// the UI scripts; afterwards Sing_805DC5F0() holds s_bpCtrl. Replaces the block pasted inline at each use.
static inline void EnsureBpCtrl(char* dtorRegistration) {
    if (!((int&)Sing_805DC5F0())) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, dtorRegistration);
            s_bpCtrlInitialized = 1;
        }
        ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
    }
}

struct Obj3ED4 {
    void** vtable;
    char unk[0x1C];
    void* field20;
};

struct EncodedWideStringFields {
    unsigned int flags; // High bit selects heap storage; inline characters start at +2.
    int length;
    unsigned short* data;
};

struct WideStringWords { int flags, length, dataAddress; };

void WString_ConcatCStringAndString(WideStringWords*, void*, EncodedWideStringFields*);

struct DestroyLocal {
    void* item;
    void* vtable;
    // Release the temporary match-table wrapper without deleting its stack storage.
    ~DestroyLocal() { CComMatchTbl_dtor(this, -1); }
};

struct Local8007F694 {
    int flags, length, dataAddress;
    // Leave explicit label-string cleanup to the draw routine.
    ~Local8007F694() {}
};

struct MatchTextureStorage {
    void* vtable;
    char unk[0x24];
    // Initialize an owned BP texture wrapper before loading its resource.
    MatchTextureStorage() { CBpTex_Init(this); }
};
struct TextureCall { 
virtual void v08(); virtual void load(void*);
};


union Vec2U { float f[2]; int i[2]; };

class CComSelectMatchSelf { public:
    virtual void v08(); virtual void v0C(); virtual void v10(); virtual void v14();
    virtual void v18(); virtual void v1C(); virtual void v20(); virtual void v24(); virtual void v28(); virtual void v2C();
    virtual void v30(); virtual void* v34(void*, int); virtual void v38(); virtual void v3C(); virtual void v40(); virtual void v44();
    virtual void v48(); virtual void v4C(); virtual void v50(); virtual void v54(); virtual void v58(); virtual void v5C();
    virtual void v60(); virtual void v64(); virtual void v68(void*, void*);
};
// Local dispatch contract: v34 (slot 13) maps (page,row 0..3) to a table key,
// returned in a pointer-sized register; it does not return a dereferenceable row.
// v68 (slot 26) writes its label into an initialized wide string; raw slot 27
// writes a description. These names stay ABI views pending shared-name review.
// Layout/BP groups are borrowed. Wide string high flag bit owns a heap buffer;
// otherwise UTF-16 characters begin at +2 within the three-word string object.

#pragma push
#pragma opt_common_subs off
// Draw one match entry label, shifting selected or hovered text and applying its tint.
void CComSelectBase_DrawEntryLabel(void* self, int idx) {
    // Phase 1: resolve row -> table key -> label string. idx must be 0..3.
    Local8007F694 labelString;
    int groupNameAddresses[4];
    union Vec2U labelPosition;
    union Vec2U labelSize;
    labelString.flags = 0;
    labelString.length = 0;
    labelString.dataAddress = 0;
    void* entryKey = ((CComSelectMatchSelf*)self)->v34((*(void**)&AsCComSelectBase(self)->pageIndex), idx);
    ((CComSelectMatchSelf*)self)->v68(entryKey, &labelString);

    int* gp = (int*)s_matchEntryLabelGroupNames;
    groupNameAddresses[0] = gp[0];
    groupNameAddresses[1] = gp[1];
    groupNameAddresses[2] = gp[2];
    groupNameAddresses[3] = gp[3];
    int cond = ((int&)Sing_805DC5F0());

    labelPosition.i[0] = 0;
    labelPosition.i[1] = 0;
    labelSize.i[0] = 0;
    labelSize.i[1] = 0;

    if (!cond) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlRegistration_8052FED4);
            s_bpCtrlInitialized = 1;
        }
        ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
    }

    // Phase 2: borrow the BP row's position/size (layout coordinates).
    void* bp = (void*)((int&)Sing_805DC5F0());
    void* node = IdMap_Lookup(bp, (*(void**)&AsCComSelectBase(self)->bpEntryId));
    void* found = BpObject_GetOrCreateNamedGroup(node, (void*)groupNameAddresses[idx]);
    UIGroup_GetPosition(found, (int)&labelPosition);
    UIGroup_GetSourceSize(found, &labelSize);

    // Phase 3: selected and hovered labels shift left by ten layout units;
    // only hover changes the tint. The selected value is a table key, not idx.
    void* currentEntryKey = ((CComSelectMatchSelf*)self)->v34((*(void**)&AsCComSelectBase(self)->pageIndex), idx);
    int selectedEntryKey = AsCComSelectBase(self)->selectedEntry;

    if (selectedEntryKey == (int)currentEntryKey) {
        CMsgDraw_SetColor(AsCComSelectBase(self)->textRenderer, 0x2E2E2EFF);
        labelPosition.f[0] = labelPosition.f[0] - kFloat_10_ComputerMenuLabelInset; // kFloat_10_ComputerMenuLabelInset
    } else {
        int ht = Layout_HitTest(AsCComSelectBase(self)->layout, idx, 0);
        if (ht != 0) {
            CMsgDraw_SetColor(AsCComSelectBase(self)->textRenderer, 0x3E2806FF);
            labelPosition.f[0] = labelPosition.f[0] - kFloat_10_ComputerMenuLabelInset; // kFloat_10_ComputerMenuLabelInset
        } else {
            CMsgDraw_SetColor(AsCComSelectBase(self)->textRenderer, 0x2E2E2EFF);
        }
    }

    // Phase 4: draw from inline/heap UTF-16 storage, then release the label copy.
    Text_SetPosition(AsCComSelectBase(self)->textRenderer, (void*)&labelPosition);
    CMsgDraw_SetLayoutSize(AsCComSelectBase(self)->textRenderer, &labelSize);
    CMsgDraw_ResetTextState(AsCComSelectBase(self)->textRenderer);

    void* textRenderer = AsCComSelectBase(self)->textRenderer;
    if (!((unsigned int)labelString.flags >> 31)) {
        Text_SetContent(textRenderer, (void*)((char*)&labelString + 2));
    } else {
        Text_SetContent(textRenderer, (void*)labelString.dataAddress);
    }

    CMsgDraw_ClearFormattingAndRevealAll(AsCComSelectBase(self)->textRenderer);
    ((CMsgDraw*)AsCComSelectBase(self)->textRenderer)->drawSuppressed = 0;
    Text_DrawGlyphs(AsCComSelectBase(self)->textRenderer);

    if ((unsigned int)labelString.flags >> 31) {
        appFreeUni__FPv((void*)labelString.dataAddress);
    }
}
#pragma pop

// Draw the hovered entry description in the shared title text region.
void CComSelectBase_DrawHoveredEntryText(void* self) {
    // Phase 1: hit-test rows and resolve the first hovered row to its table key.
    int hoveredEntryKey = -1;
    for (int i = 0; i < 4; ++i) {
        if (Layout_HitTest(AsCComSelectBase(self)->layout, i, 0)) {
            hoveredEntryKey = ((int(*)(void*, void*, int))(*(void***)self)[kEntryIndexVtableSlot])(self, (*(void**)&AsCComSelectBase(self)->pageIndex), i);
            break;
        }
    }
    if (hoveredEntryKey == -1) return;

    WideStringWords descriptionString;
    descriptionString.flags = 0;
    descriptionString.length = 0;
    descriptionString.dataAddress = 0;
    StringUnwindCleanup_u80322AA4 stringCleanup0(&descriptionString);

    // Phase 2: obtain the description and prepend the centered-text formatting
    // prefix. Copy/replace handles all inline/heap combinations with unwind guards.
    ((void(*)(void*, int, void*))(*(void***)self)[kEntryDescriptionVtableSlot])(self, hoveredEntryKey, &descriptionString);

    WideStringWords centeredDescription;
    WString_ConcatCStringAndString(&centeredDescription, (void*)computerMatchCenteredTextPrefix, (EncodedWideStringFields*)&descriptionString); // computerMatchCenteredTextPrefix
    StringUnwindCleanup_u80322AA4 stringCleanup1(&centeredDescription);

    if (!(((unsigned int)descriptionString.flags >> 31) & 1) && !(((unsigned int)centeredDescription.flags >> 31) & 1)) {
        descriptionString.flags = centeredDescription.flags;
        descriptionString.length = centeredDescription.length;
        descriptionString.dataAddress = centeredDescription.dataAddress;
    } else {
        int erasedCodeUnits;
        if (((unsigned int)descriptionString.flags >> 31) & 1) erasedCodeUnits = descriptionString.length;
        else erasedCodeUnits = *(unsigned char*)&descriptionString & 0x7f;
        void* centeredText;
        int centeredCodeUnits;
        if (!(((unsigned int)centeredDescription.flags >> 31) & 1)) {
            centeredText = (char*)&centeredDescription + 2;
            centeredCodeUnits = *(unsigned char*)&centeredDescription & 0x7f;
        } else {
            centeredText = (void*)centeredDescription.dataAddress;
            centeredCodeUnits = centeredDescription.length;
        }
        // Storage contract retained from the recovered string ABI: the allocator
        // policy is a COPY of one byte 0x28 before descriptionString. The source
        // object's identity/lifetime is unresolved; it is not part of the string.
        // Text offsets below count UTF-16 code units (two bytes each).
        unsigned char allocatorPolicyByte = *((unsigned char*)&descriptionString - kDescriptionAllocatorBackstep);
        ((void(*)(void*, int, int, void*, void*, unsigned char*))WString_ReplaceRange)(&descriptionString, 0, erasedCodeUnits, centeredText, (char*)centeredText + centeredCodeUnits * 2, &allocatorPolicyByte);
    }

    if (((unsigned int)centeredDescription.flags >> 31) & 1) {
        { stringCleanup1.release(); appFreeUni__FPv((void*)centeredDescription.dataAddress); }
    }

    // Phase 3: use the shared title group's rectangle and glyph scale, draw the
    // owned description, then release it. Group coordinates are layout space.
    int titlePosition[2] = {0, 0};
    int titleSize[2] = {0, 0};
    int glyphScale[2] = {kFloat_1_2_ComputerMenuGlyphScaleX, kFloat_1_2_ComputerMenuGlyphScaleY};

    EnsureBpCtrl(s_bpCtrlRegistration_8052FED4);

    void* bp = (void*)((int&)Sing_805DC5F0());
    void* node = IdMap_Lookup(bp, (*(void**)&AsCComSelectBase(self)->bpEntryId));
    void* titleGroup = BpObject_GetOrCreateNamedGroup(node, (void*)(computerVersusMenuStrings + kTitleMessageNameOffset));
    UIGroup_GetPosition(titleGroup, (int)&titlePosition);
    UIGroup_GetSourceSize(titleGroup, &titleSize);
    Text_SetPosition(AsCComSelectBase(self)->textRenderer, (void*)&titlePosition);
    CMsgDraw_SetLayoutSize(AsCComSelectBase(self)->textRenderer, &titleSize);
    CMsgDraw_SetColor(AsCComSelectBase(self)->textRenderer, -1);
    CMsgDraw_ResetTextState(AsCComSelectBase(self)->textRenderer);
    CMsgDraw_SetGlyphScale(AsCComSelectBase(self)->textRenderer, (void*)&glyphScale);
    if (((unsigned int)descriptionString.flags) & 0x80000000) {
        Text_SetContent(AsCComSelectBase(self)->textRenderer, (void*)descriptionString.dataAddress);
    } else {
        Text_SetContent(AsCComSelectBase(self)->textRenderer, (void*)((char*)&descriptionString + 2));
    }
    CMsgDraw_ClearFormattingAndRevealAll(AsCComSelectBase(self)->textRenderer);
    ((CMsgDraw*)AsCComSelectBase(self)->textRenderer)->drawSuppressed = 0;
    Text_DrawGlyphs(AsCComSelectBase(self)->textRenderer);
    if (((unsigned int)descriptionString.flags) & 0x80000000) {
        { stringCleanup0.release(); appFreeUni__FPv((void*)descriptionString.dataAddress); }
    }
}

#pragma push
#pragma opt_propagation off
#pragma opt_common_subs off
struct MatchOwned { 
virtual void destroy(int);
};
// Replace the message renderer with one loaded from the match message resource.
void CComSelectMatch_RebuildMessageDraw(void* self) {
    int owned = (*(int*)&AsCComSelectMatch(self)->messageDraw);
    if (owned) {
        if (owned) ((MatchOwned*)owned)->destroy(1);
        (*(int*)&AsCComSelectMatch(self)->messageDraw) = 0;
    }
    void* manager = (void*)((int*)g_chatMsg)[4];
    int value = CSharedFilePtr_GetLoadedBuffer(AsCComSelectMatch(self)->messageResourceFile);
    WFixedFont_CreateMsgEcbResource(manager, (void**)&AsCComSelectMatch(self)->messageDraw, value, 1);
    CComSelectBase_StartMenuScripts(self);
}
#pragma pop

// Concatenate a wide C string and an encoded wide string into a new owned string.
void WString_ConcatCStringAndString(WideStringWords* out, void* text, EncodedWideStringFields* encoded) {
    out->flags = 0;
    out->length = 0;
    out->dataAddress = 0;
    EH5StringCleanup eh5Cleanup40(out);
    int length = wcslen((const wchar_t*)text);
    int count;
    unsigned short* chars;
    if (!(encoded->flags >> 31)) {
        unsigned char b = *(unsigned char*)&encoded->flags;
        chars = (unsigned short*)((char*)encoded + 2);
        count = b & 0x7f;
    } else {
        chars = encoded->data;
        count = (int)encoded->length;
    }
    WString_ReserveCapacity(out, length + count);
    volatile unsigned char flag1;
    volatile unsigned char flag2;
    unsigned char tmp1 = flag1;
    ((void(*)(void*, int, int, void*, void*, unsigned char*))WString_ReplaceRange)(out, 0, 0, text, (void*)((char*)text + length * 2), &tmp1);
    unsigned char tmp2 = flag2;
    ((void(*)(void*, int, int, void*, void*, unsigned char*))WString_ReplaceRange)(out, length, 0, chars, (void*)((char*)chars + count * 2), &tmp2);
    eh5Cleanup40.release();
}

// Read the position of a match-row UI group.
void CComSelectMatch_GetRowGroupPosition(void* self, int index, int arg) {
    int unusedSingletonRead = ((int&)Sing_805DC5F0());
    (void)unusedSingletonRead; // read only to reproduce the original load order
    int groupNames[4]; // addresses of the four row position-group names
    for (int i = 0; i < 4; ++i) groupNames[i] = ((int*)computerMatchRowPositionGroupNames)[i]; // computerMatchRowPositionGroupNames
    EnsureBpCtrl(s_bpCtrlDtorRegistration_ComputerMatch);
    void* bpObject = IdMap_Lookup((void*)((int&)Sing_805DC5F0()), AsCComSelectMatch(self)->bpEntryId);
    void* found = BpObject_GetOrCreateNamedGroup(bpObject, (void*)groupNames[index]);
    UIGroup_GetPosition(found, arg);
}

// Initialize the computer-match angle-wrap constant at startup.
void ComputerMatch_InitAngleWrapConstant() {
    s_computerMatchFullTurnAngle = kFloat_6_2831855; // s_computerMatchFullTurnAngle
}

#pragma push
#pragma opt_common_subs off
#pragma opt_propagation off
// Resolve a match opponent and choose its corresponding character layout.
int CComSelectMatch_GetOpponentPortraitLayout(void* self, int arg) {
    void* matchTableBuffer = (void*)CSharedFilePtr_GetLoadedBuffer(AsCComMatchTbl(self)->matchTableFile);
    DestroyLocal local;
    local.item = 0;
    if (matchTableBuffer) local.item = matchTableBuffer;
    local.vtable = (void*)__vt__12CComMatchTbl;
    void* result = ((CComSelectMatchSelf*)self)->v34(AsCComSelectMatch(self)->matchPageIndex, arg);
    result = Table_GetEntry(local.item, result);
    short index = ((ComputerMatchEntry*)result)->opponentId;
    if (!((int&)Sing_805DC678())) {
        if (!s_pangyaEnemyDataInitialized) {
            CPangyaEnemyData_Init(s_pangyaEnemyData);
            __register_global_object(s_pangyaEnemyData, (void*)CPangyaEnemyData_dtor, s_enemyDataDtorRegistration_ComputerMatch); // s_enemyDataDtorRegistration_ComputerMatch
            s_pangyaEnemyDataInitialized = 1;
        }
        ((int&)Sing_805DC678()) = (int)s_pangyaEnemyData;
    }
    result = Table_GetEntry(*(void**)((int&)Sing_805DC678()), (void*)index);
    int slot = ((ComputerOpponentEntry*)result)->portraitIndex;
    int value = (*(int*)&AsCComSelectMatch(self)->matchLayouts[slot]);
    return value;
}
#pragma pop

// Read the position of a match-category UI group.
void CComSelectMatch_GetCategoryGroupPosition(void* self, int index, int arg) {
    int unusedSingletonRead = ((int&)Sing_805DC5F0());
    (void)unusedSingletonRead; // read only to reproduce the original load order
    int groupNames[4]; // addresses of the four category position-group names
    int* p = (int*)computerMatchCategoryPositionGroupNames; // computerMatchCategoryPositionGroupNames
    groupNames[0] = *p++;
    groupNames[1] = *p++;
    groupNames[2] = *p++;
    groupNames[3] = *p++;
    EnsureBpCtrl(s_bpCtrlDtorRegistration_ComputerMatch);
    void* bpObject = IdMap_Lookup((void*)((int&)Sing_805DC5F0()), AsCComSelectMatch(self)->bpEntryId);
    void* found = BpObject_GetOrCreateNamedGroup(bpObject, (void*)groupNames[index]);
    UIGroup_GetPosition(found, arg);
}

#pragma push
#pragma opt_common_subs off
struct MatchTableLocal {
    void* item;
    void* vtable;
    // End a borrowed match-table view without freeing the shared file buffer.
    ~MatchTableLocal() {}
};
// Look up the selected course description and replace the output wide string.
void CComSelectMatch_GetCourseDescription(void* self, int arg, EncodedWideStringFields* encoded) {
    unsigned int count;
    const wchar_t* text;
    if (!encoded) return;
    void* matchTableBuffer = (void*)CSharedFilePtr_GetLoadedBuffer(AsCComMatchTbl(self)->matchTableFile);
    MatchTableLocal local;
    local.item = 0;
    if (matchTableBuffer) local.item = matchTableBuffer;
    local.vtable = __vt__12CComMatchTbl;
    void* result = ((CComSelectMatchSelf*)self)->v34(AsCComSelectMatch(self)->matchPageIndex, arg);
    result = Table_GetEntry(local.item, result);
    signed char courseId = (signed char)((ComputerMatchEntry*)result)->courseId;
    text = (const wchar_t*)MessageTable_FindTextByName(AsCComMatchTbl(self)->messageDraw, *(void**)((char*)computerMatchCourseDescriptionIds + courseId * 4)); // computerMatchCourseDescriptionIds
    count = !(encoded->flags >> 31) ? *(unsigned char*)&encoded->flags & 0x7f : (unsigned int)encoded->length;
    unsigned char tmp = *(((unsigned char*)&local) - 4);
    int length = wcslen((const wchar_t*)text);
    ((void(*)(void*, int, int, const wchar_t*, const wchar_t*, unsigned char*))WString_ReplaceRange)(encoded, 0, count, text, text + length, &tmp);
}
#pragma pop

class MatchPageInterface { public: 
virtual void v08(); virtual void v0C(); virtual void v10(); virtual void v14(); virtual void v18(); virtual void v1C(); virtual void v20(); virtual void v24(); virtual void v28(); virtual void v2C(); virtual void v30(); virtual void* v34(void*,int);
};
// Choose a saved match-result category layout, returning none for the sentinel value.
// matchSlot is a row on matchPageIndex. Save byte is record+0x238+resolved key;
// it must be 0..2 or 8. No validation exists for other signed byte values.
int CComSelectMatch_GetResultCategoryLayout(void* self, int matchSlot) {
    void* matchIndex = ((MatchPageInterface*)self)->v34(AsCComSelectMatch(self)->matchPageIndex, matchSlot);
    void* saveData = SaveData_AcquireInitializedInstance();
    unsigned char* savedRecord = SaveData_GetCurrentRecord((unsigned char*)saveData);
    unsigned char* matchResult = savedRecord + (int)(long)matchIndex;
    signed char resultCategory = (signed char)matchResult[kCategoryLayoutSaveOffset];
    if (resultCategory == kNoResultCategory) return 0;
    return (*(int*)&AsCComSelectMatch(self)->categoryLayouts[resultCategory]);
}

// Return the first incomplete four-match page, or eight when all checked pages are complete.
// Page zero is deliberately skipped. This accessor uses record+0x234+page*4+row,
// four bytes earlier than the category-layout accessor's base; preserve both.
int CComSelectMatch_GetFirstIncompletePage() {
    int pageBase = 4;
    for (int pageIndex = 1; pageIndex < 8; pageIndex++, pageBase += 4) {
        for (int matchSlot = 0; matchSlot < 4; matchSlot++) {
            void* saveData = SaveData_AcquireInitializedInstance();
            unsigned char* savedRecord = SaveData_GetCurrentRecord((unsigned char*)saveData);
            int resultIndex = pageBase;
            resultIndex = resultIndex + matchSlot;
            signed char resultCategory = (signed char)savedRecord[resultIndex + kPageCompletionSaveOffset];
            if (resultCategory > kLastCompletedCategory) return pageIndex;
        }
    }
    return 8;
}

class ResourceLoadInterface { public: 
virtual void v08(); virtual void v0C(); virtual void v10(); virtual void v14(); virtual int v18();
};
// Wait for eight character layouts, three category layouts and both shared files.
int CComSelectMatch_AreResourcesLoaded(void* self) {
    if (!CComSelectBase_PollLoadComplete(self)) return 0;
    void* r29 = self;
    void* r31 = r29;
    for (unsigned int r30 = 0; r30 < 8; ++r30, r31 = (char*)r31 + 4) {
        void* x = AsCComSelectMatch(r31)->matchLayouts[0];
        if (x) {
            if (!((ResourceLoadInterface*)x)->v18()) return 0;
        }
    }
    r31 = r29;
    for (unsigned int r30 = 0; r30 < 3; ++r30, r31 = (char*)r31 + 4) {
        void* x = AsCComSelectMatch(r31)->categoryLayouts[0];
        if (x) {
            if (!((ResourceLoadInterface*)x)->v18()) return 0;
        }
    }
    void* a = AsCComSelectMatch(r29)->matchTableFile;
    if (a) {
        if (!CSharedFilePtr_GetLoadedBuffer(a)) return 0;
    }
    void* b = AsCComSelectMatch(r29)->messageResourceFile;
    if (b) {
        if (!CSharedFilePtr_GetLoadedBuffer(b)) return 0;
    }
    return 1;
}

struct CComSelectMatchFields {
    void* vtable;
    char baseStorage[0x50];
    void* matchLayouts[8];
    void* categoryLayouts[3];
    void* matchTableFile;
    void* messageResourceFile;
    void* messageDraw;
};

} // C linkage
struct SelectMatchBase {
    char storage[0x54];
    // Construct the common computer-selection menu base.
    SelectMatchBase() { CComSelectBase_Ctor(this); }
    ~SelectMatchBase();
};
struct CComSelectMatch : SelectMatchBase {
    char rest[0x38];
    CComSelectMatch();
};
#pragma push
#pragma use_lmw_stmw on
// Construct the eight character layouts, three result layouts and match/message shared files.
CComSelectMatch::CComSelectMatch() {
    void* self = this;
    CComSelectMatchFields* s = (CComSelectMatchFields*)self;
    void* p;
    s->vtable = (void*)__vt__15CComSelectMatch;
    s->matchTableFile = 0;
    s->messageResourceFile = 0;
    s->messageDraw = 0;
    for (unsigned int i = 0; i < 8; ++i) {
        p = new MatchTextureStorage;
        AsCComSelectMatch((char*)self + i * 4)->matchLayouts[0] = p;
        ((TextureCall*)p)->load(((void**)computerMatchPortraitResourceNames)[i]); // computerMatchPortraitResourceNames
    }
    for (unsigned int j = 0; j < 3; ++j) {
        p = new MatchTextureStorage;
        AsCComSelectMatch((char*)self + j * 4)->categoryLayouts[0] = p;
        ((TextureCall*)p)->load(((void**)computerMatchResultCategoryResourceNames)[j]); // computerMatchResultCategoryResourceNames
    }
    s->matchTableFile = SharedFile_Create(computerMatchResourceAndErrorStrings, 0); // computerMatchResourceAndErrorStrings
    s->messageResourceFile = SharedFile_Create((char*)computerMatchResourceAndErrorStrings + kMatchMessageResource, 0); // computerMatchResourceAndErrorStrings
}
#pragma pop
extern "C" {

struct Dtor2 { 
virtual void destroy(int);
};
#pragma push
#pragma use_lmw_stmw on
// Destroy match/category layouts, shared files and message renderer before the base.
void* CComSelectMatch_dtor(void* self, int dealloc) {
    if (self) {
    *(void**)self = (void*)__vt__15CComSelectMatch;
    void* r30 = self;
    for (unsigned int r29 = 0; r29 < 8; ++r29, r30 = (char*)r30 + 4) {
        void* x = AsCComSelectMatch(r30)->matchLayouts[0];
        if (x) {
            if (x) ((Dtor2*)x)->destroy(1);
            AsCComSelectMatch(r30)->matchLayouts[0] = 0;
        }
    }
    r30 = self;
    for (unsigned int r29 = 0; r29 < 3; ++r29, r30 = (char*)r30 + 4) {
        void* x = AsCComSelectMatch(r30)->categoryLayouts[0];
        if (x) {
            if (x) ((Dtor2*)x)->destroy(1);
            AsCComSelectMatch(r30)->categoryLayouts[0] = 0;
        }
    }
    void* x = AsCComSelectMatch(self)->matchTableFile;
    if (x) {
        CSharedFileHandle_Destroy(x, 1);
        AsCComSelectMatch(self)->matchTableFile = 0;
    }
    x = AsCComSelectMatch(self)->messageResourceFile;
    if (x) {
        CSharedFileHandle_Destroy(x, 1);
        AsCComSelectMatch(self)->messageResourceFile = 0;
    }
    x = AsCComSelectMatch(self)->messageDraw;
    if (x) {
        if (x) ((Dtor2*)x)->destroy(1);
        AsCComSelectMatch(self)->messageDraw = 0;
    }
    __dt__15SelectMatchBaseFv(self, 0);
    if (dealloc > 0) __dl__FPv(self);
    }
    return self;
}
#pragma pop

void __dt__Q23std12length_errorFv(void*);
void* memset(void*, int, size_t);
void* memcpy(void*, const void*, size_t);

struct Ref { int vt; int c1; int c2; char* s; };
struct Holder_u80322AA4 {
    char* p; char* flag;
    // Guard newly allocated error text until a shared owner is ready.
    Holder_u80322AA4(char* b, char* f) { p = b; flag = f; }
    // Free error text whose ownership was not transferred.
    ~Holder_u80322AA4() { if (p) __dla__FPv(p); }
};
struct ErrBase {
    void* vt; char* buf; Ref* ref;
    // Construct the logic-error ABI and its owned diagnostic string/control block.
    ErrBase(const char* msg0) {
        vt = __vt__Q23std11logic_error;
        char done __attribute__((aligned(4))) = 0;
        const char* msg = msg0;
        unsigned n = strlen(msg) + 1;
        char* b = (char*)__nwa__FUl(n * 2 - n);
        buf = b;
        {
            Holder_u80322AA4 h(b, &done);
            Ref* r = (Ref*)__nw__FUl(16);
            if (r) { int table = (int)s_sharedCharDeleterVtable; r->c1 = 1; r->c2 = 1; r->vt = table; r->s = b; }
            ref = r;
            h.p = 0;
        }
        strcpy(buf, msg);
    }
    // Leave explicit exception-runtime ownership unchanged at local scope exit.
    ~ErrBase() {}
};
struct Err2 : ErrBase {
    // Install the length-error vtable over the constructed logic-error base.
    Err2(const char* msg) : ErrBase(msg) { vt = __vt__Q23std12length_error; }
    // Run the implicit base cleanup without introducing additional owned resources.
    ~Err2() {}
};

// Inlined std::length_error construction + throw.
// Throw a copied vector-length diagnostic through the exception runtime.
static inline void ThrowComputerMatchVectorLengthError() {
    Err2 error(computerMatchResourceAndErrorStrings + kMatchVectorLengthError);
    void* exception = (void*)&error;
    __throw(computerMatchResourceAndErrorStrings + kMatchLengthErrorTypeName, exception, (void*)__dt__Q23std12length_errorFv);
}

struct ByteVec {
    unsigned char* data;
    unsigned size;
    unsigned cap;
};


// Round configuration built from a selected computer match; ByteVec stores one-byte IDs.
struct ComputerMatchRoundFields {
    unsigned char courseId, firstHole, reserved, holeCount;
    ByteVec holeNumbers;
    ByteVec courseIds;
    CEnemyRenderRecord player;
    CEnemyRenderRecord opponent;
    char reserved164[0x148];
    unsigned int ruleType;
    int selectedMatchIndex;
};
typedef char ComputerMatch_player_offset[((int)&((ComputerMatchRoundFields*)0)->player == 0x1C) ? 1 : -1];
typedef char ComputerMatch_opponent_offset[((int)&((ComputerMatchRoundFields*)0)->opponent == 0xC0) ? 1 : -1];
typedef char ComputerMatch_ruleType_offset[((int)&((ComputerMatchRoundFields*)0)->ruleType == 0x2AC) ? 1 : -1];

// Split buffer used while a ByteVec grows.
struct SplitBuf {
    unsigned char* buf;
    unsigned front;
    unsigned cap;
    unsigned* capRef;
    unsigned back;
    // Free the allocation left in the split buffer after append commit or unwind.
    ~SplitBuf() {
        if (buf) {
            front = 0;
            appFreeUni__FPv(buf);
        }
    }
};

// Append one byte, growing capacity by 3/5 or 1/2 and transferring old storage.
// An inline-function form changes register allocation and exception metadata here.
// Matching constraint (verified): keep this storage/EH boundary.
// V must be a stable ByteVec pointer, VAL side-effect free (both are reread).
// Fast path increments size and writes the last byte. Growth phases below:
// (1) reserve guarded storage, (2) construct suffix, (3) move/zero old prefix,
// (4) swap so the vector owns the replacement and sb frees the old allocation.
#define VEC_PUSH_BACK(V, VAL) do { \
    if ((V)->size < (V)->cap) { \
        unsigned n = (V)->size + 1; \
        (V)->size = n; \
        *((V)->data + n - 1) = (unsigned char)(VAL); \
    } else { \
        if (0xFFFFFFFFu - (V)->cap < 1) { ThrowComputerMatchVectorLengthError(); } \
        /* Phase 1: reserve with bounded capacity growth. */ \
        SplitBuf sb; \
        sb.buf = 0; \
        sb.front = 0; \
        sb.cap = 0; \
        sb.capRef = &(V)->cap; \
        sb.back = 0; \
        unsigned threeFifths; \
        unsigned half; \
        unsigned need = (V)->size + 1 - (V)->cap; \
        unsigned oldcap = (V)->cap; \
        if (need > 0xFFFFFFFFu - oldcap) { ThrowComputerMatchVectorLengthError(); } \
        unsigned newcap; \
        if (oldcap < 0x55555555u) { \
            threeFifths = ((oldcap + 1) * 4 - (oldcap + 1)) / 5; \
            const unsigned* pm = &threeFifths; \
            if (threeFifths < need) pm = &need; \
            newcap = oldcap + *pm; \
        } else if (oldcap < 0xAAAAAAAAu) { \
            half = (oldcap + 1) >> 1; \
            const unsigned* pm = &half; \
            if (half < need) pm = &need; \
            newcap = oldcap + *pm; \
        } else { \
            newcap = (unsigned)-1; \
        } \
        unsigned char* nbuf = (unsigned char*)appMallocDummy__FUl(newcap); \
        unsigned osize = (V)->size; \
        sb.buf = nbuf; \
        sb.cap = newcap; \
        sb.back = osize; \
        /* Phase 2: construct the single-byte suffix at the old logical end. */ \
        memset(nbuf + osize + sb.front, (unsigned char)(VAL), 1); \
        unsigned char* odata = (V)->data; \
        unsigned olen = (odata + (V)->size) - odata; \
        /* Phase 3: transfer old prefix ownership, zeroing the source bytes. */ \
        sb.front = sb.front + 1; \
        sb.back = sb.back - olen; \
        memcpy(sb.buf + sb.back, odata, olen); \
        memset(odata, 0, olen); \
        sb.front = sb.front + olen; \
        unsigned char* tdata = (V)->data; \
        unsigned char* nb2 = sb.buf; \
        unsigned tcap = (V)->cap; \
        /* Phase 4: publish replacement; sb now owns the old allocation. */ \
        (V)->size = 0; \
        sb.buf = tdata; \
        (V)->cap = sb.cap; \
        sb.cap = tcap; \
        (V)->data = nb2; \
        (V)->size = sb.front; \
        sb.front = 0; \
    } \
} while (0)

#pragma push
#pragma use_lmw_stmw on
// Initialize/publish the save singleton when the caller's borrowed snapshot is null.
// Do not overwrite snapshot: the round builder retains the original read sequence.
static inline void EnsureComputerMatchSaveSingleton(int& snapshot) {
    if (!snapshot) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        ((int&)Sing_805DC5E0()) = (int)s_saveData;
    }
}

// Build the selected match course/hole lists and player/opponent render records.
// roundSettings is caller-owned ComputerMatchRoundFields storage, with valid
// vector headers. Return 0 if no selection, else append to BOTH existing vectors
// (they are not cleared), overwrite records/rule/selection and return 1. The table
// must be loaded and contain the selected key; allocation failures may throw.
// Stored firstHole is zero based; holeNumbers appends firstHole+1 onward for UI.
// Append the selected round's hole/course IDs and construct its player/opponent records.
int CComSelectMatch_BuildRoundSettings(void* self, void* roundSettings) {
    // Phase 1: borrow selected table data, copying four 16-bit words in the
    // original order. matchWords has exactly the ComputerMatchEntry byte layout.
    ByteVec* holeNumbers = &((ComputerMatchRoundFields*)roundSettings)->holeNumbers;
    ByteVec* courseIds = &((ComputerMatchRoundFields*)roundSettings)->courseIds;
    int i;
    if (AsCComMatchTbl(self)->selectedMatchIndex == -1) return 0;

    int t = CSharedFilePtr_GetLoadedBuffer(AsCComMatchTbl(self)->matchTableFile);
    struct { void* item; void* vtable; } scope;
    scope.item = 0;
    if (t) scope.item = (void*)t;
    scope.vtable = __vt__12CComMatchTbl;

    unsigned short* matchEntry = (unsigned short*)Table_GetEntry(scope.item, (*(void**)&AsCComMatchTbl(self)->selectedMatchIndex));
    unsigned short matchWords[4];
    matchWords[3] = matchEntry[3];
    matchWords[2] = matchEntry[2];
    matchWords[1] = matchEntry[1];
    matchWords[0] = matchEntry[0];
    // Use field-derived byte offsets to keep the unsigned load/store scheduling;
    // typed signed-char field accesses change register allocation in this routine.
    unsigned char* matchBytes = (unsigned char*)matchWords;

    unsigned char courseId = matchBytes[kMatchCourseOffset];
    signed char firstHole = (signed char)matchBytes[kMatchFirstHoleOffset];
    signed char holeCount = (signed char)matchBytes[kMatchHoleCountOffset];
    *(unsigned char*)((char*)roundSettings + 0) = courseId;
    *(unsigned char*)((char*)roundSettings + 1) = matchBytes[kMatchFirstHoleOffset];
    *(unsigned char*)((char*)roundSettings + 2) = 0;
    *(unsigned char*)((char*)roundSettings + 3) = matchBytes[kMatchHoleCountOffset];

    // Phase 2: append parallel one-byte hole/course IDs. Each append commits
    // separately, so an exception may leave a partially extended round.
    int holeIndex = firstHole;
    int numHoles = holeCount;
    for (i = 0; i < numHoles; ++i) {
        int oneBasedHole = holeIndex + 1;
        VEC_PUSH_BACK(holeNumbers, oneBasedHole);
        VEC_PUSH_BACK(courseIds, courseId);
        ++holeIndex;
    }

    // Phase 3: acquire save data and build the player's render record. The two
    // singleton reads retain their original order and are not interchangeable.
    int initialSaveSingleton = ((int&)Sing_805DC5E0());
    ((ComputerMatchRoundFields*)roundSettings)->ruleType = kComputerMatchRuleCode;

    EnsureComputerMatchSaveSingleton(initialSaveSingleton);
    int playerSaveSingleton = ((int&)Sing_805DC5E0());
    EnsureComputerMatchSaveSingleton(playerSaveSingleton);
    unsigned char* currentPlayerSave = SaveData_GetCurrentRecord((unsigned char*)((int&)Sing_805DC5E0()));
    unsigned char* characterSave = SaveData_GetCurrentRecord((unsigned char*)playerSaveSingleton);
    CCharacterPreviewRecord_FillFromSaveData(&((ComputerMatchRoundFields*)roundSettings)->player, (signed char)characterSave[(int)&((SavePlayerRecord*)0)->characterIndex], currentPlayerSave);

    if (!((int&)Sing_805DC678())) {
        if (!s_pangyaEnemyDataInitialized) {
            CPangyaEnemyData_Init(s_pangyaEnemyData);
            __register_global_object(s_pangyaEnemyData, (void*)CPangyaEnemyData_dtor, s_enemyDataDtorRegistration_ComputerMatch); // s_enemyDataDtorRegistration_ComputerMatch
            s_pangyaEnemyDataInitialized = 1;
        }
        ((int&)Sing_805DC678()) = (int)s_pangyaEnemyData;
    }
    // Phase 4: build the table-selected opponent, then latch the selected key.
    EnemyData_BuildCharacterSlot((void*)((int&)Sing_805DC678()), &((ComputerMatchRoundFields*)roundSettings)->opponent, (short)matchWords[1], 0);

    ((ComputerMatchRoundFields*)roundSettings)->selectedMatchIndex = AsCComMatchTbl(self)->selectedMatchIndex;
    return 1;
}
#pragma pop

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462600", _ctors_u80322AA4_0, ComputerMatch_InitAngleWrapConstant); // ComputerMatch_InitAngleWrapConstant
#pragma pop
