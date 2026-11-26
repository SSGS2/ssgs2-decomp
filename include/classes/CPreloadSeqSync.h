// CPreloadSeqSync: a sequence preloader that finishes before it is queried.
//
// Create builds one and runs its script to completion; the object then reports
// itself loaded and complete without further updates.
#ifndef CLASSES_CPRELOADSEQSYNC_H
#define CLASSES_CPRELOADSEQSYNC_H

#include "game/types.h"
#include "classes/CPreloadSeq.h"

class CPreloadSeqSync : public CPreloadSeq {
public:
    static void* Create(void* arg);

    int IsLoadDone();
    int IsComplete();

    int Init(void* name);
    void* CreateFileSync(void* arg);
    void* CreateSprSync(void* arg);

    ~CPreloadSeqSync();
};

typedef char CPreloadSeqSync_size[(sizeof(CPreloadSeqSync) == 0x14) ? 1 : -1];

#endif // CLASSES_CPRELOADSEQSYNC_H
