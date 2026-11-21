#include <decomp/offset_of.h>
// Translation unit 0x80048334-0x80049254 (10 functions; boundaries inferred by decomp-toolkit from extab).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {
void OSLockMutex(void*);
void OSUnlockMutex(void*);
void appFreeUni__FPv(void*);
void __dl__FPv(void*);

// All node lists are circular, with null head for empty and self-links for a singleton.
// Registry nodes use +0xC/+0x10 links for active/free/block lists, and +0x14 for hash chains.
// A block's first 0x18-byte node is its allocation-list header; remaining nodes are recyclable.
// nodesPerBlock must exceed 1. Auxiliary slots own separate node/buffer allocations.
// The same OS mutex is acquired recursively by callers and list/hash helpers.
struct PoolListLinks {
    char prefix[0xC];
    PoolListLinks* volatile next;
    PoolListLinks* volatile prev;
};

struct WInstanceLock {
    void* lock;
    // Acquire the supplied OS mutex for this scope.
    WInstanceLock(void* l) { lock = l; OSLockMutex(l); }
    // Release the OS mutex when this scope ends.
    ~WInstanceLock() { OSUnlockMutex(lock); }
};

struct PoolListStorage {
    char prefix[8];
    PoolListLinks* head;
    char reserved0C[0x10];
    void* buf;
    char lock[4];
};

struct NamedPoolNode {
    void* value;           // 0x00
    int nameAddress;               // 0x04
    unsigned char ownsName;     // 0x08
    char padding09[3];
    NamedPoolNode* volatile next;  // 0x0C
    NamedPoolNode* volatile prev;  // 0x10
    NamedPoolNode* chain;
};

class PoolObjectFactory;
class NamedPoolSource {
public:
 virtual ~NamedPoolSource();
 virtual const char* GetSourceName();
 virtual void p02(); virtual void p03(); virtual void p04(); virtual void p05();
 virtual void p06(); virtual void p07(); virtual void p08(); virtual void p09();
 virtual void p10(); virtual void p11(); virtual void p12(); virtual void p13();
 virtual void p14(); virtual void p15(); virtual void p16(); virtual void p17();
  virtual void* CreateNamedValue(int, int, int);
};
int CString_Compare(const char*, const char*);

struct PoolListSlot { PoolListStorage owner; char pad24[0x14]; };

struct Generic3DPool {
    PoolListSlot slots[4];
    NamedPoolNode* activeHead;            // 0xE0
    NamedPoolNode* iterator;
    NamedPoolNode* allocationBlocks;            // 0xE8
    NamedPoolNode* freeNodes;            // 0xEC
    int nodesPerBlock;               // 0xF0
    unsigned mask;
    char padF8[4];
    NamedPoolNode** buckets;
    char lock[4];          // 0x100
    char pad104[0x14];
    NamedPoolSource* source;
    PoolObjectFactory* factory;       // 0x11C
};

// Owner-subobject guards are retained for ABI cleanup; derive their addresses from the layout.
enum PoolOwnerOffset {
    kPoolRegistryOwner = (int)DECOMP_OFFSETOF(Generic3DPool, activeHead),
    kPoolAuxiliarySlot0 = (int)DECOMP_OFFSETOF(Generic3DPool, slots[0]),
    kPoolAuxiliarySlot1 = (int)DECOMP_OFFSETOF(Generic3DPool, slots[1]),
    kPoolAuxiliarySlot2 = (int)DECOMP_OFFSETOF(Generic3DPool, slots[2]),
    kPoolAuxiliarySlot3 = (int)DECOMP_OFFSETOF(Generic3DPool, slots[3])
};
enum PoolNameHash { kPoolFirstByteMask = 0x2A, kPoolLastByteMask = 0x15, kPoolLengthShift = 6 };

class PoolObjectFactory {
public:
    virtual ~PoolObjectFactory(); virtual void p01(); virtual void p02(); virtual void p03();
    virtual void p04(); virtual void p05(); virtual void p06(); virtual void p07();
    virtual void p08(); virtual void p09(); virtual void p10(); virtual void p11();
    virtual void p12(); virtual void p13(); virtual void p14();
    virtual void* CreateValue(int, int);
    // 0x44
};

