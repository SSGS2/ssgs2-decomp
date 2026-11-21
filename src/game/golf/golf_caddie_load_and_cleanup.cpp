#pragma ipa file
#include "iff/IffStruct.h"
// Address-only layout of the existing string pool; no new string storage.
struct GolfCaddieStringPool {
    char caddieResourceName[11]; // 0x0
    char storyCaddieFormat[10]; // 0xB
    char storyConteMessage[11]; // 0x15
    char shadowName[7]; // 0x20
    char shadowLightName[4]; // 0x27
};

#include "classes/MWStringStorage.h"
#include "classes/RenderPacketFields.h"
#include "classes/CGolfDoc.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#pragma auto_inline off
// Translation unit 0x801270B8-0x80128808 (20 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
#include "sdk_protos.h"
#include "classes/CGolfCaddie.h"

#define AsCGolfCaddie(p) ((CGolfCaddie*)(p))
enum { kCaddieResourceType = 4, kPreloadRequestFlags = 8, kStoryGameMode = 6,
       kTutorialCaddieId = 0x1C000001, kPlayerEntryDisabled = 3, kTutorialCaddieStageLimit = 6 };
// Byte offsets into the exported vtable identify its embedded secondary interface.
enum { kCaddieSecondaryVtableOffset = 0x3C };
// A strided view of each shared player setup, used only for caddie equipment.
struct SharedCaddieSelection {
    char padding[0x1B4];
    int caddieInventoryIndex;
    unsigned int caddieTypeId;
    unsigned int caddieItemTypeId;
    char trailingPadding[0x14];
};
struct GolfPlayerEntryState { char padding[8]; int state; };
struct PtrArray4 { unsigned int *data; unsigned int count; };
struct PtrArray12 { unsigned int *data; unsigned int count; };

extern "C" void __dl__FPv(void *);
extern "C" void ResourcePreloadQueue_AddRequest(void *, int, int, float, void *, float, const char *, int, int, int);
extern "C" void *g_sceneManager;
extern "C" const char golfCaddieStrings[];
extern "C" float kFloat_1_GolfCaddie;
extern "C" float kFloat_0_GolfCaddie;
extern "C" float kFloat_0_09_CaddieShadow;
extern "C" void *FxInterpreter_GetInstance();
extern "C" void FxInterpreter_ClearEntries(void *);
extern "C" void *CaddieEntry_DestroyDialogueMaps(void *, int); // CaddieEntry_DestroyDialogueMaps.
extern "C" void SoundManager_ReleaseSoundBank(void *, void *);
extern "C" void *MWString_GetCString(void *);
extern "C" void StringArray_ClearElements(void *);
extern "C" void CaddieIntStringTree_Clear(void *);
extern "C" void IntStringTree_DestroySubtree(void *, void *);
extern "C" void CaddieDialogueStringMap_DestroySubtree(void *, void *);
extern "C" void CaddieDialogueStringPair_Destroy(void *, void *);
extern "C" void __destroy_new_array(void *, void *);
extern "C" void __dt__18NativeBase80201D18Fv(void *, int);
extern "C" char __vt__11CGolfCaddie[];
extern "C" void *CConteCaddie_DestroyStorage(void *, int); // CConteCaddie_DestroyStorage.
extern "C" void appFreeUni__FPv(void *);
extern "C" void CConteCaddie_Ctor();
extern "C" void RenderPacket_Destroy();
extern "C" unsigned int CaddieEntryIterator_PostIncrement(unsigned int *, int); // CaddieEntryIterator_PostIncrement.
extern "C" const char s_renderPacket[];
extern "C" const char s_renderPacketDtorRegistration_804F68E8[];
extern "C" void CaddieOwnedObject_DestroyAndClear(void *, unsigned int);
extern "C" void CGeneTex3D_Dtor(void *, int);
extern "C" void *__nwa__FUl(unsigned int);
extern "C" char *String_FormatTemporary(const char *, ...);
extern "C" void IActor_DispatchMessage(void *, void *, const char *, int, void *, int, int, int);
extern "C" void *GolfPlayer_GetEntryByIndex(void *, unsigned char);
extern "C" void CGolfCaddie_CreateCaddieEntry(void *, unsigned int, unsigned int, int, unsigned int);
extern "C" int RenderPacket_Init(void *);
extern "C" void CShadowManager_GetOrCreateEntry(void *, const char *, float, int, const char *, void *);
extern "C" CSharedDoc *g_sharedDoc;
extern "C" void *g_soundManager;
extern "C" signed char s_renderPacketInitialized;
extern "C" void *g_shadowManager;

