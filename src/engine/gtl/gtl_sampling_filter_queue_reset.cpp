// Translation unit 0x803F5C5C-0x803F5F00 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".
// Sampling-filter queue reset (streaming infrastructure; no gameplay tuning).
// API: gtl_CSamplingFilter_ResetQueue empties the retained-sample queue before
// gtl_IFilter_SetState transitions the base to numeric state 0. Queue storage
// layout and reference-release policy are defined below; no sample bytes move.
// Caller must stop concurrent producers/consumers before resetting or destroying.

// Local sampling-queue view; do not confuse page count/capacity with entry count.
// Each allocated page contains 128 eight-byte {retained sample*, consumedBytes} entries.
// data is a circular array of page pointers; its logical range includes a null
// sentinel. offset is the first entry's index relative to the head page; size
// counts live entries. Reset releases sample references and surplus pages, but
// keeps the pointer ring and up to two slots (including the sentinel) for reuse.
enum SamplingQueueLayout {
    kEntriesPerPage = 128, kPageShift = 7, kEntryMask = 127,
    kRetainedPageSlots = 2, kEmptyEntryOffset = 64
};

struct RefBase { int refs; }; // Ref adds its virtual table after this reference count.
struct Ref : RefBase { 
virtual ~Ref();
};
struct Entry { Ref* object; unsigned consumedBytes; }; // Byte cursor within the retained sample.

struct Ring {
    Entry** node;
    Entry** begin;
    Entry** originSlot;
    Entry** end;
    // Select head in the borrowed page-pointer allocation; originSlot records that logical origin.
    Ring(Entry** data, unsigned head, unsigned capacity) {
        node = data + head;
        begin = data;
        originSlot = data + head;
        end = data + capacity;
    }
    // Move by page slots with a single wrap; caller must keep the result within one ring turn.
    Ring& advance(int amount) {
        int slotCapacity = end - begin;
        int slotIndex = amount + (node - begin);
        if (slotIndex < 0) slotIndex += slotCapacity;
        else if (slotIndex >= slotCapacity) slotIndex -= slotCapacity;
        node = begin + slotIndex;
        return *this;
    }
};

struct RingIterator {
    Entry** node;
    Entry** begin;
    Entry** originSlot;
    Entry** end;
    Entry* item;
    // Advance one entry, crossing to the next page and wrapping the page ring if needed.
    void next() {
        ++item;
        if (item == *node + kEntriesPerPage) {
            ++node;
            if (node == end) node = begin;
            item = *node;
        }
    }
};

struct Filter {
    char pad[0x14];
    unsigned sourceWord;       // +0x14: upstream source address, despite this legacy name; cleared on reset.
    unsigned unknown18;
    unsigned firstEntryIndex;      // +0x1C: entry origin within the logical page sequence.
    unsigned entryCount;        // +0x20: live entry count.
    Entry** pageSlots;         // +0x24: owned page-pointer ring allocation.
    unsigned firstPageSlot;        // +0x28: first page-pointer slot index.
    unsigned pageSlotCount;       // +0x2C: logical page-pointer slots, including the sentinel.
    unsigned pageSlotCapacity;    // +0x30: ring capacity in pointer slots.
    // Borrow an iterator at an absolute entry position (offset already included).
    RingIterator at(unsigned position) {
        Ring ring(pageSlots, firstPageSlot, pageSlotCapacity);
        Ring& selected = ring.advance(position >> kPageShift);
        Entry* block = pageSlotCount == 0 ? 0 : *selected.node;
        RingIterator result = {selected.node, selected.begin, selected.originSlot, selected.end, block + (position & kEntryMask)};
        return result;
    }
    // Compute the one-past-last address from offset+size; do not dereference the result.
    Entry* endItem() {
        unsigned position = entryCount + firstEntryIndex;
        Ring ring(pageSlots, firstPageSlot, pageSlotCapacity);
        Ring& selected = ring.advance(position >> kPageShift);
        Entry* block = pageSlotCount == 0 ? 0 : *selected.node;
        return block + (position & kEntryMask);
    }
};

extern "C" {
void gtl_Free(void*);
void gtl_IFilter_SetState(void*, int);
#pragma opt_propagation off
// Preserve the expanded ring/iterator expressions in this function: propagation
// must not fold the iterator snapshots into subsequent mutable member reads.
#pragma opt_common_subs off
// Keep the separately evaluated page/end cursor loads across reference deletion;
// a virtual destructor is an external call between iterator/end evaluations.
#pragma opt_lifetimes off
// Keep the temporary RingIterator and its copy as distinct lifetime/storage
// candidates; merging them changes the register/stack shape of the reset walk.
#pragma opt_strength_reduction off
// Retain page division/masking and slot wrap expressions rather than converting
// this traversal to an induction-pointer loop. These four switches preserve the
// established compiler shape; they do not enable a different queue policy.
// Empty the queue, releasing retained samples and excess pages, and reset base-filter state.
// Requires a valid initialized queue; returns 1 after the base state is written.
// Clears the upstream source word first. Entry.consumedBytes is untouched.
// Retained pages contain stale entries outside the new size=0 range.
// sourceWord is a borrowed upstream address: clearing it does not release that
// source. Each live entry relinquishes one retained sample reference; zero
// references invoke virtual destruction. Empty-queue stale pointers are never
// released again unless a producer first republishes them as live entries.
// Numeric base state 0 is the constructor's initial state (gtl_media_pipeline_filters.cpp).
// It is written only AFTER queue cleanup, so state observers see an empty queue
// at that initial state. It is not the entry count or a reset success code.
// The setter is a plain void state-word write; there is no failure or rollback.
int gtl_CSamplingFilter_ResetQueue(Filter* self) {
    // Phase 1: release each live reference before discarding the logical entry range.
    self->sourceWord = 0;
    unsigned position = self->firstEntryIndex;
    Ring ring(self->pageSlots, self->firstPageSlot, self->pageSlotCapacity);
    Ring& selected = ring.advance(position >> kPageShift);
    Entry* block = self->pageSlotCount == 0 ? 0 : *selected.node;
    RingIterator temporary = {selected.node, selected.begin, selected.originSlot, selected.end, block + (position & kEntryMask)};
    RingIterator iterator = temporary;
    while (iterator.item != self->endItem()) {
        Ref* object = iterator.item->object;
        if (object && --object->refs == 0) delete object;
        iterator.next();
    }
    self->entryCount = 0;
    // Phase 2: trim front page storage while retaining two pointer slots for reuse.
    while (self->pageSlotCount > kRetainedPageSlots) {
        gtl_Free(self->pageSlots[self->firstPageSlot]);
        if (++self->firstPageSlot == self->pageSlotCapacity) self->firstPageSlot = 0;
        --self->pageSlotCount;
    }
    if (self->pageSlotCount != 0) self->firstEntryIndex = kEmptyEntryOffset;
    // Phase 3: publish base state 0 only after releasing entries/trimming pages.
    gtl_IFilter_SetState(self, 0);
    return 1;
}
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
} // extern "C"