void* appMallocUni__FUl(unsigned long);

// Link a node into a mutex-protected circular list, returning the list head.
inline NamedPoolNode* PushAfter(void* lock, NamedPoolNode* head, NamedPoolNode* node) {
    void* lk = lock;
    OSLockMutex(lk);
    if (!head) {
        node->next = node;
        node->prev = node;
        OSUnlockMutex(lk);
        return node;
    }
    node->prev = head;
    node->next = head->next;
    head->next = node;
    node->next->prev = node;
    OSUnlockMutex(lk);
    return head;
}

// Unlink a node under the mutex and return the previous node, or null for a singleton.
inline NamedPoolNode* Detach(void* lock, NamedPoolNode* n) {
    NamedPoolNode* nx;
    void* lk = lock;
    OSLockMutex(lk);
    n->prev->next = n->next;
    n->next->prev = n->prev;
    nx = n->prev;
    if (nx == n) nx = 0;
    OSUnlockMutex(lk);
    return nx;
}

// Default virtual operation reports false.
int WDevice_GetDeviceName() { return 0; }

#pragma push
#pragma use_lmw_stmw on
// Free circular-list nodes and their backing buffer, then optionally delete this storage.
PoolListStorage* dtor_80048DD4(PoolListStorage* self, int flag) {
    if (self) {
        {
            WInstanceLock g(&self->lock);
            if (self->head) {
                PoolListLinks* nx;
                void* lk = &self->lock;
                PoolListLinks* n;
                while ((n = self->head) != 0) {
                    OSLockMutex(lk);
                    n->prev->next = n->next;
                    n->next->prev = n->prev;
                    nx = n->prev;
                    if (nx == n) nx = 0;
                    OSUnlockMutex(lk);
                    self->head = nx;
                    appFreeUni__FPv(n);
                }
            }
            if (self->buf) {
                appFreeUni__FPv(self->buf);
                self->buf = 0;
            }
        }
        if (flag > 0) __dl__FPv(self);
    }
    return self;
}

// Free circular-list nodes and their backing buffer, then optionally delete this storage.
PoolListStorage* dtor_80048EAC(PoolListStorage* self, int flag) {
    if (self) {
        {
            WInstanceLock g(&self->lock);
            if (self->head) {
                PoolListLinks* nx;
                void* lk = &self->lock;
                PoolListLinks* n;
                while ((n = self->head) != 0) {
                    OSLockMutex(lk);
                    n->prev->next = n->next;
                    n->next->prev = n->prev;
                    nx = n->prev;
                    if (nx == n) nx = 0;
                    OSUnlockMutex(lk);
                    self->head = nx;
                    appFreeUni__FPv(n);
                }
            }
            if (self->buf) {
                appFreeUni__FPv(self->buf);
                self->buf = 0;
            }
        }
        if (flag > 0) __dl__FPv(self);
    }
    return self;
}

// Free circular-list nodes and their backing buffer, then optionally delete this storage.
PoolListStorage* dtor_80048F84(PoolListStorage* self, int flag) {
    if (self) {
        {
            WInstanceLock g(&self->lock);
            if (self->head) {
                PoolListLinks* nx;
                void* lk = &self->lock;
                PoolListLinks* n;
                while ((n = self->head) != 0) {
                    OSLockMutex(lk);
                    n->prev->next = n->next;
                    n->next->prev = n->prev;
                    nx = n->prev;
                    if (nx == n) nx = 0;
                    OSUnlockMutex(lk);
                    self->head = nx;
                    appFreeUni__FPv(n);
                }
            }
            if (self->buf) {
                appFreeUni__FPv(self->buf);
                self->buf = 0;
            }
        }
        if (flag > 0) __dl__FPv(self);
    }
    return self;
}

