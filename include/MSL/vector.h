#ifndef MSL_VECTOR_H
#define MSL_VECTOR_H

// std::vector as MSL C++ lays it out in the game: three words {data, size, capacity}; the allocator takes
// no space. Re-derived from the game's code, not copied from MSL.
//
// Destruction: the class sits on a chain of bases (five levels for scalar elements, four otherwise).
// Every level's destructor checks `this` before running, so an inlined ~vector() shows that many stacked
// null checks, and the bottom level clears the size and frees the storage:
//     if (data) { size -= size; deallocate(data); }
// Only the destructor and the read-only members are provided here; growth (push_back, reserve, insert)
// is still spelled out in the units.
#include "MSL/app_allocator.h"

namespace std {
template <class T> class allocator;

// Element policy of a vector. value 1: elements have destructors that must run when the vector drops
// them (the game's strings, for example); 0 (the default): elements are just dropped.
// The game's vector destructors also compile the size reset in three forms: a load and `subf r0,r0,r0`
// (0, the default), two loads and a subtract (2), and a plain store of 0 (3). Forms 4 and 5 retain one
// volatile size read before storing zero; form 5 also makes the reset store volatile. Which MSL code path made
// each is not established (element type and optimisation pragmas were ruled out), so a unit picks the
// form by specialising this for its element type with value 2, 3, 4 or 5. (A separate trait template would
// renumber compiler-numbered local symbols in units that include this header.)
// destroy() runs one element's destructor; a specialisation may spell it out inline where the unit
// otherwise calls the destructor out of line.
template <class T> struct __vec_destroys_elements {
    enum { value = 0 };
    static void destroy(T* e) { e->~T(); }
};


template <class T, class Allocator> class __vec_deleter {
public:
    __vec_deleter() : data_(0), size_(0), capacity_(0) {}
    ~__vec_deleter() {
        if (data_ != 0) {
            if (__vec_destroys_elements<T>::value == 4) {
                // A by-value clear count with a required load; the reset itself is nonvolatile.
                unsigned long n = *(volatile unsigned long*)&size_;
                size_ = 0;
                (void)n;
            } else if (__vec_destroys_elements<T>::value == 5) {
                // Preserve the volatile count read/write present in this specialization's source.
                volatile int* count = (volatile int*)&size_;
                int n = *count;
                *count = n - n;
            } else if (__vec_destroys_elements<T>::value == 2) {
                // count passed by reference: the subtraction loads the size twice
                const unsigned long& n = size_;
                size_ -= n;
            } else if (__vec_destroys_elements<T>::value == 3) {
                // unsigned count by value: `size -= size` folds to storing 0
                unsigned long n = size_;
                size_ -= n;
            } else {
                erase_at_end(size_);
            }
            Allocator().deallocate(data_, capacity_);
        }
    }
    // Drop n elements from the end. Elements with destructors (see __vec_destroys_elements) are
    // destroyed back to front; the others are just dropped. The signed count is what the game compiled:
    // `size -= size` stays a load and a subtract instead of folding to zero.
    void erase_at_end(int n) {
        if (__vec_destroys_elements<T>::value) {
            T* e = data_ + size_;
            size_ -= n;
            while (n != 0) {
                --e;
                __vec_destroys_elements<T>::destroy(e);
                --n;
            }
        } else {
            size_ -= n;
        }
    }

    T* data_;
    unsigned long size_;
    unsigned long capacity_;
};

template <class T, class Allocator, bool TrivialCopy> class __vec_constructor : public __vec_deleter<T, Allocator> {
public:
    ~__vec_constructor() {}
};

template <class T, class Allocator> class __vec_constructor<T, Allocator, true>
    : public __vec_constructor<T, Allocator, false> {
public:
    ~__vec_constructor() {}
};

// Scalar element types (arithmetic and pointers) take the specialised constructor level, so their vectors
// are one level deeper: five stacked null checks instead of four.
template <class T> struct __vec_is_scalar { enum { value = 0 }; };
template <class T> struct __vec_is_scalar<T*> { enum { value = 1 }; };
template <> struct __vec_is_scalar<bool> { enum { value = 1 }; };
template <> struct __vec_is_scalar<char> { enum { value = 1 }; };
template <> struct __vec_is_scalar<signed char> { enum { value = 1 }; };
template <> struct __vec_is_scalar<unsigned char> { enum { value = 1 }; };
template <> struct __vec_is_scalar<short> { enum { value = 1 }; };
template <> struct __vec_is_scalar<unsigned short> { enum { value = 1 }; };
template <> struct __vec_is_scalar<int> { enum { value = 1 }; };
template <> struct __vec_is_scalar<unsigned int> { enum { value = 1 }; };
template <> struct __vec_is_scalar<long> { enum { value = 1 }; };
template <> struct __vec_is_scalar<unsigned long> { enum { value = 1 }; };
template <> struct __vec_is_scalar<float> { enum { value = 1 }; };
template <> struct __vec_is_scalar<double> { enum { value = 1 }; };

template <class T, class Allocator> class __vec_base
    : public __vec_constructor<T, Allocator, (bool)__vec_is_scalar<T>::value> {
public:
    ~__vec_base() {}
};

template <class T, class Allocator = allocator<T> > class vector : public __vec_base<T, Allocator> {
public:
    vector() {}
    ~vector() {}

    unsigned long size() const { return this->size_; }
    unsigned long capacity() const { return this->capacity_; }
    bool empty() const { return this->size_ == 0; }
    T* begin() { return this->data_; }
    T* end() { return this->data_ + this->size_; }
    T& operator[](unsigned long i) { return this->data_[i]; }
    const T& operator[](unsigned long i) const { return this->data_[i]; }
};

// The compiler's deleting destructor of a vector, for the out-of-line copies the game keeps under plain
// names (dtor_XXXXXXXX(self, deleting)). The vector level's own null test is the `if (self)` that also
// guards the delete; only the base levels below it are inlined, each with its own test.
template <class T, class Allocator>
inline vector<T, Allocator>* __vector_deleting_dtor(vector<T, Allocator>* self, int deleting) {
    if (self) {
        self->__vec_base<T, Allocator>::~__vec_base();
        if (deleting > 0) operator delete(self);
    }
    return self;
}
}

#endif
