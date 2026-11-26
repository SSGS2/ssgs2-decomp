// The original vtable (__vt__9CTaskMain) and typeinfo stay data symbols.
#ifndef CLASSES_CTASKMAIN_H
#define CLASSES_CTASKMAIN_H

class CTaskMain {
public:
    void* vtbl; // 0x00: primary vtable (__vt__9CTaskMain)

    // slot 1: destroy__9CTaskMainFi  void* destroy(int flag);
    // slot 6: CTaskMain_OnProcess (not in this unit)

    void* destroy(int flag);
    void DestroyThunk18(int flag); // entry for the base at +0x18

    static void OnReload();
    void vfunc_16();
    static void HandleMsg();
    static void OnDisplay();
    static void OnProcess();
    static void OnDestroy();
    static void OnLoad();
    static void OnPreLoadInit();
    static void* GetRTTI();
};

#endif // CLASSES_CTASKMAIN_H