// Free circular-list nodes and their backing buffer, then optionally delete this storage.
PoolListStorage* dtor_8004905C(PoolListStorage* self, int flag) {
    if (self) {
        {
            WInstanceLock g(&self->lock);
            if (self->head) {
                PoolListLinks* nx;
                void* lk = &self->lock;
                PoolListLinks* n;
                while ((n = self->head) != 0) {
                    OSLockMutex(lk);
                    n->prev->next = n->next;
                    n->next->prev = n->prev;
                    nx = n->prev;
                    if (nx == n) nx = 0;
                    OSUnlockMutex(lk);
                    self->head = nx;
                    appFreeUni__FPv(n);
                }
            }
            if (self->buf) {
                appFreeUni__FPv(self->buf);
                self->buf = 0;
            }
        }
        if (flag > 0) __dl__FPv(self);
    }
    return self;
}

// Free circular-list nodes and their backing buffer, then optionally delete this storage.
PoolListStorage* dtor_80049134(PoolListStorage* self, int flag) {
    if (self) {
        {
            WInstanceLock g(&self->lock);
            if (self->head) {
                PoolListLinks* nx;
                void* lk = &self->lock;
                PoolListLinks* n;
                while ((n = self->head) != 0) {
                    OSLockMutex(lk);
                    n->prev->next = n->next;
                    n->next->prev = n->prev;
                    nx = n->prev;
                    if (nx == n) nx = 0;
                    OSUnlockMutex(lk);
                    self->head = nx;
                    appFreeUni__FPv(n);
                }
            }
            if (self->buf) {
                appFreeUni__FPv(self->buf);
                self->buf = 0;
            }
        }
        if (flag > 0) __dl__FPv(self);
    }
    return self;
}

unsigned long strlen(const char*);
class Disposable {
public:
    virtual ~Disposable();
};

// Remove a named value from its hash bucket while holding the recursive registry mutex.
inline void RemoveHash(Generic3DPool* self, NamedPoolNode* n) {

    WInstanceLock g(&self->lock);
    unsigned hash;
    {
        const unsigned char* str = (const unsigned char*)n->nameAddress;
        void* lk = &self->lock;
        OSLockMutex(lk);
        int len = strlen((const char*)str);
        int last = len > 0 ? len - 1 : 0;
        hash = ((len << kPoolLengthShift) | ((str[0] & kPoolFirstByteMask) | (str[last] & kPoolLastByteMask))) & self->mask;
        OSUnlockMutex(lk);
    }
    NamedPoolNode* p = self->buckets[hash];
    if (p == n) {
        self->buckets[hash] = n->chain;
    } else {
        while (p) {
            if (p->chain == n) {
                p->chain = n->chain;
                return;
            }
            p = p->chain;
        }
    }

}

// Unlink a registered value, free its owned name, and recycle the registry node.
// Only the owned name is freed here: the caller still owns destruction of value.
inline void RemoveValue(Generic3DPool* self, void* value) {

    WInstanceLock g(&self->lock);
    NamedPoolNode* head = self->activeHead;
    NamedPoolNode* n = head;
    if (!head) return;
    do {
        if (n->value == value) {
            if (self->iterator == n) {
                NamedPoolNode* p = self->iterator->prev;
                if (p == head) p = 0;
                self->iterator = p;
            }
            if (n->nameAddress) {
                RemoveHash(self, n);
                if (n->ownsName) appFreeUni__FPv((void*)n->nameAddress);
            }
            {
                void* lk = &self->lock;
                NamedPoolNode* h = self->activeHead;
                OSLockMutex(lk);
                n->prev->next = n->next;
                n->next->prev = n->prev;
                if (n == h) {
                    NamedPoolNode* p = n->prev;
                    if (p == h) h = 0;
                    else h = p;
                }
                OSUnlockMutex(lk);
                self->activeHead = h;
            }
            self->freeNodes = PushAfter(&self->lock, self->freeNodes, n);
            if (!self->activeHead) self->iterator = 0;
            return;
        }
        n = n->prev;
    } while (n != head);

}

// Free registry allocation blocks and hash buckets while holding the registry mutex.
inline void ReleaseRegistryStorage(Generic3DPool* self) {
    if ((char*)self + kPoolRegistryOwner) {
        WInstanceLock g(&self->lock);
        if (self->allocationBlocks) {
            NamedPoolNode* nx;
            void* lk = &self->lock;
            NamedPoolNode* n;
            while ((n = self->allocationBlocks) != 0) {
                OSLockMutex(lk);
                n->prev->next = n->next;
                n->next->prev = n->prev;
                nx = n->prev;
                if (nx == n) nx = 0;
                OSUnlockMutex(lk);
                self->allocationBlocks = nx;
                appFreeUni__FPv(n);
            }
        }
        if (self->buckets) {
            appFreeUni__FPv(self->buckets);
            self->buckets = 0;
        }
    }
}

