#include "MSL/vector.h"
/* Element type of the vector released at the end of GolfDeclarations_dtor is not known; a record stand-in. */
struct E8013 { unsigned int w[1]; };
/* The vector destructor reads its size twice for the reset (form 2). */
// Destroy no element payload: these records contain no recovered owned members.
namespace std {
template <> struct __vec_destroys_elements<E8013> {
    enum { value = 2 };
    // Perform no per-record cleanup; only the vector allocation is owned.
    static void destroy(E8013*) {}
};
}
typedef std::vector<E8013, app::app_allocator<E8013> > V8013;
#include "iff/IffStruct.h"

// This merged unit covers document ownership (ns0), turn/camera/snapshot logic
// (ns1), array/XML helpers (ns2), and parameter parsing/player setup (ns3).
// Editing guide: player snapshots have a 0x1C8 stride; live player state begins
// at document +0xCC; shared player records have a different 0x1D4 stride.
// Do not exchange these offsets or turn the ordered copies into memcpy: padding,
// byte accesses, and exception cleanup are part of the existing object code.
// A positive deleting/freeMemory argument frees self; zero or negative arguments
// perform member cleanup only. The returned address may already have been freed.
struct GolfParameterXmlStringPool {
    char materialLineFormat[12];
    char roundLineFormat[15];
    char round[6];
    char project[8];
    char wep[4];
    char name[5];
    char declare[8];
    char count[6];
    char classTag[6];
    char defaultName[8];
    char green[6];
    char fairway[8];
    char bunker[7];
    char rough[6];
    char snow[5];
    char road[5];
    char ice[4];
    char water[6];
    char startPoint[12];
    char sand[5];
    char special[8];
    char wb[3];
    char waveExtension[5];
    char textures[9];
    char item[5];
    char vectorLengthError[20];
    char lengthErrorType[55];
    char treeInsertError[26];
    char stringReserveError[35];
    char stringRangeError[27];
    char rangeErrorType[55];
    char stringLengthError[27];
    char golfDocClass[9];
};

// Layout-only views: preserve the existing allocations and CodeWarrior copy/cleanup order.
struct GolfCameraStorage {
    char prefix[0x304]; float followDistance, followHeight, groundClearance; float ballPosition[3];
    float heading; float reserved320; float cameraMatrix[12]; float pitch;
};
enum GolfCameraOffsets {
    kCamera_followDistance = IFF_OFFSET(GolfCameraStorage, followDistance),
    kCamera_followHeight = IFF_OFFSET(GolfCameraStorage, followHeight),
    kCamera_ballPosition = IFF_OFFSET(GolfCameraStorage, ballPosition),
    kCamera_ballPosition_1 = IFF_OFFSET(GolfCameraStorage, ballPosition[1]),
    kCamera_cameraMatrix = IFF_OFFSET(GolfCameraStorage, cameraMatrix),
    kCamera_cameraMatrix_6 = IFF_OFFSET(GolfCameraStorage, cameraMatrix[6]),
    kCamera_cameraMatrix_9 = IFF_OFFSET(GolfCameraStorage, cameraMatrix[9]),
    kCamera_cameraMatrix_10 = IFF_OFFSET(GolfCameraStorage, cameraMatrix[10]),
    kCamera_cameraMatrix_11 = IFF_OFFSET(GolfCameraStorage, cameraMatrix[11]),
};

struct GolfCameraTransform { float right[3], up[3], forward[3], position[3]; };
struct GolfInstanceCursor { void* first; void* current; char reserved08[0x18]; char mutex[0x18]; };
struct GolfInstanceEntry { void* payload; char reserved04[0xC]; void* next; };
struct GolfCommandLinks { void* previous; void* next; };
struct GolfCommandList { unsigned int size; GolfCommandLinks sentinel; };
struct GolfDocOwnedMembers {
    char prefix[0x14]; char declarations[0x1C]; char stringParameters[0x10];
    char plainParameters[0x10]; char playerParameters[0x10]; char instances[0x3C];
    char commands[0x20]; char path[0xC];
};

struct GolfTreeLinks { void* left; void* right; void* parent; };
// Child links of a tree node held as a void* (every node starts with GolfTreeLinks).
#define TREE_LEFT(node) (((GolfTreeLinks*)(node))->left)
#define TREE_RIGHT(node) (((GolfTreeLinks*)(node))->right)
// Tree nodes have 12 bytes of links, then their payload. Headers own nodes;
// StringMap nodes additionally own key/value heap buffers when flags bit 31 is set.
// Empty-tree iterators use the root-field address as sentinel, not a null node.
// Expanded postorder destructors release descendants before node payload/storage.
// Repeated cleanup passes/null guards are compiler-shaping expansions, not separate
// ownership of the same node; preserve their reset statements when editing them.
struct GolfStringStorage { unsigned int flags; unsigned int length; void* data; };
struct GolfStringTreeNode { GolfTreeLinks links; GolfStringStorage key, value; };
struct GolfDeclarationOwner {
    void* records; unsigned int recordCount; char reserved08[4];
    char nameIndex[4]; void* nameRoot; char reserved14[4]; int defaultDeclarationIndex;
};
struct GolfCourseGeometryOwner { char prefix[0x10]; void* resources; };
struct GolfStringPairStorage { GolfStringStorage key, value; };

struct GolfTreeHeader { unsigned int count; void* root; void* first; void* sentinel; };
struct GolfXmlNode {
    char prefix[8]; int type; void* firstChild; void* lastChild;
    char* text; unsigned int textLength; void* previousSibling; void* nextSibling;
};
struct GolfHoleRecord {
    float teePosition[3]; float cupPosition[3];
    unsigned char par; unsigned char metadata; unsigned char resetFlags[4]; char padding[2];
};
struct GolfBallResetEntry {
    int state; float position[3]; unsigned char firstFlag, pending; char tail[0x12];
};
struct GolfDocRoundStorage {
    char prefix[0x470]; GolfBallResetEntry ballReset[4];
};
struct GolfSnapshotStats {
    char prefix[0xD8]; int powerDistanceOffset, control, controlAtE0, spin, spinAtE8;
};
struct GolfSharedPlayerRecord {
    char prefix[0x44]; void* rankValue; char reserved48[0x44];
    unsigned int sharedSnapshotWords[8]; char reservedAC[0x74];
    void* characterId; char reserved124[4]; unsigned int partIds[24];
    char reserved188; signed char basePower, baseControl, reserved18B, baseSpin;
    char reserved18D[3]; unsigned int auxPartIds[5]; char reserved1A4[0x14];
    void* caddieOrItemId; char reserved1BC[0x10]; void* clubSetId; short trainingBonuses[5];
};
// Player-record offsets above are relative to the singleton base, with a 0x1D4 stride.
// This view starts at the singleton base; players are addressed using that same stride.
struct GolfCourseOrder { char prefix[0x82A]; unsigned char holes[0x100]; };
#define GOLF_LAYOUT_OFF(type, field, offset) typedef char type##_##field##_offset[(IFF_OFFSET(type, field) == offset) ? 1 : -1]
GOLF_LAYOUT_OFF(GolfXmlNode, nextSibling, 0x20);
GOLF_LAYOUT_OFF(GolfSharedPlayerRecord, characterId, 0x120);
GOLF_LAYOUT_OFF(GolfSharedPlayerRecord, partIds, 0x128);
GOLF_LAYOUT_OFF(GolfSharedPlayerRecord, auxPartIds, 0x190);
GOLF_LAYOUT_OFF(GolfSharedPlayerRecord, caddieOrItemId, 0x1B8);
GOLF_LAYOUT_OFF(GolfSharedPlayerRecord, clubSetId, 0x1CC);
typedef char GolfHoleRecord_size[(sizeof(GolfHoleRecord) == 0x20) ? 1 : -1];
typedef char GolfBallResetEntry_size[(sizeof(GolfBallResetEntry) == 0x24) ? 1 : -1];
#undef GOLF_LAYOUT_OFF
// Two different player-record strides exist: kPlayerSnapshotBytes (0x1C8) is one saved per-player snapshot
// (GolfDocSnapshot8013, live copy at document +0xCC); kSharedPlayerStride (0x1D4) is one record of the shared
// player array (GolfSharedPlayerRecord). Do not mix them. kHoleResetFlagsOffset/kUnrolledHolesPerIteration are
// codegen helpers for the ball-reset loop (the compiler's 3x unrolling); kRoundHoleCount is holes per round.
enum GolfDocConstants {
    kHoleResetFlagsOffset = IFF_OFFSET(GolfHoleRecord, resetFlags), kUnrolledHolesPerIteration = 3,
    kInstanceMutexOffset = 0x20, kDeclarationDefaultIndexOffset = IFF_OFFSET(GolfDeclarationOwner, defaultDeclarationIndex),
    kDeclarationNameIndexOffset = IFF_OFFSET(GolfDeclarationOwner, nameIndex),
    kTreeRootOffset = 4, kRoundHoleCount = 18, kPlayerSnapshotBytes = 0x1C8, kSharedPlayerStride = 0x1D4, kHoleRecordBytes = 0x20,
    kArrayAllocationHeaderBytes = 0x10, kPartSlotCount = 24, kStatPower = 0, kStatControl = 1, kStatSpin = 3,
    kPowerDistanceBias = 15, kMaxPowerStat = 100, kMaxOtherStat = 30,
    kMinimumSpinStat = 1, kPlayerWithdrawnState = 3, kPlayerHoledOutState = 1, kUsePresetStats = 1,
    kMatchRule = 1, kMatchGameMode = 1, kXmlElementNode = 1, kResourceAllGroups = 0xFFFF
};

#include "classes/WView.h"
#include "classes/GolfDocSnapshot8013.h"
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "game/string.h"
#include "game/wvector.h"

#include "game/types.h"
#include "sdk_protos.h"
#include "classes/CGolfDoc.h"
#include <decomp/cw_bitfields.h>

#define AsCGolfDoc(p) ((CGolfDoc*)(p))

extern "C" void* __nwa__FUl(unsigned int);
// Forward array allocations to the original runtime allocator.
inline void* operator new[](size_t size) { return __nwa__FUl(size); }

struct WInstanceLock {
	void *lock;
	// Acquire the borrowed instance-table mutex for the current scope.
	WInstanceLock(void *l) : lock(l) { OSLockMutex((OSMutex*)l); }
	// Release one nesting level of the recursive OS mutex.
	~WInstanceLock() { OSUnlockMutex((OSMutex*)lock); }
};

