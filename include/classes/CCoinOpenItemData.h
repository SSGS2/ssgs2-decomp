#ifndef CLASSES_CCOINOPENITEMDATA_H
#define CLASSES_CCOINOPENITEMDATA_H

#include "game/types.h"

class CCoinOpenItemData {
public:
    ~CCoinOpenItemData();

    void* vtbl;             // 0x0: original vtable pointer
    void* itemFile;         // 0x4
    void* messageFile;      // 0x8
    void* cameraController; // 0xc
    char pad_010[0x24];     // 0x10
    void* mapData;          // 0x34
    void* coinItemData;     // 0x38
    char pad_03C[0x8];      // 0x3c
    void* numberDraw;       // 0x44
    void* menuContext;      // 0x48
    char pad_04C[0x4];      // 0x4c
    void* selectionRecord;  // 0x50
    char pad_054[0x18];     // 0x54
    void* modeGroup;        // 0x6c
    void* itemGroup;        // 0x70
    void* numberGroup;      // 0x74
    char pad_078[0x18];     // 0x78
    unsigned char mode;     // 0x90
    char pad_091[0x7];      // 0x91
    short initialSelection; // 0x98

    int vfunc_31();
    int vfunc_30();
    int vfunc_29();
    int vfunc_28();
    int vfunc_27();
    int vfunc_26();
    int vfunc_25();
    int vfunc_24();
    int vfunc_23();
    int vfunc_22();
    int vfunc_21();
    int vfunc_20();
    int vfunc_19();
    int vfunc_18();
    int vfunc_17();
    int vfunc_16();
    int vfunc_15();
    int vfunc_14();
    int vfunc_13();
    int vfunc_12();
    int vfunc_11();
    int vfunc_10();
    int vfunc_9();
    int vfunc_8();
    int vfunc_7();
    int vfunc_6();
    int vfunc_5();
    int vfunc_4();
    int vfunc_3();
    int vfunc_2();
    int vfunc_1();
};

#define CCOINOPENITEMDATA_OFF(f, o) \
    typedef char CCoinOpenItemData_off_##f[((int)&((CCoinOpenItemData*)0)->f == (o)) ? 1 : -1]
CCOINOPENITEMDATA_OFF(itemFile, 0x4);
CCOINOPENITEMDATA_OFF(messageFile, 0x8);
CCOINOPENITEMDATA_OFF(cameraController, 0xc);
CCOINOPENITEMDATA_OFF(mapData, 0x34);
CCOINOPENITEMDATA_OFF(coinItemData, 0x38);
CCOINOPENITEMDATA_OFF(numberDraw, 0x44);
CCOINOPENITEMDATA_OFF(menuContext, 0x48);
CCOINOPENITEMDATA_OFF(selectionRecord, 0x50);
CCOINOPENITEMDATA_OFF(modeGroup, 0x6c);
CCOINOPENITEMDATA_OFF(itemGroup, 0x70);
CCOINOPENITEMDATA_OFF(numberGroup, 0x74);
CCOINOPENITEMDATA_OFF(mode, 0x90);
CCOINOPENITEMDATA_OFF(initialSelection, 0x98);
#undef CCOINOPENITEMDATA_OFF

#endif // CLASSES_CCOINOPENITEMDATA_H
