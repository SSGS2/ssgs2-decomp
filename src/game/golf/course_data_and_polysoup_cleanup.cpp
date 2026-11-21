#include "iff/IffStruct.h"
#include "classes/CourseData801C.h"
#include "classes/Buffer801BC334.h"
#define AsBuffer801BC334(p) ((Buffer801BC334*)(p))
#include "classes/Buffer801BCB94.h"
#define AsBuffer801BCB94(p) ((Buffer801BCB94*)(p))
#include "classes/CGolfDoc.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "classes/WPolySoup.h"
#define AsWPolySoup(p) ((WPolySoup*)(p))
#include "game/singletons.h"
#include "sdk_protos.h"
#include "classes/CAllCourse.h"

#define AsCAllCourse(p) ((CAllCourse*)(p))

// Translation unit 0x801BBD68-0x801BD08C (7 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

// Views for cleanup-only records not represented in WPolySoup's shared header.
struct CourseSurfaceOwnership {
    char properties[0x10];
    void* resource;
    char reserved14[0x9C];
    void* arrayWithCookie;
    void** buffers;
    char reservedB8[0x24];
};
struct CourseResourceOwnership {
    char descriptor[0x20];
    void* resourceId;
    char reserved24[4];
};
struct CourseTreeLinks {
    void* left;
    void* right;
};
struct CourseCleanupLinks {
    void* next;
    void* previous;
};
struct CourseRoundSelection {
    char reserved0[0x818];
    unsigned char teeChoices[18];
    unsigned char holeNumbers[18];
    unsigned char reserved83C;
    unsigned char courseIndices[18];
    char reserved84F[0x181];
    unsigned char pars[18];
};
// ECB coefficients are copied in pairs; their physical units still need confirmation.
// Row layout: id/par/alternatePar, then horizontal X/Z pairs:
// pair[0] -> hole +0x00, pair[1] -> +0x08; tee pair -> +0x0C and +0x14.
// These are X/Z components of the hole vectors (Y at +0x04/+0x10 is untouched).
// Tee choice is a direct index 0..2 into the source row: no sorting or reversal.
// Modding contract: keep tee indices 0..2 and the selected normal/alternate bank.
// This routine only copies X/Z; it neither assigns nor clears destination Y.
// Tee colors/distances and coordinate units/origin are not established here; retain row order.
struct CourseHoleCoefficients {
    int id, par, alternatePar;
    float normalPair[2], alternatePair[2];
    float normalByTee[3][2];
    float alternateByTee[3][2];
};
struct CourseTableHeader {
    char reserved0[8];
    int rowCount;
};
struct CourseModelFlagView {
    char reserved0[0x360];
    unsigned char isDaytime;
};
// Retain byte-expression shape for CodeWarrior common-subexpression allocation.
enum CourseSubobjectOffset {
    kRegionStorageOffset = (int)&((WPolySoup*)0)->regions,
    kCourseStatisticsOffset = (int)&((WPolySoup*)0)->unk_20,
    kEffectTreeOffset = (int)&((WPolySoup*)0)->effectTreeCount,
    kEffectTreeRootOffset = (int)&((WPolySoup*)0)->effectTreeRoot,
    kFirstListSentinelOffset = (int)&((WPolySoup*)0)->auxiliaryNodeHead,
    kAmbientListSentinelOffset = (int)&((WPolySoup*)0)->ambientSoundHead,
    kGeneratedListSentinelOffset = (int)&((WPolySoup*)0)->generatedHead,
};
enum CourseContainerOffset {
    kResourceObjectsVectorOffset = 0x4930,
    kGeneratedContainerOffset = 0x490C,
    kAmbientContainerOffset = 0x4900,
    kAuxiliaryVectorTwoOffset = 0x48F4,
    kAuxiliaryVectorOneOffset = 0x48E8,
    kFirstListContainerOffset = 0x48DC,
    kStaticPetGroupOffset = 0xC8,
};
typedef char CourseSurfaceOwnership_size_check[(sizeof(CourseSurfaceOwnership) == 0xDC) ? 1 : -1];
typedef char CourseResourceOwnership_size_check[(sizeof(CourseResourceOwnership) == 0x28) ? 1 : -1];
typedef char CourseRoundSelection_par_check[((int)&((CourseRoundSelection*)0)->pars == 0x9D0) ? 1 : -1];
typedef char CourseHoleCoefficients_alternate_check[((int)&((CourseHoleCoefficients*)0)->alternateByTee == 0x34) ? 1 : -1];
enum CourseCleanupConstants {
    kArrayCookieSize = 0x10,
    kSurfaceStride = sizeof(CourseSurfaceOwnership),
    kResourceRecordStride = sizeof(CourseResourceOwnership),
    kFallbackCourseIndex = 0xFD,
    kHolesPerCourse = 18,
    kForcedPar = 3,
    kAlternateCourseVariant = 2,
    kCourseVariantMask = 0xF,
    kAllCoursePath = 0x4B,
};

