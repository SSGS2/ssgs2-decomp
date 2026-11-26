// The hole selector (CHoleNoSelect) and the course selector (CCourseSelect) screens: hole
// buttons, selection feedback, and the mini-game hole tables they read.
//
// The party course table finds a course and hole position in the nine-hole party course list.

#include <decomp/static_init.h>
extern "C" char s_courseSelectBpControllerDtorRegistration[];
extern "C" char s_courseSelectBpHandleDtorRegistration[];
extern "C" char s_inputManagerDtorRegistration_CourseSelect[];
#include "classes/CBpCtrlHandle.h"
#include "classes/CSharedDoc.h"

#include "game/singletons.h"
#include "classes/CCourseSelect.h"
#include "classes/CHoleNoSelect.h"
#include "classes/CBpCtrl.h"
#include "classes/CFontNumDraw.h"
#include "classes/CPackTextureResource.h"

#define AsCHoleNoSelect(p) ((CHoleNoSelect*)(p))

#define AsCCourseSelect(p) ((CCourseSelect*)(p))
// Partial on-disk mini-game table views; no constructors or virtual tables are emitted.
struct MiniGameHoleEntry {
    unsigned char holeNumbers[3];
    char padding03[0xD];
    void* payload;
};
struct MiniGameHoleResource {
    void* table;
    void* vtable;
    void* sharedFile;
    int loaded;
};
struct DestructorRegistrationNode {
    void* next;
    void* destructor;
    void* object;
};
struct CourseMenuRegistrations {
    DestructorRegistrationNode reserved;
    DestructorRegistrationNode bpCtrl;
    DestructorRegistrationNode numberTextRegistry;
    DestructorRegistrationNode bpHandle;
    DestructorRegistrationNode messagePairs;
    DestructorRegistrationNode resourceTable;
};
struct ChatFontView {
    char padding00[0x10];
    void* fixedFont;
};
// Hole layout: IDs 0..17 select holes 1..18; IDs 18..35 are matching overlays.
// Value state 0 = normal, 2 = selected, 3 = disabled. Player-flag channel 5
// controls interaction; disabled back-nine elements remain untouched when peers reset.
// Stored selection is zero-based (-1 cancels); public hole numbers are one-based.
// Delay -1 accepts input/results; positive values count down, zero becomes -1 next update.
enum HoleSelectionConstants {
    kHolesPerCourse = 18,
    kHolesPerNine = 9,
    kHoleHighlightBase = 18,
    kHoleSelectionDelay = 3,
    kHoleScriptIndex = 13,
    kCancelAction = 6,
    kLayoutNormal = 0,
    kLayoutSelected = 2,
    kLayoutDisabled = 3,
    kFeedbackEnabled = 1,
    kPlayerFlagChannel = 5,
    kHoleControlCount = 36,
    kMiniGameSequenceLength = 3,
    kCourseTextureCount = 19
};
enum HoleSelectorResourceOffset {
    kHoleSelectorScript = 0xE,
    kHoleSelectorAnimation = 0x1C
};
enum CourseResourceOffset {
    kCourseArchive = 0x4B,
    kCourseLayout = 0x5C,
    kCourseScript = 0x6E,
    kLockedCourseIcon = 0x80,
    kCourseMessages = 0x8D
};

