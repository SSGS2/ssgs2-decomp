#ifndef CLASSES_CPRELOADSEQ_H
#define CLASSES_CPRELOADSEQ_H

#include "game/types.h"

class CPreloadSeq {
public:
    void* vtbl;         // 0x0: original vtable pointer
    char pad_004[0xc];  // 0x4
    void* scriptStream; // 0x10

    CPreloadSeq* Init();
    void* Destroy(int shouldDelete);
    int ProcessScriptEntries();
    int OpenStreamFromPath(void* arg1);
};

#define CPRELOADSEQ_OFF(f, o) typedef char CPreloadSeq_off_##f[((int)&((CPreloadSeq*)0)->f == (o)) ? 1 : -1]
CPRELOADSEQ_OFF(scriptStream, 0x10);
#undef CPRELOADSEQ_OFF

#endif // CLASSES_CPRELOADSEQ_H
