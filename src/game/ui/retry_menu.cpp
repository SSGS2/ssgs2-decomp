#include <decomp/static_init.h>
#include "classes/RetryMenuFields802618B8.h"
// Translation unit 0x802618B8-0x80261D30 (8 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

// Layout indices are arranged in button, idle, and selected-image groups.
enum RetryMenuElement {
    kRetryPrimaryButton = 0, kRetrySecondButton = 1, kRetryThirdButton = 2, kRetryFourthButton = 3,
    kRetryIdleSecondAlternate = 4, kRetryIdleSecondDefault = 5, kRetryIdlePrimary = 6,
    kRetryIdleThird = 7, kRetryIdleFourth = 8,
    kRetrySelectedPrimaryButton = 9, kRetrySelectedSecondButton = 10,
    kRetrySelectedThirdButton = 11, kRetrySelectedFourthButton = 12,
    kRetrySelectedSecondAlternate = 13, kRetrySelectedSecondDefault = 14,
    kRetrySelectedPrimary = 15, kRetrySelectedThird = 16, kRetrySelectedFourth = 17,
    kRetrySelectedBegin = kRetrySelectedPrimaryButton, kRetryElementCount = 18
};
// Result codes are consumed by the caller; names identify their originating menu choice.
enum RetryMenuResult { kRetryNoChoice = 0, kRetrySecondAlternateChoice = 1,
    kRetrySecondDefaultChoice = 2, kRetryThirdChoice = 3, kRetryPrimaryChoice = 4, kRetryFourthChoice = 5 };
// Layout_Init stores its second argument at controller +0x48; a rendering-layer role is unproven.
enum { kRetrySelectionDelayUpdates = 9, kRetryLayoutInitValue = 5, kRetryConfirmSoundOffset = 0xD };
enum { kRetryDefaultMode = 0 };

// Control       button/selected button   idle/selected image    result
// primary                0 / 9                  6 / 15             4
// second, mode 0         1 / 10                 5 / 14             2
// second, other mode     1 / 10                 4 / 13             1
// third                  2 / 11                 7 / 16             3
// fourth                 3 / 12                 8 / 17             5
// Mode 0 hides the third choice. All other modes use the alternate second image.
// CGolfRule_ProcessPostRoundMenus passes 0 for the single-player result path,
// 1 for fight/party paths. Exact on-screen labels are not established here.
// CGolfRule_ApplyPostRoundChoice consumes result: 4 restarts the round; 2 enters
// the single-player save/return flow; 1 enters fight/party with request arg 0;
// 3 enters fight/party with arg 1 (or training in mode 5); 5 exits the mode.
// state (+0x20) stores this result, not an animation phase; counter counts update calls.

extern "C" {
void appFreeUni__FPv(void*);

// The raw vector's explicit normal cleanup releases this unwind guard first.
struct ActivatedUnwindCleanup {
    volatile unsigned int* words;
    bool active;
    // Borrow the raw [data, count, capacity] vector header for exception cleanup.
    explicit ActivatedUnwindCleanup(volatile void* vector)
        : words((volatile unsigned int*)vector), active(true) {}
    // Suppress guard cleanup when the normal path frees the same allocation.
    void release() { active = false; }
    // Release only vector storage; button IDs are values, not owned button objects.
    ~ActivatedUnwindCleanup() {
        if (active && words[0]) {
            words[1] = 0;
            appFreeUni__FPv((void*)words[0]);
        }
    }
};


extern const float kFloat_6_2831855;
extern float s_twoPi_RetryMenuFirst;
extern float s_twoPi_RetryMenuSecond;
extern const float kFloat_0_5_RetryMenu;
extern const float kFloat_200_RetryMenu;
extern const float kFloat_1_RetryMenu;
extern char __vt__10CRetryMenu[];
extern const char retryMenuLayoutElements[];
extern const char retryMenuStrings[];
extern void* g_soundManager;

int Layout_Update(void*);
int Layout_PollLoadComplete(void*);
void MPBase1_Ctor(void*);
void Layout_Init(void*, int);
void Layout_InitStandard(void*, int, const char*, int, const char*);
void Layout_SetHitTestingEnabled(int);
void Layout_SetElementPlayerFlagsAndState(void*, int, int);
void Layout_SetElementEnabled(void*, int, int);
int Layout_CollectActivated(void*, void*, int);
void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);
void appFreeUni__FPv(void*);
void Layout_Destroy(void*, int);
void* __nw__FUl(unsigned int);
void __dl__FPv(void*);

