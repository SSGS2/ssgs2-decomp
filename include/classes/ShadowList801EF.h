// ShadowList801EF: layout view from CShadowManager_Ctor / dtor_801EF41C.
// No constructors, inheritance, virtual declarations, or emitted code.
#ifndef CLASSES_SHADOWLIST801EF_H
#define CLASSES_SHADOWLIST801EF_H

struct ShadowList801EF {
    unsigned int count; // 0x0: CShadowManager_Ctor / dtor_801EF41C decrement on node release
    void* sentinelNext; // 0x4: CShadowManager_Ctor list sentinel first link
    void* sentinelPrevious; // 0x8: CShadowManager_Ctor begins reverse traversal here
};

#define OFF(f, o) typedef char ShadowList801EF_off_##f[((int)&((ShadowList801EF*)0)->f == (o)) ? 1 : -1]
OFF(count, 0x0);
OFF(sentinelNext, 0x4);
OFF(sentinelPrevious, 0x8);
#undef OFF
#define AsShadowList801EF(p) ((ShadowList801EF*)(p))

#endif

