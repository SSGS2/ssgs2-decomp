#include "game/types.h"
#include "classes/CMouseCursor.h"
// Translation unit 0x8025FDB8-0x80260838 (6 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

struct TrailRibbon;
void* __nwa__FUl(int);
void TrailRibbon_SampleScreenPointer(TrailRibbon*);
void TrailRibbon_RebuildInterpolatedVertices(TrailRibbon*);
extern const float kFloat_0_AngleVector;
extern const float kTrailRibbonLifetimeDecay;
extern const float kTrailRibbonDefaultHalfWidth;
extern const float kFloat_1_805E0438;
extern const float kFloat_0_2_PathSampling;
extern const float kTrailPointerHalfScreenWidth;
extern const float kTrailPointerHalfScreenHeight;
extern const float kFloat_2_AngleInterpolation;
extern const float kTrailRibbonAngleBlend;
extern const float kFloat_3_1415927;
extern const unsigned int s_trailRibbonEdgeOffsetTemplate[];
extern void* g_mouseCursor;

int gtl_InputEx_GetScreenPointer(void*);
int gtl_CInputExDeviceBase_GetDeviceResult(int, int*);
int gtl_CScreenPointer_GetSample(int, int, void*);
void gtl_Vec2_Subtract(float*, float*, float*);
float Vec2_Length(float*);
void gtl_Vec2_Copy(float*, float*);
double atan2(float, float);
void Mtx44_SetRotationZ(float*, float);
void Mtx44_TransformVec4(float*, float*, float*);
void TrailRibbon_UpdateSegmentEdges(TrailRibbon*, int, int);

struct V2_8026 { float x, y; };
struct V4_8026 { float x, y, z, w; };

// Parallel rings: pointPositions holds screen-space x/y, segmentAngles holds radians,
// and each vertex pointer owns three V4 records (edge 0, center, edge 2).
// V4.z carries opacity, not spatial depth; the renderer converts it to an alpha byte.
// V4.w is initialized to one. Color/value members retain their signed-byte storage.
// pointLifetime is a fractional live-point budget, not seconds: append adds one,
// Update subtracts lifetimeDecay, and activePointCount is its integer truncation.
// controllerIndex carries integer 0..3 in the pointer-shaped Wii ABI word;
// it is never dereferenced or owned. The constructor allocates all rings and vertex blocks.
struct TrailRibbon {
    void* controllerIndex; int newestPointIndex; int activePointCount; float pointLifetime; float pendingPosition[2];
    V2_8026* pointPositions; V4_8026** segmentVertices; V4_8026** interpolatedVertices; float* segmentAngles; int reservedState;
    signed char edgeRed, edgeGreen, edgeBlue, edgeOpacity, centerRed, centerGreen, centerBlue, centerOpacity;
    float halfWidth, lifetimeDecay; int fadeCenterOpacity, useSeparateEdgeOpacity, taperWidth, pointCapacity, sampledThisFrame, positionPending, sampleStride;
};

enum RibbonStorageSizes { kPointPositionBytes = 8, kRingPointerBytes = 4,
                          kThreeVertexBytes = 0x30, kSegmentAngleBytes = 4 };
// Byte defaults retain the original signed representation, including alpha values.
enum RibbonDefaultColor { kDefaultEdgeRed = 0, kDefaultEdgeGreen = -128,
                          kDefaultEdgeBlue = 0x20, kDefaultEdgeOpacity = 0x50,
                          kDefaultCenterRed = -56, kDefaultCenterGreen = -1,
                          kDefaultCenterBlue = -11, kDefaultCenterOpacity = -36 };

typedef Vec3f V3_8026;
struct Mat8_8026 { unsigned int w[8]; };
struct Mat8f_8026 { float f[8]; };
struct V4u_8026 { unsigned int x, y, z, w; };
struct Mat16f_8026 { float f[16]; };
struct Locals_8026 {
    V2_8026 direction;
    float m[8];
    V4_8026 first;
    V4_8026 second;
    float rot[16];
};

