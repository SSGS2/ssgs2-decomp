// MWStreamBuffer: offset-preserving view; no constructors, bases, or virtual methods.
// Evidence: MWStreamBuffer_Read / MWStreamBuffer_Write (0x800B05A8 / 0x800B070C) copy from the get area or into the put area and advance current; MWStringBuf_Overflow grows backing storage and updates highWater.
#ifndef CLASSES_MWSTREAMBUFFER_H
#define CLASSES_MWSTREAMBUFFER_H

struct MWStreamBuffer {
    void* vtbl; // 0x0
    unsigned int getBegin; // 0x4
    unsigned int getCurrent; // 0x8
    unsigned int getEnd; // 0xC
    unsigned int putBegin; // 0x10
    unsigned int putCurrent; // 0x14
    unsigned int putEnd; // 0x18
    char pad_01C[0x8];
    unsigned char mode; // 0x24
    char pad_025[0x3];
    unsigned int highWater; // 0x28
};

#define OFF(f, o) typedef char MWStreamBuffer_off_##f[((int)&((MWStreamBuffer*)0)->f == (o)) ? 1 : -1]
OFF(vtbl, 0x0);
OFF(getBegin, 0x4);
OFF(getCurrent, 0x8);
OFF(getEnd, 0xC);
OFF(putBegin, 0x10);
OFF(putCurrent, 0x14);
OFF(putEnd, 0x18);
OFF(mode, 0x24);
OFF(highWater, 0x28);
#undef OFF
typedef char MWStreamBuffer_size[(sizeof(MWStreamBuffer) == 0x2C) ? 1 : -1];
#define AsMWStreamBuffer(p) ((MWStreamBuffer*)(p))

#endif // CLASSES_MWSTREAMBUFFER_H
