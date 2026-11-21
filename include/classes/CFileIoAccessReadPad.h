// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// The original vtable (__vt__20CFileIoAccessReadPad) and typeinfo stay data symbols.
#ifndef CLASSES_CFILEIOACCESSREADPAD_H
#define CLASSES_CFILEIOACCESSREADPAD_H

#include "classes/CFileIoAccessRead.h"

// RTTI base class: CFileIoAccess (not declared here)
class CFileIoAccessReadPad {
public:
    void* vtbl;           // 0x00: original vtable pointer (__vt__20CFileIoAccessReadPad)
    void* resource;       // 0x04: open resource (released by the CFileIoAccess destructor)
    unsigned char failureLatched;  // 0x08: 1 while the access is usable
    char pad09[3];
    int state;            // 0x0C: read state machine step (CFileIoAccessWrite_Update), 4 when finished
    void* buffer;         // 0x10: data the pad object is built from

    // 0x0C (vtable slot 3): deleting destructor; keeps the object alive when flag <= 0
    void* destroy(int flag);
    // 0x18 (vtable slot 6): the pad object built on top of buffer
    void* createPad();
};

#endif  // CLASSES_CFILEIOACCESSREADPAD_H
