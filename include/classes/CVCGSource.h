#ifndef CLASSES_CVCGSOURCE_H
#define CLASSES_CVCGSOURCE_H

#include "game/types.h"

class CVCGSource {
public:
    void* vtbl;        // 0x0: original vtable pointer
    int groupCount;    // 0x4
    void* entries;     // 0x8
    int entryCount;    // 0xc
    int entryCapacity; // 0x10

    CVCGSource();
    ~CVCGSource();
};

#define CVCGSOURCE_OFF(f, o) typedef char CVCGSource_off_##f[((int)&((CVCGSource*)0)->f == (o)) ? 1 : -1]
CVCGSOURCE_OFF(groupCount, 0x4);
CVCGSOURCE_OFF(entries, 0x8);
CVCGSOURCE_OFF(entryCount, 0xc);
CVCGSOURCE_OFF(entryCapacity, 0x10);
#undef CVCGSOURCE_OFF

#endif // CLASSES_CVCGSOURCE_H