// Release the fourth auxiliary list and its backing buffer under its mutex.
inline void ReleaseAuxiliarySlot3Storage(Generic3DPool* self) {
    if ((char*)self + kPoolAuxiliarySlot3) {
        WInstanceLock g(&self->slots[3].owner.lock);
        if (self->slots[3].owner.head) {
            PoolListLinks* nx;
            void* lk = &self->slots[3].owner.lock;
            PoolListLinks* n;
            while ((n = self->slots[3].owner.head) != 0) {
                OSLockMutex(lk);
                n->prev->next = n->next;
                n->next->prev = n->prev;
                nx = n->prev;
                if (nx == n) nx = 0;
                OSUnlockMutex(lk);
                self->slots[3].owner.head = nx;
                appFreeUni__FPv(n);
            }
        }
        if (self->slots[3].owner.buf) {
            appFreeUni__FPv(self->slots[3].owner.buf);
            self->slots[3].owner.buf = 0;
        }
    }
}

// Release the third auxiliary list and its backing buffer under its mutex.
inline void ReleaseAuxiliarySlot2Storage(Generic3DPool* self) {
    if ((char*)self + kPoolAuxiliarySlot2) {
        WInstanceLock g(&self->slots[2].owner.lock);
        if (self->slots[2].owner.head) {
            PoolListLinks* nx;
            void* lk = &self->slots[2].owner.lock;
            PoolListLinks* n;
            while ((n = self->slots[2].owner.head) != 0) {
                OSLockMutex(lk);
                n->prev->next = n->next;
                n->next->prev = n->prev;
                nx = n->prev;
                if (nx == n) nx = 0;
                OSUnlockMutex(lk);
                self->slots[2].owner.head = nx;
                appFreeUni__FPv(n);
            }
        }
        if (self->slots[2].owner.buf) {
            appFreeUni__FPv(self->slots[2].owner.buf);
            self->slots[2].owner.buf = 0;
        }
    }
}

// Release the second auxiliary list and its backing buffer under its mutex.
inline void ReleaseAuxiliarySlot1Storage(Generic3DPool* self) {
    if ((char*)self + kPoolAuxiliarySlot1) {
        WInstanceLock g(&self->slots[1].owner.lock);
        if (self->slots[1].owner.head) {
            PoolListLinks* nx;
            void* lk = &self->slots[1].owner.lock;
            PoolListLinks* n;
            while ((n = self->slots[1].owner.head) != 0) {
                OSLockMutex(lk);
                n->prev->next = n->next;
                n->next->prev = n->prev;
                nx = n->prev;
                if (nx == n) nx = 0;
                OSUnlockMutex(lk);
                self->slots[1].owner.head = nx;
                appFreeUni__FPv(n);
            }
        }
        if (self->slots[1].owner.buf) {
            appFreeUni__FPv(self->slots[1].owner.buf);
            self->slots[1].owner.buf = 0;
        }
    }
}

// Release the first auxiliary list and its backing buffer under its mutex.
inline void ReleaseAuxiliarySlot0Storage(Generic3DPool* self) {
    if ((char*)self + kPoolAuxiliarySlot0) {
        WInstanceLock g(&self->slots[0].owner.lock);
        if (self->slots[0].owner.head) {
            PoolListLinks* nx;
            void* lk = &self->slots[0].owner.lock;
            PoolListLinks* n;
            while ((n = self->slots[0].owner.head) != 0) {
                OSLockMutex(lk);
                n->prev->next = n->next;
                n->next->prev = n->prev;
                nx = n->prev;
                if (nx == n) nx = 0;
                OSUnlockMutex(lk);
                self->slots[0].owner.head = nx;
                appFreeUni__FPv(n);
            }
        }
        if (self->slots[0].owner.buf) {
            appFreeUni__FPv(self->slots[0].owner.buf);
            self->slots[0].owner.buf = 0;
        }
    }
}

