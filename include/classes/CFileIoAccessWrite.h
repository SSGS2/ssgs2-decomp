// The original vtable (__vt__18CFileIoAccessWrite) and typeinfo stay data symbols.
#ifndef CLASSES_CFILEIOACCESSWRITE_H
#define CLASSES_CFILEIOACCESSWRITE_H

#include "classes/CFileIoAccess.h"

// RTTI base class: CFileIoAccess (not declared here)
class CFileIoAccessWrite {
public:
    void* vtbl;                   // 0x00: original vtable pointer (__vt__18CFileIoAccessWrite)
    void* resource;               // 0x04: open resource (released by the CFileIoAccess destructor)
    unsigned char failureLatched; // 0x08: 1 while the access is usable
    char pad09[3];
    int state;    // 0x0C: write state machine step, 4 when finished
    void* buffer; // 0x10: written data, freed by the destructor

    // 0x0C (vtable slot 3): deleting destructor; keeps the object alive when flag <= 0
    void* destroy(int flag);

    // Initialize the access base, write state and empty buffer on existing storage; returns this.
    void* Init();
    // Replace the owned buffer with a copy of sourceData, then create the access resource.
    int InitializeBuffer(void* openOptions, void* byteCount, void* sourceData);
    // Set the flash-write two-pi cache.
    static void InitAngleCache();

    int IsFinished();
    int vfunc_2();
    static int Update(void* access);
};

#endif // CLASSES_CFILEIOACCESSWRITE_H
