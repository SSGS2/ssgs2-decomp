// Translation unit 0x8007DB30-0x8007F694 (17 functions; boundaries inferred by decomp-toolkit from extab).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/wstring.h"
#include "game/singletons.h"
#include "classes/CNameEntryMain.h"
// The keyboard stores at most eight UTF-16 code units and uses two glyph pages.
enum NameEntryState {
    kNameEntryEditing = 0,
    kNameEntryValidate = 1,
    kNameEntryBlankDialog = 3,
    kNameEntryConfirmDialog = 4
};
enum { kNameEntryCapacity = 8, kKeyboardReady = 1,
       kDialogAccepted = 1, kDialogRejected = 2,
       kNameEntryScriptIndex = 19, kCursorBlinkPeriod = 30, kCursorBlinkHalfPeriod = 15,
       kNameEntryFadeFrames = 15, kTriangleStrip = 0x98,
       kUnicodeWideSpace = 0x3000, kAsciiSpace = 0x20,
       kNameEntryBgmOffset = 0xF };
enum KeyboardHoverState { kHoverCharacter = 0, kHoverAccept = 1, kHoverDelete = 2, kHoverPageTab = 3 };

// Layout ID map (IDs are local to each layout):
// nameLayout: 0 text bounds; 1/2/3/10 auxiliary visuals (roles unresolved),
// 4/5 page-tab rectangles; 6/7 current-page indicators; 8 delete;
// 9 accept. cursorLayout: 0 hovered-cell highlight, 1 insertion cursor.
// Each glyph-page layout uses element 0 for the page image/grid.
// Hover codes select character/accept/delete/page tab independently of editing
// state. Glyph cells are stored as one-based (column,row), two integers per unit.
// Text, glyphCells, glyphPageIndices and remapGlyph are parallel arrays.
enum NameEntryElement {
    kNameEntryTextElement = 0,
    kNameEntryFirstPageIndicator = 6,
    kNameEntrySecondPageIndicator = 7,
    kNameEntryDeleteButton = 8,
    kNameEntryConfirmButton = 9,
    kNameEntryCursorMarker = 1,
    kNameEntryFirstPageTab = 4, kNameEntrySecondPageTab = 5
};

