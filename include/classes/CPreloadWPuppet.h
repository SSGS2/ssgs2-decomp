#ifndef CLASSES_CPRELOADWPUPPET_H
#define CLASSES_CPRELOADWPUPPET_H

#include "game/types.h"
#include "classes/CPreloadParent.h"

class CPreloadWPuppet : public CPreloadParent {
public:
    void* file; // 0x10

    int LoadExternalFileReferences(void* resource, void* fileSize);
    int LoadMeshChunk(void* resource, void* fileSize);
    int QueueMeshMaterialTextures(void* resource);
    int OpenSourceFile(void* arg);
    int QueueExternalResource(const char* path);
    int LoadFileResources();
    int PollFileReady();

    int QueryFileCompletion();

    ~CPreloadWPuppet();
    CPreloadWPuppet();
};

#define CPRELOADWPUPPET_OFF(f, o) typedef char CPreloadWPuppet_off_##f[((int)&((CPreloadWPuppet*)0)->f == (o)) ? 1 : -1]
CPRELOADWPUPPET_OFF(file, 0x10);
#undef CPRELOADWPUPPET_OFF

#endif // CLASSES_CPRELOADWPUPPET_H
