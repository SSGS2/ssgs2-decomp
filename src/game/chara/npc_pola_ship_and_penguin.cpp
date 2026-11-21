typedef unsigned long new_size_t;
#include "MSL/error_message.h"
#include "classes/CFxSequence.h"
#include "game/types.h"
#include "game/wvector.h"
#include "game/exception.h"
#include "classes/CCreature.h"

#define AsCCreature(p) ((CCreature*)(p))
// Translation unit 0x801A3B4C-0x801A51D0 (9 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

// Creature clocks: animationFrame/time and dt use the animation caller's time
// units (tick-to-second scale is unresolved here). Ship pathTime advances by 10*dt in spline-distance units; positions and
// path points are world XYZ. Penguin speed is world units per dt, turnRate radians
// per dt. Bounds are opposite interpolation endpoints; no ordering is required.
// Only the literals used in this unit are named; gaps retain the original pool layout.
struct CreatureStringPool {
    char vectorLengthError[0x14];
    char lengthErrorType[0x37];
    char otherNpcNames[0xE7];
    char effectTag[4]; // "*fx"
    char effectTripleFormat[9]; // "%s %s %s"
    char sprayExtension[4];
    char sequenceExtension[4];
    char otherActions[0x53];
    char defaultShipAction[0xA]; // "NPC_Act02"
    char alternateAction[0xA];
    char vectorRangeError[0x1E];
    char rangeErrorType[0x37];
};
static const int kCreatureRangeErrorOffset = (int)&((CreatureStringPool*)0)->vectorRangeError;
static const int kCreatureRangeTypeOffset = (int)&((CreatureStringPool*)0)->rangeErrorType;
enum { kCreatureLoopOnce = 1, kEffectAttachmentBlocked = 0x40000,
       kCreatureMoveSlot = 0x90 / 4, kCreatureSetupSlot = 0x98 / 4,
       kCreatureGroundTestSlot = 0xA0 / 4, kCreatureSecondaryVtableBytes = 0xC };
static const unsigned int kMaxEffectPointers = 0x3FFFFFFFu;
static const unsigned int kEffectSmallGrowthLimit = 0x15555555u;
static const unsigned int kEffectHalfGrowthLimit = 0x2AAAAAAAu;
static const unsigned int kMaxPathPoints = 0x15555555u;
static const unsigned int kPathSmallGrowthLimit = 0x071C71C7u;
static const unsigned int kPathHalfGrowthLimit = 0x0E38E38Eu;
struct AnimationRangeView {
    float startFrame, endFrame;
    char reserved08[0x10];
    void* name;
};
struct CreatureEffectSource { char reserved00[0x74]; unsigned int flags; float transform[12]; };
struct CreatureEffectEvent { char reserved00[0x18]; char attachment[0x10]; void* commandText; };

void* CCreature_Draw(void*);
void CCreature_dtor(void*, int);
void __dl__FPv(void*);
void Spline_EvaluatePosition(void*, void*, float);
void __as__7WVectorFRC7WVector(void*, void*);
int Vec3_NearlyEqualWithTolerance(void*, void*, float);
void Matrix_BuildCameraTransform(void*, void*, void*);
void Matrix_CopyAffine(void*, void*);
void Vec3_Zero(void*);
void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
float WVectorLen__FRC7WVector(void*);
void AnimationEventParser_ProcessFrameRange(void*, void*, float, float, void*, void*, void*);
float Random_RangeFloat(int, float, float);
void* CQuadTree_GetInstance(void);
float Vector_SignedAngleXZ(void*, void*);
void Matrix_ConcatAffine(void*, void*, void*);
void __ml__FfRC7WVector(void*, void*, float);
void __ml__FRC7WVectorf(void*, void*, float);
void __apl__7WVectorFRC7WVector(void*, void*);
int CString_Compare(void*, const char*);
void* WPuppet_FindAnimationRangeByName(void*, const char*);
void* __nwa__FUl(unsigned int);
void __dla__FPv(void*);
void __throw(void*, void*, void*) __attribute__((noreturn));
void* strstr(void*, const char*);
int sscanf(void*, const char*, ...);
char* strchr(char*, int);
int String_CompareCString(const char*, const char*);
void FxAttachment_Init(void*);
void* CFx_CreateAttachedEffect(void*, void*, const char*, int, int, int);
unsigned int strlen(const char*);
char* strcpy(char*, const char*);
void* appMallocDummy__FUl(unsigned int);
void* memcpy(void*, const void*, unsigned int);
void* memset(void*, int, unsigned int);
void appFreeUni__FPv(void*);
void __dt__Q33std6detail16char_array_guardFv(void*);
void __dt__Q23std12out_of_rangeFv();
void dtor_80046988(void*);
void* Normalize__7WVectorFv(void*);
void Vector_CrossProduct(void*, void*, void*);
float Magnitude__7WVectorCFv(void*);
void Spline_Init(void*, void*, void*, int);
void* __construct_new_array(void*, void*, void*, unsigned int, unsigned int);
void* __nw__FUl(unsigned int);
extern const char npcResourceStrings[];
extern const char __vt__Q23std11logic_error[];
extern const char s_sharedCharDeleterVtable[];
extern const char __vt__Q23std12length_error[];
extern const char __vt__Q23std12out_of_range[];
extern void* g_fx;
extern char __vt__9CCreature[];
extern char __vt__18ntNpc_PolaPenguin1[];

