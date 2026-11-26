// Ring buffers for words and pointers: the word deque and pointer ring storage, their iterators,
// growth, insertion of word ranges, and the link-based ring used by the audio code.

enum WordRingLimits {
    kMaxWordRingSize = 0x3FFFFFFE, // Leave one unused slot so begin == end means empty.
    kDequeLengthErrorText = 0x4C,
    kLengthErrorTypeName = 0x15
};

// Reading key for this file. The "word ring" is one circular buffer of 32-bit words, laid out as
// {data, head (index of the first element), size, capacity}; one slot stays unused so size < capacity. The
// "page ring" / "pointer ring" / "GtlSampleDeque" helpers below all operate on that same layout (the names differ
// only because they were first found in different callers). An "iterator" is four words
// {current, storageBegin, logicalBegin, storageEnd}. The many near-identical 4-word structs (WordRing*,
// PageRing*, RingIterator*) are the same shape, kept separate only because merging them changes the generated code.
// Ring state: data/head/size/capacity. Iterators also retain the logical begin
// so comparisons and copies can distinguish the two spans of a wrapped ring.
extern "C" {

struct AppendableWordRing;
void GtlWordDeque_PushBack(AppendableWordRing*, unsigned int*);
void gtl_Free(void*);
void* gtl_Alloc(unsigned int);
void __dl__FPv(void*);
void PointerRing_InsertWordRange(void*, void*, void*, void*, unsigned char*);
extern "C" unsigned char s_gtlWordContainerMaxSizeInitialized;
extern "C" unsigned int s_gtlWordContainerMaxSize;
#include "sdk_protos.h"
#include "classes/PointerRing803F.h"

// View selection: WordRingStorage = typed data/head/size/capacity for element access;
// RingHeaderWords = the same four raw ABI words for swaps and capacity arithmetic.
// RingSizeView retains a pointer-typed return word; RingSizeState/OwnedWordBuffer
// retain volatile size reads required by deleting cleanup. They never own extra objects.
// Cursor types use either pointer fields (dereference/copy) or word fields (ABI transfer);
// the optional fifth tag byte belongs to the iterator, not to the ring header.
// Canonical typed ring storage and borrowed four-address cursor.
// A cursor never owns data; logicalBegin determines its wrapped ordering.
struct WordRingStorage {
    unsigned int* data;
    unsigned int head, size, capacity;
};
struct WordRingCursor {
    unsigned int* current;
    unsigned int* storageBegin;
    unsigned int* logicalBegin;
    unsigned int* storageEnd;
};

typedef WordRingStorage WordRingEndSource;
typedef WordRingCursor WordRingEndIterator;
struct OwnedWordBuffer {
    void* data;
    unsigned int head;
    volatile unsigned int size;
};
typedef WordRingCursor RingIteratorSource;
typedef WordRingCursor RingIteratorResult;

struct RingSizeView {
    unsigned int data;
    unsigned int head;
    void* size;
};

struct RingFrontView {
    unsigned int* data;
    unsigned int head;
};

struct RingHeaderWords {
    unsigned int data, head, size, capacity;
};
typedef RingHeaderWords RingCapacityView;

struct RingIteratorCopy {
    unsigned int current;
    unsigned int storageBegin;
    unsigned int logicalBegin;
    unsigned int storageEnd;
};

struct RingIteratorCurrentView {
    void* data;
};

struct RingSizeState {
    unsigned int data;
    unsigned int head;
    volatile unsigned int size;
};

typedef RingHeaderWords RingPopFrontState;

typedef WordRingStorage PageRingBeginSource;

typedef WordRingCursor PageRingBeginIterator;

struct TaggedRingIterator {
    unsigned int current;
    unsigned int storageBegin;
    unsigned int logicalBegin;
    unsigned int storageEnd;
    unsigned char tag;
};

typedef WordRingStorage PageRingBackSource;

typedef WordRingStorage WordRingBeginSource;

typedef WordRingCursor WordRingBeginIterator;

typedef RingHeaderWords PageRingSwapState;

struct PageRingEndSource {
    unsigned int* data;
    unsigned int head;
    unsigned int size;
    unsigned int capacity;
};

typedef WordRingCursor PageRingEndIterator;

typedef WordRingCursor RingDistanceIterator;

typedef WordRingCursor MutableRingIterator;

struct RingInsertionIterator {
    unsigned int current, storageBegin, logicalBegin, storageEnd;
};
typedef RingHeaderWords RingGrowthState;
struct TaggedRingInputIterator {
    unsigned int *current, *storageBegin, *logicalBegin, *storageEnd;
    unsigned char tag;
};
typedef WordRingCursor RingOutputIterator;
struct RingForwardCopyIterator {
    unsigned int* cur;
    unsigned int* first;
    unsigned int* logicalBegin;
    unsigned int* end;
};
struct RingBackwardCopyIterator {
    unsigned int* cur;
    unsigned int* first;
    unsigned int* logicalBegin;
    unsigned int* end;
};
struct ReservableWordRing {
    unsigned int* first;
    unsigned int start;
    unsigned int size;
    unsigned int cap;
};

struct AppendableWordRing {
    unsigned int* first;
    unsigned int start;
    unsigned int size;
    unsigned int cap;
};

extern "C" unsigned char s_gtlWordDequeMaxSizeInitialized;
extern "C" unsigned int s_gtlWordDequeMaxSize;
extern "C" const char audioDequeErrorStrings[0x60];
extern "C" void* __vt__Q23std11logic_error[4];
extern "C" void* __vt__Q23std12length_error[4];
extern "C" void __dt__Q23std12length_errorFv(void*, int);
extern "C" void* __nw__FUl(unsigned int);
extern "C" void* __nwa__FUl(unsigned int);
extern "C" void __dla__FPv(void*);
void PointerRing_dtor(void*, int);

// Reserve usable ring capacity plus its sentinel slot, preserving wrapped word order.
void GtlWordDeque_Reserve(ReservableWordRing* self, unsigned int requestedUsableCapacity) {
    unsigned int cap = self->cap;
    unsigned int currentUsableCapacity = cap ? cap - 1 : 0;
    if (requestedUsableCapacity <= currentUsableCapacity)
        return;

    if (!s_gtlWordDequeMaxSizeInitialized) {
        s_gtlWordDequeMaxSizeInitialized = 1;
        s_gtlWordDequeMaxSize = kMaxWordRingSize;
    }
    if (requestedUsableCapacity > s_gtlWordDequeMaxSize) {
        // Inlined "throw std::length_error(deque:: length error)": heap-copy the message into a ref-counted control block
        // owned by the exception object, then throw. Never returns; skip when reading.
        const char* message = (const char*)audioDequeErrorStrings + kDequeLengthErrorText;
        const char* exceptionMessage = (const char*)audioDequeErrorStrings + kLengthErrorTypeName;
        extern void* s_sharedCharDeleterVtable[];
        struct Error {
            void* vtable;
            char* text;
            void* control;
        } error;
        struct Guard {
            void* buffer;
            // Release diagnostic storage unless exception construction transferred ownership.
            ~Guard() {
                if (buffer)
                    __dla__FPv(buffer);
            }
        };
        error.vtable = __vt__Q23std11logic_error;
        error.text = (char*)__nwa__FUl(strlen(message) + 1);
        {
            Guard guard = {error.text};
            void* allocation = __nw__FUl(0x10);
            if (allocation) {
                ((unsigned int*)allocation)[2] = 1;
                ((unsigned int*)allocation)[1] = 1;
                ((void**)allocation)[0] = s_sharedCharDeleterVtable;
                ((void**)allocation)[3] = error.text;
            }
            error.control = allocation;
            guard.buffer = 0;
        }
        strcpy(error.text, message);
        error.vtable = __vt__Q23std12length_error;
        __throw(exceptionMessage, &error, (void*)__dt__Q23std12length_errorFv);
        return;
    }

    ReservableWordRing temp;
    temp.first = (unsigned int*)gtl_Alloc((requestedUsableCapacity + 1) * 4);
    temp.start = 0;
    temp.size = 0;
    temp.cap = requestedUsableCapacity + 1;
    unsigned int endIndex = self->cap ? (self->start + self->size) % self->cap : 0;
    unsigned int* oldFirst = self->first;
    unsigned int* end = oldFirst + endIndex;
    unsigned int* begin = oldFirst + self->start;
    unsigned int* capEnd = oldFirst + self->cap;
    unsigned int* dst = temp.first + (temp.start + temp.size) % temp.cap;
    if (begin <= end) {
        memmove(dst, begin, (end - begin) * 4);
        temp.size += end - begin;
    } else {
        unsigned int n = capEnd - begin;
        memmove(dst, begin, n * 4);
        memmove(dst + n, oldFirst, (end - oldFirst) * 4);
        temp.size += (capEnd - begin) + (end - oldFirst);
    }
    // Swap the new ring into self, then free the old buffer.
    self->size = 0;
    ReservableWordRing old = *self;
    *self = temp;
    if (old.first)
        gtl_Free(old.first);
}

// Append one word, growing the ring and preserving wrapped contents when necessary.
void GtlWordDeque_PushBack(AppendableWordRing* self, unsigned int* value) {
    unsigned int oldSize = self->size;
    unsigned int oldCap = self->cap;
    unsigned int newSize = oldSize + 1;
    if (newSize < oldCap) {
        unsigned int index = self->start + oldSize;
        unsigned int* slot = self->first + index % oldCap;
        if (slot)
            *slot = *value;
        self->size = newSize;
        return;
    }

    if (!s_gtlWordContainerMaxSizeInitialized) {
        s_gtlWordContainerMaxSizeInitialized = 1;
        s_gtlWordContainerMaxSize = kMaxWordRingSize;
    }
    unsigned int limit = s_gtlWordContainerMaxSize;
    unsigned int maxSize = oldCap ? oldCap - 1 : 0;
    if (limit - maxSize < 1) {
        // Inlined "throw std::length_error(deque:: length error)": heap-copy the message into a ref-counted control block
        // owned by the exception object, then throw. Never returns; skip when reading.
        const char* message = (const char*)audioDequeErrorStrings + kDequeLengthErrorText;
        const char* exceptionMessage = (const char*)audioDequeErrorStrings + kLengthErrorTypeName;
        extern void* s_sharedCharDeleterVtable[];
        struct Error {
            void* vtable;
            char* text;
            void* control;
        } error;
        struct Guard {
            void* buffer;
            // Release diagnostic storage unless exception construction transferred ownership.
            ~Guard() {
                if (buffer)
                    __dla__FPv(buffer);
            }
        };
        error.vtable = __vt__Q23std11logic_error;
        error.text = (char*)__nwa__FUl(strlen(message) + 1);
        {
            Guard guard = {error.text};
            void* allocation = __nw__FUl(0x10);
            if (allocation) {
                ((unsigned int*)allocation)[2] = 1;
                ((unsigned int*)allocation)[1] = 1;
                ((void**)allocation)[0] = s_sharedCharDeleterVtable;
                ((void**)allocation)[3] = error.text;
            }
            error.control = allocation;
            guard.buffer = 0;
        }
        strcpy(error.text, message);
        error.vtable = __vt__Q23std12length_error;
        __throw(exceptionMessage, &error, (void*)__dt__Q23std12length_errorFv);
    }
    // Grow by 3/5 while small, by 1/2 up to two thirds of the limit, then to the limit.
    unsigned int newMax = limit;
    if (maxSize < limit / 3) {
        unsigned int grow = ((maxSize + 1) * 3) / 5;
        newMax = maxSize + (grow < 1 ? 1 : grow);
    } else if (maxSize < (limit / 3) * 2) {
        unsigned int grow = (maxSize + 1) / 2;
        newMax = maxSize + (grow < 1 ? 1 : grow);
    }
    if (!s_gtlWordDequeMaxSizeInitialized) {
        s_gtlWordDequeMaxSizeInitialized = 1;
        s_gtlWordDequeMaxSize = kMaxWordRingSize;
    }
    if (newMax > s_gtlWordDequeMaxSize) {
        // Inlined "throw std::length_error(deque:: length error)": heap-copy the message into a ref-counted control block
        // owned by the exception object, then throw. Never returns; skip when reading.
        const char* message = (const char*)audioDequeErrorStrings + kDequeLengthErrorText;
        const char* exceptionMessage = (const char*)audioDequeErrorStrings + kLengthErrorTypeName;
        extern void* s_sharedCharDeleterVtable[];
        struct Error {
            void* vtable;
            char* text;
            void* control;
        } error;
        struct Guard {
            void* buffer;
            // Release diagnostic storage unless exception construction transferred ownership.
            ~Guard() {
                if (buffer)
                    __dla__FPv(buffer);
            }
        };
        error.vtable = __vt__Q23std11logic_error;
        error.text = (char*)__nwa__FUl(strlen(message) + 1);
        {
            Guard guard = {error.text};
            void* allocation = __nw__FUl(0x10);
            if (allocation) {
                ((unsigned int*)allocation)[2] = 1;
                ((unsigned int*)allocation)[1] = 1;
                ((void**)allocation)[0] = s_sharedCharDeleterVtable;
                ((void**)allocation)[3] = error.text;
            }
            error.control = allocation;
            guard.buffer = 0;
        }
        strcpy(error.text, message);
        error.vtable = __vt__Q23std12length_error;
        __throw(exceptionMessage, &error, (void*)__dt__Q23std12length_errorFv);
    }
    // The new value goes at index oldSize; the old elements are copied in front of it.
    AppendableWordRing temp;
    temp.first = (unsigned int*)gtl_Alloc((newMax + 1) * 4);
    temp.cap = newMax + 1;
    temp.start = oldSize;
    temp.size = 0;
    unsigned int* slot = temp.first + oldSize;
    if (slot)
        *slot = *value;
    temp.size++;
    unsigned int endIndex = self->cap ? (self->start + self->size) % self->cap : 0;
    unsigned int* oldFirst = self->first;
    unsigned int* end = oldFirst + endIndex;
    unsigned int* begin = oldFirst + self->start;
    unsigned int* capEnd = oldFirst + self->cap;
    unsigned int* dst = temp.first + temp.start;
    if (begin <= end) {
        unsigned int n = end - begin;
        memmove(dst - n, begin, n * 4);
        temp.size += n;
        temp.start -= n;
    } else {
        unsigned int n1 = end - oldFirst;
        unsigned int* d1 = dst - n1;
        memmove(d1, oldFirst, n1 * 4);
        unsigned int n2 = capEnd - begin;
        memmove(d1 - n2, begin, n2 * 4);
        temp.start -= n1 + n2;
        temp.size += n1 + n2;
    }
    self->size = 0;
    AppendableWordRing old = *self;
    *self = temp;
    if (old.first)
        gtl_Free(old.first);
}

// Copy insertion iterators before forwarding a range-insertion request.
void PointerRing_InsertRangeWithIteratorCopies(void* ring, RingInsertionIterator* position,
                                               RingInsertionIterator* first, RingInsertionIterator* last) {
    RingInsertionIterator positionCopy;
    RingInsertionIterator firstCopy;
    RingInsertionIterator lastCopy;
    unsigned char iteratorTag = 0;
    positionCopy.current = position->current;
    positionCopy.storageBegin = position->storageBegin;
    positionCopy.logicalBegin = position->logicalBegin;
    positionCopy.storageEnd = position->storageEnd;
    firstCopy.current = first->current;
    firstCopy.storageBegin = first->storageBegin;
    firstCopy.logicalBegin = first->logicalBegin;
    firstCopy.storageEnd = first->storageEnd;
    lastCopy.current = last->current;
    lastCopy.storageBegin = last->storageBegin;
    lastCopy.logicalBegin = last->logicalBegin;
    lastCopy.storageEnd = last->storageEnd;
    PointerRing_InsertWordRange(ring, &positionCopy, &firstCopy, &lastCopy, &iteratorTag);
}

// Copy ring words forward in contiguous spans and return the advanced destination iterator.
void RingBufferIterator_CopyForward(RingForwardCopyIterator* out, RingForwardCopyIterator* first,
                                    RingForwardCopyIterator* last, RingForwardCopyIterator* destination) {
    while (first->cur != last->cur) {
        int destinationSpan = (int)((char*)destination->end - (char*)destination->cur) / 4;
        int sourceSpan;
        if ((first->cur < first->logicalBegin) == (last->cur < last->logicalBegin))
            sourceSpan = (int)((char*)last->cur - (char*)first->cur) / 4;
        else
            sourceSpan = (int)((char*)first->end - (char*)first->cur) / 4;
        int count = destinationSpan;
        if (sourceSpan < count)
            count = sourceSpan;
        memmove(destination->cur, first->cur, count * 4);
        first->cur += count;
        if (first->cur == first->end)
            first->cur = first->first;
        destination->cur += count;
        if (destination->cur == destination->end)
            destination->cur = destination->first;
    }
    *out = *destination;
}

// Copy ring words backward across wrap boundaries and return the destination iterator.
void RingBufferIterator_CopyBackward(RingBackwardCopyIterator* out, RingBackwardCopyIterator* first,
                                     RingBackwardCopyIterator* last, RingBackwardCopyIterator* destination) {
    while (first->cur != last->cur) {
        if (last->cur == last->first)
            last->cur = last->end;
        last->cur--;
        if (destination->cur == destination->first)
            destination->cur = destination->end;
        destination->cur--;
        int destinationSpan = (int)((char*)destination->cur - (char*)(destination->cur < destination->logicalBegin
                                                                          ? destination->first
                                                                          : destination->logicalBegin)) /
                              4;
        int sourceSpan;
        if ((first->cur < first->logicalBegin) == (last->cur < last->logicalBegin))
            sourceSpan = (int)((char*)last->cur - (char*)first->cur) / 4;
        else
            sourceSpan =
                (int)((char*)last->cur - (char*)(last->cur < last->logicalBegin ? last->first : last->logicalBegin)) /
                4;
        int count = destinationSpan;
        if (sourceSpan < count)
            count = sourceSpan;
        unsigned int* src = last->cur - count;
        last->cur = src;
        memmove(destination->cur - count, src, (count + 1) * 4);
        destination->cur -= count;
    }
    *out = *destination;
}

// Forward a single-word append to the word deque.
void GtlWordDeque_ForwardPushBack(AppendableWordRing* self, unsigned int* value) {
    GtlWordDeque_PushBack(self, value);
}

// Build a word-ring end iterator including both storage boundaries and the logical beginning.
void WordRing_GetEndIterator(WordRingEndIterator* dst, WordRingEndSource* src) {
    unsigned int cap;
    unsigned int* base;
    unsigned int index;
    cap = src->capacity;
    base = src->data;
    if (cap != 0)
        index = (src->head + src->size) % cap;
    else
        index = 0;
    dst->current = base + index;
    base = src->data;
    dst->storageBegin = base;
    index = src->head;
    dst->logicalBegin = base + index;
    index = src->capacity;
    dst->storageEnd = base + index;
}

// Free owned word storage, preserve the original volatile size clear, and optionally free the owner.
void* OwnedWordBuffer_dtor(OwnedWordBuffer* self, int shouldDelete) {
    if (self != 0) {
        void* owned = self->data;
        if (owned != 0) {
            unsigned int zero = self->size - self->size;
            self->size = zero;
            gtl_Free(owned);
        }
        if (shouldDelete > 0)
            __dl__FPv(self);
    }
    return self;
}

// Return the slot holding the last page pointer in the page ring.
unsigned int* GtlSampleDeque_GetBackPageSlot(PageRingBackSource* arg) {
    unsigned int index = arg->head + arg->size;
    unsigned int* base = arg->data;
    index -= 1;
    index = index % arg->capacity;
    return base + index;
}

// Build a word-ring begin iterator including its wrap boundaries.
void WordRing_GetBeginIterator(WordRingBeginIterator* dst, WordRingBeginSource* src) {
    unsigned int* base = src->data;
    unsigned int end = src->head;
    dst->current = base + end;
    base = src->data;
    dst->storageBegin = base;
    end = src->head;
    dst->logicalBegin = base + end;
    dst->storageEnd = base + src->capacity;
}

// Swap page-ring storage, head, size and capacity words.
void GtlSampleDeque_SwapPageRingState(PageRingSwapState* a, PageRingSwapState* b) {
    unsigned int value = a->data;
    a->data = b->data;
    b->data = value;
    value = a->head;
    a->head = b->head;
    b->head = value;
    value = a->size;
    a->size = b->size;
    b->size = value;
    value = a->capacity;
    a->capacity = b->capacity;
    b->capacity = value;
}

// Return the current front slot of the ring.
unsigned int* PageRing_GetFrontSlot(RingFrontView* arg) {
    return arg->data + arg->head;
}

// Return allocated capacity minus the unused sentinel slot, or zero for an unallocated ring.
unsigned int PointerRing_GetUsableCapacity(RingCapacityView* arg) {
    unsigned int zero;
    unsigned int result;
    unsigned int value;
    value = arg->capacity;
    zero = !value;
    result = value - 1;
    zero = -zero;
    return result & ~zero;
}

// Returns dst (r3 untouched, 0x803F1EEC); PointerRing_InsertWordRange and PointerRing_InsertRingIteratorRange pass it on (0x803F20B0, 0x803F76D0).
// Copy the four iterator boundary words and return the destination.
RingIteratorCopy* RingBufferIterator_CopyBounds(RingIteratorCopy* dst, RingIteratorCopy* src) {
    unsigned int storageEnd = src->storageEnd;
    unsigned int logicalBegin = src->logicalBegin;
    unsigned int storageBegin = src->storageBegin;
    unsigned int current = src->current;
    dst->current = current;
    dst->storageBegin = storageBegin;
    dst->logicalBegin = logicalBegin;
    dst->storageEnd = storageEnd;
    return dst;
}

// Return the ring size through the existing pointer-shaped ABI.
void* PointerRing_GetSize(RingSizeView* arg) {
    return arg->size;
}

// Return the iterator current-slot address.
void* RingBufferIterator_GetCurrentSlot(RingIteratorCurrentView* arg) {
    return arg->data;
}

// Set the ring size to zero without releasing storage.
void RingBuffer_ClearSize(RingSizeState* arg) {
    arg->size = 0;
}

// Build an iterator moved backward by an offset, wrapping once across storage boundaries.
void RingBufferIterator_Subtract(RingIteratorResult* dst, RingIteratorSource* src, int amount) {
    int neg = -amount;
    volatile RingIteratorSource* input = src;
    unsigned int* begin = input->storageBegin;
    unsigned int* last = input->storageEnd;
    unsigned int* current = input->current;
    unsigned int* other = input->logicalBegin;
    volatile unsigned int* savedBegin = begin;
    volatile unsigned int* savedOther = other;
    volatile unsigned int* savedLast = last;
    int size = (int)((char*)last - (char*)begin) / 4;
    int offset = (int)((char*)current - (char*)begin) / 4;
    int index = neg + offset;
    if (index < 0)
        index += size;
    else if (index >= size)
        index -= size;
    unsigned int* chosen = (unsigned int*)((char*)begin + index * 4);
    dst->current = chosen;
    dst->storageBegin = (unsigned int*)savedBegin;
    dst->logicalBegin = (unsigned int*)savedOther;
    dst->storageEnd = (unsigned int*)savedLast;
}

// Test whether the ring has zero elements.
int PointerRing_IsEmpty(RingSizeState* arg) {
    return !arg->size;
}

// Clear the ring size while retaining the original read-before-write operation.
void PointerRing_ClearSize(RingSizeState* arg) {
    unsigned int value = arg->size;
    arg->size = value - value;
}

// Advance the ring head by one with wrapping and decrement the element count.
void PageRing_PopFront(RingPopFrontState* arg) {
    unsigned int current = arg->head + 1;
    arg->head = current;
    if (current == arg->capacity) {
        arg->head = 0;
    }
    arg->size = arg->size - 1;
}

// Build a begin iterator for the sample deque page-pointer ring.
void PageRing_GetBeginIterator(PageRingBeginIterator* dst, PageRingBeginSource* src) {
    unsigned int* base = src->data;
    unsigned int count = src->head;
    unsigned int limit = src->capacity;
    unsigned int* upper = base + limit;
    unsigned int* current = base + count;
    dst->storageBegin = base;
    dst->current = current;
    dst->logicalBegin = current;
    dst->storageEnd = upper;
}

// Returns dst (r3 untouched, 0x803F28A0); PointerRing_InsertWordRange passes it on (0x803F1FC4, 0x803F21A4).
// Copy the four iterator words and its tag byte, then return the destination.
TaggedRingIterator* RingBufferIterator_CopyTaggedBounds(TaggedRingIterator* dst, TaggedRingIterator* src) {
    unsigned int current = src->current;
    unsigned int storageBegin = src->storageBegin;
    unsigned int logicalBegin = src->logicalBegin;
    unsigned int storageEnd = src->storageEnd;
    unsigned char tag = src->tag;
    dst->current = current;
    dst->storageBegin = storageBegin;
    dst->logicalBegin = logicalBegin;
    dst->storageEnd = storageEnd;
    dst->tag = tag;
    return dst;
}

// Build an end iterator for the sample deque page-pointer ring.
void PageRing_GetEndIterator(PageRingEndIterator* dst, PageRingEndSource* src) {
    unsigned int cap = src->capacity;
    unsigned int offset;
    if (cap != 0) {
        unsigned int value = src->head + src->size;
        offset = value - (value / cap) * cap;
    } else {
        offset = 0;
    }
    unsigned int count = src->head;
    unsigned int* base = src->data;
    dst->storageBegin = base;
    dst->current = base + offset;
    dst->logicalBegin = base + count;
    dst->storageEnd = base + cap;
}

#pragma opt_lifetimes off
// Return the signed iterator distance, accounting for the logical wrap boundary.
int RingBufferIterator_Distance(RingDistanceIterator* a, RingDistanceIterator* b) {
    unsigned int* first = b->current;
    unsigned int* current = a->current;
    unsigned int* last = b->logicalBegin;
    int result = (int)((char*)first - (char*)current) / 4;
    unsigned int* begin = b->storageBegin;
    unsigned int* end = b->storageEnd;
    if ((unsigned int)first < (unsigned int)last) {
        if ((unsigned int)current < (unsigned int)last)
            return result;
        return result + (int)((char*)end - (char*)begin) / 4;
    }
    if ((unsigned int)current >= (unsigned int)last)
        return result;
    return result - (int)((char*)end - (char*)begin) / 4;
}
#pragma opt_lifetimes reset

// Move an iterator backward in place, wrapping once within storage.
void RingBufferIterator_SubtractAssign(MutableRingIterator* arg, int amount) {
    unsigned int* begin = arg->storageBegin;
    int negAmount = -amount;
    unsigned int* last = arg->storageEnd;
    unsigned int* current = arg->current;
    int size = (int)((char*)last - (char*)begin) / 4;
    int index = negAmount + (int)((char*)current - (char*)begin) / 4;
    if (index < 0)
        index += size;
    else if (index >= size)
        index -= size;
    arg->current = begin + index;
}

// Choose ring growth from the requested increment and existing capacity, checking the maximum size.
unsigned int PointerRing_ComputeGrowthCapacity(RingGrowthState* self, unsigned int additionalCount) {
    if (!s_gtlWordContainerMaxSizeInitialized) {
        s_gtlWordContainerMaxSizeInitialized = 1;
        s_gtlWordContainerMaxSize = kMaxWordRingSize;
    }
    unsigned int cap = self->capacity;
    unsigned int maxSize = s_gtlWordContainerMaxSize;
    unsigned int usableCapacity = (cap == 0) ? 0 : (cap - 1);
    if (additionalCount > maxSize - usableCapacity) {
        // Inlined "throw std::length_error(deque:: length error)": heap-copy the message into a ref-counted control block
        // owned by the exception object, then throw. Never returns; skip when reading.
        const char* message = (const char*)audioDequeErrorStrings + kDequeLengthErrorText;
        const char* exceptionMessage = (const char*)audioDequeErrorStrings + kLengthErrorTypeName;
        extern void* s_sharedCharDeleterVtable[];
        struct Error {
            void* vtable;
            char* text;
            void* control;
        } error;
        struct Guard {
            void* buffer;
            // Release diagnostic storage unless exception construction transferred ownership.
            ~Guard() {
                if (buffer)
                    __dla__FPv(buffer);
            }
        };
        error.vtable = __vt__Q23std11logic_error;
        error.text = (char*)__nwa__FUl(strlen(message) + 1);
        {
            Guard guard = {error.text};
            void* allocation = __nw__FUl(0x10);
            if (allocation) {
                ((unsigned int*)allocation)[2] = 1;
                ((unsigned int*)allocation)[1] = 1;
                ((void**)allocation)[0] = s_sharedCharDeleterVtable;
                ((void**)allocation)[3] = error.text;
            }
            error.control = allocation;
            guard.buffer = 0;
        }
        strcpy(error.text, message);
        error.vtable = __vt__Q23std12length_error;
        __throw(exceptionMessage, &error, (void*)__dt__Q23std12length_errorFv);
    }
    // Unsigned reciprocal division: floor(maxSize / 3). These spellings preserve code generation.
    enum {
        kDivideByThreeReciprocal = 0xaaaaaaab,
        kDivideByFiveReciprocal = 0xcccccccd
    };
    unsigned int oneThirdMax =
        (unsigned int)(((unsigned long long)(unsigned int)kDivideByThreeReciprocal * maxSize) >> 33);
    unsigned int result;
    if (usableCapacity < oneThirdMax) {
        unsigned int capacityWithSentinel = usableCapacity + 1;
        unsigned int tripleCapacity = capacityWithSentinel * 4 - capacityWithSentinel;
        // floor(3 * (usableCapacity + 1) / 5): same small-ring policy as PushBack.
        unsigned int growth =
            (unsigned int)(((unsigned long long)(unsigned int)kDivideByFiveReciprocal * tripleCapacity) >> 34);
        unsigned int increment = (growth >= additionalCount) ? growth : additionalCount;
        result = usableCapacity + increment;
    } else if (usableCapacity < oneThirdMax * 2) {
        unsigned int half = (usableCapacity + 1) >> 1;
        unsigned int increment = (half >= additionalCount) ? half : additionalCount;
        result = usableCapacity + increment;
    } else {
        result = maxSize;
    }
    return result;
}

// Copy words from a wrapped source range into successive destination spans.
void RingBufferIterator_CopyRange(RingOutputIterator* dst, TaggedRingInputIterator* firstIterator,
                                  RingOutputIterator* lastIterator, RingOutputIterator* destinationIterator) {
    unsigned int* first = firstIterator->current;
    unsigned int* last = lastIterator->current;
    if (first != last) {
        unsigned int* lastLogicalBegin = lastIterator->logicalBegin;
        int distance = (int)((char*)last - (char*)first) / 4;
        if (last >= lastLogicalBegin) {
            if (first < lastLogicalBegin)
                distance -= (int)((char*)lastIterator->storageEnd - (char*)lastIterator->storageBegin) / 4;
        } else if (first >= lastLogicalBegin) {
            distance += (int)((char*)lastIterator->storageEnd - (char*)lastIterator->storageBegin) / 4;
        }
        int destinationSpan = (int)((char*)destinationIterator->storageEnd - (char*)destinationIterator->current) / 4;
        while (first != last) {
            unsigned int* sourceCurrent = firstIterator->current;
            unsigned int* sourceStorageEnd = firstIterator->storageEnd;
            unsigned int* sourceLogicalBegin = firstIterator->logicalBegin;
            unsigned int* sourceStorageBegin = firstIterator->storageBegin;
            unsigned int sourceSpan = (unsigned int)((char*)sourceStorageEnd - (char*)sourceCurrent) / 4;
            int count = destinationSpan;
            if (count >= distance)
                count = distance;
            int index = count + (int)((char*)sourceCurrent - (char*)sourceStorageBegin) / 4;
            int capacity = (int)((char*)sourceStorageEnd - (char*)sourceStorageBegin) / 4;
            if (index < 0)
                index += capacity;
            else if (index >= capacity)
                index -= capacity;
            unsigned int* target = sourceStorageBegin + index;
            unsigned int* src = sourceCurrent;
            unsigned int* out = destinationIterator->current;
            while (src != target) {
                *out++ = *src++;
                if (src == sourceStorageEnd)
                    src = sourceStorageBegin;
            }
            first = target;
            distance -= count;
            destinationIterator->current += count;
            if (destinationIterator->current == destinationIterator->storageEnd)
                destinationIterator->current = destinationIterator->storageBegin;
            if (distance != 0)
                firstIterator->current = target;
            destinationSpan = (int)((char*)destinationIterator->storageEnd - (char*)destinationIterator->current) / 4;
        }
    }
    dst->current = destinationIterator->current;
    dst->storageBegin = destinationIterator->storageBegin;
    dst->logicalBegin = destinationIterator->logicalBegin;
    dst->storageEnd = destinationIterator->storageEnd;
}

// The iterators carry four addresses and, for the input iterator, one tag byte.
// Keep calls to the original iterator helpers; copying only the current pointer
// would lose their circular-buffer boundary state.
struct Link3Iterator {
    unsigned int word[4];
    unsigned char tag;
};
struct Link3Ring {
    unsigned int* data;
    unsigned int head, size, capacity;
};
// This cross-specialization accessor returns word +0xC: capacity for this ring,
// head-page index for GtlSampleDeque.
void* Container_GetWord0CAddress(void*);
void RingCursor_AddOffset(void*, void*, int);
void RingCursor_SubtractOffset(void*, void*, int);
void RingCursor_CopyFourWords(void*, const void*);
void* PointerRing_GetCapacityAddressForConstruction(void*);
void* PointerRing_GetCapacityAddress(void*);
void PointerRing_AllocateCapacity(void*, unsigned int, unsigned int);
unsigned int RingCursor_Dereference(void*);
void RingCursor_Increment(void*);
void* PointerRing_GetAllocatorValue(void*);
void Memory_CopyWordIfDestinationNonNull(void*, void*, const void*);
void PointerRing_AppendCursorRange(void*, void*, void*);
void Swap_Int32ForRingStorage(void*, void*);
void Swap_Int32(void*, void*);
void PointerRing_dtor(void*, int);
enum RingStorageWordIndex {
    kRingDataWord = 0,
    kRingHeadWord = 1,
    kRingSizeWord = 2
};
// Five ABI words: the first four are the ring header; helpers manage the trailing allocator state.
struct Link3RingStorage {
    unsigned int word[5];
    // Start an empty temporary ring using the borrowed allocator tag.
    Link3RingStorage(void* allocator) {
        ((PointerRing803F*)this)->Init((int)allocator);
    }
    // Release temporary slots, including old storage received by the final swap.
    ~Link3RingStorage() {
        PointerRing_dtor(this, -1);
    }
};

#pragma push
#pragma always_inline on
// ABI adapters retain function-pointer calls to keep the existing out-of-line helper calls.
// The four-word views agree in layout; these casts do not change the number/order of arguments.
// Copy a tagged cursor; its tag and four borrowed boundary addresses travel together.
static inline void CopyTaggedRingCursor(Link3Iterator* out, void* source) {
    ((void (*)(void*, void*))RingBufferIterator_CopyTaggedBounds)(out, source);
}
// Copy four cursor bounds without changing iterator position or ownership.
static inline void CopyRingCursorBounds(Link3Iterator* out, Link3Iterator* source) {
    ((void (*)(void*, void*))RingBufferIterator_CopyBounds)(out, source);
}
// Construct a borrowed begin cursor from the ring's current header.
static inline void GetRingBeginCursor(Link3Iterator* out, void* ring) {
    ((void (*)(void*, void*))WordRing_GetBeginIterator)(out, ring);
}
// Construct a borrowed end cursor from the ring's current header.
static inline void GetRingEndCursor(Link3Iterator* out, void* ring) {
    ((void (*)(void*, void*))WordRing_GetEndIterator)(out, ring);
}
// Copy a shifted prefix forward; destination must precede any overlapping source.
static inline void CopyRingWordsForward(Link3Iterator* out, Link3Iterator* first, Link3Iterator* last,
                                        Link3Iterator* destination) {
    ((void (*)(void*, void*, void*, void*))RingBufferIterator_CopyForward)(out, first, last, destination);
}
// Copy a shifted suffix backward; destination must follow any overlapping source.
static inline void CopyRingWordsBackward(Link3Iterator* out, Link3Iterator* first, Link3Iterator* last,
                                         Link3Iterator* destination) {
    ((void (*)(void*, void*, void*, void*))RingBufferIterator_CopyBackward)(out, first, last, destination);
}
// Copy the tagged input range into the opened gap and return the advanced output cursor.
static inline void CopyRingWordRange(Link3Iterator* out, Link3Iterator* first, Link3Iterator* last,
                                     Link3Iterator* destination) {
    ((void (*)(void*, void*, void*, void*))RingBufferIterator_CopyRange)(out, first, last, destination);
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// first/last must remain valid while a gap opens; overlapping source storage is not staged.
// The growth branch advances caller first; the in-place branch consumes iterator copies.
// Header changes precede copying in place. Allocation is owned by temporary until all swaps finish.
// Insert a word range by moving the shorter prefix/suffix, or allocate and swap in a larger ring.
void PointerRing_InsertWordRange(void* object, void* position, void* first, void* last, unsigned char*) {
    Link3Ring* ring = (Link3Ring*)object;
    Link3Iterator inputEnd, inputBegin;
    CopyTaggedRingCursor(&inputEnd, last);
    CopyTaggedRingCursor(&inputBegin, first);
    unsigned int count = ((int (*)(void*, void*))RingBufferIterator_Distance)(&inputBegin, &inputEnd);
    if (!count)
        return;

    unsigned int* begin = ring->data + ring->head;
    unsigned int* insertion = *(unsigned int**)position;
    unsigned int prefix = (int)((char*)insertion - (char*)begin) / 4;
    if ((unsigned int)insertion < (unsigned int)begin)
        prefix += *(unsigned int*)Container_GetWord0CAddress(object);
    unsigned int capacity = PointerRing_GetUsableCapacity((RingCapacityView*)object);
    // Phase 1a: enough space; shift the shorter side to open the insertion gap.
    if (count <= capacity && ring->size <= capacity - count) {
        Link3Iterator cursor;
        GetRingBeginCursor(&cursor, object);
        unsigned int suffix = ring->size - prefix;
        if (prefix < suffix) {
            Link3Iterator oldBegin;
            CopyRingCursorBounds(&oldBegin, &cursor);
            ((void (*)(void*, int))RingBufferIterator_SubtractAssign)(&cursor, count);
            unsigned int* newBegin =
                (unsigned int*)RingBufferIterator_GetCurrentSlot((RingIteratorCurrentView*)&cursor);
            ring->size += count;
            ring->head = (int)((char*)newBegin - (char*)ring->data) / 4;
            if (prefix) {
                Link3Iterator output, prefixEnd, from, result;
                CopyRingCursorBounds(&output, &cursor);
                RingCursor_AddOffset(&prefixEnd, &oldBegin, prefix);
                CopyRingCursorBounds(&from, &oldBegin);
                CopyRingWordsForward(&result, &from, &prefixEnd, &output);
                RingCursor_CopyFourWords(&cursor, &result);
            }
        } else {
            ring->size += count;
            Link3Iterator end;
            GetRingEndCursor(&end, object);
            RingCursor_CopyFourWords(&cursor, &end);
            if (suffix) {
                Link3Iterator output, shiftedEnd, shiftedStart, result;
                CopyRingCursorBounds(&output, &cursor);
                RingCursor_SubtractOffset(&shiftedEnd, &cursor, count);
                RingCursor_SubtractOffset(&shiftedStart, &cursor, count + suffix);
                CopyRingWordsBackward(&result, &shiftedStart, &shiftedEnd, &output);
                RingCursor_CopyFourWords(&cursor, &result);
            }
            ((void (*)(void*, int))RingBufferIterator_SubtractAssign)(&cursor, count);
        }
        // Phase 2a: fill the gap from copies of the borrowed source iterators.
        Link3Iterator output, endInput, beginInput, result;
        CopyRingCursorBounds(&output, &cursor);
        CopyTaggedRingCursor(&endInput, last);
        CopyTaggedRingCursor(&beginInput, first);
        CopyRingWordRange(&result, &beginInput, &endInput, &output);
    } else {
        // Phase 1b: allocate replacement storage and copy inserted words first.
        Link3RingStorage temporary(PointerRing_GetCapacityAddressForConstruction(object));
        unsigned int newCapacity = PointerRing_ComputeGrowthCapacity((RingGrowthState*)object, count);
        PointerRing_AllocateCapacity(&temporary, ring->size + count, newCapacity);
        temporary.word[kRingHeadWord] = prefix;
        unsigned int* destination = (unsigned int*)temporary.word[kRingDataWord] + prefix;
        while (count) {
            unsigned int value = RingCursor_Dereference(first);
            Memory_CopyWordIfDestinationNonNull(PointerRing_GetAllocatorValue(&temporary), destination, &value);
            --count;
            ++destination;
            ++temporary.word[kRingSizeWord];
            RingCursor_Increment(first);
        }
        Link3Iterator end, begin;
        GetRingEndCursor(&end, object);
        PointerRing_AppendCursorRange(&temporary, position, &end);
        GetRingBeginCursor(&begin, object);
        ((PointerRing803F*)(&temporary))->PrependRange(&begin, position);
        // Phase 2b: transfer all four storage words; temporary now owns the old slots.
        RingBuffer_ClearSize((RingSizeState*)object);
        Swap_Int32ForRingStorage(object, &temporary);
        Swap_Int32(&ring->head, &temporary.word[kRingHeadWord]);
        Swap_Int32(&ring->size, &temporary.word[kRingSizeWord]);
        void* temporaryCapacity = PointerRing_GetCapacityAddress(&temporary);
        Swap_Int32(Container_GetWord0CAddress(object), temporaryCapacity);
    }
}

#pragma pop

} // extern "C"