// ---- merged from golf_doc_turns_and_parameter_xml.cpp ----
namespace ns0 {
// Translation unit 0x8013246C-0x80133BB8 (13 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

extern void *g_sharedDoc;
extern unsigned int g_sceneManager;

extern void OSLockMutex(void *);
extern void OSUnlockMutex(void *);
extern void appFreeUni__FPv(void *);
extern void PlayerPlainTree_DestroyRootAndChildren(void *, void *);
extern void PlayerPlainTree_DestroySubtree(void *, void *);
extern void StringMap_FreeSubtree(void *, void *);
extern void StringMap_DestroyStringPair(void *, void *);
extern void CGolfDoc_EmptyCleanupHook();
extern void CPvsFxBox_Dtor(void *, int);
extern void GolfAimPath_dtor(void *, int);
extern void __dla__FPv(void *);
extern void *InstanceTable_ResetAndGetFirst(void *);
extern void *SceneManager_GetCurrent();
extern void ResourceManager_ClearCachedModels(void *);

extern void ResourceHashStorage_dtor(void *, int);
extern void *GolfDoc_DestroyPlayerParamTree(void *, int);
extern void *GolfPlainParameterTree_dtor(void *, int);
extern void *StringPairTree_dtor(void *, int);
extern void *GolfCommandList_dtor(void *, int);
extern void GolfDeclarations_dtor(void *, int);
extern void CTaskDoc_dtor(void *, int);
extern void __dl__FPv(void *);
extern char __vt__8CGolfDoc[];
extern void *g_resourceManager;
extern void *InstanceTable_GetNext(void *);
extern void InstanceTable_RecycleAll(void *, int);
extern void CTaskDoc_Ctor(void *);
extern void OSInitMutex(void *);
extern void OSLockMutex(void *);
extern void OSUnlockMutex(void *);
extern void *appMallocUni__FUl(unsigned int);
extern void *__nwa__FUl(unsigned int);
extern void *CSceneManager_Ctor(void *);
extern void *WPolySoup_Ctor(void *, void *);
extern void ResourcePreloadQueue_AddRequest(void *, int, int, float, int, float, int, int, int, int);
extern void DriveLineMarkers_Clear(void *);
extern void ItemSelection_Init(void *);
extern void *GolfHoleRecord_Ctor(void *);
extern void *__construct_new_array(void *, void *, void *, unsigned int, unsigned int);
extern void * __nw__FUl(unsigned int);
extern void *g_fx;
extern const float kFloat_3_GolfDoc, kFloat_0_34906584_GolfDoc, kFloat_1_GolfDoc, kFloat_0_GolfDoc, kFloat_15_GolfDoc;
extern void *memset(void *, int, unsigned int);

#pragma opt_lifetimes off
// Initialize round state, player containers, hole records, scene manager and course geometry with unwind cleanup.
void *CGolfDoc_Ctor(void *self) {
	char *p = (char *)self;
	char *lock;
	int zero = 0;
	void *obj;
	int eight = 8;
	int *slots;
	// Phase 1: construct the base, tree headers and mutex-protected instance buckets.
	CTaskDoc_Ctor(self);
	bool membersReady = false;
	bool tableReady = false;
	bool listReady = false;
	try {
	*(void **)p = __vt__8CGolfDoc;
	(*(int*)&AsCGolfDoc(p)->courseData) = zero; (*(int*)&AsCGolfDoc(p)->courseEffectBox) = zero; (*(int*)&AsCGolfDoc(p)->aimPath) = zero;
	AsCGolfDoc(p)->terrainDeclarations = zero; AsCGolfDoc(p)->terrainDeclarationCount = zero; AsCGolfDoc(p)->terrainDeclarationCapacity = zero;
	AsCGolfDoc(p)->terrainAliasCount = zero; AsCGolfDoc(p)->terrainAliasTreeRoot = zero; AsCGolfDoc(p)->terrainAliasTreeSentinel = p + 0x24;
	AsCGolfDoc(p)->defaultTerrainDeclarationIndex = -1; AsCGolfDoc(p)->equipmentStringMapCount = zero; AsCGolfDoc(p)->equipmentStringMapRoot = zero;
	AsCGolfDoc(p)->equipmentStringMapSentinel = p + 0x34; AsCGolfDoc(p)->beachParameterTreeCount = zero; AsCGolfDoc(p)->beachParameterTreeRoot = zero;
	AsCGolfDoc(p)->beachParameterTreeSentinel = p + 0x44; AsCGolfDoc(p)->unk_50 = zero; AsCGolfDoc(p)->unk_54 = zero;
	AsCGolfDoc(p)->unk_5C = p + 0x54; (*(int*)&AsCGolfDoc(p)->instanceSlots) = zero;
	membersReady = true;
	OSInitMutex(p + 0x80);
	lock = p + 0x80;
	OSLockMutex(lock);
	struct LockCleanup {
		char* mutex;
		bool active;
		// Unlock only until the explicit constructor unlock disarms this guard.
		~LockCleanup() { if (active) OSUnlockMutex(mutex); }
	} lockCleanup = {lock, true};
	AsCGolfDoc(p)->instanceListHead=0; AsCGolfDoc(p)->recycledInstanceListHead=0; AsCGolfDoc(p)->unk_68=0;
	AsCGolfDoc(p)->instanceInitialBucketCount=8; AsCGolfDoc(p)->instanceIterator=0; AsCGolfDoc(p)->instanceSlotCount=8;
	if (AsCGolfDoc(p)->instanceInitialBucketCount > 0) {
		slots = (int *)appMallocUni__FUl(0x20);
		AsCGolfDoc(p)->instanceSlots = slots;
		AsCGolfDoc(p)->instanceBucketMask = AsCGolfDoc(p)->instanceSlotCount - 1;
		int i = 0;
		int off = 0;
		for (; i < AsCGolfDoc(p)->instanceSlotCount; i++) { *(int *)((char *)AsCGolfDoc(p)->instanceSlots + off) = 0; off += 4; }
	} else {
		AsCGolfDoc(p)->instanceSlots = 0;
	}
	lockCleanup.active = false;
	OSUnlockMutex(lock);
	tableReady = true;
	// Phase 2: establish empty command/string/array owners before scene allocations.
	AsCGolfDoc(p)->shotCommandCount=0; char *list = p+0xA0; AsCGolfDoc(p)->shotCommandTail=list; AsCGolfDoc(p)->shotCommandHead=list;
	AsCGolfDoc(p)->tutorialMarkerModel=0; AsCGolfDoc(p)->tutorialMarkerVisible=0;
	AsCGolfDoc(p)->parameterXmlPathFlags=0; AsCGolfDoc(p)->parameterXmlPathLength=0; AsCGolfDoc(p)->parameterXmlPathData=0; (*(int*)&AsCGolfDoc(p)->playerSnapshots)=0;
	listReady = true;
	AsCGolfDoc(p)->unk_184=0; (*(int*)&AsCGolfDoc(p)->holeRecords)=0; AsCGolfDoc(p)->gameStateFlags=0;
	AsCGolfDoc(p)->unk_35C=0; AsCGolfDoc(p)->unk_35E=0;
	DriveLineMarkers_Clear(p+0x36C);
	char *q = p+0x53C;
	do { ItemSelection_Init(q); q += 0x68; } while (q < p+0x6DC);
	(*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc)=self;
	AsCGolfDoc(p)->cameraMode=0; AsCGolfDoc(p)->tutorialStage=6;
	AsCGolfDoc(p)->activePlayerCount=0; AsCGolfDoc(p)->playerCount=0;
	AsCGolfDoc(p)->shotPhase=0; AsCGolfDoc(p)->currentPlayer=0; AsCGolfDoc(p)->unk_2C0=0xFF;
	AsCGolfDoc(p)->cameraGroundClearance=kFloat_3_GolfDoc; AsCGolfDoc(p)->unk_320=kFloat_0_34906584_GolfDoc;
	AsCGolfDoc(p)->bright=kFloat_1_GolfDoc; AsCGolfDoc(p)->headingRadians=kFloat_0_GolfDoc;
	AsCGolfDoc(p)->unk_35D=0; AsCGolfDoc(p)->powerShotMeterOverride=0; AsCGolfDoc(p)->unk_35F=0;
	AsCGolfDoc(p)->unk_3C9=0; AsCGolfDoc(p)->dropRequest=0; AsCGolfDoc(p)->maxGrade=0; AsCGolfDoc(p)->unk_3CB=0;
	AsCGolfDoc(p)->shotMeterFadeResetRequested=0; AsCGolfDoc(p)->unk_3CC=0; AsCGolfDoc(p)->unk_361=1;
	AsCGolfDoc(p)->unk_364=-1; AsCGolfDoc(p)->unk_3D0=kFloat_1_GolfDoc; AsCGolfDoc(p)->unk_3D4=kFloat_15_GolfDoc;
	AsCGolfDoc(p)->isDaytime=1; AsCGolfDoc(p)->unk_3D8=-1; AsCGolfDoc(p)->unk_3DC=-1;
	AsCGolfDoc(p)->unk_3E0=0; AsCGolfDoc(p)->unk_3E4=0; AsCGolfDoc(p)->unk_3F4=0; AsCGolfDoc(p)->unk_3E8=0;
	AsCGolfDoc(p)->unk_3EC=0; AsCGolfDoc(p)->unk_3F0=0; AsCGolfDoc(p)->unk_3F8=0;
	// Phase 3: allocate eighteen owned hole records, a scene manager, and course geometry.
	// Arrays return the element pointer; delete[] receives its preceding 0x10-byte header.
	if (AsCGolfDoc(p)->holeRecords) { if (AsCGolfDoc(p)->holeRecords) __dla__FPv((char *)AsCGolfDoc(p)->holeRecords-0x10); (*(int*)&AsCGolfDoc(p)->holeRecords)=0; }
	obj=__nwa__FUl(0x250);
	if (obj) obj=__construct_new_array(obj, (void *)GolfHoleRecord_Ctor, 0, 0x20, 0x12);
	AsCGolfDoc(p)->holeRecords=obj;
	memset(obj,0,0x240);
	// Construct a scene manager in its original allocation size with new-expression unwind behavior.
	struct RealNew_fn_8013246C_0 {
        char storage[0x16C];
        // Initialize the scene manager in its original-sized allocation.
        RealNew_fn_8013246C_0() { CSceneManager_Ctor(this); }
    };
obj = new RealNew_fn_8013246C_0();
if (g_fx) ResourcePreloadQueue_AddRequest((void *)g_sceneManager,6,0,kFloat_1_GolfDoc,(int)g_fx,kFloat_0_GolfDoc,0,0,0,0);
	obj=__nw__FUl(0x4C90); if (obj) {
	    try {
	        WPolySoup_Ctor(obj,self);
	    } catch (...) {
	        __dl__FPv(obj);
	        throw;
	    }
	}
	AsCGolfDoc(p)->courseData=obj; ((GolfCourseGeometryOwner*)obj)->resources=g_resourceManager;
	memset(p+0x410,0,0x60); memset(p+0x470,0,0x90); memset(p+0x500,0,0x34);
	memset(p+0x534,0,8); memset(p+0xCC,0,0x1C8); memset(p+0x294,0,0x20);
	AsCGolfDoc(p)->alternateMotionPowerDecided=0; AsCGolfDoc(p)->motionSwingState=0;
	return self;
	} catch (...) {
		// Unwind only established owners in reverse order; the base is always constructed.
		if (listReady) {
			if (*(unsigned int*)(p + 0xBC) >> 31)
				appFreeUni__FPv(*(void**)(p + 0xC4));
			GolfCommandList_dtor(p + 0x9C, -1);
		}
		if (tableReady) ResourceHashStorage_dtor(p + 0x60, -1);
		if (membersReady) {
			GolfDoc_DestroyPlayerParamTree(p + 0x50, -1);
			GolfPlainParameterTree_dtor(p + 0x40, -1);
			StringPairTree_dtor(p + 0x30, -1);
			GolfDeclarations_dtor(p + 0x14, -1);
		}
		CTaskDoc_dtor(self, -1);
		throw;
	}
}
#pragma opt_lifetimes reset

// The golf-document cleanup hook is empty.
void CGolfDoc_EmptyCleanupHook() {}

// Release a non-null plain-tree leaf after its recursively owned descendants.
static inline void FreePlainPlayerLeaf(void* owner, void* node) {
    if (TREE_LEFT(node)) PlayerPlainTree_DestroySubtree(owner, TREE_LEFT(node));
    if (TREE_RIGHT(node)) PlayerPlainTree_DestroySubtree(owner, TREE_RIGHT(node));
    appFreeUni__FPv(node);
}

// Release the other plain-tree specialization using its own recursive entry point.
static inline void FreeOtherPlainPlayerLeaf(void* owner, void* node) {
    if (TREE_LEFT(node)) PlayerPlainTree_DestroyRootAndChildren(owner, TREE_LEFT(node));
    if (TREE_RIGHT(node)) PlayerPlainTree_DestroyRootAndChildren(owner, TREE_RIGHT(node));
    appFreeUni__FPv(node);
}

#pragma push
#pragma use_lmw_stmw on
// Finish the next expanded level without changing left-before-right destruction.
// Borrow caller scratch links so inlining retains the original register lifetimes.
static inline void FreePlainPlayerBranch(void* owner, void* node,
    void*& leftChild, void*& rightChild) {
    { leftChild = TREE_LEFT(node);
        if (leftChild) { FreePlainPlayerLeaf(owner, leftChild); }
    }
    { rightChild = TREE_RIGHT(node);
        if (rightChild) { FreePlainPlayerLeaf(owner, rightChild); }
    }
    appFreeUni__FPv(node);
}

// Finish the next expanded level without changing left-before-right destruction.
static inline void FreeOtherPlainPlayerBranch(void* owner, void* node,
    void*& leftChild, void*& rightChild) {
    { leftChild = TREE_LEFT(node);
        if (leftChild) { FreeOtherPlainPlayerLeaf(owner, leftChild); }
    }
    { rightChild = TREE_RIGHT(node);
        if (rightChild) { FreeOtherPlainPlayerLeaf(owner, rightChild); }
    }
    appFreeUni__FPv(node);
}

// Free the plain player tree in the original expanded postorder, then optionally delete its header.
// In plain C: free every node of the tree (children before parents), reset the header to empty
// (count 0, root 0, first = &root), then delete the header block if deleting > 0. The nested
// levels and inline helpers preserve recursive node-free inlining; names read as paths from the root
// (nodeAL = root's left child, nodeALR = that node's right child, and so on).
void *GolfDoc_DestroyPlayerParamTree(void *arg0, int deleting) {
    // Phase 1: postorder clear/reset. A/B suffixes below distinguish cleanup
    // passes; L/R suffixes give the descent path from that pass's root.
    char *self=(char *)arg0;
    if(self) {
        { void *nodeA = ((GolfTreeHeader*)self)->root;
            if (nodeA) {
                { void *nodeAL = TREE_LEFT(nodeA);
                    if (nodeAL) {
                        { void *nodeALL = TREE_LEFT(nodeAL);
                            if (nodeALL) {
                                void* nodeALLL;
                                void* nodeALLR;
                                FreePlainPlayerBranch(self, nodeALL, nodeALLL, nodeALLR);
                            }
                        }
                        { void *nodeALR = TREE_RIGHT(nodeAL);
                            if (nodeALR) {
                                void* nodeALRL;
                                void* nodeALRR;
                                FreePlainPlayerBranch(self, nodeALR, nodeALRL, nodeALRR);
                            }
                        }
                        appFreeUni__FPv(nodeAL);
                    }
                }
                { void *nodeAR = TREE_RIGHT(nodeA);
                    if (nodeAR) {
                        { void *nodeARL = TREE_LEFT(nodeAR);
                            if (nodeARL) {
                                void* nodeARLL;
                                void* nodeARLR;
                                FreePlainPlayerBranch(self, nodeARL, nodeARLL, nodeARLR);
                            }
                        }
                        { void *nodeARR = TREE_RIGHT(nodeAR);
                            if (nodeARR) {
                                void* nodeARRL;
                                void* nodeARRR;
                                FreePlainPlayerBranch(self, nodeARR, nodeARRL, nodeARRR);
                            }
                        }
                        appFreeUni__FPv(nodeAR);
                    }
                }
                appFreeUni__FPv(nodeA);
                (*(void**)&((GolfTreeHeader*)self)->count)=0; ((GolfTreeHeader*)self)->root=0; ((GolfTreeHeader*)self)->sentinel=self + kTreeRootOffset;
            }
        }
        // Phase 2: member-destruction expansion rereads the now-empty root;
        // the first clear above has already reset count/root/sentinel.
        if (self) if (self) if (self) if (self) if (self) if (self) {
            { void *nodeB = ((GolfTreeHeader*)self)->root;
                if (nodeB) {
                    { void *nodeBL = TREE_LEFT(nodeB);
                        if (nodeBL) {
                            { void *nodeBLL = TREE_LEFT(nodeBL);
                                if (nodeBLL) {
                                    void* nodeBLLL;
                                    void* nodeBLLR;
                                    FreePlainPlayerBranch(self, nodeBLL, nodeBLLL, nodeBLLR);
                                }
                            }
                            { void *nodeBLR = TREE_RIGHT(nodeBL);
                                if (nodeBLR) {
                                    void* nodeBLRL;
                                    void* nodeBLRR;
                                    FreePlainPlayerBranch(self, nodeBLR, nodeBLRL, nodeBLRR);
                                }
                            }
                            appFreeUni__FPv(nodeBL);
                        }
                    }
                    { void *nodeBR = TREE_RIGHT(nodeB);
                        if (nodeBR) {
                            { void *nodeBRL = TREE_LEFT(nodeBR);
                                if (nodeBRL) {
                                    void* nodeBRLL;
                                    void* nodeBRLR;
                                    FreePlainPlayerBranch(self, nodeBRL, nodeBRLL, nodeBRLR);
                                }
                            }
                            { void *nodeBRR = TREE_RIGHT(nodeBR);
                                if (nodeBRR) {
                                    void* nodeBRRL;
                                    void* nodeBRRR;
                                    FreePlainPlayerBranch(self, nodeBRR, nodeBRRL, nodeBRRR);
                                }
                            }
                            appFreeUni__FPv(nodeBR);
                        }
                    }
                    appFreeUni__FPv(nodeB);
                }
            }
        }
        if(deleting>0) __dl__FPv(arg0);
    }
    return arg0;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Free the second plain player tree in expanded postorder, then optionally delete its header.
// Same shape as GolfDoc_DestroyPlayerParamTree: free every node of the tree, reset the header to empty, delete the header if deleting > 0.
void *GolfPlainParameterTree_dtor(void *arg0, int deleting) {
    // Phase 1: postorder clear/reset; L/R suffixes encode each node's tree path.
    char *self=(char *)arg0;
    if(self) {
        { void *nodeA = ((GolfTreeHeader*)self)->root;
            if (nodeA) {
                { void *nodeAL = TREE_LEFT(nodeA);
                    if (nodeAL) {
                        { void *nodeALL = TREE_LEFT(nodeAL);
                            if (nodeALL) {
                                void* nodeALLL;
                                void* nodeALLR;
                                FreeOtherPlainPlayerBranch(self, nodeALL, nodeALLL, nodeALLR);
                            }
                        }
                        { void *nodeALR = TREE_RIGHT(nodeAL);
                            if (nodeALR) {
                                void* nodeALRL;
                                void* nodeALRR;
                                FreeOtherPlainPlayerBranch(self, nodeALR, nodeALRL, nodeALRR);
                            }
                        }
                        appFreeUni__FPv(nodeAL);
                    }
                }
                { void *nodeAR = TREE_RIGHT(nodeA);
                    if (nodeAR) {
                        { void *nodeARL = TREE_LEFT(nodeAR);
                            if (nodeARL) {
                                void* nodeARLL;
                                void* nodeARLR;
                                FreeOtherPlainPlayerBranch(self, nodeARL, nodeARLL, nodeARLR);
                            }
                        }
                        { void *nodeARR = TREE_RIGHT(nodeAR);
                            if (nodeARR) {
                                void* nodeARRL;
                                void* nodeARRR;
                                FreeOtherPlainPlayerBranch(self, nodeARR, nodeARRL, nodeARRR);
                            }
                        }
                        appFreeUni__FPv(nodeAR);
                    }
                }
                appFreeUni__FPv(nodeA);
                (*(void**)&((GolfTreeHeader*)self)->count)=0; ((GolfTreeHeader*)self)->root=0; ((GolfTreeHeader*)self)->sentinel=self + kTreeRootOffset;
            }
        }
        // Phase 2: preserve the member-destruction expansion after the explicit
        // clear has reset the header. Only a positive deleting argument frees self.
        if(self) if(self) if(self) if(self) if(self) if(self) {
            { void *nodeB = ((GolfTreeHeader*)self)->root;
                if (nodeB) {
                    { void *nodeBL = TREE_LEFT(nodeB);
                        if (nodeBL) {
                            { void *nodeBLL = TREE_LEFT(nodeBL);
                                if (nodeBLL) {
                                    void* nodeBLLL;
                                    void* nodeBLLR;
                                    FreeOtherPlainPlayerBranch(self, nodeBLL, nodeBLLL, nodeBLLR);
                                }
                            }
                            { void *nodeBLR = TREE_RIGHT(nodeBL);
                                if (nodeBLR) {
                                    void* nodeBLRL;
                                    void* nodeBLRR;
                                    FreeOtherPlainPlayerBranch(self, nodeBLR, nodeBLRL, nodeBLRR);
                                }
                            }
                            appFreeUni__FPv(nodeBL);
                        }
                    }
                    { void *nodeBR = TREE_RIGHT(nodeB);
                        if (nodeBR) {
                            { void *nodeBRL = TREE_LEFT(nodeBR);
                                if (nodeBRL) {
                                    void* nodeBRLL;
                                    void* nodeBRLR;
                                    FreeOtherPlainPlayerBranch(self, nodeBRL, nodeBRLL, nodeBRLR);
                                }
                            }
                            { void *nodeBRR = TREE_RIGHT(nodeBR);
                                if (nodeBRR) {
                                    void* nodeBRRL;
                                    void* nodeBRRR;
                                    FreeOtherPlainPlayerBranch(self, nodeBRR, nodeBRRL, nodeBRRR);
                                }
                            }
                            appFreeUni__FPv(nodeBR);
                        }
                    }
                    appFreeUni__FPv(nodeB);
                }
            }
        }
        if(deleting>0) __dl__FPv(arg0);
    }
    return arg0;
}
#pragma pop

// Return the shared-document singleton.
void *SharedDoc_GetCurrent() {
	return g_sharedDoc;
}

// Initialize the snapshot word cleared by the player-snapshot array constructor.
void GolfDocSnapshot_Ctor(void *arg0) {
	((GolfDocSnapshot8013*)arg0)->unk_B8 = 0;
}

// Report whether the scene-manager singleton exists.
int SceneManager_IsInitialized() {
	return g_sceneManager != 0;
}

// Unlink and free every queued shot-command node, then optionally delete the list header.
void *GolfCommandList_dtor(void *arg0, int deleting) {
	if (arg0 != 0) {
		if (arg0 != 0 && arg0 != 0 && arg0 != 0) {
		void *sentinel = &((GolfCommandList*)arg0)->sentinel;
		void *node = ((GolfCommandList*)arg0)->sentinel.next;
		if (node != sentinel) {
			void *fv = ((GolfCommandLinks*)sentinel)->previous;
			void *lv = ((GolfCommandLinks*)node)->previous;
			void *f4 = ((GolfCommandLinks*)fv)->next;
			((GolfCommandLinks*)lv)->next = f4;
			void *lv2 = ((GolfCommandLinks*)node)->previous;
			void *f4b = ((GolfCommandLinks*)fv)->next;
			((GolfCommandLinks*)f4b)->previous = lv2;
			while (node != sentinel) {
				void *cur = node;
				node = ((GolfCommandLinks*)node)->next;
				appFreeUni__FPv(cur);
				((GolfCommandList*)arg0)->size -= 1;
			}
		}
		}
		if (deleting > 0)
			__dl__FPv(arg0);
	}
	return arg0;
}

// Detach the singleton and release course, players, holes, command storage and base-document resources.
void *CGolfDoc_dtor(void *arg0, int deleting) {
	// Phase 1: detach the singleton and release directly owned scene/player/hole objects.
	if (arg0 != 0) {
		*(void **)arg0 = __vt__8CGolfDoc;
		((void(*)())CGolfDoc_EmptyCleanupHook)();
		AsCSharedDoc(((void*(*)())SharedDoc_GetCurrent)())->golfDoc = 0;
		void *p = AsCGolfDoc(arg0)->courseData;
		if (p != 0 && p != 0) {
			(*(void (**)(void *, int))(*(char **)p + 8))(p, 1);
			AsCGolfDoc(arg0)->courseData = 0;
		}
		p = AsCGolfDoc(arg0)->courseEffectBox;
		if (p != 0) { CPvsFxBox_Dtor(p, 1); AsCGolfDoc(arg0)->courseEffectBox = 0; }
		p = AsCGolfDoc(arg0)->aimPath;
		if (p != 0) { GolfAimPath_dtor(p, 1); AsCGolfDoc(arg0)->aimPath = 0; }
		p = AsCGolfDoc(arg0)->playerSnapshots;
		if (p != 0 && p != 0) { __dla__FPv((char *)p - 0x10); AsCGolfDoc(arg0)->playerSnapshots = 0; }
		p = AsCGolfDoc(arg0)->holeRecords;
		if (p != 0 && p != 0) { __dla__FPv((char *)p - 0x10); AsCGolfDoc(arg0)->holeRecords = 0; }
		// Phase 2: free instance payload arrays; recycling below only moves their table nodes.
		p = ((void*(*)(void*))InstanceTable_ResetAndGetFirst)(((GolfDocOwnedMembers*)arg0)->instances);
		while (p != 0) {
			__dla__FPv(p);
			p = ((void*(*)(void*))InstanceTable_GetNext)(((GolfDocOwnedMembers*)arg0)->instances);
		}
		((void(*)(void*,int))InstanceTable_RecycleAll)(((GolfDocOwnedMembers*)arg0)->instances, 0);
		if (((int(*)())SceneManager_IsInitialized)()) {
			p = SceneManager_GetCurrent();
			if (p != 0) (*(void (**)(void *, int))(*(char **)p + 8))(p, 1);
		}
		ResourceManager_ClearCachedModels(g_resourceManager);
		// Phase 3: destroy containers (nodes and owned string buffers), then the document base.
		((NString*)(((GolfDocOwnedMembers*)arg0)->path))->~NString();
		GolfCommandList_dtor(((GolfDocOwnedMembers*)arg0)->commands, -1);
		ResourceHashStorage_dtor(((GolfDocOwnedMembers*)arg0)->instances, -1);
		GolfDoc_DestroyPlayerParamTree(((GolfDocOwnedMembers*)arg0)->playerParameters, -1);
		GolfPlainParameterTree_dtor(((GolfDocOwnedMembers*)arg0)->plainParameters, -1);
		StringPairTree_dtor(((GolfDocOwnedMembers*)arg0)->stringParameters, -1);
		GolfDeclarations_dtor(((GolfDocOwnedMembers*)arg0)->declarations, -1);
		CTaskDoc_dtor(arg0, 0);
		if (deleting > 0) __dl__FPv(arg0);
	}
	return arg0;
}

struct ClearNode {
	ClearNode *root;
	char pad04[8];
	ClearNode *prev;
	ClearNode *next;
	int pad14;
	int count;
	ClearNode **slots;
	char lock[1];
};

#pragma push
#pragma use_lmw_stmw on
// Move active instance-table nodes into its recycled list while holding the table mutex.
// Node/payload storage is retained. Active/recycled rings are circular with null
// for empty, and buckets hold nonowning node pointers. Does not reset every cursor
// field, so callers must restart iteration. The unnamed second argument is ignored.
void InstanceTable_RecycleAll(void *arg0, int) {
	ClearNode *p = (ClearNode *)arg0;
	ClearNode *current;
	WInstanceLock guard(p->lock);
	if (p->root != 0 && (current = p->prev) != 0) {
		p->root->prev->next = p->prev->next;
		p->prev->next->prev = p->root->prev;
		p->prev->next = p->root;
		p->root->prev = p->prev;
		p->root = 0;
	} else if (p->root != 0) {
		ClearNode *node;
		ClearNode *removed;
		ClearNode *successor;
		void *lock = p->lock;
		while ((removed = p->root) != 0) {
			OSLockMutex(lock);
			removed->next->prev = removed->prev;
			removed->prev->next = removed->next;
			successor = removed->next;
			if (successor == removed) successor = 0;
			OSUnlockMutex(lock);
			node = p->root;
			current = p->prev;
			OSLockMutex(lock);
			if (current == 0) {
				node->prev = node;
				node->next = node;
				OSUnlockMutex(lock);
			} else {
				node->next = current;
				node->prev = current->prev;
				current->prev = node;
				node->prev->next = node;
				OSUnlockMutex(lock);
				node = current;
			}
			p->prev = node;
			p->root = successor;
		}
	}
	for (int i = 0; i < p->count; ++i) p->slots[i] = 0;
}
#pragma pop

// Return the cursor's current borrowed payload and advance; null means cursor exhausted.
// Each call locks independently; a multi-call traversal is not an atomic snapshot.
void *InstanceTable_GetNext(void *arg0) {
	void *next;
	void *lock;
	void *node;
	void *result;
	lock = ((GolfInstanceCursor*)arg0)->mutex;
	OSLockMutex(lock);
	node = ((GolfInstanceCursor*)arg0)->current;
	if (node != 0) {
		next = ((GolfInstanceEntry*)node)->next;
		result = ((GolfInstanceEntry*)node)->payload;
		if (next == ((GolfInstanceCursor*)arg0)->first)
			next = 0;
		((GolfInstanceCursor*)arg0)->current = next;
		OSUnlockMutex(lock);
		return result;
	}
	OSUnlockMutex(lock);
	return 0;
}

void InstanceTable_RecycleAll(void *, int);

// Reset to the first node, return its borrowed payload and advance the cursor under the mutex.
void *InstanceTable_ResetAndGetFirst(void *arg0) {
	void *result;
	WInstanceLock guard((char *)arg0 + kInstanceMutexOffset);
	*(void **)((char *)arg0 + 4) = *(void **)arg0;
	void *secondLock = ((ClearNode *)arg0)->lock;
	OSLockMutex(secondLock);
	void *next;
	void *node = *(void **)((char *)arg0 + 4);
	if (node != 0) {
		next = ((GolfInstanceEntry*)node)->next;
		result = *(void **)node;
		if (next == *(void **)arg0)
			next = 0;
		*(void **)((char *)arg0 + 4) = next;
		OSUnlockMutex(secondLock);
	} else {
		OSUnlockMutex(secondLock);
		result = 0;
	}
	return result;
}

#pragma push
#pragma use_lmw_stmw on
// Release a non-null string-map leaf, including both heap-backed key/value buffers.
static inline void FreeGolfStringPairLeaf(void* owner, void* node) {
    if (TREE_LEFT(node)) StringMap_FreeSubtree(owner, TREE_LEFT(node));
    if (TREE_RIGHT(node)) StringMap_FreeSubtree(owner, TREE_RIGHT(node));
    StringMap_DestroyStringPair(owner, &((GolfStringTreeNode*)node)->key);
    appFreeUni__FPv(node);
}

// Free the string-to-string tree, including owned key/value buffers, then optionally delete its header.
// In plain C: free every node (and its heap-backed key/value strings), reset the header to empty, then
// delete the header if deleting > 0. The stacked `if (self != 0)` guards are inlined base-class destructors;
// nodeLL / nodeLLR etc. are paths from the root (L = left child, R = right child).
void *StringPairTree_dtor(void *arg0, int deleting) {
    char *self = (char *)arg0;
    if (self != 0) {
        if (self != 0) {
            if (self != 0) {
                if (self != 0) {
                    if (self != 0) {
                        if (self != 0) {
                            if (self != 0) {
                                void *root = ((GolfTreeHeader*)self)->root;
                                if (root != 0) {
                                    void *nodeL = TREE_LEFT(root);
                                    if (nodeL != 0) {
                                        void *nodeLL = TREE_LEFT(nodeL);
                                        if (nodeLL != 0) {
                                            void *nodeLLL = TREE_LEFT(nodeLL);
                                            if (nodeLLL != 0) {
                                                FreeGolfStringPairLeaf(self, nodeLLL);
                                            }
                                            void *nodeLLR = TREE_RIGHT(nodeLL);
                                            if (nodeLLR != 0) {
                                                FreeGolfStringPairLeaf(self, nodeLLR);
                                            }
                                            StringMap_DestroyStringPair(self, &((GolfStringTreeNode*)nodeLL)->key);
                                            appFreeUni__FPv(nodeLL);
                                        }
                                        void *nodeLR = TREE_RIGHT(nodeL);
                                        if (nodeLR != 0) {
                                            void *nodeLRL = TREE_LEFT(nodeLR);
                                            if (nodeLRL != 0) {
                                                FreeGolfStringPairLeaf(self, nodeLRL);
                                            }
                                            void *nodeLRR = TREE_RIGHT(nodeLR);
                                            if (nodeLRR != 0) {
                                                FreeGolfStringPairLeaf(self, nodeLRR);
                                            }
                                            StringMap_DestroyStringPair(self, &((GolfStringTreeNode*)nodeLR)->key);
                                            appFreeUni__FPv(nodeLR);
                                        }
                                        StringMap_DestroyStringPair(self, &((GolfStringTreeNode*)nodeL)->key);
                                        appFreeUni__FPv(nodeL);
                                    }
                                    void *nodeR = TREE_RIGHT(root);
                                    if (nodeR != 0) {
                                        void *nodeRL = TREE_LEFT(nodeR);
                                        if (nodeRL != 0) {
                                            void *nodeRLL = TREE_LEFT(nodeRL);
                                            if (nodeRLL != 0) {
                                                FreeGolfStringPairLeaf(self, nodeRLL);
                                            }
                                            void *nodeRLR = TREE_RIGHT(nodeRL);
                                            if (nodeRLR != 0) {
                                                FreeGolfStringPairLeaf(self, nodeRLR);
                                            }
                                            StringMap_DestroyStringPair(self, &((GolfStringTreeNode*)nodeRL)->key);
                                            appFreeUni__FPv(nodeRL);
                                        }
                                        void *nodeRR = TREE_RIGHT(nodeR);
                                        if (nodeRR != 0) {
                                            void *nodeRRL = TREE_LEFT(nodeRR);
                                            if (nodeRRL != 0) {
                                                FreeGolfStringPairLeaf(self, nodeRRL);
                                            }
                                            void *nodeRRR = TREE_RIGHT(nodeRR);
                                            if (nodeRRR != 0) {
                                                FreeGolfStringPairLeaf(self, nodeRRR);
                                            }
                                            StringMap_DestroyStringPair(self, &((GolfStringTreeNode*)nodeRR)->key);
                                            appFreeUni__FPv(nodeRR);
                                        }
                                        StringMap_DestroyStringPair(self, &((GolfStringTreeNode*)nodeR)->key);
                                        appFreeUni__FPv(nodeR);
                                    }
                                    if (&((GolfStringTreeNode*)root)->key) {
                                        if (&((GolfStringTreeNode*)root)->value) {
                                            if ((((GolfStringTreeNode*)root)->value.flags >> 31) != 0) appFreeUni__FPv(((GolfStringTreeNode*)root)->value.data);
                                        }
                                    }
                                    if (&((GolfStringTreeNode*)root)->key) {
                                        if ((((GolfStringTreeNode*)root)->key.flags >> 31) != 0) appFreeUni__FPv(((GolfStringTreeNode*)root)->key.data);
                                    }
                                    appFreeUni__FPv(root);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if (deleting > 0) __dl__FPv(arg0);
    return arg0;
}
#pragma pop

} // extern "C"

} // namespace ns0

// The live first-player record has exactly the saved-record layout at document +0xCC.
struct GolfDocLivePlayerStorage {
    char documentPrefix[0xCC];
    GolfDocSnapshot8013 player;
    unsigned int sharedResultWords[8];
};
typedef char GolfDocLivePlayer_shared_offset[(IFF_OFFSET(GolfDocLivePlayerStorage, sharedResultWords) == 0x294) ? 1 : -1];
#define AsLivePlayer(p) (&((GolfDocLivePlayerStorage*)(p))->player)

// Keep both source reads ahead of the writes, including the original volatile byte reads.
template <class T> inline void CopySnapshotPair(T* destination, const volatile T* source) {
    T first = source[0];
    T second = source[1];
    destination[0] = first;
    destination[1] = second;
}
struct SnapshotWordPair { int words[2]; };
// Copy two adjacent result words as one aggregate, retaining the original load/store schedule.
inline void CopySnapshotWords(unsigned int* destination, const unsigned int* source) {
    *(SnapshotWordPair*)destination = *(const SnapshotWordPair*)source;
}

// ---- merged from golf_doc_turns_and_parameter_xml.cpp ----
namespace ns1 {
// Translation unit 0x80133BB8-0x80135160 (8 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

struct Entry80134B {
    // Offset aliases for snapshot +0/+4 (playerId/unk_4 in GolfDocSnapshot8013).
    // `active` is not playerState (+8); zero is the eligibility test used below.
    int snapshotWord0;
    int snapshotWord4;
};

struct Object80134B {
    char pad0[0xC8];
    Entry80134B* entries;
    char padCC[0x1EC];
    unsigned char count;
    char pad2B9[0xFD];
    unsigned char selected;
    char pad3B7[0x0A];
    unsigned char equalResult;
};

struct Global80134B {
    char pad0[0x808];
    unsigned int mode;
    char pad80C[0xB4];
    unsigned char state;
    char pad8C1[0x2CF];
    void* manager;
};

struct Vec3_8013 { float x, y, z; float pad[9]; };
struct Vec4_8013 { float x, y, z, w; };
struct CameraInfo8013 { char pad[0x59]; unsigned char type; };
struct Manager8013 { char pad[0xC8]; void* current; };
extern void* g_view;
extern void* g_inputPointer;
extern GolfCameraTransform s_cameraTransform;
extern float kFloat_15_GolfDoc, kFloat_0_GolfDoc;
void __as__7WVectorFRC7WVector(void*, const void*);
void __pl__FRC7WVectorRC7WVector(void*, const void*, const void*);
void Normalize__7WVectorFv(void*);
void Matrix_CopyAffine(void*, const void*);
void Matrix_BuildCameraTransform(void*, const void*, const void*);
void Matrix_SetAxisRotation(void*, float, int);
void __ml__FfRC7WVector(void*, const void*, float);
void __ami__7WVectorFRC7WVector(void*, const void*);
void* GolfBall__Fv();
void* ItemInventory_FindCharacterData(void*, void*);
void* CQuadTree_GetInstance();
void GetGroundPoint__9CQuadTreeF7WVectorbPib(void*, void*, void*, int, ...);
void Camera_SetFov(void*, float);
void __mi__FRC7WVectorRC7WVector(void*, const void*, const void*);
float Magnitude__7WVectorCFv(void*);
float WVectorLen__FRC7WVector(void*);
int Pointer_GetEnabledBindingDevice(void*, int);
float InstanceTable_Get();
float GolfParameter_ResolveFloat(int);
void Vector_CrossProduct(void*, const void*, const void*);
extern float kFloat_neg1_GolfDoc, kFloat_0_5_GolfDocSnapshot, kFloat_0_75_GolfDocSnapshot, kFloat_480_GolfDocSnapshot, kFloat_3_2_GolfDocSnapshot, kFloat_64_GolfCamera, kFloat_16_GolfDocSnapshot, kFloat_1_GolfDoc, kFloat_0_2_GolfShotDirection, kFloat_32_GolfShotDirection, kFloat_2_GolfDoc, kFloat_0_4_GolfDoc, kFloat_1_4_GolfShotDirection;

extern Global80134B* g_sharedDoc;
struct CopyEntry8013 { unsigned int words[2]; };

// Return the indexed player snapshot, or null when no snapshot array is allocated; index is caller-validated.
void* GolfPlayer_GetEntryByIndex(Object80134B* self, int index) {
    if (self->entries == 0)
        return 0;
    return (char*)self->entries + index * kPlayerSnapshotBytes;
}

// Select the minimum snapshot +0 word below 4 among entries with +4 == 0; default to index 0.
// Returns an array index, not the selected word; ties keep the first entry.
// The gameplay roles of snapshotWord0 and snapshotWord4 remain unresolved.
int CGolfDoc_SelectPlayerBySnapshotWords(Object80134B* self) {
    Entry80134B* entries = self->entries;
    int result = 0;
    if (entries != 0) {
        int best = 4;
        for (int i = 0; i < self->count; i++) {
            Entry80134B* entry = (Entry80134B*)((char*)entries + i * kPlayerSnapshotBytes);
            if (entry->snapshotWord4 == 0 && best > entry->snapshotWord0) {
                result = i;
                best = entry->snapshotWord0;
            }
        }
    }
    return result;
}

// In match mode, compare the first two players' strokes for this hole and retain the saved turn on ties.
int CGolfDoc_ResolveMatchTurnFromHoleStrokes(Object80134B* self) {
    Global80134B* global = g_sharedDoc;
    if (global->state != kMatchRule)
        return 0;
    int result = 0;
    if (global->mode == kMatchGameMode) {
        if (self->entries == 0)
            return 0;
        unsigned char* entry = (unsigned char*)self->entries + self->selected;
        unsigned char current = entry[0x1F0];
        unsigned char value = entry[0x28];
        if (value > current)
            result = 1;
        else if (value == current)
            result = self->equalResult;
    }
    return result;
}

// Write a camera-to-world affine transform from the document position/forward and pitch in radians.
// self is a writable 0x30-byte output, p is a borrowed document; basis vectors
// occupy triples 0..2 and world position triple 3. tan(pitch) adjusts target Y.
void CGolfDoc_BuildCameraMatrix(void* self, void* p) { Vec3_8013 d;  

    WVector a;
    char* base = (char*)p;

    WVector c; a = *(WVector*)(base + kCamera_cameraMatrix_9);
    c = *(WVector*)(base + kCamera_cameraMatrix_9) + *(WVector*)(base + kCamera_cameraMatrix_6);
    c.y += (float)tan(AsCGolfDoc(base)->cameraPitchRadians);  Matrix_BuildCameraTransform(&d, &a, &c);
    Matrix_CopyAffine(self, &d);
}

// Resolve the current turn after both match players hole out, comparing their strokes and retaining ties.
int CGolfDoc_ResolveTurnAfterMatchHoleOut(void* self) {
    Global80134B* g = g_sharedDoc;
    int result = AsCGolfDoc(self)->currentPlayer;
    unsigned char state = AsCSharedDoc(g)->golfRuleType;
    if (state == kMatchRule) {
        Manager8013* manager = (*(Manager8013**)&((CSharedDoc*)g)->golfDoc);
        void* entry = manager->current;
        void* obj;
        if (entry == 0) obj = 0; else obj = entry;
        if (((GolfDocSnapshot8013*)obj)->playerState == 1) {
            void* sub;
            if (entry == 0) sub = 0; else sub = (char*)entry + kPlayerSnapshotBytes;
            if (((GolfDocSnapshot8013*)sub)->playerState == 1) {
                int out = 0;
                if (state == kMatchRule && AsCSharedDoc(g)->gameMode == kMatchGameMode) {
                    void* arr = ((CGolfDoc*)self)->playerSnapshots;
                    if (arr != 0) {
                        unsigned char* e = (unsigned char*)arr + AsCGolfDoc(self)->currentHoleIndex;
                        unsigned char x = e[0x1F0];
                        unsigned char y = e[0x28];
                        if (y > x) out = 1;
                        else if (y == x) out = result;
                    }
                }
                result = out;
            }
        }
    }
    return result;
}

// Update the ball-follow camera using character scale and terrain clearance, optionally publishing its transform.
void CGolfDoc_UpdateCamera(void* self, int update) {
    typedef Vec3f V;
    struct M { char data[0x30]; };
    V b, t, a, scaled, hit;
    M m; M c; M d;
    struct Words { int x, y, z; } vec;
    char* s = (char*)self;
    // Phase 1: choose follow distance/height, rotate a horizontal forward vector
    // by heading radians, and scale distances using character data when available.
    CameraInfo8013* info = (CameraInfo8013*)GolfBall__Fv();
    if (2 == info->type) { *(float*)(s+kCamera_followDistance)=15.0f; *(float*)(s+kCamera_followHeight)=7.0f; }
    else { *(float*)(s+kCamera_followDistance)=20.0f; *(float*)(s+kCamera_followHeight)=5.0f; }
    Matrix_SetAxisRotation(&m, ((CGolfDoc*)s)->headingRadians, 1);
    __as__7WVectorFRC7WVector(&t, (char*)&m+0x18);
    t.y = 0.0f;
    Normalize__7WVectorFv(&t);
    __as__7WVectorFRC7WVector(s+kCamera_ballPosition, GolfBall__Fv());
    Global80134B* g=g_sharedDoc;
    Manager8013* manager=(*(Manager8013**)&((CSharedDoc*)g)->golfDoc);

    void* item=*(void**)((char*)g+(int)(AsCGolfDoc(manager)->currentPlayer)*0x1D4+0x120);
    IFF_STRUCT::sChar* effect = (IFF_STRUCT::sChar*)ItemInventory_FindCharacterData(AsCSharedDoc(g)->itemInventory,item);
    if(effect) { *(float*)(s+kCamera_followDistance)*=effect->scale; *(float*)(s+kCamera_followHeight)*=effect->scale; }
    // Phase 2: build a world-space frame at the ball, then back its position
    // along forward by followDistance. Height is max(ballY+followHeight,
    // sampledGroundY+clearance), preserving the existing comparison/NaN behavior.
    __pl__FRC7WVectorRC7WVector(&a,s+kCamera_ballPosition,&t); __as__7WVectorFRC7WVector(&b,&a); Matrix_BuildCameraTransform(&c,s+kCamera_ballPosition,&b); Matrix_CopyAffine(s+kCamera_cameraMatrix,&c);
    __ml__FfRC7WVector(&scaled,s+kCamera_cameraMatrix_6,*(float*)(s+kCamera_followDistance)); __ami__7WVectorFRC7WVector(s+kCamera_cameraMatrix_9,&scaled);
    float max=*(float*)(s+kCamera_ballPosition_1)+*(float*)(s+kCamera_followHeight);
    int vx=*(int*)(s+kCamera_cameraMatrix_9), vy=*(int*)(s+kCamera_cameraMatrix_10);
    vec.y=vy; vec.x=vx; __typeof__(hit.y) groundHitYRead = hit.y; __typeof__(groundHitYRead) groundHitYCopy = hit.y; vec.z=*(int*)(s+kCamera_cameraMatrix_11);
    ((void (*)(void*,void*,void*,int,int,int))GetGroundPoint__9CQuadTreeF7WVectorbPib)(&hit,CQuadTree_GetInstance(),&vec,0,0,0);
    __typeof__(groundHitYCopy) groundHitY = hit.y;
    float z=((CGolfDoc*)s)->cameraGroundClearance+(groundHitY);
    float savedMax = max;
    max = z < savedMax ? savedMax : z;
    float savedMax2 = max;
    *(float*)(s+kCamera_cameraMatrix_10)=savedMax2;
    // Phase 3: optionally publish pitched orientation/global position; FOV is
    // always set to 0.959931... radians (55 degrees), even when update == 0.
    if(update) { b.y += (float)tan(AsCGolfDoc(s)->cameraPitchRadians); Matrix_BuildCameraTransform(&d,s+kCamera_ballPosition,&b); Matrix_CopyAffine(&s_cameraTransform,&d); __as__7WVectorFRC7WVector(s_cameraTransform.position,s+kCamera_cameraMatrix_9); }
    Camera_SetFov(g_view,0.9599310755729675f);
}

// Restore the first saved player and shared result words into the live round state, retaining exact copy order.
void CGolfDoc_RestoreFirstPlayerSnapshot(void* self) {
    // Phase 1: copy the snapshot at array index zero into live +0xCC state.
    // Ownership does not transfer; the array is still owned by the document.
    struct HolePangBlock { int words[18]; };
    struct SnapshotTailBlock { int words[22]; };
    char* documentBytes = (char*)self;
    char* savedSnapshotBytes = (*(char**)&((CGolfDoc*)documentBytes)->playerSnapshots);
    if (savedSnapshotBytes != 0) {
        AsLivePlayer(documentBytes)->playerId=((GolfDocSnapshot8013*)savedSnapshotBytes)->playerId; AsLivePlayer(documentBytes)->unk_4=((GolfDocSnapshot8013*)savedSnapshotBytes)->unk_4; AsLivePlayer(documentBytes)->playerState=((GolfDocSnapshot8013*)savedSnapshotBytes)->playerState;
        AsLivePlayer(documentBytes)->rankTieBreakOrder=((GolfDocSnapshot8013*)savedSnapshotBytes)->rankTieBreakOrder;
        *(WVector*)AsLivePlayer(documentBytes)->previousBallPosition = (*(WVector*)&((GolfDocSnapshot8013*)savedSnapshotBytes)->previousBallPosition[0]); *(WVector*)AsLivePlayer(documentBytes)->ballPosition = (*(WVector*)&((GolfDocSnapshot8013*)savedSnapshotBytes)->ballPosition[0]);
        CopySnapshotPair(&AsLivePlayer(documentBytes)->holeStrokeCounts[0], &((GolfDocSnapshot8013*)savedSnapshotBytes)->holeStrokeCounts[0]);
        CopySnapshotPair(&AsLivePlayer(documentBytes)->holeStrokeCounts[2], &((GolfDocSnapshot8013*)savedSnapshotBytes)->holeStrokeCounts[2]);
        CopySnapshotPair(&AsLivePlayer(documentBytes)->holeStrokeCounts[4], &((GolfDocSnapshot8013*)savedSnapshotBytes)->holeStrokeCounts[4]);
        CopySnapshotPair(&AsLivePlayer(documentBytes)->holeStrokeCounts[6], &((GolfDocSnapshot8013*)savedSnapshotBytes)->holeStrokeCounts[6]);
        CopySnapshotPair(&AsLivePlayer(documentBytes)->holeStrokeCounts[8], &((GolfDocSnapshot8013*)savedSnapshotBytes)->holeStrokeCounts[8]);
        CopySnapshotPair(&AsLivePlayer(documentBytes)->holeStrokeCounts[10], &((GolfDocSnapshot8013*)savedSnapshotBytes)->holeStrokeCounts[10]);
        CopySnapshotPair(&AsLivePlayer(documentBytes)->holeStrokeCounts[12], &((GolfDocSnapshot8013*)savedSnapshotBytes)->holeStrokeCounts[12]);
        CopySnapshotPair(&AsLivePlayer(documentBytes)->holeStrokeCounts[14], &((GolfDocSnapshot8013*)savedSnapshotBytes)->holeStrokeCounts[14]);
        AsLivePlayer(documentBytes)->holeStrokeCounts[16]=((GolfDocSnapshot8013*)savedSnapshotBytes)->holeStrokeCounts[16]; AsLivePlayer(documentBytes)->holeStrokeCounts[17]=((GolfDocSnapshot8013*)savedSnapshotBytes)->holeStrokeCounts[17];
        CopySnapshotPair(&AsLivePlayer(documentBytes)->holeResultValues[0], &((GolfDocSnapshot8013*)savedSnapshotBytes)->holeResultValues[0]);
        CopySnapshotPair(&AsLivePlayer(documentBytes)->holeResultValues[2], &((GolfDocSnapshot8013*)savedSnapshotBytes)->holeResultValues[2]);
        CopySnapshotPair(&AsLivePlayer(documentBytes)->holeResultValues[4], &((GolfDocSnapshot8013*)savedSnapshotBytes)->holeResultValues[4]);
        CopySnapshotPair(&AsLivePlayer(documentBytes)->holeResultValues[6], &((GolfDocSnapshot8013*)savedSnapshotBytes)->holeResultValues[6]);
        CopySnapshotPair(&AsLivePlayer(documentBytes)->holeResultValues[8], &((GolfDocSnapshot8013*)savedSnapshotBytes)->holeResultValues[8]);
        CopySnapshotPair(&AsLivePlayer(documentBytes)->holeResultValues[10], &((GolfDocSnapshot8013*)savedSnapshotBytes)->holeResultValues[10]);
        CopySnapshotPair(&AsLivePlayer(documentBytes)->holeResultValues[12], &((GolfDocSnapshot8013*)savedSnapshotBytes)->holeResultValues[12]);
        CopySnapshotPair(&AsLivePlayer(documentBytes)->holeResultValues[14], &((GolfDocSnapshot8013*)savedSnapshotBytes)->holeResultValues[14]);
        AsLivePlayer(documentBytes)->holeResultValues[16]=((GolfDocSnapshot8013*)savedSnapshotBytes)->holeResultValues[16]; AsLivePlayer(documentBytes)->holeResultValues[17]=((GolfDocSnapshot8013*)savedSnapshotBytes)->holeResultValues[17];
        *(HolePangBlock*)(&AsLivePlayer(documentBytes)->holePangTotals[0])=(*(HolePangBlock*)&((GolfDocSnapshot8013*)savedSnapshotBytes)->holePangTotals[0]);
        *(int*)(&AsLivePlayer(documentBytes)->unk_94)=((GolfDocSnapshot8013*)savedSnapshotBytes)->unk_94; *(int*)(&AsLivePlayer(documentBytes)->playerSlot)=((GolfDocSnapshot8013*)savedSnapshotBytes)->playerSlot; *(unsigned char*)(&AsLivePlayer(documentBytes)->equippedItemCount)=((GolfDocSnapshot8013*)savedSnapshotBytes)->equippedItemCount;
        *(int*)(&AsLivePlayer(documentBytes)->itemId)=((GolfDocSnapshot8013*)savedSnapshotBytes)->itemId; *(unsigned char*)(&AsLivePlayer(documentBytes)->turnOrder)=((GolfDocSnapshot8013*)savedSnapshotBytes)->turnOrder; *(float*)(&AsLivePlayer(documentBytes)->powerShotMeter)=((GolfDocSnapshot8013*)savedSnapshotBytes)->powerShotMeter;
        *(short*)(&AsLivePlayer(documentBytes)->unk_AC)=((GolfDocSnapshot8013*)savedSnapshotBytes)->unk_AC;
        *(unsigned char*)(&AsLivePlayer(documentBytes)->unk_AE)=((GolfDocSnapshot8013*)savedSnapshotBytes)->unk_AE;
        *(unsigned char*)(&AsLivePlayer(documentBytes)->timeoutsUsed)=((GolfDocSnapshot8013*)savedSnapshotBytes)->timeoutsUsed;
        *(unsigned char*)(&AsLivePlayer(documentBytes)->unk_B0)=((GolfDocSnapshot8013*)savedSnapshotBytes)->unk_B0;
        *(int*)(&AsLivePlayer(documentBytes)->unk_B4)=((GolfDocSnapshot8013*)savedSnapshotBytes)->unk_B4; ((CGolfDoc*)documentBytes)->unk_184=((GolfDocSnapshot8013*)savedSnapshotBytes)->unk_B8;
        *(unsigned char*)(&AsLivePlayer(documentBytes)->unk_BC)=((GolfDocSnapshot8013*)savedSnapshotBytes)->unk_BC; *(unsigned char*)(&AsLivePlayer(documentBytes)->unk_BD)=((GolfDocSnapshot8013*)savedSnapshotBytes)->unk_BD;
        *(int*)(&AsLivePlayer(documentBytes)->unk_C0)=((GolfDocSnapshot8013*)savedSnapshotBytes)->unk_C0; *(unsigned char*)(&AsLivePlayer(documentBytes)->rankFlags)=((GolfDocSnapshot8013*)savedSnapshotBytes)->rankFlags; *(unsigned char*)(&AsLivePlayer(documentBytes)->totalStrokeCount)=((GolfDocSnapshot8013*)savedSnapshotBytes)->totalStrokeCount;
        for(int i=0;i<0x24;i+=4) *(int*)((char*)&AsLivePlayer(documentBytes)->roundResultWords[0]+i)=*(int*)((char*)&((GolfDocSnapshot8013*)savedSnapshotBytes)->roundResultWords[0]+i);
        CopySnapshotWords(&AsLivePlayer(documentBytes)->roundResultWords[9], &((GolfDocSnapshot8013*)savedSnapshotBytes)->roundResultWords[9]);
        CopySnapshotWords(&AsLivePlayer(documentBytes)->roundResultWords[11], &((GolfDocSnapshot8013*)savedSnapshotBytes)->roundResultWords[11]);
        CopySnapshotWords(&AsLivePlayer(documentBytes)->roundResultWords[13], &((GolfDocSnapshot8013*)savedSnapshotBytes)->roundResultWords[13]);
        *(unsigned char*)(&AsLivePlayer(documentBytes)->roundResultWords[15])=(*(unsigned char*)&((GolfDocSnapshot8013*)savedSnapshotBytes)->roundResultWords[15]);
        CopySnapshotWords(&AsLivePlayer(documentBytes)->roundResultWords[16], &((GolfDocSnapshot8013*)savedSnapshotBytes)->roundResultWords[16]);
        CopySnapshotWords(&AsLivePlayer(documentBytes)->roundResultWords[18], &((GolfDocSnapshot8013*)savedSnapshotBytes)->roundResultWords[18]);
        CopySnapshotWords(&AsLivePlayer(documentBytes)->roundResultWords[20], &((GolfDocSnapshot8013*)savedSnapshotBytes)->roundResultWords[20]);
        CopySnapshotWords(&AsLivePlayer(documentBytes)->roundResultWords[22], &((GolfDocSnapshot8013*)savedSnapshotBytes)->roundResultWords[22]);
        CopySnapshotWords(&AsLivePlayer(documentBytes)->roundResultWords[24], &((GolfDocSnapshot8013*)savedSnapshotBytes)->roundResultWords[24]);
        CopySnapshotWords(&AsLivePlayer(documentBytes)->roundResultWords[26], &((GolfDocSnapshot8013*)savedSnapshotBytes)->roundResultWords[26]);
        CopySnapshotWords(&AsLivePlayer(documentBytes)->roundResultWords[28], &((GolfDocSnapshot8013*)savedSnapshotBytes)->roundResultWords[28]);
        CopySnapshotWords(&AsLivePlayer(documentBytes)->roundResultWords[30], &((GolfDocSnapshot8013*)savedSnapshotBytes)->roundResultWords[30]);
        *(unsigned char*)(&AsLivePlayer(documentBytes)->unk_148)=((GolfDocSnapshot8013*)savedSnapshotBytes)->unk_148; *(unsigned char*)(&AsLivePlayer(documentBytes)->unk_149)=((GolfDocSnapshot8013*)savedSnapshotBytes)->unk_149;
        *(unsigned char*)(&AsLivePlayer(documentBytes)->rankTier)=((GolfDocSnapshot8013*)savedSnapshotBytes)->rankTier;
        // One byte per hole; consumers must establish the flag meanings before renaming the shared field.
        CopySnapshotPair(&AsLivePlayer(documentBytes)->unk_14B[0], &((GolfDocSnapshot8013*)savedSnapshotBytes)->unk_14B[0]);
        CopySnapshotPair(&AsLivePlayer(documentBytes)->unk_14B[2], &((GolfDocSnapshot8013*)savedSnapshotBytes)->unk_14B[2]);
        CopySnapshotPair(&AsLivePlayer(documentBytes)->unk_14B[4], &((GolfDocSnapshot8013*)savedSnapshotBytes)->unk_14B[4]);
        CopySnapshotPair(&AsLivePlayer(documentBytes)->unk_14B[6], &((GolfDocSnapshot8013*)savedSnapshotBytes)->unk_14B[6]);
        CopySnapshotPair(&AsLivePlayer(documentBytes)->unk_14B[8], &((GolfDocSnapshot8013*)savedSnapshotBytes)->unk_14B[8]);
        CopySnapshotPair(&AsLivePlayer(documentBytes)->unk_14B[10], &((GolfDocSnapshot8013*)savedSnapshotBytes)->unk_14B[10]);
        CopySnapshotPair(&AsLivePlayer(documentBytes)->unk_14B[12], &((GolfDocSnapshot8013*)savedSnapshotBytes)->unk_14B[12]);
        CopySnapshotPair(&AsLivePlayer(documentBytes)->unk_14B[14], &((GolfDocSnapshot8013*)savedSnapshotBytes)->unk_14B[14]);
        *(unsigned char*)(&AsLivePlayer(documentBytes)->unk_14B[16])=((GolfDocSnapshot8013*)savedSnapshotBytes)->unk_14B[16];
        *(unsigned char*)(&AsLivePlayer(documentBytes)->unk_14B[17])=((GolfDocSnapshot8013*)savedSnapshotBytes)->unk_14B[17];
        for(int i=0;i<3;i++) *(int*)((char*)&AsLivePlayer(documentBytes)->unk_15D[3]+i*4)=*(int*)((char*)&((GolfDocSnapshot8013*)savedSnapshotBytes)->unk_15D[3]+i*4);
        *(short*)(&AsLivePlayer(documentBytes)->unk_16C)=((GolfDocSnapshot8013*)savedSnapshotBytes)->unk_16C; *(unsigned char*)(&AsLivePlayer(documentBytes)->unk_16E)=((GolfDocSnapshot8013*)savedSnapshotBytes)->unk_16E;
        *(SnapshotTailBlock*)(&AsLivePlayer(documentBytes)->unk_170[0])=(*(SnapshotTailBlock*)&((GolfDocSnapshot8013*)savedSnapshotBytes)->unk_170[0]);
        // Phase 2: restore the eight shared-result words from the singleton, not the snapshot.
        *(void**)(&((GolfDocLivePlayerStorage*)documentBytes)->sharedResultWords[0])=(*(void**)&((GolfSharedPlayerRecord*)g_sharedDoc)->sharedSnapshotWords[0]);
        *(void**)(&((GolfDocLivePlayerStorage*)documentBytes)->sharedResultWords[1])=(*(void**)&((GolfSharedPlayerRecord*)g_sharedDoc)->sharedSnapshotWords[1]);
        *(void**)(&((GolfDocLivePlayerStorage*)documentBytes)->sharedResultWords[2])=(*(void**)&((GolfSharedPlayerRecord*)g_sharedDoc)->sharedSnapshotWords[2]);
        *(void**)(&((GolfDocLivePlayerStorage*)documentBytes)->sharedResultWords[3])=(*(void**)&((GolfSharedPlayerRecord*)g_sharedDoc)->sharedSnapshotWords[3]);
        *(void**)(&((GolfDocLivePlayerStorage*)documentBytes)->sharedResultWords[4])=(*(void**)&((GolfSharedPlayerRecord*)g_sharedDoc)->sharedSnapshotWords[4]);
        *(void**)(&((GolfDocLivePlayerStorage*)documentBytes)->sharedResultWords[5])=(*(void**)&((GolfSharedPlayerRecord*)g_sharedDoc)->sharedSnapshotWords[5]);
        *(void**)(&((GolfDocLivePlayerStorage*)documentBytes)->sharedResultWords[6])=(*(void**)&((GolfSharedPlayerRecord*)g_sharedDoc)->sharedSnapshotWords[6]);
        *(void**)(&((GolfDocLivePlayerStorage*)documentBytes)->sharedResultWords[7])=(*(void**)&((GolfSharedPlayerRecord*)g_sharedDoc)->sharedSnapshotWords[7]);
    }
}

#undef COPY_PAIR
#undef COPY_WORDS

// Build the overview/ball camera transform for the current mode, preserving the previous transform when requested.
void CGolfDoc_BuildModeCameraTransform(void* out, void* unused, int update) {
    // out receives a 0x30-byte camera-to-world frame; unused is ignored. All
    // geometry is world space (XZ horizontal). update requests continuity checks,
    // not publication: rejection copies the previous global frame into out.
    struct V { unsigned int words[3]; };
    struct M { V v[4]; };
    struct Bounds { V a; V b; };
#undef Y
#define Y(v) (((float*)&(v))[1])
#undef VECTOR
#define VECTOR(v,x,y,z) do { ((float*)&(v))[0]=(x); Y(v)=(y); ((float*)&(v))[2]=(z); } while(0)
#undef MANAGER
#define MANAGER ((char*)g_sharedDoc->manager)
#undef ITEM
#define ITEM(p) ((char*)AsCGolfDoc(p)->holeRecords + (AsCGolfDoc(p)->currentHoleIndex * 0x20))
#undef COPY_MATRIX
#define COPY_MATRIX(s) do { \
    ((M*)out)->v[0]=(s)->v[0]; ((M*)out)->v[1]=(s)->v[1]; \
    ((M*)out)->v[2]=(s)->v[2]; ((M*)out)->v[3]=(s)->v[3]; \
    ((M*)out)->v[0]=(s)->v[0]; ((M*)out)->v[1]=(s)->v[1]; \
    ((M*)out)->v[2]=(s)->v[2]; ((M*)out)->v[3]=(s)->v[3]; \
    *(M*)out=*(s); \
    ((M*)out)->v[0]=(s)->v[0]; ((M*)out)->v[1]=(s)->v[1]; \
    ((M*)out)->v[2]=(s)->v[2]; ((M*)out)->v[3]=(s)->v[3]; \
    ((M*)out)->v[0]=(s)->v[0]; ((M*)out)->v[1]=(s)->v[1]; \
    ((M*)out)->v[2]=(s)->v[2]; ((M*)out)->v[3]=(s)->v[3]; \
    *(M*)out=*(s); } while(0)
    struct Frame {
        Bounds bounds; Bounds heights;
        V first; V second; V sum; V scaled;
        V entry; V cross; V zero; V pos;
        M m;
    } frame;
#undef m
#define m frame.m
#undef pos
#define pos frame.pos
#undef zero
#define zero frame.zero
#undef cross
#define cross frame.cross
#undef entry
#define entry frame.entry
    // Phase 1: form a downward-looking basis and horizontal ball-to-cup distance.
    VECTOR(zero,kFloat_0_GolfDoc,kFloat_neg1_GolfDoc,kFloat_0_GolfDoc);
    __as__7WVectorFRC7WVector(&m.v[2],&zero);
    __as__7WVectorFRC7WVector(&m.v[1],MANAGER+0x33C);
    Normalize__7WVectorFv(&m.v[1]);
    Vector_CrossProduct(&cross,&m.v[1],&m.v[2]);
    __as__7WVectorFRC7WVector(&m.v[0],&cross);
    char* manager=MANAGER;
    char* ent=(*(char**)&((CGolfDoc*)manager)->playerSnapshots);
    unsigned char index=((CGolfDoc*)manager)->currentPlayer;
    char* selected;
    if(0==ent) selected=0; else selected=ent+index*0x1C8;
    entry=*(V*)(ITEM(manager)+0xC);
    __mi__FRC7WVectorRC7WVector(&pos,&entry,selected+0x1C);
    Y(pos)=kFloat_0_GolfDoc;
    float f=Magnitude__7WVectorCFv(&pos);
    // Phase 2: choose a position. Mode 5 frames tee/cup midpoint with a height
    // derived from their Z span, view aspect, and FOV; remaining branches select
    // terrain-probed or ball-relative offsets by device/type/distance/state.
    if(((CGolfDoc*)MANAGER)->cameraMode==5) {
        frame.first=*(V*)ITEM(MANAGER);
        frame.second=*(V*)(ITEM(MANAGER)+0xC);
        __pl__FRC7WVectorRC7WVector(&frame.sum,&frame.second,&frame.first);
        __ml__FfRC7WVector(&frame.scaled,&frame.sum,kFloat_0_5_GolfDocSnapshot);
        __as__7WVectorFRC7WVector(&m.v[3],&frame.scaled);
        char* display=(char*)g_view;
        float t=(float)tan(kFloat_0_5_GolfDocSnapshot * (((WView*)display)->fieldOfView /
            (kFloat_0_75_GolfDocSnapshot * (((WView*)display)->viewportWidth / ((WView*)display)->viewportHeight))));
        manager=MANAGER;
        float width=((WView*)display)->viewportHeight/ ((WView*)display)->viewportWidth;
        char* item=ITEM(manager);
        display=(char*)g_view;
        frame.bounds.a.words[0]=*(int*)(item);
        frame.bounds.b.words[2]=*(int*)(item+0x14);
        frame.bounds.a.words[2]=*(int*)((8+item));
        frame.bounds.b.words[0]=*(int*)(item+0xC);
        frame.bounds.a.words[1]=*(int*)(((4+item)));
        frame.bounds.b.words[1]=*(int*)(item+0x10);
        float span=((float*)&frame.bounds.b)[2]-((float*)&frame.bounds.a)[2];
        span=span>kFloat_0_GolfDoc ? span : -span;
        item=ITEM(manager);

        frame.heights.a.words[1]=*(int*)((item+4));
        frame.heights.a.words[0]=*(int*)item;
        frame.heights.b.words[1]=*(int*)(item+0x10);
        frame.heights.a.words[2]=*(int*)((((item+8))));
        frame.heights.b.words[0]=*(int*)(item+0xC);
        float low=Y(frame.heights.a);
        frame.heights.b.words[2]=*(int*)(item+0x14);
        low=Y(frame.heights.b)<low ? low : Y(frame.heights.b);
        Y(m.v[3])=low+(kFloat_0_5_GolfDocSnapshot*((kFloat_480_GolfDocSnapshot+span)))/(width*t);
    } else if(Pointer_GetEnabledBindingDevice(g_inputPointer,1)) { V sum; V scaled; V hit; V point; 

        __ml__FfRC7WVector(&scaled,MANAGER+0x33C,
            kFloat_3_2_GolfDocSnapshot * ((float (*)(void*,int))GolfParameter_ResolveFloat)(((void* (*)())InstanceTable_Get)(),0));
        __pl__FRC7WVectorRC7WVector(&sum,GolfBall__Fv(),&scaled);
        __as__7WVectorFRC7WVector(&m.v[3],&sum);
        point=m.v[3];

         int count; ((void (*)(void*,void*,void*,int,int*,int))GetGroundPoint__9CQuadTreeF7WVectorbPib)(&hit,CQuadTree_GetInstance(),&point,1,&count,1);
        float hitY=Y(hit);
        Y(m.v[3])=kFloat_64_GolfCamera+hitY;
        if(count<0) { COPY_MATRIX((M*)&s_cameraTransform); return; }
    } else if(((CameraInfo8013*)GolfBall__Fv())->type==2 &&
        f < kFloat_3_2_GolfDocSnapshot * ((float (*)(void*,int))GolfParameter_ResolveFloat)(((void* (*)())InstanceTable_Get)(),0)) {
        if(f<kFloat_16_GolfDocSnapshot) {
            V sum; V scaled; V added; V dir; V up;
            VECTOR(up,kFloat_0_GolfDoc,kFloat_1_GolfDoc,kFloat_0_GolfDoc);
            __ml__FfRC7WVector(&dir,MANAGER+0x33C,kFloat_0_2_GolfShotDirection);
            __pl__FRC7WVectorRC7WVector(&added,&dir,&up);
            __ml__FfRC7WVector(&scaled,&added,kFloat_32_GolfShotDirection);
            __pl__FRC7WVectorRC7WVector(&sum,GolfBall__Fv(),&scaled);
            __as__7WVectorFRC7WVector(&m.v[3],&sum);
        } else {
            V sum; V scaled; V added; V dir; V up;
            VECTOR(up,kFloat_0_GolfDoc,kFloat_2_GolfDoc,kFloat_0_GolfDoc);
            __ml__FfRC7WVector(&dir,MANAGER+0x33C,kFloat_0_4_GolfDoc);
            __pl__FRC7WVectorRC7WVector(&added,&dir,&up);
            __ml__FfRC7WVector(&scaled,&added,f);
            __pl__FRC7WVectorRC7WVector(&sum,GolfBall__Fv(),&scaled);
            __as__7WVectorFRC7WVector(&m.v[3],&sum);
        }
    } else if(*((unsigned char*)((void* (*)())InstanceTable_Get)()+0x48)<=3) {
        V sum; V scaled; V added; V dir; V up;
        VECTOR(up,kFloat_0_GolfDoc,kFloat_1_4_GolfShotDirection,kFloat_0_GolfDoc);
        manager=MANAGER;
        f=kFloat_32_GolfShotDirection<f ? f : kFloat_32_GolfShotDirection;
        __ml__FfRC7WVector(&dir,manager+0x33C,kFloat_0_5_GolfDocSnapshot);

        __pl__FRC7WVectorRC7WVector(&added,&dir,&up);
        __ml__FfRC7WVector(&scaled,&added,f);
        __pl__FRC7WVectorRC7WVector(&sum,GolfBall__Fv(),&scaled);
        __as__7WVectorFRC7WVector(&m.v[3],&sum);
    } else {
        V sum; V scaled; V added; V dir; V up;
        VECTOR(up,kFloat_0_GolfDoc,kFloat_1_4_GolfShotDirection,kFloat_0_GolfDoc);
        __ml__FfRC7WVector(&dir,MANAGER+0x33C,kFloat_0_5_GolfDocSnapshot);
        __pl__FRC7WVectorRC7WVector(&added,&dir,&up);
        __ml__FfRC7WVector(&scaled,&added,
            kFloat_3_2_GolfDocSnapshot * ((float (*)(void*,int))GolfParameter_ResolveFloat)(((void* (*)())InstanceTable_Get)(),0));
        __pl__FRC7WVectorRC7WVector(&sum,GolfBall__Fv(),&scaled);
        __as__7WVectorFRC7WVector(&m.v[3],&sum);
    }
    // Phase 3: retain the previous frame if the new height falls or displacement
    // exceeds 32 world units. The earlier failed-ground-probe path also retains it.
    if(update) {
        V delta;
        if(Y(m.v[3])<*(float*)(&s_cameraTransform.position[1]) ||
            (__mi__FRC7WVectorRC7WVector(&delta,&m.v[3],s_cameraTransform.position),
             WVectorLen__FRC7WVector(&delta)>kFloat_32_GolfShotDirection)) {
            COPY_MATRIX((M*)&s_cameraTransform); return;
        }
    }
    COPY_MATRIX(&m);
#undef COPY_MATRIX
#undef ITEM
#undef MANAGER
#undef VECTOR
#undef Y
#undef m
#undef pos
#undef zero
#undef cross
#undef entry
}

} // extern "C"

} // namespace ns1

// ---- merged from golf_doc_turns_and_parameter_xml.cpp ----
namespace ns2 {
// Translation unit 0x80135160-0x80135BB4 (11 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" void* __nwa__FUl(unsigned int);
extern "C" void GolfDocSnapshot_Ctor(void*);
struct Item355F4 {
    unsigned char data[0x1C8];
    // Run the original snapshot element hook in this array slot.
    Item355F4() { GolfDocSnapshot_Ctor(this); }
};

extern "C" {

extern void __as__7WVectorFRC7WVector(void*, const void*);
extern void* g_sharedDoc;

struct Blob8 { unsigned int w[2]; };
struct Blob16 { unsigned int w[4]; };
struct Blob24 { unsigned int w[6]; };
struct Blob18 { unsigned char c[18]; };
struct Blob48 { unsigned int w[18]; };
struct Blob88 { unsigned int w[22]; };
struct Tail { short s; unsigned char b; Blob88 arr; };

struct fn_80135160_State {
    unsigned char reserved00[0xC8];
    unsigned char* output;
    unsigned int reservedCC[2];
    unsigned char reservedD4[0x1E0];
};

// Save live player and shared result words into the first snapshot without changing copy order or padding.
void CGolfDoc_SaveFirstPlayerSnapshot(fn_80135160_State* self) {
    // Phase 1: copy live +0xCC state into snapshot zero. An absent array is a no-op.
    unsigned char* savedSnapshotBytes = self->output;
    if (savedSnapshotBytes == 0) return;
    const unsigned char* documentBytes = (const unsigned char*)self;
    (*(unsigned int*)&((GolfDocSnapshot8013*)savedSnapshotBytes)->playerId) = *(const unsigned int*)(&AsLivePlayer(documentBytes)->playerId);
    (*(unsigned int*)&((GolfDocSnapshot8013*)savedSnapshotBytes)->unk_4) = *(const unsigned int*)(&AsLivePlayer(documentBytes)->unk_4);
    (*(unsigned int*)&((GolfDocSnapshot8013*)savedSnapshotBytes)->playerState) = *(const unsigned int*)(&AsLivePlayer(documentBytes)->playerState);
    ((GolfDocSnapshot8013*)savedSnapshotBytes)->rankTieBreakOrder = *(const unsigned char*)(&AsLivePlayer(documentBytes)->rankTieBreakOrder);
    (*(WVector*)&((GolfDocSnapshot8013*)savedSnapshotBytes)->previousBallPosition[0]) = *(WVector*)(&AsLivePlayer(documentBytes)->previousBallPosition[0]);
    (*(WVector*)&((GolfDocSnapshot8013*)savedSnapshotBytes)->ballPosition[0]) = *(WVector*)(&AsLivePlayer(documentBytes)->ballPosition[0]);
    (*(Blob18*)&((GolfDocSnapshot8013*)savedSnapshotBytes)->holeStrokeCounts[0]) = *(const Blob18*)(&AsLivePlayer(documentBytes)->holeStrokeCounts[0]);
    (*(Blob18*)&((GolfDocSnapshot8013*)savedSnapshotBytes)->holeResultValues[0]) = *(const Blob18*)(&AsLivePlayer(documentBytes)->holeResultValues[0]);
    {
        // Per-hole arrays are copied as ABI-sized aggregates; Blob48 is 72 bytes.
    (*(Blob48*)&((GolfDocSnapshot8013*)savedSnapshotBytes)->holePangTotals[0]) = *(const Blob48*)(&AsLivePlayer(documentBytes)->holePangTotals[0]);
    }
    (*(unsigned int*)&((GolfDocSnapshot8013*)savedSnapshotBytes)->unk_94) = *(const unsigned int*)(&AsLivePlayer(documentBytes)->unk_94);
    (*(unsigned int*)&((GolfDocSnapshot8013*)savedSnapshotBytes)->playerSlot) = *(const unsigned int*)(&AsLivePlayer(documentBytes)->playerSlot);
    ((GolfDocSnapshot8013*)savedSnapshotBytes)->equippedItemCount = *(const unsigned char*)(&AsLivePlayer(documentBytes)->equippedItemCount);
    (*(unsigned int*)&((GolfDocSnapshot8013*)savedSnapshotBytes)->itemId) = *(const unsigned int*)(&AsLivePlayer(documentBytes)->itemId);
    ((GolfDocSnapshot8013*)savedSnapshotBytes)->turnOrder = *(const unsigned char*)(&AsLivePlayer(documentBytes)->turnOrder);
    ((GolfDocSnapshot8013*)savedSnapshotBytes)->powerShotMeter = *(const float*)(&AsLivePlayer(documentBytes)->powerShotMeter);
    ((GolfDocSnapshot8013*)savedSnapshotBytes)->unk_AC = *(const short*)(&AsLivePlayer(documentBytes)->unk_AC);
    ((GolfDocSnapshot8013*)savedSnapshotBytes)->unk_AE = *(const unsigned char*)(&AsLivePlayer(documentBytes)->unk_AE); ((GolfDocSnapshot8013*)savedSnapshotBytes)->timeoutsUsed = *(const unsigned char*)(&AsLivePlayer(documentBytes)->timeoutsUsed); ((GolfDocSnapshot8013*)savedSnapshotBytes)->unk_B0 = *(const unsigned char*)(&AsLivePlayer(documentBytes)->unk_B0);
    (*(unsigned int*)&((GolfDocSnapshot8013*)savedSnapshotBytes)->unk_B4) = *(const unsigned int*)(&AsLivePlayer(documentBytes)->unk_B4);
    (*(unsigned int*)&((GolfDocSnapshot8013*)savedSnapshotBytes)->unk_B8) = *(const unsigned int*)(&AsLivePlayer(documentBytes)->unk_B8);
    ((GolfDocSnapshot8013*)savedSnapshotBytes)->unk_BC = *(const unsigned char*)(&AsLivePlayer(documentBytes)->unk_BC); ((GolfDocSnapshot8013*)savedSnapshotBytes)->unk_BD = *(const unsigned char*)(&AsLivePlayer(documentBytes)->unk_BD);
    (*(unsigned int*)&((GolfDocSnapshot8013*)savedSnapshotBytes)->unk_C0) = *(const unsigned int*)(&AsLivePlayer(documentBytes)->unk_C0);
    ((GolfDocSnapshot8013*)savedSnapshotBytes)->rankFlags = *(const unsigned char*)(&AsLivePlayer(documentBytes)->rankFlags); ((GolfDocSnapshot8013*)savedSnapshotBytes)->totalStrokeCount = *(const unsigned char*)(&AsLivePlayer(documentBytes)->totalStrokeCount);
    // Copy result words in their original mixed word/byte/aggregate schedule.
    ((GolfDocSnapshot8013*)savedSnapshotBytes)->roundResultWords[0] = *(const unsigned int*)(&AsLivePlayer(documentBytes)->roundResultWords[0]);
    ((GolfDocSnapshot8013*)savedSnapshotBytes)->roundResultWords[1] = *(const unsigned int*)(&AsLivePlayer(documentBytes)->roundResultWords[1]);
    ((GolfDocSnapshot8013*)savedSnapshotBytes)->roundResultWords[2] = *(const unsigned int*)(&AsLivePlayer(documentBytes)->roundResultWords[2]);
    ((GolfDocSnapshot8013*)savedSnapshotBytes)->roundResultWords[3] = *(const unsigned int*)(&AsLivePlayer(documentBytes)->roundResultWords[3]);
    ((GolfDocSnapshot8013*)savedSnapshotBytes)->roundResultWords[4] = *(const unsigned int*)(&AsLivePlayer(documentBytes)->roundResultWords[4]);
    ((GolfDocSnapshot8013*)savedSnapshotBytes)->roundResultWords[5] = *(const unsigned int*)(&AsLivePlayer(documentBytes)->roundResultWords[5]);
    ((GolfDocSnapshot8013*)savedSnapshotBytes)->roundResultWords[6] = *(const unsigned int*)(&AsLivePlayer(documentBytes)->roundResultWords[6]);
    ((GolfDocSnapshot8013*)savedSnapshotBytes)->roundResultWords[7] = *(const unsigned int*)(&AsLivePlayer(documentBytes)->roundResultWords[7]);
    ((GolfDocSnapshot8013*)savedSnapshotBytes)->roundResultWords[8] = *(const unsigned int*)(&AsLivePlayer(documentBytes)->roundResultWords[8]);
    (*(Blob8*)&((GolfDocSnapshot8013*)savedSnapshotBytes)->roundResultWords[9]) = *(const Blob8*)(&AsLivePlayer(documentBytes)->roundResultWords[9]);
    (*(Blob8*)&((GolfDocSnapshot8013*)savedSnapshotBytes)->roundResultWords[11]) = *(const Blob8*)(&AsLivePlayer(documentBytes)->roundResultWords[11]);
    (*(Blob8*)&((GolfDocSnapshot8013*)savedSnapshotBytes)->roundResultWords[13]) = *(const Blob8*)(&AsLivePlayer(documentBytes)->roundResultWords[13]);
    *(unsigned char*)&((GolfDocSnapshot8013*)savedSnapshotBytes)->roundResultWords[15] = *(const unsigned char*)(&AsLivePlayer(documentBytes)->roundResultWords[15]);
    (*(Blob8*)&((GolfDocSnapshot8013*)savedSnapshotBytes)->roundResultWords[16]) = *(const Blob8*)(&AsLivePlayer(documentBytes)->roundResultWords[16]);
    (*(Blob8*)&((GolfDocSnapshot8013*)savedSnapshotBytes)->roundResultWords[18]) = *(const Blob8*)(&AsLivePlayer(documentBytes)->roundResultWords[18]);
    (*(Blob8*)&((GolfDocSnapshot8013*)savedSnapshotBytes)->roundResultWords[20]) = *(const Blob8*)(&AsLivePlayer(documentBytes)->roundResultWords[20]);
    (*(Blob8*)&((GolfDocSnapshot8013*)savedSnapshotBytes)->roundResultWords[22]) = *(const Blob8*)(&AsLivePlayer(documentBytes)->roundResultWords[22]);
    (*(Blob8*)&((GolfDocSnapshot8013*)savedSnapshotBytes)->roundResultWords[24]) = *(const Blob8*)(&AsLivePlayer(documentBytes)->roundResultWords[24]);
    (*(Blob24*)&((GolfDocSnapshot8013*)savedSnapshotBytes)->roundResultWords[26]) = *(const Blob24*)(&AsLivePlayer(documentBytes)->roundResultWords[26]);
    { unsigned char c14A = *(const unsigned char*)(&AsLivePlayer(documentBytes)->rankTier);
        unsigned char c148 = *(const unsigned char*)(&AsLivePlayer(documentBytes)->unk_148);
        unsigned char c149 = *(const unsigned char*)(&AsLivePlayer(documentBytes)->unk_149);

        ((GolfDocSnapshot8013*)savedSnapshotBytes)->unk_148 = c148;
        ((GolfDocSnapshot8013*)savedSnapshotBytes)->unk_149 = c149;
        ((GolfDocSnapshot8013*)savedSnapshotBytes)->rankTier = c14A;
    }
    (*(Blob18*)&((GolfDocSnapshot8013*)savedSnapshotBytes)->unk_14B[0]) = *(const Blob18*)(&AsLivePlayer(documentBytes)->unk_14B[0]);
    (*(unsigned int*)&((GolfDocSnapshot8013*)savedSnapshotBytes)->unk_15D[3]) = *(const unsigned int*)(&AsLivePlayer(documentBytes)->unk_15D[3]);
    (*(unsigned int*)&((GolfDocSnapshot8013*)savedSnapshotBytes)->unk_15D[7]) = *(const unsigned int*)(&AsLivePlayer(documentBytes)->unk_15D[7]);
    (*(unsigned int*)&((GolfDocSnapshot8013*)savedSnapshotBytes)->unk_15D[11]) = *(const unsigned int*)(&AsLivePlayer(documentBytes)->unk_15D[11]);
    *(Tail*)(savedSnapshotBytes + IFF_OFFSET(GolfDocSnapshot8013, unk_16C)) = *(const Tail*)(documentBytes + IFF_OFFSET(GolfDocLivePlayerStorage, player.unk_16C));
    // Phase 2: publish the eight live shared-result words separately to the singleton.
    ((GolfSharedPlayerRecord*)g_sharedDoc)->sharedSnapshotWords[0] = *(const unsigned int*)(&((GolfDocLivePlayerStorage*)documentBytes)->sharedResultWords[0]);
    ((GolfSharedPlayerRecord*)g_sharedDoc)->sharedSnapshotWords[1] = *(const unsigned int*)(&((GolfDocLivePlayerStorage*)documentBytes)->sharedResultWords[1]);
    ((GolfSharedPlayerRecord*)g_sharedDoc)->sharedSnapshotWords[2] = *(const unsigned int*)(&((GolfDocLivePlayerStorage*)documentBytes)->sharedResultWords[2]);
    ((GolfSharedPlayerRecord*)g_sharedDoc)->sharedSnapshotWords[3] = *(const unsigned int*)(&((GolfDocLivePlayerStorage*)documentBytes)->sharedResultWords[3]);
    ((GolfSharedPlayerRecord*)g_sharedDoc)->sharedSnapshotWords[4] = *(const unsigned int*)(&((GolfDocLivePlayerStorage*)documentBytes)->sharedResultWords[4]);
    ((GolfSharedPlayerRecord*)g_sharedDoc)->sharedSnapshotWords[5] = *(const unsigned int*)(&((GolfDocLivePlayerStorage*)documentBytes)->sharedResultWords[5]);
    ((GolfSharedPlayerRecord*)g_sharedDoc)->sharedSnapshotWords[6] = *(const unsigned int*)(&((GolfDocLivePlayerStorage*)documentBytes)->sharedResultWords[6]);
    ((GolfSharedPlayerRecord*)g_sharedDoc)->sharedSnapshotWords[7] = *(const unsigned int*)(&((GolfDocLivePlayerStorage*)documentBytes)->sharedResultWords[7]);
}

struct ArrayItem;
// The hole-record array element constructor performs no initialization.
void GolfHoleRecord_Ctor(ArrayItem*) {}

struct ArrayItem {
    unsigned char data[0x20];
    // Preserve the original no-op hole-record construction hook.
    ArrayItem() { GolfHoleRecord_Ctor(this); }
};

struct fn_801356B4_State {
    unsigned char reserved00[0x2BC];
    ArrayItem* array;
};

extern void* memset(void*, int, unsigned int);
extern void __dla__FPv(void*);

// Replace the hole-record array with eighteen zeroed records, preserving array-header ownership.
void CGolfDoc_AllocateHoleRecords(fn_801356B4_State* self) {
    if (self->array != 0) {
        if (self->array != 0) {
            __dla__FPv((unsigned char*)self->array - 0x10);
        }
        self->array = 0;
    }
    ArrayItem* array = new ArrayItem[kRoundHoleCount];
    self->array = array;
    memset(array, 0, kRoundHoleCount * kHoleRecordBytes);
}

struct fn_8013574C_State {
    unsigned char reserved00[0x2BC];
    unsigned char* array;
};

// Clear the four reset bytes in each of the eighteen hole records, retaining the unrolled three-record loop.
void CGolfDoc_ClearHoleResetFlags(fn_8013574C_State* self) {
    unsigned int offset = 0;
    for (int i = 0; i < kRoundHoleCount / kUnrolledHolesPerIteration; ++i) {
        self->array[offset + kHoleResetFlagsOffset] = 0;
        self->array[offset + kHoleResetFlagsOffset + 1] = 0;
        self->array[offset + kHoleResetFlagsOffset + 2] = 0;
        self->array[offset + kHoleResetFlagsOffset + 3] = 0;
        offset += kHoleRecordBytes;
        self->array[offset + kHoleResetFlagsOffset] = 0;
        self->array[offset + kHoleResetFlagsOffset + 1] = 0;
        self->array[offset + kHoleResetFlagsOffset + 2] = 0;
        self->array[offset + kHoleResetFlagsOffset + 3] = 0;
        offset += kHoleRecordBytes;
        self->array[offset + kHoleResetFlagsOffset] = 0;
        self->array[offset + kHoleResetFlagsOffset + 1] = 0;
        self->array[offset + kHoleResetFlagsOffset + 2] = 0;
        self->array[offset + kHoleResetFlagsOffset + 3] = 0;
        offset += kHoleRecordBytes;
    }
}

extern void TiXmlDocument_Ctor(void*);
extern void* Resource_OpenResolvedPath(void*, const char*, unsigned int, unsigned int, unsigned int, unsigned int);
extern unsigned int appMallocUni__FUl(unsigned int);
extern void Resource_Release(void*);
extern void* TiXmlDocument_ClearAndParse(void*, const char*);
extern void GolfParameters_LoadTerrainDeclarationsXml(void*, void*);
extern void GolfParameters_LoadEquipmentStringMapXml(void*, void*, void*);
extern unsigned int String_ReplaceRange(void*, unsigned int, unsigned int, const char*, const char*, const unsigned char*);
extern void appFreeUni__FPv(void*);
extern void TiXmlNode_dtor(void*, int);
extern void __dla__FPv(void*);
extern void* g_resourceManager;

struct fn_80135800_State {
    unsigned char reserved00[0xBC];
    unsigned char smallLen;
    unsigned char reservedBD[3];
    unsigned int heapLength;
    char* data;
};
struct File35800 {
    virtual void slot0();
    virtual void slot1();
    virtual void read(char*, unsigned int);
    unsigned int size;
};
struct Flags35800 { CW_BITS2( unsigned int isLong : 1, unsigned int other : 31) };

struct fn_80135800_Buffer {
    unsigned char reserved00[0x30];
    void* array;
    void* arrayEnd;
    // Free XML wrapper array storage, then destroy its document base.
    ~fn_80135800_Buffer() {
        if (array != 0) __dla__FPv(array);
        array = 0;
        arrayEnd = 0;
        TiXmlNode_dtor(this, 0);
    }
};

struct FileGuard35800 {
    void** file;
    // Borrow the local resource-handle variable so explicit release can disarm cleanup.
    FileGuard35800(void** p) : file(p) {}
    // Release an outstanding handle and clear the caller's variable on scope exit.
    ~FileGuard35800() {
        if (*file != 0) {
            if (*file != 0) {
                Resource_Release(*file);
                *file = 0;
            }
        }
    }
};

// Read the cached parameter-path byte length from its inline/heap string tag.
inline unsigned int length35800(fn_80135800_State* s) {
    unsigned int result;
    if ((*(unsigned int*)&s->smallLen >> 31) == 0) result = s->smallLen & 0x7F;
    else result = s->heapLength;
    return result;
}
// Choose the shorter comparison bound without copying either argument.
inline const unsigned int& min35800(const unsigned int& a, const unsigned int& b) {
    return a < b ? a : b;
}
// Lexically compare the cached path and a borrowed C string; expose original length temporaries.
inline int compareText35800(fn_80135800_State* s, const char* text, unsigned int& length, unsigned int& n, unsigned int& size, unsigned int& count) {
    const char* data;
    n = strlen(text);
    unsigned int total = n;
    length = length35800(s);
    unsigned int sz;
    if (!((Flags35800*)&s->smallLen)->isLong) {
        data = (const char*)s + 0xBD;
        sz = s->smallLen & 0x7F;
    } else {
        data = s->data;
        sz = s->heapLength;
    }
    size = sz;
    count = min35800(length, size);
    int c = memcmp(data, text, min35800(n, count));
    if (c == 0) {
        if (count < total) c = -1;
        else if (count == total) c = 0;
        else c = 1;
    }
    return c;
}

#pragma push
#pragma use_lmw_stmw on
// Read and parse a parameter XML resource, refresh declarations when the path changes, and load its indexed mappings.
// text is a borrowed resource path copied into the cache only after a successful
// parse. equipmentIndexArgument is pointer-typed in this reconstruction but is
// forwarded as an ABI word to the equipment loader's unsigned-byte index; it is
// never dereferenced. The local XML document dies before return; loaders must copy data.
void CGolfDoc_LoadParameterXml(fn_80135800_State* self, const char* text, const char* equipmentIndexArgument) {
    fn_80135800_Buffer buffer;
    TiXmlDocument_Ctor(&buffer);
    void* file = Resource_OpenResolvedPath(g_resourceManager, text, kResourceAllGroups, 0, 0, 0);
    FileGuard35800 holder(&file);
    unsigned int length;
    unsigned int n;
    unsigned int size;
    unsigned int count;
    struct Allocator35800 { volatile unsigned char value; unsigned char pad[3]; } allocator;
    unsigned char trailing;
    if (file != 0) {
        // Phase 1: read a null-terminated copy, then drop the resource handle.
        char* copy = (char*)appMallocUni__FUl(*((unsigned int*)file + 1) + 1);
        ((File35800*)file)->read(copy, ((File35800*)file)->size);
        copy[*((unsigned int*)file + 1)] = 0;
        if (file != 0) {
            Resource_Release(file);
            file = 0;
        }
        // Phase 2: only successful parses update declarations/mappings and the cached path.
        if (TiXmlDocument_ClearAndParse(&buffer, copy) != 0) {
        if (length35800(self) == 0 || compareText35800(self, text, length, n, size, count) != 0)
            GolfParameters_LoadTerrainDeclarationsXml(((GolfDocOwnedMembers*)self)->declarations, &buffer);
        GolfParameters_LoadEquipmentStringMapXml(((GolfDocOwnedMembers*)self)->stringParameters, &buffer, (void*)equipmentIndexArgument);
        unsigned int cap2 = length35800(self);
        trailing = allocator.value;
        unsigned int len2 = strlen(text);
        String_ReplaceRange(((GolfDocOwnedMembers*)self)->path, 0, cap2, text, text + len2, &trailing);
        if (copy != 0) appFreeUni__FPv(copy);
        return;
        }
        if (copy != 0) appFreeUni__FPv(copy);
    }
}

#pragma pop
struct fn_801355F4_State {
    unsigned char reserved00[0xC8];
    unsigned char* array;
    unsigned char reservedCC[0x1EC];
    unsigned char countA;
    unsigned char countB;
};

extern void GolfDocSnapshot_Ctor(void*);

// Replace the snapshot array for the requested player count and zero every record.
void CGolfDoc_AllocatePlayerSnapshots(fn_801355F4_State* self, unsigned char count) {
    self->countA = count;
    self->countB = count;
    if (self->array != 0) {
        if (self->array != 0) {
            __dla__FPv((unsigned char*)self->array - 0x10);
        }
        self->array = 0;
    }
    unsigned char* array = (unsigned char*)new Item355F4[self->countB];
    self->array = array;
    memset(array, 0, self->countB * 0x1C8);
}

struct fn_80135B5C_State {
    unsigned char reserved00[0x20];
    unsigned int value;
};

// Return the XML node's next sibling.
unsigned int TiXmlNode_GetNextSibling(fn_80135B5C_State* self) {
    return self->value;
}

struct fn_80135B90_State {
    unsigned char reserved00[0x0C];
    unsigned int value;
};

// Return the XML node's first child.
unsigned int TiXmlNode_GetFirstChild(fn_80135B90_State* self) {
    return self->value;
}

struct fn_80135B64_State {
    unsigned char reserved00[0x08];
    int value;
};

// Return this node only when it is a TinyXML element; otherwise return null.
void* TiXmlNode_AsElement(fn_80135B64_State* self) {
    int valid = 0;
    if (self != 0 && self->value == 1) {
        valid = 1;
    }
    if (valid == 0) return 0;
    return self;
}

struct fn_80135B98_State {
    unsigned char reserved00[0x14];
    unsigned int data;
    unsigned int flag;
};

extern char s_tinyXmlEmptyString[8];

// Return the XML node's text buffer, falling back to the shared empty string.
char* TiXmlNode_GetValueOrEmpty(fn_80135B98_State* self) {
    if (self->flag != 0) {
        return (char*)self->data;
    }
    return s_tinyXmlEmptyString;
}

extern void __dla__FPv(void*);
extern void __dl__FPv(void*);
extern void TiXmlNode_dtor(void*, int);

struct dtor_80135ADC_State {
    unsigned char reserved00[0x30];
    void* array;
    void* arrayEnd;
};

// Release the parsed XML array and its XML document base, then optionally delete the wrapper.
void* TiXmlDocument_dtor(dtor_80135ADC_State* self, int shouldDelete) {
    if (self != 0) {
        if ((unsigned int)self + 0x30 != 0) {
            if (self->array != 0) {
                __dla__FPv(self->array);
            }
            self->array = 0;
            self->arrayEnd = 0;
        }
        TiXmlNode_dtor(self, 0);
        if (shouldDelete > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

} // extern "C"

} // namespace ns2

// ---- merged from golf_doc_turns_and_parameter_xml.cpp ----
namespace ns3 {
// Translation unit 0x80135BB4-0x8013798C (14 functions; boundaries inferred by decomp-toolkit from extab).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

struct U80135BB4 {
    char reserved00[0x25];
    unsigned char firstTextByte;
    char reserved26[0x3F];
    unsigned char secondTextByte;
    char reserved66[0x252];
    unsigned char activePlayerCount;
};

struct U80135BB4Target {
    char reserved00[0xC8];
    void* snapshots;
    char reservedCC[0x1EC];
    unsigned char activePlayerCount;
    unsigned char playerCount;
};

struct U80135BB4Camera {
    char reserved00[0xC0];
    float ballVerticalOffset;
};

struct U80135BB4Globals {
    char reserved0000[0x82A];
    unsigned char reserved082A[0x100];
    char reserved092A[0x9B];
    unsigned char roundHoleCount;
};

extern U80135BB4Globals* g_sharedDoc;
extern float kFloat_0_GolfDoc;
void* memset(void*, int, unsigned int);
void __dl__FPv(void*);
U80135BB4Camera* GolfBall__Fv();
void __pl__FRC7WVectorRC7WVector(float*, void*, float*);
unsigned char PlayerRank_FindTierForValue(void*);
int rand();
unsigned char ItemInventory_SumAttributeContributions(void*, int, void*, void*, void*, void*, void*, unsigned char);
int ItemInventory_IsSlot14ClubSetId(void*);
unsigned char CharacterStats_GetBasePlusBonusByte(int, void*, void*);

void* TiXmlNode_FindChildByValue(void*, const char*);
void* TiXmlNode_FindNextSiblingByValue(void*, const char*);
void* TiXmlElement_GetAttribute(void*, const char*, int*);
void* TiXmlElement_CopyAttributeToEngineString(void*, const char*, const char*);
void VectorStorage_ClearCount(void*);
void StringValueTree_ClearUnrolled(void*);
void* TiXmlNode_AsElement(void*);
void* TiXmlNode_GetFirstChild(void*);
const char* TiXmlNode_GetValueOrEmpty(void*);
void TerrainDeclarationVector_Reserve(void*, int);
int String_EqualsCString(void*, const char*);
int CString_FindSubstringForwarder(void*, const char*);
void TerrainDeclarationVector_Append(void*, void*);
void* NamedObjectValueTree_GetOrInsert(void*, void*);
void* TiXmlNode_GetNextSibling(void*);
void* MWString_ConstructCString(void*, const char*);
const char* MWString_GetCString(void*);

void StringValueTree_DestroySubtree(void*, void*);
void appFreeUni__FPv(void*);
int String_CompareCString(const char*, const char*);
extern GolfParameterXmlStringPool golfParameterXmlStrings;
extern unsigned int s_golfDeclarationFieldDescriptors[];
extern void* g_resourceManager;
extern float kFloat_0_001_GolfDoc;
void* Resource_OpenResolvedPath(void*, const char*, unsigned int, int, int, int);
void Resource_Release(void*);
void PlayerPlainTree_DestroyRootAndChildren(void*, void*);
void* BeachParameterTree_FindInsertionPosition(void*, int*, void**, unsigned char*, unsigned char*);
void* Payload44Tree_InsertAndRebalance(void*, void*, int, int, const void*);
void StringMap_FreeSubtree(void*, void*);
void StringMap_DestroyStringPair(void*, void*);
void String_ReserveCapacity(void*, unsigned int);
void String_ReplaceRange(void*, int, int, const char*, const char*, unsigned char*);
void* CameraStringMap_FindInsertPosition(void*, void*, void*, unsigned char*, unsigned char*);
void* CameraStringMap_InsertNode(void*, void*, unsigned char, unsigned char, void*);
void StringPair800D750C_dtor(void*, int);

struct U80136094String { char reserved00[0xC]; };

struct TreeNode { TreeNode* left; TreeNode* right; TreeNode* parent; int key; };

struct U801370F4Allocator { unsigned char byte; };

struct U801370F4String {
    union {
        struct { unsigned int flags; unsigned int length; void* data; };
        struct { unsigned char tag; char shortData[11]; };
    };
    // Leave storage untouched; this specialization's callers initialize the tag explicitly.
    U801370F4String() {}
    // Copy inline storage or allocate an independent copy of heap-backed text.
    U801370F4String(const volatile U801370F4String& src) {
        unsigned int f = src.flags;
        if ((f >> 31) == 0) {
            unsigned int n = src.length;
            void* p = src.data;
            flags = f; length = n; data = p;
        } else {
            unsigned int n = src.length;
            flags = 0; length = 0; data = 0;
            String_ReserveCapacity(this, n);
            U801370F4Allocator allocator;
            U801370F4Allocator copy = allocator;
            const char* text = (const char*)src.data;
            String_ReplaceRange(this, 0, 0, text, text + src.length, &copy.byte);
        }
    }
    // Release only heap-backed string storage, as selected by tag bit 31.
    ~U801370F4String() { if (flags >> 31) appFreeUni__FPv(data); }
};

struct U801370F4Twin {
    U801370F4String first;
    U801370F4String second;
    // Construct the key/value pair independently so each string owns its copied heap text.
    U801370F4Twin(const U801370F4String& a, const volatile U801370F4String& b) : first(a), second(b) {}
};

extern char s_tinyXmlEmptyString[];

// Choose the shorter string comparison length without creating another temporary.
inline const unsigned int& U801370F4Min(const unsigned int& a, const unsigned int& b) {
    return a < b ? a : b;
}

// Copy inline bytes or duplicate heap text into a fresh, unowned destination string.
inline void U801370F4Copy(U801370F4String* dst, const U801370F4String* src) {
    if ((src->flags >> 31) == 0) {
        dst->flags = src->flags;
        dst->length = src->length;
        dst->data = src->data;
    } else {
        dst->flags = 0; dst->length = 0; dst->data = 0;
        String_ReserveCapacity(dst, src->length);
        U801370F4Allocator allocator;
        U801370F4Allocator copy = allocator;
        String_ReplaceRange(dst, 0, 0, (const char*)src->data, (const char*)src->data + src->length, &copy.byte);
    }
}

struct U801370F4Self {
    void* count;
    void* owner;
    void* unknown;
    void* last;
};

struct U801370F4Owner {
    TreeNode* left;
    TreeNode* right;
    void* parent;
    U801370F4String first;
    U801370F4String second;
};

#pragma use_lmw_stmw on
// Rebuild the string mapping table from the selected project/wep XML entry and its child values.
// xmlDocument is a borrowed parsed TinyXML node, despite the legacy const char*
// ABI type. It is not a filesystem path. Required round/project/wep siblings
// and child text must exist at index; there is no malformed-input validation.
void GolfParameters_LoadEquipmentStringMapXml(void* self, const char* xmlDocument, unsigned char index) {
    TreeNode* child;
    TreeNode* pair;
    TreeNode* root;
    void* node;
    U801370F4Owner* owner;
    node = TiXmlNode_FindChildByValue((void*)xmlDocument, golfParameterXmlStrings.round);
    node = TiXmlNode_FindChildByValue(node, golfParameterXmlStrings.project);
    if (!node)
        return;

    // Phase 1: release all old map nodes and owned key/value buffers.
    owner = *(U801370F4Owner**)((char*)self + 4);
    if (owner) {
        root = owner->left;
        if (root) {
            TreeNode* savedLeft = root->left;
            pair = savedLeft;
            if (pair != 0) {
                child = pair->left;
                if (child) {
                    if (child->left) StringMap_FreeSubtree(self, child->left);
                    if (child->right) StringMap_FreeSubtree(self, child->right);
                    StringMap_DestroyStringPair(self, &((GolfStringTreeNode*)child)->key);
                    appFreeUni__FPv(child);
                }
                child = pair->right;
                if (0 != child) {
                    if (child->left) StringMap_FreeSubtree(self, child->left);
                    if (child->right) StringMap_FreeSubtree(self, child->right);
                    StringMap_DestroyStringPair(self, &((GolfStringTreeNode*)child)->key);
                    appFreeUni__FPv(child);
                }
                StringMap_DestroyStringPair(self, &((GolfStringTreeNode*)pair)->key);
                appFreeUni__FPv(pair);
            }
            pair = root->right;
            if (pair) {
                child = pair->left;
                if (0 != child) {
                    if (child->left) StringMap_FreeSubtree(self, child->left);
                    if (child->right) StringMap_FreeSubtree(self, child->right);
                    StringMap_DestroyStringPair(self, &((GolfStringTreeNode*)child)->key);
                    appFreeUni__FPv(child);
                }
                child = pair->right;
                if (child != 0) {
                    if (child->left) StringMap_FreeSubtree(self, child->left);
                    if (child->right) StringMap_FreeSubtree(self, child->right);
                    StringMap_DestroyStringPair(self, &((GolfStringTreeNode*)child)->key);
                    appFreeUni__FPv(child);
                }
                StringMap_DestroyStringPair(self, &((GolfStringTreeNode*)pair)->key);
                appFreeUni__FPv(pair);
            }
            StringMap_DestroyStringPair(self, &((GolfStringTreeNode*)root)->key);
            appFreeUni__FPv(root);
        }
        root = owner->right;
        if (0 != root) {
            pair = root->left;
            if (pair) {
                child = pair->left;
                if (child) {
                    if (child->left) StringMap_FreeSubtree(self, child->left);
                    if (child->right) StringMap_FreeSubtree(self, child->right);
                    StringMap_DestroyStringPair(self, &((GolfStringTreeNode*)child)->key);
                    appFreeUni__FPv(child);
                }
                child = pair->right;
                if (child) {
                    if (child->left) StringMap_FreeSubtree(self, child->left);
                    if (child->right) StringMap_FreeSubtree(self, child->right);
                    StringMap_DestroyStringPair(self, &((GolfStringTreeNode*)child)->key);
                    appFreeUni__FPv(child);
                }
                StringMap_DestroyStringPair(self, &((GolfStringTreeNode*)pair)->key);
                appFreeUni__FPv(pair);
            }
            pair = root->right;
            if (pair) {
                child = pair->left;
                if (child) {
                    if (child->left) StringMap_FreeSubtree(self, child->left);
                    if (child->right) StringMap_FreeSubtree(self, child->right);
                    StringMap_DestroyStringPair(self, &((GolfStringTreeNode*)child)->key);
                    appFreeUni__FPv(child);
                }
                child = pair->right;
                if (child) {
                    if (child->left) StringMap_FreeSubtree(self, child->left);
                    if (child->right) StringMap_FreeSubtree(self, child->right);
                    StringMap_DestroyStringPair(self, &((GolfStringTreeNode*)child)->key);
                    appFreeUni__FPv(child);
                }
                StringMap_DestroyStringPair(self, &((GolfStringTreeNode*)pair)->key);
                appFreeUni__FPv(pair);
            }
            StringMap_DestroyStringPair(self, &((GolfStringTreeNode*)root)->key);
            appFreeUni__FPv(root);
        }
        if (&owner->first != 0) {
            if (&owner->second != 0 && (owner->second.flags >> 31))
                appFreeUni__FPv(owner->second.data);
            if (&owner->first != 0 && (owner->first.flags >> 31))
                appFreeUni__FPv(owner->first.data);
        }
        appFreeUni__FPv(owner);
        ((U801370F4Self*)self)->count = 0;
        ((U801370F4Self*)self)->last = &((U801370F4Self*)self)->owner;
        ((U801370F4Self*)self)->owner = 0;
    }

    // Phase 2: choose the indexed wep sibling. Its name attribute is read into
    // prefix but is not concatenated into keys or values by the code below.
    node = TiXmlNode_FindChildByValue(node, golfParameterXmlStrings.wep);
    void* current = node;
    for (unsigned int i = 0; i < index; ++i)
        current = ((GolfXmlNode*)current)->nextSibling;
    U801370F4String prefix;
    void* selected = ((0 != current) && ((GolfXmlNode*)current)->type == 1) ? current : 0;
    TiXmlElement_CopyAttributeToEngineString(&prefix, (const char*)selected, golfParameterXmlStrings.name);
    // Phase 3: copy child element names to keys and their first-child text to values.
    // Duplicate keys reuse the existing node and replace its owned string value.
    current = ((GolfXmlNode*)current)->firstChild;
    if (current) {
        U801370F4Allocator sep1; U801370F4Allocator sep2;
        for (; 0 != current;) { U801370F4String key;  
            unsigned char out0; unsigned char out1;
            void* value = (((0 != current)) && ((GolfXmlNode*)current)->type == 1) ? current : 0;
            const char* text1 = ((GolfXmlNode*)value)->textLength ? ((GolfXmlNode*)value)->text : "";

             key.flags = 0; key.length = 0; key.data = 0;
            unsigned int keyLen = strlen(text1);

            String_ReserveCapacity(&key, keyLen);
            U801370F4Allocator keyChar = sep1;
            String_ReplaceRange(&key, 0, 0, text1, (text1 + keyLen), &keyChar.byte);

            void* other = ((GolfXmlNode*)current)->firstChild;
            const char* text2 = ((GolfXmlNode*)other)->textLength ? ((GolfXmlNode*)other)->text : "";
            U801370F4String name;
            name.flags = 0; name.length = 0; name.data = 0;
            unsigned int nameLen = strlen(text2);
            String_ReserveCapacity(&name, nameLen);
            U801370F4Allocator nameChar = sep2;
            void* entry;

            String_ReplaceRange(&name, 0, 0, text2, text2 + nameLen, &nameChar.byte);
            void* ret = CameraStringMap_FindInsertPosition(self, &key, &entry, &out0, &out1);
            void* target;
            // Original 0x801370F4: join lookup and insertion before assignment.
            do {
                if (entry) { unsigned int entrySize; 
                    target = &((GolfStringTreeNode*)entry)->key;
                    U801370F4String* entryStr = (U801370F4String*)target;
                    unsigned int keySize;
                    const char* keyText;
                    if ((key.flags >> 31) == 0) {
                        keyText = key.shortData;
                        keySize = *(unsigned char*)&key & 0x7F;
                    } else {
                        keyText = (const char*)key.data;
                        keySize = key.length;
                    }
                    unsigned int savedKeySize = keySize;
                
                    if ((entryStr->flags >> 31) == 0) entrySize = *(unsigned char*)entryStr & 0x7F;
                    else entrySize = entryStr->length;
                    int entryCount;
                    const char* entryText;
                    if ((entryStr->flags >> 31) == 0) {
                        entryText = entryStr->shortData;
                        entryCount = *(unsigned char*)entryStr & 0x7F;
                    } else {
                        entryText = (const char*)entryStr->data;
                        entryCount = entryStr->length;
                    }
                    int savedEntryCount = entryCount;
                    unsigned int count = U801370F4Min(entrySize, savedEntryCount);
                    int cmp = memcmp(entryText, keyText, U801370F4Min((savedKeySize), count));
                    if (!cmp) {
                        if (count < keySize) cmp = -1;
                        else if (count == keySize) cmp = 0;
                        else cmp = 1;
                    }
                    if (0 <= cmp) break;
                }
                {
                    volatile U801370F4String empty;
                    empty.flags = 0; empty.length = 0; empty.data = 0;
                    U801370F4Twin twin(key, empty);
                    unsigned char savedOut = out1;
                    target = (char*)CameraStringMap_InsertNode(self, ret, out0, savedOut, &twin) + 0xC;
                }
            } while (0);
            {
                U801370F4String* dst = (U801370F4String*)(&((GolfStringPairStorage*)target)->value);
                if ((dst->flags >> 31) == 0 && (name.flags >> 31) == 0) {
                    unsigned int sourceFlags = name.flags;
                    unsigned int sourceLength = name.length;
                    dst->flags = sourceFlags;
                    dst->length = sourceLength;
                    dst->data = name.data;
                } else {
                    // Preserve allocator-bound flag snapshots; shortening this chain swaps registers.
                    unsigned int flagsBeforeAllocator = dst->flags;
                    U801370F4Allocator allocator;
                    unsigned int flagsAtAllocator = dst->flags;
                    unsigned int flagsAfterAllocator = dst->flags;
                    unsigned int flagsAfterAllocatorCopy = flagsAfterAllocator;
                    unsigned int srcLen;
                    unsigned int flagsForLength = flagsAtAllocator;
                    int dstLen = (flagsForLength >> 31) ? dst->length : (*(unsigned char*)dst & 0x7F);

                    const char* srcText;
                    if ((name.flags >> 31) == 0) {
                        srcText = name.shortData;
                        srcLen = *(unsigned char*)&name & 0x7F;
                    } else {
                        unsigned int savedLength = name.length;
                        srcText = (const char*)name.data;
                        srcLen = savedLength;
                    }

                    const char* savedSrcLen = srcText + srcLen;
                    U801370F4Allocator assignChar = allocator;
                    int savedDstLen = dstLen;
                    String_ReplaceRange(dst, 0, savedDstLen, srcText, savedSrcLen, &assignChar.byte);
                }
            }
            current = ((GolfXmlNode*)current)->nextSibling;
        }
    }
}

// Load keyed material records from text lines, skipping comments and keeping the first record for each key.
// path is a borrowed filename. File content must fit buffer[0x400] INCLUDING
// its added terminator (size <= 0x3FF); sscanf's text destination is 0x20 bytes.
// Existing code relies on valid resource sizes and formats rather than checking.
void GolfParameters_LoadKeyedTextRecords(void* self, const char* path) {
    struct Guard { void* p; };
    Guard guardObj;
    Guard* volatile guardRef;
    guardObj.p = Resource_OpenResolvedPath(g_resourceManager, path, kResourceAllGroups, 0, 0, 0);
    guardRef = &guardObj;
    if (guardObj.p == 0) {
        if (guardObj.p != 0) {
            Resource_Release(guardObj.p);
            guardRef->p = 0;
        }
        return;
    }

    // Phase 1: after a successful open, postorder-free old nodes and reset the header.
    TreeNode* root = *(TreeNode**)((char*)self + 4);
    if (root != 0) {
        TreeNode* pair = root->left;
        if (pair != 0) {
            TreeNode* node = pair->left;
            if (node != 0) {
                TreeNode* item = node->left;
                if (item != 0) {
                    if (item->left) PlayerPlainTree_DestroyRootAndChildren(self, item->left);
                    if (item->right) PlayerPlainTree_DestroyRootAndChildren(self, item->right);
                    appFreeUni__FPv(item);
                }
                item = node->right;
                if (item != 0) {
                    if (item->left) PlayerPlainTree_DestroyRootAndChildren(self, item->left);
                    if (item->right) PlayerPlainTree_DestroyRootAndChildren(self, item->right);
                    appFreeUni__FPv(item);
                }
                appFreeUni__FPv(node);
            }
            node = pair->right;
            if (node != 0) {
                TreeNode* item = node->left;
                if (item != 0) {
                    if (item->left) PlayerPlainTree_DestroyRootAndChildren(self, item->left);
                    if (item->right) PlayerPlainTree_DestroyRootAndChildren(self, item->right);
                    appFreeUni__FPv(item);
                }
                item = node->right;
                if (item != 0) {
                    if (item->left) PlayerPlainTree_DestroyRootAndChildren(self, item->left);
                    if (item->right) PlayerPlainTree_DestroyRootAndChildren(self, item->right);
                    appFreeUni__FPv(item);
                }
                appFreeUni__FPv(node);
            }
            appFreeUni__FPv(pair);
        }
        pair = root->right;
        if (pair != 0) {
            TreeNode* node = pair->left;
            if (node != 0) {
                TreeNode* item = node->left;
                if (item != 0) {
                    if (item->left) PlayerPlainTree_DestroyRootAndChildren(self, item->left);
                    if (item->right) PlayerPlainTree_DestroyRootAndChildren(self, item->right);
                    appFreeUni__FPv(item);
                }
                item = node->right;
                if (item != 0) {
                    if (item->left) PlayerPlainTree_DestroyRootAndChildren(self, item->left);
                    if (item->right) PlayerPlainTree_DestroyRootAndChildren(self, item->right);
                    appFreeUni__FPv(item);
                }
                appFreeUni__FPv(node);
            }
            node = pair->right;
            if (node != 0) {
                TreeNode* item = node->left;
                if (item != 0) {
                    if (item->left) PlayerPlainTree_DestroyRootAndChildren(self, item->left);
                    if (item->right) PlayerPlainTree_DestroyRootAndChildren(self, item->right);
                    appFreeUni__FPv(item);
                }
                item = node->right;
                if (item != 0) {
                    if (item->left) PlayerPlainTree_DestroyRootAndChildren(self, item->left);
                    if (item->right) PlayerPlainTree_DestroyRootAndChildren(self, item->right);
                    appFreeUni__FPv(item);
                }
                appFreeUni__FPv(node);
            }
            appFreeUni__FPv(pair);
        }
        appFreeUni__FPv(root);
        *(void**)self = 0;
        *(void**)((char*)self + 4) = 0;
        ((GolfTreeHeader*)self)->sentinel = (char*)self + 4;
    }

    // Phase 2: read the entire file after clearing the old tree; failed open
    // above preserves the old tree. Parse records into owned, copied payloads.
    char buffer[0x400];
    int length = 0;
    void** vtable = *(void***)guardObj.p;
    ((void (*)(void*, char*, int))vtable[4])(guardObj.p, buffer, *(int*)((char*)guardObj.p + 4)); // read the whole file: size is the value at +4
    length = *(int*)((char*)guardObj.p + 4);
    buffer[length] = 0;
    float scale = kFloat_0_001_GolfDoc;
    char* line = buffer;
    while (*line != 0) {
        char* lineStart = line;
        while (*line != 0 && *line != '\n') ++line;
        if (*line != 0) ++line;
        if (*lineStart == '/' && lineStart[1] == '/')
            continue;
        int key;
        void* node = 0;
        unsigned char a, b;
        unsigned char record[0x20];
        volatile float value0, value1;
        if (((int (*)(const char*, const char*, ...))sscanf)(lineStart, golfParameterXmlStrings.materialLineFormat, &key, record, &value0, &value1) == 4) {
            TreeNode* header = (TreeNode*)((char*)self + 4);
            TreeNode* current = *(TreeNode**)((char*)self + 4);
            TreeNode* parent = header;
            while (current != 0) {
                if (current->key >= key) { parent = current; current = current->left; }
                else current = current->right;
            }
            if (parent != header && key < parent->key)
                parent = header;
            if (parent == header) {
                // The second scalar is scaled by 0.001 before storage; its later
                // consumer treats it as a timing multiplier, not a spatial radius.
                volatile float scaledTimingValue = value1 * scale;
                void* where = BeachParameterTree_FindInsertionPosition(self, &key, &node, &a, &b);
                TreeNode* entry;
                if (node == 0 || ((TreeNode*)node)->key < key) {
                    struct Init { int key; unsigned int words[20]; } init;
                    init.key = key;
                    for (int i = 0; i < 20; ++i) init.words[i] = 0;
                    entry = (TreeNode*)((char*)Payload44Tree_InsertAndRebalance(self, where, a, b, &init) + 0xC);
                } else entry = (TreeNode*)(&((TreeNode*)node)->key);
                unsigned char* dst = (unsigned char*)entry;
                dst[4] = record[0]; dst[5] = record[1];
                dst[6] = record[2]; dst[7] = record[3];
                dst[8] = record[4]; dst[9] = record[5];
                dst[10] = record[6]; dst[11] = record[7];
                dst[12] = record[8]; dst[13] = record[9];
                dst[14] = record[10]; dst[15] = record[11];
                dst[16] = record[12]; dst[17] = record[13];
                dst[18] = record[14]; dst[19] = record[15];
                dst[20] = record[16]; dst[21] = record[17];
                dst[22] = record[18]; dst[23] = record[19];
                dst[24] = record[20]; dst[25] = record[21];
                dst[26] = record[22]; dst[27] = record[23];
                dst[28] = record[24]; dst[29] = record[25];
                dst[30] = record[26]; dst[31] = record[27];
                dst[32] = record[28]; dst[33] = record[29];
                dst[34] = record[30]; dst[35] = record[31];
                *(float*)(dst + 0x24) = value0;
                *(float*)(dst + 0x28) = scaledTimingValue;
            }
        }
    }
    if (guardObj.p != 0) {
        Resource_Release(guardObj.p);
        guardObj.p = 0;
    }
    if (guardRef->p != 0) {
        Resource_Release(guardRef->p);
        guardRef->p = 0;
    }
}

#pragma dont_inline on
// Optionally free the parameter-record wrapper; its embedded fields need no cleanup here.
void* dtor_80136574(void* self, int freeMemory) {
    if (self != 0 && freeMemory > 0)
        __dl__FPv(self);
    return self;
}

// Return whether a short or heap-backed Metrowerks string has zero length.
unsigned int String_IsEmpty(const void* self) {
    const unsigned int* words = (const unsigned int*)self;
    unsigned int value;
    if ((words[0] >> 31) == 0)
        value = *(const unsigned char*)self & 0x7F;
    else
        value = words[1];
    return value == 0;
}

// Clear the first byte of each declaration record's two text buffers.
void GolfDeclaration_ClearTextBuffers(U80135BB4* self) {
    self->firstTextByte = 0;
    self->secondTextByte = 0;
}
#pragma dont_inline reset

struct U80136094Str {
    char reserved00[0xC];
    // Destroy the engine string returned by the XML attribute helper.
    ~U80136094Str() { ((NString*)(this))->~NString(); }
};

struct U80136094Entry { const char* name; const char* fmt; void* dest; };
struct U80136094Labels { U80136094Entry e[12]; };
struct U80136094Owner { char reserved00[0xC]; int map; };

struct U80136094Data {
    int w[9];
    unsigned char type;
    char s1[0x40];
    char s2[0x40];
    // Initialize only the two text terminators; numeric fields are parsed later.
    U80136094Data() { GolfDeclaration_ClearTextBuffers((U80135BB4*)this); }
    // Preserve the nondeleting trivial record cleanup hook.
    ~U80136094Data() { dtor_80136574(this, -1); }
};

#pragma use_lmw_stmw on
#pragma opt_loop_invariants off
// Load terrain/material declarations from XML, inherit missing default fields, and index texture aliases.
// xmlDocument is a borrowed parsed document with a legacy pointer type, not a path.
// The 12 field descriptors supply names, scanf formats and destinations. Missing
// fields fall back to the named default class. Resource XML must contain usable
// class/default nodes and values; this routine does not add validation.
void GolfParameters_LoadTerrainDeclarationsXml(void* self, const char* xmlDocument) {
    void* node;
    const GolfParameterXmlStringPool* headerBase = &golfParameterXmlStrings;
    node = TiXmlNode_FindChildByValue((void*)xmlDocument, headerBase->round);
    node = TiXmlNode_FindChildByValue(node, headerBase->declare);
    if (node != 0) {
        // Phase 1: clear old records/aliases and reserve the declared record count.
        VectorStorage_ClearCount(self);
        StringValueTree_ClearUnrolled(&((U80136094Owner*)self)->map);
        int value = 0;
        TiXmlElement_GetAttribute(TiXmlNode_AsElement(node), headerBase->count, &value);
        if (value != 0)
            TerrainDeclarationVector_Reserve(self, value);
        *(int*)((char*)self + kDeclarationDefaultIndexOffset) = -1;
        int outerIndex = 0;

        U80136094Data data;
        U80136094Labels labels;
        char buffer[0x80];
        labels =*(U80136094Labels*)s_golfDeclarationFieldDescriptors;
        labels.e[0].dest = &data.w[0];
        labels.e[1].dest = &data.w[1];
        labels.e[2].dest = &data.w[2];
        labels.e[3].dest = &data.w[5];
        labels.e[4].dest = &data.w[3];
        labels.e[5].dest = &data.w[6];
        labels.e[6].dest = &data.w[4];
        labels.e[7].dest = &data.w[7];
        labels.e[8].dest = &data.w[8];
        labels.e[9].dest = buffer;
        labels.e[10].dest = data.s1;
        labels.e[11].dest = data.s2;

        const GolfParameterXmlStringPool* base = &golfParameterXmlStrings;
        // Phase 2: locate the class named default and retain its index for fallback lookup.
        void* cursor = TiXmlNode_FindChildByValue(node, base->classTag);
        while (cursor != 0) {
            ++*(int*)((char*)self + kDeclarationDefaultIndexOffset);
            U80136094Str text;
            TiXmlElement_CopyAttributeToEngineString(&text, (const char*)TiXmlNode_AsElement(cursor), base->name);
            if (String_EqualsCString(&text, base->defaultName))
                break;
            cursor = TiXmlNode_FindNextSiblingByValue(cursor, base->classTag);
        }

        {
        const GolfParameterXmlStringPool* base = &golfParameterXmlStrings;
        // Phase 3: parse each class with per-field fallback, classify terrain,
        // strip .wav suffixes and copy the record plus texture-alias indices.
        void* item = TiXmlNode_FindChildByValue(node, base->classTag);
        do {
            void* textNode = TiXmlNode_AsElement(item);
            if (String_CompareCString(TiXmlNode_GetValueOrEmpty(textNode), base->classTag) != 0)
                break;
            U80136094Str name;
            TiXmlElement_CopyAttributeToEngineString(&name, (const char*)textNode, base->name);
            if (String_IsEmpty(&name))
                break;
            Memory_Clear(buffer, sizeof(buffer));
            for (unsigned int i = 0; i < 12; ++i) {
                void* field = TiXmlNode_FindChildByValue(item, labels.e[i].name);
                if (field == 0)
                    field = TiXmlNode_FindChildByValue(cursor, labels.e[i].name);
                U80136094Str fieldValue;
                MWString_ConstructCString(&fieldValue, TiXmlNode_GetValueOrEmpty(TiXmlNode_GetFirstChild(TiXmlNode_AsElement(field))));
                ((int (*)(const char*, const char*, ...))sscanf)(MWString_GetCString(&fieldValue), labels.e[i].fmt, labels.e[i].dest);
            }
            // Independent tests are ordered: when multiple names occur, the last
            // matching terrain below wins. Unmatched data.type is not reset here.
            if (CString_FindSubstringForwarder(buffer, base->green)) data.type = 2;
            if (CString_FindSubstringForwarder(buffer, base->fairway)) data.type = 1;
            if (CString_FindSubstringForwarder(buffer, base->bunker)) data.type = 3;
            if (CString_FindSubstringForwarder(buffer, base->rough)) data.type = 4;
            if (CString_FindSubstringForwarder(buffer, base->snow)) data.type = 5;
            if (CString_FindSubstringForwarder(buffer, base->road)) data.type = 6;
            if (CString_FindSubstringForwarder(buffer, base->ice)) data.type = 7;
            if (CString_FindSubstringForwarder(buffer, base->water)) data.type = 8;
            if (CString_FindSubstringForwarder(buffer, base->startPoint)) data.type = 0;
            if (CString_FindSubstringForwarder(buffer, base->sand)) data.type = 10;
            if (CString_FindSubstringForwarder(buffer, base->special)) data.type = 11;
            if (CString_FindSubstringForwarder(buffer, base->wb)) data.type = 12;
            {
                char* p = (char*)CString_FindSubstringForwarder(data.s1, base->waveExtension);
                if (p != 0) *p = 0;
            }
            {
                char* p = (char*)CString_FindSubstringForwarder(data.s2, base->waveExtension);
                if (p != 0) *p = 0;
            }
            TerrainDeclarationVector_Append(self, &data);
            void* extra = TiXmlNode_FindChildByValue(item, base->textures);
            if (extra != 0) {
                void* sub = TiXmlNode_FindChildByValue(extra, base->item);
                while (sub != 0) {
                    U80136094Str key;
                    MWString_ConstructCString(&key, TiXmlNode_GetValueOrEmpty(TiXmlNode_GetFirstChild(TiXmlNode_AsElement(sub))));
                    sub = TiXmlNode_GetNextSibling(sub);
                    *(int*)NamedObjectValueTree_GetOrInsert((char*)self + kDeclarationNameIndexOffset, &key) = outerIndex;
                }
            }
            ++outerIndex;
            item = TiXmlNode_FindNextSiblingByValue(item, base->classTag);
        } while (item != 0);
        }
    }
}

#pragma opt_loop_invariants reset

// Return the document's active-player count.
unsigned char CGolfDoc_GetActivePlayerCount(U80135BB4* self) {
    return self->activePlayerCount;
}

#pragma use_lmw_stmw on
// Build snapshot stats from fixed, randomized or equipped-item values, then clamp and bias the power distance.
void CGolfDoc_RefreshPlayerSnapshotStats(void* self, int index) {
    // Requires allocated snapshots and a valid index in both document/shared
    // player arrays. Preset stats return without clamping; the other paths clamp
    // power to 0..100 then subtract 15, control to 0..30, spin to 1..30.
    char* base = (*(char**)&((CGolfDoc*)self)->playerSnapshots);
    char* entry;
    if (base == 0)
        entry = 0;
    else
        entry = base + index * kPlayerSnapshotBytes;

    char* record = (char*)g_sharedDoc + index * kSharedPlayerStride;
    void* characterId = ((GolfSharedPlayerRecord*)record)->characterId;
    void* outfitPartIds = ((GolfSharedPlayerRecord*)record)->partIds;
    void* auxPartIds = ((GolfSharedPlayerRecord*)record)->auxPartIds;
    void* clubSetId = ((GolfSharedPlayerRecord*)record)->clubSetId;
    void* trainingBonuses = ((GolfSharedPlayerRecord*)record)->trainingBonuses;
    void* caddieOrItemId = ((GolfSharedPlayerRecord*)record)->caddieOrItemId;
    unsigned int playerRank = (unsigned char)PlayerRank_FindTierForValue(((GolfSharedPlayerRecord*)record)->rankValue);
    char* entries = (*(char**)&AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->playerSnapshots);
    char* current;
    if (entries == 0)
        current = 0;
    else
        current = entries + index * kPlayerSnapshotBytes;

    if (((GolfDocSnapshot8013*)current)->unk_4 == kUsePresetStats) {
        *(int*)(entry + IFF_OFFSET(GolfSnapshotStats, powerDistanceOffset)) = ((GolfSharedPlayerRecord*)record)->basePower - kPowerDistanceBias;
        *(int*)(entry + IFF_OFFSET(GolfSnapshotStats, controlAtE0)) = ((GolfSharedPlayerRecord*)record)->baseControl;
        *(int*)(entry + IFF_OFFSET(GolfSnapshotStats, control)) = ((GolfSharedPlayerRecord*)record)->baseControl;
        *(int*)(entry + IFF_OFFSET(GolfSnapshotStats, spin)) = ((GolfSharedPlayerRecord*)record)->baseSpin;
        *(int*)(entry + IFF_OFFSET(GolfSnapshotStats, spinAtE8)) = ((GolfSharedPlayerRecord*)record)->baseSpin;
        return;
    }

    if ((*(int*)&((CSharedDoc*)g_sharedDoc)->randomizePlayerStats) != 0) {
        int value = rand();
        *(int*)(entry + IFF_OFFSET(GolfSnapshotStats, powerDistanceOffset)) = value % 20 + 10;
        value = rand();
        *(int*)(entry + IFF_OFFSET(GolfSnapshotStats, controlAtE0)) = value % 10 + 15;
        value = rand();
        *(int*)(entry + IFF_OFFSET(GolfSnapshotStats, control)) = value % 5 + 5;
        value = rand();
        *(int*)(entry + IFF_OFFSET(GolfSnapshotStats, spin)) = value % 5 + 5;
        value = rand();
        *(int*)(entry + IFF_OFFSET(GolfSnapshotStats, spinAtE8)) = value % 5 + 5;
    } else {
        int a = ItemInventory_SumAttributeContributions(AsCSharedDoc(g_sharedDoc)->itemInventory, kStatPower, characterId, outfitPartIds, auxPartIds, trainingBonuses, caddieOrItemId, playerRank);
        *(int*)(entry + IFF_OFFSET(GolfSnapshotStats, powerDistanceOffset)) = (unsigned char)a;
        a = ItemInventory_SumAttributeContributions(AsCSharedDoc(g_sharedDoc)->itemInventory, kStatControl, characterId, outfitPartIds, auxPartIds, trainingBonuses, caddieOrItemId, playerRank);
        *(int*)(entry + IFF_OFFSET(GolfSnapshotStats, controlAtE0)) = (unsigned char)a;
        a = ItemInventory_SumAttributeContributions(AsCSharedDoc(g_sharedDoc)->itemInventory, kStatControl, characterId, outfitPartIds, auxPartIds, trainingBonuses, caddieOrItemId, playerRank);
        *(int*)(entry + IFF_OFFSET(GolfSnapshotStats, control)) = (unsigned char)a;
        a = ItemInventory_SumAttributeContributions(AsCSharedDoc(g_sharedDoc)->itemInventory, kStatSpin, characterId, outfitPartIds, auxPartIds, trainingBonuses, caddieOrItemId, playerRank);
        *(int*)(entry + IFF_OFFSET(GolfSnapshotStats, spin)) = (unsigned char)a;
        a = ItemInventory_SumAttributeContributions(AsCSharedDoc(g_sharedDoc)->itemInventory, kStatSpin, characterId, outfitPartIds, auxPartIds, trainingBonuses, caddieOrItemId, playerRank);
        *(int*)(entry + IFF_OFFSET(GolfSnapshotStats, spinAtE8)) = (unsigned char)a;
        if (ItemInventory_IsSlot14ClubSetId(clubSetId))
            *(int*)(entry + IFF_OFFSET(GolfSnapshotStats, powerDistanceOffset)) = (unsigned char)CharacterStats_GetBasePlusBonusByte(kStatPower, characterId, trainingBonuses);
    }

    // Clamp only the computed/randomized path. The two control/spin slots stay
    // distinct because their random samples and inventory calls may differ.
    int value0 = *(int*)(entry + IFF_OFFSET(GolfSnapshotStats, powerDistanceOffset));
    int clamped0 = (value0 < 0) ? 0 : (value0 > kMaxPowerStat ? kMaxPowerStat : value0);
    *(int*)(entry + IFF_OFFSET(GolfSnapshotStats, powerDistanceOffset)) = clamped0 - kPowerDistanceBias;

    int value1 = *(int*)(entry + IFF_OFFSET(GolfSnapshotStats, control));
    int clamped1 = (value1 < 0) ? 0 : (value1 > kMaxOtherStat ? kMaxOtherStat : value1);
    *(int*)(entry + IFF_OFFSET(GolfSnapshotStats, control)) = clamped1;

    int value2 = *(int*)(entry + IFF_OFFSET(GolfSnapshotStats, controlAtE0));
    int clamped2 = (value2 < 0) ? 0 : (value2 > kMaxOtherStat ? kMaxOtherStat : value2);
    *(int*)(entry + IFF_OFFSET(GolfSnapshotStats, controlAtE0)) = clamped2;

    int value3 = *(int*)(entry + IFF_OFFSET(GolfSnapshotStats, spin));
    int clamped3 = (value3 < kMinimumSpinStat) ? kMinimumSpinStat : (value3 > kMaxOtherStat ? kMaxOtherStat : value3);
    *(int*)(entry + IFF_OFFSET(GolfSnapshotStats, spin)) = clamped3;

    int value4 = *(int*)(entry + IFF_OFFSET(GolfSnapshotStats, spinAtE8));
    int clamped4 = (value4 < kMinimumSpinStat) ? kMinimumSpinStat : (value4 > kMaxOtherStat ? kMaxOtherStat : value4);
    *(int*)(entry + IFF_OFFSET(GolfSnapshotStats, spinAtE8)) = clamped4;
}

// Recount allocated player snapshots whose state is not withdrawn.
void CGolfDoc_RecountActivePlayers(U80135BB4Target* self) {
    self->activePlayerCount = 0;
    for (int i = 0; (unsigned char)i < self->playerCount; ++i) {
        void* entry = (char*)self->snapshots + (unsigned char)i * kPlayerSnapshotBytes;
        if (((GolfDocSnapshot8013*)entry)->playerState != kPlayerWithdrawnState)
            ++self->activePlayerCount;
    }
}

// Return the indexed 0x68-byte item-selection subobject; the caller validates the player index.
void* CGolfDoc_GetItemSelection(void* self, int index) {
    return (char*)self + index * 0x68 + 0x53C;
}

// Clear the four per-player ball-reset records.
void CGolfDoc_ClearPlayerBallResetRecords(void* self) {
    memset(((GolfDocRoundStorage*)self)->ballReset, 0, sizeof(((GolfDocRoundStorage*)self)->ballReset));
}

// Clamp the requested round index and select its mapped course-hole number.
void CGolfDoc_SetCurrentHole(void* self, unsigned int index) {
    // Requires roundHoleCount > 0. Both round index and mapped course-hole number
    // are zero-based; they differ for reordered/short rounds.
    unsigned int maximum = (unsigned char)(AsCSharedDoc(g_sharedDoc)->roundHoleCount - 1);
    if (index > maximum)
        index = maximum;
    AsCGolfDoc(self)->currentHoleIndex = index;
    unsigned char selected = (unsigned char)index;
    *(unsigned char*)&AsCGolfDoc(self)->pad_3B7 = g_sharedDoc->reserved082A[selected];
}

// Set each player's pending ball position to the current tee plus its vertical offset and clear the pending flag.
void CGolfDoc_ResetPlayerBallPositionsForHole(void* self) {
    float output[3];
    float source[3];
    source[1] = GolfBall__Fv()->ballVerticalOffset;
    source[0] = kFloat_0_GolfDoc;
    source[2] = kFloat_0_GolfDoc;
    unsigned char index = AsCGolfDoc(self)->currentHoleIndex;
    void* holeRecords = ((CGolfDoc*)self)->holeRecords;
    __pl__FRC7WVectorRC7WVector(output, (char*)holeRecords + index * 0x20, source);
    char* destination = (char*)self;
    for (int i = 0; i < AsCGolfDoc(self)->playerCount; ++i) {
        ((GolfDocRoundStorage*)destination)->ballReset[0].position[0] = output[0];
        ((GolfDocRoundStorage*)destination)->ballReset[0].position[1] = output[1];
        ((GolfDocRoundStorage*)destination)->ballReset[0].position[2] = output[2];
        ((GolfDocRoundStorage*)destination)->ballReset[0].pending = 0;
        destination += sizeof(GolfBallResetEntry);
    }
}

struct DtorNode {
    void* first;
    void* second;
    char reserved08[4];
    unsigned int flags;
    char reserved10[4];
    void* owned;
};

struct DtorPair {
    DtorNode* first;
    DtorNode* second;
    char reserved08[4];
    unsigned int flags;
    char reserved10[4];
    void* owned;
};

struct DtorRoot {
    DtorPair* first;
    DtorPair* second;
    char reserved08[4];
    unsigned int flags;
    char reserved10[4];
    void* owned;
};

#pragma use_lmw_stmw on
// Release the declaration-name tree and record storage, then optionally delete the declaration table.
void* GolfDeclarations_dtor(void* self, int freeMemory) {
    if (!self)
        return self;
    {
        void* owner = ((GolfDeclarationOwner*)self)->nameRoot;
        if (owner != 0) {
            DtorRoot* root = *(DtorRoot**)owner;
            if (0 != root) {
                DtorPair* pair = root->first;
                if (pair != 0) {
                    DtorNode* node = pair->first;
                    if (0 != node) {
                        if (node->first != 0)
                            StringValueTree_DestroySubtree(((GolfDeclarationOwner*)self)->nameIndex, node->first);
                        if (node->second != 0)
                            StringValueTree_DestroySubtree(((GolfDeclarationOwner*)self)->nameIndex, node->second);
                        if (&node->flags != 0 && &node->flags != 0 && (node->flags >> 31) != 0)
                            appFreeUni__FPv(node->owned);
                        appFreeUni__FPv(node);
                    }
                    DtorNode* savedSecond = pair->second;
                    node = savedSecond;
                    if (node != 0) {
                        if (node->first != 0)
                            StringValueTree_DestroySubtree(((GolfDeclarationOwner*)self)->nameIndex, node->first);
                        if (0 != node->second)
                            StringValueTree_DestroySubtree(((GolfDeclarationOwner*)self)->nameIndex, node->second);
                        if (&node->flags != 0 && &node->flags != 0 && (node->flags >> 31) != 0)
                            appFreeUni__FPv(node->owned);
                        appFreeUni__FPv(node);
                    }
                    if (&pair->flags != 0 && &pair->flags != 0 && (pair->flags >> 31) != 0)
                        appFreeUni__FPv(pair->owned);
                    appFreeUni__FPv(pair);
                }
                DtorPair* savedSecond2 = root->second;
                pair = savedSecond2;
                if (pair != 0) {
                    DtorNode* savedFirst = pair->first;
                    DtorNode* node = savedFirst;
                    if (node != 0) {
                        if (node->first != 0)
                            StringValueTree_DestroySubtree(((GolfDeclarationOwner*)self)->nameIndex, node->first);
                        if (node->second != 0)
                            StringValueTree_DestroySubtree(((GolfDeclarationOwner*)self)->nameIndex, node->second);
                        if (&node->flags != 0 && &node->flags != 0 && (node->flags >> 31) != 0)
                            appFreeUni__FPv(node->owned);
                        appFreeUni__FPv(node);
                    }
                    node = pair->second;
                    if (node != 0) {
                        if (node->first != 0)
                            StringValueTree_DestroySubtree(((GolfDeclarationOwner*)self)->nameIndex, node->first);
                        if (node->second != 0)
                            StringValueTree_DestroySubtree(((GolfDeclarationOwner*)self)->nameIndex, node->second);
                        if (&node->flags != 0 && &node->flags != 0 && (node->flags >> 31) != 0)
                            appFreeUni__FPv(node->owned);
                        appFreeUni__FPv(node);
                    }
                    if (&pair->flags != 0 && &pair->flags != 0 && (pair->flags >> 31) != 0)
                        appFreeUni__FPv(pair->owned);
                    appFreeUni__FPv(pair);
                }
                if (&root->flags != 0 && &root->flags != 0 && (root->flags >> 31) != 0)
                    appFreeUni__FPv(root->owned);
                appFreeUni__FPv(root);
            }
            root = *(DtorRoot**)((char*)owner + 4);
            if (root != 0) {
                DtorPair* savedFirst2 = root->first;
                DtorPair* pair = savedFirst2;
                if (0 != pair) {
                    DtorNode* savedFirst3 = pair->first;
                    DtorNode* node = savedFirst3;
                    if (0 != node) {
                        if (node->first)
                            StringValueTree_DestroySubtree(((GolfDeclarationOwner*)self)->nameIndex, node->first);
                        if (node->second != 0)
                            StringValueTree_DestroySubtree(((GolfDeclarationOwner*)self)->nameIndex, node->second);
                        if (&node->flags != 0 && &node->flags != 0 && (node->flags >> 31) != 0)
                            appFreeUni__FPv(node->owned);
                        appFreeUni__FPv(node);
                    }
                    node = pair->second;
                    if (node) {
                        if (0 != node->first)
                            StringValueTree_DestroySubtree(((GolfDeclarationOwner*)self)->nameIndex, node->first);
                        if (node->second != 0)
                            StringValueTree_DestroySubtree(((GolfDeclarationOwner*)self)->nameIndex, node->second);
                        if (&node->flags != 0 && &node->flags != 0 && (node->flags >> 31) != 0)
                            appFreeUni__FPv(node->owned);
                        appFreeUni__FPv(node);
                    }
                    if (&pair->flags != 0 && &pair->flags != 0 && (pair->flags >> 31) != 0)
                        appFreeUni__FPv(pair->owned);
                    appFreeUni__FPv(pair);
                }
                pair = root->second;
                if (pair != 0) {
                    DtorNode* node = pair->first;
                    if (node != 0) {
                        if (node->first != 0)
                            StringValueTree_DestroySubtree(((GolfDeclarationOwner*)self)->nameIndex, node->first);
                        if (node->second != 0)
                            StringValueTree_DestroySubtree(((GolfDeclarationOwner*)self)->nameIndex, node->second);
                        if (&node->flags != 0 && &node->flags != 0 && (node->flags >> 31) != 0)
                            appFreeUni__FPv(node->owned);
                        appFreeUni__FPv(node);
                    }
                    node = pair->second;
                    if (node != 0) {
                        if (node->first != 0)
                            StringValueTree_DestroySubtree(((GolfDeclarationOwner*)self)->nameIndex, node->first);
                        if (node->second)
                            StringValueTree_DestroySubtree(((GolfDeclarationOwner*)self)->nameIndex, node->second);
                        if (&node->flags != 0 && &node->flags != 0 && (node->flags >> 31) != 0)
                            appFreeUni__FPv(node->owned);
                        appFreeUni__FPv(node);
                    }
                    if (&pair->flags != 0 && &pair->flags != 0 && (pair->flags >> 31) != 0)
                        appFreeUni__FPv(pair->owned);
                    appFreeUni__FPv(pair);
                }
                if (&root->flags != 0 && &root->flags != 0 && (root->flags >> 31) != 0)
                    appFreeUni__FPv(root->owned);
                appFreeUni__FPv(root);
            }
            if ((((GolfStringTreeNode*)owner)->key.flags >> 31) != 0)
                appFreeUni__FPv(((GolfStringTreeNode*)owner)->key.data);
            appFreeUni__FPv(owner);
        }
        ((V8013*)self)->~V8013();
    }
    if (0 < freeMemory)
        __dl__FPv(self);
    return self;
}

} // extern "C"

} // namespace ns3

