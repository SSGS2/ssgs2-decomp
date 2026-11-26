// Pet animation state: startup full-turn angle constants, the pet animation constructor, and the
// destructors for its owned buffers and node list.

#include <decomp/static_init.h>
#include "classes/CPetAnimation.h"

extern "C" {

extern const float kFloat_6_2831855;
extern const float kFloat_0_PetAnimation;
extern float s_twoPi_80046850;
extern float s_twoPi_8004690C;
extern float s_twoPi_800469FC;

// Initialize this unit's first full-turn angle during static initialization.
void InitializeFullTurnAngle_80046850() {
    s_twoPi_80046850 = kFloat_6_2831855;
}

// Initialize the second independent full-turn angle slot.
void InitializeFullTurnAngle_8004690C() {
    s_twoPi_8004690C = kFloat_6_2831855;
}

// Initialize the pet-animation full-turn angle slot.
void InitializeFullTurnAngle_800469FC() {
    s_twoPi_800469FC = kFloat_6_2831855;
}

// Constructor view only: the destruction helpers below describe buffer and list specializations.
struct PetAnimationState {
    unsigned int referenceCount;
    void* volatile vtable;
    void* entries;
    short entryCount;
    float amount;
    short mode;
};
struct OwnedBuffer {
    void* entries;               // Owned contiguous storage; cleanup does not destroy its elements.
    unsigned int volatile count; // Logical count, not an allocation size in bytes.
};

extern const char __vt__13CPetAnimation[];
void appFreeUni__FPv(void*);
void __dl__FPv(void*);

// Initialize a referenced pet-animation object with no entries and mode/amount zero.
CPetAnimation::CPetAnimation() {
    PetAnimationState* self = (PetAnimationState*)this;
    self->referenceCount = 1;
    self->vtable = (void*)__vt__13CPetAnimation;
    self->entries = 0;
    self->entryCount = 0;
    self->amount = kFloat_0_PetAnimation;
    self->mode = 0;
}

// Clear the owned buffer count, release storage and optionally delete the wrapper.
// Null self is accepted; dealloc > 0 frees self, otherwise only entries is freed.
// Returns the original address, which may have been freed. entries is not nulled.
OwnedBuffer* dtor_80046918(OwnedBuffer* self, int dealloc) {
    if (self != 0) {
        void* data = self->entries;
        if (data != 0) {
            int previousCount = *(int*)&self->count;
            // Preserve the original volatile read/subtract sequence used to clear.
            self->count = previousCount - *(unsigned int*)&self->count;
            appFreeUni__FPv(data);
        }
        if (dealloc > 0)
            __dl__FPv(self);
    }
    return self;
}

// Reset the buffer wrapper before releasing its storage and optional object allocation.
// Same ownership/dealloc contract as dtor_80046918; count is cleared even if empty.
OwnedBuffer* dtor_80046988(OwnedBuffer* self, int dealloc) {
    if (self != 0) {
        self->count = 0;
        if (self != 0) {
            if (self->entries != 0) {
                self->count = 0;
                appFreeUni__FPv(self->entries);
            }
        }
        if (dealloc > 0)
            __dl__FPv(self);
    }
    return self;
}

struct ListNode {
    ListNode* prev;
    ListNode* next;
};

struct OwnedNodeList {
    int count;
    ListNode sentinel; // Embedded circular sentinel; empty links point to this node.
};

// Unlink and free every circular-list node, then optionally delete the list object.
// Nodes own their allocation; no separate payload destruction is performed here.
// Null self is accepted; dealloc > 0 also frees the wrapper. Returns its old address.
OwnedNodeList* OwnedNodeList_dtor(OwnedNodeList* self, int dealloc) {
    if (self != 0) {
        ListNode* sentinel = &self->sentinel;
        ListNode* node = self->sentinel.next;
        if (node != sentinel) {
            // Detach the entire range: the sentinel becomes an empty ring before
            // freeing nodes. Detached next links still lead back to the sentinel.
            ListNode* last = sentinel->prev;
            node->prev->next = last->next;
            last->next->prev = node->prev;
            while (node != sentinel) {
                ListNode* nodeToFree = node;
                node = node->next;
                appFreeUni__FPv(nodeToFree);
                self->count -= 1;
            }
        }
        if (dealloc > 0)
            __dl__FPv(self);
    }
    return self;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80461FAC", _ctors_u80046850_0, InitializeFullTurnAngle_80046850);
STATIC_INIT_ENTRY(".CRT$XCU80461FAC", _ctors_u80046850_1, InitializeFullTurnAngle_8004690C);
STATIC_INIT_ENTRY(".CRT$XCU80461FAC", _ctors_u80046850_2, InitializeFullTurnAngle_800469FC);
#pragma pop
