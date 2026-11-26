#ifndef CLASSES_GTL_ICALLBACK_H
#define CLASSES_GTL_ICALLBACK_H

namespace gtl {

// Base interface of the file-I/O callbacks. Its vtable provides the deleting
// destructor at dispatch slot 2.
class ICallBack {
public:
    // Destroy the callback through its deleting destructor; null is a no-op.
    void Destroy();
};

} // namespace gtl

#endif // CLASSES_GTL_ICALLBACK_H