typedef Vec3f Vec3_801A;

struct ShipSplineView {
    char reserved00[0x28];
    Vec3_801A rightAxis;
    char reserved34[0x18];
    Vec3_801A position;
    char reserved58[0x68];
    Vec3_801A* pathPoints;
    int pathPointCount;
    char reservedC8[0x60];
    float pathTime;
    char reserved12C[0x10];
    Vec3_801A spline;
};

typedef Vec3f Vec3_801A4574;

// Owned contiguous world-space Vec3 vector: begin/element count/capacity.
// Temporary buffers release storage, never external effects or the caller's path.
struct CreaturePathVector {
    Vec3_801A4574* begin;
    unsigned int size;
    unsigned int cap;
};

struct ShipPathSetupView {
    char reserved00[0x28];
    Vec3_801A4574 rightAxis;
    Vec3_801A4574 upAxis;
    Vec3_801A4574 forwardAxis;
    Vec3_801A4574 position;
    float animationFrame;
    char reserved5C[8];
    Vec3_801A4574 targetPosition;
    char reserved70[0x50];
    CreaturePathVector points;
    char reservedCC[0x70];
    char spline[0x10];
};

struct Vec3Buf_801A4574 {
    Vec3_801A4574* begin;
    unsigned int size;
    unsigned int cap;
    // Initialize an empty temporary world-point vector.
    Vec3Buf_801A4574() : begin(0), size(0), cap(0) {}
    // Release temporary point storage; the element type has no owned subobjects.
    ~Vec3Buf_801A4574() {
        if (begin) {
            size = size - size;
            appFreeUni__FPv(begin);
        }
    }
};
// Keep the original class name for its exception-table destructor relocation.
typedef Vec3Buf_801A4574 CreaturePathBuffer;

// Return caller-provided placement storage without allocating.
extern "C++" { inline void* operator new(new_size_t, void* p) { return p; } }

struct CreaturePathInput {
    Vec3_801A4574* begin;
    unsigned int count;
};

extern const float kFloat_0_1_Creature;
extern const float kFloat_0_00001_CreaturePath;
extern const float kFloat_0_Creature;
extern const float kFloat_48_Creature;
extern const float kFloat_1_Creature;
extern const float kFloat_0_017453292_Creature;
extern const float kFloat_10_CreatureMotion;
extern const float kFloat_0_8_805DF2D8;

// Draw the ship only while animation is active and its one-shot suppression byte is clear.
void* ntNpc_PolaShip_Draw(void* self) {
    if (AsCCreature(self)->animationActive == 0)
        return self;
    if (*(unsigned char*)&AsCCreature(self)->unk_120 == 1)
        return self;
    return CCreature_Draw(self);
}

// Destroy the creature base and optionally free the penguin.
void* ntNpc_PolaPenguin1_Dtor(void* self, int shouldDelete) {
    if (self != 0) {
        CCreature_dtor(self, 0);
        if (shouldDelete > 0)
            __dl__FPv(self);
    }
    return self;
}

struct ErrorAlloc_801A40AC {
    char value; char pad[3];
    // Initialize the empty exception allocator token.
    ErrorAlloc_801A40AC() : value(0) {}
};
struct BufferGuard_801A40AC {
    char* data; void* allocator;
    // Guard the allocated error-message array.
    BufferGuard_801A40AC(char* p, void* a) : data(p), allocator(a) {}
    // Transfer the message buffer to its shared owner.
    char* release() { char* p = data; data = 0; return p; }
    // Release an error-message array still owned by this guard.
    ~BufferGuard_801A40AC() { if (data) __dla__FPv(data); }
};
// Shared RTTI: std::tr1::detail::shared_ptr_deleter<char, std::detail::default_delete<char[]> >.
// Use the common 16-byte ABI view in MSL/error_message.h; keep this local alias for callers.
typedef ::std::detail::__message_control ExceptionCount_801A40AC;
namespace std { struct logic_error : exception {
    // Install the requested standard exception vtable in existing storage.
    void SetTable(const void* vt) { *(const void**)this = vt; }
    char* data;
    ExceptionCount_801A40AC* owner;
    // Own a copied message through the runtime's reference-counted char deleter.
    logic_error(const char* text) {
        SetTable(__vt__Q23std11logic_error);
        ErrorAlloc_801A40AC allocator;
        unsigned len = strlen(text) + 1;
        data = (char*)__nwa__FUl((len << 1) - len);
        {
            char* p = data;
            BufferGuard_801A40AC guard(p, &allocator);
            owner = new ExceptionCount_801A40AC(p);
            guard.release();
        }
        strcpy(data, text);
    }
    ~logic_error();
}; }
namespace std {
struct out_of_range : logic_error {
    // Install the out-of-range exception vtable after copying the message.
    out_of_range(const char* p) : logic_error(p) { SetTable(__vt__Q23std12out_of_range); }
    ~out_of_range();
};
}