extern "C" {

// A dialogue-map value contains a tag followed by its string; its key is another string.
struct DtorNode {
    DtorNode* left;
    DtorNode* right;
    void* parent;
    MWStringStorage key;
    unsigned int valueTag;
    MWStringStorage value;
};
// Tree header: size, root, reserved word, end. Empty end is &root, not a heap sentinel.
// Dialogue nodes own two strings; pending-load nodes own an integer key and one string.
// Subtree cleanup frees descendants; the caller then destroys the current payload and node.
// Caddie entry vectors own entry pointers; sound-name vectors own strings AND sound-bank
// references, released separately by OnDestroy. Destructor frees remaining container storage.
struct CaddieDialogueTree { unsigned int size; DtorNode* root; void* reserved; void* end; };
struct CaddieEffectEntry {
    char padding[0x18];
    CaddieDialogueTree firstDialogueMap;
    CaddieDialogueTree secondDialogueMap;
};

#pragma push
#pragma use_lmw_stmw on
#pragma opt_common_subs off
#pragma opt_propagation off
// Destroy both child subtrees, the node's two dialogue strings, and the node allocation.
static inline void dtorLeaf(void *array, DtorNode *node) {
    if (node == 0) return;
    if (node->left) CaddieDialogueStringMap_DestroySubtree(array, node->left);
    if (node->right) CaddieDialogueStringMap_DestroySubtree(array, node->right);
    CaddieDialogueStringPair_Destroy(array, &node->key);
    appFreeUni__FPv(node);
}

// Unroll one extra dialogue-tree level while retaining child destruction order.
static inline void dtorPair(void *array, DtorNode *node) {
    if (node == 0) return;
    dtorLeaf(array, node->left);
    dtorLeaf(array, node->right);
    CaddieDialogueStringPair_Destroy(array, &node->key);
    appFreeUni__FPv(node);
}

// Unroll two extra dialogue-tree levels before freeing the parent string pair.
static inline void dtorBranch(void *array, DtorNode *node) {
    if (node == 0) return;
    dtorPair(array, node->left);
    dtorPair(array, node->right);
    CaddieDialogueStringPair_Destroy(array, &node->key);
    appFreeUni__FPv(node);
}

// Release heap-backed strings in the root pair, then free the root itself.
static inline void dtorRootRelease(DtorNode *top) {
    if (&top->key) {
        if (&top->valueTag) {
            if (&top->value) {
                if (top->value.flags >> 31)
                    appFreeUni__FPv(top->value.data);
            }
        }
        if (&top->key) {
            if (top->key.flags >> 31)
                appFreeUni__FPv(top->key.data);
        }
    }
    appFreeUni__FPv(top);
}

// Leaf cleanup uses typed helpers. Keep these larger expansions at the call site:
// their distinct child-local lifetimes are needed for the existing register allocation.
#define DT_PAIR(A, N) do { \
    if (N) { \
        DtorNode *c = (N)->left; dtorLeaf(A, c); \
        c = (N)->right; dtorLeaf(A, c); \
        CaddieDialogueStringPair_Destroy((A), &(N)->key); \
        appFreeUni__FPv(N); \
    } \
} while (0)
#define DT_SEPARATE_PAIR(A, pair0) do { \
        if (pair0) { \
            DtorNode *c0 = pair0->left; dtorLeaf(A, c0); \
            DtorNode *c1 = pair0->right; dtorLeaf(A, c1); \
            CaddieDialogueStringPair_Destroy((A), &pair0->key); \
            appFreeUni__FPv(pair0); \
        } \
} while (0)
#define DT_TREE(A, TOP, FIRST, REST) do { \
    DtorNode *mid0 = (TOP)->left; \
    if (mid0) { \
        DtorNode *pair0 = mid0->left; FIRST(A, pair0); \
        DtorNode *pair1 = mid0->right; REST(A, pair1); \
        CaddieDialogueStringPair_Destroy((A), &mid0->key); \
        appFreeUni__FPv(mid0); \
    } \
    DtorNode *mid1 = (TOP)->right; \
    if (mid1) { \
        DtorNode *pair0 = mid1->left; REST(A, pair0); \
        DtorNode *pair1 = mid1->right; REST(A, pair1); \
        CaddieDialogueStringPair_Destroy((A), &mid1->key); \
        appFreeUni__FPv(mid1); \
    } \
} while (0)

