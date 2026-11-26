// Pet texture manager: a circular list of texture entries keyed by resource ID, with lookup,
// appending, and freeing the list and manager.

typedef unsigned long new_size_t;
// Construct in caller-supplied storage; allocation remains the caller's responsibility.
inline void* operator new(new_size_t, void* p) {
    return p;
}

// Manager layout: +0 reference count, +4 vtable, +8 node count, +0xC sentinel.
// head.next/head.prev form a circular list; the embedded head has no live Entry.
// Each allocated 16-byte Node owns only its storage. CPetTexManager::~CPetTexManager also
// releases one resource-manager ID per Entry before freeing the list nodes.
// Entry.key is that resource ID. val is the byte returned by List_GetByteByKey;
// extra is stored with the entry and never interpreted here.
#include "MSL/list.h"
#include "classes/CPetTexManager.h"
typedef PetTextureEntry PetListEntry;
typedef std::list<PetListEntry, app::app_allocator<PetListEntry> > PetEntryList;
struct Node;
typedef PetTextureEntry Entry;
struct Node {
    Node* prev;
    Node* next;
    Entry e;
};
enum {
    kTextureEntryNodeBytes = 16
};
typedef char TextureEntryNode_size[(sizeof(Node) == kTextureEntryNodeBytes) ? 1 : -1];
struct List {
    int unk0;
    int vtable;
    int count;
    Node head;
};

extern "C" {

Node* appMallocDummy__FUl(int size);
void appFreeUni__FPv(void* n);
void __dl__FPv(void* p);

// Borrow the first matching payload, or return null; the list keeps ownership.
static inline Entry* findEntry(List* l, int key) {
    Node* end = (Node*)&l->head;
    for (Node* n = l->head.next; n != end; n = n->next) {
        if (key == n->e.key)
            return &n->e;
    }
    return 0;
}

// Destroy the embedded list's nodes; flag > 0 also frees self, which is returned.
// self is the list subobject at manager +8: [0] count, [1..] sentinel links.
// Repeated null checks preserve the nested destructor expansion in the object.
void* PetTextureEntryList_Dtor(int* self, int flag) {
    return ::std::__list_deleting_dtor((PetEntryList*)self, flag);
}

extern int __vt__14CPetTexManager[];
extern void* g_resourceManager;
void ResourceManager_ReleaseId(void*, int);

// Release each texture ID, free its list node, and optionally delete the manager.
CPetTexManager::~CPetTexManager() {
    List* l = (List*)this;
    l->vtable = (int)&__vt__14CPetTexManager;
    // Phase 1: release resources while all entry keys are still accessible.
    Node* end = &l->head;
    Node* cur = l->head.next;
    while (cur != end) {
        ResourceManager_ReleaseId(g_resourceManager, cur->e.key);
        cur = cur->next;
    }
    // Phase 2: unlink the entire range, then free nodes without visiting the sentinel.
    Node* end2 = &l->head;
    Node* cur2 = l->head.next;
    if (cur2 != end2) {
        Node* a = *(Node**)end2;
        cur2->prev->next = a->next;
        a->next->prev = cur2->prev;
        while (cur2 != end2) {
            Node* t = cur2;
            cur2 = cur2->next;
            appFreeUni__FPv(t);
            l->count--;
        }
    }
    // Phase 3: embedded-list destructor expansion; phase 2 left it empty.
    if (&l->count) {
        if (&l->count) {
            if (&l->count) {
                if (&l->count) {
                    Node* end3 = &l->head;
                    Node* cur3 = l->head.next;
                    if (cur3 != end3) {
                        Node* a = *(Node**)end3;
                        cur3->prev->next = a->next;
                        a->next->prev = cur3->prev;
                        while (cur3 != end3) {
                            Node* t = cur3;
                            cur3 = cur3->next;
                            appFreeUni__FPv(t);
                            l->count--;
                        }
                    }
                }
            }
        }
    }
}
#pragma optimize_for_size on
// Write the matching entry byte and return 1; return 0 leaving *out unchanged if absent.
int List_GetByteByKey(List* l, int key, unsigned char* out) {
    Entry* found = findEntry(l, key);
    if (found == 0)
        return 0;
    *out = found->val;
    return 1;
}

#pragma optimize_for_size off
struct Guard_u80047564 {
    char flag;
    Node* allocation;
    Node* copy;
    Node* unused;
    Node* head;
    Node* n;
    // Start with no owned node; the other words reproduce the allocator guard ABI.
    Guard_u80047564() : flag(0), n(0) {}
    // Free an allocated node if append exits before transferring it to the list.
    ~Guard_u80047564() {
        if (n)
            appFreeUni__FPv(n);
    }
    // Take temporary ownership of p until append clears n after linking.
    void init(Node* p, Node* h) {
        allocation = p;
        copy = p;
        head = h;
        n = p;
    }
};

// Copy the eight-byte payload, including padding; retain the original exception cleanup.
static inline void constructEntry(Node* n, const Entry& e) {
    Entry* p = &n->e;
    if (p) {
        try {
            struct W {
                int a, b;
            };
            *(W*)p = *(const W*)&e;
        } catch (...) {
            appFreeUni__FPv(n);
            throw;
        }
    }
}
// Append (resource ID, lookup byte, uninterpreted short); list owns the new node.
// Always returns 1 after insertion. No retain call or duplicate-key check occurs here.
int CPetTexManager::AppendTextureEntry(int key, unsigned char lookupByte, unsigned short extraValue) {
    // Prepare the payload and keep the allocation guarded until it is linked.
    Entry e = {};
    Guard_u80047564 guard;
    Node* head = (Node*)&entries.prev;
    e.extra = extraValue;
    e.val = lookupByte;
    e.key = key;
    Node* n = appMallocDummy__FUl(kTextureEntryNodeBytes);
    guard.init(n, head);
    constructEntry(n, e);
    // Splice before the sentinel, publish the count, then relinquish the guard.
    head->prev->next = n;
    n->prev = head->prev;
    head->prev = n;
    n->next = head;
    entries.size += 1;
    guard.n = 0;
    return 1;
}

} // extern "C"