namespace std {
struct length_error : logic_error {
    // Install the length-error exception vtable after copying the message.
    length_error(const char* p) : logic_error(p) { SetTable(__vt__Q23std12length_error); }
    ~length_error();
};
}

// Capacity errors below use direct standard throws to keep the caller's cleanup scope.

void __throw(void*, void*, void*) __attribute__((noreturn));
// Owned array of retained sequence handles. Reallocation copies handles only;
// its buffer guard frees the array, not the sequence objects managed by the FX system.
struct CreatureEffectVector {
    void** begin;
    unsigned int size;
    unsigned int cap;
};

struct PtrBuf_801A3B4C {
    void** begin;
    unsigned int size;
    unsigned int cap;
    unsigned int* capRef;
    unsigned int lead;
    // Initialize an empty growth guard for the retained-effect handle array.
    PtrBuf_801A3B4C(unsigned int* c) : begin(0), size(0), cap(0), capRef(c), lead(0) {}
    // Release array storage without destroying the FX-managed sequence handles.
    ~PtrBuf_801A3B4C() {
        if (begin) {
            size = 0;
            appFreeUni__FPv(begin);
        }
    }
};
// Keep the original class name for its exception-table destructor relocation.
typedef PtrBuf_801A3B4C CreatureEffectBuffer;

struct CreatureEffectAttachment {
    char reserved00[8];
    void* sourceTransform;
    char reservedC[0xc];
    void* eventAttachment;
    char reserved1C[0x14];
};

// Return the larger requested vector growth increment by reference.
extern "C++" { inline const unsigned int& max_801A3B4C(const unsigned int& a, const unsigned int& b) { return a < b ? b : a; } }

