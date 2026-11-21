#include <decomp/static_init.h>
#include "game/wvector.h"
#include "classes/CGeneTex3D.h"
#include "classes/CFxParticle.h"
#include "classes/CFxSpray.h"

#define AsCFxParticle(p) ((CFxParticle*)(p))

#define AsCGeneTex3D(p) ((CGeneTex3D*)(p))
// Translation unit 0x8010F380-0x80110A50 (28 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

extern const float kFloat_6_2831855;
extern float s_twoPi_FxParticle;
extern const float kFloat_0_FxSpray;
extern const float kFloat_1_FxSpray;
extern const float kFloat_0_5_FxSpray;
extern const float kFloat_neg_1_805DE648;
extern const float kFloat_0_00001_FxSpray;
extern char __vt__11CFxParticle[0xC];
extern char __vt__8CFxSpray[0xC];
extern char __vt__10CGeneTex3D[0x24];

void FxSpray_NoOpLifecycleHook(void*, void*);
void CFxSpray_SetWindVector(void*, void*);
void RBTree_RebalanceAfterErase(void*, void*);
void appFreeUni__FPv(void*);
void __dl__FPv(void*);
void __dla__FPv(void*);
void Vec3_Zero(void*);
void CFxSpray_Reset(void*);
void* memset(void*, int, unsigned);
void __pl__FRC7WVectorRC7WVector(void*, void*, void*);
void __ml__FRC7WVectorf(void*, void*, float);
void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
void AABB_Copy(void*, void*);
void __as__7WVectorFRC7WVector(void*, void*);
float Magnitude__7WVectorCFv(void*);

// Initialize this particle unit's cached full-turn angle.
void CFxParticle_InitializeUnitConstants() {
    s_twoPi_FxParticle = kFloat_6_2831855;
}

struct SprayMapNode {
    SprayMapNode* left;
    SprayMapNode* right;
    unsigned parent;
    unsigned key;
    void* value;
};
struct SpraySequenceMap {
    char pad[0x20];
    unsigned count;
    SprayMapNode* root;
    SprayMapNode* first;
    char pad2[0x190];
    void* value;
};

enum { kSprayMapSentinelOffset = (int)&((SpraySequenceMap*)0)->root };

// Erase a spray by ID and clear the selected-spray pointer when it refers to that spray.
void CFxSequence_UnregisterSpray(SpraySequenceMap* self, unsigned key) {
    void* candidate = (void*)((unsigned)self + kSprayMapSentinelOffset);
    void* node = self->root;
    while (node != 0) {
        if (((SprayMapNode*)node)->key >= key) {
            candidate = node;
            node = *(void**)node;
        } else {
            node = (*(void**)&((SprayMapNode*)node)->right);
        }
    }
    if (candidate == (char*)self + kSprayMapSentinelOffset || key < ((SprayMapNode*)candidate)->key) candidate = (char*)self + kSprayMapSentinelOffset;
    if (candidate != (char*)self + kSprayMapSentinelOffset) {
        void* val = ((SprayMapNode*)candidate)->value;
        if (val == self->value) self->value = 0;
        void* firstNode = (*(void**)&self->first);
        void* erased = candidate;
        if (candidate == firstNode) {
            void* parent = (*(void**)&((SprayMapNode*)candidate)->right);
            if (parent != 0) {
                while (*(void**)parent != 0) parent = *(void**)parent;
                candidate = parent;
            } else {
                void* ancestor;
                while (candidate != *(void**)(ancestor = (void*)(((SprayMapNode*)candidate)->parent & ~1u))) {
                    candidate = ancestor;
                }
                candidate = ancestor;
            }
            (*(void**)&self->first) = candidate;
        }
        RBTree_RebalanceAfterErase(erased, self->root);
        appFreeUni__FPv(erased);
        --self->count;
    }
}

// Initialize an empty particle list, two history buffers and default force/opacity scales.
void CFxParticle_ctor(void* self) {
    *(void**)self = (char*)&__vt__11CFxParticle;
    AsCFxParticle(self)->listNodeCount = 0;
    void* end = (char*)&AsCFxParticle(self)->listTail;
    AsCFxParticle(self)->listHead = end;
    AsCFxParticle(self)->listTail = end;
    AsCFxParticle(self)->positionHistory = 0;
    AsCFxParticle(self)->positionHistoryCapacity = 0;
    AsCFxParticle(self)->positionHistoryWriteIndex = 0;
    AsCFxParticle(self)->positionHistoryReadIndex = 0;
    AsCFxParticle(self)->ribbonHistory = 0;
    AsCFxParticle(self)->ribbonHistoryCapacity = 0;
    AsCFxParticle(self)->ribbonHistoryWriteIndex = 0;
    AsCFxParticle(self)->ribbonHistoryReadIndex = 0;
    AsCFxParticle(self)->spriteResource = 0;
    (*(unsigned int*)&AsCFxParticle(self)->petBody) = 0;
    AsCFxParticle(self)->forceScale = kFloat_0_FxSpray;
    AsCFxParticle(self)->opacityScale = kFloat_1_FxSpray;
}

