// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CCOMMATCHTBL_H
#define CLASSES_CCOMMATCHTBL_H

#include "game/types.h"

class CComMatchTbl {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x18];                 // 0x4
    int selectedMatchIndex;                         // 0x1c: 3 uses in CComSelectMatch_BuildRoundSettings
    void* matchPageIndex;                       // 0x20: 2 uses in CComSelectMatch_GetOpponentPortraitLayout, CComSelectMatch_GetCourseDescription
    char pad_024[0x5c];                 // 0x24
    void* matchTableFile;                       // 0x80: 3 uses in CComSelectMatch_GetOpponentPortraitLayout, CComSelectMatch_GetCourseDescription, CComSelectMatch_BuildRoundSettings
    char pad_084[0x4];                  // 0x84
    void* messageDraw;                       // 0x88: 1 use in CComSelectMatch_GetCourseDescription
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CCOMMATCHTBL_OFF(f, o) typedef char CComMatchTbl_off_##f[((int)&((CComMatchTbl*)0)->f == (o)) ? 1 : -1]
CCOMMATCHTBL_OFF(selectedMatchIndex, 0x1c);
CCOMMATCHTBL_OFF(matchPageIndex, 0x20);
CCOMMATCHTBL_OFF(matchTableFile, 0x80);
CCOMMATCHTBL_OFF(messageDraw, 0x88);
#undef CCOMMATCHTBL_OFF

#endif  // CLASSES_CCOMMATCHTBL_H