// arg is a borrowed [source,event] pair; nonnull events must contain a valid *fx
// triple of tag/filename/attachment tokens fitting the fixed arrays. sscanf
// has no width limits and its result is unchecked. Spray handles are not retained;
// sequence handles are appended even if the FX factory returns null.
// Parse an animation effect tag, attach a spray or sequence, and retain sequence handles.
void ntNpc_PolaShip_ParseAttachedEffectCommand(char* self, void** arg) {
    if (!arg) return;
    void* commandText = ((CreatureEffectEvent*)arg[1])->commandText;
    if (!commandText) return;
    char* strings = (char*)npcResourceStrings;
    CreatureEffectAttachment attachment;
    char effectFilename[0x20];
    char attachmentName[0x20];
    char tag[0x40];
    sscanf(strstr(commandText, ((CreatureStringPool*)strings)->effectTag), ((CreatureStringPool*)strings)->effectTripleFormat, tag, effectFilename, attachmentName);
    if (!arg[0] || (((CreatureEffectSource*)arg[0])->flags & kEffectAttachmentBlocked)) return;
    char* extension = strchr(effectFilename, '.');
    if (!extension || strlen(extension) < 4) return;
    if (String_CompareCString(extension + 1, ((CreatureStringPool*)strings)->sprayExtension) && String_CompareCString(extension + 1, ((CreatureStringPool*)strings)->sequenceExtension)) return;
    // Parse/validate is complete. Bind the source transform and event attachment.
    FxAttachment_Init(&attachment);
    attachment.sourceTransform = ((CreatureEffectSource*)arg[0])->transform;
    attachment.eventAttachment = ((CreatureEffectEvent*)arg[1])->attachment;
    void* effect = CFx_CreateAttachedEffect(g_fx, &attachment, effectFilename, 0, 0, 0);
    const char* text = npcResourceStrings;
    if (String_CompareCString(extension + 1, ((const CreatureStringPool*)text)->sequenceExtension)) return;
    // Retain only sequences; append in place or reserve/copy/publish a larger array.
    CreatureEffectVector* effects = (CreatureEffectVector*)&AsCCreature(self)->sequenceEffectHandles;
    unsigned int effectCount = effects->size;
    unsigned int effectCapacity = effects->cap;
    if (effectCount < effectCapacity) {
        effects->size = effectCount + 1;
        effects->begin[effectCount] = effect;
        return;
    }
    // Growth policy: add max(required, 3/5 of capacity+1) for small
    // arrays, max(required, 1/2) for medium arrays, otherwise saturate.
    if (kMaxEffectPointers - effects->cap < 1)
        throw std::length_error(npcResourceStrings);
    CreatureEffectBuffer buf(&effects->cap);
    unsigned int additionalCount = effects->size + 1 - effects->cap;
    unsigned int newCapacity = effects->cap;
    if (additionalCount > kMaxEffectPointers - newCapacity) {
        throw std::length_error(npcResourceStrings);
        newCapacity = kMaxEffectPointers;
    } else if (newCapacity < kEffectSmallGrowthLimit) {
        unsigned int growth = (newCapacity + 1) * 3 / 5;
        newCapacity += max_801A3B4C(growth, additionalCount);
    } else if (newCapacity < kEffectHalfGrowthLimit) {
        unsigned int growth = (newCapacity + 1) / 2;
        newCapacity += max_801A3B4C(growth, additionalCount);
    } else {
        newCapacity = kMaxEffectPointers;
    }
    if (newCapacity > kMaxEffectPointers)
        throw std::length_error(npcResourceStrings);
    buf.begin = (void**)appMallocDummy__FUl(newCapacity * 4);
    buf.lead = effects->size;
    (buf.begin + buf.lead)[buf.size] = effect;
    buf.size++;
    // Copy the oldHandles handle prefix before exchanging array ownership with the guard.
    // Capacity transaction: move handles, zero the oldHandles slots, then exchange
    // storage with the guard. The guard releases only the abandoned allocation.
    void** oldHandles = effects->begin;
    buf.cap = newCapacity;
    int movedHandleCount = (oldHandles + effects->size) - oldHandles;
    buf.lead -= movedHandleCount;
    memcpy(buf.begin + buf.lead, oldHandles, movedHandleCount * 4);
    memset(oldHandles, 0, movedHandleCount * 4);
    buf.size += movedHandleCount;
    unsigned int newSize = buf.size;
    void** newBegin = buf.begin;
    unsigned int oldCapacity = effects->cap;
    unsigned int publishedCapacity = buf.cap;
    effects->size = 0;
    buf.begin = effects->begin;
    effects->cap = publishedCapacity;
    buf.cap = oldCapacity;
    effects->begin = newBegin;
    effects->size = newSize;
    buf.size = 0;
}

// Requires a populated spline and at least one retained path point. The previous
// spline sample is flattened to the new height when building the orientation.
// Evaluate the ship spline, wrap at the last path point, and orient the ship along its path.
void ntNpc_PolaShip_UpdateSplineTransform(ShipSplineView* self) {
    float result[12];
    Vec3_801A previousPosition;
    Vec3_801A pathPosition;
    float previousPathTime = self->pathTime - kFloat_0_1_Creature;
    Spline_EvaluatePosition(&previousPosition, &self->spline, previousPathTime);
    Spline_EvaluatePosition(&pathPosition, &self->spline, self->pathTime);
    *(WVector*)(&self->position) = *(WVector*)&pathPosition;
    if (Vec3_NearlyEqualWithTolerance(&previousPosition, self->pathPoints + (self->pathPointCount - 1), kFloat_0_00001_CreaturePath))
        self->pathTime = kFloat_0_Creature;
    previousPosition.y = self->position.y;
    Matrix_BuildCameraTransform(&result, &previousPosition, &self->position);
    Matrix_CopyAffine(&self->rightAxis, &result);
}

