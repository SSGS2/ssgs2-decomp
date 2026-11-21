#include <decomp/static_init.h>
typedef unsigned long new_size_t;
#include "game/types.h"
#include "game/wvector.h"
#include "classes/CSparrow.h"
#include "classes/CCreature.h"
#include "classes/WPuppet.h"

#define AsCSparrow(p) ((CSparrow*)(p))
// Translation unit 0x801A6854-0x801A7580 (12 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

// Sparse view of the existing byte pool; resource bytes and storage are unchanged.
struct NpcResourcePool {
    char beforeDefaultAnimation[0x11E];
    char defaultAnimation[12];
    char opacityDirective[8];
    char effectDirective[4];
    char effectScanFormat[9];
    char sprayExtension[4];
    char sequenceExtension[4];
    char beforeWalkAnimation[0x32];
    char walkAnimation[9];
    char beforeShadowBitmap[0x81];
    char shadowBitmap[12];
    char beforeNpcClassName[0x79];
    char npcClassName[5];
};

enum SplineConstants { kSplineQuadratureSamples = 5, kSplineMinimumPoints = 4 };
extern "C" {

extern char s_npcRttiDescriptor;
extern int s_actorTypeInfo;
extern int g_soundManager;
extern const float kFloat_6_2831855;
extern float s_twoPi_801A6854;
extern const float kFloat_0_1_Creature;
extern const float kFloat_1000_Creature;
extern const float kFloat_1_Creature;
extern const int kFloat_400_CreatureSound;
extern const int kSparrowSoundLimit1000Bits;
extern char s_npcFactoryClassName;
extern char s_npcTaskClassName;
extern char npcResourceStrings[];
extern char __vt__8CSparrow[];
extern void IntPair_Set(void*, void*, void*);
extern void* ActorFactoryRegistry_GetInstance();
extern void StringMap_SetByName(void*, void*, const char*);
extern void* TaskRegistry_GetInstance();
extern void TaskRegistry_AppendFactory(void*, const char*);
extern void CNPC_Create();
extern int Parse_NonemptyQuotedTokenUnder64Bytes(void*, int, void*);
extern void Sound_PlayByName(void*, void*, void*, int, void*, void*, float, float, float);
extern void __dla__FPv(void*);
extern int FxAttachment_ParseBoneReferences(void*, int, int, int, int, int, int);
extern void CCreature_dtor(void*, int);
extern void __dl__FPv(void*);
extern void appFreeUni__FPv(void*);
extern void __dt__18NativeBase80201D18Fv(void*, int);
extern void* __nwa__FUl(unsigned int);
extern void* __construct_new_array(void*, void*, void*, unsigned int, unsigned int);
extern void __as__7WVectorFRC7WVector(void*, void*);
extern const float kFloat_0_Creature;
extern const float kFloat_2_805DF2B0;
extern const float kFloat_3_CreatureSpline;
extern const float kFloat_4_Spline;
extern const float kFloat_1_5_SplineIntegration;
extern const float kFloat_0_5_Creature;
extern const float kFloat_5_creature;
extern void __ml__FfRC7WVector(void*, const void*, float);
extern void __mi__FRC7WVectorRC7WVector(void*, const void*, const void*);
extern void __pl__FRC7WVectorRC7WVector(void*, const void*, const void*);
extern float Magnitude__7WVectorCFv(void*);
extern const float kSplineGaussLegendreNodes5[];
extern const float kSplineGaussLegendreWeights5[];

// Return the registered NPC runtime type descriptor.
char* CNPC_GetRTTI() { return &s_npcRttiDescriptor; }

// Parse creature bone references using the current puppet and animation range.
int CCreature_ParseBoneReferences(void* self, int parser, int argument) {
    // CCreature_ParseBoneReferences: original puppet +0x24 and animation range +0x60.
    int puppet = ((CCreature*)self)->puppet;
    int animationRange = ((CCreature*)self)->animationRange;
    return FxAttachment_ParseBoneReferences(&((CCreature*)self)->secondaryVtable, parser, argument, puppet, animationRange, 0, 0);
}

// Destroy the crab creature base and optionally release object storage.
void* CCrab_Dtor(void* self, int flag) {
    if (self != 0) {
        CCreature_dtor(self, 0);
        if (flag > 0)
            __dl__FPv(self);
    }
    return self;
}

// Destroy the dolphin creature base and optionally release object storage.
void* CDolphin_Dtor(void* self, int flag) {
    if (self != 0) {
        CCreature_dtor(self, 0);
        if (flag > 0)
            __dl__FPv(self);
    }
    return self;
}

// Destroy the mole creature base and optionally release object storage.
void* CMole_Dtor(void* self, int flag) {
    if (self != 0) {
        CCreature_dtor(self, 0);
        if (flag > 0)
            __dl__FPv(self);
    }
    return self;
}

// Initialize the NPC angle constant and register its actor factory and task name.
void CNPC_RegisterType() {
    s_twoPi_801A6854 = kFloat_6_2831855;
    char* p = ((NpcResourcePool*)npcResourceStrings)->npcClassName;
    IntPair_Set(&s_npcRttiDescriptor, p, &s_actorTypeInfo);
    StringMap_SetByName(ActorFactoryRegistry_GetInstance(), (void*)CNPC_Create, &s_npcFactoryClassName);
    TaskRegistry_AppendFactory(TaskRegistry_GetInstance(), &s_npcTaskClassName);
}

struct ListNodeLink { ListNodeLink* prev; ListNodeLink* next; };
struct ListAt18 { char pad[0x18]; int size; ListNodeLink head; };

// Unlink and free owned NPC list nodes, destroy the native base, and optionally delete.
void* CNPC_dtor(ListAt18* self, int flag) {
    if (self) {
        if (&self->size != 0 && &self->size != 0 && &self->size != 0 && &self->size != 0) {
            ListNodeLink* end = &self->head;
            ListNodeLink* node = self->head.next;
            if (node != end) {
                ListNodeLink* tail = end->prev;
                node->prev->next = tail->next;
                tail->next->prev = node->prev;
                while (node != end) {
                    ListNodeLink* current = node;
                    node = node->next;
                    appFreeUni__FPv(current);
                    self->size = self->size - 1;
                }
            }
        }
        __dt__18NativeBase80201D18Fv(self, 0);
        if (0 < flag)
            __dl__FPv(self);
    }
    return self;
}

// One packed xyz control point (three floats); construction deliberately leaves it uninitialized.
struct Entry {
    // Leave the coordinates for Spline_Init to copy from its input.
    Entry() {}
    static void* operator new[](new_size_t size);
    char data[0x0c];
};
// Allocate control-point array storage through the game's array allocator.
inline void* Entry::operator new[](new_size_t size) { return __nwa__FUl(size); }
// Control points, their parameter knots, cached segment lengths, total length and point count.
// entries/values/sums are owned allocations, not aliases of the initialization inputs.
// values has count strictly increasing knots; sums has count-1 individual lengths
// (not prefix sums). total is their sum in the control points' distance units.
// Successful initialization supplies at least four points. Evaluation's short-count
// fallback does not make arbitrary partially initialized containers safe to use.
struct Container { Entry* entries; float* values; float* sums; float total; int count; };

typedef Vec3f Vec3;
struct IntVec { int a, b, c; };
// Estimate a spline segment arc length over a clamped interval using five-point Gaussian quadrature.
// index selects the span [P[index],P[index+1]]; start/end are normalized span
// coordinates, not global knots. Callers supply a valid nonnegative index and
// 0 <= start <= end <= 1; the partial clamping below is not general validation.
// No ownership transfer. Return distance, independent of the knot interval width.
extern "C" float Spline_IntegrateSegmentLength(Container* self, int index, float start, float end) {
    Vec3 quadraticCoefficient, linearCoefficient, tangentCoefficient, result;
    Vec3 firstQ, firstP0MinusTwiceP1, firstTwiceP1;
    Vec3 firstL, firstFourP1MinusThreeP0, firstFourP1, firstThreeP0;
    Vec3 lastQ, lastPreviousMinusTwiceCurrent, lastTwiceCurrent;
    Vec3 lastL;
    Vec3 interiorQ, interiorQWithoutFarPoint, interiorThreeCurrentMinusPrevious, interiorThreeCurrent, interiorThreeNext;
    Vec3 interiorL, interiorLWithoutFarPoint, interiorTwicePreviousMinusFiveCurrent, interiorTwicePrevious, interiorFiveCurrent, interiorFourNext;
    Vec3 interiorT;
    Vec3 endpointDerivative, endpointHalfL, endpointQu;
    Vec3 interiorDerivative, interiorHalfT, interiorDerivativeTimesU, interiorDerivativeInner, interiorThreeHalvesQu;
    if (end <= start)
        return kFloat_0_Creature;
    if (start < kFloat_0_Creature) start = kFloat_0_Creature;
    if (end > kFloat_1_Creature) end = kFloat_1_Creature;
    float threeHalves, half, span;
    float weightedLength = kFloat_0_Creature;
    // Derive coefficients from points in their original coordinate space.
    // First span: Q=P0-2P1+P2, L=-3P0+4P1-P2, derivative Q*u+L/2.
    // Last span: Q=P[i-1]-2P[i]+P[i+1], L=P[i+1]-P[i-1], same derivative.
    // Interior: Q=-P[i-1]+3P[i]-3P[i+1]+P[i+2],
    // L=2P[i-1]-5P[i]+4P[i+1]-P[i+2], T=P[i+1]-P[i-1].
    // Interior derivative is (3/2)*Q*u*u + L*u + T/2.
    // Named stage temporaries preserve each vector operation's rounding/order.
    if (index == 0) {
        Entry* points = self->entries;
        __ml__FfRC7WVector(&firstTwiceP1, &points[1], kFloat_2_805DF2B0);
        __mi__FRC7WVectorRC7WVector(&firstP0MinusTwiceP1, self->entries, &firstTwiceP1);
        __pl__FRC7WVectorRC7WVector(&firstQ, &firstP0MinusTwiceP1, &points[2]);
        __as__7WVectorFRC7WVector(&quadraticCoefficient, &firstQ);
        points = self->entries;
        __ml__FfRC7WVector(&firstThreeP0, points, kFloat_3_CreatureSpline);
        __ml__FfRC7WVector(&firstFourP1, &self->entries[1], kFloat_4_Spline);
        __mi__FRC7WVectorRC7WVector(&firstFourP1MinusThreeP0, &firstFourP1, &firstThreeP0);
        __mi__FRC7WVectorRC7WVector(&firstL, &firstFourP1MinusThreeP0, &points[2]);
        __as__7WVectorFRC7WVector(&linearCoefficient, &firstL);
    } else {
        int last = self->count - 2;
        if (index >= last) {
            int i = last;
            index = i;
            char* pointBytes;
            int previousByteOffset, nextByteOffset;
            nextByteOffset = (i + 1) * 0xC;
            previousByteOffset = (i - 1) * 0xC;
            pointBytes = (char*)self->entries;
            __ml__FfRC7WVector(&lastTwiceCurrent, pointBytes + i * 0xC, kFloat_2_805DF2B0);
            __mi__FRC7WVectorRC7WVector(&lastPreviousMinusTwiceCurrent, (char*)self->entries + previousByteOffset, &lastTwiceCurrent);
            __pl__FRC7WVectorRC7WVector(&lastQ, &lastPreviousMinusTwiceCurrent, pointBytes + nextByteOffset);
            __as__7WVectorFRC7WVector(&quadraticCoefficient, &lastQ);
            char* reloadedPointBytes = (char*)self->entries;
            __mi__FRC7WVectorRC7WVector(&lastL, reloadedPointBytes + nextByteOffset, reloadedPointBytes + previousByteOffset);
            __as__7WVectorFRC7WVector(&linearCoefficient, &lastL);
        } else {
            int farByteOffset, nextByteOffset;
            char* pointBytes;
            int currentByteOffset, previousByteOffset;
            pointBytes = (char*)self->entries;
            farByteOffset = (index + 2) * 0xC;
            nextByteOffset = (index + 1) * 0xC;
            currentByteOffset = index * 0xC;
            previousByteOffset = (index - 1) * 0xC;
            __ml__FfRC7WVector(&interiorThreeNext, pointBytes + nextByteOffset, kFloat_3_CreatureSpline);
            __ml__FfRC7WVector(&interiorThreeCurrent, pointBytes + currentByteOffset, kFloat_3_CreatureSpline);
            __mi__FRC7WVectorRC7WVector(&interiorThreeCurrentMinusPrevious, &interiorThreeCurrent, pointBytes + previousByteOffset);
            __mi__FRC7WVectorRC7WVector(&interiorQWithoutFarPoint, &interiorThreeCurrentMinusPrevious, &interiorThreeNext);
            __pl__FRC7WVectorRC7WVector(&interiorQ, &interiorQWithoutFarPoint, pointBytes + farByteOffset);
            __as__7WVectorFRC7WVector(&quadraticCoefficient, &interiorQ);
            char* reloadedPointBytes = (char*)self->entries;
            __ml__FfRC7WVector(&interiorFourNext, reloadedPointBytes + nextByteOffset, kFloat_4_Spline);
            __ml__FfRC7WVector(&interiorFiveCurrent, (char*)self->entries + currentByteOffset, kFloat_5_creature);
            __ml__FfRC7WVector(&interiorTwicePrevious, (char*)self->entries + previousByteOffset, kFloat_2_805DF2B0);
            __mi__FRC7WVectorRC7WVector(&interiorTwicePreviousMinusFiveCurrent, &interiorTwicePrevious, &interiorFiveCurrent);
            __pl__FRC7WVectorRC7WVector(&interiorLWithoutFarPoint, &interiorTwicePreviousMinusFiveCurrent, &interiorFourNext);
            __mi__FRC7WVectorRC7WVector(&interiorL, &interiorLWithoutFarPoint, reloadedPointBytes + farByteOffset);
            __as__7WVectorFRC7WVector(&linearCoefficient, &interiorL);
            char* tangentPointBytes = (char*)self->entries;
            __mi__FRC7WVectorRC7WVector(&interiorT, tangentPointBytes + nextByteOffset, tangentPointBytes + previousByteOffset);
            __as__7WVectorFRC7WVector(&tangentCoefficient, &interiorT);
        }
    }
    // Map each Gauss node from [-1,1] to [start,end], sample |dP/du|,
    // then scale the weighted sum by half the integration interval.
    span = end - start;
    half = kFloat_0_5_Creature;
    threeHalves = kFloat_1_5_SplineIntegration;
    for (int n = 0; n < kSplineQuadratureSamples; ++n) {
        float t = half * (start + (span * kSplineGaussLegendreNodes5[n] + end));
        if (index == 0 || index >= self->count - 2) {
            __ml__FfRC7WVector(&endpointQu, &quadraticCoefficient, t);
            __ml__FfRC7WVector(&endpointHalfL, &linearCoefficient, kFloat_0_5_Creature);
            __pl__FRC7WVectorRC7WVector(&endpointDerivative, &endpointHalfL, &endpointQu);
            __as__7WVectorFRC7WVector(&result, &endpointDerivative);
        } else {
            __ml__FfRC7WVector(&interiorThreeHalvesQu, &quadraticCoefficient, threeHalves * t);
            __pl__FRC7WVectorRC7WVector(&interiorDerivativeInner, &linearCoefficient, &interiorThreeHalvesQu);
            __ml__FfRC7WVector(&interiorDerivativeTimesU, &interiorDerivativeInner, t);
            __ml__FfRC7WVector(&interiorHalfT, &tangentCoefficient, kFloat_0_5_Creature);
            __pl__FRC7WVectorRC7WVector(&interiorDerivative, &interiorHalfT, &interiorDerivativeTimesU);
            __as__7WVectorFRC7WVector(&result, &interiorDerivative);
        }
        float speed = Magnitude__7WVectorCFv(&result);
        float weight = kSplineGaussLegendreWeights5[n];
        weightedLength = weight * speed + weightedLength;
    }
    weightedLength *= kFloat_0_5_Creature * (end - start);
    return weightedLength;
}
#pragma opt_strength_reduction off
#pragma opt_propagation off
// Clamp to endpoint positions or evaluate the selected quadratic/cubic spline span.
// Borrow self; write xyz to out in the control points' coordinate space.
// t uses the units of values (knots), while u below is dimensionless in [0,1].
// Requires increasing knots and an initialized container; no array is modified.
void Spline_EvaluatePosition(Vec3* out, Container* self, float t) {
    int knotByteOffset;
    int count;
    int last;
    float* knots;
    int segmentIndex;
    count = self->count;
    if (2 > count) {
        float zero = kFloat_0_Creature;
        float savedZero = zero;
        out->x = savedZero;
        out->y = zero;
        out->z = zero;
        return;
    }
    knots = self->values;
    if (t <= knots[0]) {
        { IntVec* src = (IntVec*)self->entries; *(unsigned long long*)out = *(unsigned long long*)src; ((IntVec*)out)->c = src->c; }
        return;
    }
    last = count - 1;
    if (t >= knots[last]) {
        { IntVec* src = (IntVec*)&self->entries[last]; *(unsigned long long*)out = *(unsigned long long*)src; ((IntVec*)out)->c = src->c; }
        return;
    }
    // Find the first knot interval whose right endpoint contains t.
    // knotByteOffset is a float-array byte knotByteOffset; point offsets below use 12-byte xyz entries.
    segmentIndex = 0;
    knotByteOffset = 0;
    for (; last > segmentIndex; knotByteOffset += 4, ++segmentIndex) {
        if (t <= *(float*)((char*)knots + knotByteOffset + 4)) break;
    }
    float start = knots[segmentIndex];
    float end = knots[segmentIndex + 1];
    float u = (t - start) / (end - start);
    Vec3 firstQ;
    Vec3 firstL;
    Vec3 lastQ;
    Vec3 lastL;
    Vec3 interiorQ;
    Vec3 interiorL;
    Vec3 interiorT;
    Vec3 firstP0MinusTwiceP1;
    Vec3 firstTwiceP1;
    Vec3 firstFourP1MinusThreeP0;
    Vec3 firstFourP1;
    Vec3 firstThreeP0;
    Vec3 firstDisplacement;
    Vec3 firstInner;
    Vec3 firstQu;
    Vec3 lastPreviousMinusTwiceCurrent;
    Vec3 lastTwiceCurrent;
    Vec3 lastDisplacement;
    Vec3 lastInner;
    Vec3 lastQu;
    Vec3 interiorQWithoutFarPoint;
    Vec3 interiorThreeCurrentMinusPrevious;
    Vec3 interiorThreeCurrent;
    Vec3 interiorThreeNext;
    Vec3 interiorLWithoutFarPoint;
    Vec3 interiorTwicePreviousMinusFiveCurrent;
    Vec3 interiorTwicePrevious;
    Vec3 interiorFiveCurrent;
    Vec3 interiorFourNext;
    Vec3 interiorDisplacement;
    Vec3 interiorTangentInner;
    Vec3 interiorQuadraticTimesU;
    Vec3 interiorQuadraticInner;
    Vec3 interiorQu;
    // Evaluate in Horner order without regrouping floating-point operations.
    // With Q/L/T as in Spline_IntegrateSegmentLength:
    // endpoint spans: P[segmentIndex] + (Q*u + L)*u/2;
    // interior spans: P[segmentIndex] + ((Q*u + L)*u + T)*u/2.
    // first*/last*/interior* temporaries follow those equations without regrouping.
    if (0 == segmentIndex) {
        Entry* en = self->entries;
        __ml__FfRC7WVector(&firstTwiceP1, &en[1], kFloat_2_805DF2B0);
        __mi__FRC7WVectorRC7WVector(&firstP0MinusTwiceP1, &self->entries[0], &firstTwiceP1);
        __pl__FRC7WVectorRC7WVector(&firstQ, &firstP0MinusTwiceP1, &en[2]);
        en = self->entries;
        __ml__FfRC7WVector(&firstThreeP0, &en[0], kFloat_3_CreatureSpline);
        __ml__FfRC7WVector(&firstFourP1, &self->entries[1], kFloat_4_Spline);
        __mi__FRC7WVectorRC7WVector(&firstFourP1MinusThreeP0, &firstFourP1, &firstThreeP0);
        __mi__FRC7WVectorRC7WVector(&firstL, &firstFourP1MinusThreeP0, &en[2]);
        __ml__FfRC7WVector(&firstQu, &firstQ, u);
        float savedU = u;
        __pl__FRC7WVectorRC7WVector(&firstInner, &firstL, &firstQu);
        __ml__FfRC7WVector(&firstDisplacement, &firstInner, kFloat_0_5_Creature * savedU);
        __pl__FRC7WVectorRC7WVector(out, &self->entries[0], &firstDisplacement);
    } else {
      count -= 2;
      if (segmentIndex >= count) {
        Entry* en;
        int previousByteOffset, nextByteOffset;
        int currentByteOffset;
        currentByteOffset = count * 0xc;
        nextByteOffset = (count + 1) * 0xc;
        previousByteOffset = (count - 1) * 0xc;
        en = self->entries;
        __ml__FfRC7WVector(&lastTwiceCurrent, (char*)en + currentByteOffset, kFloat_2_805DF2B0);
        __mi__FRC7WVectorRC7WVector(&lastPreviousMinusTwiceCurrent, (char*)self->entries + previousByteOffset, &lastTwiceCurrent);
        __pl__FRC7WVectorRC7WVector(&lastQ, &lastPreviousMinusTwiceCurrent, (char*)en + nextByteOffset);
        Entry* en2 = self->entries;
        __mi__FRC7WVectorRC7WVector(&lastL, (char*)en2 + nextByteOffset, (char*)en2 + previousByteOffset);
        __ml__FfRC7WVector(&lastQu, &lastQ, u);
        __pl__FRC7WVectorRC7WVector(&lastInner, &lastL, &lastQu);
        float savedU2 = u;
        float savedValue = kFloat_0_5_Creature * savedU2;
        __ml__FfRC7WVector(&lastDisplacement, &lastInner, savedValue);
        __pl__FRC7WVectorRC7WVector(out, (char*)self->entries + currentByteOffset, &lastDisplacement);
    } else {
        int farByteOffset, nextByteOffset;
        Entry* en;
        int currentByteOffset, previousByteOffset;
        farByteOffset = (segmentIndex + 2) * 0xc;
        previousByteOffset = (segmentIndex - 1) * 0xc;
        nextByteOffset = (segmentIndex + 1) * 0xc;
        en = self->entries;
        currentByteOffset = segmentIndex * 0xc;
        __ml__FfRC7WVector(&interiorThreeNext, (char*)en + nextByteOffset, kFloat_3_CreatureSpline);
        __ml__FfRC7WVector(&interiorThreeCurrent, (char*)en + currentByteOffset, kFloat_3_CreatureSpline);
        __mi__FRC7WVectorRC7WVector(&interiorThreeCurrentMinusPrevious, &interiorThreeCurrent, (char*)en + previousByteOffset);
        __mi__FRC7WVectorRC7WVector(&interiorQWithoutFarPoint, &interiorThreeCurrentMinusPrevious, &interiorThreeNext);
        int savedI = farByteOffset;
        __pl__FRC7WVectorRC7WVector(&interiorQ, &interiorQWithoutFarPoint, (char*)en + savedI);
        Entry* savedEntries = self->entries;
        en = savedEntries;
        __ml__FfRC7WVector(&interiorFourNext, (char*)en + nextByteOffset, kFloat_4_Spline);
        Entry* savedEntries2 = self->entries;
        __ml__FfRC7WVector(&interiorFiveCurrent, (char*)savedEntries2 + currentByteOffset, kFloat_5_creature);
        __ml__FfRC7WVector(&interiorTwicePrevious, (char*)self->entries + previousByteOffset, kFloat_2_805DF2B0);
        __mi__FRC7WVectorRC7WVector(&interiorTwicePreviousMinusFiveCurrent, &interiorTwicePrevious, &interiorFiveCurrent);
        __pl__FRC7WVectorRC7WVector(&interiorLWithoutFarPoint, &interiorTwicePreviousMinusFiveCurrent, &interiorFourNext);
        __mi__FRC7WVectorRC7WVector(&interiorL, &interiorLWithoutFarPoint, (char*)en + farByteOffset);
        __mi__FRC7WVectorRC7WVector(&interiorT, (char*)self->entries + nextByteOffset, (char*)self->entries + previousByteOffset);
        __ml__FfRC7WVector(&interiorQu, &interiorQ, u);
        __pl__FRC7WVectorRC7WVector(&interiorQuadraticInner, &interiorL, &interiorQu);

        __ml__FfRC7WVector(&interiorQuadraticTimesU, &interiorQuadraticInner, u);
        __pl__FRC7WVectorRC7WVector(&interiorTangentInner, &interiorT, &interiorQuadraticTimesU);
        float savedU3 = kFloat_0_5_Creature * u;
        __ml__FfRC7WVector(&interiorDisplacement, &interiorTangentInner, savedU3);
        __pl__FRC7WVectorRC7WVector(out, (char*)self->entries + currentByteOffset, &interiorDisplacement);
    }
    }
}
#pragma opt_strength_reduction reset
#pragma opt_propagation reset

// Copy at least four control points and knots, then cache segment and total arc lengths.
// Return 1 on initialization, 0 if already initialized or inputs/count rejected.
// Inputs are borrowed for this call and copied; knots must be strictly increasing
// (not checked here). The container retains the three newly allocated arrays.
int Spline_Init(Container* self, void* controlPoints, float* parameterKnots, int pointCount) {
    if (0 != self->count)
        return 0;
    if (pointCount < kSplineMinimumPoints || controlPoints == 0 || 0 == parameterKnots)
        return 0;
    // Allocate/copy the point and knot arrays before computing any lengths.
    Entry* entries = new Entry[pointCount];
    self->entries = entries;
    self->values = (float*)__nwa__FUl(pointCount * 4);
    self->count = pointCount;
    char* pointCursor = (char*)controlPoints;
    float* knotCursor = parameterKnots;
    int pointOffset;
    int knotOffset;
    unsigned int i;
    i = 0;
    pointOffset = 0;
    knotOffset = 0;
    for (; i < pointCount; ) {
        *(WVector*)((char*)self->entries + pointOffset) = *(WVector*)pointCursor;
        float knot = *knotCursor;
        pointCursor += 0xc;
        pointOffset += 0xc;
        ++knotCursor;
        i++;
        *(float*)((char*)self->values + knotOffset) = knot;
        knotOffset += 4;
    }
    // Cache one distance per span, then accumulate total distance; knots do not
    // affect geometric arc length because integration uses local span coordinates.
    int segmentIndex;
    unsigned int segmentOffset;
    unsigned int segmentCount;
    float* sums = (float*)__nwa__FUl((pointCount - 1) * 4);
    float zero = kFloat_0_Creature;
    self->sums = sums;
    segmentCount = pointCount - 1;
    segmentIndex = 0;
    segmentOffset = 0;
    self->total = zero;
    for (; segmentIndex < segmentCount; ) {
        float segmentLength = Spline_IntegrateSegmentLength(self, segmentIndex, kFloat_0_Creature, kFloat_1_Creature);
        segmentIndex += 1;
        *(float*)((char*)self->sums + segmentOffset) = segmentLength;
        self->total = self->total + *(float*)((char*)self->sums + segmentOffset);
        segmentOffset += 4;
    }
    return 1;
}

#pragma opt_propagation off
// Parse a sound token and play it at the model translation with creature sound limits.
void CCreature_PlaySoundFromParsedToken(void* self, int parser) {
    char soundName[0x40];
    if (Parse_NonemptyQuotedTokenUnder64Bytes((char*)self + 4, parser, soundName)) {
        int soundLimits[2];
        int position[3];
        unsigned int positionY;
        void* model;
        model = AsCSparrow(self)->model;
        { IntVec* src = (IntVec*)&((WPuppet*)model)->worldMatrix[9]; int positionX; positionX = src->a; positionY = src->b; position[0] = positionX; position[1] = positionY; position[2] = src->c; }
        soundLimits[0] = kFloat_400_CreatureSound;
        soundLimits[1] = kSparrowSoundLimit1000Bits;
        Sound_PlayByName((void*)g_soundManager, soundName, position, 0, soundName, soundLimits,
                    kFloat_0_1_Creature, kFloat_1000_Creature, kFloat_1_Creature);
    }
}
#pragma opt_propagation reset

// Free sparrow instance matrices, destroy the creature base, and optionally delete.
void* CSparrow_Dtor(void* self, int flag) {
    if (self != 0) {
        void* array = AsCSparrow(self)->instanceMatrices;
        *(void**)self = __vt__8CSparrow;
        AsCSparrow(self)->secondaryVtable = __vt__8CSparrow + 0x0c;
        if (array != 0) {
            if (array != 0) {
                __dla__FPv((char*)array - 0x10);
            }
            AsCSparrow(self)->instanceMatrices = 0;
        }
        CCreature_dtor(self, 0);
        if (flag > 0)
            __dl__FPv(self);
    }
    return self;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU8046216C", _ctors_u801A6854_0, CNPC_RegisterType);
#pragma pop