extern "C" {
void appFreeUni__FPv(void*);

// The raw vector's explicit normal cleanup releases this unwind guard first.
struct ActivatedUnwindCleanup {
    volatile unsigned int* words;
    bool active;
    // Guard the raw activation vector against an exceptional exit.
    explicit ActivatedUnwindCleanup(volatile void* vector) : words((volatile unsigned int*)vector), active(true) {}
    // Disable guard cleanup before the caller releases the array explicitly.
    void release() {
        active = false;
    }
    // Free active collection storage on unwinding.
    ~ActivatedUnwindCleanup() {
        if (active && words[0]) {
            words[1] = 0;
            appFreeUni__FPv((void*)words[0]);
        }
    }
};

extern float kFloat_6_2831855;
extern float s_miniGameTableFullTurnAngle;
extern float s_holeSelectorFullTurnAngle;
extern float s_courseSelectorFullTurnAngle;
extern float kFloat_1_HoleSelectorOpacity;
extern char s_bpCtrl[];
extern char s_bpCtrlDtorRegistration_CourseSelect[];
extern signed char s_bpCtrlInitialized;
extern void* g_sharedDoc;
extern signed char s_bpCtrlHandleInitialized;
extern int s_bpCtrlHandle[2];
extern signed char s_inputManagerInitialized;
extern char s_gameInputManager[];
extern char s_inputManagerDtorRegistration_HoleSelection[];
extern char s_bpHandleDtorRegistration_CourseMenu[];
extern signed char kPartyCourseTableHoleNumbers[];
extern signed char kPartyCourseTableCourseIds[];
extern char __vt__13CHoleNoSelect[];
extern char kCourseSelectionTextureNames[];
extern char __vt__13CMiniGameHole[];
extern char __vt__13CCourseSelect[];
extern char kDriveHoleTableFilename[];
extern char kDartsHoleTableFilename[];
extern char holeSelectorStrings[];
extern char courseSelectStrings[];
extern char s_fontNumDrawRegistry[];
extern int kHoleSelectorControlGroupIds[];
extern signed char s_resourceTableInitialized;
extern void* s_resourceTableStorage;
extern void* g_chatMsg;
extern signed char s_pointerVectorInitialized;
extern char s_inputManagerDtorRegistration_CourseSelect[];

void Layout_Update(void*);
void* Table_GetEntry(void*, int);
void BpObject_TryInvokeCurrentCallback(void*, int);
void BpObject_Draw(void*);
void Layout_DrawWithOpacity(void*, float);
void Layout_SetElementValueState(void*, int, char);
void Layout_SetElementFeedbackState(void*, int, char);
void InputManager_Init(void*);
void InputManager_Dtor(void*);
void appFreeUni__FPv(void*);
int Input_IsActionPressed(void*, int, int);
int Layout_CollectActivated(void*, void*, int);
void Layout_SetElementPlayerFlag(void*, int, int, int);
int Layout_PollLoadComplete(void*);
void __register_global_object(void*, void*, void*);
void CBpCtrl_dtor(void*);
void BpObject_StartScriptByName(void*, const char*, int);
void BpObject_SetEnabled(void*, int);
void Layout_ReadElementRect(void*, int, void*);
void Layout_WriteElementRect(void*, int, void*);
void* UIGroup_GetOrCreateById(void*, int);
void UIGroup_SetEnabled(void*, int);
void Layout_Init(void*, int);
void Layout_LoadExtended(void*, int, const char*);
void* IdMap_Lookup(void*, int);
void Layout_Destroy(void*, int);
void __dl__FPv(void*);
void __dla__FPv(void*);
void dtor_8031F364(void*, int);
void* __nw__FUl(unsigned int);
void ByteVector_Clear(void*);
void ByteVector_AppendOne(void*, void*);
int Struct12_Clear(void*);
void* ByteVector_GetDataPointer(void*);
void* ByteVector_GetOffsetPointer(void*, int);
void ByteVector_InsertAtIterator(void*, void*, void*);
int ByteVector_GetSize(void*);
unsigned int rand();
void* ByteVector_GetElementPtr(void*, int);
void Allocation_Dtor(void*);
extern "C" void* __ct__6CBpTexFv(void*);
extern "C" void __dt__6CBpTexFv(void*, int);
void PointerVector_DestroyStorage(void*);
void CBpCtrlHandle_Dtor();
void* SharedDoc_GetCurrent();
void SharedDoc_SetPartyCourseTableIndex(void*, int);
void MPBase1_Ctor(void*);
void __dt__7MPBase1Fv(void*, int);
void* CourseSelection_ReleaseAndFree(void*, int);
void* __nwa__FUl(unsigned int);
void* __construct_new_array(void*, void*, void*, int, int);
void* ResourceTable_FindEntryByName(void*, const char*, void*);
void PointerVector_InitReserved(void*);
void NString_AssignCString(void*, const char*);
void* SharedFile_Create(const char*, int);
void dtor_80248D58(void*, int);

// Initialize this unit's first cached full-turn angle.
void InitMiniGameTableFullTurnAngle() {
    s_miniGameTableFullTurnAngle = kFloat_6_2831855;
}
// Initialize this unit's second cached full-turn angle.
void InitHoleSelectorFullTurnAngle() {
    s_holeSelectorFullTurnAngle = kFloat_6_2831855;
}
// Initialize this unit's third cached full-turn angle.
void InitCourseSelectorFullTurnAngle() {
    s_courseSelectorFullTurnAngle = kFloat_6_2831855;
}

// Advance the hole selector's layout animation.
int CHoleNoSelect::UpdateLayout() {
    Layout_Update(layout);
    return 1;
}

// Enable the hole selector and save or restore the previously active script index.
int CHoleNoSelect::SetEnabled(int enabled) {
    BpObject_SetEnabled((*(void**)&scriptObject), enabled);
    selectionComplete = 0;
    if (enabled) {
        if (!Sing_805DC5F8()) {
            if (!s_bpCtrlHandleInitialized) {
                s_bpCtrlHandle[0] = -1;
                s_bpCtrlHandle[1] = -1;
                __register_global_object(s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor,
                                         s_bpHandleDtorRegistration_CourseMenu);
                s_bpCtrlHandleInitialized = 1;
            }
            Sing_805DC5F8() = s_bpCtrlHandle;
        }
        void* handle = Sing_805DC5F8();
        previousScriptIndex = ((CBpCtrlHandle*)handle)->scriptIndex;
        if (handle == 0) {
            if (!s_bpCtrlHandleInitialized) {
                s_bpCtrlHandle[0] = -1;
                s_bpCtrlHandle[1] = -1;
                __register_global_object(s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor,
                                         s_bpHandleDtorRegistration_CourseMenu);
                s_bpCtrlHandleInitialized = 1;
            }
            Sing_805DC5F8() = s_bpCtrlHandle;
        }
        ((CBpCtrlHandle*)Sing_805DC5F8())->SetScriptIndex(kHoleScriptIndex);
    } else {
        if (!Sing_805DC5F8()) {
            if (!s_bpCtrlHandleInitialized) {
                s_bpCtrlHandle[0] = -1;
                s_bpCtrlHandle[1] = -1;
                __register_global_object(s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor,
                                         s_bpHandleDtorRegistration_CourseMenu);
                s_bpCtrlHandleInitialized = 1;
            }
            Sing_805DC5F8() = s_bpCtrlHandle;
        }
        ((CBpCtrlHandle*)Sing_805DC5F8())->SetScriptIndex(previousScriptIndex);
    }
    return 1;
}

#pragma use_lmw_stmw on
// Both outputs are mutable byte vectors whose contents are replaced. tableIndex
// addresses a nine-entry row. For multiple holes, holeCount must be <= 9; for one
// hole, specifiedHole selects a one-based table position (not an actual hole number).
// Shuffle builds an insertion permutation; the global party-table index is updated.
// orderState enters as a shuffle option (nonzero), then becomes a one-based hole ordinal.
// Build parallel hole/course lists from a nine-hole table, optionally randomizing their order.
void PartyCourseTable_BuildHoleAndCourseLists(void* holeNumbers, void* courseIds, int tableIndex, int holeCount,
                                              int specifiedHole, int orderState) {
    unsigned char holeOrder[0xC];
    Struct12_Clear(holeOrder);
    int seed;
    int insertionIndex = 0;
    // Build the slot order before clearing outputs, so paired lists stay aligned.
    if (orderState) {
        orderState = 1;
        do {
            unsigned char holeIndex = (unsigned char)(orderState - 1);
            void* elem;
            void* part = ByteVector_GetDataPointer(holeOrder);
            elem = ByteVector_GetOffsetPointer(&part, insertionIndex);
            ByteVector_InsertAtIterator(holeOrder, &elem, &holeIndex);
            unsigned int radix = ByteVector_GetSize(holeOrder);
            unsigned int random = rand();
            orderState++;
            insertionIndex = random % radix;
        } while (orderState <= kHolesPerNine);
    } else {
        orderState = 1;
        do {
            unsigned char holeIndex = (unsigned char)(orderState - 1);
            ByteVector_AppendOne(holeOrder, &holeIndex);
            orderState++;
        } while (orderState <= kHolesPerNine);
    }
    // Flatten table slots into parallel hole/course values, then publish the table choice.
    ByteVector_Clear(holeNumbers);
    ByteVector_Clear(courseIds);
    if (holeCount != 1) {
        signed char* tableHoleNumbers = kPartyCourseTableHoleNumbers + (tableIndex * kHolesPerNine);
        signed char* tableCourseIds = kPartyCourseTableCourseIds + (tableIndex * kHolesPerNine);
        int i = 0;
        while (i < holeCount) {
            signed char ch = *(signed char*)ByteVector_GetElementPtr(holeOrder, i);
            ByteVector_AppendOne(holeNumbers, tableHoleNumbers + ch);
            ch = *(signed char*)ByteVector_GetElementPtr(holeOrder, i);
            ByteVector_AppendOne(courseIds, tableCourseIds + ch);
            ++i;
        }
    } else {
        ByteVector_AppendOne(holeNumbers, ((signed char (*)[9])kPartyCourseTableHoleNumbers)[tableIndex] +
                                              ((signed char)specifiedHole - 1));
        ByteVector_AppendOne(courseIds, ((signed char (*)[9])kPartyCourseTableCourseIds)[tableIndex] +
                                            ((signed char)specifiedHole - 1));
    }
    void* result = SharedDoc_GetCurrent();
    SharedDoc_SetPartyCourseTableIndex(result, tableIndex);
    dtor_80248D58(holeOrder, -1);
}
#pragma use_lmw_stmw off

// Draw the hole selector's script and layout.
int CHoleNoSelect::Draw() {
    BpObject_TryInvokeCurrentCallback((*(void**)&scriptObject), 0);
    BpObject_Draw((*(void**)&scriptObject));
    Layout_DrawWithOpacity(layout, kFloat_1_HoleSelectorOpacity);
    return 1;
}

// Menu modes (separate from CSharedDoc::partyGameMode): 1 is course 0 with nine
// holes, 2 is course 0 with one hole, 3 is party-table course 14 with nine holes.
// Other values only reset the mode and weather.
void CCourseSelect::SetMode(int mode) {
    enum {
        nineHoleMode = 1,
        singleHoleMode = 2,
        partyTableMode = 3,
        firstPartyCourse = 14
    };
    selectionMode = mode;
    weatherMode = 0;
    holeOrder = 0;
    if (mode != singleHoleMode) {
        if (mode < 2) {
            if (mode < nineHoleMode)
                return;
        } else {
            if (mode < partyTableMode + 1) {
                selectedCourseIndex = firstPartyCourse;
                holeCount = kHolesPerNine;
            }
            return;
        }
        selectedCourseIndex = 0;
        holeCount = kHolesPerNine;
        return;
    }
    selectedCourseIndex = 0;
    holeCount = 1;
}

struct CollectArr {
    void* ptr;
    volatile int count;
    int capacity;
};

// Compare indices by reference to preserve the original short-circuit button checks.
inline bool CourseIndexEquals(const int& expectedIndex, const int& index) {
    return expectedIndex == index;
}

#pragma opt_strength_reduction off
// Return 0 only when no activation was collected; return 1 after cancellation or
// processing the activation list (even if every ID was outside the hole-button range).
// Mouse selection sets delay to 3 and then overwrites it with 1; preserve that timing.
// Handle cancellation or confirm an enabled hole button and reset its peer highlights.
int CHoleNoSelect::ProcessInput() {
    CollectArr activatedElements;
    CollectArr cancelScratch;
    cancelScratch.ptr = 0;
    cancelScratch.count = 0;
    cancelScratch.capacity = 0;
    int cancelled;

    if (GetInputManagerSlot() == 0) {
        if (s_inputManagerInitialized == 0) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                     s_inputManagerDtorRegistration_HoleSelection);
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = s_gameInputManager;
    }
    if (Input_IsActionPressed(GetInputManagerSlot(), kCancelAction, -1) != 0) {
        selectedHoleIndex = -1;
        selectionComplete = 1;
        if (cancelScratch.ptr != 0) {
            int t;
            cancelScratch.count = (t = cancelScratch.count) - t;
            appFreeUni__FPv(cancelScratch.ptr);
        }
        cancelled = 1;
    } else {
        if (cancelScratch.ptr != 0) {
            int t;
            cancelScratch.count = (t = cancelScratch.count) - t;
            appFreeUni__FPv(cancelScratch.ptr);
        }
        cancelled = 0;
    }
    if (cancelled != 0)
        return 1;

    // Cancellation is finished; collect owned activation storage for hole-button input.
    activatedElements.ptr = 0;
    activatedElements.count = 0;
    activatedElements.capacity = 0;
    ActivatedUnwindCleanup activatedCleanup(&activatedElements);
    if (Layout_CollectActivated(layout, &activatedElements, 0) == 0) {
        if (activatedElements.ptr != 0) {
            int t;
            activatedElements.count = (t = activatedElements.count) - t;
            activatedCleanup.release();
            appFreeUni__FPv(activatedElements.ptr);
        }
        return 0;
    }

    unsigned int activatedCount = activatedElements.count;
    int* activatedIds = (int*)activatedElements.ptr;
    for (unsigned int i = 0; i < activatedCount; ++i) {
        int holeElementId = activatedIds[i];
        if (holeElementId >= 0 && holeElementId <= kHolesPerCourse - 1) {
            Layout_SetElementValueState(layout, holeElementId, kLayoutSelected);
            Layout_SetElementFeedbackState(layout, holeElementId, kLayoutSelected);
            Layout_SetElementValueState(layout, holeElementId + kHoleHighlightBase, 2);
            Layout_SetElementPlayerFlag(layout, holeElementId + kHoleHighlightBase, 0, kPlayerFlagChannel);
            selectedHoleIndex = holeElementId;
            selectionDelayFrames = kHoleSelectionDelay;
            for (int j = 0; j <= kHolesPerCourse - 1; ++j) {
                if (j == holeElementId)
                    continue;
                if (backNineDisabled != 0) {
                    if (CourseIndexEquals(9, j) || CourseIndexEquals(10, j) || CourseIndexEquals(11, j) ||
                        CourseIndexEquals(12, j) || CourseIndexEquals(13, j) || CourseIndexEquals(14, j) ||
                        CourseIndexEquals(15, j) || CourseIndexEquals(16, j) || CourseIndexEquals(17, j))
                        continue;
                }
                Layout_SetElementValueState(layout, j, kLayoutNormal);
                Layout_SetElementFeedbackState(layout, j, kFeedbackEnabled);
                Layout_SetElementValueState(layout, j + kHoleHighlightBase, 0);
                Layout_SetElementPlayerFlag(layout, j + kHoleHighlightBase, 1, kPlayerFlagChannel);
            }
            selectionComplete = 1;
            selectionDelayFrames = 1;
            break;
        }
    }

    if (activatedElements.ptr != 0) {
        int t;
        activatedElements.count = (t = activatedElements.count) - t;
        activatedCleanup.release();
        appFreeUni__FPv(activatedElements.ptr);
    }
    return 1;
}
#pragma opt_strength_reduction reset

