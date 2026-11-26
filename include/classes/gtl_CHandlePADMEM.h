#ifndef CLASSES_GTL_CHANDLEPADMEM_H
#define CLASSES_GTL_CHANDLEPADMEM_H

namespace gtl {

// A PAD memory card file handle. Destroy is the deleting destructor (vtable slot 0).
class CHandlePADMEM {
public:
    // Free the handle storage when flags > 0; returns this, which may now be dangling.
    void* Destroy(int flags);
};

} // namespace gtl

#endif // CLASSES_GTL_CHANDLEPADMEM_H
