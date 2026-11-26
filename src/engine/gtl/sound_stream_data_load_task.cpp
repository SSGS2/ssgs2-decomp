// The stream-data load task of CSoundStreamSource and its Destroy function, which frees the
// task's storage when asked to delete it.

#include "classes/SoundStreamSource.h"

extern "C" {

void gtl_Audio_Free(void*);

} // extern "C"

// Destroy the stream-data load task, optionally releasing its object storage.
// No members are cleaned up here. Return the original address even after freeing it;
// that return value does not grant ownership or make freed storage usable.
void* gtl::CSoundStreamSource::StrmDataLoadTask::Destroy(int deleteFlag) {
    if (this != 0 && deleteFlag > 0) {
        gtl_Audio_Free(this);
    }
    return this;
}