extern "C" {

// Accept the name-entry lifecycle request unconditionally.
int CSoftKeyboardNameEntry_vfunc_5() {
    return 1;
}

struct Object {
    char pad[0xAC];
    unsigned char state;
};

// Report whether the keyboard has finished its initial layout setup.
bool CSoftKeyboardNameEntry_vfunc_4(Object* self) {
    return self->state == kKeyboardReady;
}

void __dl__FPv(void*);

// Delete warning-logo base storage when the deleting-destructor flag requests it.
void* __dt__16CWarningLogoBaseFv(void* self, int deleting) {
    if (self) {
        if (deleting > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Delete MPBase storage when the deleting-destructor flag requests it.
void* __dt__7MPBase1Fv(void* self, int deleting) {
    if (self) {
        if (deleting > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

struct VirtualObject {
    virtual void slot08();
    virtual void slot0C();
    virtual int update();
    virtual void finish();
    virtual int ready();
};
struct Resource {
    virtual ~Resource();
};
struct Owner {
    char pad00[0x20];
    VirtualObject* keyboard;
    char pad24[0x828];
    VirtualObject* messageDialog;
    int state;
    Resource* packedTexture;
};

int CPackTextureResource_IsLoadFinished(void*);
extern char nameEntryAndStringErrorStrings[];
extern void* g_soundManager;
void SoundManager_RequestBgmChange(void*, char*, int, int);

// Wait for the packed texture resource before reporting keyboard readiness.
int CNameEntryMain_IsKeyboardReady(Owner* self) {
    if (self->packedTexture && !CPackTextureResource_IsLoadFinished(self->packedTexture)) {
        return 0;
    }
    return self->keyboard->ready();
}

struct Allocation {
    void* data;
};

// Delete the wrapped allocation and optionally free the wrapper.
void* Allocation_Dtor(Allocation* self, int deleting) {
    if (self) {
        if (self->data) {
            __dl__FPv(self->data);
            self->data = 0;
        }
        if (deleting > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Release the packed texture resource and request the tutorial background music.
void CNameEntryMain_StartMenu(Owner* self) {
    if (self->packedTexture) {
        delete self->packedTexture;
        self->packedTexture = 0;
    }
    SoundManager_RequestBgmChange(g_soundManager, nameEntryAndStringErrorStrings + kNameEntryBgmOffset, 0, 1);
}

void CPackTextureResource_PollLoad(void*);

// Update the keyboard and poll the optional packed texture load.
void CNameEntryMain_UpdateWhileWaiting(Owner* self) {
    self->keyboard->update();
    if (self->packedTexture) {
        CPackTextureResource_PollLoad(self->packedTexture);
    }
}

// Finish keyboard input and the blank-name dialog when it is active.
void CNameEntryMain_Draw(Owner* self) {
    self->keyboard->finish();
    if (self->state == kNameEntryBlankDialog) {
        self->messageDialog->finish();
    }
}

} // extern "C"

extern "C" {
extern char __vt__14CNameEntryMain[];
struct CleanupOwner {
    void* vtable;
    char pad04[0x1C];
    Resource* keyboard;
    char pad24[0x830];
    Resource* packedTexture;
};
// Delete the keyboard and packed texture resource, then optionally free the actor.
void* CNameEntryMain_dtor(CleanupOwner* self, int deleting) {
    if (self) {
        self->vtable = __vt__14CNameEntryMain;
        if (self->keyboard) {
            delete self->keyboard;
            self->keyboard = 0;
        }
        if (self->packedTexture) {
            delete self->packedTexture;
            self->packedTexture = 0;
        }
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}
extern signed char s_bpCtrlInitialized;
extern char s_bpCtrl[];
char s_bpCtrlDtorRegistration_804E9130[12];
char lbl_804E913C[12]; // Destructor-registration record for the shared BP controller handle.
char s_fadeOverlayDtorRegistration_8007DB30[12];
char s_nowReloadingDtorRegistration_804E9154[12];
void __ct__7CBpCtrlFv(void*);
void CBpCtrl_dtor(void*, int);
void __register_global_object(void*, void (*)(void*, int), void*);
void CBpCtrl_DeleteEntryById(void*, int);
struct Handle { int value; };
// Remove the registered UI handle, mark it invalid and optionally delete the wrapper.
void* CBpCtrlHandle_Dtor(Handle* self, int deleting) {
    if (self) {
        if (self->value != -1) {
            if (!Sing_805DC5F0()) {
                if (!s_bpCtrlInitialized) {
                    __ct__7CBpCtrlFv(s_bpCtrl);
                    __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_804E9130);
                    s_bpCtrlInitialized = 1;
                }
                Sing_805DC5F0() = s_bpCtrl;
            }
            CBpCtrl_DeleteEntryById(Sing_805DC5F0(), self->value);
            self->value = -1;
        }
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}
int CSoftKeyboard_GetTextLength(void*);
void* __nwa__FUl(unsigned int);
void TextInput_CopyWideText(void*, unsigned short*, int);
void wcsncpy(void*, unsigned short*, int);
void __dla__FPv(void*);
// Returns 1 iff a nonblank code unit exists; success copies the entire name,
// including surrounding spaces and its terminator, into enteredName. This is
// a mutating validation step. The keyboard must enforce the eight-unit limit.
// enteredName names the complete 0x828-byte pending player record; only its
// leading text is replaced here. Submission later copies the record as a whole.
// Accept text containing a non-space code unit and copy it into the pending player record.
int NameEntry_CopyNonBlankTextToPendingRecord(Owner* self) {
    unsigned short* buffer = (unsigned short*)__nwa__FUl((CSoftKeyboard_GetTextLength(self->keyboard) + 1) * 2);
    TextInput_CopyWideText(self->keyboard, buffer, 0);
    int textLength = CSoftKeyboard_GetTextLength(self->keyboard);
    unsigned short* character = buffer;
    for (int i = 0; i < textLength; ++i, ++character) {
        if (*character != kUnicodeWideSpace && *character != 0 && *character != kAsciiSpace) {
            wcsncpy(((CNameEntryMain*)self)->pendingPlayerRecord, buffer, CSoftKeyboard_GetTextLength(self->keyboard) + 1);
            if (buffer) __dla__FPv(buffer);
            return 1;
        }
    }
    if (buffer) __dla__FPv(buffer);
    return 0;
}
}

extern "C" {
extern char s_nameEntryPanelPath[], s_nameEntryFirstGlyphPageLayoutPath[], s_nameEntrySecondGlyphPageLayoutPath[], s_nameEntryTextLayoutPath[], s_nameEntryCursorLayoutPath[];
extern char s_nameEntryFirstGlyphPageLayoutDefaults[], s_nameEntrySecondGlyphPageLayoutDefaults[], s_nameEntryTextLayoutDefaults[], s_nameEntryCursorLayoutDefaults[], __vt__22CSoftKeyboardNameEntry[];
void CSoftKeyboard_InitHoverOffsets(void*);
void CBpCtrl_CreateEntry(void*, char*, void*, int);
void Layout_InitStandard(void*, int, char*, int, char*);
void Layout_Destroy(void*, int);
void SoftKeyboard_dtor(void*, int);
struct NameEntry {
    void* vtable;
    char pad04[0xAC];
    int uiEntryId;
    char padB4[0x10];
    void* firstGlyphPage;
    void* secondGlyphPage;
    void* nameLayout;
    void* cursorLayout;
};
// Initialize the base keyboard and register its panel, glyph pages, name and cursor layouts.
int CSoftKeyboardNameEntry_InitPanelAndLayouts(NameEntry* self) {
    CSoftKeyboard_InitHoverOffsets(self);
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_804E9130);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    CBpCtrl_CreateEntry(Sing_805DC5F0(), s_nameEntryPanelPath, &self->uiEntryId, 0);
    Layout_InitStandard(self->firstGlyphPage, 0, s_nameEntryFirstGlyphPageLayoutDefaults, 1, s_nameEntryFirstGlyphPageLayoutPath);
    Layout_InitStandard(self->secondGlyphPage, 0, s_nameEntrySecondGlyphPageLayoutDefaults, 1, s_nameEntrySecondGlyphPageLayoutPath);
    Layout_InitStandard(self->nameLayout, 0, s_nameEntryTextLayoutDefaults, 11, s_nameEntryTextLayoutPath);
    Layout_InitStandard(self->cursorLayout, 0, s_nameEntryCursorLayoutDefaults, 2, s_nameEntryCursorLayoutPath);
    return 1;
}
// Release each keyboard layout and its registered panel before base destruction.
void* CSoftKeyboardNameEntry_dtor(NameEntry* self, int deleting) {
    if (self) {
        void* firstGlyphPage = self->firstGlyphPage;
        *(void* volatile*)self = __vt__22CSoftKeyboardNameEntry;
        if (firstGlyphPage) { Layout_Destroy(firstGlyphPage, 1); self->firstGlyphPage = 0; }
        if (self->secondGlyphPage) { Layout_Destroy(self->secondGlyphPage, 1); self->secondGlyphPage = 0; }
        if (self->nameLayout) { Layout_Destroy(self->nameLayout, 1); self->nameLayout = 0; }
        if (self->cursorLayout) { Layout_Destroy(self->cursorLayout, 1); self->cursorLayout = 0; }
        if (self->uiEntryId != -1) {
            if (!Sing_805DC5F0()) {
                if (!s_bpCtrlInitialized) {
                    __ct__7CBpCtrlFv(s_bpCtrl);
                    __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_804E9130);
                    s_bpCtrlInitialized = 1;
                }
                Sing_805DC5F0() = s_bpCtrl;
            }
            CBpCtrl_DeleteEntryById(Sing_805DC5F0(), self->uiEntryId);
            self->uiEntryId = -1;
        }
        SoftKeyboard_dtor(self, 0);
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}
void __ct__19CPangyaCommonDialogFPvi(void*, int, int);
extern signed char s_saveDataInitialized;
extern char s_saveData[], s_nameEntryBlankNameMessage[];
void SaveData_Ctor(void*);
void SaveData_SetCurrentRecord(void*, void*);
void CPangyaCommonDialog_SetText(void*, char*);
}
struct Message {

    virtual ~Message();
    virtual void slot0C();
    virtual void update(float);
    int pad04;
    int state;
    char pad0C[0x38];
};
struct MessageAllocation {
    char data[0x44];
    // Construct a common dialog with the existing opaque style/mode arguments.
    MessageAllocation(int a, int b) { __ct__19CPangyaCommonDialogFPvi(this, a, b); }
};
extern "C" float kFloat_0_MessageCursor;
// Return 1 when a name has been submitted or confirmation accepted, 0 while
// editing/waiting. The actor owns messageDialog during states 3 and 4.
// Blank-dialog acceptance returns to editing; it never submits blank text.
// Advance editing/dialog states and submit a nonblank player name to save data.
extern "C" int CNameEntryMain_UpdateStateMachine(Owner* self) {
    switch (self->state) {
    // Process one owner state; keyboard state 0/1 is a separate initialization flag.
    case kNameEntryEditing:
        if (!self->keyboard->update()) self->state = kNameEntryValidate;
        break;
    case kNameEntryValidate:
        if (NameEntry_CopyNonBlankTextToPendingRecord(self)) {
            if (!Sing_805DC5E0()) {
                if (!s_saveDataInitialized) {
                    SaveData_Ctor(s_saveData);
                    s_saveDataInitialized = 1;
                }
                Sing_805DC5E0() = s_saveData;
            }
            SaveData_SetCurrentRecord(Sing_805DC5E0(), ((CNameEntryMain*)self)->pendingPlayerRecord);
            return 1;
        }
        self->messageDialog = (VirtualObject*)new MessageAllocation(3, 10);
        CPangyaCommonDialog_SetText(self->messageDialog, s_nameEntryBlankNameMessage);
        self->state = kNameEntryBlankDialog;
        break;
    case kNameEntryBlankDialog:
        ((Message*)self->messageDialog)->update(kFloat_0_MessageCursor);
        if (((Message*)self->messageDialog)->state == kDialogAccepted) {
            *(volatile int*)&self->state = kNameEntryEditing;
            if (self->messageDialog) { delete (Message*)self->messageDialog; self->messageDialog = 0; }
        }
        break;
    case kNameEntryConfirmDialog:
        ((Message*)self->messageDialog)->update(kFloat_0_MessageCursor);
        if (((Message*)self->messageDialog)->state == kDialogAccepted) {
            if (self->messageDialog) { delete (Message*)self->messageDialog; self->messageDialog = 0; }
            return 1;
        } else if (((Message*)self->messageDialog)->state == kDialogRejected) {
            *(volatile int*)&self->state = kNameEntryEditing;
            if (self->messageDialog) { delete (Message*)self->messageDialog; self->messageDialog = 0; }
        }
        break;
    }
    return 0;
}

extern "C" {
    struct EntryUpdate {
        char pad00[0x34];
        int selectedCharacter;
        int glyphPageIndex;
        int characterCount;
        char pad40[0x5C];
        int excludedCharacter;
        char padA0[0xC];
        unsigned char state;
        char padAD[3];
        int uiEntryId;
        int padB4;
        float textOriginX;
        float textOriginY;
        int padC0;
        void* firstGlyphPage;
        void* secondGlyphPage;
        void* nameLayout;
        void* cursorLayout;
    };
    struct EntryRect {
        int x, y, z, w;
    };
    struct EntryDesc {
        int x, y, z, w, a, b, c;
    };
    extern EntryDesc s_nameEntryFirstPageDescription, s_nameEntrySecondPageDescription;
    extern signed char s_bpCtrlHandleInitialized;
    extern int s_bpCtrlHandle[2];
    extern signed char s_nowReloadingInitialized;
    extern int s_nowReloading[2];
    extern char s_nameEntryPanelScriptName[];
    int CBpCtrl_PollAllEntriesLoaded(void*);
    int Layout_PollLoadComplete(void*);
    void* IdMap_Lookup(void*, int);
    void BpObject_SetEnabled(void*, int);
    void CBpCtrlHandle_SetScriptIndex(void*, int);
    void SoftKeyboard_AllocateInputBuffers(void*, int);
    void Layout_ReadElementRect(void*, int, EntryRect*);
    void SoftKeyboard_SetRectangle(void*, int, EntryRect*);
    int CSoftKeyboardNameEntry_AppendEntry(void*, EntryDesc*);
    void SoftKeyboard_SetPageIndex(void*, int);
    void Layout_SetElementEnabled(void*, int, int);
    void Layout_SetElementSecondaryState(void*, int, int);
    void CNowReloading_Init(void*);
    void CNowReloading_dtor(void*, int);
    void CB2dPlayerHandle_RequestPendingScript(void*);
    void Layout_GetElementSize(void*, int, void*);
    void Layout_GetElementPosition(void*, int, void*);
    int SoftKeyboard_HitTestRectangle(void*, int, int, int, int);
    void BpObject_StartScriptByName(void*, char*, int);
    int CSoftKeyboard_UpdateInput(void*);
    void Layout_Update(void*);
    // Set up loaded keyboard layouts, update pointer selection and switch the visible glyph page.
    int CSoftKeyboardNameEntry_UpdateLayoutsAndInput(EntryUpdate* self) {
        int result=1;
        void* initial=Sing_805DC5F0();
        extern char lbl_804E913C[], s_nowReloadingDtorRegistration_804E9154[];
        if (!initial) {
            if (!s_bpCtrlInitialized) {
                __ct__7CBpCtrlFv(s_bpCtrl);
                __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_804E9130);
                s_bpCtrlInitialized=1;
            }
            Sing_805DC5F0()=s_bpCtrl;
        }
        if (CBpCtrl_PollAllEntriesLoaded(Sing_805DC5F0()) &&
            Layout_PollLoadComplete(self->firstGlyphPage) &&
            Layout_PollLoadComplete(self->secondGlyphPage) &&
            Layout_PollLoadComplete(self->nameLayout) &&
            Layout_PollLoadComplete(self->cursorLayout)) {
            if (Sing_805DC5F0() == 0) {
                if (!s_bpCtrlInitialized) {
                    __ct__7CBpCtrlFv(s_bpCtrl);
                    __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_804E9130);
                    s_bpCtrlInitialized=1;
                }
                Sing_805DC5F0()=s_bpCtrl;
            }
            // All five loads are ready; perform one-time input and rectangle setup.
            BpObject_SetEnabled(IdMap_Lookup(Sing_805DC5F0(), self->uiEntryId), 1);
            if (self->state==0) {
                if (!Sing_805DC5F8()) {
                    if (!s_bpCtrlHandleInitialized) {
                        s_bpCtrlHandle[1]=-1;
                        s_bpCtrlHandle[0]=-1;
                        __register_global_object(s_bpCtrlHandle,(void (*)(void*, int))CBpCtrlHandle_Dtor, lbl_804E913C);
                        s_bpCtrlHandleInitialized=1;
                    }
                    Sing_805DC5F8()=s_bpCtrlHandle;
                }
                CBpCtrlHandle_SetScriptIndex(Sing_805DC5F8(), kNameEntryScriptIndex);
                SoftKeyboard_AllocateInputBuffers(self, kNameEntryCapacity);
                EntryRect rect;
                Layout_ReadElementRect(self->nameLayout, kNameEntryTextElement, &rect);
                SoftKeyboard_SetRectangle(self, 0, &rect);
                Layout_ReadElementRect(self->nameLayout, kNameEntryConfirmButton, &rect);
                SoftKeyboard_SetRectangle(self, 1, &rect);
                Layout_ReadElementRect(self->nameLayout, kNameEntryDeleteButton, &rect);
                SoftKeyboard_SetRectangle(self, 2, &rect);
                Layout_ReadElementRect(self->nameLayout, kNameEntryFirstPageTab, &rect);
                EntryDesc d1=s_nameEntryFirstPageDescription;
                *(EntryRect*)&d1=rect;
                SoftKeyboard_SetPageIndex(self, CSoftKeyboardNameEntry_AppendEntry(self, &d1));
                Layout_ReadElementRect(self->nameLayout, kNameEntrySecondPageTab, &rect);
                EntryDesc d2=s_nameEntrySecondPageDescription;
                *(EntryRect*)&d2=rect;
                CSoftKeyboardNameEntry_AppendEntry(self, &d2);
                Layout_SetElementEnabled(self->firstGlyphPage, 0, 0);
                Layout_SetElementEnabled(self->secondGlyphPage, 0, 0);
                Layout_SetElementEnabled(self->nameLayout, kNameEntrySecondPageIndicator, 0);
                Layout_SetElementEnabled(self->nameLayout, kNameEntryTextElement, 0);
                Layout_SetElementEnabled(self->nameLayout, 1, 0);
                Layout_SetElementEnabled(self->nameLayout, 2, 0);
                Layout_SetElementEnabled(self->nameLayout, 3, 0);
                Layout_SetElementEnabled(self->nameLayout, 10, 0);
                Layout_SetElementSecondaryState(self->nameLayout, 7, 0);
                Layout_SetElementSecondaryState(self->nameLayout, 0, 0);
                Layout_SetElementSecondaryState(self->nameLayout, 1, 0);
                Layout_SetElementSecondaryState(self->nameLayout, 2, 0);
                Layout_SetElementSecondaryState(self->nameLayout, 3, 0);
                Layout_SetElementSecondaryState(self->nameLayout, 10, 0);
                for(int i=0;i<=1;++i) Layout_SetElementSecondaryState(self->cursorLayout, i, 0);
                if (Sing_805DC610() == 0) {
                    if (!s_nowReloadingInitialized) {
                        CNowReloading_Init(s_nowReloading);
                        __register_global_object(s_nowReloading, CNowReloading_dtor, s_nowReloadingDtorRegistration_804E9154);
                        s_nowReloadingInitialized=1;
                    }
                    Sing_805DC610()=s_nowReloading;
                }
                CB2dPlayerHandle_RequestPendingScript(Sing_805DC610());
                self->state=1;
                return result;
            }
            // Hit-test the insertion positions, including the slot after the
            // final character. The square uses cursor width on both axes.
            float size[2]= {
                0, 0
            };
            Layout_GetElementSize(self->cursorLayout, 1, size);
            if (kFloat_0_MessageCursor==self->textOriginX) Layout_GetElementPosition(self->cursorLayout, 1,(void*)&self->textOriginX);
            for(int i=0;i<1+self->characterCount;++i) {
                if(SoftKeyboard_HitTestRectangle(self,(int)(i*size[0]+self->textOriginX),(int)(self->textOriginY-size[0]),(int)(i*size[0]+(self->textOriginX+size[0])),(int)self->textOriginY)) {
                    if(i!=self->excludedCharacter) self->selectedCharacter=i;
                }
            }
            if (0 == Sing_805DC5F0()) {
                if (!s_bpCtrlInitialized) {
                    __ct__7CBpCtrlFv(s_bpCtrl);
                    __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_804E9130);
                    s_bpCtrlInitialized=1;
                }
                Sing_805DC5F0()=s_bpCtrl;
            }
            BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), self->uiEntryId), s_nameEntryPanelScriptName, 1);
            result=CSoftKeyboard_UpdateInput(self);
            // Switch both the glyph image and its paired page indicator together.
            switch(self->glyphPageIndex) {
                case 0:
                Layout_SetElementEnabled(self->firstGlyphPage, 0, 1);
                Layout_SetElementEnabled(self->secondGlyphPage, 0, 0);
                Layout_SetElementEnabled(self->nameLayout, kNameEntryFirstPageIndicator, 1);
                Layout_SetElementEnabled(self->nameLayout, kNameEntrySecondPageIndicator, 0);
                break;
                case 1:
                Layout_SetElementEnabled(self->firstGlyphPage, 0, 0);
                Layout_SetElementEnabled(self->secondGlyphPage, 0, 1);
                Layout_SetElementEnabled(self->nameLayout, kNameEntryFirstPageIndicator, 0);
                Layout_SetElementEnabled(self->nameLayout, kNameEntrySecondPageIndicator, 1);
                break;
                default: break;
            }
        }
        Layout_Update(self->firstGlyphPage);
        Layout_Update(self->secondGlyphPage);
        Layout_Update(self->nameLayout);
        Layout_Update(self->cursorLayout);
        return result;
    }
}
extern "C" {
    extern signed char s_fadeOverlayInitialized;
    extern char s_fadeOverlay[], s_fadeOverlayDtorRegistration_8007DB30[];
    void FadeOverlay_Init(void*);
    void ResourceState_Dtor(void*, int);
    void FadeState_StartFadeOut(void*, int);
    void BpObject_TryInvokeCurrentCallback(void*, int);
    void BpObject_Draw(void*);
    void Layout_DrawWithOpacity(void*, float);
    void Layout_SetElementPosition(void*, int, void*);
    void Layout_SetElementColor(void*, int, unsigned int);
    void appFreeUni__FPv(void*);
    void WString_ReplaceRepeatedChar(void*, unsigned int, unsigned int, unsigned int, unsigned short);
    int wcscmp(unsigned short*, unsigned short*);
    struct DrawTexture {
        virtual void slot08();
        virtual void slot0C();
        virtual void slot10();
        virtual void bind();
    };
    DrawTexture* Layout_GetElementValue(void*, int, int);
    void BpSprite_GetTextureDescriptor(void*, void*);
    int GX_GetPrimitiveCount(int, int);
    void DrawPrimitiveUP(int, int, void*, int);
    struct DrawVec {
        float x, y;
    };
    struct DrawVertex {
        float x, y, z;
        unsigned int color;
        float u, v;
    };
    extern DrawVertex s_nameEntryGlyphQuadTemplate[4];
    extern float kFloat_1_NameEntry;
    struct GlyphGrid {
        int a;
        unsigned short* chars;
        int b;
        int columns, rows;
        int c;
    };
    struct GlyphPage {
        char pad[0x14];
        int columns, rows;
    };
    struct GlyphPos {
        int x, y;
    };
    struct DrawEntry {
        char pad00[12];
        unsigned int text[3];
        int* glyphCells;
        int* glyphPageIndices;
        unsigned char* remapGlyph;
        int hoverState, hoverX, hoverY, previousSelection, selectedCharacter, glyphPageIndex, characterCount;
        char pad40[0x40];
        GlyphPage* glyphPageMetrics;
        char pad84[8];
        GlyphGrid* glyphGrids;
        char pad90[0x20];
        int uiEntryId;
        int padB4;
        volatile float textOriginX, textOriginY;
        int cursorBlinkFrame;
        void* firstGlyphPage;
        void* secondGlyphPage;
        void* nameLayout;
        void* cursorLayout;
    };
}
// Select inline or heap UTF-16 characters using the wide string's ownership bit.
inline unsigned short* DrawString_data(WString& s) {
    if((s.flags>>31)==0) return (unsigned short*)((char*)&s+2);
    else return (unsigned short*)s.data;
}
// Read the inline or heap wide-string length in code units.
inline unsigned int DrawString_size(WString& s) {
    if((s.flags>>31)==0) return (*(unsigned char*)&s & 0x7f);
    else return s.length;
}
// Resize a temporary UTF-16 string to one code unit for glyph comparisons.
inline void DrawString_EnsureOneCodeUnit(WString& s) {
    unsigned int n=DrawString_size(s);
    if(n<1) WString_ReplaceRepeatedChar(&s, n, 0, 1-n, 0);
    else WString_ReplaceRepeatedChar(&s, 1, n-1, 0, 0);
}
#pragma push
#pragma opt_loop_invariants off
// Draw the keyboard, remap selected glyph cells and blink the insertion cursor.
extern "C" int CSoftKeyboardNameEntry_DrawKeyboardAndText(DrawEntry* self) {
    if(!Sing_805DC5F0()) {
        if(!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_804E9130);
            s_bpCtrlInitialized=1;
        }
        Sing_805DC5F0()=s_bpCtrl;
    }
    if(CBpCtrl_PollAllEntriesLoaded(Sing_805DC5F0()) && Layout_PollLoadComplete(self->firstGlyphPage) && Layout_PollLoadComplete(self->secondGlyphPage) && Layout_PollLoadComplete(self->nameLayout) && Layout_PollLoadComplete(self->cursorLayout)) {
        if(!Sing_805DC608()) {
            if(!s_fadeOverlayInitialized) {
                FadeOverlay_Init(s_fadeOverlay);
                __register_global_object(s_fadeOverlay, ResourceState_Dtor, s_fadeOverlayDtorRegistration_8007DB30);
                s_fadeOverlayInitialized=1;
            }
            Sing_805DC608()=s_fadeOverlay;
        }
        FadeState_StartFadeOut(Sing_805DC608(), kNameEntryFadeFrames);
        if(!Sing_805DC5F0()) {
            if(!s_bpCtrlInitialized) {
                __ct__7CBpCtrlFv(s_bpCtrl);
                __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_804E9130);
                s_bpCtrlInitialized=1;
            }
            Sing_805DC5F0()=s_bpCtrl;
        }
        BpObject_TryInvokeCurrentCallback(IdMap_Lookup(Sing_805DC5F0(), self->uiEntryId), 0);
        if(!Sing_805DC5F0()) {
            if(!s_bpCtrlInitialized) {
                __ct__7CBpCtrlFv(s_bpCtrl);
                __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_804E9130);
                s_bpCtrlInitialized=1;
            }
            Sing_805DC5F0()=s_bpCtrl;
        }
        BpObject_Draw(IdMap_Lookup(Sing_805DC5F0(), self->uiEntryId));
        Layout_DrawWithOpacity(self->firstGlyphPage, kFloat_1_NameEntry);
        Layout_DrawWithOpacity(self->secondGlyphPage, kFloat_1_NameEntry);
        Layout_DrawWithOpacity(self->nameLayout, kFloat_1_NameEntry);
        // Place and blink the insertion cursor in screen pixels.
        unsigned int textureSize[8];
        DrawVec pos= {
            0, 0
        }
        , size= {
            0, 0
        };
        Layout_GetElementPosition(self->cursorLayout, 1, &pos);
        if(kFloat_0_MessageCursor==self->textOriginX) Layout_GetElementPosition(self->cursorLayout, 1,(void*)&self->textOriginX);
        Layout_GetElementSize(self->cursorLayout, 1, &size);
        if(self->previousSelection>self->selectedCharacter) pos.x-=size.x*(self->previousSelection-self->selectedCharacter);
        else if(self->previousSelection<self->selectedCharacter) pos.x+=size.x*(self->selectedCharacter-self->previousSelection);
        Layout_SetElementPosition(self->cursorLayout, 1, &pos);
        self->previousSelection=self->selectedCharacter;
        self->cursorBlinkFrame++;
        if(self->cursorBlinkFrame%kCursorBlinkPeriod>=kCursorBlinkHalfPeriod) Layout_SetElementColor(self->cursorLayout, 1, 0xffffff00);
        else Layout_SetElementColor(self->cursorLayout, 1, 0xffffffff);
        pos.x=self->hoverX;
        pos.y=self->hoverY;
        if((unsigned int)(self->hoverState-1)<=2) Layout_SetElementEnabled(self->cursorLayout, 0, 0);
        else if(self->hoverState==kHoverCharacter) {
            Layout_SetElementEnabled(self->cursorLayout, 0, 1);
            Layout_SetElementPosition(self->cursorLayout, 0, &pos);
        } else Layout_SetElementEnabled(self->cursorLayout, 0, 0);
        DrawVec fontSize= {
            0, 0
        }
        , alternateSize= {
            0, 0
        }
        , fontPos= {
            0, 0
        }
        , alternatePos= {
            0, 0
        }
        , uv0= {
            0, 0
        }
        , uv1= {
            0, 0
        };
        // Draw entered code units as textured quads. Texture descriptor width/
        // height convert cell pixel coordinates to normalized UV coordinates.
        int i=0, textIndex=0;
        for(;i<self->characterCount;++i,++textIndex) {
            switch(self->glyphPageIndices[i]) {
                case 0:
                Layout_GetElementValue(self->firstGlyphPage, 0, 0)->bind();
                Layout_GetElementPosition(self->firstGlyphPage, 0, &fontPos);
                Layout_GetElementSize(self->firstGlyphPage, 0, &fontSize);
                BpSprite_GetTextureDescriptor(Layout_GetElementValue(self->firstGlyphPage, 0, 0), textureSize);
                break;
                case 1:
                Layout_GetElementValue(self->secondGlyphPage, 0, 0)->bind();
                Layout_GetElementPosition(self->secondGlyphPage, 0, &fontPos);
                Layout_GetElementSize(self->secondGlyphPage, 0, &fontSize);
                BpSprite_GetTextureDescriptor(Layout_GetElementValue(self->secondGlyphPage, 0, 0), textureSize);
                break;
            }
            // Remapping searches the page's UTF-16 grid and writes one-based
            // cell coordinates. Preserve repeated resize and loop-break behavior:
            // candidate is not explicitly resized here and only the inner loop
            // breaks on a match. alternateSize/alternatePos stay zero in this path.
            if((unsigned int)self->glyphPageIndices[i]<=1 && self->remapGlyph[i]==1) {
                WString character, candidate;
                DrawString_EnsureOneCodeUnit(character);
                DrawString_EnsureOneCodeUnit(character);
                int row, col;
                unsigned short* chars=self->glyphGrids[self->glyphPageIndices[i]].chars;
                unsigned short* text=!(self->text[0] >> 31) ? (unsigned short*)((char*)&self->text + sizeof(unsigned short)) : (unsigned short*)self->text[2];
                *DrawString_data(character)=text[textIndex];
                for(row=0;row<self->glyphGrids[self->glyphPageIndices[i]].rows;++row) {
                    for(col=0;col<self->glyphGrids[self->glyphPageIndices[i]].columns;++col) {
                        *DrawString_data(candidate)=chars[col+row*self->glyphGrids[self->glyphPageIndices[i]].columns];
                        unsigned short* candidateText=DrawString_data(candidate);
                        unsigned short* characterText=DrawString_data(character);
                        if(wcscmp(characterText, candidateText)==0) {
                            self->glyphCells[i+i+1]=row+1;
                            self->glyphCells[i+i]=col+1;
                            break;
                        }
                        if(col+1==self->glyphCells[i+i] && row+1==self->glyphCells[i+i+1]) break;
                    }
                }
                uv0.x=(self->glyphCells[i+i]-1)*(alternateSize.x/textureSize[0]/self->glyphGrids[self->glyphPageIndices[i]].columns)+(alternatePos.x-fontPos.x)/textureSize[0];
                uv0.y=(self->glyphCells[i+i+1]-1)*(fontSize.y/textureSize[1]/self->glyphGrids[self->glyphPageIndices[i]].rows);
                uv1.x=self->glyphCells[i+i]*(alternateSize.x/textureSize[0]/self->glyphGrids[self->glyphPageIndices[i]].columns)+(alternatePos.x-fontPos.x)/textureSize[0];
                uv1.y=self->glyphCells[i+i+1]*(fontSize.y/textureSize[1]/self->glyphGrids[self->glyphPageIndices[i]].rows);
            } else {
                uv0.x=(self->glyphCells[i+i]-1)*(fontSize.x/textureSize[0]/self->glyphPageMetrics[self->glyphPageIndices[i]].columns);
                uv0.y=(self->glyphCells[i+i+1]-1)*(fontSize.y/textureSize[1]/self->glyphPageMetrics[self->glyphPageIndices[i]].rows);
                uv1.x=self->glyphCells[i+i]*(fontSize.x/textureSize[0]/self->glyphPageMetrics[self->glyphPageIndices[i]].columns);
                uv1.y=self->glyphCells[i+i+1]*(fontSize.y/textureSize[1]/self->glyphPageMetrics[self->glyphPageIndices[i]].rows);
            }
            // Reconstruct the screen-space quad from UV corners; glyph height uses size.x.
            struct Quad {
                DrawVertex vertices[4];
            };
            Quad quad=*(Quad*)s_nameEntryGlyphQuadTemplate;
            quad.vertices[0].x=i*size.x+self->textOriginX;
            quad.vertices[0].y=self->textOriginY-size.x;
            quad.vertices[0].u=uv0.x;
            quad.vertices[0].v=uv0.y;
            quad.vertices[1].x=i*size.x+(self->textOriginX+size.x);
            quad.vertices[1].y=self->textOriginY-size.x;
            quad.vertices[1].u=uv1.x;
            quad.vertices[1].v=uv0.y;
            quad.vertices[2].x=i*size.x+self->textOriginX;
            quad.vertices[2].y=self->textOriginY;
            quad.vertices[2].u=uv0.x;
            quad.vertices[2].v=uv1.y;
            quad.vertices[3].x=i*size.x+(self->textOriginX+size.x);
            quad.vertices[3].y=self->textOriginY;
            quad.vertices[3].u=uv1.x;
            quad.vertices[3].v=uv1.y;
            DrawPrimitiveUP(kTriangleStrip, GX_GetPrimitiveCount(4, kTriangleStrip), &quad, sizeof(DrawVertex));
        }
        // Cursor/highlight overlay is drawn last, above the name glyphs.
        Layout_DrawWithOpacity(self->cursorLayout, kFloat_1_NameEntry);
    }
    return 1;
}
#pragma pop
