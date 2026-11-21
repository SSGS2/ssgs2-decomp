// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CPROJECTG_H
#define CLASSES_CPROJECTG_H

#include "game/types.h"

class CProjectG {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0xc];                  // 0x4
    void* interfaceVtable;                       // 0x10: 1 use in CProjectG_dtor
    char pad_014[0xc];                  // 0x14
    void* secondaryInterfaceVtable;                       // 0x20: 2 uses in CProject_Ctor, CProjectG_dtor
    int frameUpdateFlags;                         // 0x24: 1 use in CProject_Ctor
    char unk_28;                        // 0x28: 1 use in CProject_Ctor
    char pad_029[0x3];                  // 0x29
    unsigned int projectNameTag;                // 0x2c: 3 uses in CProject_Ctor (CONFLICT sizes [1, 4])
    int projectNameLongLength;                         // 0x30: 2 uses in CProject_Ctor
    int projectNameHeap;                         // 0x34: 1 use in CProject_Ctor
    char unk_38;                        // 0x38: 1 use in CProject_Ctor
    char unk_39;                        // 0x39: 1 use in CProject_Ctor
    char pad_03A[0x2];                  // 0x3a
    int string1Tag;                         // 0x3c: 1 use in CProject_Ctor
    int string1LongLength;                         // 0x40: 1 use in CProject_Ctor
    int string1Heap;                         // 0x44: 1 use in CProject_Ctor
    int string2Tag;                         // 0x48: 1 use in CProject_Ctor
    int string2LongLength;                         // 0x4c: 1 use in CProject_Ctor
    int string2Heap;                         // 0x50: 1 use in CProject_Ctor
    int string3Tag;                         // 0x54: 1 use in CProject_Ctor
    int string3LongLength;                         // 0x58: 1 use in CProject_Ctor
    int string3Heap;                         // 0x5c: 1 use in CProject_Ctor
    char pad_060[0x4];                  // 0x60
    float timeScale;                       // 0x64: 1 use in CProject_Ctor
    char pad_068[0x8];                  // 0x68
    void* allocatedDeviceBlocks;                       // 0x70: 3 uses in CProjectG_dtor
    char pad_074[0x10];                 // 0x74
    int deviceBuckets;                         // 0x84: 1 use in CProject_Ctor
    char pad_088[0x18];                 // 0x88
    int projectFlags;                         // 0xa0: 1 use in CProject_Ctor
    char restoreInputPending;                        // 0xa4: 1 use in CProject_Ctor
    char pad_0A5[0x3];                  // 0xa5
    int clockOriginMilliseconds;                         // 0xa8: 2 uses in CProject_Ctor
    int unk_AC;                         // 0xac: 1 use in CProject_Ctor
    int deviceFactory;                         // 0xb0: 2 uses in CProject_Ctor
    int deviceRegistrations;                         // 0xb4: 2 uses in CProject_Ctor
    char pad_0B8[0x4];                  // 0xb8
    int unk_BC;                         // 0xbc: 1 use in CProject_Ctor
    char pad_0C0[0x1fbc];               // 0xc0
    unsigned int unk_207C;              // 0x207c: 2 uses in CQuadTree_FreeBuffers
    char pad_2080[0xc8];                // 0x2080
    void* unk_2148;                     // 0x2148: 4 uses in CQuadTree_FreeBuffers
    char pad_214C[0x64];                // 0x214c
    void* unk_21B0;                     // 0x21b0: 2 uses in CQuadTree_FreeBuffers
    unsigned int unk_21B4;              // 0x21b4: 1 use in CQuadTree_FreeBuffers
    char pad_21B8[0x34];                // 0x21b8
    unsigned int unk_21EC;              // 0x21ec: 2 uses in CQuadTree_FreeBuffers
    char pad_21F0[0x14];                // 0x21f0
    void* unk_2204;                     // 0x2204: 2 uses in CQuadTree_FreeBuffers
    void* unk_2208;                     // 0x2208: 4 uses in CQuadTree_FreeBuffers

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x10, 0x2c, 0x3c, 0x48, 0x54, 0x68, 0x88, 0x21b4
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CPROJECTG_OFF(f, o) typedef char CProjectG_off_##f[((int)&((CProjectG*)0)->f == (o)) ? 1 : -1]
CPROJECTG_OFF(interfaceVtable, 0x10);
CPROJECTG_OFF(secondaryInterfaceVtable, 0x20);
CPROJECTG_OFF(frameUpdateFlags, 0x24);
CPROJECTG_OFF(unk_28, 0x28);
CPROJECTG_OFF(projectNameTag, 0x2c);
CPROJECTG_OFF(projectNameLongLength, 0x30);
CPROJECTG_OFF(projectNameHeap, 0x34);
CPROJECTG_OFF(unk_38, 0x38);
CPROJECTG_OFF(unk_39, 0x39);
CPROJECTG_OFF(string1Tag, 0x3c);
CPROJECTG_OFF(string1LongLength, 0x40);
CPROJECTG_OFF(string1Heap, 0x44);
CPROJECTG_OFF(string2Tag, 0x48);
CPROJECTG_OFF(string2LongLength, 0x4c);
CPROJECTG_OFF(string2Heap, 0x50);
CPROJECTG_OFF(string3Tag, 0x54);
CPROJECTG_OFF(string3LongLength, 0x58);
CPROJECTG_OFF(string3Heap, 0x5c);
CPROJECTG_OFF(timeScale, 0x64);
CPROJECTG_OFF(allocatedDeviceBlocks, 0x70);
CPROJECTG_OFF(deviceBuckets, 0x84);
CPROJECTG_OFF(projectFlags, 0xa0);
CPROJECTG_OFF(restoreInputPending, 0xa4);
CPROJECTG_OFF(clockOriginMilliseconds, 0xa8);
CPROJECTG_OFF(unk_AC, 0xac);
CPROJECTG_OFF(deviceFactory, 0xb0);
CPROJECTG_OFF(deviceRegistrations, 0xb4);
CPROJECTG_OFF(unk_BC, 0xbc);
CPROJECTG_OFF(unk_207C, 0x207c);
CPROJECTG_OFF(unk_2148, 0x2148);
CPROJECTG_OFF(unk_21B0, 0x21b0);
CPROJECTG_OFF(unk_21B4, 0x21b4);
CPROJECTG_OFF(unk_21EC, 0x21ec);
CPROJECTG_OFF(unk_2204, 0x2204);
CPROJECTG_OFF(unk_2208, 0x2208);
#undef CPROJECTG_OFF

#endif  // CLASSES_CPROJECTG_H
