// Buffer801BCB94: partial layout for methods in the address-named translation unit.
// Field comments identify the function providing layout evidence.
#ifndef CLASSES_BUFFER801BCB94_H
#define CLASSES_BUFFER801BCB94_H

class Buffer801BCB94 {
public:
    char pad_000[0x30];
    unsigned int storageFlags; // 0x30: CourseNamedPoint_dtor
    char pad_034[0x4];
    void* ownedStorage; // 0x38: CourseNamedPoint_dtor
};

#define OFF(f, o) typedef char Buffer801BCB94_off_##f[((int)&((Buffer801BCB94*)0)->f == (o)) ? 1 : -1]
OFF(storageFlags, 0x30);
OFF(ownedStorage, 0x38);
#undef OFF

#endif
