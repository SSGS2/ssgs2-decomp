// CSwapEndianFormat: a formatter that swaps byte order in place, using a list of
// registered format strings. It has no virtual functions; the vtable is the
// original game data, so vtbl is an explicit member.
#ifndef CLASSES_CSWAPENDIANFORMAT_H
#define CLASSES_CSWAPENDIANFORMAT_H

struct CSwapEndianFormat {
    void* vtbl;                  // 0x00
    void* formatStrings;         // 0x04  12-byte format strings, appended by RegisterFormat
    unsigned int formatCount;    // 0x08
    unsigned int formatCapacity; // 0x0C
    unsigned int context;        // 0x10  set by SetContext; 4 by default

    int RegisterFormat(const char* pattern, int outputSize, int iterations);
    int CopyAndSwap(void* output, void* input, int outputSize);
    int SetContext(unsigned int value);
    void* Destroy(int shouldDelete);

    CSwapEndianFormat();
};

#define OFF(f, o) typedef char CSwapEndianFormat_off_##f[((int)&((CSwapEndianFormat*)0)->f == (o)) ? 1 : -1]
OFF(vtbl, 0x0);
OFF(formatStrings, 0x4);
OFF(formatCount, 0x8);
OFF(formatCapacity, 0xC);
OFF(context, 0x10);
#undef OFF

#endif
