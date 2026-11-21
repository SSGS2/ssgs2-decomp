enum ComputerMenuConstants {
    kComputerRowsPerPage = 4, kComputerPageIndicatorCount = 8, kComputerTitleGroupCount = 10, kPreviousPageElement = 4, kNextPageElement = 5,
    kCancelAction = 6, kPreviousPageAction = 2, kNextPageAction = 3, kOptionsAction = 0x11,
    kAnyInputPlayer = -1, kComputerMenuScriptIndex = 10,
    kEntryTextColor = 0x2E2E2EFF,
    kSpriteQuadPrimitive = 0x98, kSpriteVertexBytes = 0x18,
    kOptionDialogAllocationBytes = 0x860, kSelectedRowValueState = 2,
    kRowInputDisabledFlags = 5, kPageFadeFrames = 5, kHoveredTextColor = 0x3E2806FF
};
// Byte offsets into the existing packed string pool; no new string storage.
enum ComputerMenuStringOffset {
    kComputerMenuScript = 0x44,
    kComputerBackgroundScript = 0x4A,
    kComputerTitleScript = 0x4D,
    kComputerTitleFontTwoGroup = 0x53,
    kComputerTitleFontOneGroup = 0x64,
    kComputerRewardSound = 0x75,
    kComputerPageChangeSound = 0x88,
    kComputerCancelSound = 0x97,
};

#include "classes/CSharedDoc.h"
#include "classes/CMsgDraw.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))

// Translation unit 0x80321370-0x80322AA4 (12 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
#include "game/wstring.h"
#include "classes/CComSelectBase.h"

#define AsCComSelectBase(p) ((CComSelectBase*)(p))
struct Obj8032V {
    virtual void v0(int);
    virtual int v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual void v8();
    virtual void v9(int);
    virtual int v10(int);
    virtual int v11(int page, int row);   // CComSelectBase_GetEntryIndex: entry number of (page, row)
    virtual void v12();
    virtual void v13();
    virtual void v14();
    virtual void v15();
    virtual void v16();
    virtual void v17();
    virtual void v18();
    virtual void v19();
    virtual void v20();
    virtual void v21();
    virtual void v22();
    virtual void v23(int, void*);
    virtual void v24();
    virtual void v25();
    virtual void v26(void*);
};

