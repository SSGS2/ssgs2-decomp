#include <decomp/static_init.h>
#include "MSL/vector.h"
// Vector destroyed by dtor_8013AA48; element type not known yet (Elem_8013AA48 is a tag);
// its size reset loads the size twice (form 2).
struct Elem_8013AA48;
namespace std { template <> struct __vec_destroys_elements<Elem_8013AA48*> { enum { value = 2 }; static void destroy(Elem_8013AA48**) {} }; }
typedef std::vector<Elem_8013AA48*, app::app_allocator<Elem_8013AA48*> > Elem_8013AA48Vector;
#include "classes/CGolfPlayer.h"
// 0x8013A3F8: existing tree-sentinel fields and four affine matrices at +0x2E8.

#define AsCGolfPlayer(p) ((CGolfPlayer*)(p))
enum { kPlayerSecondaryVtableIndex = 15, kWhitePlayerLightRgb = 0x00FFFFFF,
       kHeapStringFlagShift = 31, kInlineStringLengthMask = 0x7F };
// Translation unit 0x801398B0-0x8013AE84 (16 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

struct PlayerCleanupItem { char prefix[0x80]; void* allocation; };
struct ListNode { ListNode* prev; ListNode* next; };
struct PlayerCleanupNode { ListNode links; PlayerCleanupItem* item; };
void __dl__FPv(void*);
void appFreeUni__FPv(void*);
extern void __ct__18NativeBase80201D18Fv(void*);
extern void Matrix_SetIdentityAffine(void*);
extern void IntStringTree_DestroySubtree(void*, void*);
extern void* IntStringTree_dtor(void*, int);
extern unsigned int __vt__11CGolfPlayer[];

extern void* InstanceTable_Get();
extern void InstanceTable_ClearCachedResources(void*);
extern void CTailEffect_dtor(void*, int);
extern void __destroy_new_array(void*, void*);
extern void PlayerModel_Dtor();
extern void* PlayerAnimationState_dtor(void*, int);
extern void SoundManager_ReleaseSoundBank(void*, void*);
extern void __dla__FPv(void*);
extern void* g_soundManager;

struct CleanupEntry { unsigned int flags; char reserved04[4]; void* heapName; };
struct ICleanup {
    virtual void destroy(int);
};
struct CleanupList { int count; ListNode sentinel; };
struct CleanupObject {
    char prefix[0x2C0]; int cleanupCount; ListNode cleanupSentinel;
    char reserved2CC[0x100]; CleanupEntry* entries; int entryCount;
};

#pragma use_lmw_stmw on
// Release player models, animation states, tail effects, loaded sound banks and pending-load resources.
void CGolfPlayer_OnDestroy(CleanupObject* self) {
    InstanceTable_ClearCachedResources(InstanceTable_Get());
    ListNode* node = self->cleanupSentinel.next;
    ListNode* sentinel = &self->cleanupSentinel;
    while (node != sentinel) {
        PlayerCleanupItem* item = ((PlayerCleanupNode*)node)->item;
        if (item != 0) {
            if (item != 0) {
                void* allocation = item->allocation;
                if (allocation != 0) {
                    __dl__FPv(allocation);
                    item->allocation = 0;
                }
                __dl__FPv(item);
            }
            *(void**)((char*)node + 8) = 0;
        }
        node = node->next;
    }
    ListNode* sentinel2 = &self->cleanupSentinel;
    ListNode* node2 = self->cleanupSentinel.next;
    if (node2 != sentinel2) {
        ListNode* last = sentinel2->prev;
        node2->prev->next = last->next;
        last->next->prev = node2->prev;
        while (node2 != sentinel2) {
            ListNode* cur = node2;
            node2 = node2->next;
            appFreeUni__FPv(cur);
            --self->cleanupCount;
        }
    }
    if (AsCGolfPlayer(self)->tailEffects[0] != 0) { CTailEffect_dtor(AsCGolfPlayer(self)->tailEffects[0], 1); AsCGolfPlayer(self)->tailEffects[0] = 0; }
    if (AsCGolfPlayer(self)->tailEffects[1] != 0) { CTailEffect_dtor(AsCGolfPlayer(self)->tailEffects[1], 1); AsCGolfPlayer(self)->tailEffects[1] = 0; }
    if (AsCGolfPlayer(self)->tailEffects[2] != 0) { CTailEffect_dtor(AsCGolfPlayer(self)->tailEffects[2], 1); AsCGolfPlayer(self)->tailEffects[2] = 0; }
    if (AsCGolfPlayer(self)->tailEffects[3] != 0) { CTailEffect_dtor(AsCGolfPlayer(self)->tailEffects[3], 1); AsCGolfPlayer(self)->tailEffects[3] = 0; }
    if (AsCGolfPlayer(self)->playerModels != 0) {
        __destroy_new_array(AsCGolfPlayer(self)->playerModels, (void*)PlayerModel_Dtor);
        AsCGolfPlayer(self)->playerModels = 0;
    }
    if (AsCGolfPlayer(self)->animationStates != 0) {
        __destroy_new_array(AsCGolfPlayer(self)->animationStates, (void*)PlayerAnimationState_dtor);
        AsCGolfPlayer(self)->animationStates = 0;
    }
    if (AsCGolfPlayer(self)->playerStates != 0) {
        if (AsCGolfPlayer(self)->playerStates != 0) {
            __dla__FPv((char*)AsCGolfPlayer(self)->playerStates - 0x10);
        }
        AsCGolfPlayer(self)->playerStates = 0;
    }
    int count;
    CleanupEntry* entry = self->entries;
    CleanupEntry* end;
    while (entry != (end = self->entries + self->entryCount)) {
        void* bankName;
        if ((entry->flags >> kHeapStringFlagShift) == 0)
            bankName = (void*)((char*)entry + 1);
        else
            bankName = entry->heapName;
        SoundManager_ReleaseSoundBank(g_soundManager, bankName);
        ++entry;
    }
    count = self->entryCount;
    self->entryCount = self->entryCount - count;
    while (count != 0) {
        if (--end) {
            if ((end->flags >> kHeapStringFlagShift) != 0)
                appFreeUni__FPv(end->heapName);
        }
        --count;
    }
    if (AsCGolfPlayer(self)->avatarPreloader != 0) {
        if (AsCGolfPlayer(self)->avatarPreloader != 0) {
            ((ICleanup*)AsCGolfPlayer(self)->avatarPreloader)->destroy(1);
        }
        AsCGolfPlayer(self)->avatarPreloader = 0;
    }
}
#pragma use_lmw_stmw reset