// Advance selection feedback delays before accepting another hole input.
int CHoleNoSelect::Update() {
    Layout_Update(layout);
    int delayFrames = selectionDelayFrames;
    int result;
    if (delayFrames == -1) {
        result = 0;
    } else if (delayFrames > 0) {
        selectionDelayFrames = delayFrames - 1;
        result = 1;
    } else {
        selectionDelayFrames = -1;
        result = 1;
    }
    if (result == 0)
        ProcessInput();
    return 1;
}

// Return the pointer stored at entry +0x10, or null when the entry is absent.
// Also used for BGM composer text; the MiniGameHoleEntry cast only supplies the offset.
void* OffsetTable_GetEntryWord10(void** table, int index) {
    void* entry = Table_GetEntry(*(void**)table, index);
    if (entry != 0) {
        return ((MiniGameHoleEntry*)entry)->payload;
    }
    return 0;
}

// out must be writable on confirmation. Return 1 also signals cancellation, without
// writing *out; callers distinguish it by preserving their own initial output value.
// Return a confirmed one-based hole number after feedback completes; cancellation leaves output unchanged.
int CHoleNoSelect::GetSelectionResult(int* out) {
    int delayFrames = selectionDelayFrames;
    if (delayFrames != -1 || selectionComplete == 0) {
        return 0;
    }
    int selectedHoleIndex = this->selectedHoleIndex;
    if (selectedHoleIndex != -1)
        *out = selectedHoleIndex + 1;
    return 1;
}

