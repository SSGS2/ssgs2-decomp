// The original vtable (lbl_804B56E0) and typeinfo stay data symbols.
#ifndef CLASSES_CFILEIOACCESS_H
#define CLASSES_CFILEIOACCESS_H

class CFileIoAccess {
public:
    CFileIoAccess();

    void* vtbl;                   // 0x00: original vtable pointer (lbl_804B56E0)
    void* resource;               // 0x04: open resource, released by the destructor
    unsigned char failureLatched; // 0x08: 1 while the access is usable

    // 0x0C (vtable slot 3): deleting destructor; keeps the object alive when flag <= 0
    void* destroy(int flag);

    int GetBuffer();
    int GetStorageFullFlags();
    int GetTransferredBytes();
    int GetResource();
    int CreateResource(void* openOptions, void* byteCount);
    int GetFlag();
    int SetFailureFlag(int v);
    int QueryResourceStatusSlot9();
    int QueryResourcePredicate11();
    static int CreateDefaultResource(void* p);
    int QueryResourcePredicate8();
    static int HasFailure(void* p);
};

#endif // CLASSES_CFILEIOACCESS_H