#pragma push
#pragma optimize_for_size on
// Unlink a circular-list node, preserving the predecessor as the head when removing the head.
inline NamedPoolNode* UnlinkTargetHead(NamedPoolNode* head, NamedPoolNode* n) {
    n->prev->next = n->next;
    n->next->prev = n->prev;
    if (n != head) return head;
    NamedPoolNode* p = n->prev;
    if (p == head) return 0;
    return p;
}

// Detach a registry-ring node; caller publishes predecessor and owns node release.
static inline void DetachRegistryStorageNode(NamedPoolNode* node, NamedPoolNode*& predecessor, void* mutex) {
    OSLockMutex(mutex);
    node->prev->next = node->next;
    node->next->prev = node->prev;
    predecessor = node->prev;
    if (predecessor == node) predecessor = 0;
    OSUnlockMutex(mutex);
}
// Detach an auxiliary-ring node under its owner's mutex; retain the predecessor as the new head.
static inline void DetachAuxiliaryStorageNode(PoolListLinks* node, PoolListLinks*& predecessor, void* mutex) {
    OSLockMutex(mutex);
    node->prev->next = node->next;
    node->next->prev = node->prev;
    predecessor = node->prev;
    if (predecessor == node) predecessor = 0;
    OSUnlockMutex(mutex);
}

