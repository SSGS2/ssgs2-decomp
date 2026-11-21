// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CTAILEFFECT_H
#define CLASSES_CTAILEFFECT_H

#include "game/types.h"

class CTailEffect {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* records;                        // 0x4: 1 use in CTailEffect_dtor
    int recordCount; // 0x8: CTailEffect_ClearTrail clears after invalidating 30 records
    unsigned int recordCapacity; // 0xc: first TBuf capacity (TTail layout in tail_effect_spin_and_bubble.cpp)
    void* pendingRecords;                       // 0x10: 1 use in CTailEffect_dtor
    int pendingCount; // 0x14: CTailEffect_dtor clears before freeing pending records
    unsigned int pendingCapacity; // 0x18: second TBuf capacity (TTail layout)
    float previousFirst[3]; // 0x1c: CTailEffect_AddSegment compares and stores first endpoint
    float previousSecond[3]; // 0x28: CTailEffect_AddSegment compares and stores second endpoint
    float recordAge; // 0x34: CTailEffect_AddSegment assigns to input/output record ages
    float unk_38; // 0x38: TTail layout
    int writeIndex; // 0x3c: CTailEffect_AddSegment appends interpolated records modulo 30
    int readIndex; // 0x40: CTailEffect_AddSegment advances on ring collision
    unsigned char unk_44; // 0x44: TTail layout
    char pad_045[3];
    void* texture; // 0x48: CTailEffect_dtor releases through ResourceManager_ReleaseId
    int pendingWriteIndex; // 0x4c: CTailEffect_AddSegment appends source records modulo 30
    int pendingReadIndex; // 0x50: CTailEffect_AddSegment advances on pending ring collision
    unsigned char unk_54; // 0x54: TTail layout
    char pad_055[3];
    void* textureGeneratorVtable;                       // 0x58: 1 use in CTailEffect_dtor

    void* renderBuffer; // 0x5c: CTailEffect_dtor frees with __dla__FPv

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x4, 0x8, 0x10, 0x14, 0x54
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CTAILEFFECT_OFF(f, o) typedef char CTailEffect_off_##f[((int)&((CTailEffect*)0)->f == (o)) ? 1 : -1]
CTAILEFFECT_OFF(records, 0x4);
CTAILEFFECT_OFF(pendingRecords, 0x10);
CTAILEFFECT_OFF(textureGeneratorVtable, 0x58);
CTAILEFFECT_OFF(vtbl, 0x0);
CTAILEFFECT_OFF(recordCount, 0x8);
CTAILEFFECT_OFF(recordCapacity, 0xc);
CTAILEFFECT_OFF(pendingCount, 0x14);
CTAILEFFECT_OFF(pendingCapacity, 0x18);
CTAILEFFECT_OFF(previousFirst, 0x1c);
CTAILEFFECT_OFF(previousSecond, 0x28);
CTAILEFFECT_OFF(recordAge, 0x34);
CTAILEFFECT_OFF(unk_38, 0x38);
CTAILEFFECT_OFF(writeIndex, 0x3c);
CTAILEFFECT_OFF(readIndex, 0x40);
CTAILEFFECT_OFF(unk_44, 0x44);
CTAILEFFECT_OFF(texture, 0x48);
CTAILEFFECT_OFF(pendingWriteIndex, 0x4c);
CTAILEFFECT_OFF(pendingReadIndex, 0x50);
CTAILEFFECT_OFF(unk_54, 0x54);
CTAILEFFECT_OFF(renderBuffer, 0x5c);
#undef CTAILEFFECT_OFF

#endif  // CLASSES_CTAILEFFECT_H