// Compute box-axis or radial falloff in the same coordinate space as position.
// Box layout: both shapes are {minX,minY,minZ,maxX,maxY,maxZ}; the scale arguments
// only enable X/Z or Y falloff when <1, rather than multiplying the result.
// Radial layout: outerShape[0..2] is the center; innerShape[3] ends the rising
// distance/radius ramp, innerShape[4] starts the falling ramp, outerShape[4] ends
// it. Other slots are unused here. Degenerate ramps leave the multiplier at 1;
// the result is not clamped, so callers must provide positions/limits accordingly.
float FxSpray_ComputeSpatialFalloff(float* position, float* outerShape, float* innerShape, float horizontalScale, float verticalScale, int radialMode) {
    float result = kFloat_1_FxSpray;
    if (radialMode == 0) {
        if (horizontalScale < kFloat_1_FxSpray) {
            float axisPosition = position[0];
            float innerEdge = innerShape[0];
            if (axisPosition < innerEdge) {
                result *= kFloat_1_FxSpray - (axisPosition - innerEdge) / (outerShape[0] - innerEdge);
            } else {
                innerEdge = innerShape[3];
                if (innerEdge < axisPosition) result *= kFloat_1_FxSpray - (axisPosition - innerEdge) / (outerShape[3] - innerEdge);
            }
            axisPosition = position[2];
            innerEdge = innerShape[2];
            if (axisPosition < innerEdge) result *= kFloat_1_FxSpray - (axisPosition - innerEdge) / (outerShape[2] - innerEdge);
            else {
                innerEdge = innerShape[5];
                if (innerEdge < axisPosition) result *= kFloat_1_FxSpray - (axisPosition - innerEdge) / (outerShape[5] - innerEdge);
            }
        }
        if (verticalScale < kFloat_1_FxSpray) {
            float axisPosition = position[1];
            float innerEdge = innerShape[1];
            if (axisPosition < innerEdge) result *= kFloat_1_FxSpray - (axisPosition - innerEdge) / (outerShape[1] - innerEdge);
            else {
                innerEdge = innerShape[4];
                if (innerEdge < axisPosition) result *= kFloat_1_FxSpray - (axisPosition - innerEdge) / (outerShape[4] - innerEdge);
            }
        }
    } else {
        
        float delta;
        WVector centerOffset = *(WVector*)position - *(WVector*)outerShape;
        float distance = centerOffset.Magnitude();
        if (distance <= innerShape[3]) {
            unsigned extentBits = *(unsigned*)&innerShape[3];
            float extent;
            if (extentBits & 0x80000000u) extent = -*(volatile float*)&innerShape[3];
            else extent = *(volatile float*)&innerShape[3];
            int less = extent < kFloat_0_00001_FxSpray;
            if (!less) result *= distance / innerShape[3];
        } else if (innerShape[4] <= distance) {
            delta = outerShape[4] - innerShape[4];
            unsigned deltaBits = *(unsigned*)&delta;
            float absDelta = delta;
            if (deltaBits & 0x80000000u) absDelta = -delta;
            int less = absDelta < kFloat_0_00001_FxSpray;
            if (!less) result *= kFloat_1_FxSpray - (distance - innerShape[4]) / (outerShape[4] - innerShape[4]);
        }
    }
    return result;
}

