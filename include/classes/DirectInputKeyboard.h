// The original vtable (__vt__19DirectInputKeyboard) and typeinfo stay data symbols.
#ifndef CLASSES_DIRECTINPUTKEYBOARD_H
#define CLASSES_DIRECTINPUTKEYBOARD_H

#include "classes/WProc.h"

// A keyboard device. Its WProc part (vtable __vt__5WProc) sits at 0x0c, the three tracked key
// states after it.
class DirectInputKeyboard {
public:
    void* vtbl;                  // 0x00: original vtable pointer (__vt__19DirectInputKeyboard)
    char inputBaseStateBytes[8]; // 0x04
    WProc proc;                  // 0x0C: WProc part (vtable at 0x0c, registry at 0x10)
    signed char key32;           // 0x14: nonzero while key code 32 is down
    signed char key5;            // 0x15: nonzero while key code 5 is down
    signed char key4;            // 0x16: nonzero while key code 4 is down

    // slot 0: destroy__19DirectInputKeyboardFi  DirectInputKeyboard* destroy(int flag);
    // slot 3: DirectInputKeyboard_GetWProc (not in this unit)
    // slot 4: vfunc_4__19DirectInputKeyboardFv  int vfunc_4();
    // slot 7: poll__19DirectInputKeyboardFv  void poll();
    // slot 8: isKeyDown__19DirectInputKeyboardFii  int isKeyDown(int kind, int code);
    // slot 15: WInputDev_MakeClone (not in this unit)

    DirectInputKeyboard* destroy(int flag);
    int vfunc_4();
    void poll();
    int isKeyDown(int kind, int code);
};

#endif // CLASSES_DIRECTINPUTKEYBOARD_H
