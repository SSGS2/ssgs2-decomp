// GtlNoticeOwner: layout view owned by gtl_heap_allocators.cpp.
// Evidence: GtlNoticeOwner_AllocateWithOptionalLock; GtlNoticeOwner_InvokeFreeLocked. No virtual declarations or runtime objects.
#ifndef CLASSES_GTLNOTICEOWNER_H
#define CLASSES_GTLNOTICEOWNER_H

class GtlNoticeOwner {
public:
    int AllocateWithOptionalLock(unsigned int byteCount, int lockOptions);
    int InvokeFreeLocked(void* arg, int condition);

    char pad_0[0x8];
    void* object; // 0x8; GtlNoticeOwner_AllocateWithOptionalLock
    char pad_C[0x1C];
    int flags; // 0x28; GtlNoticeOwner_AllocateWithOptionalLock
    char mutex
        [0x18]; // 0x2C: GtlNoticeOwner_AllocateWithOptionalLock/GtlNoticeOwner_InvokeFreeLocked pass to OSLockMutex (OSLockMutex)
};

#define OFF(f, o) typedef char GtlNoticeOwner_off_##f[((int)&((GtlNoticeOwner*)0)->f == (o)) ? 1 : -1]
OFF(object, 0x8);
OFF(flags, 0x28);
OFF(mutex, 0x2C);
#undef OFF

#endif