// path and its positions are borrowed only during this call and must contain at
// least two points, with a nonzero first segment for normalization. Copies replace
// the owned vector. Spline_Init consumes/copies temporary positions and cumulative
// distances synchronously; both arrays are freed immediately after it returns.
// Copy path points, set the initial pose, and build an arc-length spline from cumulative distances.
void ntNpc_PolaShip_SetPathAndInitSpline(ShipPathSetupView* self, const CreaturePathInput* path, float initialAnimationFrame) {
    // Phase 1: clear logical contents and reserve enough storage for the new path.
    CreaturePathVector* points = &self->points;
    unsigned int pointCount = path->count;
    points->size = 0;
    if (pointCount > points->cap) {
        CreaturePathBuffer newPoints;
        if (pointCount > kMaxPathPoints)
            throw std::length_error(npcResourceStrings);
        newPoints.begin = (Vec3_801A4574*)appMallocDummy__FUl(pointCount * 12);
        newPoints.cap = pointCount;
        Vec3_801A4574* end = points->begin + points->size;
        Vec3_801A4574* src = points->begin;
        Vec3_801A4574* dst = newPoints.begin + newPoints.size;
        while (src < end) {
            new (dst) Vec3_801A4574(*src);
            src++;
            dst++;
            newPoints.size++;
        }
        points->size = 0;
        Vec3_801A4574* oldBegin = points->begin;
        unsigned int oldCapacity = points->cap;
        unsigned int oldSize = points->size;
        points->begin = newPoints.begin;
        points->cap = newPoints.cap;
        points->size = newPoints.size;
        newPoints.begin = oldBegin;
        newPoints.cap = oldCapacity;
        newPoints.size = oldSize;
    }
    // Phase 2: copy every borrowed point; successful reserve leaves no old elements.
    // This capacity/copy branch is retained even after the reserve above:
    // construction/unwind lifetimes and original statement order are significant.
    unsigned int needed = pointCount;
    if (needed > points->cap) {
        unsigned int capacity = points->cap;
        unsigned int additionalCount = needed - capacity;
        unsigned int newCapacity;
        if (additionalCount > kMaxPathPoints - capacity) {
            throw std::length_error(npcResourceStrings);
            newCapacity = kMaxPathPoints;
        } else if (capacity < kPathSmallGrowthLimit) {
            unsigned int growth = (capacity + 1) * 3 / 5;
            newCapacity = capacity + (growth < additionalCount ? additionalCount : growth);
        } else if (capacity < kPathHalfGrowthLimit) {
            unsigned int growth = (capacity + 1) / 2;
            newCapacity = capacity + (growth < additionalCount ? additionalCount : growth);
        } else {
            newCapacity = kMaxPathPoints;
        }
        points->size = 0;
        if (points->begin) {
            appFreeUni__FPv(points->begin);
            points->begin = 0;
            points->cap = 0;
        }
        if (newCapacity > kMaxPathPoints)
            throw std::length_error(npcResourceStrings);
        Vec3_801A4574* newBegin = (Vec3_801A4574*)appMallocDummy__FUl(newCapacity * 12);
        points->begin = newBegin;
        points->cap = newCapacity;
        Vec3_801A4574* dst = newBegin + points->size;
        Vec3_801A4574* src = path->begin;
        Vec3_801A4574* end = path->begin + needed;
        while (src != end) {
            memcpy(dst, src, 12);
            src++;
            dst++;
            points->size++;
        }
    } else {
        unsigned int overlapCount = points->size < needed ? points->size : needed;
        for (unsigned int i = 0; i < overlapCount; ++i)
            *(WVector*)(&points->begin[i]) = *(WVector*)(&path->begin[i]);
        if (points->size > needed) {
            points->size = needed;
        } else if (points->size < needed) {
            Vec3_801A4574* dst = points->begin + points->size;
            Vec3_801A4574* src = path->begin;
            Vec3_801A4574* end = path->begin + needed;
            while (src != end) {
                memcpy(dst, src, 12);
                src++;
                dst++;
                points->size++;
            }
        }
    }
    // Phase 3: derive the world-space basis from the first segment and world up.
    Vec3_801A4574 firstDirection;
    __mi__FRC7WVectorRC7WVector(&firstDirection, points->begin + 1, points->begin);
    *(WVector*)(&self->forwardAxis) = (*(WVector*)&firstDirection).Normalize();
    Vec3_801A4574 up;
    up.x = kFloat_0_Creature;
    up.y = kFloat_1_Creature;
    up.z = kFloat_0_Creature;
    *(WVector*)(&self->upAxis) = *(WVector*)&up;
    Vec3_801A4574 rightAxis;
    Vector_CrossProduct(&rightAxis, &self->upAxis, &self->forwardAxis);
    *(WVector*)(&self->rightAxis) = *(WVector*)&rightAxis;
    *(WVector*)(&self->position) = *(WVector*)path->begin;
    *(WVector*)(&self->targetPosition) = *(WVector*)(points->begin + 1);
    self->animationFrame = initialAnimationFrame;
    // Phase 4: build arc-length keys, initialize the spline, then release scratch arrays.
    unsigned int count = path->count;
    Vec3_801A4574* splinePoints = (Vec3_801A4574*)__nwa__FUl(count * 12 + 0x10);
    if (splinePoints)
        splinePoints = (Vec3_801A4574*)__construct_new_array(splinePoints, 0, 0, 12, count);
    float* cumulativeDistance = (float*)__nwa__FUl(count * 4);
    *(WVector*)splinePoints = *(WVector*)path->begin;
    cumulativeDistance[0] = kFloat_0_Creature;
    for (unsigned int i = 1; i < count; ++i) {
        *(WVector*)(&splinePoints[i]) = *(WVector*)(&path->begin[i]);
        Vec3_801A4574 d;
        __mi__FRC7WVectorRC7WVector(&d, &path->begin[i], &path->begin[i - 1]);
        cumulativeDistance[i] = cumulativeDistance[i - 1] + (*(WVector*)&d).Magnitude();
    }
    Spline_Init(&self->spline, splinePoints, cumulativeDistance, path->count);
    if (splinePoints)
        __dla__FPv((char*)splinePoints - 0x10);
    if (cumulativeDistance)
        __dla__FPv(cumulativeDistance);
}