#pragma dont_inline on
// Read one of three hole numbers from a loaded mini-game course table.
int MiniGameHoleTable_GetHoleNumber(void* resource, int courseIndex, int holeSlot) {
    if (((MiniGameHoleResource*)resource)->loaded == 0 || ((MiniGameHoleResource*)resource)->sharedFile == 0)
        return 0;
    void* entry = Table_GetEntry(*(void**)resource, courseIndex);
    if (entry == 0)
        return 0;
    unsigned char holeNumbers[3];
    holeNumbers[0] = ((MiniGameHoleEntry*)entry)->holeNumbers[0];
    holeNumbers[1] = ((MiniGameHoleEntry*)entry)->holeNumbers[1];
    holeNumbers[2] = ((MiniGameHoleEntry*)entry)->holeNumbers[2];
    if ((unsigned int)holeSlot >= 3)
        return 0;
    return holeNumbers[holeSlot];
}
#pragma dont_inline off

// Select a one-based hole number and start its button-feedback delay.
int CHoleNoSelect::SelectHole(int holeNumber) {
    if (holeNumber < 1 || holeNumber > kHolesPerCourse)
        return 0;
    int holeIndex = holeNumber - 1;
    selectedHoleIndex = holeIndex;
    Layout_SetElementValueState(layout, holeIndex, 2);
    Layout_SetElementFeedbackState(layout, holeIndex, 2);
    Layout_SetElementValueState(layout, holeIndex + kHoleHighlightBase, 2);
    Layout_SetElementPlayerFlag(layout, holeIndex + kHoleHighlightBase, 0, kPlayerFlagChannel);
    selectedHoleIndex = holeIndex;
    selectionDelayFrames = kHoleSelectionDelay;
    return 1;
}