struct AllocatedLayout {
    char data[0x5C];
    // Initialize a layout that becomes owned by the retry menu.
    AllocatedLayout(int initValue) { Layout_Init(this, initValue); }
};

// Initialize the first retry-menu angle constant to one full turn.
void RetryMenu_InitializeFirstAngleConstant(void) {
    s_twoPi_RetryMenuFirst = kFloat_6_2831855;
}

// Initialize the second retry-menu angle constant to one full turn.
void RetryMenu_InitializeSecondAngleConstant(void) {
    s_twoPi_RetryMenuSecond = kFloat_6_2831855;
}

} // C linkage

// Retain this ABI spelling: constructor unwind cleanup references
// __dt__16CWarningLogoBaseFv. Here it represents the common MPBase1 menu base.
struct CWarningLogoBase {
    char data[0x18];
    // Initialize the common menu-process base used by the retry menu.
    CWarningLogoBase() { MPBase1_Ctor(this); }
    ~CWarningLogoBase();
};
struct CRetryMenu : CWarningLogoBase {
    char rest[0x10];
    CRetryMenu(int);
};
// Create the owned 18-element layout and remember the caller's mode; loading is polled later.
CRetryMenu::CRetryMenu(int menuMode) {
    void* self = this;
    *(void**)self = __vt__10CRetryMenu;
    ((RetryMenuFields802618B8*)self)->layout = 0;
    ((RetryMenuFields802618B8*)self)->mode = menuMode;
    ((RetryMenuFields802618B8*)self)->selectedResult = kRetryNoChoice;
    AllocatedLayout* layout = new AllocatedLayout(kRetryLayoutInitValue);
    ((RetryMenuFields802618B8*)self)->layout = layout;
    Layout_InitStandard(layout, 0, retryMenuLayoutElements, kRetryElementCount, retryMenuStrings);
}