// Destroy registered objects and factories, then release the node pool and auxiliary lists.
Generic3DPool* Generic3DPool_dtor(Generic3DPool* self, int flag) {
    // registry guard; hash removal nests a second guard on the same recursive mutex.
    // hash_removed leaves that inner guard, removed_value leaves the registry guard.
    // delete value runs after both guards have ended, so virtual cleanup may reenter.
    // Slot storage is released in reverse index order; slot payload roles are unknown.
    if (self) {
        Disposable* value;
        NamedPoolNode* n;
        // Phase 1: unlink each value from hash/active lists before invoking its destructor.
        // Returned nodes go to the free ring; whole blocks remain allocated until phase 3.
        for (;;) {
            {
                WInstanceLock g(&self->lock);
                void* lk = &self->lock;
                self->iterator = self->activeHead;
                OSLockMutex(lk);
                NamedPoolNode* cursor = self->iterator;
                if (cursor) {
                    struct Prev { NamedPoolNode* node; } p = { cursor->prev };
                    value = (Disposable*)cursor->value;
                    if (p.node == self->activeHead) p.node = 0;
                    self->iterator = p.node;
                    OSUnlockMutex(lk);
                } else {
                    OSUnlockMutex(lk);
                    value = 0;
                }
            }
            if (value == 0) break;
            do {
                WInstanceLock g(&self->lock);
                struct Head { NamedPoolNode* node; } first = { self->activeHead };
                n = first.node;
                // The single-pass scope lets the empty-list exit release its mutex owner.
                if (first.node == 0) break;
                do {
                    if (n->value == value) {
                        if (self->iterator == n) {
                            NamedPoolNode* p = self->iterator->prev;
                            if (p == first.node) p = 0;
                            self->iterator = p;
                        }
                        if (n->nameAddress) {
                            {
                                WInstanceLock g(&self->lock);
                                unsigned int hash;
                                {
                                    void* lk = &self->lock;
                                    const unsigned char* str = (const unsigned char*)n->nameAddress;
                                    OSLockMutex(lk);
                                    int len = strlen((const char*)str);
                                    int last = len > 0 ? len - 1 : 0;
                                    unsigned raw = (len << kPoolLengthShift) | ((str[0] & kPoolFirstByteMask) | (str[last] & kPoolLastByteMask));
                                    hash = raw & self->mask;
                                    OSUnlockMutex(lk);
                                }
                                NamedPoolNode* p = self->buckets[hash];
                                if (p == n) {
                                    self->buckets[hash] = n->chain;
                                } else {
                                    while (p) {
                                        if (p->chain == n) {
                                            p->chain = n->chain;
                                            // Preserve the distinct recursive-mutex cleanup for this exit.
                                            goto hash_removed;
                                        }
                                        p = p->chain;
                                    }
                                }
                            }
hash_removed:;
                            if (n->ownsName) appFreeUni__FPv((void*)n->nameAddress);
                        }
                        {
                            NamedPoolNode* h = self->activeHead;
                            void* lk = &self->lock;
                            OSLockMutex(lk);
                            h = UnlinkTargetHead(h, n);
                            OSUnlockMutex(lk);
                            self->activeHead = h;
                        }
                        NamedPoolNode* freeListHead = self->freeNodes;
                        self->freeNodes = PushAfter(&self->lock, freeListHead, n);
                        if (0 == self->activeHead) self->iterator = 0;
                        // Preserve the distinct outer-mutex cleanup after successful removal.
                        goto removed_value;
                    }
                    n = n->prev;
                } while (n != first.node);
            } while (0);
removed_value:;
            delete value;
        }
        // Phase 2: all registered values are gone; destroy both provider objects.
        if (self->factory) {
            delete self->factory;
            self->factory = 0;
        }
        if (self->source) {
            delete self->source;
            self->source = 0;
        }
        // Phase 3: release registry blocks/buckets, then auxiliary slots in reverse order.
        {
            if ((char*)self + kPoolRegistryOwner) {
                WInstanceLock g(&self->lock);
                if (self->allocationBlocks) {
                    NamedPoolNode* nx;
                    void* lk = &self->lock;
                    NamedPoolNode* n;
                    while ((n = self->allocationBlocks) != 0) {
                        DetachRegistryStorageNode(n, nx, lk);
                        self->allocationBlocks = nx;
                        appFreeUni__FPv(n);
                    }
                }
                if (self->buckets) {
                    appFreeUni__FPv(self->buckets);
                    self->buckets = 0;
                }
            }

        }
        {
            if ((char*)self + kPoolAuxiliarySlot3) {
                WInstanceLock g(&self->slots[3].owner.lock);
                if (self->slots[3].owner.head) {
                    PoolListLinks* nx;
                    void* lk = &self->slots[3].owner.lock;
                    PoolListLinks* n;
                    while ((n = self->slots[3].owner.head) != 0) {
                        DetachAuxiliaryStorageNode(n, nx, lk);
                        self->slots[3].owner.head = nx;
                        appFreeUni__FPv(n);
                    }
                }
                if (self->slots[3].owner.buf) {
                    appFreeUni__FPv(self->slots[3].owner.buf);
                    self->slots[3].owner.buf = 0;
                }
            }

        }
        {
            if ((char*)self + kPoolAuxiliarySlot2) {
                WInstanceLock g(&self->slots[2].owner.lock);
                if (self->slots[2].owner.head) {
                    PoolListLinks* nx;
                    void* lk = &self->slots[2].owner.lock;
                    PoolListLinks* n;
                    while ((n = self->slots[2].owner.head) != 0) {
                        DetachAuxiliaryStorageNode(n, nx, lk);
                        self->slots[2].owner.head = nx;
                        appFreeUni__FPv(n);
                    }
                }
                if (self->slots[2].owner.buf) {
                    appFreeUni__FPv(self->slots[2].owner.buf);
                    self->slots[2].owner.buf = 0;
                }
            }

        }
        {
            if ((char*)self + kPoolAuxiliarySlot1) {
                WInstanceLock g(&self->slots[1].owner.lock);
                if (self->slots[1].owner.head) {
                    PoolListLinks* nx;
                    void* lk = &self->slots[1].owner.lock;
                    PoolListLinks* n;
                    while ((n = self->slots[1].owner.head) != 0) {
                        DetachAuxiliaryStorageNode(n, nx, lk);
                        self->slots[1].owner.head = nx;
                        appFreeUni__FPv(n);
                    }
                }
                if (self->slots[1].owner.buf) {
                    appFreeUni__FPv(self->slots[1].owner.buf);
                    self->slots[1].owner.buf = 0;
                }
            }

        }
        {
            if ((char*)self + kPoolAuxiliarySlot0) {
                WInstanceLock g(&self->slots[0].owner.lock);
                if (self->slots[0].owner.head) {
                    PoolListLinks* nx;
                    void* lk = &self->slots[0].owner.lock;
                    PoolListLinks* n;
                    while ((n = self->slots[0].owner.head) != 0) {
                        DetachAuxiliaryStorageNode(n, nx, lk);
                        self->slots[0].owner.head = nx;
                        appFreeUni__FPv(n);
                    }
                }
                if (self->slots[0].owner.buf) {
                    appFreeUni__FPv(self->slots[0].owner.buf);
                    self->slots[0].owner.buf = 0;
                }
            }

        }
        if (flag > 0) __dl__FPv(self);
    }
    return self;
}
#pragma pop

