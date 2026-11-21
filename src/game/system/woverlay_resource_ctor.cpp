// WOverlay constructor view: resource base [0x00,0x10), reset state at +0x10,
// flag at +0x24, and tail state at +0x2C. Other bytes remain owned by base/virtual setup.
extern "C" {
void BaseObject_ctor(void*);
extern char __vt__8WOverlay[];
}

struct WResource {
    void* vtable;
    char baseResourceStorage[12];
    // Initialize the inherited resource prefix through the common base constructor.
    WResource() { BaseObject_ctor(this); }
    ~WResource();
};

struct VirtualView {
    virtual void unused();
    virtual void init(int);
};

struct WOverlay : WResource {
    int initialState;                 // Reset here; later state semantics unresolved.
    char overlayStorage[16];
    unsigned char initialFlag;        // Cleared after virtual resource setup.
    char overlayTailStorage[7];
    int initialTailState;             // Reset here; later state semantics unresolved.

    WOverlay();
};

// Construct the resource base, request overlay resource flags, and clear initial state.
// The base is constructed implicitly before this body. The meaning of individual
// 0x2200 bits and the later uses of the reset fields are unresolved in this view.
WOverlay::WOverlay() {
    enum { kOverlayResourceFlags = 0x2200 };
    initialState = 0;
    // Publish derived dispatch before requesting resource setup through virtual slot 1.
    *(void* volatile*)this = __vt__8WOverlay;
    reinterpret_cast<VirtualView*>(this)->init(kOverlayResourceFlags);
    initialTailState = 0;
    initialFlag = 0;
}