// Release the owned particle body, history arrays and list nodes; optionally free the object.
void* CFxParticle_dtor(void* self, int deleting) {
    if (self != 0) {
        void* p = AsCFxParticle(self)->petBody;
        *(void**)self = (char*)&__vt__11CFxParticle;
        if (p != 0) {
            if (p != 0) {
                ((void (**)(void*, int))(*(void**)p))[2](p, 1);
                AsCFxParticle(self)->petBody = 0;
            }
        }
        if (&AsCFxParticle(self)->ribbonHistory != 0) {
            p = (*(void**)&AsCFxParticle(self)->ribbonHistory);
            if (p != 0) {
                if (p != 0) __dla__FPv((char*)p - 0x10);
            }
        }
        if (&AsCFxParticle(self)->positionHistory != 0) {
            p = (*(void**)&AsCFxParticle(self)->positionHistory);
            if (p != 0) {
                if (p != 0) __dla__FPv((char*)p - 0x10);
            }
        }
        if ((int)self + 0x68 != 0) {
            if ((int)self + 0x68 != 0) {
                if ((int)self + 0x68 != 0) {
                    if ((int)self + 0x68 != 0) {
                        void* node = AsCFxParticle(self)->listHead;
                        void* end = (char*)&AsCFxParticle(self)->listTail;
                        if (node != end) {
                            void* first = *(void**)end;
                            *(void**)((char*)*(void**)node + 4) = *(void**)((char*)first + 4);
                            *(void**)(*(void**)((char*)first + 4)) = *(void**)node;
                            while (node != end) {
                                void* cur = node;
                                node = *(void**)((char*)node + 4);
                                appFreeUni__FPv(cur);
                                --(*(volatile unsigned*)&AsCFxParticle(self)->listNodeCount);
                            }
                        }
                    }
                }
            }
        }
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

// Constructor view for the SAME spray object as CFxSpray, not a second allocation.
// effectOwner is borrowed; particle/keyframe nodes and vertex storage belong to
// the spray. Each TrackList is {count, sentinel.prev, sentinel.next}; the sentinel
// is embedded at &previous, and both links point there when empty. Unknown tracks
// keep neutral names. The header view and this view must retain identical offsets.
struct SprayConstructorStorage {
    char reserved00[0x0c]; float attachmentX, attachmentY;
    char positionAttachment[0x10], rotationAttachment[0x10];
    char reserved34[0x4c]; void* particleSentinel;
    char reserved84[0x124]; float emitterPosition[3], previousEmitterPosition[3];
    char reserved1c0[0x9c];
    struct TrackList { unsigned count; void* previous; void* next; } initialSpeedPercentTrack;
    char reserved268[0x30];
    TrackList angularSpeed, velocityRotation, particleOrbit, emitterOrbit;
    TrackList additionalTrack0, spawn, gravityPoint, color;
    float minInitialSize, maxInitialSize, spriteAspectRatio;
    TrackList size, spriteFrame, additionalTrack1, additionalTrack2;
    float minDrawDistance, maxDrawDistance;
    TrackList additionalTrack3, additionalTrack4, additionalTrack5;
    char reserved360[0x0c]; unsigned char vertexCount; char reserved36d[3];
    void* vertexBufferVtable; unsigned vertexBuffer;
};
#define SPRAY_CTOR_OFF(f, o) typedef char SprayConstructor_##f[((int)&((SprayConstructorStorage*)0)->f == o) ? 1 : -1]
SPRAY_CTOR_OFF(particleSentinel, 0x80);
SPRAY_CTOR_OFF(emitterPosition, 0x1a8);
SPRAY_CTOR_OFF(initialSpeedPercentTrack, 0x25c);
SPRAY_CTOR_OFF(angularSpeed, 0x298);
SPRAY_CTOR_OFF(color, 0x2ec);
SPRAY_CTOR_OFF(size, 0x304);
SPRAY_CTOR_OFF(additionalTrack3, 0x33c);
SPRAY_CTOR_OFF(vertexCount, 0x36c);
#undef SPRAY_CTOR_OFF

// Keep sentinel address lifetimes stable across the reset call.
enum SprayListOffsets {
    kParticleSentinelOffset = (int)&((SprayConstructorStorage*)0)->particleSentinel,
    kEmissionTrackSentinelOffset = (int)&((SprayConstructorStorage*)0)->initialSpeedPercentTrack.previous,
    kAngularSpeedTrackSentinelOffset = (int)&((SprayConstructorStorage*)0)->angularSpeed.previous,
    kVelocityRotationTrackSentinelOffset = (int)&((SprayConstructorStorage*)0)->velocityRotation.previous,
    kParticleOrbitTrackSentinelOffset = (int)&((SprayConstructorStorage*)0)->particleOrbit.previous,
    kEmitterOrbitTrackSentinelOffset = (int)&((SprayConstructorStorage*)0)->emitterOrbit.previous,
    kAdditionalTrack0SentinelOffset = (int)&((SprayConstructorStorage*)0)->additionalTrack0.previous,
    kSpawnTrackSentinelOffset = (int)&((SprayConstructorStorage*)0)->spawn.previous,
    kGravityPointTrackSentinelOffset = (int)&((SprayConstructorStorage*)0)->gravityPoint.previous,
    kColorTrackSentinelOffset = (int)&((SprayConstructorStorage*)0)->color.previous,
    kSizeTrackSentinelOffset = (int)&((SprayConstructorStorage*)0)->size.previous,
    kSpriteFrameTrackSentinelOffset = (int)&((SprayConstructorStorage*)0)->spriteFrame.previous,
    kAdditionalTrack1SentinelOffset = (int)&((SprayConstructorStorage*)0)->additionalTrack1.previous,
    kAdditionalTrack2SentinelOffset = (int)&((SprayConstructorStorage*)0)->additionalTrack2.previous,
    kAdditionalTrack3SentinelOffset = (int)&((SprayConstructorStorage*)0)->additionalTrack3.previous,
    kAdditionalTrack4SentinelOffset = (int)&((SprayConstructorStorage*)0)->additionalTrack4.previous,
    kAdditionalTrack5SentinelOffset = (int)&((SprayConstructorStorage*)0)->additionalTrack5.previous
};

// Initialize spray ownership, attachment records, keyframe-list sentinels and vertex storage.
void* CFxSpray_Ctor(void* self, void* effectOwner) {
    // Phase 1: establish owner, empty particle list and emitter positions.
    unsigned char* p = (unsigned char*)self;
    *(void**)p = (char*)&__vt__8CFxSpray;
    ((CFxSpray*)p)->effectOwner = effectOwner;
    ((CFxSpray*)p)->particleCount = 0;
    (*(void**)&((CFxSpray*)p)->particleListHead) = p + kParticleSentinelOffset;
    (*(void**)&((SprayConstructorStorage*)p)->particleSentinel) = p + kParticleSentinelOffset;
    Vec3_Zero(((SprayConstructorStorage*)p)->emitterPosition);
    Vec3_Zero(((SprayConstructorStorage*)p)->previousEmitterPosition);
    // Phase 2: self-link every embedded keyframe sentinel before Reset sees them.
    ((SprayConstructorStorage*)p)->initialSpeedPercentTrack.next = p + kEmissionTrackSentinelOffset;
    ((SprayConstructorStorage*)p)->initialSpeedPercentTrack.previous = p + kEmissionTrackSentinelOffset;
    ((SprayConstructorStorage*)p)->initialSpeedPercentTrack.count = 0;
    (*(void**)&((CFxSpray*)p)->angularSpeedTrackHead) = p + kAngularSpeedTrackSentinelOffset;
    ((SprayConstructorStorage*)p)->angularSpeed.previous = p + kAngularSpeedTrackSentinelOffset;
    ((CFxSpray*)p)->angularSpeedTrackCount = 0;
    (*(void**)&((CFxSpray*)p)->velocityRotationTrackHead) = p + kVelocityRotationTrackSentinelOffset;
    ((SprayConstructorStorage*)p)->velocityRotation.previous = p + kVelocityRotationTrackSentinelOffset;
    ((CFxSpray*)p)->velocityRotationTrackCount = 0;
    ((CFxSpray*)p)->particleOrbitTrackCount = 0; // CFxSpray_Ctor at 0x8010FB70
    (*(void**)&((CFxSpray*)p)->particleOrbitTrackHead) = p + kParticleOrbitTrackSentinelOffset;
    ((SprayConstructorStorage*)p)->particleOrbit.previous = p + kParticleOrbitTrackSentinelOffset;
    ((CFxSpray*)p)->emitterOrbitTrackCount = 0;
    (*(void**)&((CFxSpray*)p)->emitterOrbitTrackHead) = p + kEmitterOrbitTrackSentinelOffset;
    ((SprayConstructorStorage*)p)->emitterOrbit.previous = p + kEmitterOrbitTrackSentinelOffset;
    ((SprayConstructorStorage*)p)->additionalTrack0.count = 0;
    ((SprayConstructorStorage*)p)->additionalTrack0.next = p + kAdditionalTrack0SentinelOffset;
    ((SprayConstructorStorage*)p)->additionalTrack0.previous = p + kAdditionalTrack0SentinelOffset;
    ((CFxSpray*)p)->spawnTrackCount = 0;
    (*(void**)&((CFxSpray*)p)->spawnTrackHead) = p + kSpawnTrackSentinelOffset;
    ((SprayConstructorStorage*)p)->spawn.previous = p + kSpawnTrackSentinelOffset;
    ((CFxSpray*)p)->gravityPointTrackCount = 0;
    (*(void**)&((CFxSpray*)p)->gravityPointTrackHead) = p + kGravityPointTrackSentinelOffset;
    ((SprayConstructorStorage*)p)->gravityPoint.previous = p + kGravityPointTrackSentinelOffset;
    ((CFxSpray*)p)->colorTrackCount = 0;
    (*(void**)&((CFxSpray*)p)->colorTrackHead) = p + kColorTrackSentinelOffset;
    ((SprayConstructorStorage*)p)->color.previous = p + kColorTrackSentinelOffset;
    ((CFxSpray*)p)->sizeTrackCount = 0;
    (*(void**)&((CFxSpray*)p)->sizeTrackHead) = p + kSizeTrackSentinelOffset;
    ((SprayConstructorStorage*)p)->size.previous = p + kSizeTrackSentinelOffset;
    ((CFxSpray*)p)->spriteFrameTrackCount = 0;
    (*(void**)&((CFxSpray*)p)->spriteFrameTrackHead) = p + kSpriteFrameTrackSentinelOffset;
    ((SprayConstructorStorage*)p)->spriteFrame.previous = p + kSpriteFrameTrackSentinelOffset;
    ((SprayConstructorStorage*)p)->additionalTrack1.count = 0;
    ((SprayConstructorStorage*)p)->additionalTrack1.next = p + kAdditionalTrack1SentinelOffset;
    ((SprayConstructorStorage*)p)->additionalTrack1.previous = p + kAdditionalTrack1SentinelOffset;
    ((SprayConstructorStorage*)p)->additionalTrack2.count = 0;
    ((SprayConstructorStorage*)p)->additionalTrack2.next = p + kAdditionalTrack2SentinelOffset;
    ((SprayConstructorStorage*)p)->additionalTrack2.previous = p + kAdditionalTrack2SentinelOffset;
    ((SprayConstructorStorage*)p)->additionalTrack3.count = 0;
    ((SprayConstructorStorage*)p)->additionalTrack3.next = p + kAdditionalTrack3SentinelOffset;
    ((SprayConstructorStorage*)p)->additionalTrack3.previous = p + kAdditionalTrack3SentinelOffset;
    ((SprayConstructorStorage*)p)->additionalTrack4.count = 0;
    ((SprayConstructorStorage*)p)->additionalTrack4.next = p + kAdditionalTrack4SentinelOffset;
    ((SprayConstructorStorage*)p)->additionalTrack4.previous = p + kAdditionalTrack4SentinelOffset;
    ((SprayConstructorStorage*)p)->additionalTrack5.count = 0;
    ((SprayConstructorStorage*)p)->additionalTrack5.next = p + kAdditionalTrack5SentinelOffset;
    ((SprayConstructorStorage*)p)->additionalTrack5.previous = p + kAdditionalTrack5SentinelOffset;
    // Phase 3: initialize geometry, then reset runtime state with valid empty lists.
    ((SprayConstructorStorage*)p)->vertexCount = 0;
    ((SprayConstructorStorage*)p)->vertexCount = 0;
    ((SprayConstructorStorage*)p)->vertexBufferVtable = (char*)&__vt__10CGeneTex3D;
    ((SprayConstructorStorage*)p)->vertexBuffer = 0;
    CFxSpray_Reset(self);
    // Phase 4: leave both attachment records cleared and their indices unselected.
    ((SprayConstructorStorage*)p)->attachmentX = kFloat_neg_1_805DE648;
    ((SprayConstructorStorage*)p)->attachmentY = kFloat_neg_1_805DE648;
    memset(((SprayConstructorStorage*)p)->positionAttachment, 0, 0x10);
    memset(((SprayConstructorStorage*)p)->rotationAttachment, 0, 0x10);
    return self;
}

// Scale box bounds or radial extents for the requested horizontal and vertical falloff.
void FxSpray_ComputeScaledBounds(int arg, void* out, void* box, float x, float y) {
    struct Bounds { float x, y, z, upperX, upperY, upperZ; };
    if (0 == arg) {
        if (x < kFloat_1_FxSpray || y < kFloat_1_FxSpray) {
            float center[3];
            float extent[3];
            
            
            WVector sum = *(WVector*)((float*)box + 3) + *(WVector*)box;
            __ml__FRC7WVectorf(center, &sum, kFloat_0_5_FxSpray);
            WVector difference = *(WVector*)((void*)((unsigned)box + 0x0C)) - *(WVector*)box;
            __ml__FRC7WVectorf(extent, &difference, kFloat_0_5_FxSpray);
            if (x < kFloat_1_FxSpray) {
                ((Bounds*)out)->x = (center[0] - extent[0]) * x;
                ((Bounds*)out)->z = (center[2] - extent[2]) * x;
                ((Bounds*)out)->upperX = (center[0] + extent[0]) * x;
                ((Bounds*)out)->upperZ = (center[2] + extent[2]) * x;
            } else {
                ((Bounds*)out)->x = *(float*)box;
                ((Bounds*)out)->upperX = ((Bounds*)box)->upperX;
                ((Bounds*)out)->z = ((Bounds*)box)->z;
                ((Bounds*)out)->upperZ = ((Bounds*)box)->upperZ;
            }
            if (y < kFloat_1_FxSpray) {
                ((Bounds*)out)->y = (center[1] - extent[1]) * y;
                ((Bounds*)out)->upperY = (center[1] + extent[1]) * y;
            } else {
                ((Bounds*)out)->y = ((Bounds*)box)->y;
                ((Bounds*)out)->upperY = ((Bounds*)box)->upperY;
            }
        } else {
            AABB_Copy(out, box);
        }
        return;
    }
    *(WVector*)out = *(WVector*)box;
    ((Bounds*)out)->upperX = ((Bounds*)box)->upperY * x;
    float height = ((Bounds*)box)->upperY * y;
    ((Bounds*)out)->upperZ = kFloat_0_FxSpray;
    ((Bounds*)out)->upperY = height;
}

struct SpraySequenceNode { SpraySequenceNode* left; SpraySequenceNode* child; SpraySequenceNode* parent; unsigned sprayId; void* value; };
struct SpraySequenceTree { char pad[0x24]; void* sentinel; SpraySequenceNode* root; };

// Visit the spray tree in key order and apply the wind vector to each spray.
void CFxSequence_SetWindVector(SpraySequenceTree* tree, void* arg) {
    SpraySequenceNode* node = tree->root;
    SpraySequenceNode* stop = (SpraySequenceNode*)&tree->sentinel;
    while (node != stop) {
        CFxSpray_SetWindVector(node->value, arg);
        SpraySequenceNode* child = node->child;
        if (child != 0) {
            while (child->left != 0) child = child->left;
            node = child;
        } else {
            while (node != ((SpraySequenceNode*)((unsigned)node->parent & ~1u))->left) node = (SpraySequenceNode*)((unsigned)node->parent & ~1u);
            node = (SpraySequenceNode*)((unsigned)node->parent & ~1u);
        }
    }
}

// List link layout used by dtor_8010FDDC through dtor_80110204. Each wrapper
// owns its nodes, whose payload needs no destructor here. The sentinel is the
// embedded pair at self+4, never a separately allocated node. Clear first splices
// the range out of the ring, then follows next links, freeing nodes and reducing
// count. Deleting (>0) additionally releases the wrapper; -1/0 retain it.
// ParticleList_dtor is called for CFxSpray::particleCount in fx_spray_list_and_loading.cpp. The precise
// payload-to-address mapping for the other specializations is not established;
// they deliberately retain their shared address names and their guard shapes.
struct ParticleListLinks { void* previous; void* next; };
struct ParticleListStorage { unsigned int size; void* sentinelPrevious; void* first; };
#define PARTICLE_LIST_OFF(T, f, o) typedef char T##_off_##f[((int)&((T*)0)->f == (o)) ? 1 : -1]
PARTICLE_LIST_OFF(ParticleListLinks, previous, 0x0);
PARTICLE_LIST_OFF(ParticleListLinks, next, 0x4);
PARTICLE_LIST_OFF(ParticleListStorage, size, 0x0);
PARTICLE_LIST_OFF(ParticleListStorage, sentinelPrevious, 0x4);
PARTICLE_LIST_OFF(ParticleListStorage, first, 0x8);
#undef PARTICLE_LIST_OFF

// Splice all nodes out of a counted ring and release their trivial payload storage.
static inline void ParticleList_UnlinkAndFreeNodes(void* self, void* end, void* node) {
    if (node != end) {
        void* sentinel = ((ParticleListLinks*)end)->previous;
        void* first = ((ParticleListLinks*)node)->previous;
        ((ParticleListLinks*)first)->next = ((ParticleListLinks*)sentinel)->next;
        void* next = ((ParticleListLinks*)node)->previous;
        void* prev = ((ParticleListLinks*)sentinel)->next;
        ((ParticleListLinks*)prev)->previous = next;
        while (node != end) {
            void* current = node;
            node = ((ParticleListLinks*)node)->next;
            appFreeUni__FPv(current);
            ((ParticleListStorage*)self)->size -= 1;
        }
    }
}

// Unlink and free this particle/keyframe list's nodes, then optionally free the list storage.
void* dtor_8010FDDC(void* self, int deleting) {
    if (self != 0) {
        void* end = (char*)&((ParticleListStorage*)self)->sentinelPrevious;
        void* node = ((ParticleListStorage*)self)->first;
        ParticleList_UnlinkAndFreeNodes(self, end, node);
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

// Unlink and free this particle/keyframe list's nodes, then optionally free the list storage.
void* ParticleList_dtor(void* self, int deleting) {
    if (self != 0) {
        if (self != 0) {
            if (self != 0) {
                if (self != 0) {
                    void* end = (char*)&((ParticleListStorage*)self)->sentinelPrevious;
                    void* node = ((ParticleListStorage*)self)->first;
                    ParticleList_UnlinkAndFreeNodes(self, end, node);
                }
            }
        }
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

// Unlink and free this particle/keyframe list's nodes, then optionally free the list storage.
void* dtor_801108EC(void* self, int deleting) {
    if (self != 0) {
        if (self != 0) {
            void* end = (char*)&((ParticleListStorage*)self)->sentinelPrevious;
            void* node = ((ParticleListStorage*)self)->first;
            ParticleList_UnlinkAndFreeNodes(self, end, node);
        }
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

// Unlink and free this particle/keyframe list's nodes, then optionally free the list storage.
void* dtor_8010FE8C(void* self, int deleting) {
    if (self != 0) {
        void* end = (char*)&((ParticleListStorage*)self)->sentinelPrevious;
        void* node = ((ParticleListStorage*)self)->first;
        ParticleList_UnlinkAndFreeNodes(self, end, node);
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

// Unlink and free this particle/keyframe list's nodes, then optionally free the list storage.
void* dtor_8010FF3C(void* self, int deleting) {
    if (self != 0) {
        void* end = (char*)&((ParticleListStorage*)self)->sentinelPrevious;
        void* node = ((ParticleListStorage*)self)->first;
        ParticleList_UnlinkAndFreeNodes(self, end, node);
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

// Unlink and free this particle/keyframe list's nodes, then optionally free the list storage.
void* dtor_801102B4(void* self, int deleting) {
    if (self != 0) {
        void* end = (char*)&((ParticleListStorage*)self)->sentinelPrevious;
        void* node = ((ParticleListStorage*)self)->first;
        ParticleList_UnlinkAndFreeNodes(self, end, node);
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

// Unlink and free this particle/keyframe list's nodes, then optionally free the list storage.
void* dtor_80110418(void* self, int deleting) {
    if (self != 0) {
        void* end = (char*)&((ParticleListStorage*)self)->sentinelPrevious;
        void* node = ((ParticleListStorage*)self)->first;
        ParticleList_UnlinkAndFreeNodes(self, end, node);
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

// Unlink and free this particle/keyframe list's nodes, then optionally free the list storage.
void* dtor_801104C8(void* self, int deleting) {
    if (self != 0) {
        void* end = (char*)&((ParticleListStorage*)self)->sentinelPrevious;
        void* node = ((ParticleListStorage*)self)->first;
        ParticleList_UnlinkAndFreeNodes(self, end, node);
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

// Unlink and free this particle/keyframe list's nodes, then optionally free the list storage.
void* dtor_80110578(void* self, int deleting) {
    if (self != 0) {
        void* end = (char*)&((ParticleListStorage*)self)->sentinelPrevious;
        void* node = ((ParticleListStorage*)self)->first;
        ParticleList_UnlinkAndFreeNodes(self, end, node);
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

// Unlink and free this particle/keyframe list's nodes, then optionally free the list storage.
void* dtor_80110628(void* self, int deleting) {
    if (self != 0) {
        void* end = (char*)&((ParticleListStorage*)self)->sentinelPrevious;
        void* node = ((ParticleListStorage*)self)->first;
        ParticleList_UnlinkAndFreeNodes(self, end, node);
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

// Unlink and free this particle/keyframe list's nodes, then optionally free the list storage.
void* dtor_8011078C(void* self, int deleting) {
    if (self != 0) {
        void* end = (char*)&((ParticleListStorage*)self)->sentinelPrevious;
        void* node = ((ParticleListStorage*)self)->first;
        ParticleList_UnlinkAndFreeNodes(self, end, node);
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

// Unlink and free this particle/keyframe list's nodes, then optionally free the list storage.
void* dtor_8011083C(void* self, int deleting) {
    if (self != 0) {
        void* end = (char*)&((ParticleListStorage*)self)->sentinelPrevious;
        void* node = ((ParticleListStorage*)self)->first;
        ParticleList_UnlinkAndFreeNodes(self, end, node);
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

// Unlink and free this particle/keyframe list's nodes, then optionally free the list storage.
void* dtor_801109A0(void* self, int deleting) {
    if (self != 0) {
        void* end = (char*)&((ParticleListStorage*)self)->sentinelPrevious;
        void* node = ((ParticleListStorage*)self)->first;
        ParticleList_UnlinkAndFreeNodes(self, end, node);
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

// Unlink and free this particle/keyframe list's nodes, then optionally free the list storage.
void* dtor_8010FFEC(void* self, int deleting) {
    if (self != 0) {
        if (self != 0) {
            void* end = (char*)&((ParticleListStorage*)self)->sentinelPrevious;
            void* node = ((ParticleListStorage*)self)->first;
            ParticleList_UnlinkAndFreeNodes(self, end, node);
        }
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

// Unlink and free this particle/keyframe list's nodes, then optionally free the list storage.
void* dtor_801100A0(void* self, int deleting) {
    if (self != 0) {
        void* end = (char*)&((ParticleListStorage*)self)->sentinelPrevious;
        void* node = ((ParticleListStorage*)self)->first;
        ParticleList_UnlinkAndFreeNodes(self, end, node);
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

// Unlink and free this particle/keyframe list's nodes, then optionally free the list storage.
void* dtor_80110150(void* self, int deleting) {
    if (self != 0) {
        if (self != 0) {
            void* end = (char*)&((ParticleListStorage*)self)->sentinelPrevious;
            void* node = ((ParticleListStorage*)self)->first;
            ParticleList_UnlinkAndFreeNodes(self, end, node);
        }
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

// Unlink and free this particle/keyframe list's nodes, then optionally free the list storage.
void* dtor_80110364(void* self, int deleting) {
    if (self != 0) {
        if (self != 0) {
            void* end = (char*)&((ParticleListStorage*)self)->sentinelPrevious;
            void* node = ((ParticleListStorage*)self)->first;
            ParticleList_UnlinkAndFreeNodes(self, end, node);
        }
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

// Unlink and free this particle/keyframe list's nodes, then optionally free the list storage.
void* dtor_801106D8(void* self, int deleting) {
    if (self != 0) {
        if (self != 0) {
            void* end = (char*)&((ParticleListStorage*)self)->sentinelPrevious;
            void* node = ((ParticleListStorage*)self)->first;
            ParticleList_UnlinkAndFreeNodes(self, end, node);
        }
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

// Unlink and free this particle/keyframe list's nodes, then optionally free the list storage.
void* dtor_80110204(void* self, int deleting) {
    if (self != 0) {
        void* end = (char*)&((ParticleListStorage*)self)->sentinelPrevious;
        void* node = ((ParticleListStorage*)self)->first;
        ParticleList_UnlinkAndFreeNodes(self, end, node);
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

// Visit each registered spray and forward the optional lifecycle notification.
void CFxSequence_InvokeSprayLifecycleHooks(SpraySequenceTree* tree, void* arg) {
    SpraySequenceNode* node = tree->root;
    SpraySequenceNode* stop = (SpraySequenceNode*)&tree->sentinel;
    while (node != stop) {
        FxSpray_NoOpLifecycleHook(node->value, arg);
        SpraySequenceNode* child = node->child;
        if (child != 0) {
            while (child->left != 0) child = child->left;
            node = child;
        } else {
            while (node != ((SpraySequenceNode*)((unsigned)node->parent & ~1u))->left) node = (SpraySequenceNode*)((unsigned)node->parent & ~1u);
            node = (SpraySequenceNode*)((unsigned)node->parent & ~1u);
        }
    }
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462110", _ctors_u8010F380_0, CFxParticle_InitializeUnitConstants);
#pragma pop
