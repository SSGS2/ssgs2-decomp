#ifndef MSL_CDEQUE_H
#define MSL_CDEQUE_H

#include "MSL/app_allocator.h"

namespace std {
template <class T> class allocator;

// MSL's circular storage: capacity includes its spare slot. Iterator origin
// distinguishes wrapped ranges; increment only needs the allocation boundaries.
// Construction, destruction and insertion remain to be reconstructed.
template <class T, class Allocator = allocator<T> > class cdeque {
public:
    cdeque();
    ~cdeque();
    struct iterator {
        T* current_;
        T* begin_;
        T* origin_;
        T* end_;
        iterator& operator++() {
            T* next = current_ + 1;
            current_ = next;
            if (next == end_) current_ = begin_;
            return *this;
        }
    };
    unsigned long size() const { return size_; }
    unsigned long capacity() const { return capacity_ - 1; }
    T* data_;
    unsigned long first_;
    unsigned long size_;
    unsigned long capacity_;
};
}

#endif
