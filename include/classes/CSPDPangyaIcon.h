// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CSPDPANGYAICON_H
#define CLASSES_CSPDPANGYAICON_H

#include "game/types.h"

class CSPDPangyaIcon {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x44];                 // 0x4
    void* ownedChild0;                       // 0x48: 3 uses in CSPDPangyaIcon_PANGYA_ICON_OBJECT_VAR_dtor, CSPDPangyaIcon_PangyaIconObject_ctor
    void* ownedChild1;                       // 0x4c: 3 uses in CSPDPangyaIcon_PANGYA_ICON_OBJECT_VAR_dtor, CSPDPangyaIcon_PangyaIconObject_ctor
    void* ownedChild2;                       // 0x50: 3 uses in CSPDPangyaIcon_PANGYA_ICON_OBJECT_VAR_dtor, CSPDPangyaIcon_PangyaIconObject_ctor
    void* ownedChild3;                       // 0x54: 3 uses in CSPDPangyaIcon_PANGYA_ICON_OBJECT_VAR_dtor, CSPDPangyaIcon_PangyaIconObject_ctor
    unsigned char primaryChildIndex;               // 0x58: 1 use in CSPDPangyaIcon_PangyaIconObject_ctor
    unsigned char alternateChildIndex;               // 0x59: 1 use in CSPDPangyaIcon_PangyaIconObject_ctor
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CSPDPANGYAICON_OFF(f, o) typedef char CSPDPangyaIcon_off_##f[((int)&((CSPDPangyaIcon*)0)->f == (o)) ? 1 : -1]
CSPDPANGYAICON_OFF(ownedChild0, 0x48);
CSPDPANGYAICON_OFF(ownedChild1, 0x4c);
CSPDPANGYAICON_OFF(ownedChild2, 0x50);
CSPDPANGYAICON_OFF(ownedChild3, 0x54);
CSPDPANGYAICON_OFF(primaryChildIndex, 0x58);
CSPDPANGYAICON_OFF(alternateChildIndex, 0x59);
#undef CSPDPANGYAICON_OFF

#endif  // CLASSES_CSPDPANGYAICON_H
