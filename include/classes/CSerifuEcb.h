// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CSERIFUECB_H
#define CLASSES_CSERIFUECB_H

#include "game/types.h"

class CSerifuEcb {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* table;                        // 0x4: 5 uses in Text_ExpandTaggedText, MessageTable_GetTextByIndex, MessageTable_FindTextByName
    void* groupedTextTables;                        // 0x8: 1 use in MessageTable_FindTextByName
    int formatType;                          // 0xc: 2 uses in Text_ExpandTaggedText, MessageTable_GetTextByIndex
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CSERIFUECB_OFF(f, o) typedef char CSerifuEcb_off_##f[((int)&((CSerifuEcb*)0)->f == (o)) ? 1 : -1]
CSERIFUECB_OFF(table, 0x4);
CSERIFUECB_OFF(groupedTextTables, 0x8);
CSERIFUECB_OFF(formatType, 0xc);
#undef CSERIFUECB_OFF

#endif  // CLASSES_CSERIFUECB_H
