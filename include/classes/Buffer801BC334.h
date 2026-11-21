// Buffer801BC334: partial layout for methods in the address-named translation unit.
// Field comments identify the function providing layout evidence.
#ifndef CLASSES_BUFFER801BC334_H
#define CLASSES_BUFFER801BC334_H

class Buffer801BC334 {
public:
    char pad_000[0x4];
    void* data; // 0x4: Buffer_dtor
    short unk_8; // 0x8: Buffer_dtor
    unsigned char unk_A; // 0xA: Buffer_dtor
};

#define OFF(f, o) typedef char Buffer801BC334_off_##f[((int)&((Buffer801BC334*)0)->f == (o)) ? 1 : -1]
OFF(data, 0x4);
OFF(unk_8, 0x8);
OFF(unk_A, 0xA);
#undef OFF

#endif
