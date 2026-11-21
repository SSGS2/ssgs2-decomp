#ifndef MSL_DEQUE_H
#define MSL_DEQUE_H

#include "MSL/cdeque.h"

namespace std {
// MSL's segmented deque: 1 KiB blocks, held by a circular map of block pointers.
// This initial subset exposes layout and read-only bookkeeping. Growth and
// owning construction/destruction are deliberately not defined yet.
template <class T, class Allocator = allocator<T> > class deque {
public:
    deque();
    ~deque();
    enum { __block_size = sizeof(T) < 1024 ? 1024 / sizeof(T) : 1 };
    typedef typename Allocator::template rebind<T*>::other map_allocator;
    typedef cdeque<T*, map_allocator> block_map;
    unsigned long size() const { return size_; }
    bool empty() const { return size_ == 0; }
    unsigned long __begin_index() const { return first_; }
    long __capacity() const {
        long count = (long)map_.size();
        if (count == 0) return 0;
        return (count - 1) * __block_size;
    }
    block_map& __map() { return map_; }
    unsigned long first_;
    unsigned long size_;
    block_map map_;
};
}

#endif
