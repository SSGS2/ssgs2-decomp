#ifndef CLASSES_CSAVEPROCESSMAIN_H
#define CLASSES_CSAVEPROCESSMAIN_H

#include "game/types.h"

class CSaveProcessMain {
public:
    void* vtbl;               // 0x0: original vtable pointer
    char pad_004[0x14];       // 0x4
    void* secondaryVtable;    // 0x18
    int retainedRequestWord0; // 0x1c
    int transitionArg0;       // 0x20
    int transitionArg1;       // 0x24
    int transitionArg2;       // 0x28
    int transitionArg3;       // 0x2c
    int retainedRequestArg0;  // 0x30
    int retainedRequestArg1;  // 0x34
    int transitionArg4;       // 0x38
    int saveMenu;             // 0x3c

    static void* GetRTTI();
    void OnProcess(float dt);
    void OnDisplay();
    static void HandleMsg(void* dst, const void* src);

    CSaveProcessMain();
    ~CSaveProcessMain();

    // Destructor entry for the secondary interface's vtable: its this pointer is
    // 0x18 bytes past the object's start.
    void DtorThunk18(int deleting);
};

#define CSAVEPROCESSMAIN_OFF(f, o) \
    typedef char CSaveProcessMain_off_##f[((int)&((CSaveProcessMain*)0)->f == (o)) ? 1 : -1]
CSAVEPROCESSMAIN_OFF(secondaryVtable, 0x18);
CSAVEPROCESSMAIN_OFF(retainedRequestWord0, 0x1c);
CSAVEPROCESSMAIN_OFF(transitionArg0, 0x20);
CSAVEPROCESSMAIN_OFF(transitionArg1, 0x24);
CSAVEPROCESSMAIN_OFF(transitionArg2, 0x28);
CSAVEPROCESSMAIN_OFF(transitionArg3, 0x2c);
CSAVEPROCESSMAIN_OFF(retainedRequestArg0, 0x30);
CSAVEPROCESSMAIN_OFF(retainedRequestArg1, 0x34);
CSAVEPROCESSMAIN_OFF(transitionArg4, 0x38);
CSAVEPROCESSMAIN_OFF(saveMenu, 0x3c);
#undef CSAVEPROCESSMAIN_OFF

#endif // CLASSES_CSAVEPROCESSMAIN_H