// Smooth adjacent segment angles across the wrap boundary and rebuild their three ribbon vertices.
// Both indices must name initialized points in the ring; fewer than two live points is a no-op.
void TrailRibbon_UpdateSegmentEdges(TrailRibbon* self, int toIndex, int fromIndex) {
    if (self->activePointCount < 2)
        return;

    // Measure heading with swapped atan2 arguments: angle is relative to the Y axis.
    Locals_8026 L;
    gtl_Vec2_Subtract((float*)&L.direction, (float*)&self->pointPositions[toIndex],
                (float*)&self->pointPositions[fromIndex]);
    float dy = L.direction.y;
    float dx = L.direction.x;
    float heading = atan2(dx, dy);
    ((float*)self->segmentAngles)[toIndex] = heading;
    if (self->activePointCount == 2)
        ((float*)self->segmentAngles)[fromIndex] = ((float*)self->segmentAngles)[toIndex];

    *(Mat8_8026*)L.m = *(Mat8_8026*)s_trailRibbonEdgeOffsetTemplate;
    L.m[0] = -self->halfWidth;
    L.m[4] = self->halfWidth;

    float* angles = (float*)self->segmentAngles;
    // Headings from atan2 are radians in [-pi, pi]. Across the branch cut,
    // e.g. -179 to +179 degrees, lift the previous angle by one full turn so
    // delta is the short arc. Blend = previous + angleBlend*(next-previous),
    // then fold by one full turn to restore the atan2 interval.
    // Preserve the two condition tests/order: they also define the +/-pi boundary.
    float nextAngle = angles[toIndex];
    float previousAngle = angles[fromIndex];
    if (nextAngle - previousAngle > kFloat_3_1415927) {
        if (previousAngle < kFloat_0_AngleVector)
            previousAngle = kFloat_2_AngleInterpolation * kFloat_3_1415927 + previousAngle;
    } else if (nextAngle - previousAngle < -kFloat_3_1415927 && previousAngle > kFloat_0_AngleVector) {
        previousAngle = previousAngle - kFloat_2_AngleInterpolation * kFloat_3_1415927;
    }
    float angleDelta = nextAngle - previousAngle;
    float blendedAngle = angleDelta * kTrailRibbonAngleBlend + previousAngle;
    if (blendedAngle > kFloat_3_1415927)
        blendedAngle -= kFloat_2_AngleInterpolation * kFloat_3_1415927;
    else if (blendedAngle < -kFloat_3_1415927)
        blendedAngle += kFloat_2_AngleInterpolation * kFloat_3_1415927;
    angles[fromIndex] = blendedAngle;

    // Rotate local +/-halfWidth offsets by negative heading and translate to screen space.
    // Rebuild the previous point with the smoothed angle, keeping its center unchanged.
    Mtx44_SetRotationZ(L.rot, -((float*)self->segmentAngles)[fromIndex]);
    Mtx44_TransformVec4((float*)&L.first, L.m, L.rot);
    Mtx44_TransformVec4((float*)&L.second, L.m + 4, L.rot);
    float zv = (float)self->edgeOpacity;
    V2_8026* src = &self->pointPositions[fromIndex];
    V4_8026* out = ((V4_8026**)self->segmentVertices)[fromIndex];
    float x0 = src->x + L.first.x;
    float y0 = src->y + L.first.y;
    out[0].x = x0;
    out[0].y = y0;
    out[0].z = zv;
    out[0].w = kFloat_1_805E0438;
    zv = (float)self->centerOpacity;
    float sx = src->x;
    float sy = src->y;
    out = ((V4_8026**)self->segmentVertices)[fromIndex];
    out[1].x = sx;
    out[1].y = sy;
    out[1].z = zv;
    out[1].w = kFloat_1_805E0438;
    zv = (float)self->edgeOpacity;
    float sx2 = src->x + L.second.x;
    float sy2 = src->y + L.second.y;
    out = ((V4_8026**)self->segmentVertices)[fromIndex];
    out[2].x = sx2;
    out[2].y = sy2;
    out[2].z = zv;
    out[2].w = kFloat_1_805E0438;

    // Rebuild the new point with its unsmoothed segment angle using the same edge layout.
    Mtx44_SetRotationZ(L.rot, -((float*)self->segmentAngles)[toIndex]);
    Mtx44_TransformVec4((float*)&L.first, L.m, L.rot);
    Mtx44_TransformVec4((float*)&L.second, L.m + 4, L.rot);
    zv = (float)self->edgeOpacity;
    src = &self->pointPositions[toIndex];
    out = ((V4_8026**)self->segmentVertices)[toIndex];
    float x0t = src->x + L.first.x;
    float y0t = src->y + L.first.y;
    out[0].x = x0t;
    out[0].y = y0t;
    out[0].z = zv;
    out[0].w = kFloat_1_805E0438;
    zv = (float)self->centerOpacity;
    float syt = src->y;
    float sxt = src->x;
    out = ((V4_8026**)self->segmentVertices)[toIndex];
    out[1].x = sxt;
    out[1].y = syt;
    out[1].z = zv;
    out[1].w = kFloat_1_805E0438;
    zv = (float)self->edgeOpacity;
    float tx2 = src->x + L.second.x;
    float ty2 = src->y + L.second.y;
    out = ((V4_8026**)self->segmentVertices)[toIndex];
    out[2].x = tx2;
    out[2].y = ty2;
    out[2].z = zv;
    out[2].w = kFloat_1_805E0438;
}

