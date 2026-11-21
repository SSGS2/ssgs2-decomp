// IActor: layout view owned by task_registry_actor_and_debug_text.cpp.
// IActor_Create and __ct__18NativeBase80201D18Fv install __vt__6IActor.
#ifndef CLASSES_IACTOR_H
#define CLASSES_IACTOR_H
class IActor {
public:
    void* vtbl; // 0x00: IActor_Create / __ct__18NativeBase80201D18Fv
    unsigned int unk_4; // 0x04: constructor sets zero
    unsigned int displayPriority; // 0x08: constructor sets two
    unsigned int skipMask; // 0x0c: IActor_Skipped tests against *g_actorSkipMask
    void* dispatchObject; // 0x10: IActor_FindDispatchTargetByName lookup; IActor_DispatchMessage virtual dispatch
    unsigned int unk_14; // 0x14: constructor sets zero
};
#define OFF(f, o) typedef char IActor_off_##f[((int)&((IActor*)0)->f == (o)) ? 1 : -1]
OFF(vtbl, 0x00);
OFF(unk_4, 0x04);
OFF(displayPriority, 0x08);
OFF(skipMask, 0x0c);
OFF(dispatchObject, 0x10);
OFF(unk_14, 0x14);
#undef OFF
#endif
