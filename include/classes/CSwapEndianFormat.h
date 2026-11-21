// CSwapEndianFormat: non-virtual view; ctor 0x80068998, SetContext 0x800698CC,
// CopyAndSwap 0x800693F0 and format insertion 0x80068B98 in endian_format_and_binary_table.cpp.
#ifndef CLASSES_CSWAPENDIANFORMAT_H
#define CLASSES_CSWAPENDIANFORMAT_H

struct CSwapEndianFormat {
    void* vtbl; // CSwapEndianFormat_ctor stores __vt__17CSwapEndianFormat.
    void* formatStrings; // 0x4: CSwapEndianFormat_RegisterFormat appends 12-byte strings; CopyAndSwap iterates them.
    unsigned int formatCount; // 0x8: CSwapEndianFormat_RegisterFormat increments after insertion.
    unsigned int formatCapacity; // 0xC: CSwapEndianFormat_RegisterFormat compares count and grows storage.
    unsigned int context; // 0x10: SetContext; ctor defaults to 4; forwarded to format descriptors.
};

#define OFF(f, o) typedef char CSwapEndianFormat_off_##f[((int)&((CSwapEndianFormat*)0)->f == (o)) ? 1 : -1]
OFF(vtbl, 0x0);
OFF(formatStrings, 0x4);
OFF(formatCount, 0x8);
OFF(formatCapacity, 0xC);
OFF(context, 0x10);
#undef OFF
#define AsCSwapEndianFormat(p) ((CSwapEndianFormat*)(p))

#endif