// Check the hole selector's layout and the shared script controller for load completion.
int CHoleNoSelect::IsLoaded() {
    void* layout = AsCCourseSelect(this)->unk_004;
    if (Layout_PollLoadComplete(layout) == 0)
        return 0;
    if (Sing_805DC5F0() == 0) {
        if (s_bpCtrlInitialized == 0) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_CourseSelect);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    if (((CBpCtrl*)Sing_805DC5F0())->PollAllEntriesLoaded() == 0)
        return 0;
    return 1;
}

// Find a course/hole pair's one-based position in the current nine-hole party table.
int PartyCourseTable_FindCourseHolePosition(int courseIndex, int holeNumber) {
    void* sharedDocument = g_sharedDoc;
    int partyMode = (*(int*)&((CSharedDoc*)sharedDocument)->partyGameMode);
    if (partyMode != 3)
        return -1;
    int tableIndex = (*(int*)&((CSharedDoc*)sharedDocument)->partyCourseTableIndex);
    signed char* tableHoleNumbers = (signed char*)kPartyCourseTableHoleNumbers + tableIndex * 8 + tableIndex;
    signed char* tableCourseIds = (signed char*)kPartyCourseTableCourseIds + tableIndex * 8 + tableIndex;
    int i = 0;
    int triplesRemaining = 3;
    (void)tableIndex;
    do {
        signed char r0 = tableHoleNumbers[0];
        if (holeNumber == (int)r0) {
            signed char r1v = tableCourseIds[0];
            if (courseIndex == (int)r1v)
                return i + 1;
        }
        r0 = tableHoleNumbers[1];
        if (holeNumber == (int)r0) {
            signed char r1v = tableCourseIds[1];
            if (courseIndex == (int)r1v)
                return i + 2;
        }
        r0 = tableHoleNumbers[2];
        if (holeNumber == (int)r0) {
            signed char r1v = tableCourseIds[2];
            if (courseIndex == (int)r1v)
                return i + 3;
        }
        tableHoleNumbers += 3;
        tableCourseIds += 3;
        i += 3;
    } while (--triplesRemaining);
    return -1;
}

