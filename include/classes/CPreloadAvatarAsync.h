// CPreloadAvatarAsync: the loader for one avatar's resources.
//
// It owns the child preloaders for the character's model and textures and
// reports when they have all finished, and it keeps the optional parts loader
// in a field of its own.
#ifndef CLASSES_CPRELOADAVATARASYNC_H
#define CLASSES_CPRELOADAVATARASYNC_H

#include "game/types.h"

namespace IFF_STRUCT {
struct sChar;
}

class CPreloadAvatarAsync {
public:
    // Allocate a loader for the given selection and queue its resources, or
    // return null if it cannot be initialized.
    static CPreloadAvatarAsync* Create(void* selection, int animationKind, int storyOverride, int partsContext);

    // Queue the avatar's own resources.
    int QueueModelFiles(const IFF_STRUCT::sChar* character);
    int QueueFaceTexture(const IFF_STRUCT::sChar* character);
    int QueueBaseTexture(const IFF_STRUCT::sChar* character);

    int InitializeDependencies(void* selection, void* animationKind, void* storyOverride, void* partsContext);
    static int QueueAnimationPuppet(void* loader, void* characterRecord, void* animationKind, void* storyOverride);
    int IsLoadComplete();
    static int UpdateAndLatchComplete(void* loader);
    int ReloadParts(void* selection, int flags);
    int QueueTextureAndDigitVariant(const char* path, int digit);
    int ClearAndRestart(void* selection, void* animationKind, void* storyOverride, void* partsContext);
    static int QueuePetPuppet(void* loader, void* characterRecord);

    ~CPreloadAvatarAsync();

    void* vtbl;                   // 0x00
    void** requests;              // 0x04  child preloaders
    unsigned int requestCount;    // 0x08
    unsigned int requestCapacity; // 0x0C
    int loadComplete;             // 0x10
    void* partsPreload;           // 0x14
};

#define CPRELOADAVATARASYNC_OFF(f, o) \
    typedef char CPreloadAvatarAsync_off_##f[((int)&((CPreloadAvatarAsync*)0)->f == (o)) ? 1 : -1]
CPRELOADAVATARASYNC_OFF(requests, 0x04);
CPRELOADAVATARASYNC_OFF(requestCount, 0x08);
CPRELOADAVATARASYNC_OFF(requestCapacity, 0x0C);
CPRELOADAVATARASYNC_OFF(loadComplete, 0x10);
CPRELOADAVATARASYNC_OFF(partsPreload, 0x14);
#undef CPRELOADAVATARASYNC_OFF

typedef char CPreloadAvatarAsync_size[(sizeof(CPreloadAvatarAsync) == 0x18) ? 1 : -1];

#endif // CLASSES_CPRELOADAVATARASYNC_H
