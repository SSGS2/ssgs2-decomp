// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CASYNCEXHIBITIONAVATAR_H
#define CLASSES_CASYNCEXHIBITIONAVATAR_H

#include "game/types.h"

class CAsyncExhibitionAvatar {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x18];                 // 0x4
    void* resourcePreloader;                       // 0x1c: 2 uses in CAsyncExhibitionAvatar_UpdateDeferredRequests
    char pad_020[0x104];                // 0x20
    char avatarDescriptor[0x68];        // 0x124: CAsyncExhibitionAvatar_RequestModelUpdate copies a 0x68-byte descriptor; vfunc_7 passes it to CPreloadAvatarAsync_Create
    int pendingUpdateFlags;                        // 0x18c: 2 uses in CAsyncExhibitionAvatar_UpdateDeferredRequests
    unsigned char avatarLoadPending;              // 0x190: 3 uses in CAsyncExhibitionAvatar_UpdateDeferredRequests, CAsyncExhibitionAvatar_GetDeferredInstallAttemptPending
    unsigned char modelUpdatePending;              // 0x191: 2 uses in CAsyncExhibitionAvatar_UpdateDeferredRequests
    char pad_192[0x2];                  // 0x192
    void* preloadAvatar;                      // 0x194: 5 uses in CAsyncExhibitionAvatar_UpdateDeferredRequests, CAsyncExhibitionAvatar_CreateAvatarPreloader, CAsyncExhibitionAvatar_RestartAvatarPreloader

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x24, 0xec, 0x124
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CASYNCEXHIBITIONAVATAR_OFF(f, o) typedef char CAsyncExhibitionAvatar_off_##f[((int)&((CAsyncExhibitionAvatar*)0)->f == (o)) ? 1 : -1]
CASYNCEXHIBITIONAVATAR_OFF(resourcePreloader, 0x1c);
CASYNCEXHIBITIONAVATAR_OFF(avatarDescriptor, 0x124);
CASYNCEXHIBITIONAVATAR_OFF(pendingUpdateFlags, 0x18c);
CASYNCEXHIBITIONAVATAR_OFF(avatarLoadPending, 0x190);
CASYNCEXHIBITIONAVATAR_OFF(modelUpdatePending, 0x191);
CASYNCEXHIBITIONAVATAR_OFF(preloadAvatar, 0x194);
#undef CASYNCEXHIBITIONAVATAR_OFF

#endif  // CLASSES_CASYNCEXHIBITIONAVATAR_H
