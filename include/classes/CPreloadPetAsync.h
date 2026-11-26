// CPreloadPetAsync: the loader for one pet's model and shadow.
//
// It owns the queued child preloaders, tracks whether the pet is still loading,
// and is initialized from the selected item's model path.
#ifndef CLASSES_CPRELOADPETASYNC_H
#define CLASSES_CPRELOADPETASYNC_H

#include "game/types.h"

class CPreloadPetAsync {
public:
    // The child preloaders, in request order.
    struct RequestList {
        void* data;            // 0x04
        unsigned int count;    // 0x08
        unsigned int capacity; // 0x0C
    };

    // Allocate a loader and initialize it for the given item, destroying it if
    // initialization fails.
    static CPreloadPetAsync* Create(void* itemId, int includeShadow);

    int ClearAndRestart(void* itemId, int includeShadow);
    int IsReady();
    int UpdateAndMarkReady();
    int QueueModelAndShadowFromItemId(unsigned itemId, int includeShadow);

    ~CPreloadPetAsync();

    void* vtbl;                  // 0x00
    RequestList preloadRequests; // 0x04
    int loadState;               // 0x10
};

#define CPRELOADPETASYNC_OFF(f, o) \
    typedef char CPreloadPetAsync_off_##f[((int)&((CPreloadPetAsync*)0)->f == (o)) ? 1 : -1]
CPRELOADPETASYNC_OFF(preloadRequests, 0x04);
CPRELOADPETASYNC_OFF(loadState, 0x10);
#undef CPRELOADPETASYNC_OFF
typedef char CPreloadPetAsync_size[(sizeof(CPreloadPetAsync) == 0x14) ? 1 : -1];

#endif // CLASSES_CPRELOADPETASYNC_H