// Initialize ribbon appearance and allocate the point, angle and vertex rings.
// Require pointCapacity > 0, sampleStride > 0 and successful allocations; storage is
// uninitialized until sampled. Defaults below are stored color/opacity bytes, not modes.
void* TrailRibbon_Ctor(TrailRibbon* self, void* controllerIndex, int pointCapacity, int sampleStride) {
    float defaultHalfWidth, defaultDecay, zero;
    zero = kFloat_0_AngleVector;
    defaultDecay = kTrailRibbonLifetimeDecay;
    defaultHalfWidth = kTrailRibbonDefaultHalfWidth;
    self->sampledThisFrame = 0;
    self->controllerIndex = controllerIndex;
    self->newestPointIndex = 0;
    self->activePointCount = 0;
    self->pointLifetime = zero;
    self->reservedState = 0;
    self->positionPending = 0;
    self->edgeRed = kDefaultEdgeRed;
    self->edgeGreen = kDefaultEdgeGreen;
    self->edgeBlue = kDefaultEdgeBlue;
    self->edgeOpacity = kDefaultEdgeOpacity;
    self->centerRed = kDefaultCenterRed;
    self->centerGreen = kDefaultCenterGreen;
    self->centerBlue = kDefaultCenterBlue;
    self->centerOpacity = kDefaultCenterOpacity;
    self->lifetimeDecay = defaultDecay;
    self->pointCapacity = pointCapacity;
    self->fadeCenterOpacity = 1;
    self->useSeparateEdgeOpacity = 1;
    self->taperWidth = 1;
    self->halfWidth = defaultHalfWidth;
    self->sampleStride = sampleStride;
    // Allocate parallel ring tables, then one edge/center/edge triplet per slot.
    self->pointPositions = (V2_8026*)__nwa__FUl(pointCapacity * kPointPositionBytes);
    self->segmentVertices = (V4_8026**)__nwa__FUl(self->pointCapacity * kRingPointerBytes);
    self->interpolatedVertices = (V4_8026**)__nwa__FUl(self->pointCapacity * kRingPointerBytes);
    int offset = 0;
    int i = 0;
    while (i < self->pointCapacity) {
        ((void**)self->segmentVertices)[i] = __nwa__FUl(kThreeVertexBytes);
        ((void**)self->interpolatedVertices)[i] = __nwa__FUl(kThreeVertexBytes);
        ++i;
        offset += 4;
    }
    self->segmentAngles = (float*)__nwa__FUl(self->pointCapacity * kSegmentAngleBytes);
    return self;
}

// Decay the live point count, sample the pointer, then rebuild tapered display vertices.
// One call advances one lifetime step (no elapsed-time argument); always returns 1.
int TrailRibbon_Update(TrailRibbon* self) {
    float decay;
    float lifetime;
    float zero;
    zero = kFloat_0_AngleVector;
    lifetime = self->pointLifetime;
    decay = self->lifetimeDecay;
    zero = (lifetime - decay <= zero) ? zero : (lifetime - decay);
    double clampedLifetime = zero;
    self->pointLifetime = zero;
    self->activePointCount = (int)(float)clampedLifetime;
    TrailRibbon_SampleScreenPointer(self);
    TrailRibbon_RebuildInterpolatedVertices(self);
    return 1;
}

// Copy two borrowed screen-space coordinates and mark a pending sampling request.
// This position is used during scripted cursor movement; otherwise device history is used.
void TrailRibbon_SetPointerPosition(TrailRibbon* self, float* input) {
    float second = input[1];
    int one = 1;
    float first = input[0];
    self->pendingPosition[0] = first;
    self->pendingPosition[1] = second;
    self->positionPending = one;
}

