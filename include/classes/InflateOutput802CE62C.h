// InflateOutput802CE62C: layout view from Zlib_InflateFlushWindow (0x802CE62C); provisional type name.
// No constructors, inheritance, virtual declarations, or emitted code.
#ifndef CLASSES_INFLATEOUTPUT802CE62C_H
#define CLASSES_INFLATEOUTPUT802CE62C_H

struct InflateOutput802CE62C {
    char pad_000[0xc];
    unsigned char* nextOutput; // 0xc: Zlib_InflateFlushWindow copies to and advances pointer
    unsigned int availableOutput; // 0x10: Zlib_InflateFlushWindow caps copy size and subtracts
    unsigned int totalOutput; // 0x14: Zlib_InflateFlushWindow accumulates copied bytes
    char pad_018[0x18];
    void* checksum; // 0x30: Zlib_InflateFlushWindow stores callback result
};

#define OFF(f, o) typedef char InflateOutput802CE62C_off_##f[((int)&((InflateOutput802CE62C*)0)->f == (o)) ? 1 : -1]
OFF(nextOutput, 0xc);
OFF(availableOutput, 0x10);
OFF(totalOutput, 0x14);
OFF(checksum, 0x30);
#undef OFF
#define AsInflateOutput802CE62C(p) ((InflateOutput802CE62C*)(p))

#endif

