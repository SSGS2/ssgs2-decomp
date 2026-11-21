// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CPASSWORDMAIN_H
#define CLASSES_CPASSWORDMAIN_H

#include "game/types.h"

class CPasswordMain {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x14];                 // 0x4
    void* taskMainVtable;                       // 0x18: 2 uses in CPasswordMain_Ctor, CPasswordMain_dtor
    int passwordPanelEntryId;                         // 0x1c: 9 uses in CPasswordMain_OnDestroy, CPasswordMain_dtor, CPasswordMain_OnProcess...
    int backgroundEntryId;                         // 0x20: 9 uses in CPasswordMain_OnDestroy, CPasswordMain_dtor, CPasswordMain_OnProcess...
    int titleCaptionEntryId;                         // 0x24: 9 uses in CPasswordMain_OnDestroy, CPasswordMain_dtor, CPasswordMain_OnProcess...
    void* textRenderer;                       // 0x28: 10 uses in CPasswordMain_OnDestroy, CPasswordMain_OnProcess, CPasswordMain_dtor...
    unsigned char transitionRequested;               // 0x2c: 3 uses in CPasswordMain_OnProcess, CPasswordMain_Ctor
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CPASSWORDMAIN_OFF(f, o) typedef char CPasswordMain_off_##f[((int)&((CPasswordMain*)0)->f == (o)) ? 1 : -1]
CPASSWORDMAIN_OFF(taskMainVtable, 0x18);
CPASSWORDMAIN_OFF(passwordPanelEntryId, 0x1c);
CPASSWORDMAIN_OFF(backgroundEntryId, 0x20);
CPASSWORDMAIN_OFF(titleCaptionEntryId, 0x24);
CPASSWORDMAIN_OFF(textRenderer, 0x28);
CPASSWORDMAIN_OFF(transitionRequested, 0x2c);
#undef CPASSWORDMAIN_OFF

#endif  // CLASSES_CPASSWORDMAIN_H