extern "C" {

// Release the layout and restore hit testing after closing the retry menu.
// shouldDelete > 0 also deletes self; the returned self address may then be dangling.
void* CRetryMenu_dtor(void* self, int shouldDelete) {
    if (self != 0) {
        *(void**)self = __vt__10CRetryMenu;
        Layout_SetHitTestingEnabled(1);
        void* layout = ((RetryMenuFields802618B8*)self)->layout;
        if (layout != 0) {
            Layout_Destroy(layout, 1);
            ((RetryMenuFields802618B8*)self)->layout = 0;
        }
        if (shouldDelete > 0)
            __dl__FPv(self);
    }
    return self;
}

// Advance the layout loading and animation work.
int CRetryMenu_UpdateLayout(void* self) {
    return Layout_Update(((RetryMenuFields802618B8*)self)->layout);
}

// Report whether the retry-menu layout has finished loading.
int CRetryMenu_PollLoadComplete(void* self) {
    return Layout_PollLoadComplete(((RetryMenuFields802618B8*)self)->layout);
}

// All methods use the shared retry-menu field view.

// Hide unavailable choices for this mode and reset the completion countdown.
void CRetryMenu_ResetSelectionState(void* self_) {
    RetryMenuFields802618B8* self = (RetryMenuFields802618B8*)self_;
    // Hide the unused mode-specific images before clearing all selection overlays.
    if (self->mode == kRetryDefaultMode) {
        Layout_SetElementPlayerFlagsAndState(self->layout, kRetryThirdButton, 0);
        Layout_SetElementPlayerFlagsAndState(self->layout, kRetryIdleSecondAlternate, 0);
        Layout_SetElementPlayerFlagsAndState(self->layout, kRetryIdleThird, 0);
    } else {
        Layout_SetElementPlayerFlagsAndState(self->layout, kRetryIdleSecondDefault, 0);
    }
    unsigned int elementIndex = kRetrySelectedBegin;
    do {
        Layout_SetElementEnabled(self->layout, elementIndex, 0);
        ++elementIndex;
    } while (elementIndex < kRetryElementCount);
    self->selectionDelayUpdates = 0;
}



// Apply the activated choice, play confirmation audio, and finish after nine update calls.
// Returns 1 only when a previously started countdown reaches zero, otherwise 0.
// Invalid activated IDs still trigger the sound/countdown, retaining the previous result.
int CRetryMenu_UpdateSelection(void* self_) {
    RetryMenuFields802618B8* self = (RetryMenuFields802618B8*)self_;
    // Finish the selection animation before polling another activation on this call.
    if (self->selectionDelayUpdates > 0) {
        Layout_SetHitTestingEnabled(0);
        --self->selectionDelayUpdates;
        if (self->selectionDelayUpdates == 0)
            return 1;
    }
    Layout_Update(self->layout);
    // Raw vector header [data, count, capacity], filled by the layout and freed here.
    void* activatedButtons[3] = {0, 0, 0};
    ActivatedUnwindCleanup activatedCleanup(&activatedButtons);
    if (Layout_CollectActivated(self->layout, activatedButtons, 0)) {
        // Only the first activated ID is consumed; swap its idle and selected artwork.
        int choiceIndex = **(int**)activatedButtons;
        switch (choiceIndex) {
        case kRetryPrimaryButton:
            self->selectedResult = kRetryPrimaryChoice;
            Layout_SetElementPlayerFlagsAndState(self->layout, kRetryPrimaryButton, 0);
            Layout_SetElementEnabled(self->layout, kRetrySelectedPrimaryButton, 1);
            Layout_SetElementEnabled(self->layout, kRetryIdlePrimary, 0);
            Layout_SetElementEnabled(self->layout, kRetrySelectedPrimary, 1);
            break;
        case kRetrySecondButton:
            if (self->mode == kRetryDefaultMode) {
                self->selectedResult = kRetrySecondDefaultChoice;
                Layout_SetElementPlayerFlagsAndState(self->layout, kRetrySecondButton, 0);
                Layout_SetElementEnabled(self->layout, kRetrySelectedSecondButton, 1);
                Layout_SetElementEnabled(self->layout, kRetryIdleSecondDefault, 0);
                Layout_SetElementEnabled(self->layout, kRetrySelectedSecondDefault, 1);
            } else {
                self->selectedResult = kRetrySecondAlternateChoice;
                Layout_SetElementPlayerFlagsAndState(self->layout, kRetrySecondButton, 0);
                Layout_SetElementEnabled(self->layout, kRetrySelectedSecondButton, 1);
                Layout_SetElementEnabled(self->layout, kRetryIdleSecondAlternate, 0);
                Layout_SetElementEnabled(self->layout, kRetrySelectedSecondAlternate, 1);
            }
            break;
        case kRetryThirdButton:
            self->selectedResult = kRetryThirdChoice;
            Layout_SetElementPlayerFlagsAndState(self->layout, kRetryThirdButton, 0);
            Layout_SetElementEnabled(self->layout, kRetrySelectedThirdButton, 1);
            Layout_SetElementEnabled(self->layout, kRetryIdleThird, 0);
            Layout_SetElementEnabled(self->layout, kRetrySelectedThird, 1);
            break;
        case kRetryFourthButton:
            self->selectedResult = kRetryFourthChoice;
            Layout_SetElementPlayerFlagsAndState(self->layout, kRetryFourthButton, 0);
            Layout_SetElementEnabled(self->layout, kRetrySelectedFourthButton, 1);
            Layout_SetElementEnabled(self->layout, kRetryIdleFourth, 0);
            Layout_SetElementEnabled(self->layout, kRetrySelectedFourth, 1);
            break;
        default:
            break;
        }
        // Start the fixed delay even for an unrecognized ID, matching existing behavior.
        self->selectionDelayUpdates = kRetrySelectionDelayUpdates;
        Sound_PlayByName(g_soundManager, retryMenuStrings + kRetryConfirmSoundOffset, 0, 0, 0, 0, kFloat_0_5_RetryMenu, kFloat_200_RetryMenu, kFloat_1_RetryMenu);
    }
    // Normal cleanup disarms the unwind guard before freeing the vector allocation.
    if (activatedButtons[0] != 0) {
        activatedButtons[1] = 0;
        activatedCleanup.release(); appFreeUni__FPv(activatedButtons[0]);
    }
    return 0;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462314", _ctors_u802618B8_0, RetryMenu_InitializeFirstAngleConstant);
STATIC_INIT_ENTRY(".CRT$XCU80462314", _ctors_u802618B8_1, RetryMenu_InitializeSecondAngleConstant);
#pragma pop
