#ifndef CLASSES_CTASK_H
#define CLASSES_CTASK_H

#include "game/types.h"

class CTask {
public:
    ~CTask();

    char pad_000[4];                 // 0x0: unmodelled bytes
    char pad_004[0xc];               // 0x4
    unsigned char transitionPending; // 0x10: CTask_RequestTransition original object access
    char pad_011[0x3];               // 0x11
    void* transitionArg0;            // 0x14: CTask_RequestTransition original object access
    void* transitionArg1;            // 0x18: CTask_RequestTransition original object access
    void* child;                     // 0x1c: Object_GetChild original object access

    void* Init(); // fills in the task's storage; the original returns the task
    void SetMainActor(void* actor);

    void UpdateCurrentTaskWithFadeGates(float deltaSeconds);
    void DrawCurrentTaskOrLoading();
    static void* GetRTTI();

    void RequestTransition(const char* taskName, const char* transitionData);
    void RequestTransitionWithFade(const char* taskName, const char* transitionData, int startFade);
    void ProcessPendingTransition();
    void ReplaceActiveTaskByName(const char* name, const char* childname);
    void CancelChildThread();

    void vfunc_16();
    void vfunc_18();
    void vfunc_19();

    static void vfunc_1_ThunkMinus8(void* a, int forwarded1);
    void LoadActorsCallback_Thunk4(void* forwarded1, void* forwarded2);

    static void OnCreateActorsDefault();

    void StartActorLoading();
    void InitActors();
    void LoadActorsCallback(void*, void* slot);
    static int IsActorLoadingComplete(void*);

    void ReloadActors();
    void NotifyActorsDestroy();
    void PreLoadInitActors();
    void ApplyModeToActors(int mode);
    void DisplayActorsByPriority();
    void ProcessActors(float time);
};

#define CTASK_OFF(f, o) typedef char CTask_off_##f[((int)&((CTask*)0)->f == (o)) ? 1 : -1]
CTASK_OFF(transitionPending, 0x10);
CTASK_OFF(transitionArg0, 0x14);
CTASK_OFF(transitionArg1, 0x18);
CTASK_OFF(child, 0x1c);
#undef CTASK_OFF

#endif // CLASSES_CTASK_H