struct Vec2 { float x, y; };
struct QueryResult {
    char reservedBeforePosition[0xC];
    float x, y;
    char reservedBeforeFrame[0xC];
    int frame;
};

#pragma opt_propagation off
// Append sufficiently separated pointer samples to the ring and consume the pending position.
// Require a valid cursor singleton, initialized newest point, positive capacity/stride.
// Failed device queries preserve positionPending; sampledThisFrame means query accepted,
// even if no point passed the distance threshold. Successful queries consume the request.
void TrailRibbon_SampleScreenPointer(TrailRibbon* self) {
    self->sampledThisFrame = 0;
    int screenPointer = gtl_InputEx_GetScreenPointer(self->controllerIndex);
    if (screenPointer != 0 && self->positionPending != 0) {
        struct { int sampleCount, pad; Vec2 fromLoop, deltaLoop, from, delta, desired; QueryResult result; } L;
        if (gtl_CInputExDeviceBase_GetDeviceResult(screenPointer, &L.sampleCount) != 0) {
            int minimumFrame = 1;
            // Scripted cursor movement uses the caller's pending screen position.
            if (((CMouseCursor*)g_mouseCursor)->moveToActive != 0) { // cursor +0x108
                float* slot;
                int target;
                int newestIndex, wrapQuotient, nextUnwrappedIndex, capacity;
                V2_8026* points;
                newestIndex = self->newestPointIndex;
                capacity = self->pointCapacity;
                points = self->pointPositions;
                // next = (newest+capacity+1) % capacity. The quotient form below
                // is the matched remainder expansion; all indices require positive capacity.
                int combined = newestIndex + capacity;
                nextUnwrappedIndex = combined + 1;
                wrapQuotient = nextUnwrappedIndex / capacity;
                L.from.x = points[newestIndex].x;
                L.from.y = self->pointPositions[self->newestPointIndex].y;
                target = nextUnwrappedIndex - wrapQuotient * capacity;
                slot = (float*)&points[target];
                gtl_Vec2_Subtract((float*)&L.delta, self->pendingPosition, (float*)&L.from);
                if (!(Vec2_Length((float*)&L.delta) <= kFloat_0_2_PathSampling)) {
                    gtl_Vec2_Copy(slot, self->pendingPosition);
                    float livePointBudget = kFloat_1_805E0438 + self->pointLifetime;
                    if (livePointBudget >= (float)self->pointCapacity)
                        livePointBudget = (float)self->pointCapacity;
                    self->pointLifetime = livePointBudget;
                    self->activePointCount = (int)self->pointLifetime;
                    TrailRibbon_UpdateSegmentEdges(self, target, self->newestPointIndex);
                    self->newestPointIndex = target;
                }
            } else {
                // Replay device history from sampleCount-1 downward, accepting nondecreasing
                // frame values. Map normalized coordinates with x=halfWidth*(x+1),
                // y=halfHeight*(y+1); these are the same screen units as pendingPosition.
                float halfScreenHeight, halfScreenWidth;
                halfScreenWidth = kTrailPointerHalfScreenWidth;
                halfScreenHeight = kTrailPointerHalfScreenHeight;
                for (int i = 0; i < L.sampleCount; ++i) {
                    if (i % self->sampleStride == 0) {
                        if (gtl_CScreenPointer_GetSample(screenPointer, L.sampleCount - i - 1, &L.result) != 0 && L.result.frame >= minimumFrame) {
                            minimumFrame = L.result.frame;
                            L.desired.x = halfScreenWidth * L.result.x + halfScreenWidth;
                            L.desired.y = halfScreenHeight * L.result.y + halfScreenHeight;
                            float* slot;
                            int target;
                            int newestIndex, wrapQuotient, nextUnwrappedIndex, capacity;
                            V2_8026* points;
                            newestIndex = self->newestPointIndex;
                            capacity = self->pointCapacity;
                            points = self->pointPositions;
                            // Same ring advance as the pending-position path above.
                            int combined = newestIndex + capacity;
                            nextUnwrappedIndex = combined + 1;
                            wrapQuotient = nextUnwrappedIndex / capacity;
                            L.fromLoop.x = points[newestIndex].x;
                            L.fromLoop.y = self->pointPositions[self->newestPointIndex].y;
                            target = nextUnwrappedIndex - wrapQuotient * capacity;
                            slot = (float*)&points[target];
                            gtl_Vec2_Subtract((float*)&L.deltaLoop, (float*)&L.desired, (float*)&L.fromLoop);
                            if (!(Vec2_Length((float*)&L.deltaLoop) <= kFloat_0_2_PathSampling)) {
                                gtl_Vec2_Copy(slot, (float*)&L.desired);
                                float livePointBudget = kFloat_1_805E0438 + self->pointLifetime;
                                if (livePointBudget >= (float)self->pointCapacity)
                                    livePointBudget = (float)self->pointCapacity;
                                self->pointLifetime = livePointBudget;
                                self->activePointCount = (int)self->pointLifetime;
                                TrailRibbon_UpdateSegmentEdges(self, target, self->newestPointIndex);
                                self->newestPointIndex = target;
                            }
                        }
                    }
                }
            }
            // Query processing completed; ring count can still be unchanged.
            self->sampledThisFrame = 1;
            self->positionPending = 0;
        }
    }
}

