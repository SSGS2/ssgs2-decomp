// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// The original vtable (__vt__18CFileIoAccessWrite) and typeinfo stay data symbols.
#ifndef CLASSES_CFILEIOACCESSWRITE_H
#define CLASSES_CFILEIOACCESSWRITE_H

#include "classes/CFileIoAccess.h"

// RTTI base class: CFileIoAccess (not declared here)
class CFileIoAccessWrite {
public:
    void* vtbl;           // 0x00: original vtable pointer (__vt__18CFileIoAccessWrite)
    void* resource;       // 0x04: open resource (released by the CFileIoAccess destructor)
    unsigned char failureLatched;  // 0x08: 1 while the access is usable
    char pad09[3];
    int state;            // 0x0C: write state machine step, 4 when finished
    void* buffer;         // 0x10: written data, freed by the destructor

    // 0x0C (vtable slot 3): deleting destructor; keeps the object alive when flag <= 0
    void* destroy(int flag);
};

#endif  // CLASSES_CFILEIOACCESSWRITE_H
