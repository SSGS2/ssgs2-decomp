// Translation unit 0x80403E80-0x80403EF4 (2 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"

namespace gtl {
class FileLoadTask {};
class CSoundStreamSource {
public:
    class StrmCallback {};
};
}

extern "C" {
void __dl__FPv(void*);
void* GtlStreamTaskManager_GetInstance(gtl::CSoundStreamSource::StrmCallback*);
void AudioWorkQueue_CancelOwnerTasks(void*, void*);

// Free task storage through C++ delete only for deleting > 0; return its old address.
// No payload cleanup occurs here; the returned pointer may already be freed.
gtl::FileLoadTask* gtl_FileLoadTask_dtor(gtl::FileLoadTask* self, int deleting) {
    if (self) {
        if (deleting > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Cancel queued work whose owner key is ownerKey; self selects the shared task manager.
// ownerKey is an identity passed through unchanged, with no ownership transfer.
void gtl_CSoundStreamSource_StrmCallback_vfunc_3(gtl::CSoundStreamSource::StrmCallback* self, void* ownerKey) {
    AudioWorkQueue_CancelOwnerTasks(GtlStreamTaskManager_GetInstance(self), ownerKey);
}

} // extern "C"