#pragma opt_propagation reset
// Screen-plane edge/center/edge triplet; each vertex occupies four floats.
// +00 edge0X/edge0Y, +08 edge0Opacity/edge0W; +10 centerX/centerY, +18 centerOpacity/centerW;
// +20 edge2X/edge2Y, +28 edge2OpacityAndW[2]. Opacity is in the V4 z lane.
// The legacy u/v declaration at +30 lies beyond the 0x30-byte allocation and is never accessed.
// Do not use sizeof(RibbonVertexTriplet) to allocate or step through vertex blocks.
struct RibbonVertexTriplet {
    float edge0X, edge0Y;
    float edge0Opacity, edge0W;
    float centerX, centerY;
    float centerOpacity, centerW;
    float edge2X, edge2Y;
    float edge2OpacityAndW[2];
    float u, v;
};

// Walk newest-to-oldest points and apply the configured width taper and center-value fade.
// Require 0 <= activePointCount <= pointCapacity; output remains owned by the ribbon.
void TrailRibbon_RebuildInterpolatedVertices(TrailRibbon* self) {
    float one = kFloat_1_805E0438;
    int selected = self->useSeparateEdgeOpacity != 0 ? self->edgeOpacity : self->centerOpacity;
    float edgeOutputValue = (float)selected;
    int i = 0;
    while (i < self->activePointCount) {
        // i is point age, not a device frame; modulo wraps newest-i into the ring.
        int wrappedAgeIndex = self->pointCapacity + (self->newestPointIndex - i);
        int index = wrappedAgeIndex % self->pointCapacity;
        float t = (float)i / (float)self->activePointCount;
        float blend = one - t;
        RibbonVertexTriplet* src = ((RibbonVertexTriplet**)self->segmentVertices)[index];
        RibbonVertexTriplet* dst = ((RibbonVertexTriplet**)self->interpolatedVertices)[index];
        float centerOutputValue;
        if (self->fadeCenterOpacity != 0)
            centerOutputValue = (float)self->centerOpacity - ((float)self->centerOpacity * (float)i) / (float)self->activePointCount;
        else
            centerOutputValue = (float)self->centerOpacity;
        if (self->taperWidth != 0) {
            // Screen edge = center + (1 - age/count) * (edge - center).
            dst->edge0X = blend * (src->edge0X - src->centerX) + src->centerX;
            dst->edge0Y = blend * (src->edge0Y - src->centerY) + src->centerY;
            dst->edge2X = blend * (src->edge2X - src->centerX) + src->centerX;
            dst->edge2Y = blend * (src->edge2Y - src->centerY) + src->centerY;
        } else {
            dst->edge0X = src->edge0X; dst->edge0Y = src->edge0Y;
            dst->edge2X = src->edge2X; dst->edge2Y = src->edge2Y;
        }
        // Center stays fixed; write opacity to each V4's z lane and one to w.
        dst->centerX = src->centerX; dst->centerY = src->centerY;
        dst->edge0Opacity = edgeOutputValue; dst->centerOpacity = centerOutputValue;
        dst->edge2OpacityAndW[0] = edgeOutputValue; dst->edge2OpacityAndW[1] = one;
        dst->centerW = one; dst->edge0W = one;
        ++i;
    }
}

} // extern "C"
