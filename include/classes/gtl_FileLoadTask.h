// gtl::FileLoadTask: the base type of the file-load tasks queued for the audio stream. The vtable comes from
// the original data, so no virtual functions are declared here.
#ifndef CLASSES_GTL_FILELOADTASK_H
#define CLASSES_GTL_FILELOADTASK_H

namespace gtl {

class FileLoadTask {
public:
    // Cancel the borrowed active stream when supported, choosing async cancellation if available.
    // No stream is freed or cleared here; both cancellation return values are ignored.
    static void CancelActiveStream();
    // Free task storage through C++ delete only for deleting > 0; return its old address.
    // No payload cleanup occurs here; the returned pointer may already be freed.
    FileLoadTask* Destroy(int deleting);
};

} // namespace gtl

#endif // CLASSES_GTL_FILELOADTASK_H
