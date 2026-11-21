#include <decomp/offset_of.h>
#include <decomp/ct_member.h>
typedef unsigned long new_size_t;
#include "classes/WPuppet.h"
#include "classes/CCreature.h"
// View the same creature allocation through its movement/path layout.
inline CCreature* AsCCreature(void* object) { return (CCreature*)object; }
// The existing layout leaves +0x80 as padding; movement uses it as a float.
// Read/write the shared +0x80 parameter: heading-error threshold in one test,
// bank recovery rate when multiplied by dt. Its intended physical unit is unresolved.
inline float& CreatureHeadingBankParameter(void* object) { return *(float*)AsCCreature(object)->pad_080; }
namespace exception_runtime {
extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12length_errorFv(void*, int);
extern const char npcResourceStrings[];
}
}

#include "game/wvector.h"
#include "classes/CDisposable.h"

// View the same allocation's animation/render fields; this does not adjust the pointer.
inline CDisposable* AsCDisposable(void* object) { return (CDisposable*)object; }
// Translation unit 0x80199DCC-0x8019B4B0 (4 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

// Path-point vector limits. A point is 3 floats (12 bytes), so the largest element count is 0xFFFFFFFF/12 = 0x15555555.
// The literal growth thresholds are 0x071c71c7 and 0x0e38e38e: below tier 1 the capacity grows by 3/5, below tier 2 by 1/2, else it jumps
// to the maximum (MSL vector growth policy).
enum CreatureVectorLimits { kMaxPathPoints = 0x15555555U, kPathGrowthTier1 = 0x071c71c7U, kPathGrowthTier2 = 0x0e38e38eU };
// Byte-addition views are intentional here: direct member addresses changed instruction scheduling.
enum CreatureFieldOffsets {
    kCreatureForward = (int)DECOMP_OFFSETOF(CCreature, transform[6]),
    kCreaturePosition = (int)DECOMP_OFFSETOF(CCreature, transform[9]),
    kCreatureTarget = (int)DECOMP_OFFSETOF(CCreature, targetPosition),
    kCreatureTransform = (int)DECOMP_OFFSETOF(CCreature, transform),
    kCreatureHistoryCount = (int)DECOMP_OFFSETOF(CCreature, historyCount),
    kCreatureHistory = (int)DECOMP_OFFSETOF(CCreature, history)
};
enum CreatureAnimationFlags { kOpacityKeyframed = 1 };
enum CreatureVirtualSlots { kUpdateUnkeyedMovementSlot = 0x90 / 4,
                           kResetKeyedAnimationSlot = 0x94 / 4 };
enum CreatureMovement { kGridAxisCount = 3, kTargetSearchAttempts = 5, kLengthErrorTypeName = 0x14 };
struct AnimationFrameRange { float first; float last; };

struct CreaturePointWords { unsigned int a, b, c; };
// Point words preserve three float bit patterns. Both vector specializations own
// their data; range is a borrowed source array, with size measured in xyz points.
struct CreaturePointVector { CreaturePointWords* begin; unsigned int size, capacity; };
struct CreaturePointRange { void* data; unsigned size; };
struct OpacityKeyframe { OpacityKeyframe* prev; OpacityKeyframe* next; float value; float time; };
void* appMallocDummy__FUl(unsigned int);
void appFreeUni__FPv(void*);
void __as__7WVectorFRC7WVector(void*, const void*);
void __mi__FRC7WVectorRC7WVector(void*, const void*, const void*);
void* Normalize__7WVectorFv(void*);
void Vector_CrossProduct(void*, const void*, const void*);
void* __nwa__FUl(unsigned int);
void Timer_ReadQuantizedTickCount(void*);
unsigned int srand(unsigned int);
unsigned int rand();
float WVectorLen__FRC7WVector(const void*);
float Random_RangeFloat(int, float, float);
void __ml__FRC7WVectorf(void*, const void*, float);
float Vector_SignedAngleXZ(const void*, const void*);
void __apl__7WVectorFRC7WVector(void*, const void*);
void Matrix_CopyAffine(void*, const void*);
extern const char __vt__Q23std12length_error[];
extern const char npcResourceStrings[];
extern const char __vt__Q23std11logic_error[];
extern const char s_sharedCharDeleterVtable[];
extern float kFloat_0_Creature, kDouble_SignedIntConversionBias_Creature, kFloat_1_Creature, kFloat_0_017453292_Creature, kFloat_neg1_CreatureDirection;
extern const char npcResourceStrings[];
extern const char __vt__Q23std11logic_error[];
extern const char s_sharedCharDeleterVtable[];

