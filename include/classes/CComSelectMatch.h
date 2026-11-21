// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CCOMSELECTMATCH_H
#define CLASSES_CCOMSELECTMATCH_H

#include "game/types.h"

class CComSelectMatch {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x14];                 // 0x4
    void* bpEntryId;                       // 0x18: 2 uses in CComSelectMatch_GetRowGroupPosition, CComSelectMatch_GetCategoryGroupPosition
    int selectedMatchIndex;                         // 0x1c: 3 uses in CComSelectMatch_BuildRoundSettings
    void* matchPageIndex;                       // 0x20: 3 uses in CComSelectMatch_GetOpponentPortraitLayout, CComSelectMatch_GetCourseDescription, CComSelectMatch_GetResultCategoryLayout
    char pad_024[0x30];                 // 0x24
    union {
        struct { void* firstMatchLayout; char pad_058[0x1c]; };
        void* matchLayouts[8]; // 0x54: CComSelectMatch_ctor creates eight layouts; vfunc_2 polls each.
    };
    union {
        struct { void* firstCategoryLayout; char pad_078[0x8]; };
        void* categoryLayouts[3]; // 0x74: ctor creates three layouts; dtor destroys each.
    };
    void* matchTableFile;                       // 0x80: 7 uses in CComSelectMatch_dtor, CComSelectMatch_GetOpponentPortraitLayout, CComSelectMatch_GetCourseDescription...
    void* messageResourceFile;                       // 0x84: 4 uses in CComSelectMatch_dtor, CComSelectMatch_RebuildMessageDraw, CComSelectMatch_AreResourcesLoaded
    void* messageDraw;                       // 0x88: 13 uses in CComSelectMatch_DescribeMatchSelection, CComSelectMatch_RebuildMessageDraw, CComSelectMatch_dtor...

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x4, 0x88
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CCOMSELECTMATCH_OFF(f, o) typedef char CComSelectMatch_off_##f[((int)&((CComSelectMatch*)0)->f == (o)) ? 1 : -1]
CCOMSELECTMATCH_OFF(bpEntryId, 0x18);
CCOMSELECTMATCH_OFF(selectedMatchIndex, 0x1c);
CCOMSELECTMATCH_OFF(matchPageIndex, 0x20);
CCOMSELECTMATCH_OFF(firstMatchLayout, 0x54);
CCOMSELECTMATCH_OFF(firstCategoryLayout, 0x74);
CCOMSELECTMATCH_OFF(matchTableFile, 0x80);
CCOMSELECTMATCH_OFF(messageResourceFile, 0x84);
CCOMSELECTMATCH_OFF(messageDraw, 0x88);
CCOMSELECTMATCH_OFF(matchLayouts, 0x54);
CCOMSELECTMATCH_OFF(categoryLayouts, 0x74);
#undef CCOMSELECTMATCH_OFF

#endif  // CLASSES_CCOMSELECTMATCH_H
