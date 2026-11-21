#ifndef MSL_LIST_H
#define MSL_LIST_H

// std::list as MSL C++ lays it out in the game: three words {size, end.prev, end.next}. `end` is the
// sentinel node of the circular doubly linked list; every node is {prev, next, value}. The allocator takes
// no space. Re-derived from the game's code, not copied from MSL.
//
// Destruction: four levels of classes, each checking `this`, so an inlined ~list() shows four stacked
// null checks around erase(begin(), end()): unlink the whole run from the sentinel in one step, then
// free the nodes front to back, decrementing the size per node.
// Only clear, the destructor and read-only members are provided here; insertion is still spelled out
// in the units.
#include "MSL/app_allocator.h"

namespace std {
template <class T> class allocator;

template <class T, class Allocator> class __list_deleter {
public:
    struct node;
    struct node_base {
        node* prev_;
        node* next_;
    };
    struct node : node_base {
        T value_;
    };
    class iterator {
    public:
        iterator(node* n) : node_(n) {}
        T& operator*() const { return node_->value_; }
        iterator& operator++() { node_ = node_->next_; return *this; }
        bool operator==(const iterator& x) const { return node_ == x.node_; }
        bool operator!=(const iterator& x) const { return node_ != x.node_; }
        node* node_;
    };

    ~__list_deleter() { erase(begin(), end()); }

    iterator begin() { return iterator(end_.next_); }
    iterator end() { return iterator((node*)&end_); }
    unsigned long size() const { return size_; }
    bool empty() const { return size_ == 0; }
    void clear() { erase(begin(), end()); }

    // Remove [first, last): unlink the run in one step, then free its nodes.
    void erase(iterator first, iterator last) {
        node* f = first.node_;
        node* l = last.node_;
        if (f != l) {
            node* back = l->prev_;
            f->prev_->next_ = back->next_;
            back->next_->prev_ = f->prev_;
            while (f != l) {
                node* n = f;
                f = f->next_;
                Allocator().deallocate((T*)n, 1);  // nodes go back to the game heap
                --size_;
            }
        }
    }

    unsigned long size_;
    node_base end_;
};

template <class T, class Allocator, bool B> class __list_imp : public __list_deleter<T, Allocator> {
public:
    ~__list_imp() {}
};

template <class T, class Allocator> class __list_imp<T, Allocator, true> : public __list_imp<T, Allocator, false> {
public:
    ~__list_imp() {}
};

template <class T, class Allocator = allocator<T> > class list : public __list_imp<T, Allocator, true> {
public:
    ~list() {}
};

// The compiler's deleting destructor of a list, for the out-of-line copies the game keeps under plain
// names (List_Dtor(self, deleting)). The list level's own null test is the `if (self)` that also guards
// the delete; only the base levels below it are inlined, each with its own test.
template <class T, class Allocator>
inline list<T, Allocator>* __list_deleting_dtor(list<T, Allocator>* self, int deleting) {
    if (self) {
        self->__list_imp<T, Allocator, true>::~__list_imp();
        if (deleting > 0) operator delete(self);
    }
    return self;
}
}

#endif
