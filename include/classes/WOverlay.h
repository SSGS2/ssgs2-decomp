// WOverlay: partial layout for WOverlay_SetCoordMode at 0x8004F8B4.
// Plain data view; original virtual dispatch remains in the game object.
#ifndef CLASSES_WOVERLAY_H
#define CLASSES_WOVERLAY_H

extern "C" {
void BaseObject_ctor(void*);
}

struct WResource {
    void* vtable;
    char baseResourceStorage[12];
    // Initialize the inherited resource prefix through the common base constructor.
    WResource() {
        BaseObject_ctor(this);
    }
    ~WResource();
};

class WOverlay : public WResource {
public:
    WOverlay();
    int initialState; // 0x10: WOverlay::WOverlay resets it; later state semantics unresolved
    char pad_014[0x10];
    unsigned char initialFlag; // 0x24: WOverlay::WOverlay clears it after virtual resource setup
    char pad_025[3];
    int coordMode;        // 0x28: WOverlay_SetCoordMode stores its integer argument; meaning unresolved
    int initialTailState; // 0x2C: WOverlay::WOverlay resets it; later state semantics unresolved
};

#define OFF(f, o) typedef char WOverlay_off_##f[((int)&((WOverlay*)0)->f == (o)) ? 1 : -1]
OFF(initialState, 0x10);
OFF(initialFlag, 0x24);
OFF(coordMode, 0x28);
OFF(initialTailState, 0x2C);
#undef OFF
#define AsWOverlay(p) ((WOverlay*)(p))

#endif
