// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CVOICEMANAGER_H
#define CLASSES_CVOICEMANAGER_H

#include "game/types.h"

class CVoiceManager {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x14];                 // 0x4
    void* pendingVoices;                       // 0x18: 3 uses in CVoiceManager_dtor, CVoiceManager_OnProcess
    int pendingVoiceCount;                         // 0x1c: 2 uses in CVoiceManager_dtor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x18
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CVOICEMANAGER_OFF(f, o) typedef char CVoiceManager_off_##f[((int)&((CVoiceManager*)0)->f == (o)) ? 1 : -1]
CVOICEMANAGER_OFF(pendingVoices, 0x18);
CVOICEMANAGER_OFF(pendingVoiceCount, 0x1c);
#undef CVOICEMANAGER_OFF

#endif  // CLASSES_CVOICEMANAGER_H
