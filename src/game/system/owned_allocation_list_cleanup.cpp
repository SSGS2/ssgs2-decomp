// Cleanup for lists of owned objects (DtorList). Each entry runs its destructor, and its
// storage is freed when the entry owns it. Clearing, releasing and container destruction build on this.

struct OwnedAllocation {
    void* vtable;
    void* allocation;
};
enum {
    kOwnedStorageFlagShift = 31
};

extern "C" {

void appFreeUni__FPv(void*);
void __dl__FPv(void*);

struct DtorEntry {
    unsigned flags;
    char reserved04[4]; // Unread slot metadata.
    void* object;
    char reserved0C[4]; // Unread slot metadata.
};

struct DtorList {
    DtorEntry* entries;
    int count;
    char reserved08[8]; // Unread allocation metadata.
    int first;
};

// entries owns the whole slot array; the live range is [first, first + count).
// An entry owns its +8 allocation only when flags bit 31 is set. Other metadata
// and the payload's type are unresolved; cleanup calls free, not a payload destructor.
// Both public destructors clear the live range before releasing the array.

// Free a single owned allocation, then optionally delete its wrapper.
// deleteFlag > 0 frees self; returning self does not make it valid after deletion.
void* dtor_80065F18(void* self, int deleteFlag) {
    if (self != 0) {
        void* object = ((OwnedAllocation*)self)->allocation;
        if (object != 0)
            appFreeUni__FPv(object);
        if (deleteFlag > 0)
            __dl__FPv(self);
    }
    return self;
}

// The sign bit marks entries whose object storage is owned by this slot.
inline void destroyEntry(DtorEntry* entry) {
    if (entry != 0) {
        if (entry != 0 && (entry->flags >> kOwnedStorageFlagShift) != 0)
            appFreeUni__FPv(entry->object);
    }
}

// Release backing storage after clearList has set the live count to zero.
// Precondition: count == 0. The preserved fallback loop decrements from entries,
// not from the live-range end, so this is not a stand-alone clear operation.
inline void releaseList(DtorList* self) {
    if (self != 0) {
        DtorEntry* allocationStart = self->entries;
        if (allocationStart != 0) {
            int remaining = self->count;
            self->count = 0;
            while (remaining != 0) {
                destroyEntry(--allocationStart);
                --remaining;
            }
            appFreeUni__FPv(self->entries);
        }
    }
}

// Destroy the active subrange in reverse order without freeing the slot array.
// first and count must describe valid slots; first is a slot index, not a byte offset.
inline void clearList(DtorList* self) {
    DtorEntry* first = self->entries + self->first;
    DtorEntry* entry = first + self->count;
    while (entry > first) {
        destroyEntry(--entry);
    }

    self->count = 0;
}

// Clear owned slots and release the array, optionally deleting the container.
void* PtrList_dtor(DtorList* self, int deleteFlag) {
    if (self != 0) {
        clearList(self);
        releaseList(self);
        if (deleteFlag > 0)
            __dl__FPv(self);
    }
    return self;
}

// Same container cleanup for the second instantiated slot-list type.
void* dtor_80065E14(DtorList* self, int deleteFlag) {
    if (self != 0) {
        clearList(self);
        releaseList(self);
        if (deleteFlag > 0)
            __dl__FPv(self);
    }
    return self;
}

typedef char owned_slot_size[(sizeof(DtorEntry) == 0x10) ? 1 : -1];
typedef char owned_slot_object_at_08[((int)&((DtorEntry*)0)->object == 8) ? 1 : -1];

} // extern "C"