extern "C" {
extern char s_comSelectResourceTableDtorRegistration[];

void __dl__FPv(void*);

struct Obj8032Call {
    virtual void c0();
    virtual void c1();
    virtual void c2();
    virtual void c3();
    virtual void c4();
    virtual void c5();
    virtual void c6();
    virtual void c7();
    virtual void c8();
    virtual void c9();
    virtual void c10();
    virtual int c11(int, int);
};

// Menu storage, in bytes from self. The result dialog is at +0x44 and its
// pending byte at +0x48, agreeing with the shared layout. The old local names
// were four bytes too high: textRenderer ends at +0x3C, then eight bytes of padding.
// +0x4C/+0x50 remain BP IDs throughout initialization, drawing and update.
struct ComputerMenuLocalView {
    char pad00[0x24];
    int pageCount;
    char pad28[4];
    int requestedPageIndex;
    void* packTextureResource;
    void* layoutController;
    void* textRenderer;
    char padding3C[8];
    void* courseClearDialog;
    unsigned char courseClearPending;
    char padding49[3];
    int secondaryBpIdWord;
    int tertiaryBpIdWord;
};

typedef char ComputerMenuLocalView_dialogOffset[((int)&((ComputerMenuLocalView*)0)->courseClearDialog == 0x44) ? 1 : -1];
typedef char ComputerMenuLocalView_pendingOffset[((int)&((ComputerMenuLocalView*)0)->courseClearPending == 0x48) ? 1 : -1];

void Layout_SetElementValueState(void*, int, int);
void Layout_SetElementPlayerFlagsAndState(void*, int, int);
void* IdMap_Lookup(void*, int);
void* BpObject_GetOrCreateNamedGroup(void*, int);
void UIGroup_SetEnabled(void*, int);
extern int computerMenuPageIndicatorGroupNames[]; // Eight numbered page-indicator group names.
extern signed char s_bpCtrlInitialized;
extern char s_bpCtrl[];
extern char s_bpCtrlRegistration_8052FED4[];
void* __ct__7CBpCtrlFv(void*);
void CBpCtrl_dtor(void);
void __register_global_object(void*, void*, void*);

// Ensure the borrowed script controller exists; callers still sample its global slot.
static inline void EnsureComputerMenuScriptController() {
    if (Sing_805DC5F0() == 0) {
        if (s_bpCtrlInitialized == 0) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlRegistration_8052FED4);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
}

#pragma use_lmw_stmw on
// Select a valid page and update its navigation buttons and page indicator groups.
int CComSelectBase_SetPage(ComputerMenuLocalView* self, int pageIndex) {
    if (pageIndex < 0 || pageIndex >= self->pageCount)
        return 0;
    if (AsCComSelectBase(self)->pageIndex == pageIndex)
        return 0;
    AsCComSelectBase(self)->pageIndex = pageIndex;
    Layout_SetElementValueState(AsCComSelectBase(self)->layout, kPreviousPageElement, 0);
    Layout_SetElementValueState(AsCComSelectBase(self)->layout, kNextPageElement, 0);
    if (pageIndex == 0)
        Layout_SetElementPlayerFlagsAndState(AsCComSelectBase(self)->layout, kPreviousPageElement, 0);
    else
        Layout_SetElementPlayerFlagsAndState(AsCComSelectBase(self)->layout, kPreviousPageElement, 1);
    if (pageIndex == self->pageCount - 1)
        Layout_SetElementPlayerFlagsAndState(AsCComSelectBase(self)->layout, kNextPageElement, 0);
    else
        Layout_SetElementPlayerFlagsAndState(AsCComSelectBase(self)->layout, kNextPageElement, 1);
    int page7Group;
    int page6Group;
    int page5Group;
    int page4Group;
    int page3Group;
    int page2Group;
    int page1Group;
    int page0Group;
    int* groupIds = computerMenuPageIndicatorGroupNames;
    page0Group = *groupIds++;
    page1Group = *groupIds++;
    page2Group = *groupIds++;
    page3Group = *groupIds++;
    page4Group = *groupIds++;
    page5Group = *groupIds++;
    page6Group = *groupIds++;
    page7Group = *groupIds++;
    int ids[kComputerPageIndicatorCount] = { page0Group, page1Group, page2Group, page3Group, page4Group, page5Group, page6Group, page7Group };
    int* groupCursor = ids;
    for (unsigned int pageIndicatorIndex = 0; pageIndicatorIndex < kComputerPageIndicatorCount; ++pageIndicatorIndex, ++groupCursor) {
        int isCurrentPage = (pageIndicatorIndex == AsCComSelectBase(self)->pageIndex);
        EnsureComputerMenuScriptController();
        void* scriptObject = IdMap_Lookup(Sing_805DC5F0(), AsCComSelectBase(self)->bpEntryId);
        void* pageGroup = BpObject_GetOrCreateNamedGroup(scriptObject, *groupCursor);
        UIGroup_SetEnabled(pageGroup, isCurrentPage);
    }
    return 1;
}
#pragma use_lmw_stmw reset

void BpSprite_GetTextureDescriptor(void*, void*);
void* GX_GetPrimitiveCount(int, int, unsigned int, float, float, float, float);
void DrawPrimitiveUP(int, void*, void*, int);

struct Quad24 {
    float v[24];
};
extern const Quad24 computerMenuSpriteQuadDefaults; // Default textured sprite quad.

#pragma opt_lifetimes off
// Draw a sprite-sized textured quad at the supplied screen position.
void CComSelectBase_DrawRowSprite(void*, void* object, float* position) {
    if (object == 0)
        return;
    unsigned int size[7];
    BpSprite_GetTextureDescriptor(object, size);
    ((Obj8032V*)object)->v3();
    Quad24 verts = computerMenuSpriteQuadDefaults;
    verts.v[0] = position[0];
    verts.v[1] = position[1];
    verts.v[6] = position[0] + (float)size[0];
    verts.v[7] = position[1];
    verts.v[12] = position[0];
    verts.v[13] = position[1] + (float)size[1];
    verts.v[18] = position[0] + (float)size[0];
    verts.v[19] = position[1] + (float)size[1];
    DrawPrimitiveUP(kSpriteQuadPrimitive, GX_GetPrimitiveCount(4, kSpriteQuadPrimitive, size[0], (float)size[1], (float)size[0], (float)size[1], position[0]), &verts, kSpriteVertexBytes);
}
#pragma opt_lifetimes reset

// Store a page cursor index only when it is within the menu range.
int CComSelectBase_SetPageCursor(ComputerMenuLocalView* self, int pageIndex) {
    if (pageIndex < 0 || pageIndex >= self->pageCount)
        return 0;
    self->requestedPageIndex = pageIndex;
    return 1;
}

// Map a valid page and row to an integer entry index; return -1 if either is invalid.
// ABI words are integers, including the result. Four rows form a page; no pointer
// is received, dereferenced, returned or owned by this operation.
int CComSelectBase_GetEntryIndex(ComputerMenuLocalView* self, int pageIndex, int rowIndex) {
    if (pageIndex < 0 || pageIndex > self->pageCount - 1)
        return -1;
    if (rowIndex < 0 || rowIndex >= kComputerRowsPerPage)
        return -1;
    return rowIndex + kComputerRowsPerPage * pageIndex;
}

void CPackTextureResource_PollLoad(void*);
int Layout_PollLoadComplete(void*);
void Layout_Update(void*);
int CPackTextureResource_IsLoadFinished(void*);
void* __ct__7CBpCtrlFv(void*);
void CBpCtrl_dtor(void);
int CBpCtrl_PollAllEntriesLoaded(void*);
void __register_global_object(void*, void*, void*);
int Layout_HitTest(void*, int, int);
extern signed char s_bpCtrlInitialized;
extern char s_bpCtrl[];
extern char s_bpCtrlRegistration_8052FED4[];
extern const float kFloat_1_ComSelect;
extern const float kFloat_10_ComputerMenuLabelInset;
void BpObject_Draw(void*);
void Layout_DrawWithOpacity(void*, float);
void MPBase1_DrawIfInitialized(void*);
void UIGroup_GetPosition(void*, void*);
void UIGroup_GetSourceSize(void*, void*);
void CMsgDraw_SetColor(void*, int);
void Text_SetPosition(void*, void*);
void CMsgDraw_SetLayoutSize(void*, void*);
void CMsgDraw_ResetTextState(void*);
void Text_SetContent(void*, void*);
void CMsgDraw_ClearFormattingAndRevealAll(void*);
void Text_DrawGlyphs(void*);
void appFreeUni__FPv(void*);
extern int computerMenuCourseLabelGroupNames[]; // Four course-label group names.

void BpObject_StartScriptByName(void*, void*, int);
void BpObject_TryInvokeCurrentCallback(void*, int);
void CBpCtrlHandle_SetScriptIndex(void*, int);
void IFadeMenuBase_StartMenuFadeOut(void*);
extern void* g_sharedDoc;
extern signed char s_bpCtrlHandleInitialized;
extern int s_bpCtrlHandle[];
extern char computerVersusMenuStrings[];
extern int computerMenuEntryButtonGroupNames[]; // Four computer-versus row-button group names.
extern int computerMenuTitleFontGroupNames[]; // Ten title-font group names.
void CBpCtrlHandle_Dtor(void);
extern char s_computerMenuBpHandleDtorRegistration[]; // Destructor registration storage for computerMenuBpHandleDtorRegistration.

// Start menu/background/title scripts, reset selection, and consume pending reward state.
void CComSelectBase_StartMenuScripts(ComputerMenuLocalView* self) {
    // Phases: release the old texture, start row/background/title BP scripts,
    // configure visible groups, set notification state, then enter the fade-menu base.
    void* previousResource = AsCComSelectBase(self)->textureResource;
    if (previousResource != 0) {
        if (previousResource != 0) {
            ((void(*)(void*, int))((void**)*(void**)previousResource)[2])(previousResource, 1);
        }
        AsCComSelectBase(self)->textureResource = 0;
    }
    int pageCount = ((int(*)(ComputerMenuLocalView*))((void**)*(void**)self)[0x3c / 4])(self);
    void* bpController = Sing_805DC5F0();
    self->pageCount = pageCount;
    self->requestedPageIndex = pageCount - 1;
    if (bpController == 0) {
        if (s_bpCtrlInitialized == 0) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlRegistration_8052FED4);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    void* bpObject = IdMap_Lookup(Sing_805DC5F0(), AsCComSelectBase(self)->bpEntryId);
    BpObject_StartScriptByName(bpObject, computerVersusMenuStrings + kComputerMenuScript, 1);
    int entryGroupIds[kComputerRowsPerPage];
    int* entryGroupNames = computerMenuEntryButtonGroupNames;
    int row0Group = *entryGroupNames++;
    int row1Group = entryGroupNames[0];
    int row2Group = entryGroupNames[1];
    int row3Group = entryGroupNames[2];
    unsigned int groupIndex = 0;
    int* groupCursor = entryGroupIds;
    entryGroupIds[0] = row0Group;
    entryGroupIds[1] = row1Group;
    entryGroupIds[2] = row2Group;
    entryGroupIds[3] = row3Group;
    while (groupIndex < kComputerRowsPerPage) {
        void* scriptGroup = BpObject_GetOrCreateNamedGroup(bpObject, *groupCursor);
        UIGroup_SetEnabled(scriptGroup, 0);
        ++groupIndex;
        ++groupCursor;
    }
    BpObject_TryInvokeCurrentCallback(bpObject, 0);

    EnsureComputerMenuScriptController();
    bpObject = IdMap_Lookup(Sing_805DC5F0(), AsCComSelectBase(self)->secondaryBpEntryId);
    BpObject_StartScriptByName(bpObject, computerVersusMenuStrings + kComputerBackgroundScript, 1);
    BpObject_TryInvokeCurrentCallback(bpObject, 0);

    int titleGroupIds[kComputerTitleGroupCount];
    int* titleGroupNames = computerMenuTitleFontGroupNames;
    int title0Group = *titleGroupNames++;
    int title1Group = titleGroupNames[0];
    int title2Group = titleGroupNames[1];
    int title3Group = titleGroupNames[2];
    int title4Group = titleGroupNames[3];
    int title5Group = titleGroupNames[4];
    int title6Group = titleGroupNames[5];
    int title7Group = titleGroupNames[6];
    int title8Group = titleGroupNames[7];
    int title9Group = titleGroupNames[8];
    titleGroupIds[0]=title0Group; titleGroupIds[1]=title1Group; titleGroupIds[2]=title2Group; titleGroupIds[3]=title3Group; titleGroupIds[4]=title4Group;
    titleGroupIds[5]=title5Group; titleGroupIds[6]=title6Group; titleGroupIds[7]=title7Group; titleGroupIds[8]=title8Group; titleGroupIds[9]=title9Group;

    EnsureComputerMenuScriptController();
    bpObject = IdMap_Lookup(Sing_805DC5F0(), AsCComSelectBase(self)->tertiaryBpEntryId);
    BpObject_StartScriptByName(bpObject, computerVersusMenuStrings + kComputerTitleScript, 1);
    groupIndex = 0;
    groupCursor = titleGroupIds;
    while (groupIndex < kComputerTitleGroupCount) {
        void* scriptGroup = BpObject_GetOrCreateNamedGroup(bpObject, *groupCursor);
        UIGroup_SetEnabled(scriptGroup, 0);
        ++groupIndex;
        ++groupCursor;
    }
    if (AsCSharedDoc(g_sharedDoc)->golfRuleType == 0) {
        void* scriptGroup = BpObject_GetOrCreateNamedGroup(bpObject, (int)(computerVersusMenuStrings + kComputerTitleFontTwoGroup));
        UIGroup_SetEnabled(scriptGroup, 1);
    } else {
        void* scriptGroup = BpObject_GetOrCreateNamedGroup(bpObject, (int)(computerVersusMenuStrings + kComputerTitleFontOneGroup));
        UIGroup_SetEnabled(scriptGroup, 1);
    }
    BpObject_TryInvokeCurrentCallback(bpObject, 0);

    ((void(*)(ComputerMenuLocalView*, int))((void**)*(void**)self)[0x30 / 4])(self, self->requestedPageIndex);

    void* sharedDocument = g_sharedDoc;
    int pendingNotification = (*(int*)&((CSharedDoc*)sharedDocument)->pendingCourseClearNotification);
    int hasNotification = (pendingNotification != 0) ? 1 : 0;
    // Consume the pending course-clear notification into this menu's local latch.
    AsCComSelectBase(self)->courseClearNotificationPending = (unsigned char)hasNotification;
    (*(int*)&((CSharedDoc*)sharedDocument)->pendingCourseClearNotification) = 0;

    if (Sing_805DC5F8() == 0) {
        if (s_bpCtrlHandleInitialized == 0) {
            s_bpCtrlHandle[0] = -1;
            s_bpCtrlHandle[1] = -1;
            __register_global_object(s_bpCtrlHandle, CBpCtrlHandle_Dtor, s_computerMenuBpHandleDtorRegistration);
            s_bpCtrlHandleInitialized = 1;
        }
        Sing_805DC5F8() = s_bpCtrlHandle;
    }
    CBpCtrlHandle_SetScriptIndex(Sing_805DC5F8(), kComputerMenuScriptIndex);
    IFadeMenuBase_StartMenuFadeOut(self);
}

// Poll the texture pack and continue loading the layout controller.
void CComSelectBase_PollResourceLoading(ComputerMenuLocalView* self) {
    if (self->packTextureResource != 0)
        CPackTextureResource_PollLoad(self->packTextureResource);
    if (self->layoutController != 0 && Layout_PollLoadComplete(self->layoutController) == 0)
        Layout_Update(self->layoutController);
}

// Report readiness after the texture pack, BP entries, and layout finish loading.
int CComSelectBase_PollLoadComplete(ComputerMenuLocalView* self) {
    if (self->packTextureResource != 0 && CPackTextureResource_IsLoadFinished(self->packTextureResource) == 0)
        return 0;
    EnsureComputerMenuScriptController();
    if (CBpCtrl_PollAllEntriesLoaded(Sing_805DC5F0()) == 0)
        return 0;
    if (self->layoutController != 0 && Layout_PollLoadComplete(self->layoutController) == 0)
        return 0;
    return 1;
}

// Row operations use byte offsets in the original vtable. Accessors borrow
// sprites; position outputs are two floats in layout/screen coordinates.
struct ComputerMenuRowVtable {
    char prefix[0x34];
    int (*mapEntry)(ComputerMenuLocalView*, int page, int row); // +0x34
    void (*drawSprite)(ComputerMenuLocalView*, void*, float*); // +0x38
    char unused3C[0x18];
    void* (*primarySprite)(ComputerMenuLocalView*, int);       // +0x54
    void (*primaryPosition)(ComputerMenuLocalView*, int, float*);
    void* (*secondarySprite)(ComputerMenuLocalView*, int);     // +0x5C
    void (*secondaryPosition)(ComputerMenuLocalView*, int, float*);
};
typedef char ComputerMenuRowVtable_primaryOffset[((int)&((ComputerMenuRowVtable*)0)->primarySprite == 0x54) ? 1 : -1];

// Draw primary row sprites, shifting selected or hovered entries left.
void CComSelectBase_DrawPrimaryRowSprites(ComputerMenuLocalView* self) {
    float rowPosition[2] = {0.0f, 0.0f};
    float selectionInsetPixels = kFloat_10_ComputerMenuLabelInset;
    int rowIndex = 0;
    for (; rowIndex < kComputerRowsPerPage; ++rowIndex) {
        void* rowSprite = (*(ComputerMenuRowVtable**)self)->primarySprite(self, rowIndex);
        if (rowSprite != 0) {
            (*(ComputerMenuRowVtable**)self)->primaryPosition(self, rowIndex, rowPosition);
            int logicalEntryIndex = (*(ComputerMenuRowVtable**)self)->mapEntry(self, AsCComSelectBase(self)->pageIndex, rowIndex);
            if (AsCComSelectBase(self)->selectedEntry == logicalEntryIndex || Layout_HitTest(AsCComSelectBase(self)->layout, rowIndex, 0))
                rowPosition[0] -= selectionInsetPixels;
            (*(ComputerMenuRowVtable**)self)->drawSprite(self, rowSprite, rowPosition);
        }
    }
}

// Draw secondary row sprites with the same selection/hover inset.
void CComSelectBase_DrawSecondarySprites(ComputerMenuLocalView* self) {
    float rowPosition[2] = {0.0f, 0.0f};
    float selectionInsetPixels = kFloat_10_ComputerMenuLabelInset;
    int rowIndex = 0;
    for (; rowIndex < kComputerRowsPerPage; ++rowIndex) {
        void* rowSprite = (*(ComputerMenuRowVtable**)self)->secondarySprite(self, rowIndex);
        if (rowSprite != 0) {
            (*(ComputerMenuRowVtable**)self)->secondaryPosition(self, rowIndex, rowPosition);
            int logicalEntryIndex = (*(ComputerMenuRowVtable**)self)->mapEntry(self, AsCComSelectBase(self)->pageIndex, rowIndex);
            if (AsCComSelectBase(self)->selectedEntry == logicalEntryIndex || Layout_HitTest(AsCComSelectBase(self)->layout, rowIndex, 0))
                rowPosition[0] -= selectionInsetPixels;
            (*(ComputerMenuRowVtable**)self)->drawSprite(self, rowSprite, rowPosition);
        }
    }
}

struct VS8032_ { 
virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5(); virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9(); virtual void v10(); virtual void v11(); virtual void v12(); virtual void v13(); virtual void v14(); virtual void v15(); virtual void v16(int); virtual void v17(int); virtual void v18();
};
struct VP44_ { 
virtual void a0(); virtual void a1(); virtual void a2();
};
// Draw BP layers, layout, row sprites/text, and active modal UI.
void CComSelectBase_Draw(ComputerMenuLocalView* self) {
    EnsureComputerMenuScriptController();
    BpObject_Draw(IdMap_Lookup(Sing_805DC5F0(), AsCComSelectBase(self)->secondaryBpEntryId));
    EnsureComputerMenuScriptController();
    BpObject_Draw(IdMap_Lookup(Sing_805DC5F0(), AsCComSelectBase(self)->tertiaryBpEntryId));
    EnsureComputerMenuScriptController();
    BpObject_Draw(IdMap_Lookup(Sing_805DC5F0(), AsCComSelectBase(self)->bpEntryId));
    Layout_DrawWithOpacity(AsCComSelectBase(self)->layout, kFloat_1_ComSelect);
    ((VS8032_*)self)->v14();
    ((VS8032_*)self)->v15();
    for (int i = 0; i < 4; ++i) {
        ((VS8032_*)self)->v16(i);
        ((VS8032_*)self)->v17(i);
    }
    ((VS8032_*)self)->v18();
    void* p3c = AsCComSelectBase(self)->optionDialog;
    if (p3c != 0)
        MPBase1_DrawIfInitialized(p3c);
    void* auxiliaryDrawObject = AsCComSelectBase(self)->courseClearDialog;
    if (auxiliaryDrawObject != 0) {
        ((VP44_*)auxiliaryDrawObject)->a2();
    }
}

#pragma opt_common_subs off
#pragma opt_lifetimes off
// Render a row label using its group position, layout size, and hover/selection color.
void CComSelectBase_DrawEntryText(ComputerMenuLocalView* self, int rowIndex) {
    WString entryText;
    int rowLabelGroupIds[4];
    float position[2];
    float layoutSize[2];
    ((Obj8032V*)self)->v23(rowIndex, &entryText);
    int row3LabelGroup;
    int row2LabelGroup;
    int row1LabelGroup;
    int row0LabelGroup;
    int* rowLabelGroupNames = computerMenuCourseLabelGroupNames;
    row0LabelGroup = *rowLabelGroupNames++;
    row1LabelGroup = *rowLabelGroupNames++;
    row2LabelGroup = *rowLabelGroupNames++;
    row3LabelGroup = *rowLabelGroupNames++;
    rowLabelGroupIds[0] = row0LabelGroup;
    rowLabelGroupIds[1] = row1LabelGroup;
    rowLabelGroupIds[2] = row2LabelGroup;
    rowLabelGroupIds[3] = row3LabelGroup;
    *(int*)&position[0] = 0;
    *(int*)&position[1] = 0;
    *(int*)&layoutSize[0] = 0;
    *(int*)&layoutSize[1] = 0;
    EnsureComputerMenuScriptController();
    void* entryGroup = BpObject_GetOrCreateNamedGroup(IdMap_Lookup(Sing_805DC5F0(), AsCComSelectBase(self)->bpEntryId), rowLabelGroupIds[rowIndex]);
    UIGroup_GetPosition(entryGroup, &position);
    UIGroup_GetSourceSize(entryGroup, layoutSize);
    int entryIndex = ((Obj8032Call*)self)->c11(AsCComSelectBase(self)->pageIndex, rowIndex);
    if (AsCComSelectBase(self)->selectedEntry == entryIndex) {
        CMsgDraw_SetColor(AsCComSelectBase(self)->textRenderer, kEntryTextColor);
        position[0] -= kFloat_10_ComputerMenuLabelInset;
    } else {
        if (Layout_HitTest(AsCComSelectBase(self)->layout, rowIndex, 0)) {
            CMsgDraw_SetColor(AsCComSelectBase(self)->textRenderer, kHoveredTextColor);
            position[0] -= kFloat_10_ComputerMenuLabelInset;
        } else {
            CMsgDraw_SetColor(AsCComSelectBase(self)->textRenderer, kEntryTextColor);
        }
    }
    Text_SetPosition(AsCComSelectBase(self)->textRenderer, &position);
    CMsgDraw_SetLayoutSize(AsCComSelectBase(self)->textRenderer, layoutSize);
    CMsgDraw_ResetTextState(AsCComSelectBase(self)->textRenderer);
    void* text = (entryText.flags >> 31) == 0 ? (void*)((char*)&entryText + 2) : (void*)entryText.data;
    Text_SetContent(AsCComSelectBase(self)->textRenderer, text);
    CMsgDraw_ClearFormattingAndRevealAll(AsCComSelectBase(self)->textRenderer);
    ((CMsgDraw*)AsCComSelectBase(self)->textRenderer)->drawSuppressed = 0; // 0x80322884: renderer +0x94
    Text_DrawGlyphs(AsCComSelectBase(self)->textRenderer);
}
#pragma opt_common_subs reset
#pragma opt_lifetimes reset

void* __nw__FUl(unsigned long);
void COptionDialog_Ctor(void*, int);
int MPBase1_Process(void*, float);
void Layout_SetElementPlayerFlag(void*, int, int, int);
int Layout_CollectActivated(void*, int*, int);
void* fn_8023E074_helper(void*);
int FadeState_GetState(void*);
void FadeOverlay_Init(void*);
void ResourceState_Dtor(void);
void FadeState_RequestFramebufferCapture(void*); // Latches framebuffer capture in endpoint phases.
void FadeState_StartFadeIn(void*, int);
void FadeState_StartFadeOut(void*, int);
void* InputManager_Init(void*);
void InputManager_Dtor(void);
int Input_IsActionPressed(void*, int, int);
void Sound_PlayByName(void*, char*, int, int, int, int, float, float, float);
extern char s_computerMenuDtorRegistrations[]; // Destructor registration storage for computerMenuDtorRegistrations.
struct RegBlock { int a, b, c; };
extern signed char s_fadeOverlayInitialized;
extern char s_fadeOverlay[];
extern void* g_soundManager;
extern char computerVersusMenuStrings[];
extern signed char s_inputManagerInitialized;
extern char s_gameInputManager[];
extern const float kFloat_0_ComputerMenuInputDelay; // 0: kFloat_0_ComputerMenuInputDelay.
extern const float kFloat_0_5_ComSelectBase;
extern const float kFloat_200_ComSelect;

// Ensure the update path uses its original bounded registration storage.
static inline void EnsureComputerMenuUpdateController(char* regbase) {
    if (Sing_805DC5F0() == 0) {
        if (s_bpCtrlInitialized == 0) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, (regbase + 0xC));
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
}

// Ensure the borrowed fade overlay exists without retaining it across calls.
static inline void EnsureComputerMenuFade(char* regbase) {
    if (Sing_805DC608() == 0) {
        if (s_fadeOverlayInitialized == 0) {
            FadeOverlay_Init(s_fadeOverlay);
            __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor, (regbase + 0x24));
            s_fadeOverlayInitialized = 1;
        }
        Sing_805DC608() = s_fadeOverlay;
    }
}

