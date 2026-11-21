// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CEVENTCONTEPROC_H
#define CLASSES_CEVENTCONTEPROC_H

#include "game/types.h"

class CEventConteProc {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x4];                  // 0x4
    char eventNameTag;                         // 0x8: 1 use in CEventConteProc_dtor
    char inlineEventName;                         // 0x9: 1 use in CEventConteProc_dtor
    char pad_00A[0xa];                  // 0xa
    char eventKindTag;                        // 0x14: 1 use in CEventConteProc_dtor
    char inlineEventKind;                        // 0x15: 1 use in CEventConteProc_dtor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x470, 0x474
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CEVENTCONTEPROC_OFF(f, o) typedef char CEventConteProc_off_##f[((int)&((CEventConteProc*)0)->f == (o)) ? 1 : -1]
CEVENTCONTEPROC_OFF(eventNameTag, 0x8);
CEVENTCONTEPROC_OFF(inlineEventName, 0x9);
CEVENTCONTEPROC_OFF(eventKindTag, 0x14);
CEVENTCONTEPROC_OFF(inlineEventKind, 0x15);
#undef CEVENTCONTEPROC_OFF

#endif  // CLASSES_CEVENTCONTEPROC_H
