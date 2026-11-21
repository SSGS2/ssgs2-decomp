// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_STRINGPAIR800D750C_H
#define CLASSES_STRINGPAIR800D750C_H

#include "game/types.h"

class StringPair800D750C {
public:
    char pad_000[4]; // 0x0: unmodelled bytes
    char pad_004[0x4];                  // 0x4
    void* firstStringData;              // 0x8: StringPair800D750C_dtor original object access
    char pad_00C[0x8];                  // 0xc
    void* secondStringData;             // 0x14: StringPair800D750C_dtor original object access
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define STRINGPAIR800D750C_OFF(f, o) typedef char StringPair800D750C_off_##f[((int)&((StringPair800D750C*)0)->f == (o)) ? 1 : -1]
STRINGPAIR800D750C_OFF(firstStringData, 0x8);
STRINGPAIR800D750C_OFF(secondStringData, 0x14);
#undef STRINGPAIR800D750C_OFF

#endif  // CLASSES_STRINGPAIR800D750C_H