// Ensure the borrowed input manager exists without retaining it across calls.
static inline void EnsureComputerMenuInput(char* regbase) {
    if (Sing_805DC5C8() == 0) {
        if (s_inputManagerInitialized == 0) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, &((RegBlock*)regbase)[0].a);
            s_inputManagerInitialized = 1;
        }
        Sing_805DC5C8() = s_gameInputManager;
    }
}

// Process fade and modal UI, selection delays, activated rows, paging, cancel, and options.
// deltaTime is seconds. Return 1 on cancel or after the selected-entry delay;
// return 0 while fading, handling modals or waiting. UI IDs: 0..3 rows, 4 previous,
// 5 next. Activated results own an ID array, freed on normal exit and cancel.
// A modal/options dialog consumes its whole update and blocks base input.
int CComSelectBase_Update(ComputerMenuLocalView* self, float deltaTime) {
    // Phase 1: tick three BP objects, then gate input on the global fade.
    char* regbase = s_computerMenuDtorRegistrations;

    EnsureComputerMenuUpdateController(regbase);
    BpObject_TryInvokeCurrentCallback(IdMap_Lookup(Sing_805DC5F0(), AsCComSelectBase(self)->bpEntryId), 0);

    EnsureComputerMenuUpdateController(regbase);
    BpObject_TryInvokeCurrentCallback(IdMap_Lookup(Sing_805DC5F0(), AsCComSelectBase(self)->secondaryBpEntryId), 0);

    EnsureComputerMenuUpdateController(regbase);
    BpObject_TryInvokeCurrentCallback(IdMap_Lookup(Sing_805DC5F0(), AsCComSelectBase(self)->tertiaryBpEntryId), 0);

    EnsureComputerMenuFade(regbase);
    if (FadeState_GetState(Sing_805DC608()) != 0) {
        return 0;
    }

    // Phase 2: create/poll/release the result modal, then the independent options dialog.
    if (self->courseClearPending != 0) {
        if (self->courseClearDialog == 0) {
            Sound_PlayByName(g_soundManager, computerVersusMenuStrings + kComputerRewardSound, 0, 0, 0, 0, kFloat_0_5_ComSelectBase, kFloat_200_ComSelect, kFloat_1_ComSelect);
            ((Obj8032V*)self)->v26(&self->courseClearDialog);
        }
        void* modalObject = self->courseClearDialog;
        if (((Obj8032V*)modalObject)->v1() != 0) {
            void* obj = self->courseClearDialog;
            if (obj != 0) {
                if (obj != 0) {
                    ((Obj8032V*)obj)->v0(1);
                }
                self->courseClearDialog = 0;
            }
            self->courseClearPending = 0;
        }
        return 0;
    }

    if (AsCComSelectBase(self)->optionDialogPending != 0) {
        if (AsCComSelectBase(self)->optionDialog == 0) {
            void* temp = __nw__FUl(kOptionDialogAllocationBytes);
            if (temp != 0)
            {
                try {
                    COptionDialog_Ctor(temp, 1);
                } catch (...) {
                    __dl__FPv(temp);
                    throw;
                }
            }
            AsCComSelectBase(self)->optionDialog = temp;
        }
        if (MPBase1_Process(AsCComSelectBase(self)->optionDialog, deltaTime) != 0) {
            void* obj = AsCComSelectBase(self)->optionDialog;
            if (obj != 0) {
                if (obj != 0) {
                    ((Obj8032V*)obj)->v0(1);
                }
                AsCComSelectBase(self)->optionDialog = 0;
            }
            AsCComSelectBase(self)->optionDialogPending = 0;
        }
        return 0;
    }

    // Phase 3: tick layout and input delay. Expiry completes a selection or
    // commits a requested page and starts its fade transition.
    Layout_Update(AsCComSelectBase(self)->layout);
    float delayRemaining = AsCComSelectBase(self)->inputDelay;
    float zero = kFloat_0_ComputerMenuInputDelay;
    if (delayRemaining > zero) {
        delayRemaining = delayRemaining - deltaTime;
        AsCComSelectBase(self)->inputDelay = delayRemaining;
        if (delayRemaining > zero) {
            EnsureComputerMenuFade(regbase);
            FadeState_RequestFramebufferCapture(Sing_805DC608());
            return 0;
        }
        if (AsCComSelectBase(self)->selectedEntry != -1) {
            return 1;
        }
        int pageChanged = ((Obj8032V*)self)->v10(AsCComSelectBase(self)->requestedPageIndex);
        if (pageChanged != 0) {
            Sound_PlayByName(g_soundManager, computerVersusMenuStrings + kComputerPageChangeSound, 0, 0, 0, 0, kFloat_0_5_ComSelectBase, kFloat_200_ComSelect, kFloat_1_ComSelect);
            EnsureComputerMenuFade(regbase);
            FadeState_StartFadeIn(Sing_805DC608(), 0);
            EnsureComputerMenuFade(regbase);
            FadeState_StartFadeOut(Sing_805DC608(), kPageFadeFrames);
        }
    }

    // Phase 4: prefer activated layout IDs to global actions. A row locks the
    // selected entry; navigation requests a page and waits the same half second.
    int activatedItems[3];
    unsigned int activatedByteOffset = 0;
    activatedItems[0] = activatedByteOffset;
    activatedItems[1] = activatedByteOffset;
    activatedItems[2] = activatedByteOffset;
    if (Layout_CollectActivated(AsCComSelectBase(self)->layout, activatedItems, 0) != 0) {
        float step = kFloat_0_5_ComSelectBase;
        unsigned int i = 0;
        while (i < (unsigned int)activatedItems[1]) {
            int activatedElement = *(int*)((char*)activatedItems[0] + activatedByteOffset);
            switch (activatedElement) {
            case 0:
            case 1:
            case 2:
            case 3: {
                    // Map the requested page and activated row to a logical entry index.
                    int n = ((Obj8032V*)self)->v11(AsCComSelectBase(self)->requestedPageIndex, activatedElement);
                    AsCComSelectBase(self)->selectedEntry = n;
                    Layout_SetElementValueState(AsCComSelectBase(self)->layout, *(int*)((char*)activatedItems[0] + activatedByteOffset), kSelectedRowValueState);
                    for (int j = 0; j < 4; ++j) {
                        Layout_SetElementPlayerFlag(AsCComSelectBase(self)->layout, j, 0, kRowInputDisabledFlags);
                    }
                    AsCComSelectBase(self)->inputDelay = step;
                    break;
                }
            case kPreviousPageElement: {
                    int cur = AsCComSelectBase(self)->requestedPageIndex;
                    ((Obj8032V*)self)->v9(cur - 1);
                    Layout_SetElementPlayerFlag(AsCComSelectBase(self)->layout, *(int*)((char*)activatedItems[0] + activatedByteOffset), 0, kRowInputDisabledFlags);
                    Layout_SetElementValueState(AsCComSelectBase(self)->layout, *(int*)((char*)activatedItems[0] + activatedByteOffset), kSelectedRowValueState);
                    AsCComSelectBase(self)->inputDelay = step;
                    break;
                }
            case kNextPageElement: {
                    int cur = AsCComSelectBase(self)->requestedPageIndex;
                    ((Obj8032V*)self)->v9(cur + 1);
                    Layout_SetElementPlayerFlag(AsCComSelectBase(self)->layout, *(int*)((char*)activatedItems[0] + activatedByteOffset), 0, kRowInputDisabledFlags);
                    Layout_SetElementValueState(AsCComSelectBase(self)->layout, *(int*)((char*)activatedItems[0] + activatedByteOffset), kSelectedRowValueState);
                    AsCComSelectBase(self)->inputDelay = step;
                    break;
                }
            default:
                break;
            }
            ++i;
            activatedByteOffset += 4;
        }
    } else {
        EnsureComputerMenuInput(regbase);
        if (Input_IsActionPressed(Sing_805DC5C8(), kCancelAction, kAnyInputPlayer) != 0) {
            Sound_PlayByName(g_soundManager, computerVersusMenuStrings + kComputerCancelSound, 0, 0, 0, 0, kFloat_0_5_ComSelectBase, kFloat_200_ComSelect, kFloat_1_ComSelect);
            if (activatedItems[0] != 0) {
                activatedItems[1] -= activatedItems[1];
                appFreeUni__FPv((void*)activatedItems[0]);
            }
            return 1;
        }

        EnsureComputerMenuInput(regbase);
        if (Input_IsActionPressed(Sing_805DC5C8(), kPreviousPageAction, kAnyInputPlayer) != 0) {
            int cur = AsCComSelectBase(self)->requestedPageIndex;
            ((Obj8032V*)self)->v9(cur - 1);
            AsCComSelectBase(self)->inputDelay = kFloat_0_5_ComSelectBase;
        } else {

            EnsureComputerMenuInput(regbase);
            if (Input_IsActionPressed(Sing_805DC5C8(), kNextPageAction, kAnyInputPlayer) != 0) {
                int cur = AsCComSelectBase(self)->requestedPageIndex;
                ((Obj8032V*)self)->v9(cur + 1);
                AsCComSelectBase(self)->inputDelay = kFloat_0_5_ComSelectBase;
            } else {

                EnsureComputerMenuInput(regbase);
                if (Input_IsActionPressed(Sing_805DC5C8(), kOptionsAction, kAnyInputPlayer) != 0) {
                    AsCComSelectBase(self)->optionDialogPending = 1;
                }
            }
        }
    }

    // Phase 5: free the activated-ID array; no pointer to it escapes this update.
    if (activatedItems[0] != 0) {
        activatedItems[1] -= activatedItems[1];
        appFreeUni__FPv((void*)activatedItems[0]);
    }
    return 0;
}

} // extern "C"
