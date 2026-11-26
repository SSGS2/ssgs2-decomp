// MWStreamState: offset-preserving view; no constructors, bases, or virtual methods.
// Evidence: Stream_WriteFixedWidthHex32 and Stream_WriteUnsigned64 compare width with formatted length, test flags for base/adjustment, and clear width; MWStream_FormatDoubleFixed uses precision for decimal places.
#ifndef CLASSES_MWSTREAMSTATE_H
#define CLASSES_MWSTREAMSTATE_H

struct MWStreamState {
    char pad_000[0x28];
    unsigned int precision; // 0x28
    unsigned int width;     // 0x2C
    unsigned short flags;   // 0x30
};

#define OFF(f, o) typedef char MWStreamState_off_##f[((int)&((MWStreamState*)0)->f == (o)) ? 1 : -1]
OFF(precision, 0x28);
OFF(width, 0x2C);
OFF(flags, 0x30);
#undef OFF
typedef char MWStreamState_size[(sizeof(MWStreamState) == 0x34) ? 1 : -1];
#define AsMWStreamState(p) ((MWStreamState*)(p))

#endif // CLASSES_MWSTREAMSTATE_H
