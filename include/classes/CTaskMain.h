// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// The original vtable (__vt__9CTaskMain) and typeinfo stay data symbols.
#ifndef CLASSES_CTASKMAIN_H
#define CLASSES_CTASKMAIN_H

class CTaskMain {
public:
    void* vtbl;  // 0x00: primary vtable (__vt__9CTaskMain)

    // slot 1: destroy__9CTaskMainFi  void* destroy(int flag);
    // slot 6: CTaskMain_OnProcess (not in this unit)

    void* destroy(int flag);
};

#endif  // CLASSES_CTASKMAIN_H
