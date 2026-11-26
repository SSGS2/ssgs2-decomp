#ifndef CLASSES_CSCORE_H
#define CLASSES_CSCORE_H

#include "game/types.h"

struct Fn801EC280Object;
class CScore {
public:
    void* vtbl;               // 0x0: original vtable pointer
    char pad_004[0x18];       // 0x4
    int bonusOverlayEnabled;  // 0x1c
    float elapsedTime;        // 0x20
    int transitionState;      // 0x24
    char pad_028[0x4];        // 0x28
    float transitionProgress; // 0x2c
    char pad_030[0x8];        // 0x30
    void* bonusOverlay;       // 0x38
    int result;               // 0x3c
    int scoreMenuActive;      // 0x40
    int currentPlayerId;      // 0x44
    char pad_048[0x8];        // 0x48
    int rankViewActive;       // 0x50
    int menusInitialized;     // 0x54
    char pad_058[0x4];        // 0x58
    void* scoreForm;          // 0x5c
    void* rankView;           // 0x60

    void OnDestroy();
    static void OnLoad();
    static void OnReload(Fn801EC280Object* first);

    void OnProcess(float deltaSeconds);
    void HandleMsg(void* message);
    void OnDisplay();
    static void* GetRTTI();

    CScore();

    ~CScore();

    // Destructor thunk for the secondary interface at 0x18.
    void* DestroyThunkMinus24(int deleting);
};

#define CSCORE_OFF(f, o) typedef char CScore_off_##f[((int)&((CScore*)0)->f == (o)) ? 1 : -1]
CSCORE_OFF(bonusOverlayEnabled, 0x1c);
CSCORE_OFF(elapsedTime, 0x20);
CSCORE_OFF(transitionState, 0x24);
CSCORE_OFF(transitionProgress, 0x2c);
CSCORE_OFF(bonusOverlay, 0x38);
CSCORE_OFF(result, 0x3c);
CSCORE_OFF(scoreMenuActive, 0x40);
CSCORE_OFF(currentPlayerId, 0x44);
CSCORE_OFF(rankViewActive, 0x50);
CSCORE_OFF(menusInitialized, 0x54);
CSCORE_OFF(scoreForm, 0x5c);
CSCORE_OFF(rankView, 0x60);
#undef CSCORE_OFF

#endif // CLASSES_CSCORE_H
