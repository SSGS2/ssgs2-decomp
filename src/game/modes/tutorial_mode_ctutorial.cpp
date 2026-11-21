#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "classes/CGolfDoc.h"
#include "classes/GameInputManager.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
// Translation unit 0x802076A4-0x80208B30 (7 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
#include "game/wvector.h"
#include "sdk_protos.h"
#include "classes/CTutorial.h"
#include "classes/CameraTransform800D.h"
#include <decomp/cw_bitfields.h>
extern "C" char s_tutorialDialogRectMemberDescriptor[]; // Tutorial-dialog rectangle member descriptor used by Runtime_AdjustMemberPointer.

#define AsCTutorial(p) ((CTutorial*)(p))
extern "C" {

void appFreeUni__FPv(void*);
void SharedDoc_SelectCourse(int);
char* strcpy(char*, const char*);
void InputManager_ClearNodeFlagBit1(void*);
void Pointer_ClearInputSuppression(void*);
void IntStringTree_DestroySubtree(void*, void*);
void ResourceManager_ReleaseObject(void*, void*, int);
void TutorialDialog_ReleaseSharedResources(void);
void __dl__FPv(void*);
extern void* g_sharedDoc;
extern void* g_resourceManager;
extern void* g_inputPointer;
extern void* g_sharedDoc;
extern char s_gameInputManager[];
extern char s_inputManagerDtorRegistration_tutorial[];
extern char s_cameraTransform[];
void* Resource_OpenResolvedPath(void*, const char*, int, int, int, int);
void* __nwa__FUl(unsigned int);
void* memset(void*, int, unsigned int);
void* IntStringTree_FindInsertPosition(void*, const int*, void**, unsigned char*, unsigned char*);
void String_ReserveCapacity(void*, int);
void String_ReplaceRange(void*, int, int, const char*, const char*, void*);
void* IntStringTree_InsertAtPosition(void*, void*, unsigned char, unsigned char, void*);
void* Resource_Release(void*);
void __dla__FPv(void*);
void* ResourceManager_LoadByExtension(void*, const char*, int, int, int);
void Tutorial_LoadMessageResources(void);
int CTutorial_PollSharedDialogLayout(void);
void HeapString_dtor(void*);

void ResourceHandle_dtor(void*);
// ignoredCapacity is an ABI argument and is NEVER checked. dst must hold the full
// decoded string plus NUL (the loader supplies 256 bytes and trusts the resource).
// No opening quote: return null without writing dst. No closing quote: return null
// after writing a prefix WITHOUT a terminator. Success writes NUL and returns the
// next source byte. Source and destination are borrowed; only backslash-n is decoded.
char* Text_ParseQuotedString(void*, char*, char*, int);
extern signed char s_inputManagerInitialized;
extern "C" const float kFloat_1000_Tutorial, kFloat_6_Tutorial, kTutorialMarkerProjectionEpsilon, kTutorialMarkerMinForwardProjection;
extern "C" const float kFloat_288_Tutorial, kTutorialMarkerMinCosine, kFloat_1_Tutorial_805DFD74, kFloat_0_Tutorial;
extern "C" const float kFloat_0_05235988_Tutorial, kTutorialMarkerDirectionScale, kTutorialGroundMarkerRadius;
void InputManager_Init(void*);
void InputManager_Dtor(void);
void CTutorial_ExecuteCurrentCommand(void*);
void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
float __ml__FRC7WVectorRC7WVector(void*, void*);
float Magnitude__7WVectorCFv(void*);
double acos(double);
float GolfParameter_ResolveFloat(void*, int);
void* InstanceTable_Get(void);
void __ml__FfRC7WVector(void*, void*, float);
void* GolfBall__Fv(void);
void __pl__FRC7WVectorRC7WVector(void*, void*, void*);
float WVectorLen__FRC7WVector(void*);
void CTutorial_BeginStageCompletion(void*);
void* CWind_GetInstance();
void CWind_LoadCourseRegions(void*);
void CWind_InitOverrideFromCourse(void*);
void CWind_SetStrength(void*, float);
void Pointer_ClearInputSuppression(void*);
void CTutorial_ResetLessonCommands(void*, int);
extern void* g_inputPointer;
void Vec2_Copy(void*, int);
void TutorialDialog_CreateForMessageIndex(int, int, void*, float);
void* Runtime_AdjustMemberPointer(int, void*, void*);
extern int g_mouseCursor;
extern int tutorialMessageIndices[];
extern signed char s_inputManagerInitialized;
extern float kFloat_0_5_Tutorial, kFloat_119_0, kFloat_82_InGameDialog, kTutorialDialogRightX, kFloat_341_TutorialDialog, kFloat_390_TutorialDialog;

// Lazily construct the global input manager (registering its destructor) and publish it in the singleton slot.
// The caller tests the slot first; the one-time construction is guarded by s_inputManagerInitialized.
static inline void EnsureGameInputManager() {
    if (!s_inputManagerInitialized) {
        InputManager_Init((void*)s_gameInputManager);
        __register_global_object((void*)s_gameInputManager, (void*)InputManager_Dtor, (void*)s_inputManagerDtorRegistration_tutorial);
        s_inputManagerInitialized = 1;
    }
    ((int&)Sing_805DC5C8()) = (int)s_gameInputManager;
}
#pragma opt_common_subs off
// Tutorial messages carry a command at +4 and either an integer or an output pointer at +8.
struct TutorialEvent {
    unsigned int sender;
    unsigned int command;
    union { int value; int* result; };
};
enum TutorialMessage {
    kExecuteCommand, kCompleteStage, kSetMessagePosition,
    kShowMessage = 4, kSetDialogFlag, kQueryDialog, kQueryActive
};
// Input IDs below describe this tutorial's policy; their physical control names are
// unresolved. Dialog mode unblocks exactly 19, 24 and 17 after blocking all 39 actions.
// The repeated singleton checks are inlined accessors with global registration side
// effects; preserve their ordering relative to action stores and external calls.
enum TutorialDialogInput { kDialogAllowedAction19 = 19,
    kDialogAllowedAction24 = 24, kDialogAllowedAction17 = 17 };
enum TutorialLessonId { kCameraAimLesson = 0, kGroundPositionLesson = 1,
    kLastTutorialLesson = 5 };
enum TutorialProcessState { kTutorialMessageWaiting = 0, kTutorialMonitorLesson = 1 };
enum { kTutorialMessageBufferBytes = 256, kCommandRecordsPerBlock = 25,
    kCommandRetainThreshold = 50, kEmptyCommandReadOffset = 12 };
enum { kTutorialRule = 2, kTutorialFinishedStage = 6, kTutorialActionCount = 39,
    kTutorialMessageIndexCount = 5, kTutorialDialogKind = 6,
    kTutorialUpperCursorGroup = 0, kTutorialLowerCursorGroup = 1 };
const unsigned int kTutorialFloatSignBit = 0x80000000U;
struct UiObj802083AC {
    int objectTag;
    void* vtable;
    float left, top;
};
struct TutorialCursor { char reserved[0x108]; int moveToActive; };
struct TutorialMarkerVectors {
    char beforePosition[0xac];
    float position[3];
    char beforeDirection[0x33c - 0xb8];
    float direction[3];
};
struct UiVtable802083AC {
    char pad[0x4C];
    void (*setRect)(void*, void*, void*);
    char pad60[0x10];
    void (*setPos)(void*, int, float, float);
    char pad64[0x4];
    void (*setFlag)(void*, int, int);
    char pad6C[0x4];
    int (*query)(void*);
};
// Opaque 12-byte member descriptor, copied before setRect; its words are not recovered coordinates.
struct Rect802083AC { int x, y, z; };
// Borrow a dialog's dispatch table; callbacks may replace self->messageDialog.
static inline UiVtable802083AC* TutorialDialogMethods(void* dialog) {
    return (UiVtable802083AC*)((UiObj802083AC*)dialog)->vtable;
}

// Dispatch tutorial commands, position updates, dialog creation and state queries.
// event is borrowed for this call. +8 is an integer/message index, a Vec2 pointer
// for kSetMessagePosition, or a writable int* for query messages. The latter must
// outlive this synchronous call. kShowMessage clamps only the upper index bound;
// callers must not pass negative message indices. Dialogs are stored on self.
void CTutorial_HandleMsg(void* self, const void* event) {
    switch (((const TutorialEvent*)event)->command) {
    case kExecuteCommand:
        CTutorial_ExecuteCurrentCommand(self);
        break;
    case kCompleteStage:
        CTutorial_BeginStageCompletion(self);
        break;
    case kSetMessagePosition:
        Vec2_Copy(AsCTutorial(self)->messagePosition, ((const TutorialEvent*)event)->value); // original 0x802083AC: self +0x54
        break;
    case kShowMessage: {
        int index = ((const TutorialEvent*)event)->value;
        if (index >= kTutorialMessageIndexCount) index = kTutorialMessageIndexCount - 1;
        int file = tutorialMessageIndices[index];
        AsCTutorial(self)->state = kTutorialMessageWaiting;
        if (((int&)Sing_805DC5C8()) == 0) EnsureGameInputManager();
        // Phase 1: block physical lesson inputs, then allow the three dialog inputs.
        void* state = (void*)((int&)Sing_805DC5C8());
        for (int action = 0; action < kTutorialActionCount; ++action) ((GameInputManager*)state)->actionBlocked[action] = 1;
        if (((int&)Sing_805DC5C8()) == 0) EnsureGameInputManager();
        ((GameInputManager*)((int&)Sing_805DC5C8()))->selectedPadLookupDisabled = 1; // original +0x154
        if (((int&)Sing_805DC5C8()) == 0) EnsureGameInputManager();
        ((GameInputManager*)((int&)Sing_805DC5C8()))->actionBlocked[kDialogAllowedAction19] = 0;
        if (((int&)Sing_805DC5C8()) == 0) EnsureGameInputManager();
        ((GameInputManager*)((int&)Sing_805DC5C8()))->actionBlocked[kDialogAllowedAction24] = 0;
        if (((int&)Sing_805DC5C8()) == 0) EnsureGameInputManager();
        ((GameInputManager*)((int&)Sing_805DC5C8()))->actionBlocked[kDialogAllowedAction17] = 0;
        Pointer_ClearInputSuppression(g_inputPointer);
        // Phase 2: create the owned dialog and attach the tutorial's secondary interface.
        TutorialDialog_CreateForMessageIndex(kTutorialDialogKind, file, ((char*)&AsCTutorial(self)->messageDialog), kFloat_0_5_Tutorial);
        void* child = self;
        if (self) child = ((char*)&AsCTutorial(self)->secondaryVtable);
        Rect802083AC rect;
        Rect802083AC* sourceRect = (Rect802083AC*)Runtime_AdjustMemberPointer(-24, (void*)((char*)&s_tutorialDialogRectMemberDescriptor), &rect);
        rect = *sourceRect;
        UiObj802083AC* obj = (*(UiObj802083AC**)&AsCTutorial(self)->messageDialog);
        // Phase 3: position the dialog and both cursor groups in UI layout coordinates.
        TutorialDialogMethods(obj)->setRect(obj, &rect, child);
        obj = (*(UiObj802083AC**)&AsCTutorial(self)->messageDialog);
        obj->left = kFloat_119_0;
        obj->top = kFloat_82_InGameDialog;
        TutorialDialogMethods(obj)->setPos(obj, kTutorialUpperCursorGroup, kTutorialDialogRightX, kFloat_341_TutorialDialog);
        obj = (*(UiObj802083AC**)&AsCTutorial(self)->messageDialog);
        TutorialDialogMethods(obj)->setPos(obj, kTutorialLowerCursorGroup, kTutorialDialogRightX, kFloat_390_TutorialDialog);
        int cancelCursorMove = 0;
        ((TutorialCursor*)g_mouseCursor)->moveToActive = cancelCursorMove;
        break;
    }
    case kSetDialogFlag: {
        void* obj = AsCTutorial(self)->messageDialog;
        if (obj == 0) break;
        void* golfDocument = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        if (AsCGolfDoc(golfDocument)->tutorialStage == kLastTutorialLesson) break;
        if (((const TutorialEvent*)event)->value != 0)
            TutorialDialogMethods(obj)->setFlag(obj, 0, 1);
        else
            TutorialDialogMethods(obj)->setFlag(obj, 0, 0);
        break;
    }
    case kQueryDialog: {
        void* obj = AsCTutorial(self)->messageDialog;
        if (obj != 0 && TutorialDialogMethods(obj)->query(obj))
            *((const TutorialEvent*)event)->result = 1;
        else
            *((const TutorialEvent*)event)->result = 0;
        break;
    }
    case kQueryActive:
        if (AsCTutorial(self)->state == kTutorialMonitorLesson)
            *((const TutorialEvent*)event)->result = 1;
        else
            *((const TutorialEvent*)event)->result = 0;
        break;
    }
}
#pragma opt_common_subs reset

// Reset lesson completion, wind and input suppression before starting the selected lesson.
void CTutorial_OnInit(void* self) {
    void* golfDocument = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    if (AsCGolfDoc(golfDocument)->tutorialStage != kTutorialFinishedStage) {
        // Phase 1: clear completion state and remove course wind for deterministic lessons.
        int zero = 0;
        AsCTutorial(self)->stage0Completed = zero;
        AsCTutorial(self)->stage1Completed = zero;
        AsCTutorial(self)->stage2Completed = zero;
        AsCTutorial(self)->stage3Completed = zero;
        AsCTutorial(self)->stage4Completed = zero;
        AsCTutorial(self)->stage5Completed = zero;
        void* courseWind = (void*)CWind_GetInstance(); CWind_LoadCourseRegions(courseWind); void* windOverride = (void*)CWind_GetInstance(); CWind_InitOverrideFromCourse(windOverride); void* wind = (void*)CWind_GetInstance();
        CWind_SetStrength(wind, kFloat_0_Tutorial);
        // Phase 2: establish the dialog input policy before rebuilding the lesson queue.
        int g0 = ((int&)Sing_805DC5C8());
        AsCTutorial(self)->firstStageCompletion = zero;
        AsCTutorial(self)->completedStageCount = zero;
        (*(int*)&AsCTutorial(self)->modeMessageDialog) = zero;
        if (g0 == 0) {
            if (!s_inputManagerInitialized) {
                InputManager_Init((void*)s_gameInputManager);
                __register_global_object((void*)s_gameInputManager, (void*)InputManager_Dtor, (void*)s_inputManagerDtorRegistration_tutorial);
                s_inputManagerInitialized = 1;
            }
            ((int&)Sing_805DC5C8()) = (int)s_gameInputManager;
        }
        void* state = (void*)((int&)Sing_805DC5C8());
        for (int action = 0; action < kTutorialActionCount; ++action) ((GameInputManager*)state)->actionBlocked[action] = 1;
        if (state == 0) EnsureGameInputManager();
        state = (void*)((int&)Sing_805DC5C8());
        ((GameInputManager*)state)->selectedPadLookupDisabled = 1; // original +0x154
        if (state == 0) EnsureGameInputManager();
        state = (void*)((int&)Sing_805DC5C8());
        ((GameInputManager*)state)->actionBlocked[kDialogAllowedAction19] = 0;
        if (state == 0) EnsureGameInputManager();
        state = (void*)((int&)Sing_805DC5C8());
        ((GameInputManager*)state)->actionBlocked[kDialogAllowedAction24] = 0;
        if (state == 0) EnsureGameInputManager();
        state = (void*)((int&)Sing_805DC5C8());
        ((GameInputManager*)state)->actionBlocked[kDialogAllowedAction17] = 0;
        Pointer_ClearInputSuppression(g_inputPointer);
        golfDocument = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        // Phase 3: start commands for the existing tutorialStage.
        CTutorial_ResetLessonCommands(self, AsCGolfDoc(golfDocument)->tutorialStage);
    }
}

#pragma use_lmw_stmw on
#pragma opt_common_subs off
#pragma opt_strength_reduction off
struct TutorialFile {
    virtual void v0();
    virtual void v1();
    virtual void read(char*, unsigned int);
    unsigned int size;
};
struct TutorialFileGuard {
    TutorialFile** file;
    // Guard one borrowed file-pointer slot, releasing its resource on scope exit.
    TutorialFileGuard(TutorialFile** p) : file(p) {}
    // Release the still-live resource and clear the caller's pointer slot.
    ~TutorialFileGuard() {
        if (*file) {
            if (*file) {
                Resource_Release(*file);
                *file = 0;
            }
        }
    }
};
struct TutorialAllocator {
    // Preserve the original empty string allocator tag.
    TutorialAllocator() {}
};
struct TutorialString {
    union {
        unsigned int flags;
        struct { CW_BITS2( unsigned int long_mode : 1, unsigned int capacity : 31) } bits;
    };
    unsigned int length;
    char* data;
    // Initialize the short-string empty representation.
    TutorialString() {
        unsigned int zero;
        zero = 0;
        flags = zero; length = zero; data = (char*)zero;
    }
    // Return length from the active short-string or heap representation.
    unsigned int size() const {
        unsigned int n;
        if (bits.long_mode == 0) n = *(const unsigned char*)this & 0x7f;
        else n = length;
        return n;
    }
    TutorialString(const TutorialString& other, unsigned int n);
    // Replace a range using the original allocator-by-address ABI.
    TutorialString& replace(int pos, int count, const char* first, const char* last, TutorialAllocator allocator) {
        String_ReplaceRange(this, pos, count, first, last, &allocator);
        return *this;
    }
    // Return the allocator tag used by this string specialization.
    TutorialAllocator get_allocator() const { TutorialAllocator allocator; return allocator; }
    // Replace a range with exactly n source bytes.
    TutorialString& replace_count(int pos, int count, const char* str, unsigned int n, TutorialAllocator allocator) {
        String_ReplaceRange(this, pos, count, str, str + n, &allocator);
        return *this;
    }
    TutorialString& operator=(const char* str);
    // Free only heap-mode text; short text occupies the object itself.
    ~TutorialString() {
        if (flags >> 31) appFreeUni__FPv(data);
    }
};
struct TutorialPair {
    int key;
    TutorialString value;
    // Copy the map key and its string payload into a new entry.
    TutorialPair(const int& k, const TutorialString& s) : key(k), value(s, s.length) {}
};
struct TutorialTreeNode { void* left; void* right; void* parent; TutorialPair pair; };
struct TutorialMap {
// Return the mapped string, inserting an empty owned value when the key is absent.
inline TutorialString& operator[](const int& token) {
    struct FindFlags { unsigned char a, b; } flags;
    void* field;
    TutorialPair* value;
    void* position;
    position = IntStringTree_FindInsertPosition(this, &token, &field, &flags.a, &flags.b);
    if (field == 0 || (value = &((TutorialTreeNode*)field)->pair)->key < token) {
        value = &((TutorialTreeNode*)IntStringTree_InsertAtPosition(this, position, flags.a, flags.b, &TutorialPair(token, TutorialString())))->pair;
    }
    return value->value;
}
};
// Copy the supplied string through the existing length-sensitive construction path.
inline TutorialString::TutorialString(const TutorialString& other, unsigned int n) {
    unsigned int zero;
    TutorialString* dest;
    zero = 0;
    dest = this;
    if (!n) {
        flags = zero; length = zero; data = (char*)zero;
    } else {
        flags = zero; length = zero; data = (char*)zero;
        String_ReserveCapacity(dest, 0);
        replace(0, 0, other.data, other.data + other.length, other.get_allocator());
    }
}
// Assign NUL-terminated text, replacing the complete previous value.
inline TutorialString& TutorialString::operator=(const char* str) {
    unsigned int n = size();
    return replace_count(0, n, str, strlen(str), get_allocator());
}
struct TutorialCommandPool {
    char screen[7];
    char filename[13];
    char integerFormat[3];
    char markerFilename[18];
};
extern char tutorialCommandStrings[];
// Load quoted tutorial messages into the map and prepare marker/dialog resources.
void CTutorial_OnLoad(void* self) {
    const char* stringPool;
    char* text;
    char* cursor;
    long size;
    void* golfDocument = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    if (AsCGolfDoc(golfDocument)->tutorialStage != kTutorialFinishedStage) {
        // Phase 1: read the NUL-padded message resource and populate the owned text map.
        stringPool = tutorialCommandStrings;
        TutorialFile* parsed = (TutorialFile*)Resource_OpenResolvedPath(g_resourceManager, ((const TutorialCommandPool*)stringPool)->filename, -1, 0, 0, 0);
        TutorialFileGuard guard(&parsed);
        if (parsed != 0) {
            size = parsed->size;
            text = (char*)__nwa__FUl(size + 1);
            memset(text, 0, size + 1);
            parsed->read(text, size);
            // Resource contract: each decoded quoted message fits in 255 bytes plus NUL.
            // Text_ParseQuotedString ignores the nominal capacity and may fail without NUL.
            // This loop assumes valid input; it inserts output even on a failed parse.
            char output[kTutorialMessageBufferBytes];
            cursor = text;
            while (cursor && strlen(cursor)) {
                int token;
                sscanf(cursor, ((const TutorialCommandPool*)stringPool)->integerFormat, &token);
                cursor = Text_ParseQuotedString(self, cursor, output, 0x100);
                (*(TutorialMap*)&AsCTutorial(self)->textMapCount)[token] = output; // original map +0x40
            }
            if (parsed) {
                Resource_Release(parsed);
                parsed = 0;
            }
            __dla__FPv(text);
        }
        golfDocument = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        // Phase 2: load the world-space marker model and wait for shared dialog layout.
        *(void**)&AsCGolfDoc(golfDocument)->tutorialMarkerModel = ResourceManager_LoadByExtension(g_resourceManager, ((const TutorialCommandPool*)tutorialCommandStrings)->markerFilename, 0, 1, 0); // original +0xA8
        Tutorial_LoadMessageResources();
        while (!CTutorial_PollSharedDialogLayout()) {}
    }
}
#pragma opt_common_subs reset
#pragma opt_strength_reduction reset

// Copy the next quoted string, expanding backslash-n escapes; return the cursor after its closing quote.
char* Text_ParseQuotedString(void* unused, char* src, char* dst, int ignoredCapacity) {
    while (*src != '"' && *src != 0) {
        ++src;
    }
    if (*src == 0) {
        return 0;
    }
    ++src;
    char fill = '\n';
    while (*src != 0 && *src != '"') {
        if (*src == '\\' && src[1] == 'n') {
            *dst++ = fill;
            src += 2;
        } else {
            *dst++ = *src++;
        }
    }
    if (*src == 0) {
        return 0;
    }
    *dst = 0;
    return src + 1;
}

struct CleanupNode80207DC8 {
    void* leftChild;  // left subtree link
    void* rightChild; // right subtree link
    int parentLink;
    void* optional;
    unsigned int flags;
    int textLength;
    void* heapCommandText;
};
struct CleanupList80207DC8 { CleanupNode80207DC8* first; CleanupNode80207DC8* second; };

// Release the two subtrees and the node's heap-backed command text.
inline void CleanTutorialNode(CleanupNode80207DC8* node, void* owner) {
    if (node->leftChild) IntStringTree_DestroySubtree(&AsCTutorial(owner)->textMapCount, node->leftChild);
    if (node->rightChild) IntStringTree_DestroySubtree(&AsCTutorial(owner)->textMapCount, node->rightChild);
    if (&node->optional && &node->flags && (node->flags >> 31)) appFreeUni__FPv(node->heapCommandText);
    appFreeUni__FPv(node);
}

// Release one expanded branch: both children, then this node's text and allocation.
// The scratch link is borrowed from the caller to retain its register lifetime.
static inline void CleanTutorialBranch(CleanupNode80207DC8* child, void* owner,
    CleanupNode80207DC8*& grandchild) {
    grandchild = (CleanupNode80207DC8*)child->leftChild;
    if (grandchild) { CleanTutorialNode(grandchild, owner); }
    grandchild = (CleanupNode80207DC8*)child->rightChild;
    if (grandchild) { CleanTutorialNode(grandchild, owner); }
    if ((&child->optional) && (&child->flags) && (child->flags >> 31))
        appFreeUni__FPv(child->heapCommandText);
    appFreeUni__FPv(child);
}

#pragma use_lmw_stmw on
// Restore course selection and release queued commands, message-map nodes and shared tutorial resources.
void CTutorial_OnDestroy(void* self) {
    int emptyReadOffset;

    int index;

    CleanupNode80207DC8* branch;
    CleanupNode80207DC8* root;
    CleanupNode80207DC8* child;
    CleanupNode80207DC8* grandchild;
    if (AsCSharedDoc(g_sharedDoc)->golfRuleType == kTutorialRule) { unsigned int zero;
        SharedDoc_SelectCourse(0);
        zero = 0;
        AsCSharedDoc(g_sharedDoc)->golfRuleType = zero;
        strcpy(AsCSharedDoc(g_sharedDoc)->pad_8C1, (*(char**)&AsCSharedDoc(g_sharedDoc)->strokePlayName));
        InputManager_ClearNodeFlagBit1(g_inputPointer);
        Pointer_ClearInputSuppression(g_inputPointer);
        // Phase 1: drain queue entries; blocks retain a consumed prefix until offset 50.
        if (AsCTutorial(self)->pendingCommandCount) {
            emptyReadOffset = kEmptyCommandReadOffset;
            do {
                AsCTutorial(self)->pendingCommandCount -= 1;
                AsCTutorial(self)->commandReadOffset += 1;
                while ((*(unsigned int*)&AsCTutorial(self)->commandReadOffset) >= kCommandRetainThreshold) {
                    appFreeUni__FPv(*(void**)((*(char**)&AsCTutorial(self)->commandBlocks) + AsCTutorial(self)->firstCommandBlockIndex * 4));
                    AsCTutorial(self)->firstCommandBlockIndex = AsCTutorial(self)->firstCommandBlockIndex + 1;
                    if ((unsigned int)AsCTutorial(self)->firstCommandBlockIndex == AsCTutorial(self)->commandBlockCapacity) AsCTutorial(self)->firstCommandBlockIndex = zero;
                    AsCTutorial(self)->commandBlockCount -= 1;
                    AsCTutorial(self)->commandReadOffset -= kCommandRecordsPerBlock;
                }
                if (AsCTutorial(self)->pendingCommandCount == 0) AsCTutorial(self)->commandReadOffset = emptyReadOffset;
            } while (AsCTutorial(self)->pendingCommandCount != 0);
        }
        // Phase 2: release map nodes and only heap-mode string payloads.
        root = (*(CleanupNode80207DC8**)&AsCTutorial(self)->textMapRoot);
        if (root) {
            branch = (CleanupNode80207DC8*)root->leftChild;
            if (branch) {
                child = (CleanupNode80207DC8*)branch->leftChild;
                if (child) { CleanTutorialBranch(child, self, grandchild); }
                child = (CleanupNode80207DC8*)branch->rightChild;
                if (child) { CleanTutorialBranch(child, self, grandchild); }
                if ((&branch->optional) && (&branch->flags) && (branch->flags >> 31)) appFreeUni__FPv(branch->heapCommandText);
                appFreeUni__FPv(branch);
            }
            branch = (CleanupNode80207DC8*)root->rightChild;
            if (branch) {
                child = (CleanupNode80207DC8*)branch->leftChild;
                if (child) { CleanTutorialBranch(child, self, grandchild); }
                child = (CleanupNode80207DC8*)branch->rightChild;
                if (child) { CleanTutorialBranch(child, self, grandchild); }
                if ((&branch->optional) && (&branch->flags) && (branch->flags >> 31)) appFreeUni__FPv(branch->heapCommandText);
                appFreeUni__FPv(branch);
            }
            if ((&root->optional) && (&root->flags) && (root->flags >> 31)) appFreeUni__FPv(root->heapCommandText);
            appFreeUni__FPv(root);
            AsCTutorial(self)->textMapCount = 0;
            AsCTutorial(self)->textMapRoot = 0;
            AsCTutorial(self)->textMapLast = ((char*)&AsCTutorial(self)->textMapRoot);
        }
        // Phase 3: return marker ownership to the resource manager and release shared UI.
        if (g_resourceManager) {
            void* golfDocument = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
            void* obj = *(void**)&AsCGolfDoc(golfDocument)->tutorialMarkerModel; // original +0xA8
            if (obj) {
                ResourceManager_ReleaseObject(g_resourceManager, obj, 0);
                golfDocument = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
                *(void**)&AsCGolfDoc(golfDocument)->tutorialMarkerModel = 0; // original +0xA8
            }
            TutorialDialog_ReleaseSharedResources();
        }
    }
}


#pragma use_lmw_stmw on
#pragma opt_common_subs off
#pragma opt_strength_reduction off
#pragma opt_propagation off
#pragma opt_lifetimes off
// Inject a forced input, advance the command timer, and check lesson-specific aim or position completion.
void CTutorial_OnProcess(void* self, float dt) {
    // Vector scratch stays in declaration order to preserve register and stack allocation.
    struct Vec { float x, y, z; } horizontalSeparation, projectedBallPosition, directionOffset, cameraToMarker;
    float ratio, aimErrorRadians, forwardProjection, length;
    if (AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->tutorialStage == kTutorialFinishedStage) return;
    // Phase 1: synthesize held action presses and consume elapsed time in truncated milliseconds.
    // dt is seconds; countdown crosses below zero to advance (exactly zero waits).
    if (AsCTutorial(self)->forcedActionIndex >= 0) {
        if (((int&)Sing_805DC5C8()) == 0) EnsureGameInputManager();
        ((GameInputManager*)((int&)Sing_805DC5C8()))->pendingActionPress[AsCTutorial(self)->forcedActionIndex] = 1;
    }
    if (AsCTutorial(self)->stepCountdown >= 0) {
        AsCTutorial(self)->stepCountdown = AsCTutorial(self)->stepCountdown - (int)(kFloat_1000_Tutorial * dt);
        if (AsCTutorial(self)->stepCountdown < 0) CTutorial_ExecuteCurrentCommand(self);
    }
    // Phase 2: only the monitor state checks geometric lesson completion.
    int lessonComplete;
    switch (AsCTutorial(self)->state) {
    case kTutorialMessageWaiting:
        break;
    case kTutorialMonitorLesson: {
        void* golfDocument = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        int lesson = AsCGolfDoc(golfDocument)->tutorialStage;
        switch (lesson) {
        case kCameraAimLesson: {
            // World-space camera-to-marker vector, raised 6 world units in Y.
            // Dot with camera forward gives depth: require 96 < depth < 288 world units,
            // abs(depth) >= 1e-5, then acos(clamp(depth/length,-1,1)) < pi/60 (3 degrees).
            // This checks facing AND depth; it is not a radial distance test.
            __mi__FRC7WVectorRC7WVector(&cameraToMarker, ((TutorialMarkerVectors*)golfDocument)->position, AsCameraTransform800D(s_cameraTransform)->position);
            cameraToMarker.y += kFloat_6_Tutorial;
            forwardProjection = (*(WVector*)(&AsCameraTransform800D(s_cameraTransform)->basis[6]) * *(WVector*)&cameraToMarker);
            int projectionBits = *(int*)&forwardProjection;
            float absoluteProjection;
            if (projectionBits & kTutorialFloatSignBit) { absoluteProjection = -forwardProjection; } else { absoluteProjection = forwardProjection; }
            int projectionTooSmall = absoluteProjection < kTutorialMarkerProjectionEpsilon;
            if (projectionTooSmall) { lessonComplete = 0; break; }
            if (!(forwardProjection > kTutorialMarkerMinForwardProjection && forwardProjection < kFloat_288_Tutorial)) { lessonComplete = 0; break; }
            ratio = forwardProjection / (*(WVector*)&cameraToMarker).Magnitude();
            if (ratio < kTutorialMarkerMinCosine) ratio = kTutorialMarkerMinCosine;
            else if (ratio > kFloat_1_Tutorial_805DFD74) ratio = kFloat_1_Tutorial_805DFD74;
            aimErrorRadians = (float)acos((double)ratio);
            if (aimErrorRadians < kFloat_0_Tutorial) aimErrorRadians = -aimErrorRadians;
            if (aimErrorRadians < kFloat_0_05235988_Tutorial) {
                ++AsCTutorial(self)->aimAlignmentCount;
                CTutorial_ExecuteCurrentCommand(self);
                lessonComplete = 0;
            }
            lessonComplete = 0;
            break;
        }
        case kGroundPositionLesson: {
            // Project a point from the ball along marker direction by 3.2 * parameter(0).
            // Ignore Y, then require horizontal separation from the marker <16 world units.
            // The physical units of parameter(0) are unresolved; 3.2 is its conversion factor.
            void* parameterTable = InstanceTable_Get();
            float projectionParameter = GolfParameter_ResolveFloat(parameterTable, 0);
            __ml__FfRC7WVector(&directionOffset, ((TutorialMarkerVectors*)golfDocument)->direction, kTutorialMarkerDirectionScale * projectionParameter);
            void* ballPosition = GolfBall__Fv();
            __pl__FRC7WVectorRC7WVector(&projectedBallPosition, ballPosition, &directionOffset);
            __mi__FRC7WVectorRC7WVector(&horizontalSeparation, ((TutorialMarkerVectors*)golfDocument)->position, &projectedBallPosition);
            horizontalSeparation.y = kFloat_0_Tutorial;
            lessonComplete = (WVectorLen(*(WVector*)&horizontalSeparation) < kTutorialGroundMarkerRadius);
            break;
        }
        default:
            lessonComplete = 0;
            break;
        }
        if (lessonComplete) CTutorial_BeginStageCompletion(self);
        break;
    }
    default:
        break;
    }
}
#pragma use_lmw_stmw reset
#pragma opt_common_subs reset
#pragma opt_strength_reduction reset
#pragma opt_propagation reset
#pragma opt_lifetimes reset

struct Node802076A4 { Node802076A4* prev; Node802076A4* next; };
struct List802076A4 { char unk[4]; int count; Node802076A4 head; };


// Unlink and free all reference-list nodes, then optionally delete the list header.
// Inlined std::list destructor: the nested identical `(char*)self + 4` tests are the library's repeated
// null checks (always true); the body unlinks every node and frees it, decrementing the count.
void* RefObjectList_dtor(List802076A4* self, int del) {
    if (self != 0) {
        if ((char*)self + 4) { if ((char*)self + 4) { if ((char*)self + 4) { if ((char*)self + 4) { Node802076A4* end = &self->head;
        Node802076A4* node = end->next;
        if (node != end) {
            Node802076A4* t1 = end->prev;
            Node802076A4* t2 = node->prev;
            Node802076A4* t3 = t1->next;
            t2->next = t3;
            Node802076A4* t4 = node->prev;
            Node802076A4* t5 = t1->next;
            t5->prev = t4;
            while (node != end) {
                Node802076A4* current = node;
                node = node->next;
                appFreeUni__FPv(current);
                --self->count;
            }
        }
        }
        }
        }
        }
        if (del > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

} // extern "C"