struct PenguinAnimationView {
    void** vtable;
    void* vtableEnd;
    int flags;
    char reserved0C[0x0C];
    char vec18[0x0C];
    void* puppet;
    char reserved28[0x30];
    float animationFrame;
    char reserved5C[4];
    void* animationRange;
    char reserved64[0x49];
    unsigned char active;
};

// Aliases the same creature prefix as PenguinAnimationView. +58 is the
// animation clock, +AD gates updates, +12C/+130 retain the sequence vector.
// Sequence byte +80 is set to 0 on ground-test entry and 1 on exit; the
// wider CFxSequence playback semantics are deliberately not inferred here.
enum { kSequencePlaybackByte = 0x80 };
struct ShipAnimationView {
    void** vtable;
    char reserved04[0x20];
    void* animation;
    char reserved28[0x30];
    float animationFrame;
    char reserved5c[4];
    void* animationRange;
    char reserved64[0x49];
    unsigned char animationActive;
    char reservedAE[0x72];
    unsigned char oneShot;
    char reserved121[3];
    float oneShotEndFrame;
    float pathTime;
    void** sequenceHandles;
    unsigned int sequenceCount;
    char reserved134[4];
    unsigned char groundTestLatched;
};

#pragma push
#pragma opt_common_subs off
// While oneShot is active, updates return until its end frame; that also pauses
// path motion and event processing. Afterwards event ranges may wrap once. Ground
// transitions write byte +0x80 of each retained sequence; no effect is freed here.
// Advance ship animation/path time and toggle retained sequence playback on ground-test transitions.
void ntNpc_PolaShip_UpdateAnimationAndMovement(ShipAnimationView* self, float dt) {
    if (!self->animationActive)
        return;
    void* anim = self->animation;
    if (!anim)
        return;
    self->animationFrame += dt;
    if (self->oneShot) {
        if (!(self->animationFrame > self->oneShotEndFrame))
            return;
        self->oneShot = 0;
        self->animationFrame = kFloat_0_Creature;
        const char* action = (const char*)(((const CreatureStringPool*)npcResourceStrings)->defaultShipAction);
        // 0x801A40AC: retain the animation guard before changing its range.
        if (anim != 0) {
            if (self->animationRange == 0 || CString_Compare(((AnimationRangeView*)self->animationRange)->name, action) != 0) {
                void* found = WPuppet_FindAnimationRangeByName(anim, action);
                if (found) {
                    self->animationRange = found;
                    float start = *(float*)found;
                    float end = *((float*)found + 1);
                    self->animationFrame = Random_RangeFloat(-1, start, end);
                } else {
                    self->animationFrame = kFloat_0_Creature;
                }
            }
        }
    }
    {
        float t = self->animationFrame;
        AnimationEventParser_ProcessFrameRange(&AsCCreature(self)->secondaryVtable, self->animation, t - dt, t, self->animationRange, (void*)-1, 0);
    }
    if (self->animationFrame > ((AnimationRangeView*)self->animationRange)->endFrame)
        self->animationFrame -= ((AnimationRangeView*)self->animationRange)->endFrame - *(float*)self->animationRange;
    // Animation events are dispatched; advance spline distance and refresh placement.
    self->pathTime += kFloat_10_CreatureMotion * dt;
    ((void (*)(void*, float))self->vtable[kCreatureMoveSlot])(self, dt);
    // Toggle sequence state only on a ground-test edge, using the retained latch.
    if (!self->groundTestLatched && ((int (*)(void*, void*, float))self->vtable[kCreatureGroundTestSlot])(self, (void*)&AsCCreature(self)->transform[9], kFloat_0_8_805DF2D8)) {
        unsigned int handleByteOffset = 0;
        for (unsigned int i = 0; i < self->sequenceCount; ++i) {
            if (i >= self->sequenceCount)
                do {
                    const char* msg_ = npcResourceStrings + kCreatureRangeErrorOffset;
                    std::out_of_range err_(msg_);
                    __throw((void*)(npcResourceStrings + kCreatureRangeTypeOffset), &err_, (void*)__dt__Q23std12out_of_rangeFv);
                } while (0);
            unsigned char* sequence = *(unsigned char**)((char*)self->sequenceHandles + handleByteOffset);
            if (sequence[kSequencePlaybackByte] != 0)
                sequence[kSequencePlaybackByte] = 0;
            handleByteOffset += 4;
        }
        self->groundTestLatched = 1;
    } else if (self->groundTestLatched && !((int (*)(void*, void*, float))self->vtable[kCreatureGroundTestSlot])(self, (void*)&AsCCreature(self)->transform[9], kFloat_0_8_805DF2D8)) {
        unsigned int handleByteOffset = 0;
        for (unsigned int i = 0; i < self->sequenceCount; ++i) {
            if (i >= self->sequenceCount)
                do {
                    const char* msg_ = npcResourceStrings + kCreatureRangeErrorOffset;
                    std::out_of_range err_(msg_);
                    __throw((void*)(npcResourceStrings + kCreatureRangeTypeOffset), &err_, (void*)__dt__Q23std12out_of_rangeFv);
                } while (0);
            unsigned char* sequence = *(unsigned char**)((char*)self->sequenceHandles + handleByteOffset);
            if (sequence[kSequencePlaybackByte] != 1)
                sequence[kSequencePlaybackByte] = 1;
            handleByteOffset += 4;
        }
        self->groundTestLatched = 0;
    }
}
#pragma pop

