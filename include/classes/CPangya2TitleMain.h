#ifndef CLASSES_CPANGYA2TITLEMAIN_H
#define CLASSES_CPANGYA2TITLEMAIN_H

#include "game/types.h"

class CPangya2TitleMain {
public:
    void* vtbl;            // 0x0: original vtable pointer
    char pad_004[0x14];    // 0x4
    void* interfaceVtable; // 0x18
    void* activeMenu;      // 0x1c
    char pad_020[0x4];     // 0x20
    int menuState;         // 0x24
    char pad_028[0x4];     // 0x28
    int topMenuResult;     // 0x2c
    void* textureResource; // 0x30

    static void* GetRTTI();
    void OnProcess();
    void HandleMsg(const void* arg);
    void OnDisplay();
    void StartPracticeSession(int alternatePractice);

    void OnDestroy();
    static void OnPreLoadInit();
    void OnLoad();
    void OnInit();

    CPangya2TitleMain();
    ~CPangya2TitleMain();
    void* DtorThunk24(int deleting);
};

#define CPANGYA2TITLEMAIN_OFF(f, o) \
    typedef char CPangya2TitleMain_off_##f[((int)&((CPangya2TitleMain*)0)->f == (o)) ? 1 : -1]
CPANGYA2TITLEMAIN_OFF(interfaceVtable, 0x18);
CPANGYA2TITLEMAIN_OFF(activeMenu, 0x1c);
CPANGYA2TITLEMAIN_OFF(menuState, 0x24);
CPANGYA2TITLEMAIN_OFF(topMenuResult, 0x2c);
CPANGYA2TITLEMAIN_OFF(textureResource, 0x30);
#undef CPANGYA2TITLEMAIN_OFF

#endif // CLASSES_CPANGYA2TITLEMAIN_H