extern unsigned long strlen(const char*);
extern char* strcpy(char*, const char*);
extern void* __nw__FUl(unsigned long);
extern void __throw(void*, void*, void*);
extern void __dla__FPv(void*);
extern "C" void __dt__Q23std9exceptionFv(void*, int);
// Exception-message array guard; keep this type name for unwind relocation identity.
struct VecAllocation80199DCC {
    char* pointer; char* allocator;
    // Guard an allocated exception-message array until its control block takes ownership.
    VecAllocation80199DCC(char* p, char* a) : pointer(p), allocator(a) {}
    // Free the message array if ownership has not been transferred.
    ~VecAllocation80199DCC() { if (pointer) __dla__FPv(pointer); }
};
struct ExceptionTextControlBase {
    void* table; unsigned strong, weak;
    // Initialize strong/weak counts to one and install the shared-text deleter.
    ExceptionTextControlBase() { strong = 1; weak = 1; table = (void*)s_sharedCharDeleterVtable; }
};
struct ExceptionTextControl : ExceptionTextControlBase {
    char* pointer;
    // Retain the allocated exception-message pointer in its control block.
    ExceptionTextControl(char* p) { pointer = p; }
};
// Logic-error base cleanup; its destructor is referenced by the exception tables.
struct VecErrorBase80199DCC {
    void* table;
    // Install the logic-error base interface before building the derived exception.
    VecErrorBase80199DCC() { table = (void*)__vt__Q23std11logic_error; }
    // Destroy the exception base without deleting this embedded storage.
    ~VecErrorBase80199DCC() { __dt__Q23std9exceptionFv(this, -1); }
};
namespace std {
struct length_error : VecErrorBase80199DCC {
    char* text; void* control;
    // Copy the length-error text into shared owned storage and install its interface.
    length_error() {
        const char* message = npcResourceStrings;
        struct Allocator { char value; char pad[3]; } allocator;
        allocator.value = 0;
        unsigned n = strlen(message) + 1;
        text = (char*)__nwa__FUl(2 * n - n);
        {
            char* p = text;
            VecAllocation80199DCC allocation(p, &allocator.value);
            control = new ExceptionTextControl(p);
            allocation.pointer = 0;
        }
        strcpy(text, message);
        table = (void*)__vt__Q23std12length_error;
    }
    DECOMP_CT_MEMBER(length_error)
};
}
} // extern C
// Return caller-provided placement storage without allocating.
inline void* operator new(new_size_t, void* p) { return p; }
extern "C" {
// One path point: three float bit patterns (x, y, z) moved as raw words.
struct PathPointWords { unsigned xBits, yBits, zBits; };
enum CreaturePathStorage { kPathPointBytes = sizeof(PathPointWords),
                          kMovementScratchWords = 12, kVectorComponents = 3 };
// Temporary path-point buffer that frees its storage during normal or exceptional exit.
struct TempVec80199DCC {
    PathPointWords* data; unsigned size, capacity;
    // Start an empty temporary point vector with no owned storage.
    TempVec80199DCC() : data(0), size(0), capacity(0) {}
    // Release the temporary point-vector allocation, if any.
    ~TempVec80199DCC() { if (data) { size = 0; appFreeUni__FPv(data); } }
};
struct PathPointVector { PathPointWords* data; unsigned size, capacity; };
// Return a borrowed reference to the larger capacity requirement.
inline const unsigned& LargerCapacity(const unsigned& a, const unsigned& b) { return a < b ? b : a; }
struct PathPointAllocator {
    volatile char flag;
    // Initialize the recovered allocator tag without allocating point storage.
    PathPointAllocator() : flag(0) {}
};
// Temporary vector used to exchange the reserved point allocation safely.
struct ReserveTmp80199DCC {
    PathPointWords* data; unsigned size, capacity;
    // Release the displaced point allocation after a reserve swap or unwind.
    ~ReserveTmp80199DCC() { if (data) { PathPointAllocator allocator; unsigned n = size; size -= n; appFreeUni__FPv(data); } }
};
// Copy one point into reserved storage, retaining the original construction/unwind boundary.
inline void ConstructPathPoint(PathPointWords* destination, const PathPointWords& source) {
    try { new (destination) PathPointWords(source); } catch (...) { throw; }
}

// Set the local-to-world basis, first target and animation time after the owned copy is complete.
// up/cross borrow caller scratch to preserve its lifetime; range still supplies the origin.
inline void InitializeCreaturePathTransform(void* object, PathPointVector* self,
        CreaturePointRange* range, float value, float* up, WVector& cross) {
    WVector dir = *(WVector*)(self->data + 1) - *(WVector*)self->data;
    *(WVector*)&AsCCreature(object)->transform[6] = dir.Normalize(); // 0x80199DCC unit: +0x40, forward basis
    up[0] = kFloat_0_Creature;
    up[1] = kFloat_1_Creature;
    up[2] = kFloat_0_Creature;
    *(WVector*)&AsCCreature(object)->transform[3] = *(WVector*)up; // +0x34, up basis
    Vector_CrossProduct(&cross, &AsCCreature(object)->transform[3], &AsCCreature(object)->transform[6]);
    *(WVector*)&AsCCreature(object)->transform[0] = cross; // +0x28, right basis
    *(WVector*)&AsCCreature(object)->transform[9] = *(WVector*)range->data; // +0x4C, translation
    *(WVector*)AsCCreature(object)->targetPosition = *(WVector*)(self->data + 1); // +0x64
    AsCDisposable(object)->animationTime = value;
}

#pragma opt_common_subs off
#pragma push
#pragma always_inline on
// Select MSL's grown point capacity after the caller checks the additional-count limit.
// Caller-owned temporaries keep the original register lifetimes when this is inlined.
static inline void RecommendCreaturePointCapacity(unsigned& capacity, unsigned& additional,
        unsigned& newCapacity, unsigned& smallGrowth, const unsigned*& smallAmount,
        unsigned& largeGrowth, const unsigned*& largeAmount) {
    if (capacity < kPathGrowthTier1) {
        smallGrowth = ((capacity + 1) * 3) / 5;
        smallAmount = &smallGrowth;
        if (smallGrowth < additional) smallAmount = &additional;
        newCapacity = capacity + *smallAmount;
    } else if (capacity < kPathGrowthTier2) {
        largeGrowth = (capacity + 1) / 2;
        largeAmount = &largeGrowth;
        if (largeGrowth < additional) largeAmount = &additional;
        newCapacity = capacity + *largeAmount;
    } else {
        newCapacity = kMaxPathPoints;
    }
}
#pragma pop

// Copy the supplied path points, aim the transform from point zero toward point one, and set animation time.
// range->data must contain at least TWO xyz points, with distinct first/second
// positions for normalization. No check is added here. The range stays borrowed;
// targetPoints owns its copy. value is animation time in the caller's frame units.
void CCreature_SetPathPointsAndOrient(void* object, CreaturePointRange* range, float value) {
    PathPointVector* targetPoints = (PathPointVector*)&AsCCreature(object)->targetPoints;

    float up[3];
    WVector cross;
    PathPointAllocator allocator0;
    // In plain C++ this whole function is:  targetPoints.assign(range->data, range->data + range->size);
    // then set up the transform. The vector code below is MSL's inlined reserve() + assign(); each
    // `union {...} storage; error->__ct(); __throw(...)` block is an inlined `throw std::length_error("vector length error")`
    // for a count above kMaxPathPoints; PathPointAllocator locals are empty allocator tags kept to match the compiler.
    // Phase 1: clear logical contents and reserve storage, swapping the old
    // allocation into the guard so normal/unwind cleanup releases it.
    unsigned requestedPointCount = range->size;
    targetPoints->size -= targetPoints->size;
    if (requestedPointCount > targetPoints->capacity) {
        ReserveTmp80199DCC temporary = { 0, 0, 0 };
        PathPointAllocator allocator;
        if (requestedPointCount > kMaxPathPoints) {
            union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
            std::length_error* error = (std::length_error*)storage.bytes;
            error->__ct();
            exception_runtime::__throw(exception_runtime::npcResourceStrings + kLengthErrorTypeName, error,
                (void*)exception_runtime::__dt__Q23std12length_errorFv);
        }
        temporary.data = (PathPointWords*)appMallocDummy__FUl(requestedPointCount * kPathPointBytes);
        PathPointWords* sourceBegin = targetPoints->data;
        PathPointWords* sourceEnd = sourceBegin + targetPoints->size;
        temporary.capacity = requestedPointCount;
        PathPointAllocator allocator1;
        PathPointWords* destination = temporary.data + temporary.size;
        while (sourceBegin < sourceEnd) {
            ConstructPathPoint(destination, *sourceBegin);
            ++sourceBegin;
            ++destination;
            ++temporary.size;
        }
        PathPointWords* oldData = targetPoints->data;
        targetPoints->size = 0;
        unsigned oldCapacity = targetPoints->capacity;
        unsigned oldSize = targetPoints->size;
        PathPointAllocator allocator2;
        targetPoints->capacity = temporary.capacity;
        temporary.capacity = oldCapacity;
        targetPoints->data = temporary.data;
        temporary.data = oldData;
        targetPoints->size = temporary.size;
        temporary.size = oldSize;
    }
    // Reserve leaves size zero and capacity >= range size. The assignment below
    // retains the original generic growth paths even though this reserve covers requestedPointCount.
    // Phase 2: assign the entire borrowed range. Growth/copy branches below are
    // expanded vector machinery; after them size equals range->size.
    PathPointWords* sourceBegin = (PathPointWords*)range->data;
    PathPointWords* sourceEnd = sourceBegin + range->size;
    unsigned sourcePointCount = sourceEnd - sourceBegin;
    PathPointAllocator allocator3;
    unsigned assignedCapacity;
    // 0x80199DCC: if the range fits the capacity, overwrite the common prefix and then erase or append the rest;
    // otherwise compute the grown capacity, free the old block and copy-construct every point into a fresh one.
    do {
        if (sourcePointCount > targetPoints->capacity) {
            PathPointAllocator allocator4;
            unsigned additional = sourcePointCount - targetPoints->capacity;
            unsigned capacity = targetPoints->capacity;
            if (additional > kMaxPathPoints - capacity) {
                union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
            std::length_error* error = (std::length_error*)storage.bytes;
                error->__ct();
            exception_runtime::__throw(exception_runtime::npcResourceStrings + kLengthErrorTypeName, error,
                    (void*)exception_runtime::__dt__Q23std12length_errorFv);
            }
            // New capacity = capacity + max(growth step, additional) (growth step: 3/5 or 1/2 of the capacity).
            unsigned newCapacity;
            unsigned smallGrowth; const unsigned* smallAmount;
            unsigned largeGrowth; const unsigned* largeAmount;
            RecommendCreaturePointCapacity(capacity, additional, newCapacity,
                smallGrowth, smallAmount, largeGrowth, largeAmount);
            assignedCapacity = newCapacity;
        } else {
            const unsigned* commonPointCount;
            if (targetPoints->size < sourcePointCount) commonPointCount = &targetPoints->size;
            else commonPointCount = &sourcePointCount;
            PathPointWords* commonEnd = sourceBegin + *commonPointCount;
            PathPointWords* input = sourceBegin;
            PathPointWords* output = targetPoints->data;
            while (input < commonEnd) {
                *(WVector*)output = *(WVector*)input;
                ++input;
                ++output;
            }
            if (targetPoints->size > sourcePointCount) {
                PathPointAllocator allocator5;
                targetPoints->size -= targetPoints->size - sourcePointCount;
            } else if (targetPoints->size < sourcePointCount) {
                PathPointAllocator allocator6;
                PathPointWords* destination = targetPoints->data + targetPoints->size;
                PathPointWords* input = commonEnd;
                PathPointWords* inputEnd = sourceEnd;
                while (input != inputEnd) {
                    ConstructPathPoint(destination, *input);
                    ++destination;
                    ++input;
                    ++targetPoints->size;
                }
            }
            break;
        }

        {
            PathPointAllocator allocator7;
            targetPoints->size -= targetPoints->size;
            if (targetPoints->data) {
                appFreeUni__FPv(targetPoints->data);
                targetPoints->data = 0;
                targetPoints->capacity = 0;
            }
            PathPointAllocator allocator8;
            if (assignedCapacity > kMaxPathPoints) {
                union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
            std::length_error* error = (std::length_error*)storage.bytes;
                error->__ct();
            exception_runtime::__throw(exception_runtime::npcResourceStrings + kLengthErrorTypeName, error,
                    (void*)exception_runtime::__dt__Q23std12length_errorFv);
            }
            targetPoints->data = (PathPointWords*)appMallocDummy__FUl(assignedCapacity * kPathPointBytes);
            targetPoints->capacity = assignedCapacity;
            PathPointAllocator allocator9;
            PathPointWords* destination = targetPoints->data + targetPoints->size;
            PathPointWords* input = sourceBegin;
            PathPointWords* inputEnd = sourceEnd;
            while (input != inputEnd) {
                ConstructPathPoint(destination, *input);
                ++destination;
                ++input;
                ++targetPoints->size;
            }
        }
    } while (0);
    // Phase 3: build the creature transform from the first two world positions:
    // forward=normalize(P1-P0), up=(0,1,0), right=up cross forward, origin=P0.
    // The cross product is deliberately not normalized again.
    InitializeCreaturePathTransform(object, targetPoints, range, value, up, cross);
}
#pragma opt_common_subs reset

extern float kFloat_0_Creature;
extern float kFloat_1_Creature;
extern float kFloat_5_creature;
extern float kFloat_0_8_805DF2D8;
extern int g_drawSceneDebugGeometry;
extern void* g_view;
extern const char s_sceneLighting[];
// Real signature (self, manager r4, bounds r5, f1, f2, r6, r7); the body is 0x8019A538..0x8019A54C.
void AnimationEventParser_ProcessFrameRange(void*, void*, float, float, void*, int, int); /* (self, manager r4, bounds r5, f1, f2, r6, r7): 0x8019A538-0x8019A54C */
void WPuppet_SetOpacityRecursive(void*, unsigned char);
void WView_DrawFacingCircleVec4(void*, void*, unsigned int, int, int);
void __ml__FfRC7WVector(void*, const void*, float);
void __mi__FRC7WVectorRC7WVector(void*, const void*, const void*);
void WView_DrawLine(void*, void*, int, void*, int, int);
int WPuppet_SetAnimationTimeFromRanges(void*, int, float, int, int);
void Matrix_SetAxisRotation(void*, float, int);
void Matrix_ConcatAffine(void*, const void*, const void*);
void Model_SetMatrixRecursive(void*, void*, const void*);
void WPuppet_UpdateBoneWorldMatrices(void*, void*, int, float);
void WPuppet_UpdateBoundsFromBone(void*);
struct CreatureRenderDispatch {
    virtual void p00(); virtual void p01(); virtual void p02(); virtual void p03(); virtual void p04(); virtual void p05(); virtual void p06(); virtual void p07();
    virtual void p08(); virtual void p09(); virtual void p10(); virtual void p11(); virtual void p12(); virtual void p13(); virtual void p14(); virtual void p15();
    virtual void p16(); virtual void p17(); virtual void p18(); virtual void p19(); virtual void p20(); virtual void p21(); virtual void p22(); virtual void p23();
    virtual void p24(); virtual void p25(); virtual void p26(); virtual void p27(); virtual void p28(); virtual void p29(); virtual void p30(); virtual void p31();
    virtual void p32(); virtual void p33(); virtual void p34(); virtual void p35(); virtual void p36(); virtual void p37();
    virtual int vfA0(const void*, float);
};
struct PuppetRenderDispatch {
    virtual void p00(); virtual void p01(); virtual void p02();
    virtual void vf14(const char*, int, int);
    virtual void vf18(void*, int, int, int, float, int);
};

// Query the position-based draw-skip predicate; nonzero suppresses bone updates and drawing.
static inline int CreatureDrawIsSuppressed(void* creature, const void* worldPosition) {
    return ((CreatureRenderDispatch*)creature)->vfA0(worldPosition, kFloat_0_8_805DF2D8);
}
// Apply the scene-lighting selection through the puppet's +0x14 virtual slot.
static inline void ApplyCreaturePuppetLighting(void* puppet) {
    ((PuppetRenderDispatch*)puppet)->vf14(s_sceneLighting, 0, 0);
}
// Submit the puppet through its +0x18 draw slot using the original pass flags.
static inline void SubmitCreaturePuppetDraw(void* puppet) {
    ((PuppetRenderDispatch*)puppet)->vf18(g_view, 1, 1, 1, kFloat_1_Creature, 0);
}
// Reset keyed animation at a range wrap; this slot receives an integer zero.
static inline void ResetCreatureKeyedAnimation(void* creature) {
    ((void (**)(void*, int))(*(void***)creature))[kResetKeyedAnimationSlot](creature, 0);
}
// Advance unkeyed movement in the same time units as the animation delta.
static inline void AdvanceCreatureUnkeyedMovement(void* creature, float delta) {
    ((void (**)(void*, float))(*(void***)creature))[kUpdateUnkeyedMovementSlot](creature, delta);
}

// 0x8019A4E4: CDisposable animation time/flags share the ctor and OnProcess layout.
// Advance animation, dispatch crossed frame events, and interpolate opacity keys or update movement.
// CDisposable and CCreature are overlapping recovered views of this same address,
// not a pair of owned objects or a pointer-adjusting base conversion.
// delta uses animation-time units. vtable byte +0x94 receives int 0 at an
// opacity animation wrap; +0x90 receives delta for movement when opacity is unkeyed.
// Those slot contracts are known here; broader virtual method names are unresolved.
void CCreature_UpdateAnimationTimeAndOpacity(void* self, float delta) {
    // Phase 1: advance time, dispatch crossed-frame events, and wrap the animation range.
    void* puppet = AsCDisposable(self)->puppet;
    if (!puppet || !AsCDisposable(self)->animationEnabled) return;
    float zero = kFloat_0_Creature;
    float time = ((CDisposable*)self)->animationTime;
    time = (((((time + delta)))));
    ((CDisposable*)self)->animationTime = time;
    if (time < zero) return;
    AnimationEventParser_ProcessFrameRange(&AsCDisposable(self)->secondaryVtable, puppet, time - delta, time, (*(void**)&((CDisposable*)self)->animationRange), -1, 0);
    /* original: r4 = self+0x24 (still loaded), f2 = new time, ints in r6/r7 */
    float end = ((AnimationFrameRange*)AsCDisposable(self)->animationRange)->last;
    if (end < ((CDisposable*)self)->animationTime) {
        if (((CDisposable*)self)->flags & kOpacityKeyframed) {
            ResetCreatureKeyedAnimation(self);
            ((CDisposable*)self)->animationTime = ((AnimationFrameRange*)AsCDisposable(self)->animationRange)->first;
        } else {
            ((CDisposable*)self)->animationTime -= end;
        }
    }
    // Phase 2: opacity keys form a sentinel list in animation-time order. The
    // last key holds its value; an empty list leaves opacity unchanged.
    if (((CDisposable*)self)->flags & kOpacityKeyframed) {
        OpacityKeyframe* node = (OpacityKeyframe*)AsCDisposable(self)->opacityKeyframePrevious;
        OpacityKeyframe* head = (OpacityKeyframe*)&AsCDisposable(self)->opacityKeyframeNext;
        /* original 0x8019A5B4/0x8019A634: test node != head before the first pass (while, not do-while); an empty list writes nothing */
        while (head != node) {
            OpacityKeyframe* previous = node;
            node = node->next;
            if (head == node) {
                AsCDisposable(self)->opacity = (unsigned char)(int)previous->value;
                break;
            }
            float start = previous->time;
            float finish = node->time;

            float current = ((CDisposable*)self)->animationTime;
            if (current > start) { if (current < finish) {

                float t = (current - start) / (finish - start);

                AsCDisposable(self)->opacity = (unsigned char)(int)((1.0f-t)*previous->value + t*node->value);
                return;
            } }
        }
    } else {
        AdvanceCreatureUnkeyedMovement(self, delta);
    }
}

// Apply animation, opacity, bank rotation and lighting, then draw the puppet and optional debug geometry.
// Puppet slots +0x14/+0x18 select lighting and submit the draw respectively.
// rotationZ is bank in radians; the affine transform maps local to world space.
void CCreature_Draw(void* self) {
    char* s = (char*)self;
    if (!AsCDisposable(s)->puppet || !AsCDisposable(s)->animationEnabled || AsCDisposable(s)->animationTime < kFloat_0_Creature) return;
    if (AsCDisposable(s)->flags & kOpacityKeyframed) WPuppet_SetOpacityRecursive(AsCDisposable(s)->puppet, AsCDisposable(s)->opacity);
    if (g_drawSceneDebugGeometry) {
        WView_DrawFacingCircleVec4(g_view, &((WPuppet*)AsCDisposable(s)->puppet)->boundsCenter, 0xff00ff00, 0x14, 0);
        if (!(AsCDisposable(s)->flags & kOpacityKeyframed)) {
            float a[3], b[3];
            __ml__FfRC7WVector(a, s + kCreatureForward, kFloat_5_creature);
            __mi__FRC7WVectorRC7WVector(b, s + kCreaturePosition, a);
            WView_DrawLine(g_view, s + kCreaturePosition, -256, b, -256, 0);
            WView_DrawLine(g_view, s + kCreaturePosition, 0xffff0000, s + kCreatureTarget, 0xffff0000, 0);
        }
    }
    WPuppet_SetAnimationTimeFromRanges(AsCDisposable(s)->puppet, 0, AsCDisposable(s)->animationTime, 0, 0);
    if (CreatureDrawIsSuppressed(self, s + kCreaturePosition)) return;
    if (AsCDisposable(s)->matrixXX > kFloat_1_Creature) AsCDisposable(s)->matrixXX = kFloat_1_Creature;
    if (AsCDisposable(s)->matrixYY > kFloat_1_Creature) AsCDisposable(s)->matrixYY = kFloat_1_Creature;
    if (AsCDisposable(s)->matrixZZ > kFloat_1_Creature) AsCDisposable(s)->matrixZZ = kFloat_1_Creature;
    // Compose local bank with the local-to-world transform before updating puppet bones.
    float absoluteBankAngle = AsCDisposable(s)->rotationZ > kFloat_0_Creature ? AsCDisposable(s)->rotationZ : -AsCDisposable(s)->rotationZ;
    float matrix[12], composed[12];
    void* m;
    if (absoluteBankAngle > kFloat_0_Creature) {
        Matrix_SetAxisRotation(matrix, -AsCDisposable(s)->rotationZ, 2);
        Matrix_ConcatAffine(composed, matrix, s + kCreatureTransform);
        m = composed;
    } else {
        m = s + kCreatureTransform;
    }
    Model_SetMatrixRecursive(AsCDisposable(s)->puppet, AsCDisposable(s)->puppet, m);
    WPuppet_UpdateBoneWorldMatrices(AsCDisposable(s)->puppet, g_view, 0, kFloat_1_Creature);
    WPuppet_UpdateBoundsFromBone(AsCDisposable(s)->puppet);
    ApplyCreaturePuppetLighting(AsCDisposable(s)->puppet);
    SubmitCreaturePuppetDraw(AsCDisposable(s)->puppet);
}

// Choose or replay a grid target, record targets when enabled, then steer and move toward it.
// dt scales speed and turn/bank rates; its absolute time unit is not established.
// Positions/grid steps are world-space distances; heading/bank/turn steps are radians.
// Requires positive grid dimensions and an initialized nonempty history buffer.
void CCreature_UpdateWanderMovement(void* self, float dt) {
    char* s = (char*)self;
    int cellDelta[kGridAxisCount];
    // Three float components, used as position-target or displacement in world space.
    float directionScratch[kVectorComponents];
    // Raw 32-bit words: first a big-endian 64-bit timer (high,low), then float
    // vector bit patterns. +3 addresses the next xyz vector, never a pointer slot.
    unsigned int vectorScratch[kMovementScratchWords];
    Timer_ReadQuantizedTickCount(vectorScratch);
    // Seed from the low word of the quantized hardware tick count.
    srand((unsigned int)vectorScratch[1]);
    __mi__FRC7WVectorRC7WVector(vectorScratch, s + kCreaturePosition, s + kCreatureTarget);
    float distanceToTarget = WVectorLen(*(WVector*)vectorScratch);
    // Phase 1: only choose/replay a target inside the one-world-unit arrival radius.
    if (distanceToTarget < 1.0f) {
        // 0x80199DCC unit: generate a point only if no recorded point is available.
        if (!(((CCreature*)s)->pathCursor < (unsigned int)AsCCreature(s)->pathPointCount && !((CCreature*)s)->recordPath))
        {
            int targetCellX, targetCellZ, targetCellY, nextCell;
            // 1a: choose a clamped neighbor and reject recent flattened cell IDs.
            int targetAttempts=0;
            do {
                ++targetAttempts;
                // Independent steps in {-1,0,+1}; zero on all axes is allowed.
                for (int axis = 0; axis < kGridAxisCount; axis++)
                    cellDelta[axis] = ((int)rand() % 3) - 1;
                // Cell order is X within Z rows within Y layers.
                int xCellCount=((CCreature*)s)->gridXCount, zCellCount=((CCreature*)s)->gridZCount;
                int previousCell=**(int**)(s + kCreatureHistory);
                int cellsPerLayer=xCellCount*zCellCount;
                int currentLayer=previousCell/cellsPerLayer;
                int currentRow=(previousCell%cellsPerLayer)/xCellCount;
                targetCellX=previousCell%xCellCount+cellDelta[0];
                if (targetCellX < 0) targetCellX = 0;
                else if (targetCellX > xCellCount - 1)
                    targetCellX = xCellCount - 1;
                targetCellY=currentLayer+cellDelta[1];
                int yCellCount=((CCreature*)s)->gridYCount;
                if (targetCellY < 0) targetCellY = 0;
                else if (targetCellY > yCellCount - 1)
                    targetCellY = yCellCount - 1;
                targetCellZ=currentRow+cellDelta[2];
                if (targetCellZ < 0) targetCellZ = 0;
                else if (targetCellZ > zCellCount - 1)
                    targetCellZ = zCellCount - 1;
                nextCell=targetCellX+xCellCount*(targetCellZ+targetCellY*zCellCount);
                bool alreadyInHistory = false;
                for (int i = 0; i < *(int*)(s + kCreatureHistoryCount); i++)
                    if ((*(int**)(s + kCreatureHistory))[i] == nextCell) {
                    alreadyInHistory = true;
                    break;
                }
                if (!alreadyInHistory) break;
            }
            while (targetAttempts<kTargetSearchAttempts);
            // Flattened cell = x + X*(z + y*Z). Avoid recent cells for up to five
            // attempts; the final candidate is accepted even if still in history.
            // 1b: shift recent-cell history, then jitter in the accepted cell.
            // A one-layer grid keeps its existing target Y coordinate.
            for (int i = *(int*)(s + kCreatureHistoryCount) - 1; i >= 1; --i)
                (*(int**)(s + kCreatureHistory))[i] = (*(int**)(s + kCreatureHistory))[i - 1];
            *(int*)AsCCreature(s)->history=nextCell;
            float jitterX=Random_RangeFloat(-1,0.0f,((CCreature*)s)->gridStepX);
            ((CCreature*)s)->targetPosition[0] =
                (((CCreature*)s)->gridStepX * (float)targetCellX + ((CCreature*)s)->movementBoundsOrigin[0]) + jitterX;
            if (((CCreature*)s)->gridYCount > 1)
                ((CCreature*)s)->targetPosition[1] = ((CCreature*)s)->movementBoundsOrigin[1] +
                    ((CCreature*)s)->gridStepY * (float)targetCellY +
                    Random_RangeFloat(-1, 0.0f, ((CCreature*)s)->gridStepY);
            ((CCreature*)s)->targetPosition[2] = ((CCreature*)s)->movementBoundsOrigin[2] +
                ((CCreature*)s)->gridStepZ * (float)targetCellZ +
                Random_RangeFloat(-1, 0.0f, ((CCreature*)s)->gridStepZ);
            // Phase 2: in plain C++ this is  pathPoints.push_back(targetPosition)  when recordPath is set.
            // The fast path writes into spare capacity; the else branch is MSL's inlined growth (kMaxPathPoints /
            // kPathGrowthTier1/2 policy) with the three length_error throw blocks.
            // Append the generated world target to the owned recording.
            // On growth, construct the new suffix, copy the prefix backward,
            // exchange allocations, and let newPath release the former storage.
            if (((CCreature*)s)->recordPath) {
                PathPointVector* recordedPath=(PathPointVector*)&AsCCreature(s)->pathPoints;
                PathPointWords* targetPointWords=(PathPointWords*)(s + kCreatureTarget);
                if (recordedPath->size<recordedPath->capacity) {
                    new(recordedPath->data+recordedPath->size) PathPointWords(*targetPointWords);
                    ++recordedPath->size;
                }
                else {
                    if (kMaxPathPoints-recordedPath->capacity<1) {
                        union {
                            double alignment;
                            unsigned char bytes[sizeof(std::length_error)];
                        }
                        storage;
                        std::length_error* error = (std::length_error*)storage.bytes;
                        error->__ct();
                        exception_runtime::__throw(exception_runtime::npcResourceStrings + kLengthErrorTypeName, error,
                        (void*)exception_runtime::__dt__Q23std12length_errorFv);
                    }
                    unsigned needed=recordedPath->size+1-recordedPath->capacity;
                    TempVec80199DCC newPath;
                    unsigned* capacityAddress=&recordedPath->capacity;
                    unsigned prefixCount=0;
                    unsigned newCapacity=recordedPath->capacity;
                    if (needed>kMaxPathPoints-newCapacity) {
                        union {
                            double alignment;
                            unsigned char bytes[sizeof(std::length_error)];
                        }
                        storage;
                        std::length_error* error = (std::length_error*)storage.bytes;
                        error->__ct();
                        exception_runtime::__throw(exception_runtime::npcResourceStrings + kLengthErrorTypeName, error,
                        (void*)exception_runtime::__dt__Q23std12length_errorFv);
                    }
                    if (newCapacity<kPathGrowthTier1) newCapacity+=LargerCapacity(3*(newCapacity+1)/5,needed);
                    else if (newCapacity<kPathGrowthTier2) newCapacity+=LargerCapacity((newCapacity+1)/2,needed);
                    else newCapacity=kMaxPathPoints;
                    if (newCapacity>kMaxPathPoints) {
                        union {
                            double alignment;
                            unsigned char bytes[sizeof(std::length_error)];
                        }
                        storage;
                        std::length_error* error = (std::length_error*)storage.bytes;
                        error->__ct();
                        exception_runtime::__throw(exception_runtime::npcResourceStrings + kLengthErrorTypeName, error,
                        (void*)exception_runtime::__dt__Q23std12length_errorFv);
                    }
                    newPath.data=(PathPointWords*)appMallocDummy__FUl(newCapacity*kPathPointBytes);
                    newPath.capacity=newCapacity;
                    // Construct the new suffix first, then copy the old prefix backward.
                    prefixCount=recordedPath->size;
                    PathPointWords* output=newPath.data+prefixCount+newPath.size;
                    for (int count=1;count!=0;--count,++output) {
                        new(output) PathPointWords(*targetPointWords);
                        ++newPath.size;
                    }
                    PathPointWords* input=recordedPath->data+recordedPath->size;
                    output=newPath.data+prefixCount;
                    while (input>recordedPath->data) {
                        --output;
                        --input;
                        new(output) PathPointWords(*input);
                        --prefixCount;
                        ++newPath.size;
                    }
                    // Commit: swap buffers; newPath now owns the displaced allocation.
                    recordedPath->size=0;
                    PathPointWords* oldData=recordedPath->data;
                    unsigned previousCapacity=recordedPath->capacity;
                    recordedPath->capacity=newPath.capacity;
                    newPath.capacity=previousCapacity;
                    recordedPath->data=newPath.data;
                    newPath.data=oldData;
                    recordedPath->size=newPath.size;
                    newPath.size=0;
                    newPath.size = (newPath.data) ? newPath.size-newPath.size : newPath.size;
                }
            }
        }
        else {
            // Replay consumes one recorded point and advances the byte-sized cursor.
            // No grid-history update or recording append occurs on this path.
            *(WVector*)(s + kCreatureTarget) = *(WVector*)((char*)AsCCreature(s)->pathPoints+kPathPointBytes*(unsigned char)((CCreature*)s)->pathCursor);
            ++((CCreature*)s)->pathCursor;
        }
    }
    // Phase 3: multi-layer creatures translate using normalized position-target;
    // one-layer creatures steer the forward basis in XZ and apply visual bank.
    // Preserve the subtraction/sign conventions in these recovered operations.
    int layerCount=((CCreature*)s)->gridYCount;
    if (layerCount>1) {
        __mi__FRC7WVectorRC7WVector(directionScratch,s + kCreaturePosition,s + kCreatureTarget);
        __ml__FfRC7WVector(vectorScratch,Normalize__7WVectorFv(directionScratch),((CCreature*)s)->movementSpeed);
        __ml__FRC7WVectorf(vectorScratch+3,vectorScratch,dt);
        *(WVector*)(s + kCreaturePosition) += *(WVector*)(vectorScratch+3);
    }
    else {
        __ml__FfRC7WVector(vectorScratch,s + kCreatureForward,-1.0f);
        __mi__FRC7WVectorRC7WVector(directionScratch,s + kCreaturePosition,s + kCreatureTarget);
        float headingDelta=Vector_SignedAngleXZ(directionScratch,vectorScratch);
        // Ignore heading errors within one degree (radians); retain the original literal pool entry.
        if ((headingDelta>0.0f?headingDelta:-headingDelta)>0.01745329238474369f) {
            // Above the shared threshold, build bank; below it, recover bank
            // using that same parameter as a rate. The dual use is intentional.
            if ((headingDelta>0.0f?headingDelta:-headingDelta)>CreatureHeadingBankParameter(s)) {
                float bankAngle=AsCDisposable(s)->rotationZ;
                if ((bankAngle > 0.0f ? bankAngle : -bankAngle) < ((CCreature*)s)->maxBankAngle)
                    AsCDisposable(s)->rotationZ -= headingDelta * dt;
            }
            else {
                float turnStep=headingDelta*dt;
                turnStep=turnStep>0.0f?turnStep:-turnStep;
                float bankAngle=AsCDisposable(s)->rotationZ;
                bankAngle=bankAngle>0.0f?bankAngle:-bankAngle;
                if (bankAngle > turnStep)
                    AsCDisposable(s)->rotationZ += dt * (headingDelta > 0.0f ?
                        CreatureHeadingBankParameter(s) : -CreatureHeadingBankParameter(s));
                else AsCDisposable(s)->rotationZ = 0.0f;
            }
            // Clamp yaw to +/-turnRate*dt, rotate about world Y, then concatenate
            // with the existing affine transform. Bank is applied later by Draw.
            float minimumTurn=-((CCreature*)s)->turnRate*dt, maximumTurn=((CCreature*)s)->turnRate*dt;
            float turnStep=headingDelta<minimumTurn?minimumTurn:headingDelta>maximumTurn?maximumTurn:headingDelta;
            AsCDisposable(s)->heading+=turnStep;
            minimumTurn=-((CCreature*)s)->turnRate*dt;
            maximumTurn=((CCreature*)s)->turnRate*dt;
            turnStep=headingDelta<minimumTurn?minimumTurn:headingDelta>maximumTurn?maximumTurn:headingDelta;
            float matrix[12], composed[12];
            Matrix_SetAxisRotation(matrix,turnStep,(signed char)1);
            Matrix_ConcatAffine(composed,matrix,s + kCreatureTransform);
            Matrix_CopyAffine(s + kCreatureTransform,composed);
        }
        else {
            // Inside the heading dead zone, relax bank toward zero without yaw.
            float t=AsCDisposable(s)->rotationZ;
            if ((t>0.0f?t:-t)>0.0f) {
                t=AsCDisposable(s)->rotationZ;
                float recoveryRate=CreatureHeadingBankParameter(s);
                if ((t>0.0f?t:-t)>recoveryRate*dt) {
                    if (AsCDisposable(s)->rotationZ>0.0f) recoveryRate=-recoveryRate;
                    AsCDisposable(s)->rotationZ+=dt*recoveryRate;
                }
                else AsCDisposable(s)->rotationZ=0.0f;
            }
        }
        __ml__FfRC7WVector(vectorScratch,s + kCreatureForward,((CCreature*)s)->movementSpeed);
        __ml__FRC7WVectorf(directionScratch,vectorScratch,dt);
        *(WVector*)(s + kCreaturePosition) += *(WVector*)directionScratch;
    }
}

} // extern "C"
