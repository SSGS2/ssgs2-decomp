// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CCOMSELECTSTROKE_H
#define CLASSES_CCOMSELECTSTROKE_H

#include "game/types.h"

class CComSelectStroke {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x14];                 // 0x4
    void* bpEntryId;                       // 0x18: 2 uses in CComSelectStroke_GetCourseMarkerPosition, CComSelectStroke_GetRankMarkerPosition
    char pad_01C[0x38];                 // 0x1c
    void* firstCourseMapTexture;                       // 0x54: 2 uses in CComSelectStroke_dtor
    char pad_058[0x34];                 // 0x58
    void* firstRankTexture;                       // 0x8c: 2 uses in CComSelectStroke_dtor
    char pad_090[0x1c];                 // 0x90
    void* strokeLevelFile;                       // 0xac: 5 uses in CComSelectStroke_Ctor, CComSelectStroke_dtor, CComSelectStroke_AreResourcesLoaded
    void* messageResourceFile;                       // 0xb0: 6 uses in CComSelectStroke_Ctor, CComSelectStroke_dtor, CComSelectStroke_AreResourcesLoaded...
    void* messageDraw;                       // 0xb4: 3 uses in CComSelectStroke_dtor, CComSelectStroke_Ctor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x4, 0x54, 0x8c
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CCOMSELECTSTROKE_OFF(f, o) typedef char CComSelectStroke_off_##f[((int)&((CComSelectStroke*)0)->f == (o)) ? 1 : -1]
CCOMSELECTSTROKE_OFF(bpEntryId, 0x18);
CCOMSELECTSTROKE_OFF(firstCourseMapTexture, 0x54);
CCOMSELECTSTROKE_OFF(firstRankTexture, 0x8c);
CCOMSELECTSTROKE_OFF(strokeLevelFile, 0xac);
CCOMSELECTSTROKE_OFF(messageResourceFile, 0xb0);
CCOMSELECTSTROKE_OFF(messageDraw, 0xb4);
#undef CCOMSELECTSTROKE_OFF

#endif  // CLASSES_CCOMSELECTSTROKE_H
