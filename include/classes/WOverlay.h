// WOverlay: partial layout for WOverlay_SetCoordMode at 0x8004F8B4.
// Plain data view; original virtual dispatch remains in the game object.
#ifndef CLASSES_WOVERLAY_H
#define CLASSES_WOVERLAY_H

class WOverlay {
public:
    char pad_000[0x28];
    int coordMode; // 0x28: WOverlay_SetCoordMode stores its integer argument; meaning unresolved
};

#define OFF(f, o) typedef char WOverlay_off_##f[((int)&((WOverlay*)0)->f == (o)) ? 1 : -1]
OFF(coordMode, 0x28);
#undef OFF
#define AsWOverlay(p) ((WOverlay*)(p))

#endif
