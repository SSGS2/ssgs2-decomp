// Translation unit 0x801E6024-0x801E7790 (16 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
#include "classes/CSceneManager.h"

#define AsCSceneManager(p) ((CSceneManager*)(p))
// The four scene pools own allocationBlocks and a separate buckets array.
// Nodes live inside allocation blocks; pool destruction frees blocks, not
// individual node addresses. The mutex follows the bucket pointer at +0x20.
struct ScenePoolHeader {
    int firstNode, iterator;
    void* allocationBlocks;
    int freeNodes, allocationBatchSize, bucketMask, bucketCount;
    void* buckets;
    char mutex[0x18];
};
// Scene lists embed a two-link sentinel after their count. Empty links point
// to that sentinel; payload starts eight bytes into separately allocated nodes.
// Forward cleanup follows next (+4). Never read payload fields on the sentinel.
struct SceneListLinks { char* previous; char* next; };
struct SceneListHeader { unsigned int count; SceneListLinks sentinel; };
struct CourseGlowRequestStrings { char blueHouseLamp[22], wizardHouseAnimation[22]; };
struct SceneAllocationLinks { char reserved[0xC]; char* previous; char* next; };
struct SceneRenderRecord { SceneListLinks links; char reserved[0x3C]; void* ownedObject; };
struct ChatFontProvider { char reserved[0x10]; int font; };
#define SCENE_OFFSET(field) ((int)&((CSceneManager*)0)->field)
#define SCENE_LAYOUT_OFFSET(type, field) ((int)&((type*)0)->field)
// Each pool owns live OSMutex storage. These constants expose its byte offset.
enum SceneMutexOffsets {
    kBeachPoolMutexOffset = SCENE_OFFSET(beachPoolMutex),
    kRiverPoolMutexOffset = SCENE_OFFSET(riverPoolMutex),
    kGeneratedTexturePoolMutexOffset = SCENE_OFFSET(generatedTexturePoolMutex),
    kBoundaryPoolMutexOffset = SCENE_OFFSET(boundaryPoolMutex)
};
enum { kScenePoolInitialBuckets = 16, kScenePoolInitialBucketBytes = 0x40,
       kSceneOptionSection = 0x1F, kAlternateRenderSelectionOption = 2, kRetainSceneRecord = 0x8,
       kNamedGlowRenderFlag = 0x100, kRenderFlagsType1Or5Or6 = 0xB,
       kRenderFlagsType3 = 6, kRenderFlagsType4 = 7,
       kRenderRequestNodeBytes = 0x50, kAuxiliaryRequestNodeBytes = 0x10 };

