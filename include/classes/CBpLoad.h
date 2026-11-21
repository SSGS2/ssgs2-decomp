// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CBPLOAD_H
#define CLASSES_CBPLOAD_H

#include "game/types.h"

class CBpLoad {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    int loaded;                          // 0x4: 1 use in CBpObject_Ctor
    unsigned int filenameStorage;                 // 0x8: 5 uses in CBpLoad_dtor, CBpLoad_SetFilenameAndOpenSharedFile (CONFLICT sizes [1, 4])
    void* filenameLongLength;                        // 0xc: 3 uses in CBpLoad_dtor, CBpObject_Ctor
    void* filenameHeapData;                       // 0x10: 3 uses in CBpLoad_dtor, CBpObject_Ctor
    void* sharedFileHandle;                       // 0x14: 3 uses in CBpLoad_dtor, CBpObject_Ctor
    int mutexStorage;                         // 0x18: 2 uses in CBpObject_Ctor
    void* unk_1C;                       // 0x1c: 3 uses in CBpObject_Ctor
    void* unk_20;                       // 0x20: 2 uses in CBpObject_Ctor
    void* sprite;                       // 0x24: 1 use in CBpObject_Ctor
    int unk_28;                         // 0x28: 1 use in CBpObject_Ctor
    int unk_2C;                         // 0x2c: 1 use in CBpObject_Ctor
    int unk_30;                         // 0x30: 1 use in CBpObject_Ctor
    int unk_34;                         // 0x34: 1 use in CBpObject_Ctor
    int unk_38;                         // 0x38: 1 use in CBpObject_Ctor
    int unk_3C;                         // 0x3c: 1 use in CBpObject_Ctor
    char pad_040[0x4];                  // 0x40
    int unk_44;                         // 0x44: 1 use in CBpObject_Ctor
    void* unk_48;                       // 0x48: 1 use in CBpObject_Ctor
    void* unk_4C;                       // 0x4c: 1 use in CBpObject_Ctor
    int unk_50;                         // 0x50: 1 use in CBpObject_Ctor
    void* unk_54;                       // 0x54: 1 use in CBpObject_Ctor
    void* unk_58;                       // 0x58: 1 use in CBpObject_Ctor
    void* unk_5C;                       // 0x5c: 1 use in CBpObject_Ctor
    int unk_60;                         // 0x60: 1 use in CBpObject_Ctor
    int unk_64;                         // 0x64: 1 use in CBpObject_Ctor
    int unk_68;                         // 0x68: 1 use in CBpObject_Ctor
    float unk_6C;                       // 0x6c: 1 use in CBpObject_Ctor
    float unk_70;                       // 0x70: 1 use in CBpObject_Ctor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x8, 0x9, 0x18, 0x1c, 0x44, 0x48, 0x54
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CBPLOAD_OFF(f, o) typedef char CBpLoad_off_##f[((int)&((CBpLoad*)0)->f == (o)) ? 1 : -1]
CBPLOAD_OFF(loaded, 0x4);
CBPLOAD_OFF(filenameStorage, 0x8);
CBPLOAD_OFF(filenameLongLength, 0xc);
CBPLOAD_OFF(filenameHeapData, 0x10);
CBPLOAD_OFF(sharedFileHandle, 0x14);
CBPLOAD_OFF(mutexStorage, 0x18);
CBPLOAD_OFF(unk_1C, 0x1c);
CBPLOAD_OFF(unk_20, 0x20);
CBPLOAD_OFF(sprite, 0x24);
CBPLOAD_OFF(unk_28, 0x28);
CBPLOAD_OFF(unk_2C, 0x2c);
CBPLOAD_OFF(unk_30, 0x30);
CBPLOAD_OFF(unk_34, 0x34);
CBPLOAD_OFF(unk_38, 0x38);
CBPLOAD_OFF(unk_3C, 0x3c);
CBPLOAD_OFF(unk_44, 0x44);
CBPLOAD_OFF(unk_48, 0x48);
CBPLOAD_OFF(unk_4C, 0x4c);
CBPLOAD_OFF(unk_50, 0x50);
CBPLOAD_OFF(unk_54, 0x54);
CBPLOAD_OFF(unk_58, 0x58);
CBPLOAD_OFF(unk_5C, 0x5c);
CBPLOAD_OFF(unk_60, 0x60);
CBPLOAD_OFF(unk_64, 0x64);
CBPLOAD_OFF(unk_68, 0x68);
CBPLOAD_OFF(unk_6C, 0x6c);
CBPLOAD_OFF(unk_70, 0x70);
#undef CBPLOAD_OFF

#endif  // CLASSES_CBPLOAD_H
