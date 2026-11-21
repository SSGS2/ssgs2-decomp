#include <decomp/local_class.h>
#include "classes/GolfDocSnapshot8013.h"
#include "classes/CDartsTarget.h"
#include "classes/CourseBounds801C.h"
#define AsCDartsTarget(p) ((CDartsTarget*)(p))
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "classes/CSharedDoc.h"
#include "classes/CGolfDoc.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))

#include "game/types.h"
#include "game/wvector.h"
// Translation unit 0x802E1770-0x802E2DD8 (12 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".
#include "sdk_protos.h"
#include "classes/CDartsPlates.h"
#include "classes/CDartsSystem.h"
#include <decomp/cw_bitfields.h>

#define AsCDartsSystem(p) ((CDartsSystem*)(p))

#define AsCDartsPlates(p) ((CDartsPlates*)(p))

// Construct in caller-owned storage without allocating a new block.
inline void* operator new(size_t, void* p) { return p; }

extern "C" {

enum { kDartsMeshCells = 32, kDartsMeshSamples = kDartsMeshCells + 1,
       kDartsMeshVertices = kDartsMeshCells * kDartsMeshSamples * 2,
       kDartsMeshBytes = kDartsMeshVertices * 24,
       kDartsVertexFormat = 0x11F001, kDartsHoleCount = 18,
       kDartsHiddenGameStates = 0x000A0000, kDartsCheckPlayerStates = 0x10000800,
       kPlayerStateSuppressDarts = 3, kDartsDisplayPriority = 5,
       kDartsGroundColor = -128, kDartsPlateColor = -64 };
struct DartsStringPool {
    char aimTableFilename[0x11]; // "darts_target.ecb"
    char boundsName[6]; // "DARTS"
    char vectorLengthError[0x14];
    char lengthErrorType[0x37];
};
static const int kDartsVectorErrorOffset = (int)&((DartsStringPool*)0)->vectorLengthError;
static const int kDartsLengthErrorTypeOffset = (int)&((DartsStringPool*)0)->lengthErrorType;
static const int kDartsVertexBytes = sizeof(Vec3f) + sizeof(unsigned int) + 2 * sizeof(float);
static const int kDartsColorOffset = sizeof(Vec3f);
struct DartsHoleRecord { char reserved00[0xC]; unsigned int pinPositionBits[3]; char reserved18[8]; };
struct MiniGameAimDispatch { void* primaryVtable; void** secondaryVtable; };

struct DartsDialogOwnerView {
    char reserved00[0x8C];
    void* dialog;
};
extern void* CDartsDialog_Draw(void*);

// Draw the darts dialog when one is present.
void CDartsSystem_OnDisplay(DartsDialogOwnerView* self) {
    if (self->dialog != 0)
        CDartsDialog_Draw(self->dialog);
}

struct DartsSystemVtable {
    char reserved00[0x28];
    void (*destroy)(void*);
};
struct DartsSystemVtableView { DartsSystemVtable* vtable; };

extern DartsSystemVtable __vt__12CDartsSystem;
extern void __dt__18NativeBase80201D18Fv(void*, int);
extern void __dl__FPv(void*);

struct D8_ { 
virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5(); virtual void v6(); virtual void v7(); virtual void v8();
};
// Run the darts destruction hook, destroy the native base, and optionally free the system.
void* CDartsSystem_dtor(DartsSystemVtableView* self, int deleting) {
    if (self != 0) {
        void* vt = &__vt__12CDartsSystem;
        self->vtable = (DartsSystemVtable*)vt;
        ((D8_*)self)->v8();
        __dt__18NativeBase80201D18Fv(self, 0);
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

struct DartsOwnedBuffer { void* allocation; volatile int count; };
extern void appFreeUni__FPv(void*);

// Clear the owned buffer count, free its storage, and optionally delete the buffer owner.
void* DartsOwnedBuffer_dtor(DartsOwnedBuffer* self, int deleting) {
    if (self != 0) {
        if (self != 0) {
            if (self != 0) {
                if (self != 0) {
                    if (self != 0) {
                        if (self->allocation != 0) {
                            int oldCount = self->count;
                            self->count = oldCount - oldCount;
                            appFreeUni__FPv(self->allocation);
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

struct DartsSystemStorage {
    void* vtable;
    char reserved04[4];
    int displayPriority;
    char reserved0C[12];
    int hasAimPosition;
    float aimX, aimY, aimZ, originX, originY, originZ, heading;
    char reserved38[0x48];
    void* aimTable;
    void* target;
    void* plates;
    void* dialog;
    void* dialogRequestWord;
};

extern void __ct__18NativeBase80201D18Fv(void*);
extern void* memset(void*, int, unsigned int);
extern const float kFloat_0_darts;
extern CSharedDoc* g_sharedDoc;
extern int CDartsDialog_Update(void*, float);
extern void* GolfPlayer_GetEntryByIndex(void*, int);
extern int Get__15ISharedResourceFv(void*);
extern void CubeResourceStorage_dtor(void*, int);
extern void* ResourceManager_LoadByExtension(void*, const char*, int, int, int);
extern int CMiniGameAim_PollLoad(void*, int);
extern void DrawScene_SetNamedObjectPuppet(void*, const char*, int);
extern void ResourceManager_ReleaseObject(void*, void*, int);
extern const char dartsTargetStrings[];
extern char __vt__12CDartsPlates[];
extern char __vt__12CDartsTarget[];
extern char kDartsPlatesDetachSceneName[]; // plate scene name used when detaching the balloon
extern char kDartsBalloonModelFilename[]; // "dart_baloon_10.pet" model path
extern void* g_sceneManager;
extern void* g_resourceManager;

// Initialize the darts actor, position state, eighteen hole selections, and owned resource pointers.
DartsSystemStorage* CDartsSystem_Ctor(DartsSystemStorage* self) {
    __ct__18NativeBase80201D18Fv(self);
    float f0 = kFloat_0_darts;
    void* vt = &__vt__12CDartsSystem;
    int zero = 0;
    self->displayPriority = kDartsDisplayPriority;
    self->vtable = (DartsSystemVtable*)vt;
    self->aimTable = 0;
    self->target = 0;
    self->plates = 0;
    self->dialog = 0;
    self->dialogRequestWord = 0;
    self->hasAimPosition = 0;
    self->aimX = f0;
    self->aimY = f0;
    self->aimZ = f0;
    self->originX = f0;
    self->originY = f0;
    self->originZ = f0;
    self->heading = f0;
    memset(AsCDartsSystem(self)->holeEntryIds, 0, 0x48);
    return self;
}

// Suppress target/plate rendering for blocked game states and dispose of a completed dialog.
void CDartsSystem_OnProcess(DartsSystemStorage* self, float deltaSeconds) {
    void* shared = g_sharedDoc;
    void* golf = (*(void**)&AsCSharedDoc(shared)->golfDoc);
    int flags = AsCGolfDoc(golf)->gameStateFlags;
    int suppressRendering = (flags & kDartsHiddenGameStates) != 0;
    if ((flags & kDartsCheckPlayerStates) != 0) {
        void* player = GolfPlayer_GetEntryByIndex(golf, AsCGolfDoc(golf)->currentPlayer);
        if (((GolfDocSnapshot8013*)player)->playerState == kPlayerStateSuppressDarts)
            suppressRendering = 1;
    }
    AsCDartsTarget(AsCDartsSystem(self)->target)->renderSuppressed = suppressRendering;
    AsCDartsPlates(AsCDartsSystem(self)->plates)->renderSuppressed = suppressRendering;
    if (AsCDartsSystem(self)->dialog != 0) {
        if (CDartsDialog_Update(AsCDartsSystem(self)->dialog, deltaSeconds) != 0) {
            if (AsCDartsSystem(self)->dialog != 0) {
                if (AsCDartsSystem(self)->dialog != 0) {
                    struct Vtbl8C { 
                    virtual void destroy(int) DECOMP_LOCAL_VIRTUAL;
                    };
                    Vtbl8C* q = (Vtbl8C*)AsCDartsSystem(self)->dialog;
                    q->destroy(1);
                }
                AsCDartsSystem(self)->dialog = 0;
            }
        }
    }
}

struct Y7B { char pad[4]; };
class Y7 : public Y7B { public: 
virtual void v08(int);
};
// Delete the dialog, plates, target, and aim table through their original virtual slots.
void CDartsSystem_OnDestroy(DartsSystemStorage* self) {
    if (self->dialog != 0) {
        if (self->dialog != 0) {
            struct V8C { 
            virtual void destroy(int) DECOMP_LOCAL_VIRTUAL;
            };
            ((V8C*)self->dialog)->destroy(1);
        }
        self->dialog = 0;
    }
    if (self->plates != 0) {
        if (self->plates != 0) {
            struct V8D { 
            virtual void m0() DECOMP_LOCAL_VIRTUAL; virtual void destroy(int) DECOMP_LOCAL_VIRTUAL;
            };
            ((V8D*)self->plates)->destroy(1);
        }
        self->plates = 0;
    }
    if (self->target != 0) {
        if (self->target != 0) {
            struct V8D { 
            virtual void m0() DECOMP_LOCAL_VIRTUAL; virtual void destroy(int) DECOMP_LOCAL_VIRTUAL;
            };
            ((V8D*)self->target)->destroy(1);
        }
        self->target = 0;
    }
    if (self->aimTable != 0) {
        if (self->aimTable != 0) {
((Y7*)self->aimTable)->v08(1);
        }
        self->aimTable = 0;
    }
}

// Wait for aim/texture resources, then lazily load the balloon model.
void CDartsSystem_OnLoad(DartsSystemStorage* self) {
    while (!CMiniGameAim_PollLoad(self->aimTable, (int)dartsTargetStrings)) {
    }
    while (1) {
        void* p0C = AsCDartsTarget(self->target)->textureResource;
        int ready;
        // 0x802E1770 unit: a missing texture is already ready.
        if (p0C != 0 && Get__15ISharedResourceFv(p0C) == 0) {
            ready = 0;
        } else {
            ready = 1;
        }
        if (ready) break;
    }
    while (1) {
        void* p88 = self->plates;
        void* p20 = AsCDartsPlates(p88)->texture;
        int ready;
        do {
            if (p20 != 0) {
                if (Get__15ISharedResourceFv(p20) == 0) {
                    ready = 0;
                    break;
                }
            }
            if (AsCDartsPlates(p88)->balloonModel == 0) {
                AsCDartsPlates(p88)->balloonModel =
                ResourceManager_LoadByExtension(g_resourceManager, kDartsBalloonModelFilename, 0, 1, 0);
            }
            ready = 1;
        } while (0);
        if (ready) break;
    }
}

struct CDartsTargetObj { void* vtable; void* primaryMesh; void* secondaryMesh; void* textureResource; };

#pragma opt_propagation off
struct V1_ { 
virtual void s0(); virtual void s1();
};
// Release the texture and both reference-counted mesh handles; optionally delete the target.
void* CDartsTarget_dtor(volatile CDartsTargetObj* self, int deleting) {
    // Phase 1: release the texture interface. Phase 2: drop each buffer's packed
    // reference count, deleting the buffer at zero and then its owning handle.
    void* r29;
    if (self != 0) {
        void* vt = (void*)__vt__12CDartsTarget;
        r29 = 0;
        self->vtable = vt;
        void* p0C = self->textureResource;
        if (p0C != 0) {
            ((V1_*)p0C)->s1();
            self->textureResource = r29;
        }
        r29 = self->secondaryMesh;
        if (r29 != 0) {
            if (r29 != 0) {
            void* inner = *(void**)r29;
            if (inner != 0) {
                struct Counts { CW_BITS2( unsigned int refs:16, unsigned int other:16) };
                Counts c = *(Counts*)inner;
                --c.refs;
                *(Counts*)inner = c;
                if (c.refs == 0) {
                    CubeResourceStorage_dtor(inner, 1);
                }
                *(void**)r29 = 0;
            }
            __dl__FPv(r29);
            }
            self->secondaryMesh = 0;
        }
        r29 = self->primaryMesh;
        if (r29 != 0) {
            if (r29 != 0) {
            void* inner = *(void**)r29;
            if (inner != 0) {
                struct Counts { CW_BITS2( unsigned int refs:16, unsigned int other:16) };
                Counts c = *(Counts*)inner;
                --c.refs;
                *(Counts*)inner = c;
                if (c.refs == 0) {
                    CubeResourceStorage_dtor(inner, 1);
                }
                *(void**)r29 = 0;
            }
            __dl__FPv(r29);
            }
            self->primaryMesh = 0;
        }
        if (deleting > 0) {
            __dl__FPv((void*)self);
        }
    }
    return (void*)self;
}
#pragma opt_propagation reset

#pragma opt_propagation off
#pragma opt_common_subs off
#pragma use_lmw_stmw on
// Release every plate mesh, detach/free the balloon, release the texture, and free vector storage.
void* CDartsPlates_dtor(void* self, int deleting) {
    // Phase 1: drop mesh buffer references and delete handles; null the slots.
    void* zero;
    void* item;
    void** arr;
    if (self != 0) {
        arr = (*(void***)&AsCDartsPlates(self)->plates);
        *(void**)self = __vt__12CDartsPlates;
        zero = 0;
        while (arr != (void**)((char*)(*(void***)&AsCDartsPlates(self)->plates) + (AsCDartsPlates(self)->plateCount << 2))) {
            item = *arr;
            if (item != 0) {
                if (item != 0) {
                    if (item != 0) {
                        void* inner = *(void**)item;
                        if (inner != 0) {
                            struct Counts { CW_BITS2( unsigned int refs:16, unsigned int other:16) };
                            Counts c = *(Counts*)inner;
                            --c.refs;
                            *(Counts*)inner = c;
                            if (c.refs == 0) {
                                CubeResourceStorage_dtor(inner, 1);
                            }
                            *(void**)item = zero;
                        }
                        __dl__FPv(item);
                        *arr = zero;
                    }
                }
            }
            arr = arr + 1;
        }
        {
            AsCDartsPlates(self)->plateCount -= AsCDartsPlates(self)->plateCount;
        }
        // Phase 2: detach the borrowed scene binding before releasing its model.
        if (AsCDartsPlates(self)->balloonModel != 0) {
            DrawScene_SetNamedObjectPuppet(g_sceneManager, kDartsPlatesDetachSceneName, 0);
            ResourceManager_ReleaseObject(g_resourceManager, AsCDartsPlates(self)->balloonModel, 0);
            int r0 = 0;
            AsCDartsPlates(self)->balloonModel = (void*)r0;
        }
        zero = 0;
        if (AsCDartsPlates(self)->texture != 0) {
            struct V20 { 
            virtual void a() DECOMP_LOCAL_VIRTUAL; virtual void release() DECOMP_LOCAL_VIRTUAL;
            };
            ((V20*)AsCDartsPlates(self)->texture)->release();
            AsCDartsPlates(self)->texture = zero;
        }
        // Phase 3: free pointer-vector storage, then optionally this owner.
        if (&AsCDartsPlates(self)->plates != 0) {
            if (&AsCDartsPlates(self)->plates != 0) {
                if (&AsCDartsPlates(self)->plates != 0) {
                    if (&AsCDartsPlates(self)->plates != 0) {
                        if (&AsCDartsPlates(self)->plates != 0) {
                            if (AsCDartsPlates(self)->plates != 0) {
                                AsCDartsPlates(self)->plateCount -= AsCDartsPlates(self)->plateCount;
                                appFreeUni__FPv(AsCDartsPlates(self)->plates);
                            }
                        }
                    }
                }
            }
        }
        if (deleting > 0) {
            __dl__FPv((void*)self);
        }
    }
    return (void*)self;
}
#pragma use_lmw_stmw reset
#pragma opt_common_subs reset
#pragma opt_propagation reset

extern void* __nw__FUl(unsigned long);
extern void* CreateSharedTextureResource__FPCci(void*, int);
extern void ResourcePreloadQueue_AddRequest(void*, int, void*, float, void*, float, const char*, void*, unsigned int, unsigned int);
extern void CMiniGameAim_Ctor(void*);
extern const float kFloat_1_0_MenuModel;
extern void* s_dartsTargetTexturePath; // target shared-texture path
extern void* s_dartsPlatesTexturePath; // plate shared-texture path
extern char kDartsPlatesPreloadName[]; // plate preload request name
extern char kDartsTargetPreloadName[]; // target preload request name
extern const float kDartsGroundGridUvStep; // 1/32: ground-grid UV step
extern const float kDartsGroundGridUvToLocalScale; // 2: map UV [0,1] to local [-1,1]
extern float kFloat_0_12_GroundEffect;
extern void* Mtx44_SetRotationY(float*, float);
extern void Mtx44_TransformPointToVec4(float*, float*, float*);
extern void Vec3_Add(float*, float*, void*);
extern void Vec3_CopySourceToDestination(float*, float*);
extern void* CQuadTree_GetInstance(void);
extern void GetGroundPoint__9CQuadTreeF7WVectorbPib(float*, void*, float*, int, int, int);
extern void __as__7WVectorFRC7WVector(float*, float*);
extern void Vec3_CopyToOutput(float*, void*);
extern void gtl_PixelCopy_CopyPackedColor(void*, void*);
extern void VertexBuffer_Create(unsigned int, int, unsigned int, int, void*, int);
extern void Resource_GetOffsetPtr(void*, int, int, void*, void*);
extern void GraphicsBuffer_Flush(void*);
extern void* memcpy(void*, const void*, unsigned int);

// Owned handle to a reference-counted vertex buffer. size is the strip primitive
// count (vertices - 2), not a byte size; release ptr before deleting the handle.
struct DartsMeshHandle { void* ptr; int size; };
typedef Vec3f DartsPoint;
struct DartsVertex { DartsPoint pos; unsigned int color; float u, v; };
// Build a terrain-conforming serpentine triangle strip with UVs, color, and a small height bias.
// origin borrows a world-space XYZ vector; halfExtent uses course distance units,
// heading is radians about Y. The output handle receives a newly created buffer.
// Vertex world Y is max(rotatedOriginY, terrainY) + 0.12, keeping it above both.
void DartsGroundMesh_Create(DartsMeshHandle* self, unsigned int color, void* origin, float halfExtent, float heading) {
    float zero, z, one, v;
    DartsVertex results[kDartsMeshVertices];
    void* ptr;
    DartsPoint hit, raised, position, contact;
    struct { unsigned int x, y, z; } query;
    float transformed[4];
    float matrix[16];
    float uvSamples[kDartsMeshSamples], localSamples[kDartsMeshSamples];
    unsigned int i;
    // Phase 1: sample local coordinates [-halfExtent,+halfExtent] and UV [0,1].
    for (i = 0; i <= kDartsMeshCells; ++i) {
        uvSamples[i] = (float)i * kDartsGroundGridUvStep;
        localSamples[i] = halfExtent * (kDartsGroundGridUvToLocalScale * uvSamples[i] - kFloat_1_0_MenuModel);
    }
    Mtx44_SetRotationY(matrix, heading);
    ((unsigned int*)&position)[0] = 0;
    ((unsigned int*)&position)[1] = 0;
    ((unsigned int*)&position)[2] = 0;
    DartsVertex* out = results;
    zero = kFloat_0_darts;
    one = kFloat_1_0_MenuModel;
    int count = 0;
    // Phase 2: alternate each row's traversal to join all 32 strips in one
    // stream. Rotate local X/Z, translate to world, then query ground height.
    for (unsigned int row = 0; row < kDartsMeshCells; ++row) {
        int reverse = row & 1;
        for (unsigned int col = 0; col <= kDartsMeshCells; ++col) {
            unsigned int bi = col;
            if (reverse) bi = kDartsMeshCells - col;
            z = localSamples[bi];
            v = one - uvSamples[bi];
            for (unsigned int side = 0; side < 2; ++side) {
                unsigned int index = row + side;
                position.y = zero;
                position.z = z;
                position.x = localSamples[index];
                Mtx44_TransformPointToVec4((float*)&transformed, (float*)&position, matrix);
                position.x = transformed[0];
                position.y = transformed[1];
                position.z = transformed[2];
                Vec3_Add((float*)&position, (float*)&position, origin);
                Vec3_CopySourceToDestination((float*)&position, (float*)&hit);
                query.x = ((unsigned int*)&hit)[0];
                query.y = ((unsigned int*)&hit)[1];
                query.z = ((unsigned int*)&hit)[2];
                GetGroundPoint__9CQuadTreeF7WVectorbPib((float*)&contact, CQuadTree_GetInstance(), (float*)&query, 1, 0, 1);
                *(WVector*)((float*)&raised) = *(WVector*)((float*)&contact);
                if (hit.y < raised.y)
                    *(WVector*)((float*)&hit) = *(WVector*)((float*)&raised);
                hit.y += kFloat_0_12_GroundEffect;
                Vec3_CopyToOutput((float*)&hit, out);
                void* colorOut = (void*)((unsigned int)results + count * kDartsVertexBytes + kDartsColorOffset);
                out->u = uvSamples[index];
                ++count;
                out->v = v;
                ++out;
                gtl_PixelCopy_CopyPackedColor(colorOut, &color);
            }
        }
    }
    // Phase 3: upload 2112 position/color/UV vertices and flush the GPU buffer.
    VertexBuffer_Create(kDartsMeshBytes, 0, kDartsVertexFormat, 0, self, 0);
    Resource_GetOffsetPtr(self->ptr, 0, 0, &ptr, 0);
    memcpy(ptr, results, kDartsMeshBytes);
    GraphicsBuffer_Flush(*(void**)self);
    self->size = kDartsMeshVertices - 2;
}

extern void* g_randomHouse;
extern float g_dartsGroundMeshHalfExtent; // 40: ground mesh half extent
extern const float kFloat_1e02_Darts;
extern const float kDartsHeadingDegreeCount; // 360: random heading degree count
extern const float kFloat_6_2831855;
extern const float kFloat_1_5707964_tweenDartsAudio;
extern unsigned int MT19937_NextU32(void*);
extern void* CourseData_FindBoundsByNameSubstring(void*, const char*);
extern void* CMiniGameAim_ResolveEntries(void*);
extern unsigned int CMiniGameAim_GetEntryCount(void*);
extern void Matrix_SetIdentityAffine(void*);
extern void Matrix_CopyScaledBasis(void*, void*, float);
extern void Matrix_CopyAffine(void*, void*);
extern void Model_SetMatrixRecursive(void*, int, void*);
extern void WPuppet_UpdateBoneWorldMatrices(void*, int, int, float);
extern void WPuppet_UpdateBoundsFromBone(void*);
extern void WPuppet_SetOpacityRecursive(void*, int);

struct DartsSystemInitView {
    char reserved00[0x18];
    int enabled;
    float x;
    char reserved20[4];
    float z;
    float pos[3];
    float angle;
    char reserved38[0x48];
    void* resource;
    void* effect;
    void* model;
    void* child;
    void* manager;
    void* state;
    void* list;
    int tail;
};

extern char kDartsPlatesAttachSceneName[]; // plate scene name used when binding the balloon
extern char __vt__Q23std11logic_error[], s_sharedCharDeleterVtable[], __vt__Q23std12length_error[];
extern void __dt__Q23std12length_errorFv(void*, int);
extern void* __nwa__FUl(unsigned int);
extern void* appMallocDummy__FUl(unsigned int);
extern void __dla__FPv(void*);
extern "C" void __dt__Q23std9exceptionFv(void*, int);
extern void __dt__Q33std6detail16char_array_guardFv(void*, int);
extern void dtor_80046988(void*, int);
extern void Vector_CopyComponents(void*, void*);
extern void Vec3_Subtract(void*, void*, void*);
extern double atan2(double, double);
extern void Darts_CreateRotatedTexturedQuad(void*, unsigned int, unsigned int, void*, void*, float, float);
struct DartPreset { unsigned int id; float v[3]; };
extern DartPreset kDartsPlateMeshPresets[4]; // four plate texture/size presets

struct DartArray {
    DartsMeshHandle** data;
    unsigned int count, capacity;
    unsigned int* owner;
    unsigned int offset;
    // Start an empty guarded replacement array; p identifies the owning capacity.
    DartArray(unsigned int* p) : data(0), count(0), capacity(0), owner(p), offset(0) {}
    // Free pointer storage only; mesh handles are released explicitly by the owner.
    ~DartArray() {
        if (data) {
            count = 0;
            appFreeUni__FPv(data);
        }
    }
};
struct DartBuffer {
    void* data;
    unsigned char* flag;
    // Guard an exception-message array until its control block is created.
    DartBuffer(void* p, unsigned char* f) : data(p), flag(f) {}
    // Delete the message array unless ownership was transferred by clearing data.
    ~DartBuffer() { void* p = data; data = 0; if (p) __dla__FPv(p); }
};
struct DartErrorBase {
    void* vtable;
    char* message;
    void* control;
    // Duplicate the length-error message into reference-counted character storage.
    DartErrorBase(const char* text) {
        vtable = __vt__Q23std11logic_error;
        struct Flag { unsigned char value; char pad[3]; } flag;
        flag.value = 0;
        unsigned int size = strlen(text) + 1;
        message = (char*)__nwa__FUl(size * 2 - size);
        char* allocated = message;
        {
            DartBuffer buffer(allocated, &flag.value);
            unsigned int* p = (unsigned int*)__nw__FUl(16);
            if (p) {
                p[1] = 1;
                p[2] = 1;
                p[0] = (unsigned int)s_sharedCharDeleterVtable;
                p[3] = (unsigned int)allocated;
            }
            control = p;
            buffer.data = 0;
        }
        strcpy(message, text);
    }
    // Destroy the exception base without deleting its enclosing storage.
    ~DartErrorBase() { __dt__Q23std9exceptionFv(this, 0); }};
struct DartError : DartErrorBase {
    // Install the derived length-error runtime table after constructing the message.
    DartError(const char* text) : DartErrorBase(text) { vtable = __vt__Q23std12length_error; }
};

// Keep this throw scope at each call site: an inline function changes .text.
#define DART_LENGTH_ERROR() { \
    struct { unsigned int words[3]; } storage; \
    DartError& error = *new (&storage) DartError(dartsTargetStrings + kDartsVectorErrorOffset); \
    __throw(dartsTargetStrings + kDartsLengthErrorTypeOffset, &error, (void*)__dt__Q23std12length_errorFv); \
}

// Decrement the packed 16-bit reference field, deleting the resource at zero.
static inline void DartsMesh_DropReference(DartsMeshHandle* item) {
    void* inner = item->ptr;
    if (inner) {
        struct Counts { CW_BITS2( unsigned int refs:16, unsigned int other:16) };
        Counts counts = *(Counts*)inner;
        --counts.refs;
        *(Counts*)inner = counts;
        if (counts.refs == 0) CubeResourceStorage_dtor(inner, 1);
        item->ptr = 0;
    }
}
// Release a mesh handle and its shared resource, if present.
static inline void DartsMesh_Delete(DartsMeshHandle* item) {
    if (item) {
        DartsMesh_DropReference(item);
        __dl__FPv(item);
    }
}

struct DartEffect {
    void* vtable;
    DartsMeshHandle* first;
    DartsMeshHandle* second;
    void* resource;
    int active;
};
struct DartModel {
    void* vtable;
    DartsPoint pos;
    float angle;
    DartsMeshHandle** data;
    unsigned int count;
    unsigned int capacity;
    void* resource;
    void* model;
    int active;
};

// Choose the larger vector growth increment by reference.
inline const unsigned int& DartMax(const unsigned int& a, const unsigned int& b) {
    return a < b ? b : a;
}

// Build the ground strip at a world position with a quarter-turn heading correction.
inline void BuildDartRay(DartsMeshHandle* item, DartsPoint position, float angle) {
    DartsGroundMesh_Create(item, kDartsGroundColor, &position, g_dartsGroundMeshHalfExtent, angle - kFloat_1_5707964_tweenDartsAudio);
}

// Choose a random darts position/orientation and hole entry, then rebuild ground and plate meshes.
void CDartsSystem_OnInit(DartsSystemInitView* self) {
    // Requires the preload/load stages to have created target, plates and an
    // aim table with at least one entry (selection below uses modulo its count).
    // Phase 1: choose an X/Z point inside named bounds, a radian heading, and
    // this hole's aim-table index. Missing bounds leave enabled cleared.
    float rotated[12], matrix[12];
    DartsPoint hit;
    DartsPoint query;
    volatile DartsPoint original;
    DartPreset* preset;
    unsigned int i;
    self->enabled = 0;
    void* game = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    void* res = CourseData_FindBoundsByNameSubstring(AsCGolfDoc(game)->courseData, ((const DartsStringPool*)dartsTargetStrings)->boundsName);
    if (res) {
        float t = (float)(MT19937_NextU32(g_randomHouse) % 101) / kFloat_1e02_Darts;
        float u = (float)(MT19937_NextU32(g_randomHouse) % 101) / kFloat_1e02_Darts;
        // 0x802E1770 unit: sample X/Z within the named course bounds (+0x44..0x58).
        self->x = t * (AsCourseBounds801C(res)->maxX - AsCourseBounds801C(res)->minX) + AsCourseBounds801C(res)->minX;
        self->z = u * (AsCourseBounds801C(res)->maxZ - AsCourseBounds801C(res)->minZ) + AsCourseBounds801C(res)->minZ;
        self->enabled = 1;
    }
    unsigned int n = MT19937_NextU32(g_randomHouse);
    self->angle = kFloat_6_2831855 * (float)(n % 360) / kDartsHeadingDegreeCount;
    CMiniGameAim_ResolveEntries(self->resource);
    unsigned int index = AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->currentHoleIndex;
    if ((int)index < 0 || index >= kDartsHoleCount) index = 0;
    void* rng = g_randomHouse;
    unsigned int limit = CMiniGameAim_GetEntryCount(self->resource);
    ((unsigned int*)AsCDartsSystem(self)->holeEntryIds)[index] = MT19937_NextU32(rng) % limit;

    // Phase 2: ground the current pin position and rebuild the target ground mesh.
    game = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    DartsHoleRecord* entry = (DartsHoleRecord*)AsCGolfDoc(game)->holeRecords + AsCGolfDoc(game)->currentHoleIndex;
    struct Words { unsigned int x, y, z; };
    unsigned int wordZ, wordY, wordX;
    wordX = entry->pinPositionBits[0];
    wordY = entry->pinPositionBits[1];
    wordZ = entry->pinPositionBits[2];
    ((volatile Words*)&original)->x = wordX;
    ((volatile Words*)&original)->y = wordY;
    ((volatile Words*)&original)->z = wordZ;
    ((Words*)&query)->x = wordX;
    ((Words*)&query)->y = wordY;
    ((Words*)&query)->z = wordZ;
    GetGroundPoint__9CQuadTreeF7WVectorbPib((float*)&hit, CQuadTree_GetInstance(), (float*)&query, 1, 0, 1);
    hit.y += kFloat_0_12_GroundEffect;
    Vec3_CopyToOutput((float*)&hit, self->pos);

    DartEffect* effect = (DartEffect*)self->effect;
    DartsPoint position = *(DartsPoint*)self->pos;
    float angle = self->angle;
    DartsMeshHandle* item = effect->first;
    if (item) {
        DartsMesh_Delete(item);
        effect->first = 0;
    }
    item = effect->second;
    if (item) {
        DartsMesh_Delete(item);
        effect->second = 0;
    }
    item = (DartsMeshHandle*)__nw__FUl(8);
    if (item) item->ptr = 0;
    BuildDartRay(item, position, angle);
    effect->second = item;
    effect->active = 0;

    // Phase 3: bind the balloon model at the grounded pin with zero scale/opacity;
    // discard old plate handles before creating their replacements.
    DartsPoint target = *(DartsPoint*)&self->x;
    DartsPoint origin = *(DartsPoint*)self->pos;
    DartModel* model = (DartModel*)self->model;
    if (model->model) {
        Matrix_SetIdentityAffine(matrix);
        Matrix_CopyScaledBasis(rotated, matrix, kFloat_0_darts);
        Matrix_CopyAffine(matrix, rotated);
        Vec3_CopySourceToDestination((float*)&origin, matrix + 9);
        Model_SetMatrixRecursive(model->model, 0, matrix);
        WPuppet_UpdateBoneWorldMatrices(model->model, 0, 0, kFloat_1_0_MenuModel);
        WPuppet_UpdateBoundsFromBone(model->model);
        WPuppet_SetOpacityRecursive(model->model, 0);
        DrawScene_SetNamedObjectPuppet(g_sceneManager, kDartsPlatesAttachSceneName, (int)model->model);
    }
    DartsMeshHandle** current = model->data;
    while (current != model->data + model->count) {
        DartsMeshHandle* item = *current;
        if (item) {
            if (item) {
                if (item) {
                    DartsMesh_DropReference(item);
                }
                __dl__FPv(item);
            }
            *current = 0;
        }
        ++current;
    }
    unsigned int oldCount = model->count;
    model->count -= oldCount;
    DartsPoint targetCopy = target;
    DartsPoint originCopy = origin;
    Vector_CopyComponents(&model->pos, &originCopy);
    DartsPoint direction;
    Vec3_Subtract(&direction, &targetCopy, &model->pos);
    model->angle = (float)atan2(direction.x, direction.z);

    // Phase 4: construct four textured plate presets facing the sampled target.
    // Vector growth moves handle pointers; each handle still owns its buffer ref.
    preset = kDartsPlateMeshPresets;
    for (i = 0; i < 4; ++i, ++preset) {
        DartsPoint pos = model->pos;
        float a = model->angle;
        DartsMeshHandle* item = (DartsMeshHandle*)__nw__FUl(8);
        if (item) item->ptr = 0;
        DartsPoint settings = *(DartsPoint*)preset->v;
        DartsPoint positionCopy = pos;
        Darts_CreateRotatedTexturedQuad(item, kDartsPlateColor, preset->id, &positionCopy, &settings, preset->v[1], a);
        if (model->capacity > model->count) {
            unsigned int size = ++model->count;
            model->data[size - 1] = item;
        } else {
            if (0x3FFFFFFF - model->capacity < 1) DART_LENGTH_ERROR();
            unsigned int extra = model->count + 1 - model->capacity;
            DartArray array(&model->capacity);
            unsigned int cap = model->capacity;
            if (extra > 0x3FFFFFFF - cap) DART_LENGTH_ERROR();
            unsigned int newCapacity;
            if (cap < 0x15555555) {
                unsigned int grow = ((cap + 1) * 3) / 5;
                newCapacity = cap + DartMax(grow, extra);
            } else if (cap < 0x2AAAAAAA) {
                unsigned int grow = (cap + 1) / 2;
                newCapacity = cap + DartMax(grow, extra);
            } else newCapacity = 0x3FFFFFFF;
            if (newCapacity > 0x3FFFFFFF) DART_LENGTH_ERROR();
            DartsMeshHandle** allocation = (DartsMeshHandle**)appMallocDummy__FUl(newCapacity * 4);
            unsigned int size = model->count;
            array.offset = size;
            array.capacity = newCapacity;
            (allocation + size)[array.count] = item;
            DartsMeshHandle** old = model->data;
            DartsMeshHandle** end = old + model->count;
            array.data = allocation;
            ++array.count;
            int moved = ((char*)end - (char*)old) / 4;
            array.offset = size - moved;
            memcpy(allocation + array.offset, old, moved * 4);
            memset(old, 0, moved * 4);
            model->count = 0;
            array.count += moved;
            unsigned int oldCapacity = model->capacity;
            model->capacity = array.capacity;
            array.capacity = oldCapacity;
            DartsMeshHandle** oldData = model->data;
            model->data = array.data;
            array.data = oldData;
            model->count = array.count;
            array.count = 0;
        }
    }
    model->active = 0;
    self->manager = 0;
}

// Replace the aim table, target, and plate resources and enqueue their preload requests.
void CDartsSystem_OnPreLoadInit(DartsSystemStorage* self) {
    // Replace each owned object in order: aim table, target, then plates.
    // The scene queue borrows the target/plate pointers; their lifetime must
    // cover the queued scene records. Both start with rendering suppressed.
    if (self->aimTable != 0) {
        if (self->aimTable != 0) {
            void* p = self->aimTable;
            ((void(*)(void*, int))((MiniGameAimDispatch*)p)->secondaryVtable[2])(p, 1);
        }
        self->aimTable = 0;
    }
    void* p = __nw__FUl(0x10);
    if (p != 0) {
        CMiniGameAim_Ctor(p);
    }
    void* old84 = self->target;
    self->aimTable = p;
    if (old84 != 0) {
        if (old84 != 0) {
            ((void(*)(void*, int))(*(void*** )old84)[3])(old84, 1);
        }
        self->target = 0;
    }
    void* p2 = __nw__FUl(0x14);
    if (p2 != 0) {
        *(void**)p2 = (void*)__vt__12CDartsTarget;
        ResourcePreloadQueue_AddRequest(g_sceneManager, 1, 0, kFloat_1_0_MenuModel, p2, kFloat_0_darts, kDartsTargetPreloadName, 0, 0, 0);
        AsCDartsTarget(p2)->primaryMeshHandle = 0;
        AsCDartsTarget(p2)->vertexStreamInfo = 0;
        void* r = CreateSharedTextureResource__FPCci(s_dartsTargetTexturePath, 0);
        AsCDartsTarget(p2)->textureResource = r;
        AsCDartsTarget(p2)->renderSuppressed = 1;
    }
    void* old88 = self->plates;
    self->target = p2;
    if (old88 != 0) {
        if (old88 != 0) {
            ((void(*)(void*, int))(*(void*** )old88)[3])(old88, 1);
        }
        self->plates = 0;
    }
    void* p3 = __nw__FUl(0x2C);
    if (p3 != 0) {
        *(void**)p3 = (void*)__vt__12CDartsPlates;
        AsCDartsPlates(p3)->plates = 0;
        (*(void**)&AsCDartsPlates(p3)->plateCount) = 0;
        (*(void**)&AsCDartsPlates(p3)->plateCapacity) = 0;
        AsCDartsPlates(p3)->balloonModel = 0;
        ResourcePreloadQueue_AddRequest(g_sceneManager, 4, 0, kFloat_1_0_MenuModel, p3, kFloat_0_darts, kDartsPlatesPreloadName, 0, 8, 0);
        void* r = CreateSharedTextureResource__FPCci(s_dartsPlatesTexturePath, 0);
        AsCDartsPlates(p3)->texture = r;
        AsCDartsPlates(p3)->renderSuppressed = 1;
    }
    self->plates = p3;
}

} // extern "C"