extern "C" {

extern void* memset(void*, int, unsigned int);

extern void appFreeUni__FPv(void*);
extern void __dl__FPv(void*);
extern void __dla__FPv(void*);
extern void* Resource_OpenResolvedPath(void*, const char*, int, int, int, int);
extern void* appMallocUni__FUl(void*);
extern void Resource_Release(void*);
extern void* Table_GetEntry(void*, int);extern void WPolySoup_ReleaseResources(void*);
extern void ResourceManager_ReleaseId(void*, void*);
extern void ResourceManager_ReleaseObject(void*, void*, int);
extern void* CQuadTree_GetInstance();
extern void CQuadTree_FreeBuffers(void*);
extern void __destroy_new_array(void*, void*);
extern void ResourceManager_ReleaseId(void*, void*);
extern void CSceneManager_RemoveUnretainedRenderRecords(void*);
extern void CFx_ClearEffects(void*, int);
extern void RenderPacket_Init(void*);
extern void RenderPacket_ReleaseResources(void*);
extern void RenderPacket_Destroy(void);
extern char s_renderPacket[];
extern char s_renderPacketDtorRegistration_804FCA30[];
extern void* g_sceneManager;
extern void* g_fx;
extern signed char s_renderPacketInitialized;
extern void CBalloonSystem_DestroyAllQueuedBalloons(void);
extern void BaseObject_dtor(void*, int);
extern void FxTree_DestroySubtreeNodes(void*, void*);
extern char courseModelResourceStrings[];
struct CourseTable {
    unsigned int entries[14];
};
extern CourseTable s_courseHoleRowGroups;
extern char __vt__10CAllCourse[];
extern void* g_sharedDoc;extern char __vt__9WPolySoup[];
extern char __vt__Q29WPolySoup13CStaticPetGrp[];
extern void* g_resourceManager;

void* WPolySoup_dtor(void* object, int shouldDelete);

// Set the golf document's day/night byte used by day-only/night-only model tags.
// Marker type 3 clears isDaytime on GolfDoc_GetCurrent() to select night models.
void GolfDoc_SetDaytime(void* object, unsigned char value) {
    ((CourseModelFlagView*)object)->isDaytime = value;
}

// Returns dest (r3 untouched, 0x801BD080); CourseModel_LoadFromFile uses it (0x801BD654, 0x801BD664).
// Copy the three vector components and return the destination.
float* CourseVector_Copy3(float* dest, const float* src) {
    dest[0] = src[0];
    dest[1] = src[1];
    dest[2] = src[2];
    return dest;
}

// Release the owned storage string and optionally delete the buffer wrapper.
void* CourseNamedPoint_dtor(void* object, int shouldDelete) {
    if (object != 0) {
        if ((unsigned int)object + 0x30 != 0 && (AsBuffer801BCB94(object)->storageFlags >> 31) != 0) {
            appFreeUni__FPv(AsBuffer801BCB94(object)->ownedStorage);
        }
        if (shouldDelete > 0) {
            __dl__FPv(object);
        }
    }
    return object;
}

// Release the array buffer, clear its two counters, and optionally delete the wrapper.
void* Buffer_dtor(void* object, int shouldDelete) {
    if (object != 0) {
        void* data = AsBuffer801BC334(object)->data;
        AsBuffer801BC334(object)->unk_8 = 0;
        AsBuffer801BC334(object)->unk_A = 0;
        if (data != 0) {
            __dla__FPv(data);
            AsBuffer801BC334(object)->data = 0;
        }
        if (shouldDelete > 0) {
            __dl__FPv(object);
        }
    }
    return object;
}

#pragma use_lmw_stmw on
struct CleanupEntry {
    virtual ~CleanupEntry();
    char data[0x1c];
};
// Release deeper effect descendants before freeing this node; payloads are not owned here.
static inline void ReleaseCourseEffectLeaf(char* self, void* node) {
    if (node != 0) {
        if (*(void**)node != 0) FxTree_DestroySubtreeNodes(self + kEffectTreeOffset, *(void**)node);
        if (((CourseTreeLinks*)node)->right != 0) FxTree_DestroySubtreeNodes(self + kEffectTreeOffset, ((CourseTreeLinks*)node)->right);
        appFreeUni__FPv(node);
    }
}

// Release loaded course geometry, resource handles, effect lists, wind-linked effects, and render state.
// Ownership inventory (release order matters):
// surfaces own cookie arrays, Buffer wrappers and resource-manager objects;
// fallbackSurface owns its own cookie array/resource and wrapper;
// resourceEntries own manager IDs, cameras/bounds own cookie arrays, namedPoints
// own strings, regions own destructible entries but retain vector storage;
// effectTree and auxiliary/ambient/generated cleanup frees nodes without payload destructors;
// resourceIds own manager references; staticPetEntries owns an array.
// Scene/fx/render-packet singletons are borrowed and only have contents cleared.
void WPolySoup_ReleaseResources(void* object) {
    char* self = (char*)object;
    memset(self + kCourseStatisticsOffset, 0, 0x40);
    void* quadtree = (void*)CQuadTree_GetInstance();
    CQuadTree_FreeBuffers(quadtree);
    // Ownership phase 1: surfaces[] own cookie arrays, buffer wrappers and resources.
    // Each stored pointer is reloaded after releases; do not cache it across manager calls.
    if ((*(void**)&AsWPolySoup(self)->surfaces) != 0) {
        int off;
        int i = 0;
        off = 0;
        int zero = 0;
        while (i < (*(int*)&AsWPolySoup(self)->surfaceCount)) {
            if (((CourseSurfaceOwnership*)((char*)(*(void**)&AsWPolySoup(self)->surfaces) + off))->arrayWithCookie) {
                if (((CourseSurfaceOwnership*)((char*)(*(void**)&AsWPolySoup(self)->surfaces) + off))->arrayWithCookie) {
                    __dla__FPv((char*)((CourseSurfaceOwnership*)((char*)(*(void**)&AsWPolySoup(self)->surfaces) + off))->arrayWithCookie - kArrayCookieSize);
                }
                ((CourseSurfaceOwnership*)((char*)(*(void**)&AsWPolySoup(self)->surfaces) + off))->arrayWithCookie = 0;
            }
            if (((CourseSurfaceOwnership*)((char*)(*(void**)&AsWPolySoup(self)->surfaces) + off))->buffers != 0 && *(void**)((CourseSurfaceOwnership*)((char*)(*(void**)&AsWPolySoup(self)->surfaces) + off))->buffers != 0) {
                __dla__FPv(*(void**)((CourseSurfaceOwnership*)((char*)(*(void**)&AsWPolySoup(self)->surfaces) + off))->buffers);
                *(void**)((CourseSurfaceOwnership*)((char*)(*(void**)&AsWPolySoup(self)->surfaces) + off))->buffers = 0;
            }
            if (((CourseSurfaceOwnership*)((char*)(*(void**)&AsWPolySoup(self)->surfaces) + off))->buffers != 0) {
                __destroy_new_array(((CourseSurfaceOwnership*)((char*)(*(void**)&AsWPolySoup(self)->surfaces) + off))->buffers, (void*)Buffer_dtor);
                ((CourseSurfaceOwnership*)((char*)(*(void**)&AsWPolySoup(self)->surfaces) + off))->buffers = 0;
            }
            void* q = ((CourseSurfaceOwnership*)((char*)(*(void**)&AsWPolySoup(self)->surfaces) + off))->resource;
            if (q != 0) {
                ResourceManager_ReleaseObject(g_resourceManager, q, 0);
                ((CourseSurfaceOwnership*)((char*)(*(void**)&AsWPolySoup(self)->surfaces) + off))->resource = 0;
            }
            off += kSurfaceStride;
            ++i;
        }
    }
    // Phase 2: fallback surface has independent ownership and a distinct release flag.
    if ((*(void**)&AsWPolySoup(self)->fallbackSurface) != 0) {
        void* qb = ((CourseSurfaceOwnership*)AsWPolySoup(self)->fallbackSurface)->arrayWithCookie;
        if (qb != 0) {
            if (qb != 0) {
                __dla__FPv((char*)qb - kArrayCookieSize);
            }
            ((CourseSurfaceOwnership*)AsWPolySoup(self)->fallbackSurface)->arrayWithCookie = 0;
        }
        void* qc = ((CourseSurfaceOwnership*)AsWPolySoup(self)->fallbackSurface)->resource;
        if (qc != 0) {
            ResourceManager_ReleaseObject(g_resourceManager, qc, 1);
            ((CourseSurfaceOwnership*)AsWPolySoup(self)->fallbackSurface)->resource = 0;
        }
        __dl__FPv((*(void**)&AsWPolySoup(self)->fallbackSurface));
        (*(void**)&AsWPolySoup(self)->fallbackSurface) = 0;
    }
    // Phase 3: return manager IDs, then discard their descriptor arrays.
    if ((*(void**)&AsWPolySoup(self)->resourceEntries) != 0) {
        int i2 = 0;
        while (i2 < AsWPolySoup(self)->resourceEntryCount) {
            if (g_resourceManager != 0) {
                void* v = ((CourseResourceOwnership*)AsWPolySoup(self)->resourceEntries)[i2].resourceId;
                if (v != 0) {
                    ResourceManager_ReleaseId(g_resourceManager, v);
                    ((CourseResourceOwnership*)AsWPolySoup(self)->resourceEntries)[i2].resourceId = 0;
                }
            }
            ++i2;
        }
    }
    if ((*(void**)&AsWPolySoup(self)->resourceEntries) != 0) {
            if ((*(void**)&AsWPolySoup(self)->resourceEntries) != 0) {
                __dla__FPv((char*)(*(void**)&AsWPolySoup(self)->resourceEntries) - kArrayCookieSize);
            }
            (*(void**)&AsWPolySoup(self)->resourceEntries) = 0;
    }
    AsWPolySoup(self)->resourceEntryCount = 0;
    // Ownership phase 3: flat camera/surface/bounds arrays use a 0x10-byte cookie;
    // namedPoints additionally destroy owned strings. Null each owner after deletion.
    {
        void* p = (*(void**)&AsWPolySoup(self)->surfaces);
        if (p != 0) {
            if (p != 0) {
                __dla__FPv((char*)p - kArrayCookieSize);
            }
            (*(void**)&AsWPolySoup(self)->surfaces) = 0;
        }
    }
    {
        void* p = (*(void**)&AsWPolySoup(self)->cameras);
        if (p != 0) {
            if (p != 0) {
                __dla__FPv((char*)p - kArrayCookieSize);
            }
            (*(void**)&AsWPolySoup(self)->cameras) = 0;
        }
    }
    {
        // Named-point rows own the strings released by this destructor.
        void* p = AsCourseData801C(self)->namedPoints;
        if (p != 0) {
            __destroy_new_array(p, (void*)CourseNamedPoint_dtor);
            AsCourseData801C(self)->namedPoints = 0;
        }
    }
    {
        void* p = (*(void**)&AsWPolySoup(self)->bounds);
        if (p != 0) {
            if (p != 0) {
                __dla__FPv((char*)p - kArrayCookieSize);
            }
            (*(void**)&AsWPolySoup(self)->bounds) = 0;
        }
    }
    {
        // Phase 4: destroy region values in reverse order; capacity remains allocated.
        CleanupEntry* ent;
        int n = (*(int*)&AsWPolySoup(self)->regionCount);
        ent = (CleanupEntry*)(*(void**)&AsWPolySoup(self)->regions) + n;
        (*(int*)&AsWPolySoup(self)->regionCount) -= n;
        while (n != 0) {
            --ent;
            ent->~CleanupEntry();
            --n;
        }
    }
    {
        // Phase 5: postorder effect-tree node cleanup, unrolled through three levels.
        // FxTree_DestroySubtreeNodes handles deeper descendants; reset sentinel last.
        void* root = (*(void**)&AsWPolySoup(self)->effectTreeRoot);
        if (root != 0) {
            {
                void* depth1Node = *(void**)root;
                if (depth1Node != 0) {
                    {
                        void* depth2Node = *(void**)depth1Node;
                        if (depth2Node != 0) {
                            {
                                void* depth3Node = *(void**)depth2Node;
                                ReleaseCourseEffectLeaf(self, depth3Node);
                            }
                            {
                                void* depth3Node = ((CourseTreeLinks*)depth2Node)->right;
                                ReleaseCourseEffectLeaf(self, depth3Node);
                            }
                            appFreeUni__FPv(depth2Node);
                        }
                    }
                    {
                        void* depth2Node = ((CourseTreeLinks*)depth1Node)->right;
                        if (depth2Node != 0) {
                            {
                                void* depth3Node = *(void**)depth2Node;
                                ReleaseCourseEffectLeaf(self, depth3Node);
                            }
                            {
                                void* depth3Node = ((CourseTreeLinks*)depth2Node)->right;
                                ReleaseCourseEffectLeaf(self, depth3Node);
                            }
                            appFreeUni__FPv(depth2Node);
                        }
                    }
                    appFreeUni__FPv(depth1Node);
                }
            }
            {
                void* depth1Node = ((CourseTreeLinks*)root)->right;
                if (depth1Node != 0) {
                    {
                        void* depth2Node = *(void**)depth1Node;
                        if (depth2Node != 0) {
                            {
                                void* depth3Node = *(void**)depth2Node;
                                ReleaseCourseEffectLeaf(self, depth3Node);
                            }
                            {
                                void* depth3Node = ((CourseTreeLinks*)depth2Node)->right;
                                ReleaseCourseEffectLeaf(self, depth3Node);
                            }
                            appFreeUni__FPv(depth2Node);
                        }
                    }
                    {
                        void* depth2Node = ((CourseTreeLinks*)depth1Node)->right;
                        if (depth2Node != 0) {
                            {
                                void* depth3Node = *(void**)depth2Node;
                                ReleaseCourseEffectLeaf(self, depth3Node);
                            }
                            {
                                void* depth3Node = ((CourseTreeLinks*)depth2Node)->right;
                                ReleaseCourseEffectLeaf(self, depth3Node);
                            }
                            appFreeUni__FPv(depth2Node);
                        }
                    }
                    appFreeUni__FPv(depth1Node);
                }
            }
            appFreeUni__FPv(root);
            (*(void**)&AsWPolySoup(self)->effectTreeCount) = 0;
            (*(void**)&AsWPolySoup(self)->effectTreeRoot) = 0;
            AsWPolySoup(self)->effectTreeFirst = self + kEffectTreeRootOffset;
        }
    }
    // Ownership phase 5b: erase auxiliary, ambient and generated list NODES only.
    // Headers are embedded (count/next/previous); they survive ReleaseResources.
    // Splice the range out first, then follow previous links before freeing each node.
    // auxiliaryNode* is the first list; its payload role is unresolved.
    // auxiliaryVector1Count belongs to the separate vector at +48E8. It is reset here before
    // ambient nodes are erased; ambientSoundCount at +4900 is decremented per node.
    {
        void* sentinel = self + kFirstListSentinelOffset;
        void* node = AsWPolySoup(self)->auxiliaryNodeTail;
        if (node != sentinel) {
            void* first = *(void**)sentinel;
            ((CourseCleanupLinks*)*(void**)node)->previous = ((CourseCleanupLinks*)first)->previous;
            *(void**)((CourseCleanupLinks*)first)->previous = *(void**)node;
            while (node != sentinel) {
                void* nodeToFree = node;
                node = ((CourseCleanupLinks*)node)->previous;
                appFreeUni__FPv(nodeToFree);
                --(*(int*)&AsWPolySoup(self)->auxiliaryNodeCount);
            }
        }
    }
    {
        // Separate vector at +48E8 retains storage until WPolySoup_dtor.
        unsigned int retainedVectorCount = AsWPolySoup(self)->auxiliaryVector1Count;
        void* sentinel = self + kAmbientListSentinelOffset;
        void* node = AsWPolySoup(self)->ambientSoundTail;
        (*(int*)&AsWPolySoup(self)->auxiliaryVector1Count) -= retainedVectorCount;
        if (node != sentinel) {
            void* first = *(void**)sentinel;
            ((CourseCleanupLinks*)*(void**)node)->previous = ((CourseCleanupLinks*)first)->previous;
            *(void**)((CourseCleanupLinks*)first)->previous = *(void**)node;
            while (node != sentinel) {
                void* nodeToFree = node;
                node = ((CourseCleanupLinks*)node)->previous;
                appFreeUni__FPv(nodeToFree);
                --(*(int*)&AsWPolySoup(self)->ambientSoundCount);
            }
        }
    }
    {
        void* sentinel = self + kGeneratedListSentinelOffset;
        void* node = AsWPolySoup(self)->generatedTail;
        if (node != sentinel) {
            void* first = *(void**)sentinel;
            ((CourseCleanupLinks*)*(void**)node)->previous = ((CourseCleanupLinks*)first)->previous;
            *(void**)((CourseCleanupLinks*)first)->previous = *(void**)node;
            while (node != sentinel) {
                void* nodeToFree = node;
                node = ((CourseCleanupLinks*)node)->previous;
                appFreeUni__FPv(nodeToFree);
                --(*(int*)&AsWPolySoup(self)->generatedCount);
            }
        }
    }
    // Phase 6: clear the 256 embedded 0x48-byte effect attachments and release flat arrays.
    // Shared windActors/windActorCount name the same storage populated by
    // CourseModel_LoadFromFile::attachedEffectCount and read by CourseData_UpdateWindActors.
    // Spray/sequence pointers are borrowed from CFx; this memset does not destroy them.
    memset(AsWPolySoup(self)->windActors, 0, 0x4800);
    AsWPolySoup(self)->windActorCount = 0;
    {
        int i = 0;
        while (i < (*(int*)&AsWPolySoup(self)->resourceIdCount)) {
            ResourceManager_ReleaseId(g_resourceManager, *(void**)((*(char**)&AsWPolySoup(self)->resourceIds) + i * 4));
            ++i;
        }
    }
    (*(int*)&AsWPolySoup(self)->resourceIdCount) = 0;
    {
        void* p = (*(void**)&AsWPolySoup(self)->resourceIds);
        if (p != 0) {
            __dla__FPv(p);
            (*(void**)&AsWPolySoup(self)->resourceIds) = 0;
        }
    }
    {
        void* p = AsWPolySoup(self)->staticPetEntries;
        (*(int*)&AsWPolySoup(self)->resourceIdCapacity) = 0;
        if (p != 0) {
            __dla__FPv(p);
            AsWPolySoup(self)->staticPetEntries = 0;
        }
    }
    AsWPolySoup(self)->staticPetCount = 0;
    // Phase 7: clear shared scene/effect/render state after local references are gone.
    if (g_sceneManager != 0) CSceneManager_RemoveUnretainedRenderRecords(g_sceneManager);
    if (g_fx != 0) CFx_ClearEffects(g_fx, 0);
    if (Sing_805DC7B0() == 0) {
        if (s_renderPacketInitialized == 0) {
            RenderPacket_Init(s_renderPacket);
            __register_global_object(s_renderPacket, (void*)RenderPacket_Destroy, s_renderPacketDtorRegistration_804FCA30);
            s_renderPacketInitialized = 1;
        }
        Sing_805DC7B0() = s_renderPacket;
    }
    RenderPacket_ReleaseResources(Sing_805DC7B0());
}

// The repeated guards and unrolled tree levels preserve the compiler's original destructor expansion.
// Inline helpers changed register allocation even with propagation disabled.
#define IF_COURSE_SUBOBJECT(o) if ((unsigned int)self + (o) != 0)
#define IF_COURSE_SUBOBJECT_FOUR_TIMES(o) IF_COURSE_SUBOBJECT(o) IF_COURSE_SUBOBJECT(o) IF_COURSE_SUBOBJECT(o) IF_COURSE_SUBOBJECT(o)
// Vector record at o: data/count/capacity. This destructor frees retained capacity;
// ReleaseResources above cleared values. All repeated guards evaluate only addresses.
#define RELEASE_COURSE_VECTOR_STORAGE(o) \
    IF_COURSE_SUBOBJECT_FOUR_TIMES(o) IF_COURSE_SUBOBJECT(o) { \
        void* p = *(void**)(self + (o)); \
        if (p != 0) { \
            unsigned int n = *(unsigned int*)(self + (o) + 4); \
            *(int*)(self + (o) + 4) -= n; \
            appFreeUni__FPv(p); \
        } \
    }
// List at o: count/next/previous with embedded sentinel at o+4.
// Unlink the range first, walk previous links, then free nodes without payload deletion.
#define RELEASE_COURSE_LIST_NODES(o) \
    IF_COURSE_SUBOBJECT_FOUR_TIMES(o) { \
        void* node = *(void**)(self + (o) + 8); \
        void* h = self + (o) + 4; \
        if (node != h) { \
            void* first = *(void**)h; \
            ((CourseCleanupLinks*)*(void**)node)->previous = ((CourseCleanupLinks*)first)->previous; \
            *(void**)((CourseCleanupLinks*)first)->previous = *(void**)node; \
            while (node != h) { \
                void* nodeToFree = node; \
                node = ((CourseCleanupLinks*)node)->previous; \
                appFreeUni__FPv(nodeToFree); \
                --*(int*)(self + (o)); \
            } \
        } \
    }
#define RELEASE_COURSE_TREE_LEAF(p) { \
    void* c = (p); \
    if (c != 0) { \
        if (*(void**)c != 0) FxTree_DestroySubtreeNodes(self + kEffectTreeOffset, *(void**)c); \
        if (((CourseTreeLinks*)c)->right != 0) FxTree_DestroySubtreeNodes(self + kEffectTreeOffset, ((CourseTreeLinks*)c)->right); \
        appFreeUni__FPv(c); \
    } \
}
#define RELEASE_COURSE_TREE_BRANCH(p) { \
    void* b = (p); \
    if (b != 0) { \
        RELEASE_COURSE_TREE_LEAF(*(void**)b); \
        RELEASE_COURSE_TREE_LEAF(((CourseTreeLinks*)b)->right); \
        appFreeUni__FPv(b); \
    } \
}
#define RELEASE_COURSE_TREE_SIDE(p) { \
    void* a = (p); \
    if (a != 0) { \
        RELEASE_COURSE_TREE_BRANCH(*(void**)a); \
        RELEASE_COURSE_TREE_BRANCH(((CourseTreeLinks*)a)->right); \
        appFreeUni__FPv(a); \
    } \
}
#pragma opt_propagation off
// Release course contents and owned containers, destroy the base, and optionally delete the soup.
void* WPolySoup_dtor(void* object, int shouldDelete) {
    char* self = (char*)object;
    if (object != 0) {
        // First clear live payloads, then release retained container allocations.
        *(void**)object = &__vt__9WPolySoup;
        WPolySoup_ReleaseResources(object);
        if (*(void**)&AsWPolySoup(object)->resources != 0) {
            int i;
            int zero;
            int off;
            i = 0;
            off = 0;
            zero = 0;
            while (i < *(int*)&AsWPolySoup(self)->resourceCount) {
                if (g_resourceManager != 0) {
                    if (((CourseResourceOwnership*)((char*)AsWPolySoup(self)->resources + off))->resourceId != 0) {
                        ResourceManager_ReleaseId(g_resourceManager, ((CourseResourceOwnership*)((char*)AsWPolySoup(self)->resources + off))->resourceId);
                        (*(int*)&((CourseResourceOwnership*)((char*)AsWPolySoup(self)->resources + off))->resourceId) = zero;
                    }
                }
                off += kResourceRecordStride;
                ++i;
            }
        }
        if (*(void**)&AsWPolySoup(self)->resources != 0) {
            if (*(void**)&AsWPolySoup(self)->resources != 0)
                __dla__FPv((char*)*(void**)&AsWPolySoup(object)->resources - kArrayCookieSize);
            *(void**)&AsWPolySoup(object)->resources = 0;
        }
        {
        unsigned int i;
        int zero;
        int off;
        zero = 0;
        i = zero;
        *(int*)&AsWPolySoup(object)->resourceCount = zero;
        off = zero;
        while (i < *(volatile unsigned int*)&AsWPolySoup(self)->resourceObjectCount) {
            if (g_resourceManager != 0) {
                if (*(void**)(*(char**)&AsWPolySoup(self)->resourceObjects + off) != 0) {
                    ResourceManager_ReleaseObject(g_resourceManager, *(void**)(*(char**)&AsWPolySoup(self)->resourceObjects + off), 0);
                    *(int*)(*(char**)&AsWPolySoup(self)->resourceObjects + off) = zero;
                }
            }
            ++i;
            off += 4;
        }
        unsigned int n = AsWPolySoup(self)->resourceObjectCount;
        *(int*)&AsWPolySoup(self)->resourceObjectCount -= n;
        }
        CBalloonSystem_DestroyAllQueuedBalloons();
        if ((unsigned int)object + 0x4c80 != 0 && ((AsWPolySoup(object)->pendingFilenameFlags >> 31) != 0))
            appFreeUni__FPv(*(void**)&AsWPolySoup(object)->pendingFilenameData);
        RELEASE_COURSE_VECTOR_STORAGE(kResourceObjectsVectorOffset);
        RELEASE_COURSE_LIST_NODES(kGeneratedContainerOffset);
        RELEASE_COURSE_LIST_NODES(kAmbientContainerOffset);
        RELEASE_COURSE_VECTOR_STORAGE(kAuxiliaryVectorTwoOffset);
        RELEASE_COURSE_VECTOR_STORAGE(kAuxiliaryVectorOneOffset);
        RELEASE_COURSE_LIST_NODES(kFirstListContainerOffset);
        IF_COURSE_SUBOBJECT(kStaticPetGroupOffset) {
        AsWPolySoup(object)->staticPetGroupVtable = &__vt__Q29WPolySoup13CStaticPetGrp;
        if (AsWPolySoup(object)->staticPetEntries != 0) {
            __dla__FPv(AsWPolySoup(object)->staticPetEntries);
            AsWPolySoup(object)->staticPetEntries = 0;
        }
        AsWPolySoup(object)->staticPetCount = 0;
        }
        IF_COURSE_SUBOBJECT_FOUR_TIMES(kEffectTreeOffset) IF_COURSE_SUBOBJECT(kEffectTreeOffset) IF_COURSE_SUBOBJECT(kEffectTreeOffset) {
        void* root = *(void**)&AsWPolySoup(object)->effectTreeRoot;
        if (root != 0) {
            RELEASE_COURSE_TREE_SIDE(*(void**)root);
            RELEASE_COURSE_TREE_SIDE(((CourseTreeLinks*)root)->right);
            appFreeUni__FPv(root);
        }
        }
        IF_COURSE_SUBOBJECT_FOUR_TIMES(kRegionStorageOffset) {
        void* data = *(void**)(self + kRegionStorageOffset);
        if (data != 0) {
            int count = *(int*)&AsWPolySoup(self)->regionCount;
            CleanupEntry* entry;
            entry = (CleanupEntry*)data + count;
            *(int*)&AsWPolySoup(self)->regionCount -= (unsigned int)count;
            while (count != 0) {
                --entry;
                entry->~CleanupEntry();
                --count;
            }
            appFreeUni__FPv(*(void**)(self + kRegionStorageOffset));
        }
        }
        if (object != 0) BaseObject_dtor(object, 0);
        if (shouldDelete > 0) __dl__FPv(object);
    }
    return object;
}
#pragma opt_propagation reset
#undef RELEASE_COURSE_TREE_SIDE
#undef RELEASE_COURSE_TREE_BRANCH
#undef RELEASE_COURSE_TREE_LEAF
#undef RELEASE_COURSE_LIST_NODES
#undef RELEASE_COURSE_VECTOR_STORAGE
#undef IF_COURSE_SUBOBJECT_FOUR_TIMES
#undef IF_COURSE_SUBOBJECT
#pragma use_lmw_stmw reset

