// Output record used by WView_ProjectWorldPoint and WView_ProjectViewPoint
// in wview_camera_projection.cpp. The clip comparisons use the coordinates at 0x28..0x30.
#ifndef CLASSES_WVIEWPROJECTEDPOINT_H
#define CLASSES_WVIEWPROJECTEDPOINT_H

struct WViewProjectedPoint {
    float screenX; // 0x00: WView_ProjectWorldPoint stores viewport-transformed coordinates
    float screenY; // 0x04
    float screenZ; // 0x08
    float rhw; // 0x0c: WView_ProjectWorldPoint stores 1.0f
    char pad_010[0x18];
    float projectedX; // 0x28: perspective-divided x, compared against -1 and 1
    float projectedY; // 0x2c: perspective-divided y, compared against -1 and 1
    float projectedZ; // 0x30: perspective-divided z, compared against 0 and 1
    unsigned char clipFlags; // 0x34: x=4/8, y=0x20/0x10, z=1/2
};

#define OFF(f, o) typedef char WViewProjectedPoint_off_##f[((int)&((WViewProjectedPoint*)0)->f == (o)) ? 1 : -1]
OFF(screenX, 0x00);
OFF(screenY, 0x04);
OFF(screenZ, 0x08);
OFF(rhw, 0x0c);
OFF(projectedX, 0x28);
OFF(projectedY, 0x2c);
OFF(projectedZ, 0x30);
OFF(clipFlags, 0x34);
#undef OFF

#endif
