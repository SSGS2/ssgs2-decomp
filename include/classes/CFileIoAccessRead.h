// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// The original vtable (__vt__17CFileIoAccessRead) and typeinfo stay data symbols.
#ifndef CLASSES_CFILEIOACCESSREAD_H
#define CLASSES_CFILEIOACCESSREAD_H

#include "classes/CFileIoAccess.h"

// RTTI base class: CFileIoAccess (not declared here)
class CFileIoAccessRead {
public:
    void* vtbl;           // 0x00: original vtable pointer (__vt__17CFileIoAccessRead)
    void* resource;       // 0x04: open resource (released by the CFileIoAccess destructor)
    unsigned char failureLatched;  // 0x08: 1 while the access is usable
    char pad09[3];
    int state;            // 0x0C: read state machine step (CFileIoAccessRead_Update), 4 when finished

    // 0x0C (vtable slot 3): deleting destructor; keeps the object alive when flag <= 0
    void* destroy(int flag);
};

#endif  // CLASSES_CFILEIOACCESSREAD_H