// Create a value through the matching named source and register it in a recycled pool node.
// name selects the source provider, not the new object's hash name; null accepts this source.
// Return the created value stored in registryNode->value; its concrete type is unknown.
// The pool owns successful values until removal/destruction; a3/a4 are forwarded unchanged.
void* Generic3DPool_CreateFromNamedSource(Generic3DPool* self, const char* name, int a3, int a4) {
    void* createdValue = 0;
    if (!name || !CString_Compare(self->source->GetSourceName(), name))
        createdValue = self->source->CreateNamedValue(a3, 0, a4);
    if (createdValue) {
        WInstanceLock g1(&self->lock);
        NamedPoolNode* registryNode;
        {
            WInstanceLock g2(&self->lock);
            if (!self->freeNodes) {
                NamedPoolNode* allocationBlock = (NamedPoolNode*)appMallocUni__FUl(self->nodesPerBlock * 0x18);
                self->allocationBlocks = PushAfter(&self->lock, self->allocationBlocks, allocationBlock);
                for (int i = 1; self->nodesPerBlock > i; i++) {
                    NamedPoolNode* freeNode = (i + allocationBlock);
                    self->freeNodes = PushAfter(&self->lock, self->freeNodes, freeNode);
                }
            }
            NamedPoolNode* freeHeadSnapshot1 = self->freeNodes;
            __typeof__(self->freeNodes) freeHeadSnapshot2 = self->freeNodes;
            __typeof__(self->freeNodes) freeHeadSnapshot3 = self->freeNodes;
            NamedPoolNode* freeHeadSnapshot4 = self->freeNodes;
            NamedPoolNode* freeHead = self->freeNodes;
            registryNode = freeHead;
            self->freeNodes = Detach(&self->lock, registryNode);
        }
        registryNode->value = createdValue;
        registryNode->ownsName = 0;
        registryNode->nameAddress = 0;
        NamedPoolNode* activeHead = self->activeHead;
        self->activeHead = PushAfter(&self->lock, activeHead, registryNode);
    }
    return createdValue;
}

// Create a value through the fallback factory and register it in a recycled pool node.
// a2 is ignored; a3 is forwarded to CreateValue with a zero second argument.
// The return is the created value, registered for pool destruction; its concrete type is unknown.
void* Generic3DPool_CreateFromFactory(Generic3DPool* self, int a2, int a3) {
    void* createdValue = self->factory->CreateValue(a3, 0);
    if (createdValue) {
        WInstanceLock g1(&self->lock);
        NamedPoolNode* registryNode;
        {
            WInstanceLock g2(&self->lock);
            if (0 == self->freeNodes) {
                NamedPoolNode* allocationBlock = (NamedPoolNode*)appMallocUni__FUl(self->nodesPerBlock * 0x18);
                self->allocationBlocks = PushAfter(&self->lock, self->allocationBlocks, allocationBlock);
                for (int i = 1; i < self->nodesPerBlock; i++) {
                    NamedPoolNode* freeNode = (i + allocationBlock);
                    self->freeNodes = PushAfter(&self->lock, self->freeNodes, freeNode);
                }
            }
            NamedPoolNode* freeHead = self->freeNodes;
            registryNode = freeHead;
            self->freeNodes = Detach(&self->lock, registryNode);
        }
        registryNode->value = createdValue;
        registryNode->ownsName = 0;
        registryNode->nameAddress = 0;
        NamedPoolNode* activeHead = self->activeHead;
        self->activeHead = PushAfter(&self->lock, activeHead, registryNode);
    }
    return createdValue;
}

#pragma pop
// Release an otherwise empty object only for a deleting-destructor request.
void* dtor_80049214(void* self, int flag) {
    if (self && flag > 0) __dl__FPv(self);
    return self;
}

} // extern "C"