extern "C" {

void* __dl__FPv(void*);
void BaseObject_dtor(void*, int);
void appFreeUni__FPv(void*);
void OSLockMutex(void*);
void OSUnlockMutex(void*);
void* appMallocDummy__FUl(unsigned int);
void BaseObject_ctor(void*);
void OSInitMutex(void*);
void OSLockMutex(void*);
void OSUnlockMutex(void*);
void* appMallocUni__FUl(unsigned int);
void* __nw__FUl(unsigned int);
void CMarker_Ctor(void*);
void* RenderPacket_Init(void*);
void __register_global_object(void*, void*, void*);
void RenderPacket_Reset(void*);
void RenderPacket_ClearResources(void*);
// Remove owned render records while preserving records carrying the retention flag.
void CSceneManager_ClearOwnedSceneObjects(void*);
int String_CompareCString(const char*, const char*);
char* strcpy(char*, const char*);
extern char courseGlowEffectStrings[];
extern unsigned int g_sceneManager;
extern void* g_resourceManager;
extern void* g_chatMsg;
extern signed char s_renderPacketInitialized;
extern char s_sceneManagerConstructionVtable[];
extern char __vt__13CSceneManager[];
extern char s_renderPacket[];
extern char s_sceneRenderPacketDestructorRegistration[];
extern void RenderPacket_Destroy(void);

struct WInstanceLock {
    void* lock;
    // Acquire the borrowed pool mutex for this scope.
    WInstanceLock(void* l) { lock = l; OSLockMutex(l); }
    // Release the pool mutex on scope exit.
    ~WInstanceLock() { OSUnlockMutex(lock); }
};

struct LockedTable {
	int f0;
	int f4;
	int f8;
	int fc;
	int cap;
	int mask;
	int cap2;
	void** table;
	char lock[0x18];
	// Initialize an empty locked bucket table with the default sixteen buckets.
	LockedTable() {
		table = 0;
		OSInitMutex(lock);
		WInstanceLock g(lock);
		f0 = 0;
		fc = 0;
		f8 = 0;
		cap = kScenePoolInitialBuckets;
		f4 = 0;
		cap2 = kScenePoolInitialBuckets;
		if (cap2 > 0) {
			table = (void**)appMallocUni__FUl(kScenePoolInitialBucketBytes);
			mask = cap2 - 1;
			for (int i = 0, o = 0; i < cap2; ++i, o += 4) *(void**)((char*)table + o) = 0;
		} else table = 0;
	}
};

// Initialize scene object pools, render lists, marker, options, and the shared render packet.
void* CSceneManager_Ctor(void* self) {
	BaseObject_ctor(self);
	char* b = (char*)self;
	*(void**)b = s_sceneManagerConstructionVtable;
	g_sceneManager = (unsigned int)self;
	*(void**)b = __vt__13CSceneManager;
	(*(int*)&AsCSceneManager(b)->beachBuckets) = 0;
	// Phase 1: create four independent, empty, mutex-protected bucket pools.
	// Beach pool: allocate buckets and clear node/block state under its new mutex.
	{
		OSInitMutex(b + kBeachPoolMutexOffset);
		WInstanceLock g(b + kBeachPoolMutexOffset);
		AsCSceneManager(b)->firstBeachNode = 0;
		AsCSceneManager(b)->freeBeachNodes = 0;
		(*(int*)&AsCSceneManager(b)->beachAllocationBlocks) = 0;
		AsCSceneManager(b)->beachAllocationBatchSize = kScenePoolInitialBuckets;
		AsCSceneManager(b)->beachIterator = 0;
		AsCSceneManager(b)->beachBucketCount = kScenePoolInitialBuckets;
		if (AsCSceneManager(b)->beachBucketCount > 0) {
			AsCSceneManager(b)->beachBuckets = appMallocUni__FUl(kScenePoolInitialBucketBytes);
			AsCSceneManager(b)->beachBucketMask = AsCSceneManager(b)->beachBucketCount - 1;
			for (int i = 0, o = 0; i < AsCSceneManager(b)->beachBucketCount; ++i, o += 4) *(void**)((char*)AsCSceneManager(b)->beachBuckets + o) = 0;
		} else AsCSceneManager(b)->beachBuckets = 0;
	}
	// River pool: initialize bucket mask, bucket count, and bucket storage.
	{
		AsCSceneManager(b)->riverBuckets = 0;
		OSInitMutex(b + kRiverPoolMutexOffset);
		WInstanceLock g(b + kRiverPoolMutexOffset);
		AsCSceneManager(b)->firstRiverNode = 0;
		AsCSceneManager(b)->freeRiverNodes = 0;
		(*(int*)&AsCSceneManager(b)->riverAllocationBlocks) = 0;
		AsCSceneManager(b)->riverAllocationBatchSize = kScenePoolInitialBuckets;
		AsCSceneManager(b)->riverIterator = 0;
		AsCSceneManager(b)->riverBucketCount = kScenePoolInitialBuckets;
		if (AsCSceneManager(b)->riverBucketCount > 0) {
			AsCSceneManager(b)->riverBuckets = appMallocUni__FUl(kScenePoolInitialBucketBytes);
			AsCSceneManager(b)->riverBucketMask = AsCSceneManager(b)->riverBucketCount - 1;
			for (int i = 0, o = 0; i < AsCSceneManager(b)->riverBucketCount; ++i, o += 4) *(void**)((char*)AsCSceneManager(b)->riverBuckets + o) = 0;
		} else AsCSceneManager(b)->riverBuckets = 0;
	}
	// Generated-texture pool: initialize bucket mask, bucket count, and bucket storage.
	{
		AsCSceneManager(b)->generatedTextureBuckets = 0;
		OSInitMutex(b + kGeneratedTexturePoolMutexOffset);
		WInstanceLock g(b + kGeneratedTexturePoolMutexOffset);
		AsCSceneManager(b)->firstGeneratedTextureNode = 0;
		AsCSceneManager(b)->freeGeneratedTextureNodes = 0;
		(*(int*)&AsCSceneManager(b)->generatedTextureAllocationBlocks) = 0;
		AsCSceneManager(b)->generatedTextureAllocationBatchSize = kScenePoolInitialBuckets;
		AsCSceneManager(b)->generatedTextureIterator = 0;
		AsCSceneManager(b)->generatedTextureBucketCount = kScenePoolInitialBuckets;
		if (AsCSceneManager(b)->generatedTextureBucketCount > 0) {
			AsCSceneManager(b)->generatedTextureBuckets = appMallocUni__FUl(kScenePoolInitialBucketBytes);
			AsCSceneManager(b)->generatedTextureBucketMask = AsCSceneManager(b)->generatedTextureBucketCount - 1;
			for (int i = 0, o = 0; i < AsCSceneManager(b)->generatedTextureBucketCount; ++i, o += 4) *(void**)((char*)AsCSceneManager(b)->generatedTextureBuckets + o) = 0;
		} else AsCSceneManager(b)->generatedTextureBuckets = 0;
	}
	// Boundary pool: initialize bucket mask, bucket count, and bucket storage.
	{
		AsCSceneManager(b)->boundaryBuckets = 0;
		OSInitMutex(b + kBoundaryPoolMutexOffset);
		WInstanceLock g(b + kBoundaryPoolMutexOffset);
		AsCSceneManager(b)->firstBoundaryNode = 0;
		AsCSceneManager(b)->freeBoundaryNodes = 0;
		(*(int*)&AsCSceneManager(b)->boundaryAllocationBlocks) = 0;
		AsCSceneManager(b)->boundaryAllocationBatchSize = kScenePoolInitialBuckets;
		AsCSceneManager(b)->boundaryIterator = 0;
		AsCSceneManager(b)->boundaryBucketCount = kScenePoolInitialBuckets;
		if (AsCSceneManager(b)->boundaryBucketCount > 0) {
			AsCSceneManager(b)->boundaryBuckets = appMallocUni__FUl(kScenePoolInitialBucketBytes);
			AsCSceneManager(b)->boundaryBucketMask = AsCSceneManager(b)->boundaryBucketCount - 1;
			for (int i = 0, o = 0; i < AsCSceneManager(b)->boundaryBucketCount; ++i, o += 4) *(void**)((char*)AsCSceneManager(b)->boundaryBuckets + o) = 0;
		} else AsCSceneManager(b)->boundaryBuckets = 0;
	}
	// Phase 2: null owned scene effects and establish all five empty list sentinels.
	AsCSceneManager(b)->generatedSurface = 0;
	(*(int*)&AsCSceneManager(b)->marker) = 0;
	(*(int*)&AsCSceneManager(b)->weatherEffect) = 0;
	(*(int*)&AsCSceneManager(b)->secondaryWeatherEffect) = 0;
	AsCSceneManager(b)->sceneViewOptionEnabled = 1;
	// Preserve the separate scene option defaults; these two controls have no established consumers here.
	AsCSceneManager(b)->unk_101 = 0;
	AsCSceneManager(b)->alternateRenderSelection = 0;
	AsCSceneManager(b)->renderRecordCount = 0;
	// CSceneManager_Ctor (0x801E6024): embedded list sentinels at +0x108..+0x13C.
	void* a108 = (char*)&AsCSceneManager(b)->lastRenderRecord;
	AsCSceneManager(b)->firstRenderRecord = a108;
	AsCSceneManager(b)->lastRenderRecord = a108;
	AsCSceneManager(b)->overlayDrawableCount = 0;
	void* a114 = (char*)&AsCSceneManager(b)->lastOverlayDrawable;
	AsCSceneManager(b)->firstOverlayDrawable = a114;
	AsCSceneManager(b)->lastOverlayDrawable = a114;
	AsCSceneManager(b)->deferredDrawableCount = 0;
	void* a120 = (char*)&AsCSceneManager(b)->lastDeferredDrawable;
	AsCSceneManager(b)->firstDeferredDrawable = a120;
	AsCSceneManager(b)->lastDeferredDrawable = a120;
	AsCSceneManager(b)->transparentObjectCount = 0;
	void* a12c = (char*)&AsCSceneManager(b)->lastTransparentObject;
	AsCSceneManager(b)->firstTransparentObject = a12c;
	AsCSceneManager(b)->lastTransparentObject = a12c;
	AsCSceneManager(b)->debugPointCount = 0;
	void* a13c = (char*)&AsCSceneManager(b)->lastDebugPoint;
	AsCSceneManager(b)->firstDebugPoint = a13c;
	AsCSceneManager(b)->lastDebugPoint = a13c;
	AsCSceneManager(b)->objectNamesVisible = 0;
	AsCSceneManager(b)->renderEnabled = 1;
	// This option is independent of renderEnabled and cameraProximityFadeEnabled.
	AsCSceneManager(b)->unk_14C = 1;
	AsCSceneManager(b)->debugNameFont = 0;
	AsCSceneManager(b)->cameraProximityFadeEnabled = 1;
	AsCSceneManager(b)->debugDrawFlags = 0;
	// Phase 3: create the marker, read the view option, and borrow the chat font.
	void* p = __nw__FUl(0x28);
	if (p) CMarker_Ctor(p);
	AsCSceneManager(b)->marker = p;
	int result;
	void* mgr = *(void**)g_resourceManager;

	void** vt = *(void***)mgr;
	  ((void(*)(void*, int, int, int*))(vt[9]))(mgr, kSceneOptionSection, kAlternateRenderSelectionOption, &result);
	if (result) AsCSceneManager(b)->alternateRenderSelection = 1;
	void* singleton = Sing_805DC7B0();
	AsCSceneManager(b)->debugNameFont = ((ChatFontProvider*)g_chatMsg)->font;
	AsCSceneManager(b)->effectRenderPassActive = 0;
	AsCSceneManager(b)->previousCameraMode = -1;
	// Phase 4: obtain the process-wide render packet and reset it for this scene.
	if (0 == singleton) {
		if (s_renderPacketInitialized == 0) {
			RenderPacket_Init(s_renderPacket);
			__register_global_object(s_renderPacket, (void*)RenderPacket_Destroy, s_sceneRenderPacketDestructorRegistration);
			s_renderPacketInitialized = 1;
		}
		Sing_805DC7B0() = s_renderPacket;
	}
	RenderPacket_Reset(Sing_805DC7B0());
	return self;
}

// Free pool blocks and buckets under recursive locking, then optionally delete the pool.
static inline void* DestroyScenePoolStorage(void* self, int deallocate) {
	if (self != 0) {
		char* base = (char*)self;
		{
			WInstanceLock g(base + SCENE_LAYOUT_OFFSET(ScenePoolHeader, mutex));
			char* node;
			char* lock;
			char* head;
			node = (*(char**)&((ScenePoolHeader*)base)->allocationBlocks);
			if (node != 0) {
				lock = base + SCENE_LAYOUT_OFFSET(ScenePoolHeader, mutex);
				while ((node = (*(char**)&((ScenePoolHeader*)base)->allocationBlocks)) != 0) {
					OSLockMutex(lock);
					char* prev = ((SceneAllocationLinks*)node)->previous;
					char* next = ((SceneAllocationLinks*)node)->next;
					((SceneAllocationLinks*)next)->previous = prev;
					((SceneAllocationLinks*)((SceneAllocationLinks*)node)->previous)->next = ((SceneAllocationLinks*)node)->next;
					head = ((SceneAllocationLinks*)node)->next;
					if (head == node)
						head = 0;
					OSUnlockMutex(lock);
					(*(char**)&((ScenePoolHeader*)base)->allocationBlocks) = head;
					appFreeUni__FPv(node);
				}
			}
			void* allocation = ((ScenePoolHeader*)base)->buckets;
			if (allocation != 0) {
				appFreeUni__FPv(allocation);
				((ScenePoolHeader*)base)->buckets = 0;
			}
		}
		if (deallocate > 0)
			__dl__FPv(self);
	}
	return self;
}

#pragma push
#pragma use_lmw_stmw on
// Destroy a locked scene pool, freeing its allocation blocks and bucket table.
void* dtor_801E6474(void* self, int deallocate) {
	return DestroyScenePoolStorage(self, deallocate);
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Destroy a locked scene pool, freeing its allocation blocks and bucket table.
void* dtor_801E654C(void* self, int deallocate) {
	return DestroyScenePoolStorage(self, deallocate);
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Destroy a locked scene pool, freeing its allocation blocks and bucket table.
void* dtor_801E6624(void* self, int deallocate) {
	return DestroyScenePoolStorage(self, deallocate);
}

// Destroy a locked scene pool, freeing its allocation blocks and bucket table.
void* dtor_801E66FC(void* self, int deallocate) {
	return DestroyScenePoolStorage(self, deallocate);
}
#pragma pop

// Drain a sentinel list of scene records and optionally release the container.
void* dtor_801E67D4(void* self, int deallocate) {
	if (self != 0) {
		char* base = (char*)self;
		char* sentinel = base + SCENE_LAYOUT_OFFSET(SceneListHeader, sentinel);
		char* node = *(char**)(base + SCENE_LAYOUT_OFFSET(SceneListHeader, sentinel.next));
		if (node != sentinel) {
			char* prev = *(char**)sentinel;
			char* next = *(char**)node;
			*(char**)(next + 4) = *(char**)(prev + 4);
			*(char**)(*(char**)(prev + 4)) = *(char**)node;
			while (node != sentinel) {
				char* current = node;
				node = *(char**)(node + 4);
				appFreeUni__FPv(current);
				*(unsigned int*)base -= 1;
			}
		}
		if (deallocate > 0)
			__dl__FPv(self);
	}
	return self;
}

// Drain a sentinel list of scene records and optionally release the container.
void* SceneList_Clear(void* self, int deallocate) {
	if (self != 0) {
		char* base = (char*)self;
		char* sentinel = base + SCENE_LAYOUT_OFFSET(SceneListHeader, sentinel);
		char* node = *(char**)(base + SCENE_LAYOUT_OFFSET(SceneListHeader, sentinel.next));
		if (node != sentinel) {
			char* prev = *(char**)sentinel;
			char* next = *(char**)node;
			*(char**)(next + 4) = *(char**)(prev + 4);
			*(char**)(*(char**)(prev + 4)) = *(char**)node;
			while (node != sentinel) {
				char* current = node;
				node = *(char**)(node + 4);
				appFreeUni__FPv(current);
				*(unsigned int*)base -= 1;
			}
		}
		if (deallocate > 0)
			__dl__FPv(self);
	}
	return self;
}

// Drain a sentinel list of scene records and optionally release the container.
void* dtor_801E6884(void* self, int deallocate) {
	if (self != 0) {
		if (self != 0 && self != 0 && self != 0) {
			char* base = (char*)self;
			char* sentinel = base + SCENE_LAYOUT_OFFSET(SceneListHeader, sentinel);
			char* node = *(char**)(base + SCENE_LAYOUT_OFFSET(SceneListHeader, sentinel.next));
			if (node != sentinel) {
				char* prev = *(char**)sentinel;
				char* next = *(char**)node;
				*(char**)(next + 4) = *(char**)(prev + 4);
				*(char**)(*(char**)(prev + 4)) = *(char**)node;
				while (node != sentinel) {
					char* current = node;
					node = *(char**)(node + 4);
					appFreeUni__FPv(current);
					*(unsigned int*)base -= 1;
				}
			}
		}
		if (deallocate > 0)
			__dl__FPv(self);
	}
	return self;
}

// Drain a sentinel list of scene records and optionally release the container.
void* dtor_801E6940(void* self, int deallocate) {
	if (self != 0) {
		if (self != 0 && self != 0 && self != 0) {
			char* base = (char*)self;
			char* sentinel = base + SCENE_LAYOUT_OFFSET(SceneListHeader, sentinel);
			char* node = *(char**)(base + SCENE_LAYOUT_OFFSET(SceneListHeader, sentinel.next));
			if (node != sentinel) {
				char* prev = *(char**)sentinel;
				char* next = *(char**)node;
				*(char**)(next + 4) = *(char**)(prev + 4);
				*(char**)(*(char**)(prev + 4)) = *(char**)node;
				while (node != sentinel) {
					char* current = node;
					node = *(char**)(node + 4);
					appFreeUni__FPv(current);
					*(unsigned int*)base -= 1;
				}
			}
		}
		if (deallocate > 0)
			__dl__FPv(self);
	}
	return self;
}

// Drain a sentinel list of scene records and optionally release the container.
void* SceneListHeader_dtor(void* self, int deallocate) {
	if (self != 0) {
		if (self != 0 && self != 0 && self != 0) {
			char* base = (char*)self;
			char* sentinel = base + SCENE_LAYOUT_OFFSET(SceneListHeader, sentinel);
			char* node = *(char**)(base + SCENE_LAYOUT_OFFSET(SceneListHeader, sentinel.next));
			if (node != sentinel) {
				char* prev = *(char**)sentinel;
				char* next = *(char**)node;
				*(char**)(next + 4) = *(char**)(prev + 4);
				*(char**)(*(char**)(prev + 4)) = *(char**)node;
				while (node != sentinel) {
					char* current = node;
					node = *(char**)(node + 4);
					appFreeUni__FPv(current);
					*(unsigned int*)base -= 1;
				}
			}
		}
		if (deallocate > 0)
			__dl__FPv(self);
	}
	return self;
}

// Drain a sentinel list of scene records and optionally release the container.
void* SceneListHeader_Clear(void* self, int deallocate) {
	if (self != 0) {
		if (self != 0 && self != 0 && self != 0) {
			char* base = (char*)self;
			char* sentinel = base + SCENE_LAYOUT_OFFSET(SceneListHeader, sentinel);
			char* node = *(char**)(base + SCENE_LAYOUT_OFFSET(SceneListHeader, sentinel.next));
			if (node != sentinel) {
				char* prev = *(char**)sentinel;
				char* next = *(char**)node;
				*(char**)(next + 4) = *(char**)(prev + 4);
				*(char**)(*(char**)(prev + 4)) = *(char**)node;
				while (node != sentinel) {
					char* current = node;
					node = *(char**)(node + 4);
					appFreeUni__FPv(current);
					*(unsigned int*)base -= 1;
				}
			}
		}
		if (deallocate > 0)
			__dl__FPv(self);
	}
	return self;
}

// Optionally release storage for an otherwise empty container destructor.
void* dtor_801E76B8(void* self, int deallocate) {
	if (self != 0 && deallocate > 0)
		__dl__FPv(self);
	return self;
}

#pragma push
#pragma use_lmw_stmw on
// Release owned allocations before freeing their list nodes.
static inline void ReleaseSceneRenderPayloads(char* b) {
    {
        char* renderNode;
        char* sentinel = b + SCENE_OFFSET(lastRenderRecord);
        renderNode = (*(char**)&AsCSceneManager(b)->firstRenderRecord);
        while (renderNode != sentinel) {
            void* ownedObject = *(void**)(renderNode + SCENE_LAYOUT_OFFSET(SceneRenderRecord, ownedObject));
            if (0 != ownedObject) { __dl__FPv(ownedObject); *(void**)(renderNode + SCENE_LAYOUT_OFFSET(SceneRenderRecord, ownedObject)) = 0; }
            renderNode = ((SceneListLinks*)renderNode)->next;
        }
    }
}

// Detach and free this embedded list's nodes; payload cleanup is handled separately.
static inline void DrainSceneRenderNodes(char* b) {
    {
        char* node;
        char* sentinel = b + SCENE_OFFSET(lastRenderRecord);
        node = (*(char**)&AsCSceneManager(b)->firstRenderRecord);
        if (node != sentinel) {
            char* lastNode = ((SceneListLinks*)sentinel)->previous;
            char* beforeFirst = ((SceneListLinks*)node)->previous;
            ((SceneListLinks*)beforeFirst)->next = ((SceneListLinks*)lastNode)->next;
            char* afterLast = ((SceneListLinks*)lastNode)->next;
            ((SceneListLinks*)afterLast)->previous = ((SceneListLinks*)node)->previous;
            while (node != sentinel) {
                char* nodeToFree = node;
                node = ((SceneListLinks*)node)->next;
                appFreeUni__FPv(nodeToFree);
                int* listCount = (&AsCSceneManager(b)->renderRecordCount);
                *listCount = *listCount - 1;
            }
        }
    }
}

// Detach and free this embedded list's nodes; payload cleanup is handled separately.
static inline void DrainSceneOverlayNodes(char* b) {
    {
        char* node;
        char* sentinel = b + SCENE_OFFSET(lastOverlayDrawable);
        node = (*(char**)&AsCSceneManager(b)->firstOverlayDrawable);
        if (node != sentinel) {
            char* beforeFirst = ((SceneListLinks*)node)->previous;

            char* lastNode = ((SceneListLinks*)sentinel)->previous; ((SceneListLinks*)beforeFirst)->next = ((SceneListLinks*)lastNode)->next;
            char* afterLast = ((SceneListLinks*)lastNode)->next;
            ((SceneListLinks*)afterLast)->previous = ((SceneListLinks*)node)->previous;
            while (node != sentinel) {
                char* nodeToFree = node;
                node = ((SceneListLinks*)node)->next;
                appFreeUni__FPv(nodeToFree);
                int* listCount = (&AsCSceneManager(b)->overlayDrawableCount);
                *listCount = *listCount - 1;
            }
        }
    }
}

// Detach and free this embedded list's nodes; payload cleanup is handled separately.
static inline void DrainSceneDeferredNodes(char* b) {
    {
        char* node;
        char* sentinel = b + SCENE_OFFSET(lastDeferredDrawable);
        node = (*(char**)&AsCSceneManager(b)->firstDeferredDrawable);
        if (node != sentinel) {
            char* lastNode = ((SceneListLinks*)sentinel)->previous;

            char* beforeFirst = ((SceneListLinks*)node)->previous; ((SceneListLinks*)beforeFirst)->next = ((SceneListLinks*)lastNode)->next;
            char* afterLast = ((SceneListLinks*)lastNode)->next;
            ((SceneListLinks*)afterLast)->previous = ((SceneListLinks*)node)->previous;
            while (node != sentinel) {

                char* nodeToFree = node; node = ((SceneListLinks*)node)->next;
                appFreeUni__FPv(nodeToFree);
                int* listCount = (&AsCSceneManager(b)->deferredDrawableCount);
                *listCount = *listCount - 1;
            }
        }
    }
}

// Detach and free this embedded list's nodes; payload cleanup is handled separately.
static inline void DrainSceneTransparentNodes(char* b) {
    {
        char* node;
        char* sentinel = b + SCENE_OFFSET(lastTransparentObject);
        node = (*(char**)&AsCSceneManager(b)->firstTransparentObject);
        if (node != sentinel) {
            char* lastNode = ((SceneListLinks*)sentinel)->previous;
            char* beforeFirst = ((SceneListLinks*)node)->previous;

            ((SceneListLinks*)beforeFirst)->next = ((SceneListLinks*)lastNode)->next; char* afterLast = ((SceneListLinks*)lastNode)->next;
            ((SceneListLinks*)afterLast)->previous = ((SceneListLinks*)node)->previous;
            while (node != sentinel) {
                char* nodeToFree = node;
                int* listCount = (&AsCSceneManager(b)->transparentObjectCount);
                node = ((SceneListLinks*)node)->next;
                appFreeUni__FPv(nodeToFree);
                *listCount = *listCount - 1;
            }
        }
    }
}

// Detach and free this embedded list's nodes; payload cleanup is handled separately.
static inline void DestroySceneDebugList(char* b) {
    if (b + SCENE_OFFSET(debugPointCount)) {
        char* node;
        char* sentinel = b + SCENE_OFFSET(lastDebugPoint);
        node = (*(char**)&AsCSceneManager(b)->firstDebugPoint);
        if (node != sentinel) {
            char* lastNode = ((SceneListLinks*)sentinel)->previous;
            char* beforeFirst = ((SceneListLinks*)node)->previous;
            ((SceneListLinks*)beforeFirst)->next = ((SceneListLinks*)lastNode)->next;
            char* afterLast = ((SceneListLinks*)lastNode)->next;
            ((SceneListLinks*)afterLast)->previous = ((SceneListLinks*)node)->previous;
            while (node != sentinel) {
                char* nodeToFree = node;
                node = ((SceneListLinks*)node)->next;
                int* listCount = (int*)(b + SCENE_OFFSET(debugPointCount));
                appFreeUni__FPv(nodeToFree);
                *listCount = *listCount - 1;
            }
        }
    }
}

// Detach and free this embedded list's nodes; payload cleanup is handled separately.
static inline void DestroySceneTransparentList(char* b) {
    if (b + SCENE_OFFSET(transparentObjectCount)) {
        char* node;
        char* sentinel = b + SCENE_OFFSET(lastTransparentObject);
        node = (*(char**)&AsCSceneManager(b)->firstTransparentObject);
        if (node != sentinel) {
            char* lastNode = ((SceneListLinks*)sentinel)->previous;
            char* beforeFirst = ((SceneListLinks*)node)->previous;
            ((SceneListLinks*)beforeFirst)->next = ((SceneListLinks*)lastNode)->next;
            char* afterLast = ((SceneListLinks*)lastNode)->next;
            ((SceneListLinks*)afterLast)->previous = ((SceneListLinks*)node)->previous;
            while (node != sentinel) {
                char* nodeToFree = node;
                node = ((SceneListLinks*)node)->next;
                int* listCount = (int*)(b + SCENE_OFFSET(transparentObjectCount));
                appFreeUni__FPv(nodeToFree);
                *listCount = *listCount - 1;
            }
        }
    }
}

// Detach and free this embedded list's nodes; payload cleanup is handled separately.
static inline void DestroySceneDeferredList(char* b) {
    if (b + SCENE_OFFSET(deferredDrawableCount)) {
        char* node;
        char* sentinel = b + SCENE_OFFSET(lastDeferredDrawable);
        node = (*(char**)&AsCSceneManager(b)->firstDeferredDrawable);
        if (node != sentinel) {
            char* beforeFirst = ((SceneListLinks*)node)->previous;

            char* lastNode = ((SceneListLinks*)sentinel)->previous; ((SceneListLinks*)beforeFirst)->next = ((SceneListLinks*)lastNode)->next;
            char* afterLast = ((SceneListLinks*)lastNode)->next;
            ((SceneListLinks*)afterLast)->previous = ((SceneListLinks*)node)->previous;
            while (node != sentinel) {
                char* nodeToFree = node;
                node = ((SceneListLinks*)node)->next;
                int* listCount = (int*)(b + SCENE_OFFSET(deferredDrawableCount));
                appFreeUni__FPv(nodeToFree);
                *listCount = *listCount - 1;
            }
        }
    }
}

// Detach and free this embedded list's nodes; payload cleanup is handled separately.
static inline void DestroySceneOverlayList(char* b) {
    if (b + SCENE_OFFSET(overlayDrawableCount)) {
        char* node;
        char* sentinel = b + SCENE_OFFSET(lastOverlayDrawable);
        node = (*(char**)&AsCSceneManager(b)->firstOverlayDrawable);
        if (node != sentinel) {
            char* beforeFirst = ((SceneListLinks*)node)->previous;
            char* lastNode = ((SceneListLinks*)sentinel)->previous;
            ((SceneListLinks*)beforeFirst)->next = ((SceneListLinks*)lastNode)->next;
            char* afterLast = ((SceneListLinks*)lastNode)->next;
            ((SceneListLinks*)afterLast)->previous = ((SceneListLinks*)node)->previous;
            while (node != sentinel) {

                char* nodeToFree = node; node = ((SceneListLinks*)node)->next;
                appFreeUni__FPv(nodeToFree);
                int* listCount = (int*)(b + SCENE_OFFSET(overlayDrawableCount));
                *listCount = *listCount - 1;
            }
        }
    }
}

// Detach and free this embedded list's nodes; payload cleanup is handled separately.
static inline void DestroySceneRenderList(char* b) {
    if (b + SCENE_OFFSET(renderRecordCount)) {
        char* node;
        char* sentinel = b + SCENE_OFFSET(lastRenderRecord);
        node = (*(char**)&AsCSceneManager(b)->firstRenderRecord);
        if (node != sentinel) {
            char* lastNode = ((SceneListLinks*)sentinel)->previous;
            char* beforeFirst = ((SceneListLinks*)node)->previous;
            ((SceneListLinks*)beforeFirst)->next = ((SceneListLinks*)lastNode)->next;
            char* afterLast = ((SceneListLinks*)lastNode)->next;
            ((SceneListLinks*)afterLast)->previous = ((SceneListLinks*)node)->previous;
            while (node != sentinel) {
                char* nodeToFree = node;
                node = ((SceneListLinks*)node)->next;
                appFreeUni__FPv(nodeToFree);
                int* listCount = (int*)(b + SCENE_OFFSET(renderRecordCount));
                *listCount = *listCount - 1;
            }
        }
    }
}

// Release this pool's blocks and buckets under its recursive mutex.
static inline void DestroySceneBoundaryPool(char* b) {
    char* nxt;
    char* mutexStorage;
    if (b + SCENE_OFFSET(firstBoundaryNode)) {
        WInstanceLock guard(b + kBoundaryPoolMutexOffset);
        char* node = (*(char**)&AsCSceneManager(b)->boundaryAllocationBlocks);
        if (0 != node) {
            mutexStorage = b + kBoundaryPoolMutexOffset;
            while (node != 0) {
                OSLockMutex(mutexStorage);
                char* next;
                char* prev = ((SceneAllocationLinks*)node)->previous;
                next = ((SceneAllocationLinks*)node)->next;
                ((SceneAllocationLinks*)next)->previous = prev;
                ((SceneAllocationLinks*)prev)->next = next;
                nxt = ((SceneAllocationLinks*)node)->next;
                if (node == nxt) nxt = 0;
                OSUnlockMutex(mutexStorage);
                (*(char**)&AsCSceneManager(b)->boundaryAllocationBlocks) = nxt;
                appFreeUni__FPv(node);
                node = nxt;
            }
        }
        void* bucketStorage = AsCSceneManager(b)->boundaryBuckets;
        if (bucketStorage) {
            appFreeUni__FPv(bucketStorage);
            AsCSceneManager(b)->boundaryBuckets = 0;
        }
    }
}

// Release this pool's blocks and buckets under its recursive mutex.
static inline void DestroySceneGeneratedTexturePool(char* b) {
    char* nxt;
    char* mutexStorage;
    if (b + SCENE_OFFSET(firstGeneratedTextureNode)) {
        WInstanceLock guard(b + kGeneratedTexturePoolMutexOffset);
        char* node = (*(char**)&AsCSceneManager(b)->generatedTextureAllocationBlocks);
        if (0 != node) {
            mutexStorage = b + kGeneratedTexturePoolMutexOffset;
            while (node) {
                OSLockMutex(mutexStorage);
                char* next;
                char* prev = ((SceneAllocationLinks*)node)->previous;
                next = ((SceneAllocationLinks*)node)->next;
                ((SceneAllocationLinks*)next)->previous = prev;
                ((SceneAllocationLinks*)prev)->next = next;
                nxt = ((SceneAllocationLinks*)node)->next;
                if (node == nxt) nxt = 0;
                OSUnlockMutex(mutexStorage);
                (*(char**)&AsCSceneManager(b)->generatedTextureAllocationBlocks) = nxt;
                appFreeUni__FPv(node);
                node = nxt;
            }
        }
        void* bucketStorage = AsCSceneManager(b)->generatedTextureBuckets;
        if (0 != bucketStorage) {
            appFreeUni__FPv(bucketStorage);
            AsCSceneManager(b)->generatedTextureBuckets = 0;
        }
    }
}

// Release this pool's blocks and buckets under its recursive mutex.
static inline void DestroySceneRiverPool(char* b) {
    char* nxt;
    char* mutexStorage;
    if (b + SCENE_OFFSET(firstRiverNode)) {
        WInstanceLock guard(b + kRiverPoolMutexOffset);
        char* node = (*(char**)&AsCSceneManager(b)->riverAllocationBlocks);
        if (node) {
            mutexStorage = b + kRiverPoolMutexOffset;
            while (node) {
                OSLockMutex(mutexStorage);
                char* next;
                char* prev = ((SceneAllocationLinks*)node)->previous;
                next = ((SceneAllocationLinks*)node)->next;
                ((SceneAllocationLinks*)next)->previous = prev;
                ((SceneAllocationLinks*)prev)->next = next;
                nxt = ((SceneAllocationLinks*)node)->next;
                if (node == nxt) nxt = 0;
                OSUnlockMutex(mutexStorage);
                (*(char**)&AsCSceneManager(b)->riverAllocationBlocks) = nxt;
                appFreeUni__FPv(node);
                node = nxt;
            }
        }
        void* bucketStorage = AsCSceneManager(b)->riverBuckets;
        if (bucketStorage) {
            appFreeUni__FPv(bucketStorage);
            AsCSceneManager(b)->riverBuckets = 0;
        }
    }
}

// Release this pool's blocks and buckets under its recursive mutex.
static inline void DestroySceneBeachPool(char* b) {
    char* nxt;
    char* mutexStorage;
    if (b + SCENE_OFFSET(firstBeachNode)) {
        WInstanceLock guard(b + kBeachPoolMutexOffset);
        char* node = (*(char**)&AsCSceneManager(b)->beachAllocationBlocks);
        if (node) {
            mutexStorage = b + kBeachPoolMutexOffset;
            while (0 != node) {
                OSLockMutex(mutexStorage);
                char* next;
                char* prev = ((SceneAllocationLinks*)node)->previous;
                next = ((SceneAllocationLinks*)node)->next;
                ((SceneAllocationLinks*)next)->previous = prev;
                ((SceneAllocationLinks*)prev)->next = next;
                nxt = ((SceneAllocationLinks*)node)->next;
                if (node == nxt) nxt = 0;
                OSUnlockMutex(mutexStorage);
                (*(char**)&AsCSceneManager(b)->beachAllocationBlocks) = nxt;
                appFreeUni__FPv(node);
                node = nxt;
            }
        }
        void* bucketStorage = AsCSceneManager(b)->beachBuckets;
        if (bucketStorage) {
            appFreeUni__FPv(bucketStorage);
            AsCSceneManager(b)->beachBuckets = 0;
        }
    }
}

// Release scene resources, drain owned lists and locked pools, then destroy the base object.
// Deleting-destructor ABI: null is accepted; deallocate > 0 also frees self,
// so the returned address is then dangling. Teardown must not race other users.
// Ownership order is payload allocations -> list nodes -> scene objects ->
// marker/weather -> residual lists -> pool blocks/buckets -> BaseObject/storage.
// Repeated embedded-address guards are destructor expansion, not separate
// readiness tests. Pool scopes use a recursive mutex: each unlink locks again
// inside the outer guard and drops that inner acquisition before freeing a block.
void* CSceneManager_dtor(void* self, int deallocate) {
	char* b = (char*)self;
	if (self) {
		void* singleton = Sing_805DC7B0();
		*(void**)b = __vt__13CSceneManager;
		if (singleton == 0) {
			if (0 == s_renderPacketInitialized) {
				RenderPacket_Init(s_renderPacket);
				__register_global_object(s_renderPacket, (void*)RenderPacket_Destroy, s_sceneRenderPacketDestructorRegistration);
				s_renderPacketInitialized = 1;
			}
			Sing_805DC7B0() = s_renderPacket;
		}
		RenderPacket_ClearResources(Sing_805DC7B0());

		// Phase 1: free each render record's owned +0x44 allocation before its node.
		ReleaseSceneRenderPayloads(b);

		// Phase 2: unlink and free list nodes; counts reach zero independently.
		// Render-record list (its owned allocations were released above).
		DrainSceneRenderNodes(b);
		// Overlay-drawable list: free nodes only.
		DrainSceneOverlayNodes(b);
		// Deferred-drawable list: free nodes only.
		DrainSceneDeferredNodes(b);
		// Transparent-object list: free nodes only.
		DrainSceneTransparentNodes(b);

		CSceneManager_ClearOwnedSceneObjects(self);

		// Release the marker through deleting virtual slot 2, then clear ownership.
		{
			void* ownedObject = AsCSceneManager(b)->marker;
			if (ownedObject) {
				void** vt = *(void***)ownedObject;
				((void(*)(void*, int))(vt[2]))(ownedObject, 1);
				AsCSceneManager(b)->marker = 0;
			}
		}
		// Release the primary weather object through the same deleting slot.
		{
			void* ownedObject = AsCSceneManager(b)->weatherEffect;
			if (ownedObject) {
				void** vt = *(void***)ownedObject;
				((void(*)(void*, int))(vt[2]))(ownedObject, 1);
				AsCSceneManager(b)->weatherEffect = 0;
			}
		}
		// Release the secondary weather object through the same deleting slot.
		{
			void* ownedObject = AsCSceneManager(b)->secondaryWeatherEffect;
			if (ownedObject) {
				void** vt = *(void***)ownedObject;
				((void(*)(void*, int))(vt[2]))(ownedObject, 1);
				AsCSceneManager(b)->secondaryWeatherEffect = 0;
			}
		}

		// Embedded-list destruction: detach the entire [first,sentinel) range,
		// restore both sentinel links, then follow saved +4 links and free nodes.
		// The earlier drain emptied the four render lists; their repeated cleanup
		// below is the member-destructor path. Debug points are first drained here.
		DestroySceneDebugList(b);
		// Transparent list member cleanup (normally already empty after phase 2).
		DestroySceneTransparentList(b);
		// Deferred list member cleanup.
		DestroySceneDeferredList(b);
		// Overlay list member cleanup.
		DestroySceneOverlayList(b);
		// Render-record list member cleanup; payloads were freed in phase 1.
		DestroySceneRenderList(b);

		// Phase 3: under each pool mutex, unlink/free allocation blocks and then
		// free its separate bucket table, in reverse construction order.
		DestroySceneBoundaryPool(b);
		DestroySceneGeneratedTexturePool(b);
		DestroySceneRiverPool(b);
		DestroySceneBeachPool(b);

		g_sceneManager = 0;
		BaseObject_dtor(self, 0);
		if (deallocate > 0)
			__dl__FPv(self);
	}
	return self;
}
#pragma pop

// Clear the scene-manager singleton during base cleanup and optionally free its storage.
void* dtor_801E6414(void* self, int deallocate) {
	if (self != 0) {
		g_sceneManager = 0;
		BaseObject_dtor(self, 0);
		if (deallocate > 0)
			__dl__FPv(self);
	}
	return self;
}

struct Node_801E76F8 {
	Node_801E76F8* prev;
	Node_801E76F8* next;
	char reserved08[0x34];
	unsigned int flags;
	char reserved40[4];
	void* allocation;
};

#pragma push
#pragma use_lmw_stmw on
// Remove and release unretained scene render records, preserving the list's retained nodes.
// "Unlocked" in the shared name means flags lacking 0x8, not mutex ownership.
// This routine takes no lock and deletes +0x44 allocations before freeing nodes.
void CSceneManager_RemoveUnretainedRenderRecords(void* object) {
	// 0x801E76F8: first list's sentinel, tail and count at +0x108/+0x10C/+0x104.
	char* base = (char*)object;
	char* sentinel = (char*)&AsCSceneManager(base)->lastRenderRecord;
	int zero = 0;
	Node_801E76F8* node = (Node_801E76F8*)AsCSceneManager(base)->firstRenderRecord;
	while ((char*)node != sentinel) {
		if (node->flags & kRetainSceneRecord) {
			node = node->next;
		} else {
			if (node->allocation != 0) {
				__dl__FPv(node->allocation);
				node->allocation = (void*)zero;
			}
			Node_801E76F8* next = node->next;
			node->prev->next = next;
			node->next->prev = node->prev;
			appFreeUni__FPv(node);
			node = next;
			*(unsigned int*)&AsCSceneManager(base)->renderRecordCount -= 1;
		}
	}
}
#pragma pop

// Request kinds retain numeric suffixes where a universal resource class is not proved.
enum SceneRequestKind {
    kSceneRequestKind0 = 0, kSceneRequestKind1 = 1, kSceneRequestKind2 = 2,
    kSceneRequestKind3 = 3, kSceneRequestKind4 = 4,
    kSceneRequestKind5WithAuxiliaryPair = 5, kSceneRequestKind6 = 6
};
// Observed payload roles (pointers are borrowed; no common class is assumed):
// kind 0: course fallback puppet in pointer0; name points at its resource name.
// kinds 2/3: course mesh puppet in pointer0; scalarParameter is origin distance,
//            context is the mesh matrix; request flags may carry expanded bounds.
// kind 1: pointer1 is a drawable owner (guideline/ball-shadow callers).
// kind 4: pointer0 may be a resource handle; pointer1 may be an actor owner;
//         context may supply a transform (creature caller creature_puppet_loading.cpp).
// kind 5: pointer1 is also copied into the auxiliary list; class remains unproved.
// kind 6: pointer1 is an effect/drawable owner (tail, particle, ball-track callers).
// Wii passes floats separately from pointer words: declarations at callers differ
// in source argument order. Preserve this function's ABI when adding new requests.
struct Name_801E733C { unsigned char bytes[0x20]; };
struct Record_801E733C {
	void* resourceType;
	void* context;
	float scalarParameter;
	void* resourcePointer0;
	void* resourcePointer1;
	Name_801E733C name;
	unsigned int flags;
	unsigned char b38;
	unsigned char b39;
	unsigned char pad[2];
	unsigned int b3c;
	unsigned int requestFlags;
	float squaredDistanceLimit;
	// No implicit payload cleanup; this temporary only copies pointer words.
	~Record_801E733C() {}
};
struct ListNode_801E733C { ListNode_801E733C* previous; ListNode_801E733C* next; Record_801E733C record; };
// Node previous (+0) and next (+4) link the list in both directions.
// List lastNode/firstNode cache its endpoints; the list address is the sentinel.
// Object.renderRequestCount/renderRequests at +0x104/+0x108 are render requests;
// auxiliaryRequestCount/auxiliaryRequests at +0x110/+0x114
// hold the auxiliary pairs added for type 5. They overlay the scene lists above.
struct List_801E733C { ListNode_801E733C* lastNode; ListNode_801E733C* firstNode; };
// Auxiliary pair cleanup is empty; pointed-to resource data is untouched here.
struct Pair_801E733C { void* a; void* b; ~Pair_801E733C() {} };
struct Guard_801E733C {
	ListNode_801E733C* allocation; ListNode_801E733C* copy; List_801E733C* head; ListNode_801E733C* n;
	// Free a node still owned by this insertion guard.
	~Guard_801E733C() { if (n) appFreeUni__FPv(n); }

	// Associate an unlinked allocation with its destination sentinel and cleanup owner.
	void init(ListNode_801E733C* p, List_801E733C* h) { allocation = p; copy = p; head = h; n = p; }
};
// Materialize the zero allocator/construction flag with its original volatile store.
struct Flag_801E733C { volatile char flag; Flag_801E733C(): flag(0) {} };
// Empty iterator-pair cleanup preserves the original insertion scope.
struct Anchor_801E733C { ListNode_801E733C* a; ListNode_801E733C* b; ~Anchor_801E733C() {} };
struct Tail_801E733C {
	List_801E733C* head; ListNode_801E733C* n;
	// Start without a pending node to free.
	Tail_801E733C() : n(0) {}
	// Free an insertion allocation unless the caller committed by clearing n.
	~Tail_801E733C() { if (n) appFreeUni__FPv(n); }
};
struct Object_801E733C { char pad[0x104]; int renderRequestCount; List_801E733C renderRequests; int auxiliaryRequestCount; List_801E733C auxiliaryRequests; char pad2[0x18]; unsigned char active; };

// Insert a render/preload record and, for type 5, an auxiliary resource pair.
// name is optional and must fit 32 bytes including NUL. Pointers are copied,
// with no retain or deep copy here; callers must keep their targets usable.
// scalarParameter is forwarded unchanged; distanceLimit is squared for the record.
// The manager owns both new nodes. active is published only after both insertions.
void ResourcePreloadQueue_AddRequest(Object_801E733C* obj, int type, void* resourcePointer0, float scalarParameter, void* resourcePointer1, float distanceLimit, const char* name, void* context, unsigned int renderFlags, unsigned int flags) {
	// Phase 1: flatten caller inputs into the 0x48-byte payload and derive flags.
	Record_801E733C rec;
	rec.name.bytes[0] = 0;
	rec.b3c = 0;
	rec.flags = 0;
	rec.b38 = 0xff;
	rec.b39 = 1;
	rec.resourceType = (void*)type;
	rec.resourcePointer0 = resourcePointer0;
	rec.resourcePointer1 = resourcePointer1;
	rec.context = context;
	rec.scalarParameter = scalarParameter;
	rec.requestFlags = flags;
	rec.squaredDistanceLimit = distanceLimit * distanceLimit;
	if (name != 0) {
		strcpy((char*)rec.name.bytes, name);
		if (String_CompareCString(name, courseGlowEffectStrings) == 0 || String_CompareCString(name, (char*)courseGlowEffectStrings + SCENE_LAYOUT_OFFSET(CourseGlowRequestStrings, wizardHouseAnimation)) == 0)
			renderFlags |= kNamedGlowRenderFlag;
	} else rec.name.bytes[0] = 0;
	switch (type) {
	case kSceneRequestKind1: renderFlags |= kRenderFlagsType1Or5Or6; break;
	case kSceneRequestKind2: break;
	case kSceneRequestKind3: renderFlags |= kRenderFlagsType3; break;
	case kSceneRequestKind4: renderFlags |= kRenderFlagsType4; break;
	case kSceneRequestKind5WithAuxiliaryPair: case kSceneRequestKind6: renderFlags |= kRenderFlagsType1Or5Or6; break;
	case kSceneRequestKind0: break;
}
	rec.flags = renderFlags;
	// Phase 2: allocate/copy before linking; the guard owns the node until splice.
	List_801E733C* renderRequests = &obj->renderRequests;
	Pair_801E733C auxiliaryPair;
	{
	Tail_801E733C renderNodeOwner;
	Anchor_801E733C renderIteratorPair;
	Flag_801E733C renderAllocatorFlag;
	ListNode_801E733C* renderNode = (ListNode_801E733C*)appMallocDummy__FUl(kRenderRequestNodeBytes);
	Record_801E733C* renderPayload = (Record_801E733C*)((char*)renderNode + 8);
	renderIteratorPair.a = renderNode; renderIteratorPair.b = renderNode; renderNodeOwner.head = &obj->renderRequests; renderNodeOwner.n = renderNode;
	if (renderPayload != 0) {
		try {
			renderPayload->resourceType = rec.resourceType; renderPayload->context = rec.context; renderPayload->scalarParameter = rec.scalarParameter;
			renderPayload->resourcePointer0 = rec.resourcePointer0; renderPayload->resourcePointer1 = rec.resourcePointer1;
			renderPayload->name = rec.name;
			renderPayload->flags = rec.flags; renderPayload->b38 = rec.b38; renderPayload->b39 = rec.b39;
			renderPayload->b3c = rec.b3c; renderPayload->requestFlags = rec.requestFlags; renderPayload->squaredDistanceLimit = rec.squaredDistanceLimit;
		} catch (...) { appFreeUni__FPv(renderNode); throw; }
	}
	// Append between the old last node and sentinel, then transfer guard ownership.
	renderRequests->lastNode->next = renderNode;
	renderNode->previous = renderRequests->lastNode;
	renderRequests->lastNode = renderNode;
	renderNode->next = (ListNode_801E733C*)renderRequests;
	obj->renderRequestCount++;
	renderNodeOwner.n = 0;
	}
	// Phase 3: type 5 also inserts (type,resourcePointer1) into the second list.
	if (type == kSceneRequestKind5WithAuxiliaryPair) {
		List_801E733C* auxiliaryRequests = &obj->auxiliaryRequests;
		Tail_801E733C auxiliaryNodeOwner;
		Anchor_801E733C auxiliaryIteratorPair;
		auxiliaryPair.a = (void*)type;
		auxiliaryPair.b = resourcePointer1;
		Flag_801E733C auxiliaryAllocatorFlag;
		ListNode_801E733C* auxiliaryNode = (ListNode_801E733C*)appMallocDummy__FUl(kAuxiliaryRequestNodeBytes);
		Pair_801E733C* auxiliaryPayload = (Pair_801E733C*)((char*)auxiliaryNode + 8);
		auxiliaryIteratorPair.a = auxiliaryNode; auxiliaryIteratorPair.b = auxiliaryNode; auxiliaryNodeOwner.head = &obj->auxiliaryRequests; auxiliaryNodeOwner.n = auxiliaryNode;
		if (auxiliaryPayload != 0) {
			try {
				*auxiliaryPayload = auxiliaryPair;
			} catch (...) { appFreeUni__FPv(auxiliaryNode); throw; }
		}
		auxiliaryRequests->lastNode->next = auxiliaryNode;
		auxiliaryNode->previous = auxiliaryRequests->lastNode;
		auxiliaryRequests->lastNode = auxiliaryNode;
		auxiliaryNode->next = (ListNode_801E733C*)auxiliaryRequests;
		obj->auxiliaryRequestCount++;
		auxiliaryNodeOwner.n = 0;
	}
	obj->active = 1;
}

} // extern "C"
