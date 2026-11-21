// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// The original vtable (__vt__10CShop2Main) and typeinfo stay data symbols.
#ifndef CLASSES_CSHOP2MAIN_H
#define CLASSES_CSHOP2MAIN_H

// setState()'s argument: its second word selects the new state (1 or 2).
struct CShop2MainEvent {
    char unk00[4];
    int state;
};

class CShop2Main {
public:
    void* vtbl;        // 0x00: primary vtable (__vt__10CShop2Main)
    char unk04[0x14];  // 0x04
    void* vtbl2;       // 0x18: secondary vtable (__vt__10CShop2Main + 0x3c)
    char unk1C[4];     // 0x1C
    int state;         // 0x20: 1 or 2 after setState()
    void* shop;        // 0x24: object built by create(), freed by release()/destroy()

    // slot 0: CShop2Main_GetRTTI (not in this unit)
    // slot 1: destroy__10CShop2MainFi  void* destroy(int flag);
    // slot 2: vfunc_2__10CShop2MainFv  void vfunc_2();
    // slot 3: create__10CShop2MainFv  void create();
    // slot 4: vfunc_4__10CShop2MainFv  void vfunc_4();
    // slot 6: update__10CShop2MainFv  void update();
    // slot 7: draw__10CShop2MainFv  void release();
    // slot 8: vfunc_8__10CShop2MainFv  void vfunc_8();
    // slot 9: setState__10CShop2MainFP15CShop2MainEvent  void setState(CShop2MainEvent* arg);

    void* destroy(int flag);
    void vfunc_2();
    void create();
    void vfunc_4();
    void update();
    void release();
    void vfunc_8();
    void setState(CShop2MainEvent* arg);
};

#endif  // CLASSES_CSHOP2MAIN_H