// Release the dialogue and animation-prerequisite maps, then optionally delete the entry.
void *CaddieEntry_DestroyDialogueMaps(void *object, int deleting) {
    if (object) {
        if (&((CaddieEffectEntry*)object)->secondDialogueMap) {
        if (&((CaddieEffectEntry*)object)->secondDialogueMap) {
        if (&((CaddieEffectEntry*)object)->secondDialogueMap) {
        if (&((CaddieEffectEntry*)object)->secondDialogueMap) {
        if (&((CaddieEffectEntry*)object)->secondDialogueMap) {
        if (&((CaddieEffectEntry*)object)->secondDialogueMap) {
        DtorNode *top = ((CaddieEffectEntry*)object)->secondDialogueMap.root;
        if (top) {
            DT_TREE(&((CaddieEffectEntry*)object)->secondDialogueMap, top, DT_SEPARATE_PAIR, DT_SEPARATE_PAIR);
            dtorRootRelease(top);
        }
        } } } } } }
        if (&((CaddieEffectEntry*)object)->firstDialogueMap) {
        if (&((CaddieEffectEntry*)object)->firstDialogueMap) {
        if (&((CaddieEffectEntry*)object)->firstDialogueMap) {
        if (&((CaddieEffectEntry*)object)->firstDialogueMap) {
        if (&((CaddieEffectEntry*)object)->firstDialogueMap) {
        if (&((CaddieEffectEntry*)object)->firstDialogueMap) {
        DtorNode *second = ((CaddieEffectEntry*)object)->firstDialogueMap.root;
        if (second) {
            DT_TREE(&((CaddieEffectEntry*)object)->firstDialogueMap, second, DT_SEPARATE_PAIR, DT_PAIR);
            if (&second->key) {
                if (&second->valueTag) {
                    if (&second->value) {
                        if (second->value.flags >> 31)
                            appFreeUni__FPv(second->value.data);
                    }
                }
                if (&second->key) {
                    if (second->key.flags >> 31)
                        appFreeUni__FPv(second->key.data);
                }
            }
            appFreeUni__FPv(second);
        }
        } } } } } }
        if (deleting > 0) __dl__FPv(object);
    }
    return object;
}
#undef DT_TREE
#undef DT_PAIR
#undef DT_SEPARATE_PAIR
#pragma pop

struct CleanupNode {
    CleanupNode *left;
    CleanupNode *right;
    void* parent;
    int key;
    MWStringStorage value;
};

#define CLEAN_PAIR(SELF, N) do { \
    if ((N) != 0) { \
        CleanupNode *c = (N)->left; \
        if (c) { destroyLoadChildren((SELF), c); releaseLoadNode(c); } \
        c = (N)->right; \
        if (c) { destroyLoadChildren((SELF), c); releaseLoadNode(c); } \
        releaseLoadNode((N)); \
    } \
} while (0)

// Release descendant subtrees while leaving the current node and its string alive.
static inline void destroyLoadChildren(void* owner, CleanupNode* node) {
    if (node->left) IntStringTree_DestroySubtree(owner, (void*)node->left);
    if (node->right) IntStringTree_DestroySubtree(owner, (void*)node->right);
}
// Release the current node's heap string, when present, then free the node itself.
static inline void releaseLoadNode(CleanupNode* node) {
    if (&node->key) {
        if (&node->value) {
            if (node->value.flags >> 31) appFreeUni__FPv(node->value.data);
        }
    }
    appFreeUni__FPv(node);
}