struct V90_ { 
virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5(); virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9(); virtual void v10(); virtual void v11(); virtual void v12(); virtual void v13(); virtual void v14(); virtual void v15(); virtual void v16(); virtual void v17(); virtual void v18(); virtual void v19(); virtual void v20(); virtual void v21(); virtual void v22(); virtual void v23(); virtual void v24(); virtual void v25(); virtual void v26(); virtual void v27(); virtual void v28(); virtual void v29(); virtual void v30(); virtual void v31(); virtual void v32(); virtual void v33(); virtual void v34(float);
};
// Advance animation events, stop or wrap at the range end, then update creature movement.
void ntNpc_PolaPenguin1_UpdateAnimationAndMovement(PenguinAnimationView* self, float dt) {
    if (self->puppet == 0 || 0 == self->active)
        return;
    self->animationFrame = (self->animationFrame + dt);
    if (self->animationFrame < kFloat_0_Creature)
        return;
    AnimationEventParser_ProcessFrameRange(&AsCCreature(self)->secondaryVtable, self->puppet, self->animationFrame - dt, self->animationFrame, self->animationRange, (void*)-1, 0);
    float cur = self->animationFrame;
    float endFrame = ((AnimationRangeView*)self->animationRange)->endFrame;
    if (cur > endFrame) {
        if (self->flags & kCreatureLoopOnce) {
            self->active = 0;
            self->animationFrame = *(float*)self->animationRange;
        } else {
            self->animationFrame = cur - (endFrame - *(float*)self->animationRange);
        }
    }
    ((V90_*)self)->v34(dt);
}

// self is caller-provided storage; arg is forwarded to the setup hook. value is
// movement speed in world units per dt. Construction failure destroys the base.
// Initialize the penguin creature, run its setup hook, and set its movement speed and cell size.
void* ntNpc_PolaPenguin1_Ctor(PenguinAnimationView* self, void* arg, float movementSpeed) {
    self->vtable = (void**)__vt__9CCreature;
    self->vtableEnd = (char*)self->vtable + kCreatureSecondaryVtableBytes;
    self->flags = 0;
    Vec3_Zero(AsCCreature(self)->movementBoundsOrigin);
    Vec3_Zero(AsCCreature(self)->movementBoundsMax);
    float zero = kFloat_0_Creature;
    self->puppet = 0;
    self->animationFrame = zero;
    self->animationRange = 0;
    AsCCreature(self)->heading = zero;
    (*(void**)&AsCCreature(self)->history) = 0;
    AsCCreature(self)->ballSpawnOffsetDistance = zero;
    (*(void**)&AsCCreature(self)->opacityKeyframeCount) = 0;
    AsCCreature(self)->opacityKeyframePrevious = (void*)&AsCCreature(self)->opacityKeyframeNext;
    AsCCreature(self)->opacityKeyframeNext = (void*)&AsCCreature(self)->opacityKeyframeNext;
    (*(void**)&AsCCreature(self)->targetPoints) = 0;
    (*(void**)&AsCCreature(self)->targetPointCount) = 0;
    (*(void**)&AsCCreature(self)->targetPointCapacity) = 0;
    (*(void**)&AsCCreature(self)->pathPoints) = 0;
    (*(void**)&AsCCreature(self)->pathPointCount) = 0;
    (*(void**)&AsCCreature(self)->pathPointCapacity) = 0;
    self->vtable = (void**)__vt__18ntNpc_PolaPenguin1;
    self->vtableEnd = (char*)self->vtable + kCreatureSecondaryVtableBytes;
    AsCCreature(self)->movementSpeed = movementSpeed;
    void** vtable = *(void***)(volatile char*)self;
    try {
    ((void (*)(PenguinAnimationView*, void*))vtable[kCreatureSetupSlot])(self, arg);
    AsCCreature(self)->cellSize = kFloat_48_Creature;
    return self;
    } catch (...) {
        CCreature_dtor(self, -1);
        throw;
    }
}

