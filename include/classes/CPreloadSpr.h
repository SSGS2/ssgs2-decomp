#ifndef CLASSES_CPRELOADSPR_H
#define CLASSES_CPRELOADSPR_H

#include "game/types.h"
#include "classes/CPreloadParent.h"

// Only the vtable, the base block and the stream handle are declared here; the other members reach the
// remaining object data through file-local layouts.
class CPreloadSpr : public CPreloadParent {
public:
    int stream; // 0x10: owned spray script stream, 0 when none

    int ProcessScriptEntries();
    int ParseResourceDirective(void* inputArg, unsigned char* loadedResource);
    int OpenNamedSourceFile(void* arg);

    ~CPreloadSpr();
    CPreloadSpr();
};

#define CPRELOADSPR_OFF(f, o) typedef char CPreloadSpr_off_##f[((int)&((CPreloadSpr*)0)->f == (o)) ? 1 : -1]
CPRELOADSPR_OFF(stream, 0x10);
#undef CPRELOADSPR_OFF

#endif // CLASSES_CPRELOADSPR_H