// Release and clear the watched resource handle on scope exit, including early returns.
struct PathHandle { void** value; ~PathHandle() {
    if (*value != 0) {
        if (*value != 0) {
            Resource_Release(*value);
            *value = 0;
        }
    }
} };
// Borrow table bytes; their explicit owner frees them after row copying.
struct InfoHandle { void* value; void* vtable; ~InfoHandle() {
} };
struct CourseResource {
    virtual void slot0();
    virtual void slot1();
    virtual void load(void*, void*);
    void* data;
};

// Load all_course.ecb and copy per-hole par and tee-selected coefficients for the round.
enum CourseParPolicy { kPartyForcePar3Mode1 = 1, kPartyForcePar3Mode2 = 2, kPartyAlternateParMode3 = 3 };

// Copy the selected ECB rows into round hole records; count is holes, not bytes.
int CAllCourse_LoadRoundHoleRecords(void* object, int count) {
    void* resource = Resource_OpenResolvedPath(AsCAllCourse(object)->resourceManager,
        courseModelResourceStrings + kAllCoursePath, -1, 0, 0, 0);
    PathHandle handle = { &resource };
    if (resource == 0) {
        return 0;
    }
    void* info = appMallocUni__FUl(((CourseResource*)resource)->data);
    if (info == 0) {
        if (resource != 0) {
            Resource_Release(resource);
            resource = 0;
        }
        return 0;
    }
    ((CourseResource*)resource)->load(info, ((CourseResource*)resource)->data);
    if (resource != 0) {
        Resource_Release(resource);
        resource = 0;
    }
    InfoHandle infoHandle;
    infoHandle.value = 0;
    if (info != 0) infoHandle.value = info;
    infoHandle.vtable = __vt__10CAllCourse;
    // For each selected hole, source selection lives in CSharedDoc; destination
    // lives in golfDoc->holeRecords (0x20-byte records). Choices must be 0..2.
    // Mode 3 uses alternate par; modes 1/2 force par 3; other modes use normal par.
    // The game-mode labels are not proven by this loader. 0xFD uses the final row
    // and ends the loop after one destination record. No validation is added.
    float teeVectorX[3];
    float teeVectorZ[3];
    int roundHoleIndex = 0;
    while (roundHoleIndex < count) {
#define COURSE_ELEM(field) (*(unsigned char*)((unsigned int)roundHoleIndex + (unsigned int)g_sharedDoc + (int)&((CourseRoundSelection*)0)->field))
        int courseIndex = COURSE_ELEM(courseIndices);
        int holeNumber = COURSE_ELEM(holeNumbers);
        void* row;
        if (courseIndex == kFallbackCourseIndex) {
            void* tmp = infoHandle.value;
            int r4 = (tmp != 0) ? ((CourseTableHeader*)tmp)->rowCount : 0;
            r4 = r4 - 1;
            row = Table_GetEntry(tmp, r4);
        } else {
            CourseTable stkTable = s_courseHoleRowGroups;
            row = Table_GetEntry(infoHandle.value, holeNumber + stkTable.entries[courseIndex] * kHolesPerCourse - 1);
        }
        int mode = (*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode);
        unsigned int holeRecordByteOffset;
#define COURSE_G ((char*)g_sharedDoc)
#define COURSE_DEST ((char*)AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->holeRecords + holeRecordByteOffset)
        // Par selection is independent of the low-nibble course variant below.
        // These policy names intentionally avoid guessing the party-mode game labels.
        switch (mode) {
        case kPartyAlternateParMode3:
            holeRecordByteOffset = ((unsigned int)roundHoleIndex & 0xFFu) << 5;
            *(unsigned char*)(COURSE_DEST + 0x18) = ((CourseHoleCoefficients*)row)->alternatePar;
            COURSE_ELEM(pars) = ((CourseHoleCoefficients*)row)->alternatePar;
            break;
        case kPartyForcePar3Mode1:
        case kPartyForcePar3Mode2:
            holeRecordByteOffset = ((unsigned int)roundHoleIndex & 0xFFu) << 5;
            *(unsigned char*)(COURSE_DEST + 0x18) = kForcedPar;
            COURSE_ELEM(pars) = kForcedPar;
            break;
        default:
            holeRecordByteOffset = ((unsigned int)roundHoleIndex & 0xFFu) << 5;
            *(unsigned char*)(COURSE_DEST + 0x18) = ((CourseHoleCoefficients*)row)->par;
            COURSE_ELEM(pars) = ((CourseHoleCoefficients*)row)->par;
            break;
        }
        void* x = AsCSharedDoc(g_sharedDoc)->selectedCourseData;
        unsigned char courseVariant;
        if (x == 0) courseVariant = 0;
        else courseVariant = ((IFF_STRUCT::sCourse*)x)->courseFlags & kCourseVariantMask;
        if (courseVariant == kAlternateCourseVariant) {
            *(float*)COURSE_DEST = ((CourseHoleCoefficients*)row)->alternatePair[0];
            *(float*)(COURSE_DEST + 8) = ((CourseHoleCoefficients*)row)->alternatePair[1];
            teeVectorX[0] = ((CourseHoleCoefficients*)row)->alternateByTee[0][0];
            teeVectorX[1] = ((CourseHoleCoefficients*)row)->alternateByTee[1][0];
            teeVectorX[2] = ((CourseHoleCoefficients*)row)->alternateByTee[2][0];
            teeVectorZ[0] = ((CourseHoleCoefficients*)row)->alternateByTee[0][1];
            teeVectorZ[1] = ((CourseHoleCoefficients*)row)->alternateByTee[1][1];
            teeVectorZ[2] = ((CourseHoleCoefficients*)row)->alternateByTee[2][1];
            *(float*)(COURSE_DEST + 0xc) = teeVectorX[COURSE_ELEM(teeChoices)];
            *(float*)(COURSE_DEST + 0x14) = teeVectorZ[COURSE_ELEM(teeChoices)];
        } else {
            *(float*)COURSE_DEST = ((CourseHoleCoefficients*)row)->normalPair[0];
            *(float*)(COURSE_DEST + 8) = ((CourseHoleCoefficients*)row)->normalPair[1];
            teeVectorX[0] = ((CourseHoleCoefficients*)row)->normalByTee[0][0];
            teeVectorX[1] = ((CourseHoleCoefficients*)row)->normalByTee[1][0];
            teeVectorX[2] = ((CourseHoleCoefficients*)row)->normalByTee[2][0];
            teeVectorZ[0] = ((CourseHoleCoefficients*)row)->normalByTee[0][1];
            teeVectorZ[1] = ((CourseHoleCoefficients*)row)->normalByTee[1][1];
            teeVectorZ[2] = ((CourseHoleCoefficients*)row)->normalByTee[2][1];
            *(float*)(COURSE_DEST + 0xc) = teeVectorX[COURSE_ELEM(teeChoices)];
            *(float*)(COURSE_DEST + 0x14) = teeVectorZ[COURSE_ELEM(teeChoices)];
        }
        if (courseIndex == kFallbackCourseIndex) break;
        ++roundHoleIndex;
    }
    appFreeUni__FPv(info);
    return 1;
}
#undef COURSE_ELEM
#undef COURSE_DEST
#undef COURSE_G

} // extern "C"
