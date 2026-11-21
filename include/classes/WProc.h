// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// The original vtable (__vt__5WProc) and typeinfo stay data symbols.
#ifndef CLASSES_WPROC_H
#define CLASSES_WPROC_H

// One registration in a WProcRegistry: the key it was added with and the registered object.
struct WProcEntry {
    int key;       // 0x00
    void* object;  // 0x04
};

// The list a WProc registers itself in; destroy() removes the object from it.
struct WProcRegistry {
    WProcEntry entries[16];  // 0x00
    int count;               // 0x80: number of live entries
};

class WProc {
public:
    void* vtbl;               // 0x00: original vtable pointer (__vt__5WProc)
    WProcRegistry* registry;  // 0x04: registry this object was added to, 0 when not registered

    // slot 0 (destructor): destroy__5WProcFi  WProc* destroy(int flag);

    WProc* destroy(int flag);
};

#endif  // CLASSES_WPROC_H
