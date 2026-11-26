// Shop task: the task that creates, draws and releases the Pangya2 shop (CShop2Main).
// Its state is set through setState; destroy frees the task when the flag asks for it.

#include "game/singletons.h"
#include "classes/CShop2Main.h"
#include "classes/CTaskMain.h"
#include "classes/CTask.h"
#include "classes/CPangya2Shop.h"

// Type of the object held in CShop2Main::shop; only its virtual destructor is needed here (delete).
struct Base80080CF4 {
    virtual ~Base80080CF4();
};

extern "C" {

extern char __vt__10CShop2Main[];
extern char titleSaveTransitionStrings[];
extern char s_saveData[];
extern void* g_soundManager;
extern char s_saveDataInitialized;
extern void* g_taskManager;

void* __dt__18NativeBase80201D18Fv(void*, int);
void* __dl__FPv(void*);
void* __nw__FUl(int);
void SoundManager_RequestBgmChange(void*, void*, int, int);
void SaveData_Ctor(void*);
void* SaveData_GetCurrentRecord(void*);
void SaveRecord_CopyCtor(void*, void*);
int MPBase1_Process(void*, float);
void SaveProcess_SetTransitionTargetName(void*);
void SaveProcess_SetFollowUpTaskName(void*);
void TaskCommandQueue_AppendNamedRequest(void*, int, void*, int, int, int, int, int);
void MPBase1_DrawIfInitialized(int);

} // extern "C"

// The 0x8e0-byte object built from a buffer and stored in CShop2Main::shop.
struct Cls80080DA4 {
    char data[0x8e0];
    // Construct an owned shop from a borrowed save-record snapshot; z is forwarded unchanged.
    Cls80080DA4(void* b, int z) {
        ((CPangya2Shop*)this)->Init(b, z);
    }
};

// Offsets into the existing packed string table, not independently owned strings.
enum ShopTransitionStrings {
    kShopBgmName = 0,
    kSaveTransitionTarget = 0xE,
    kSaveFollowUpTask = 0x19,
    kSaveTransitionTask = 0x24,
    kSaveTransitionArgument = 0x35,
    kQueuedShopTask = 0x36
};
// The event handler accepts exactly these values; their wider task meaning is unresolved.
enum ShopTaskState {
    kShopTaskState1 = 1,
    kShopTaskState2 = 2
};
enum {
    kShopSecondaryVtableOffset = 0x3C,
    kShopTaskRequestCode = 6
};

// Deleting destructor: restore the vtables, drop the shop object, then the base destructor and, for a
// positive flag, the freed memory.
void* CShop2Main::destroy(int flag) {
    if (this) {
        Base80080CF4* ownedShop = (Base80080CF4*)shop;
        char* primary = __vt__10CShop2Main;
        char* secondary = primary + kShopSecondaryVtableOffset;
        vtbl = (void*)primary;
        vtbl2 = (void*)secondary;
        if (ownedShop) {
            delete ownedShop;
            shop = 0;
        }
        if (this)
            __dt__18NativeBase80201D18Fv(this, 0);
        if (flag > 0)
            __dl__FPv(this);
    }
    return this;
}

// Empty task lifecycle hook; no shop state changes.
void CShop2Main::vfunc_4() {}

// Empty task initialization hook.
void CShop2Main::vfunc_2() {}

// Create the owned shop from the current save record; create() expects no existing shop.
void CShop2Main::create() {
    char recordSnapshot[0x830]; // 0x828-byte record plus the original stack-buffer slack.
    SoundManager_RequestBgmChange(g_soundManager, titleSaveTransitionStrings + kShopBgmName, 0, 1);
    // Resolve the save singleton, constructing its static storage only once.
    if (GetSaveDataSlot() == 0) {
        if (s_saveDataInitialized == 0) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        GetSaveDataSlot() = s_saveData;
    }
    // The stack snapshot is borrowed only during construction; shop survives this call.
    SaveRecord_CopyCtor(recordSnapshot, SaveData_GetCurrentRecord(GetSaveDataSlot()));
    shop = new Cls80080DA4(recordSnapshot, 0);
}

// Empty task lifecycle hook.
void CShop2Main::vfunc_8() {}

// Process the shop; on completion configure the save flow and queue the next task.
// Keep the original exported name while expressing the engine's incoming f1 argument.
// dt is passed through in the engine's time units; this wrapper does not rescale it.
extern "C" void update__10CShop2MainFv(CShop2Main* self, float dt) {
    if (MPBase1_Process(self->shop, dt)) {
        SaveProcess_SetTransitionTargetName(titleSaveTransitionStrings + kSaveTransitionTarget);
        SaveProcess_SetFollowUpTaskName(titleSaveTransitionStrings + kSaveFollowUpTask);
        ((CTask*)g_taskManager)
            ->RequestTransitionWithFade(titleSaveTransitionStrings + kSaveTransitionTask,
                                        titleSaveTransitionStrings + kSaveTransitionArgument, 0);
        TaskCommandQueue_AppendNamedRequest(g_taskManager, 0, titleSaveTransitionStrings + kQueuedShopTask,
                                            kShopTaskRequestCode, 0, 0, 0, 0);
    }
}

// Dispatch the shop's initialized-only draw hook; ownership remains with this task.
void CShop2Main::draw() {
    MPBase1_DrawIfInitialized((int)shop);
}

// State 1 and 2 are selectable; any other value leaves the state alone.
void CShop2Main::setState(CShop2MainEvent* arg) {
    int requestedState = arg->state;
    if (requestedState != kShopTaskState2) {
        if (requestedState >= kShopTaskState2)
            return;
        if (requestedState < kShopTaskState1)
            return;
        state = kShopTaskState1;
        return;
    }
    state = kShopTaskState2;
}

extern "C" {

// Plain object deleter (no base destructor): free the memory for a positive flag.
void* __dt__11AuxMenuBaseFv(void* self, int flag) {
    if (self != 0 && flag > 0) {
        __dl__FPv(self);
    }
    return self;
}

} // extern "C"

// Deleting destructor of the task: base destructor, then the freed memory for a positive flag.
void* CTaskMain::destroy(int flag) {
    if (this != 0) {
        __dt__18NativeBase80201D18Fv(this, 0);
        if (flag > 0)
            __dl__FPv(this);
    }
    return this;
}