// Copy hole-button rectangles and disable the back nine when the mode requires it.
int CHoleNoSelect::InitLayout(signed char disableBackNine) {
    (*(signed char*)&backNineDisabled) = disableBackNine;
    BpObject_StartScriptByName((*(void**)&scriptObject), holeSelectorStrings + kHoleSelectorAnimation, 1);
    BpObject_SetEnabled((*(void**)&scriptObject), 0);
    int elementRect[4];
    for (int i = 0; i <= kHolesPerCourse - 1; ++i) {
        Layout_ReadElementRect(layout, i, elementRect);
        Layout_WriteElementRect(layout, i + kHoleHighlightBase, elementRect);
    }
    int i = 0;
    int* ptr = kHoleSelectorControlGroupIds;
    while (i < (unsigned int)kHoleControlCount) {
        void* uiGroup = UIGroup_GetOrCreateById((*(void**)&scriptObject), *ptr);
        UIGroup_SetEnabled(uiGroup, 0);
        ++i;
        ++ptr;
    }
    if (backNineDisabled) {
        for (int i = 9; i <= kHolesPerCourse - 1; ++i) {
            Layout_SetElementValueState(layout, i, kLayoutDisabled);
            Layout_SetElementPlayerFlag(layout, i, 0, kPlayerFlagChannel);
            Layout_SetElementValueState(layout, i + kHoleHighlightBase, 3);
            Layout_SetElementPlayerFlag(layout, i + kHoleHighlightBase, 0, kPlayerFlagChannel);
        }
    }
    return 1;
}

// Create and load the hole selector's layout and script-controller entry.
CHoleNoSelect::CHoleNoSelect() {
    vtbl = __vt__13CHoleNoSelect;
    void* obj = __nw__FUl(0x5c);
    if (obj)
        Layout_Init(obj, 5);
    layout = obj;
    scriptObject = 0;
    scriptEntryId = -1;
    selectionDelayFrames = -1;
    selectedHoleIndex = -1;
    backNineDisabled = 0;
    selectionComplete = 0;
    previousScriptIndex = -1;
    Layout_LoadExtended(obj, 0, holeSelectorStrings);
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_CourseSelect);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    ((CBpCtrl*)Sing_805DC5F0())
        ->CreateEntry(holeSelectorStrings + kHoleSelectorScript, (void*)&((volatile CHoleNoSelect*)this)->scriptEntryId,
                      0);
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_CourseSelect);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    (*(void**)&scriptObject) = IdMap_Lookup(Sing_805DC5F0(), ((volatile CHoleNoSelect*)this)->scriptEntryId);
}

// Release the hole selector's layout and script-controller entry.
CHoleNoSelect::~CHoleNoSelect() {
    vtbl = (void*)__vt__13CHoleNoSelect;
    if (layout != 0) {
        Layout_Destroy(layout, 1);
        layout = 0;
    }
    if (scriptEntryId != -1) {
        if (Sing_805DC5F0() == 0) {
            if (s_bpCtrlInitialized == 0) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_CourseSelect);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = (void*)s_bpCtrl;
        }
        ((CBpCtrl*)Sing_805DC5F0())->DeleteEntryById(scriptEntryId);
        scriptEntryId = -1;
    }
}

// Outputs are replaced byte vectors. specifiedHole == 0 selects the first table
// hole for a one-hole round; otherwise it is an actual hole number. Multi-hole
// rounds emit floor(holeCount / 3) complete triples, discarding any remainder.
// Build parallel hole/course lists by repeating a mini-game course's three-hole sequence.
void MiniGameHoleTable_BuildHoleAndCourseLists(void* resource, void* holeNumbers, void* courseIds, int courseIndex,
                                               int holeCount, signed char specifiedHole) {
    ByteVector_Clear(holeNumbers);
    ByteVector_Clear(courseIds);
    if (holeCount == 1) {
        if (specifiedHole == 0) {
            unsigned char holeNumber = (unsigned char)MiniGameHoleTable_GetHoleNumber(resource, courseIndex, 0);
            ByteVector_AppendOne(holeNumbers, &holeNumber);
        } else {
            ByteVector_AppendOne(holeNumbers, &specifiedHole);
        }
        unsigned char courseId = (unsigned char)courseIndex;
        ByteVector_AppendOne(courseIds, &courseId);
    } else {
        int repeatCount = holeCount / kMiniGameSequenceLength;
        for (int i = 0; i < repeatCount; i++) {
            for (int holeSlot = 0; holeSlot < kMiniGameSequenceLength; holeSlot++) {
                unsigned char holeNumber =
                    (unsigned char)MiniGameHoleTable_GetHoleNumber(resource, courseIndex, holeSlot);
                ByteVector_AppendOne(holeNumbers, &holeNumber);
                unsigned char courseId = (unsigned char)courseIndex;
                ByteVector_AppendOne(courseIds, &courseId);
            }
        }
    }
}

