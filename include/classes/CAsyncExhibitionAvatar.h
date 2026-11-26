// An asynchronous character preview: the base preview plus an equipment
// descriptor and deferred model updates. The object is created before its model
// resources finish loading; pending changes are applied once the preloaders
// report ready.
#ifndef CLASSES_CASYNCEXHIBITIONAVATAR_H
#define CLASSES_CASYNCEXHIBITIONAVATAR_H

#include "classes/CAsyncExhibition.h"
#include "game/types.h"

class CAsyncExhibitionAvatar {
public:
    void* vtbl;              // 0x0: original vtable pointer
    char pad_004[0x18];      // 0x4
    void* resourcePreloader; // 0x1c
    char pad_020[0x104];     // 0x20
    char avatarDescriptor
        [0x68]; // 0x124: RequestModelUpdate copies a 0x68-byte descriptor; ConfigureAvatarPreview passes it to CPreloadAvatarAsync::Create
    int pendingUpdateFlags;           // 0x18c
    unsigned char avatarLoadPending;  // 0x190
    unsigned char modelUpdatePending; // 0x191
    char pad_192[0x2];                // 0x192
    void* preloadAvatar;              // 0x194

    // Allocate a preview for the given viewport, or return null if it cannot be
    // initialized.
    static CAsyncExhibitionAvatar* Create(void* viewport, int mode, float scale);

    int UpdateDeferredRequests(float delta);
    unsigned char GetDeferredInstallAttemptPending();
    int SetEquipment(void* source, void* value);
    static int CreateAvatarPreloader(void* preview);
    static int ConfigureAvatarPreview(void* preview);
    static int RestartAvatarPreloader(void* preview);
    static int RequestModelUpdate(void* preview, void* source, int flags);

    ~CAsyncExhibitionAvatar();
};

#define CASYNCEXHIBITIONAVATAR_OFF(f, o) \
    typedef char CAsyncExhibitionAvatar_off_##f[((int)&((CAsyncExhibitionAvatar*)0)->f == (o)) ? 1 : -1]
CASYNCEXHIBITIONAVATAR_OFF(resourcePreloader, 0x1c);
CASYNCEXHIBITIONAVATAR_OFF(avatarDescriptor, 0x124);
CASYNCEXHIBITIONAVATAR_OFF(pendingUpdateFlags, 0x18c);
CASYNCEXHIBITIONAVATAR_OFF(avatarLoadPending, 0x190);
CASYNCEXHIBITIONAVATAR_OFF(modelUpdatePending, 0x191);
CASYNCEXHIBITIONAVATAR_OFF(preloadAvatar, 0x194);
#undef CASYNCEXHIBITIONAVATAR_OFF
typedef char CAsyncExhibitionAvatar_size[(sizeof(CAsyncExhibitionAvatar) == 0x198) ? 1 : -1];

#endif // CLASSES_CASYNCEXHIBITIONAVATAR_H