struct CaddieLoadTree { unsigned int size; CleanupNode* root; void* reserved; void* end; };
// Walk the requested inlined cleanup depth, then release remaining subtrees and this node.
static inline void cleanupNode(void *self, CleanupNode *node, int depth) {
    if (node == 0) return;
    if (depth > 1) {
        cleanupNode(self, node->left, depth - 1);
        cleanupNode(self, node->right, depth - 1);
    }
    destroyLoadChildren(self, node);
    releaseLoadNode(node);
}

#pragma push
#pragma use_lmw_stmw on
#pragma opt_common_subs off
#pragma opt_propagation off
// Destroy queued model-load entries and restore the empty tree root/end iterator.
void CaddieIntStringTree_Clear(void *self) {
    CleanupNode *root = ((CaddieLoadTree*)self)->root;
    if (root != 0) {
        CleanupNode *mid0 = root->left;
        if (mid0) {
            CleanupNode *pair0 = mid0->left;
            if (pair0) {
                CleanupNode *c0 = pair0->left;
                if (c0) { destroyLoadChildren(self, c0); releaseLoadNode(c0); }
                CleanupNode *c1 = pair0->right;
                if (c1) { destroyLoadChildren(self, c1); releaseLoadNode(c1); }
                releaseLoadNode(pair0);
            }
            CleanupNode *pair1 = mid0->right;
            CLEAN_PAIR(self, pair1);
            releaseLoadNode(mid0);
        }
        CleanupNode *mid1 = root->right;
        if (mid1) {
            CleanupNode *pair0 = mid1->left;
            CLEAN_PAIR(self, pair0);
            CleanupNode *pair1 = mid1->right;
            CLEAN_PAIR(self, pair1);
            releaseLoadNode(mid1);
        }
        releaseLoadNode(root);
        ((CaddieLoadTree*)self)->size = 0;
        ((CaddieLoadTree*)self)->root = 0;
        ((CaddieLoadTree*)self)->end = &((CaddieLoadTree*)self)->root;
    }
}
#pragma pop
#undef CLEAN_PAIR

