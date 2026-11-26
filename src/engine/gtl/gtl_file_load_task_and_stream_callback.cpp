// File load task destructor, and the sound stream callback that cancels the queued work of one owner.

#include "sdk_protos.h"
#include "classes/SoundStreamSource.h"
#include "classes/gtl_FileLoadTask.h"

extern "C" {
void __dl__FPv(void*);
void* GtlStreamTaskManager_GetInstance(gtl::CSoundStreamSource::StrmCallback*);
void AudioWorkQueue_CancelOwnerTasks(void*, void*);

} // extern "C"

gtl::FileLoadTask* gtl::FileLoadTask::Destroy(int deleting) {
    if (this) {
        if (deleting > 0) {
            __dl__FPv(this);
        }
    }
    return this;
}

// Cancel queued work whose owner key is ownerKey; this callback selects the shared task manager.
// ownerKey is an identity passed through unchanged, with no ownership transfer.
void gtl::CSoundStreamSource::StrmCallback::CancelOwnerTasks(void* ownerKey) {
    AudioWorkQueue_CancelOwnerTasks(GtlStreamTaskManager_GetInstance(this), ownerKey);
}