// Owns layout, textures, selector and file wrappers; fixedFont is borrowed from
// chat messages. Shared BP/text registries own their registered entries.
// partyGameMode 1 = darts, 2 = drive, 3 = party table.
// Create course-card textures, selectors, description text, and mode-specific hole-table resources.
CCourseSelect::CCourseSelect() {
    void* registrations = s_inputManagerDtorRegistration_CourseSelect;
    MPBase1_Ctor(this);
    vtbl = (void*)__vt__13CCourseSelect;

    holeNumbers = 0;
    holeNumberCount = 0;
    holeNumberCapacity = 0;
    holeCourseIds = 0;
    holeCourseCount = 0;
    holeCourseCapacity = 0;
    selectedCourseIndex = 0;
    holeOrder = 0;
    weatherMode = 0;
    holeCount = kHolesPerNine;
    selectionMode = 0;

    struct ConstructionCleanup {
        CCourseSelect* object;
        // Destroy initialized course/base subobjects if construction has not completed.
        ~ConstructionCleanup() {
            if (object) {
                CourseSelection_ReleaseAndFree((char*)&object->selectedCourseIndex, -1);
                __dt__7MPBase1Fv(object, 0);
            }
        }
    } construction = {this};

    // Phase 1: construct UI storage and the hole selector; no resources are bound yet.
    void* obj = __nw__FUl(0x5c);
    void* r29 = obj;
    if (obj) {
        try {
            Layout_Init(obj, 5);
        } catch (...) {
            __dl__FPv(obj);
            throw;
        }
        r29 = obj;
    }
    layout = r29;

    scriptObject = 0;
    bpEntryId = -1;

    void* courseTexturesStorage = __nwa__FUl(0x308);
    r29 = courseTexturesStorage;
    if (courseTexturesStorage) {
        try {
            r29 = __construct_new_array(courseTexturesStorage, (void*)__ct__6CBpTexFv, (void*)__dt__6CBpTexFv, 0x28,
                                        0x13);
        } catch (...) {
            __dla__FPv(courseTexturesStorage);
            throw;
        }
    }
    courseTextures = r29;

    obj = __nw__FUl(0x28);
    r29 = obj;
    if (obj) {
        try {
            r29 = __ct__6CBpTexFv(obj);
        } catch (...) {
            __dl__FPv(obj);
            throw;
        }
    }
    auxiliaryTexture = r29;

    coursePage = (void*)1;
    selectionDelayFrames = -1;
    selectionComplete = 0;
    fixedFont = 0;
    text = 0;
    messageRenderer = 0;
    textTable = 0;

    obj = __nw__FUl(0x20);
    r29 = obj;
    if (obj) {
        try {
            ((CHoleNoSelect*)obj)->__ct();
        } catch (...) {
            __dl__FPv(obj);
            throw;
        }
    }

    holeNoSelect = r29;
    holeSelectorActive = 0;
    miniGameHole = 0;
    texturePack = 0;
    weatherOptionEnabled = 0; // Weather controls are initially unavailable.
    exitRequested = 0;        // No menu exit requested.
    int stackLocal = 0;

    // Lazily initialize the resource-name lookup table.
    if (!Sing_805DC620()) {
        if (!s_resourceTableInitialized) {
            s_resourceTableStorage = 0;
            __register_global_object(&s_resourceTableStorage, (void*)Allocation_Dtor,
                                     ((void*)&((CourseMenuRegistrations*)registrations)->resourceTable));
            s_resourceTableInitialized = 1;
        }
        Sing_805DC620() = &s_resourceTableStorage;
    }

    void* r29ptr = courseSelectStrings;
    void* resourceTable = Sing_805DC620();
    void* r5 = &stackLocal;
    void* r4 = (void*)((char*)r29ptr + kCourseArchive);
    void* r27 = ResourceTable_FindEntryByName(resourceTable, (const char*)r4, r5);

    obj = __nw__FUl(0x20);
    void* r28 = obj;
    if (obj) {
        void* r5v = (void*)stackLocal;
        try {
            r28 = ((CPackTextureResource*)obj)->Init((const char*)r4, (void**)r5v, (unsigned int)((int)r27));
        } catch (...) {
            __dl__FPv(obj);
            throw;
        }
    }
    texturePack = r28;

    r4 = courseSelectStrings;
    void* r3 = layout;
    r5 = (void*)((char*)r4 + kCourseLayout);
    r4 = 0;
    Layout_LoadExtended(r3, 0, (const char*)r5);

    // Lazily initialize the shared B2D script controller.
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor,
                                     ((void*)&((CourseMenuRegistrations*)registrations)->bpCtrl));
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }

    void* bp = Sing_805DC5F0();
    const char* courseScriptName = (const char*)courseSelectStrings;
    void* r5b = (void*)&bpEntryId;
    courseScriptName = (const char*)((char*)courseScriptName + kCourseScript);
    ((CBpCtrl*)bp)->CreateEntry(courseScriptName, r5b, 0);

    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor,
                                     ((void*)&((CourseMenuRegistrations*)registrations)->bpCtrl));
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    bp = Sing_805DC5F0();
    int idx = bpEntryId;
    void* r3c = IdMap_Lookup(bp, idx);
    scriptObject = r3c;

    // Lazily initialize the numeric-text registry.
    if (!Sing_805DC830()) {
        if (!s_pointerVectorInitialized) {
            PointerVector_InitReserved(s_fontNumDrawRegistry);
            __register_global_object(s_fontNumDrawRegistry, (void*)PointerVector_DestroyStorage,
                                     ((void*)&((CourseMenuRegistrations*)registrations)->numberTextRegistry));
            s_pointerVectorInitialized = 1;
        }
        Sing_805DC830() = s_fontNumDrawRegistry;
    }
    bp = Sing_805DC830();
    void* r3d = CFontNumDraw::CreateInVector((PtrVector*)bp);
    textTable = r3d;

    // Phase 2: bind all 19 course textures and the locked-course icon to their wrappers.
    if (courseTextures && auxiliaryTexture) {
        void* textureNameCursor = kCourseSelectionTextureNames;
        int courseTextureIndex = 0;
        int textureByteOffset = 0;
        while (courseTextureIndex < kCourseTextureCount) {
            void* arr = courseTextures;
            const char* textureName = *(const char**)((char*)textureNameCursor);
            void* elem = (void*)((char*)arr + textureByteOffset);
            void** vt = *(void***)elem;
            void (*fn12)(void*, const char*) = (void (*)(void*, const char*))vt[3]; // offset 0xc /4 =3
            fn12(elem, textureName);
            arr = courseTextures;
            void** vt2 = *(void***)((char*)arr + textureByteOffset);
            void (*fn18)(void*) = (void (*)(void*))vt2[6]; // offset 0x18/4=6
            fn18((void*)((char*)arr + textureByteOffset));
            courseTextureIndex++;
            textureByteOffset += 0x28;
            textureNameCursor = (void*)((char*)textureNameCursor + 4);
        }
        void* obj2 = auxiliaryTexture;
        const char* lockedCourseIconName = (const char*)courseSelectStrings;
        lockedCourseIconName = lockedCourseIconName + kLockedCourseIcon;
        void** vt3 = *(void***)obj2;
        void (*fn12b)(void*, const char*) = (void (*)(void*, const char*))vt3[3];
        fn12b(obj2, lockedCourseIconName);
        void** vt4 = *(void***)obj2;
        void (*fn18b)(void*) = (void (*)(void*))vt4[6];
        fn18b(obj2);
    }

    // Phase 3: load course-description messages and borrow their font provider.
    int messageFileNameString[3] = {0, 0, 0};
    void* r3l = messageFileNameString;
    const char* messageFileName = (const char*)courseSelectStrings;
    messageFileName = messageFileName + kCourseMessages;
    // NString_AssignCString(void*, const char*) modifies local0..
    NString_AssignCString(r3l, messageFileName);
    struct StringCleanup {
        int* string;
        // Release the temporary message-filename string if heap backed.
        ~StringCleanup() {
            if ((unsigned int)string[0] >> 31)
                appFreeUni__FPv((void*)string[2]);
        }
    } stringCleanup = {messageFileNameString};
    unsigned int r0l = (unsigned int)messageFileNameString[0];
    if (r0l >> 31) {
        r3l = (void*)messageFileNameString[2];
    } else {
        r3l = (void*)((char*)messageFileNameString + 1);
    }
    r4 = 0;
    void* r3m = SharedFile_Create((const char*)r3l, 0);
    sharedFile = r3m;

    void* g = g_chatMsg;
    // The chat-message singleton supplies the fixed font shared by course descriptions.
    r0l = (unsigned int)((ChatFontView*)g)->fixedFont;
    fixedFont = (void*)r0l;

    // Phase 4: darts/drive need their own three-hole table; other modes leave it null.
    void* sharedDocument = g_sharedDoc;
    int partyMode = ((CSharedDoc*)sharedDocument)->partyGameMode;
    r0l = partyMode - 1;
    if (r0l <= 1) { // cmplwi r0,1; bgt skip -> if <=1 (unsigned) enter
        void* mem2 = __nw__FUl(0x10);
        void* miniGameHoleResource = mem2;
        if (mem2) {
            ((MiniGameHoleResource*)mem2)->table = 0;
            ((MiniGameHoleResource*)mem2)->vtable = (void*)__vt__13CMiniGameHole;
            ((MiniGameHoleResource*)mem2)->sharedFile = 0;
            ((MiniGameHoleResource*)mem2)->loaded = 0;
            try {
                void* sharedDocument1 = g_sharedDoc;
                int v2 = ((CSharedDoc*)sharedDocument1)->partyGameMode;
                if (v2 == 1) {
                    const char* dartsHoleTableName = (const char*)kDartsHoleTableFilename;
                    void* r3n = SharedFile_Create(dartsHoleTableName, 0);
                    ((MiniGameHoleResource*)miniGameHoleResource)->sharedFile = r3n;
                }
                void* sharedDocument2 = g_sharedDoc;
                int v3 = ((CSharedDoc*)sharedDocument2)->partyGameMode;
                if (v3 == 2) {
                    const char* driveHoleTableName = (const char*)kDriveHoleTableFilename;
                    void* r3o = SharedFile_Create(driveHoleTableName, 0);
                    ((MiniGameHoleResource*)miniGameHoleResource)->sharedFile = r3o;
                }
            } catch (...) {
                dtor_8031F364(mem2, 0);
                __dl__FPv(mem2);
                throw;
            }
        }
        miniGameHole = miniGameHoleResource;
    }

    construction.object = 0;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804625D8", _ctors_u80319FDC_0, InitMiniGameTableFullTurnAngle);
STATIC_INIT_ENTRY(".CRT$XCU804625D8", _ctors_u80319FDC_1, InitHoleSelectorFullTurnAngle);
STATIC_INIT_ENTRY(".CRT$XCU804625D8", _ctors_u80319FDC_2, InitCourseSelectorFullTurnAngle);
#pragma pop