struct PenguinMovementView {
    char reserved00[0xc];
    float spawnMin[3];
    float spawnMax[3];
    char reserved24[4];
    Vec3_801A rightAxis;
    char reserved34[0xc];
    Vec3_801A forwardAxis;
    Vec3_801A position;
    char reserved58[0xc];
    float targetPosition[3];
    float turnRate;
    float movementSpeed;
};

struct Mat_801A4FC0 {
    float m[12];
};

// A by-value WVector argument is a pointer to a copy in CodeWarrior's ABI.
void GetGroundPoint__9CQuadTreeF7WVectorbPib(void*, void*, Vec3_801A, int, int, int);
#define GGP_VEC_ARG(p) (*(Vec3_801A*)(p))
void Matrix_SetAxisRotation(Mat_801A4FC0*, float, int);

// Uses world XYZ; turning is about world Y and ignores errors within one degree.
// When the current position is within one world unit of its target, choose
// any interpolated bounds point as the new target, then project it to terrain.
// The new target need not be near either the current position or the old target.
// Choose a terrain target when close, turn toward it at a bounded rate, and advance along the ground.
void ntNpc_PolaPenguin1_Wander(PenguinMovementView* self, float dt) {
    Mat_801A4FC0 combinedTransform; Mat_801A4FC0 turnTransform;
    Vec3_801A toCurrentPosition; Vec3_801A toTarget; Vec3_801A frameDisplacement; Vec3_801A forwardVelocity; Vec3_801A groundPosition;
    __mi__FRC7WVectorRC7WVector(&toCurrentPosition, &self->position, &self->targetPosition);
    float distanceToTarget = WVectorLen(*(WVector*)&toCurrentPosition);
    float one = kFloat_1_Creature;
    if (distanceToTarget < one) {
        int i = 0;
        char* axisView = (char*)self;
        for (; i < 3; ++i) {
            float rnd = Random_RangeFloat(-1, kFloat_0_Creature, kFloat_1_Creature);

            float y = AsCCreature(axisView)->movementBoundsMax[0];
            float inv = one - rnd;
            AsCCreature(axisView)->targetPosition[0] = rnd * AsCCreature(axisView)->movementBoundsOrigin[0] /* first bounds endpoint, not a spawn operation */ + ((inv * y));
            // Move to the next x/y/z lane of the three parallel vectors.
            axisView = sizeof(float) + axisView;
        }
        Vec3_801A targetOnGround;
        GetGroundPoint__9CQuadTreeF7WVectorbPib(&targetOnGround, CQuadTree_GetInstance(), GGP_VEC_ARG(self->targetPosition), 0, 0, 0);
        *(WVector*)self->targetPosition = *(WVector*)&targetOnGround;
    }
    // Target now lies on terrain; clamp the signed XZ heading correction to turnRate*dt.
    __mi__FRC7WVectorRC7WVector(&toTarget, self->targetPosition, &self->position);
    float angle = Vector_SignedAngleXZ(&toTarget, &self->forwardAxis);

    if ((angle > kFloat_0_Creature ? angle : -angle) > kFloat_0_017453292_Creature) {
        float maxTurn = (self->turnRate * dt);
        float minTurn = -self->turnRate * dt;
        float turn = angle < minTurn ? minTurn : (angle > maxTurn ? maxTurn : angle);
        Matrix_SetAxisRotation(&turnTransform, turn, 1);
        Matrix_ConcatAffine(&combinedTransform, &turnTransform, &self->rightAxis);
        Matrix_CopyAffine(&self->rightAxis, &combinedTransform);
    }
    __ml__FfRC7WVector(&forwardVelocity, &self->forwardAxis, self->movementSpeed);
    __ml__FRC7WVectorf(&frameDisplacement, &forwardVelocity, dt);
    *(WVector*)(&self->position) += *(WVector*)&frameDisplacement;
    GetGroundPoint__9CQuadTreeF7WVectorbPib(&groundPosition, CQuadTree_GetInstance(), GGP_VEC_ARG(&self->position), 1, 0, 0);
    *(WVector*)(&self->position) = *(WVector*)&groundPosition;
}

} // extern "C"