#pragma push
#pragma use_lmw_stmw on
#pragma opt_common_subs off
#pragma opt_propagation off
struct CaddieResource { 
virtual ~CaddieResource();
};
// The freeLoadNode* helpers below are one unrolled postorder free of the pending model-load tree (nodes are
// {left, right, parent, key, value}); the number is how many tree levels the helper covers, "Level" nodes
// destroy their deeper children through destroyLoadChildren. In plain C: free every node, children first.
// Release a pending-load node and its child resources through the load-tree owner.
static inline void freeLoadNode1Level(void *self, CleanupNode *node) {
    if (node) {
        destroyLoadChildren(&AsCGolfCaddie(self)->pendingModelLoadCount, node);
        releaseLoadNode(node);
    }
}
// Destroy the left and right pending-load branches before releasing their parent.
static inline void freeLoadNode2Levels(void *self, CleanupNode *node) {
    if (node) {
        freeLoadNode1Level(self, node->left);
        freeLoadNode1Level(self, node->right);
        releaseLoadNode(node);
    }
}
// Expand a second level of pending-load cleanup without changing release order.
static inline void freeLoadNode3Levels(void *self, CleanupNode *node) {
    if (node) {
        freeLoadNode2Levels(self, node->left);
        freeLoadNode2Levels(self, node->right);
        releaseLoadNode(node);
    }
}
// Reload each child around destruction; cleanup can invalidate cached child pointers.
static inline void freeLoadNode2LevelsReload(CleanupNode *node, void *self) {
    CleanupNode *child0;
    if (node) {
        child0 = node->left;
        if (child0) {
            destroyLoadChildren(&AsCGolfCaddie(self)->pendingModelLoadCount, child0);
            releaseLoadNode(child0);
        }
        child0 = node->right;
        if (child0) {
            destroyLoadChildren(&AsCGolfCaddie(self)->pendingModelLoadCount, child0);
            releaseLoadNode(child0);
        }
        releaseLoadNode(node);
    }
}
// Clean the first load-tree branch using the original distinct child-reload paths.
static inline void freeLoadNode3LevelsMixed(void *self, CleanupNode *node) {
    CleanupNode *child1;
    CleanupNode *child0;
    if (node) {
        child0 = node->left;
        freeLoadNode2Levels(self, child0);
        child1 = node->right;
        freeLoadNode2LevelsReload(child1, self);
        releaseLoadNode(node);
    }
}
// Release all pending model-load nodes, preserving the compiler's unrolled traversal.
static inline void freePendingLoadTree(void *self) {
    CleanupNode *node = (CleanupNode *)AsCGolfCaddie(self)->pendingModelLoadTreeRoot;
    if (node) {
        freeLoadNode3LevelsMixed(self, node->left);
        freeLoadNode3Levels(self, node->right);
        releaseLoadNode(node);
    }
}
// Destroy loaded sound-name strings in reverse order, then free their backing array.
static inline void freeSoundBankNames(void *self) {
    void *items = AsCGolfCaddie(self)->soundBankNames;
    if (items) {
        unsigned int count = *(unsigned int *)&AsCGolfCaddie(self)->soundBankNameCount;
        char *item = (char *)items + count * (int)sizeof(MWStringStorage);
        *(unsigned int *)&AsCGolfCaddie(self)->soundBankNameCount -= count;
        while (count != 0) {
            item -= (int)sizeof(MWStringStorage);
            if (item) {
                if (AsMWStringStorage(item)->flags >> 31)
                    appFreeUni__FPv(AsMWStringStorage(item)->data);
            }
            --count;
        }
        appFreeUni__FPv(AsCGolfCaddie(self)->soundBankNames);
    }
}
// Destroy preload/story wrappers and container storage, then optionally free the actor.
// OnDestroy performs active pet/sound-bank release; this destructor is not a substitute
// for that lifecycle hook. deleting > 0 requests deletion of the actor allocation.
void *CGolfCaddie_dtor(void *self, int deleting) {
    if (self != 0) {
        void *vtable = __vt__11CGolfCaddie;
        CaddieResource *resource = (*(CaddieResource**)&AsCGolfCaddie(self)->petPreload);
        *(void **)self = vtable;
        AsCGolfCaddie(self)->secondaryVtable = (char *)vtable + kCaddieSecondaryVtableOffset;
        if (resource) {
            delete resource;
            AsCGolfCaddie(self)->petPreload = 0;
        }
        // Release the story wrapper array after the preload resource, then clean container storage.
        void *array = AsCGolfCaddie(self)->conteCaddies;
        if (array) {
            __destroy_new_array(array, (void *)CConteCaddie_DestroyStorage);
            AsCGolfCaddie(self)->conteCaddies = 0;
        }
        AsCGolfCaddie(self)->storyConteCaddieOverride = 0;
        if (&AsCGolfCaddie(self)->pendingModelLoadCount) {
        if (&AsCGolfCaddie(self)->pendingModelLoadCount) {
        if (&AsCGolfCaddie(self)->pendingModelLoadCount) {
        if (&AsCGolfCaddie(self)->pendingModelLoadCount) {
        if (&AsCGolfCaddie(self)->pendingModelLoadCount) {
        if (&AsCGolfCaddie(self)->pendingModelLoadCount) {
        freePendingLoadTree(self);
        } } } } } }
        if (&AsCGolfCaddie(self)->soundBankNames) {
        if (&AsCGolfCaddie(self)->soundBankNames) {
        if (&AsCGolfCaddie(self)->soundBankNames) {
        if (&AsCGolfCaddie(self)->soundBankNames) {
        freeSoundBankNames(self);
        } } } }
        if (&AsCGolfCaddie(self)->caddieEntries) {
        if (&AsCGolfCaddie(self)->caddieEntries) {
        if (&AsCGolfCaddie(self)->caddieEntries) {
        if (&AsCGolfCaddie(self)->caddieEntries) {
        if (&AsCGolfCaddie(self)->caddieEntries) {
        void *p = (*(void**)&AsCGolfCaddie(self)->caddieEntries);
        if (p) {
            unsigned int count = AsCGolfCaddie(self)->caddieEntryCount;
            AsCGolfCaddie(self)->caddieEntryCount -= count;
            appFreeUni__FPv(p);
        }
        } } } } }
        __dt__18NativeBase80201D18Fv(self, 0);
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}
#pragma pop

// Return the stored address of the first caddie entry.
unsigned int CaddieEntryVector_GetBegin(unsigned int *value) {
	return *value;
}

// Copy a sound-name iterator word; the third ABI argument is unused.
void CaddieSoundNameIterator_Copy(unsigned int *dest, const unsigned int *value, int unused) {
	*dest = *value;
}

// Copy a caddie-entry iterator word.
void CaddieEntryIterator_Copy(unsigned int *dest, const unsigned int *value) {
	*dest = *value;
}

// Return the caddie-entry iterator's current address.
unsigned int CaddieEntryIterator_GetPointer(unsigned int *value) {
	return *value;
}

// Return the stored address of the first sound-name record.
unsigned int CaddieSoundNameVector_GetBegin(unsigned int *value) {
	return *value;
}

// Return the sound-name iterator's current address.
unsigned int CaddieSoundNameIterator_GetPointer(unsigned int *value) {
	return *value;
}

#pragma push
#pragma opt_propagation off
#pragma opt_strength_reduction off
#pragma opt_common_subs off
// Reset the vector size without destroying or releasing its storage.
void CaddieEntryVector_ResetSize(unsigned int *value) {
	unsigned int count = value[1];
	value[1] -= count;
}
#pragma pop

// Advance a sound-name iterator by one 12-byte string record.
void CaddieSoundNameIterator_Advance(unsigned int *value) {
	*value += sizeof(MWStringStorage);
}

// Return the one-past-end entry pointer: index is the vector count in all callers here.
// The shared function/field names are provisional and kept as-is.
unsigned int *PtrArray4_GetEndPtr(struct PtrArray4 *value) {
	return value->data + value->count;
}

// Return the one-past-end string pointer: index is the vector count in all callers here.
unsigned int *PtrArray12_GetCurrentPtr(struct PtrArray12 *value) {
	return (unsigned int *)((unsigned char *)value->data + value->count * sizeof(MWStringStorage));
}

// Compare the current addresses of two sound-name iterators.
unsigned int CaddieSoundNameIterator_NotEqual(const unsigned int *a, const unsigned int *b) {
	return *a != *b;
}

// Compare the current addresses of two caddie-entry iterators.
unsigned int CaddieEntryIterator_NotEqual(const unsigned int *a, const unsigned int *b) {
	return *a != *b;
}

// Optionally deallocate the story-caddie command wrapper; it has no member cleanup here.
void *CConteCaddie_DestroyStorage(void *ptr, int deletingFlag) {
	if (ptr != 0 && deletingFlag > 0) {
		__dl__FPv(ptr);
	}
	return ptr;
}

// Queue the common caddie resource using the actor's secondary preload interface.
void CGolfCaddie_OnPreLoadInit(void *ptr) {
	void *data = ptr;
	if (data != 0) {
		data = &AsCGolfCaddie(data)->secondaryVtable;
	}
	ResourcePreloadQueue_AddRequest(g_sceneManager, kCaddieResourceType, 0, kFloat_1_GolfCaddie, data, kFloat_0_GolfCaddie, golfCaddieStrings, 0, kPreloadRequestFlags, 0);
}

#pragma push
#pragma opt_common_subs off
#pragma opt_propagation off
#pragma opt_loop_invariants off
// Release every caddie entry, sound bank, pending model request, texture, and preload allocation.
void CGolfCaddie_OnDestroy(void *self) {
	unsigned int iter;
	unsigned int it;
	FxInterpreter_ClearEntries(FxInterpreter_GetInstance());
	AsCGolfCaddie(self)->effectEntryKey = -1;
	unsigned int tmp = CaddieEntryVector_GetBegin((unsigned int *)(&AsCGolfCaddie(self)->caddieEntries)); // CaddieEntryVector_GetBegin.
	CaddieEntryIterator_Copy(&iter, &tmp); // CaddieEntryIterator_Copy.
	// Release every caddie entry, refreshing the vector end each time.
	unsigned int end;
	while ((end = (unsigned int)PtrArray4_GetEndPtr((PtrArray4 *)(&AsCGolfCaddie(self)->caddieEntries)),
	        CaddieEntryIterator_NotEqual(&iter, &end))) {
		unsigned int *p = (unsigned int *)CaddieEntryIterator_GetPointer(&iter); // CaddieEntryIterator_GetPointer.
		CaddieOwnedObject_DestroyAndClear(self, *p);
		p = (unsigned int *)CaddieEntryIterator_GetPointer(&iter);
		if (*p) {
			CaddieEntry_DestroyDialogueMaps((void *)*(unsigned int *)CaddieEntryIterator_GetPointer(&iter), 1);
			p = (unsigned int *)CaddieEntryIterator_GetPointer(&iter);
			*p = 0;
		}
		CaddieEntryIterator_PostIncrement(&iter, 0);
	}
	// Owned entries are gone: clear their logical count, then release the ring texture.
	CaddieEntryVector_ResetSize((unsigned int *)(&AsCGolfCaddie(self)->caddieEntries)); // CaddieEntryVector_ResetSize.
	void *resource = AsCGolfCaddie(self)->texture3D;
	if (resource) { CGeneTex3D_Dtor(resource, 1); AsCGolfCaddie(self)->texture3D = 0; }
	unsigned int a = CaddieSoundNameVector_GetBegin((unsigned int *)(&AsCGolfCaddie(self)->soundBankNames)); // CaddieSoundNameVector_GetBegin.
	CaddieSoundNameIterator_Copy(&it, &a, 0); // CaddieSoundNameIterator_Copy.
	unsigned int b;
	while ((b = (unsigned int)PtrArray12_GetCurrentPtr((PtrArray12 *)(&AsCGolfCaddie(self)->soundBankNames)),
	        CaddieSoundNameIterator_NotEqual(&it, &b))) {
		unsigned int v = CaddieSoundNameIterator_GetPointer(&it); // CaddieSoundNameIterator_GetPointer.
		SoundManager_ReleaseSoundBank(g_soundManager, MWString_GetCString((void *)v));
		CaddieSoundNameIterator_Advance(&it); // CaddieSoundNameIterator_Advance.
	}
	// All sound-bank references are released; destroy their names and pending-load records.
	StringArray_ClearElements(&AsCGolfCaddie(self)->soundBankNames);
	CaddieIntStringTree_Clear(&AsCGolfCaddie(self)->pendingModelLoadCount);
	CaddieResource *owned = (*(CaddieResource**)&AsCGolfCaddie(self)->petPreload);
	if (owned) { delete owned; AsCGolfCaddie(self)->petPreload = 0; }
	void *array = AsCGolfCaddie(self)->conteCaddies;
	if (array) { __destroy_new_array(array, (void *)CConteCaddie_DestroyStorage); AsCGolfCaddie(self)->conteCaddies = 0; }
	AsCGolfCaddie(self)->storyConteCaddieOverride = 0;
}
#pragma pop

struct LoadVirtual {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
    virtual void v4(); virtual void v5(); virtual void v6(); virtual void v7();
    virtual void load();
};
// Load story caddies when needed, instantiate active player caddies, and register their shadow group.
void CGolfCaddie_OnLoad(void *self) {
	((LoadVirtual *)self)->load();
	void *mgr = g_sharedDoc;
	void *state = (void *)AsCSharedDoc(mgr)->golfDoc;
	if (AsCGolfDoc(state)->tutorialStage < kTutorialCaddieStageLimit) {
		((SharedCaddieSelection*)mgr)->caddieInventoryIndex = -1;
		*(int*)&((SharedCaddieSelection*)g_sharedDoc)->caddieTypeId = kTutorialCaddieId;
	}
	// Story mode owns one wrapper per player and queues each named resource before dispatch.
	if (AsCSharedDoc(g_sharedDoc)->gameMode == kStoryGameMode) {
		state = (void *)AsCSharedDoc(g_sharedDoc)->golfDoc;
		unsigned char count = AsCGolfDoc(state)->playerCount;
		void *array = __nwa__FUl(count * (int)sizeof(MWStringStorage) + 16);
		int allocated = array != 0;
		if (allocated) {
			array = __construct_new_array(array, (void *)CConteCaddie_Ctor, (void *)CConteCaddie_DestroyStorage, 12, count);
		}
		AsCGolfCaddie(self)->conteCaddies = array;
		int i = 0;
		int offset = 0;
		for (; i < AsCGolfDoc((void *)AsCSharedDoc(g_sharedDoc)->golfDoc)->playerCount; offset += 12, ++i) {
			char *name = String_FormatTemporary(golfCaddieStrings + IFF_OFFSET(GolfCaddieStringPool, storyCaddieFormat), i);
		ResourcePreloadQueue_AddRequest(g_sceneManager, kCaddieResourceType, 0, kFloat_1_GolfCaddie, (char *)AsCGolfCaddie(self)->conteCaddies + offset, kFloat_0_GolfCaddie, name, 0, kPreloadRequestFlags, 0);
		}
		IActor_DispatchMessage(self, self, golfCaddieStrings + IFF_OFFSET(GolfCaddieStringPool, storyConteMessage), 4, &AsCGolfCaddie(self)->storyConteCaddieOverride, 0, 0, 0);
	}
	// Instantiate equipped caddies for enabled players, then register the shared shadow group.
	for (int i = 0; (unsigned char)i < AsCGolfDoc((void *)AsCSharedDoc(g_sharedDoc)->golfDoc)->playerCount; ++i) {
		void *entry = GolfPlayer_GetEntryByIndex((void *)AsCSharedDoc(g_sharedDoc)->golfDoc, (unsigned char)i);
		// original r3 = lwz 0xb90 (0x80128758), the loop bound's object
		if (((GolfPlayerEntryState*)entry)->state != kPlayerEntryDisabled) {
			void *g;
			unsigned int flag;
			int ix;
			ix = (unsigned char)i;
			g = (char *)g_sharedDoc + ix * (int)sizeof(SharedCaddieSelection);
			flag = (unsigned int)(ix - 2) >> 31;
			CGolfCaddie_CreateCaddieEntry(self, ((SharedCaddieSelection*)g)->caddieTypeId, ((SharedCaddieSelection*)g)->caddieItemTypeId, 0, flag);
		}
	}
	AsCGolfCaddie(self)->storyConteCaddieOverride = 0;
	if (!Sing_805DC7B0()) {
		if (!s_renderPacketInitialized) {
			RenderPacket_Init((char *)s_renderPacket);
			__register_global_object((void *)s_renderPacket, (void *)RenderPacket_Destroy, (void *)s_renderPacketDtorRegistration_804F68E8);
			s_renderPacketInitialized = 1;
		}
		Sing_805DC7B0() = (void *)s_renderPacket;
	}
	CShadowManager_GetOrCreateEntry(g_shadowManager, golfCaddieStrings + IFF_OFFSET(GolfCaddieStringPool, shadowName), kFloat_0_09_CaddieShadow, 16, golfCaddieStrings + IFF_OFFSET(GolfCaddieStringPool, shadowLightName), AsRenderPacketFields(Sing_805DC7B0())->auxiliaryState);
}

// Return the old iterator address and advance by one four-byte caddie entry.
unsigned int CaddieEntryIterator_PostIncrement(unsigned int *value, int unused) {
	unsigned int result = *value;
	*value = result + 4;
	return result;
}

} // extern "C"
