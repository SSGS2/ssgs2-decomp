#ifndef CLASSES_CROUNDDOLMAIN_H
#define CLASSES_CROUNDDOLMAIN_H

#include "game/types.h"

class CRoundDolMain {
public:
    void* vtbl;                  // 0x0: original vtable pointer
    char pad_004[0x14];          // 0x4
    unsigned int taskMainVtable; // 0x18
    void* dolfiniSession;        // 0x1c

    void DrawStateUI();

    void OnDisplay();
    void OnDestroy();
    void OnProcess();
    void OnLoad();
    void DtorThunk18(int deleting);

    static void OnPreLoadInit();
    static void OnInit();
    static void* GetRTTI();

    CRoundDolMain();
    ~CRoundDolMain();
};

#define CROUNDDOLMAIN_OFF(f, o) typedef char CRoundDolMain_off_##f[((int)&((CRoundDolMain*)0)->f == (o)) ? 1 : -1]
CROUNDDOLMAIN_OFF(taskMainVtable, 0x18);
CROUNDDOLMAIN_OFF(dolfiniSession, 0x1c);
#undef CROUNDDOLMAIN_OFF

#endif // CLASSES_CROUNDDOLMAIN_H
