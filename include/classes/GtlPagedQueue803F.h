// GtlPagedQueue803F: a queue of 8-byte entries stored in 128-entry pages, with a ring of page pointers.
// The sampling filter embeds it at +0x1C. The ring's logical range ends with a null sentinel slot, so
// (pages.size - 1) * 128 is the entry capacity. Methods are in gtl_sampling_filter_queues.cpp,
// gtl_pointer_ring_and_paged_queue.cpp and gtl_source_split_transform_filters.cpp.
// No virtual functions; the page ring's address doubles as the allocator handle.
#ifndef CLASSES_GTLPAGEDQUEUE803F_H
#define CLASSES_GTLPAGEDQUEUE803F_H

// Ring of page pointers: first slot, live slot count and physical capacity.
struct GtlPageRing803F {
    unsigned int dataAddress; // 0x0
    unsigned int firstIndex;  // 0x4
    unsigned int size;        // 0x8
    unsigned int capacity;    // 0xC
};

class GtlPagedQueue803F {
public:
    // Setup.
    // Clear the counters and initialize an empty page ring.
    void* Init(void* allocator);

    // Page ring access.
    void* GetPageRing();
    void* GetPageAllocator();
    // Allocate an entry array of 8 bytes per entry.
    static void* AllocatePageEntries(void* allocator, unsigned int entryCount);
    // Slot just before the ring's logical end.
    static unsigned int* GetBackPageSlot(unsigned int* ring);
    // Exchange all four ring words.
    static void SwapPageRingState(unsigned int* a, unsigned int* b);
    // Build iterators over a ring's slots.
    static void MakeBeginIterator(unsigned int* out, unsigned int* ring);
    static void BuildPageRingEndIterator(void* out, void* ring);
    static void CopyRetreatedIterator(unsigned int* out, unsigned int* in, int slots);

    // Capacity.
    unsigned int GetFrontSpareCount();
    unsigned int GetBackSpareCount();
    unsigned int GetMaxSize();
    unsigned int GetStorageEntryCapacity();
    // Make room for size more entries at the front or back, rounded up to whole pages.
    void EnsureFrontSpareCapacity(unsigned int size);
    void EnsureBackSpareCapacity(unsigned int size);

    unsigned int firstIndex; // 0x0: entry index of the first live entry
    unsigned int size;       // 0x4: live entry count
    GtlPageRing803F pages;   // 0x8
};

#define OFF(f, o) typedef char GtlPagedQueue803F_off_##f[((int)&((GtlPagedQueue803F*)0)->f == (o)) ? 1 : -1]
OFF(firstIndex, 0x0);
OFF(size, 0x4);
OFF(pages, 0x8);
#undef OFF
typedef char GtlPagedQueue803F_size[(sizeof(GtlPagedQueue803F) == 0x18) ? 1 : -1];

#endif
