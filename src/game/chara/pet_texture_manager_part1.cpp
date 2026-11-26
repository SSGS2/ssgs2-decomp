// Pet texture manager: the texture entry record and its constructor, the manager constructor,
// and a startup angle constant.

#include <decomp/static_init.h>
#include "classes/CPetTexManager.h"

extern "C" {

struct Entry {
    short status;
    char pad2[2];
    void* first;
    void* second;
};
// An entry is a 16-bit status plus two pointer-sized payloads. This constructor
// establishes zero/empty state only; payload identities and ownership are unresolved.

// Initialize an empty entry; the two pointer payloads start detached.
Entry* __ct__5EntryFv(Entry* self) {
    self->status = 0;
    self->first = 0;
    self->second = 0;
    return self;
}

extern float kFloat_6_2831855;
extern float s_initializedTwoPi_800474AC;

// Initialize this translation unit's writable two-pi scalar at startup.
void PetTextureUnit_InitTwoPi() {
    s_initializedTwoPi_800474AC = kFloat_6_2831855;
}

extern void appFreeUni__FPv(void*);

struct Node {
    Node* next;
    Node* prev;
};

struct Container {
    int referenceCount;
    void* vtable;
    int textureCount;
    Node sentinel;
};
// Header layout: referenceCount +0, vtable +4, list count +8, sentinel links +0xC/+0x10.
// The sentinel lives inside the manager, never in a separately allocated node.

extern char __vt__14CPetTexManager[];

// Start a reference-counted pet texture manager with an empty circular list.
CPetTexManager::CPetTexManager() {
    Container* self = (Container*)this;
    Node* end = &self->sentinel;
    Node* node = &self->sentinel;
    self->referenceCount = 1;
    self->vtable = __vt__14CPetTexManager;
    self->textureCount = 0;
    self->sentinel.prev = node;
    self->sentinel.next = node;
    // Both iterators refer to the just-initialized sentinel: this cleanup branch
    // is empty on construction. Keep the recovered container cleanup pattern intact.
    if (node != end) {
        Node* prev = node->prev;
        Node* next = node->next;
        prev->next = next;
        next->prev = prev;
        while (node != end) {
            Node* old = node;
            node = node->next;
            appFreeUni__FPv(old);
            self->textureCount -= 1;
        }
    }
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80461FB8", _ctors_u80047498_0, PetTextureUnit_InitTwoPi);
#pragma pop
