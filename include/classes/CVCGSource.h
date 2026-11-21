// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CVCGSOURCE_H
#define CLASSES_CVCGSOURCE_H

#include "game/types.h"

class CVCGSource {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    int groupCount;                          // 0x4: 3 uses in CVCGSource_dtor, CVCGSource_Ctor
    void* entries;                        // 0x8: 5 uses in CVCGSource_dtor, CVCGSource_Ctor
    int entryCount;                          // 0xc: 5 uses in CVCGSource_dtor, CVCGSource_Ctor
    int entryCapacity;                         // 0x10: 1 use in CVCGSource_Ctor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x8
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CVCGSOURCE_OFF(f, o) typedef char CVCGSource_off_##f[((int)&((CVCGSource*)0)->f == (o)) ? 1 : -1]
CVCGSOURCE_OFF(groupCount, 0x4);
CVCGSOURCE_OFF(entries, 0x8);
CVCGSOURCE_OFF(entryCount, 0xc);
CVCGSOURCE_OFF(entryCapacity, 0x10);
#undef CVCGSOURCE_OFF

#endif  // CLASSES_CVCGSOURCE_H