extern void* s_golfDocTypeInfo;
// Return the golf-document runtime type descriptor.
void* CGolfDoc_GetRTTI() {
    return &s_golfDocTypeInfo;
}

struct OwnedArrayHeader {
    void* object;
    volatile int count;
};

// Free the owned buffer and optionally release its wrapper.
void* dtor_8013A280(OwnedArrayHeader* self, int deleting) {
    if (self != 0) {
        if (self->object != 0) {
            self->count -= self->count;
            appFreeUni__FPv(self->object);
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

// Reset the buffer count, free its storage and optionally release its wrapper.
void* dtor_8013A2F0(OwnedArrayHeader* self, int deleting) {
    if (self != 0) {
        self->count = 0;
        if (self != 0 && self->object != 0) {
            self->count = 0;
            appFreeUni__FPv(self->object);
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

// Free this owned-buffer specialization and optionally release the wrapper.
void* dtor_8013AA48(OwnedArrayHeader* self, int deleting) {
    return ::std::__vector_deleting_dtor((Elem_8013AA48Vector*)self, deleting);
}

struct EntryListRecord { char bytes[0xA8]; };
struct EntryList {
    void* entries;
    unsigned int count;
    char reserved08[0x10];
    int fallback;
};

// Return the indexed fixed-size record, substituting the configured fallback for an invalid index.
void* EntryList_GetEntryOrFallback(EntryList* self, int index) {
    if (index < 0 || (unsigned int)index >= self->count)
        index = self->fallback;
    return (EntryListRecord*)self->entries + index;
}

typedef CGolfPlayer MainObject;
extern MainObject* CGolfPlayer_Ctor(MainObject*);
struct CleanupNode { CleanupNode* left; CleanupNode* right; void* parent; unsigned int key; unsigned int flags; char pad14[4]; void* owned; };
struct GolfPlayerStorage {
    char storage[0x3E8];
    // Construct a player within the newly allocated full-size object storage.
    GolfPlayerStorage() { CGolfPlayer_Ctor((MainObject*)this); }
};

#pragma opt_propagation off
#pragma opt_lifetimes on
#pragma opt_strength_reduction off
#pragma opt_common_subs off
#pragma use_lmw_stmw on
#pragma push
#pragma always_inline on
// Destroy deeper pending-load descendants, the heap-backed name, and this frontier node.
static inline void DestroyPlayerPendingLoadFrontier(MainObject* owner, CleanupNode* node) {
    if (node->left != 0) IntStringTree_DestroySubtree((char*)&AsCGolfPlayer(owner)->pendingModelLoadCount, node->left);
    if (node->right != 0) IntStringTree_DestroySubtree((char*)&AsCGolfPlayer(owner)->pendingModelLoadCount, node->right);
    if (&node->key && &node->flags && ((node->flags >> kHeapStringFlagShift) != 0)) appFreeUni__FPv(node->owned);
    appFreeUni__FPv(node);
}
#pragma pop

// Initialize player resources, lighting, pending-load containers and four identity channel transforms.
MainObject* CGolfPlayer_Ctor(MainObject* self) {
    // Phase 1: establish base interfaces and empty resource owners. No model,
    // animation, tail effect or sound-name storage is owned at this point.
    __ct__18NativeBase80201D18Fv(self);
    self->vtbl = (void*)__vt__11CGolfPlayer;
    self->sub_vtbl = __vt__11CGolfPlayer + kPlayerSecondaryVtableIndex;
    self->attachedSprayIds = 0; self->attachedSprayCount = 0; self->attachedSprayCapacity = 0; self->attachedSequenceIds = 0;
    self->attachedSequenceCount = 0; self->attachedSequenceCapacity = 0; self->clubPuppet = 0; self->playerModels = 0; self->animationStates = 0;
    AsCGolfPlayer(self)->tailEffects[0] = 0;
    AsCGolfPlayer(self)->tailEffects[1] = 0;
    AsCGolfPlayer(self)->tailEffects[2] = 0;
    AsCGolfPlayer(self)->tailEffects[3] = 0;
    self->lightType = 0; self->lightColor = 0; self->ambientLightColor = 0; self->cleanupListCount = 0; self->playerStates = 0;
    // +0x284 starts as packed white RGB; its later lighting role is unresolved.
    self->unk_284 = kWhitePlayerLightRgb;
    self->lightDirection.z = 0.0f; self->lightDirection.y = 0.0f; self->lightDirection.x = 0.0f;
    self->attachmentPositionLocked = 0;
    self->cleanupListPrev = self->cleanupListNext = &self->cleanupListPrev;
    self->readyForQueuedAnimation = 1; self->playerIndexOffset = 0;
    self->processingSuspended = self->avatarPreloadPending = 0; self->preloadPlayerIndex = -1; self->avatarPreloader = 0;
    self->loadedSoundNames = 0; self->loadedSoundNameCapacity = 0; self->pendingModelLoadCount = self->pendingModelLoadRoot = 0;
    self->firstPendingModelLoad = (char*)&AsCGolfPlayer(self)->pendingModelLoadRoot;
    // Preserve unknown runtime defaults: bytes +2CD/+2E0=0, float +2D0=0,
    // words +2D8/+2DC/+2E4=0. These writes do not establish behavioral names.
    self->unk_2CD = 0; self->unk_2D0 = 0.0f; self->unk_2D8 = self->unk_2DC = 0;
    self->singlePowerShotReady = self->doublePowerShotDisabled = 0; self->unk_2E4 = 0;
    self->loadedSoundNameCount = 0;
    // Phase 2: compiler-shaped empty-container cleanup. The sound-name range has
    // zero elements, so this loop cannot execute; its slots/lifetimes preserve ABI.
    {
        int _loopCount = 0;
        CleanupNode* _loopPtr = (CleanupNode*)(_loopCount * 0xc);
        int _loopI = _loopCount;
        while (_loopI != 0) {
            _loopPtr = (CleanupNode*)((char*)_loopPtr - 0xc);
            if (_loopPtr != 0) {
                if ((_loopPtr->flags >> kHeapStringFlagShift) != 0) {
                    appFreeUni__FPv(_loopPtr->owned);
                }
            }
            _loopI--;
        }
    }
    {
        // The pending-load tree was also zeroed above. Retain its postorder cleanup
        // expansion; on exit count/root are zero and first is the embedded sentinel.
        register CleanupNode* root = (*(CleanupNode**)&AsCGolfPlayer(self)->pendingModelLoadRoot);
        if (root != 0) {
            register CleanupNode* n0 = root->left;
            if (n0 != 0) {
                register CleanupNode* n1 = n0->left;
                if (n1 != 0) {
                    register CleanupNode* n2 = n1->left;
                    if (n2 != 0) {
                        DestroyPlayerPendingLoadFrontier(self, n2);
                    }
                    n2 = (CleanupNode*)n1->right;
                    if (n2 != 0) {
                        DestroyPlayerPendingLoadFrontier(self, n2);
                    }
                    if (&n1->key && &n1->flags && ((n1->flags >> kHeapStringFlagShift) != 0)) appFreeUni__FPv(n1->owned);
                    appFreeUni__FPv(n1);
                }
                n1 = n0->right;
                if (n1 != 0) {
                    register CleanupNode* n2 = n1->left;
                    if (n2 != 0) {
                        DestroyPlayerPendingLoadFrontier(self, n2);
                    }
                    n2 = (CleanupNode*)n1->right;
                    if (n2 != 0) {
                        DestroyPlayerPendingLoadFrontier(self, n2);
                    }
                    if (&n1->key && &n1->flags && ((n1->flags >> kHeapStringFlagShift) != 0)) appFreeUni__FPv(n1->owned);
                    appFreeUni__FPv(n1);
                }
                if (&n0->key && &n0->flags && ((n0->flags >> kHeapStringFlagShift) != 0)) appFreeUni__FPv(n0->owned);
                appFreeUni__FPv(n0);
            }
            n0 = *(CleanupNode**)((char*)root + 4);
            if (n0 != 0) {
                register CleanupNode* n1 = n0->left;
                if (n1 != 0) {
                    register CleanupNode* n2 = n1->left;
                    if (n2 != 0) {
                        DestroyPlayerPendingLoadFrontier(self, n2);
                    }
                    n2 = (CleanupNode*)n1->right;
                    if (n2 != 0) {
                        DestroyPlayerPendingLoadFrontier(self, n2);
                    }
                    if (&n1->key && &n1->flags && ((n1->flags >> kHeapStringFlagShift) != 0)) appFreeUni__FPv(n1->owned);
                    appFreeUni__FPv(n1);
                }
                n1 = n0->right;
                if (n1 != 0) {
                    register CleanupNode* n2 = n1->left;
                    if (n2 != 0) {
                        DestroyPlayerPendingLoadFrontier(self, n2);
                    }
                    n2 = (CleanupNode*)n1->right;
                    if (n2 != 0) {
                        DestroyPlayerPendingLoadFrontier(self, n2);
                    }
                    if (&n1->key && &n1->flags && ((n1->flags >> kHeapStringFlagShift) != 0)) appFreeUni__FPv(n1->owned);
                    appFreeUni__FPv(n1);
                }
                if (&n0->key && &n0->flags && ((n0->flags >> kHeapStringFlagShift) != 0)) appFreeUni__FPv(n0->owned);
                appFreeUni__FPv(n0);
            }
            if (&root->key && &root->flags && ((root->flags >> kHeapStringFlagShift) != 0)) appFreeUni__FPv(root->owned);
            appFreeUni__FPv(root);
        }
        AsCGolfPlayer(self)->pendingModelLoadCount = 0;
        AsCGolfPlayer(self)->pendingModelLoadRoot = 0;
        AsCGolfPlayer(self)->firstPendingModelLoad = (char*)&AsCGolfPlayer(self)->pendingModelLoadRoot;
    }
    // Reset the four breath-effect 3x4 affine matrices (CGolfPlayer::slots_2E8) to identity.
    for (unsigned char i = 0; i < 4; ++i) Matrix_SetIdentityAffine(((float (*)[12])AsCGolfPlayer(self)->slots_2E8)[i]);
    return self;
}
#pragma opt_propagation reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma opt_common_subs reset
#pragma use_lmw_stmw reset

// Allocate and construct one player object.
void* CGolfPlayer_Create() {
    return new GolfPlayerStorage;
}

// Release the empty wrapper when called as a deleting destructor.
void* dtor_8013AA08(void* self, int deleting) {
    if (self != 0 && deleting > 0)
        __dl__FPv(self);
    return self;
}

// Destructor of a 3-word vector wrapper {data, size, capacity}: clears the size, frees the data array
// (not the elements) and deletes the wrapper when deleting > 0. The nested identical `self != 0` tests
// are the inlined base-class destructor chain; std::vector from MSL/vector.h does not reproduce this
// exact form (tried size-reset forms 0, 2, 3), so the hand form stays.
void* dtor_8013AAC8(OwnedArrayHeader* self, int deleting) {
    if (self != 0) {
        if (self != 0) {
            if (self != 0) {
                if (self != 0) {
                    if (self != 0) {
                    if (self->object != 0) {
                        int count = self->count;
                        self->count = count - count;
                        appFreeUni__FPv(self->object);
                    }
                    }
                }
            }
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

struct ListWithCount {
    int count;
    ListNode sentinel;
};

// Destructor of a counted circular list {count, sentinel}: unlinks every node, frees each one, then deletes
// the wrapper when deleting > 0. The repeated `if (self)` tests are the inlined base-class destructor
// chain (std::list from MSL/list.h gives 92.7% here, so the hand form stays).
void* ListWithCount_Clear(ListWithCount* self, int deleting) {
    if (self) {
        if (self) {
            if (self) {
                if (self) {
                    if (self->sentinel.next != &self->sentinel) {
                        ListNode* sentinel = &self->sentinel;
                        ListNode* savedNext = self->sentinel.next;
                        ListNode* node = savedNext;
                        ListNode* savedPrev = sentinel->prev;
                        ListNode* last = savedPrev;
                        node->prev->next = last->next;
                        last->next->prev = node->prev;
                        while (node != sentinel) {
                            ListNode* next = node->next;
                            appFreeUni__FPv(node);
                            self->count--;
                            node = next;
                        }
                    }
                }
            }
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

// Release an animation-state element when called as a deleting destructor.
void* PlayerAnimationState_dtor(void* self, int deleting) {
    if (self != 0 && deleting > 0)
        __dl__FPv(self);
    return self;
}

struct CleanupTreeNode { CleanupTreeNode* left; CleanupTreeNode* right; };
#pragma use_lmw_stmw on
// Free all nodes in this plain binary subtree without destroying payload objects.
void PlayerPlainTree_DestroySubtree(void* self, CleanupTreeNode* node) {
    CleanupTreeNode* a = node->left;
    CleanupTreeNode* b;
    CleanupTreeNode* c;
    if (a != 0) {
        b = a->left;
        if (b != 0) {
            c = b->left;
            if (c != 0) {
                if (c->left != 0) PlayerPlainTree_DestroySubtree(self, c->left);
                if (c->right != 0) PlayerPlainTree_DestroySubtree(self, c->right);
                appFreeUni__FPv(c);
            }
            c = b->right;
            if (c != 0) {
                if (c->left != 0) PlayerPlainTree_DestroySubtree(self, c->left);
                if (c->right != 0) PlayerPlainTree_DestroySubtree(self, c->right);
                appFreeUni__FPv(c);
            }
            appFreeUni__FPv(b);
        }
        c = a->right;
        if (c != 0) {
            b = c->left;
            if (b != 0) {
                if (b->left != 0) PlayerPlainTree_DestroySubtree(self, b->left);
                if (b->right != 0) PlayerPlainTree_DestroySubtree(self, b->right);
                appFreeUni__FPv(b);
            }
            b = c->right;
            if (b != 0) {
                if (b->left != 0) PlayerPlainTree_DestroySubtree(self, b->left);
                if (b->right != 0) PlayerPlainTree_DestroySubtree(self, b->right);
                appFreeUni__FPv(b);
            }
            appFreeUni__FPv(c);
        }
        appFreeUni__FPv(a);
    }
    c = node->right;
    if (c != 0) {
        b = c->left;
        if (b != 0) {
            a = b->left;
            if (a != 0) {
                if (a->left != 0) PlayerPlainTree_DestroySubtree(self, a->left);
                if (a->right != 0) PlayerPlainTree_DestroySubtree(self, a->right);
                appFreeUni__FPv(a);
            }
            a = b->right;
            if (a != 0) {
                if (a->left != 0) PlayerPlainTree_DestroySubtree(self, a->left);
                if (a->right != 0) PlayerPlainTree_DestroySubtree(self, a->right);
                appFreeUni__FPv(a);
            }
            appFreeUni__FPv(b);
        }
        b = c->right;
        if (b != 0) {
            a = b->left;
            if (a != 0) {
                if (a->left != 0) PlayerPlainTree_DestroySubtree(self, a->left);
                if (a->right != 0) PlayerPlainTree_DestroySubtree(self, a->right);
                appFreeUni__FPv(a);
            }
            a = b->right;
            if (a != 0) {
                if (a->left != 0) PlayerPlainTree_DestroySubtree(self, a->left);
                if (a->right != 0) PlayerPlainTree_DestroySubtree(self, a->right);
                appFreeUni__FPv(a);
            }
            appFreeUni__FPv(b);
        }
        appFreeUni__FPv(c);
    }
    appFreeUni__FPv(node);
}
#pragma use_lmw_stmw reset
void PlayerPlainTree_DestroyRootAndChildren(void*, void*);
#pragma push
#pragma always_inline on
// Recursively free any deeper descendants and then this payload-free frontier node.
static inline void DestroyPlayerPlainFrontier(void* owner, CleanupTreeNode* node) {
    if (node->left != 0) PlayerPlainTree_DestroyRootAndChildren(owner, node->left);
    if (node->right != 0) PlayerPlainTree_DestroyRootAndChildren(owner, node->right);
    appFreeUni__FPv(node);
}
#pragma pop

// Free both child subtrees and the supplied root node without destroying payload objects.
void PlayerPlainTree_DestroyRootAndChildren(void* self, void* root) {
    CleanupTreeNode* node = *(CleanupTreeNode**)root;
    CleanupTreeNode* grand;
    CleanupTreeNode* child;
    if (node != 0) {
        child = node->left;
        if (child != 0) {
            grand = child->left;
            if (grand != 0) {
                DestroyPlayerPlainFrontier(self, grand);
            }
            CleanupTreeNode* savedRight = child->right;
            grand = savedRight;
            if (grand != 0) {
                DestroyPlayerPlainFrontier(self, grand);
            }
            appFreeUni__FPv(child);
        }
        child = node->right;
        if (child != 0) {
            grand = child->left;
            if (grand != 0) {
                DestroyPlayerPlainFrontier(self, grand);
            }
            grand = child->right;
            if (grand) {
                DestroyPlayerPlainFrontier(self, grand);
            }
            appFreeUni__FPv(child);
        }
        appFreeUni__FPv(node);
    }
    node = ((CleanupTreeNode**)root)[1];
    if (0 != node) {
        child = node->left;
        if (child != 0) {
            grand = child->left;
            if (grand != 0) {
                DestroyPlayerPlainFrontier(self, grand);
            }
            grand = child->right;
            if (grand != 0) {
                DestroyPlayerPlainFrontier(self, grand);
            }
            appFreeUni__FPv(child);
        }
        child = node->right;
        if (child != 0) {
            // Preserve these temporary lifetimes: shortening the chain changes register allocation.
            CleanupTreeNode* savedLeft = child->left;
            CleanupTreeNode* leftBeforeCleanup = child->left;
            CleanupTreeNode* leftCopy = leftBeforeCleanup;
            CleanupTreeNode* reloadedLeft = child->left;
            CleanupTreeNode* lifetimeCopy = leftCopy;
            CleanupTreeNode* cleanupChild = child->left;
            grand = cleanupChild;
            if (grand != 0) {
                DestroyPlayerPlainFrontier(self, grand);
            }
            grand = child->right;
            if (grand != 0) {
                DestroyPlayerPlainFrontier(self, grand);
            }
            appFreeUni__FPv(child);
        }
        appFreeUni__FPv(node);
    }
    appFreeUni__FPv(root);
}

struct SmallString { unsigned char size; char data[11]; };
struct String { unsigned int flags; unsigned int size; unsigned int heapDataOrInlineBytes; };
extern unsigned int String_ReserveCapacity(String*, unsigned int);
extern void String_ReplaceRange(String*, int, int, const char*, const char*, unsigned char*);
extern int memcmp(const char*, const char*, unsigned int);
extern unsigned int StringTree_FindInsertionPosition(void*, String*, void**, unsigned char*, unsigned char*);
extern void* NamedObjectMap_InsertNode(void*, void*, unsigned char, unsigned char, String*);
extern void appFreeUni__FPv(void*);
extern unsigned int g_defaultNamedObjectValue;
extern unsigned int strlen(const char*);

// Small-string object used by the name lookup path: flags at +0, inline buffer at +1,
// heap length at +4 and heap data pointer at +8 (0xc bytes total).
struct GStr {
    union {
        unsigned int flags;
        struct { unsigned char size; char inlinePrefix[3]; unsigned int len; void* ptr; } h;
    } u;
};
// Map entry: a three-word string key followed by the mapped object at +0xc.
struct GStrB {
    unsigned int flags;
    unsigned int len;
    void* ptr;
    void* mappedValue;
};

// Tree payload: three links followed by a small-string key and its mapped object.
struct NamedObjectNode {
    NamedObjectNode* left;
    NamedObjectNode* right;
    NamedObjectNode* parent;
    GStr key;
    void* mappedObject;
};
struct NamedObjectTable {
    char prefix[0x0C];
    unsigned int count;
    NamedObjectNode* root;
    NamedObjectNode* first;
    void* fallback;
};

#pragma opt_common_subs off
#pragma use_lmw_stmw on
struct EmptyGStr { unsigned char byte; };
// Read the inline or heap string length according to the ownership flag.
static inline unsigned int StringKey_GetLength(const GStr* s) {
    unsigned int result;
    if ((s->u.flags >> kHeapStringFlagShift) == 0)
        result = s->u.h.size & kInlineStringLengthMask;
    else
        result = s->u.h.len;
    return result;
}
// Choose the shared-prefix comparison length, preserving the reference return.
static inline const unsigned int& StringKey_MinLength(const unsigned int& a, const unsigned int& b) {
    return a < b ? a : b;
}
// Copy inline bytes directly, or reserve and duplicate heap-backed text.
static inline void StringKey_Copy(GStr* dest, const GStr* arg) {
    if (!(arg->u.flags >> kHeapStringFlagShift)) {
        dest->u.flags = arg->u.flags;
        dest->u.h.len = arg->u.h.len;
        dest->u.h.ptr = arg->u.h.ptr;
    } else {
        dest->u.flags = 0;
        dest->u.h.len = 0;
        dest->u.h.ptr = 0;
        String_ReserveCapacity((String*)dest, arg->u.h.len);
        EmptyGStr allocator;
        EmptyGStr copy = allocator;
        String_ReplaceRange((String*)dest, 0, 0, (const char*)arg->u.h.ptr,
                    (const char*)arg->u.h.ptr + arg->u.h.len, (unsigned char*)&copy);
    }
}
// Assign a byte range using the original string allocator ABI.
static inline void StringKey_AssignRange(GStr* dest, const char* first, const char* last) {
    EmptyGStr allocator;
    EmptyGStr copy = allocator;
    String_ReplaceRange((String*)dest, 0, 0, first, last, (unsigned char*)&copy);
}
// Compare the stored key with a byte sequence, including a length tie-break.
static inline int StringKey_CompareToBytes(unsigned int queryLength, unsigned int queryBytes, const GStr* found) {
    unsigned int storedLength = StringKey_GetLength(found);
    unsigned int storedBytes;
    unsigned int storedLengthAgain;
    if (!(found->u.flags >> kHeapStringFlagShift)) {
        storedBytes = (unsigned int)((char*)found + 1);
        storedLengthAgain = found->u.h.size & kInlineStringLengthMask;
    } else {
        storedBytes = (unsigned int)found->u.h.ptr;
        storedLengthAgain = found->u.h.len;
    }
    unsigned int compareLength = storedLengthAgain;
    unsigned int commonLength = StringKey_MinLength(storedLength, compareLength);
    int comparison = memcmp((const char*)storedBytes, (const char*)queryBytes, (unsigned int)StringKey_MinLength(queryLength, commonLength));
    int decision;
    if (comparison == 0) {
        if (commonLength < queryLength)
            decision = -1;
        else if (commonLength == queryLength)
            decision = 0;
        else
            decision = 1;
    } else {
        decision = comparison;
    }
    return decision;
}

#pragma opt_propagation off
#pragma opt_lifetimes on
#pragma opt_strength_reduction on
#pragma push
#pragma always_inline on
// Borrow a key's bytes and length; address uses the original 32-bit comparison ABI.
static inline void ReadNamedObjectKeySpan(const GStr* key, unsigned int& byteCount, unsigned int& address) {
    if (!(key->u.flags >> kHeapStringFlagShift)) {
        byteCount = key->u.h.size & kInlineStringLengthMask;
        address = (unsigned int)((char*)key + 1);
    } else {
        address = (unsigned int)key->u.h.ptr;
        byteCount = key->u.h.len;
    }
}
#pragma pop

// Look up a named entry and return its mapped object, or the configured fallback when absent.
// name is nonnull, terminated narrow text. Result is borrowed; this call neither
// retains the mapped object nor transfers ownership of it to the caller.
extern void* NamedObjectTable_FindOrFallback(void* self, const char* name) {
    struct StringCleanup {
        void* object;
        // Release a temporary lookup/insertion key only if it owns heap storage.
        ~StringCleanup() {
            unsigned int* words = (unsigned int*)object;
            if (words[0] >> kHeapStringFlagShift) appFreeUni__FPv(*(void**)(words + 2));
        }
    };
    // Phase 1: own a temporary copy of the query, then find the first stored key
    // >= it. Representation branches select bytes/length; ordering is lexical.
    GStr key;
    key.u.flags = 0;
    key.u.h.len = 0;
    key.u.h.ptr = 0;
    unsigned int nameLength = strlen(name);
    String_ReserveCapacity((String*)&key, nameLength);
    StringKey_AssignRange(&key, name, name + nameLength);
    NamedObjectNode* node = ((NamedObjectTable*)self)->root;
    void* keyHeapData = key.u.h.ptr;
    unsigned int keyHeapLength = key.u.h.len;
    unsigned int keyUsesHeap = key.u.flags >> kHeapStringFlagShift;
    unsigned int keyInlineLength = key.u.h.size & kInlineStringLengthMask;
    void* found = &((NamedObjectTable*)self)->root;
    unsigned int isInlineKey = (keyUsesHeap == 0);
    bool useHeapKey = (isInlineKey == 0);
    unsigned int x;
    const char* xbp;
    while (node != 0) {
        if (useHeapKey == 0) {
            x = keyInlineLength;
            xbp = (const char*)&key + 1;
        } else {
            xbp = (const char*)keyHeapData;
            x = keyHeapLength;
        }
        if (StringKey_CompareToBytes(x, (unsigned int)xbp, &node->key) >= 0) {
            found = node;
            node = node->left;
        } else {
            node = node->right;
        }
    }
    // Phase 2: reject a non-equal lower-bound candidate, free the first query copy,
    // and return the configured fallback immediately on absence.
    if (found != &((NamedObjectTable*)self)->root) {
        GStr* foundKey = &((NamedObjectNode*)found)->key;
        unsigned int x;
        unsigned int xb;
        ReadNamedObjectKeySpan(foundKey, x, xb);
        if (StringKey_CompareToBytes(x, xb, &key) < 0)
            found = &((NamedObjectTable*)self)->root;
    }
    if (keyUsesHeap)
        appFreeUni__FPv(key.u.h.ptr);
    if (found == &((NamedObjectTable*)self)->root)
        return ((NamedObjectTable*)self)->fallback;

    // Phase 3: repeat through the map's get-or-insert interface on the known hit.
    // Its defensive insert branch copies the key and uses the global default;
    // temporaries have independent heap cleanup even when no insertion occurs.
    GStr value;
    value.u.flags = 0;
    value.u.h.len = 0;
    value.u.h.ptr = 0;
    unsigned int valueNameLength = strlen(name);
    String_ReserveCapacity((String*)&value, valueNameLength);
    StringKey_AssignRange(&value, name, name + valueNameLength);
    StringCleanup valueCleanup = {&value};
    name = 0;
    void* insertionPosition;
    void* entry;
    unsigned char insertSide;
    unsigned char becomesFirst;
    GStr* slot;
    insertionPosition = (void*)StringTree_FindInsertionPosition((String*)&((NamedObjectTable*)self)->count, (String*)&value, &entry, &insertSide, &becomesFirst);
    // Retain the shared value cleanup after lookup or insertion.
    do {
        if (entry != 0) {
            slot = &((NamedObjectNode*)entry)->key;
            unsigned int x;
            unsigned int xb;
            ReadNamedObjectKeySpan(&value, x, xb);
            if (StringKey_CompareToBytes(x, xb, slot) >= 0)
                break;
        }
        {
            GStrB copy;
            GStr* copyKey = (GStr*)&copy;
            StringKey_Copy(copyKey, &value);
            StringCleanup copyCleanup = {&copy};
            copy.mappedValue = (void*)g_defaultNamedObjectValue;
            slot = &((NamedObjectNode*)NamedObjectMap_InsertNode(&((NamedObjectTable*)self)->count, insertionPosition, insertSide, becomesFirst, (String*)&copy))->key;
        }
    } while (0);
    {
        void* result = ((GStrB*)slot)->mappedValue;
        return result;
    }
}
#pragma opt_propagation reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma opt_common_subs reset
#pragma use_lmw_stmw reset

extern unsigned char s_iObjectTypeInfo;
extern unsigned char golfParameterXmlStrings[];
extern float s_golfDocFactoryFullTurnAngle; // Cached full-turn angle initialized with the golf-document factory registration.
extern const float kFloat_6_2831855;
extern const char s_golfDocFactoryTypeName[];
extern void IntPair_Set(void*, const char*, void*);
extern void* ActorFactoryRegistry_GetInstance();
extern void StringMap_SetByName(void*, void*, const char*);
extern void* CGolfDoc_Create();

// Register the golf-document runtime type and its factory callback.
void CGolfDoc_RegisterType() {
    s_golfDocFactoryFullTurnAngle = kFloat_6_2831855;
    enum { kGolfDocumentTypeNameOffset = 0x1A1 };
    IntPair_Set(&s_golfDocTypeInfo, (const char*)golfParameterXmlStrings + kGolfDocumentTypeNameOffset, &s_iObjectTypeInfo);
    void* state = ActorFactoryRegistry_GetInstance();
    StringMap_SetByName(state, (void*)CGolfDoc_Create, s_golfDocFactoryTypeName);
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462128", _ctors_u801398B0_0, CGolfDoc_RegisterType);
#pragma pop
