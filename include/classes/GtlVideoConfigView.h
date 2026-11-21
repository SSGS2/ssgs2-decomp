// GtlVideoConfigView: partial layout for methods in the address-named translation unit.
// Field comments identify the function providing layout evidence.
#ifndef CLASSES_VIDEOCONFIG804326B0_H
#define CLASSES_VIDEOCONFIG804326B0_H

class GtlVideoConfigView {
public:
    unsigned int width; // 0x0: VideoConfig_InitFromTvFormat
    unsigned int height; // 0x4: VideoConfig_InitFromTvFormat
    unsigned int interlaced; // 0x8: VideoConfig_InitFromTvFormat
    unsigned int unk_C; // 0xC: VideoConfig_InitFromTvFormat
    unsigned int unk_10; // 0x10: VideoConfig_InitFromTvFormat
    float refreshRateHz; // 0x14: VideoConfig_InitFromTvFormat
    unsigned int tvMode; // 0x18: VideoConfig_InitFromTvFormat
};

#define OFF(f, o) typedef char VideoConfig804326B0_off_##f[((int)&((GtlVideoConfigView*)0)->f == (o)) ? 1 : -1]
OFF(width, 0x0);
OFF(height, 0x4);
OFF(interlaced, 0x8);
OFF(unk_C, 0xC);
OFF(unk_10, 0x10);
OFF(refreshRateHz, 0x14);
OFF(tvMode, 0x18);
#undef OFF

#endif
