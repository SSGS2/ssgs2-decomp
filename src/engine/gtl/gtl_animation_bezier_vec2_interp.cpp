// Two-component animation-key interpolation, 0x803E0594..0x803E08B0.
// EvaluateNodeLocalMatrix (gtl_animation_node_evaluate.cpp) and EvaluateNodeWorldMatrix (gtl_animation_model_matrices.cpp)
// select adjacent packed keys, publish the three time globals and dispatch here.
// Mod lookup: handle/time layout -> BezierVec2Key below; branch tolerance ->
// relative/minimumTolerance; inverse time curve -> FindBezierParameter
// (gtl_model_culling_billboard_curve.cpp); value curve -> cubic weights / Vec2_Hermite / linear blend.
// Preserve the float expression sequence: these three branches deliberately
// use different evaluations of the same data.

// Offsets are deltas from key time/value to the corresponding Bezier handle;
// incoming offsets belong to the END key, outgoing offsets to the START key.
// Time is in the animation resource's time coordinate. Value lanes retain the
// track's units; this generic routine does not establish world/screen space.
struct BezierVec2Key {
    float incomingTimeOffset[2];  // +00
    float incomingValueOffset[2]; // +08
    float outgoingTimeOffset[2];  // +10
    float outgoingValueOffset[2]; // +18
    float value[2];               // +20
    float time;                   // +28
};
typedef char BezierVec2KeySize[sizeof(BezierVec2Key) == 0x2C ? 1 : -1];
typedef char BezierVec2TimeOffset[((int)&((BezierVec2Key*)0)->time == 0x28) ? 1 : -1];

extern "C" {

extern float s_animationSegmentStartTime; // Start key time, set by the evaluator before dispatch.
extern float s_animationSampleTime; // Query time in that same coordinate.
extern float s_animationSegmentEndTime; // End key time. Shared scratch: not reentrant/thread-safe.
extern float kFloat_1_GtlModel; // 1.0: complement/linear weights.
extern float kCurveRelativeSpacingTolerance; // 0.01: relative flatness tolerance, also inverse solver.
extern float kCurveMinimumSpacingTolerance; // 0.0001: minimum absolute tolerance, also inverse solver.
extern float kBezierCubicCoefficient; // 3.0: cubic handle coefficient, also inverse solver.

extern float gtl_Curve_FindBezierParameter(float x0, float x1, float x2, float x3, float fraction);
extern void Vec2_Hermite(void* dst, const void* p0, const void* startTangent, const void* endPoint, const void* t1, float fraction);

// Require two complete BezierVec2Key records, out[2], and matching evaluator time
// globals. Borrow all storage; out must not overlap either key. fraction is
// (queryTime-startTime)/(endTime-startTime), normally 0..1; this function neither
// clamps nor rejects extrapolation/NaN. Always writes both lanes and returns 1.
// Branches per lane: curved TIME -> invert time then cubic value; linear time
// but curved VALUE -> Hermite using stored value offsets directly as tangents;
// both flat -> linear blend. Flat means successive control-point differences
// agree within max(0.01*abs(span),0.0001). Raising tolerance admits more cheap
// branches, altering samples; constants are shared with scalar/Vec3/Vec4 paths
// and the inverse solver, so isolate a future tuning control rather than retune
// the shared pool blindly. No allocations or callbacks; no failure rollback.
int gtl_Animation_InterpolateBezierVec2(float* out, const float* startKey, const float* endKey, float fraction) {
    unsigned int component;
    const float* incomingTimeOffset = ((const BezierVec2Key*)endKey)->incomingTimeOffset;
    float* outputComponent = out;
    float cubicWeight = kBezierCubicCoefficient;
    float minimumTolerance = kCurveMinimumSpacingTolerance;
    float relativeTolerance = kCurveRelativeSpacingTolerance;
    const float* outgoingTimeOffset = ((const BezierVec2Key*)startKey)->outgoingTimeOffset;
    const float* incomingValueOffset = ((const BezierVec2Key*)endKey)->incomingValueOffset;
    const float* endValue = ((const BezierVec2Key*)endKey)->value;
    const float* outgoingValueOffset = ((const BezierVec2Key*)startKey)->outgoingValueOffset;
    const float* startValue = ((const BezierVec2Key*)startKey)->value;
    float one = kFloat_1_GtlModel;
    for (component = 0; component < 2; component++) {
        float startTime = ((const BezierVec2Key*)startKey)->time;
        float endTime = ((const BezierVec2Key*)endKey)->time;
        float startTimeHandle = *outgoingTimeOffset;
        float endTimeHandle = *incomingTimeOffset;
        float timeControl1 = startTime + startTimeHandle;
        float timeControl2 = endTimeHandle + endTime;
        float timeTolerance = relativeTolerance * (float)__fabs((double)(endTime - startTime));
        int timeIsLinear = 0;

        if (timeTolerance < minimumTolerance) {
            timeTolerance = minimumTolerance;
        }
        float middleTimeStep = timeControl2 - timeControl1;
        float firstTimeStep = timeControl1 - startTime;
        if ((float)__fabs((double)((endTime - timeControl2) - middleTimeStep)) > timeTolerance) {
            timeIsLinear = 0;
        } else {
            timeIsLinear = !((float)__fabs((double)(middleTimeStep - firstTimeStep)) > timeTolerance);
        }

        // Phase 1: time controls are nonlinear; solve for the Bezier parameter.
        if (!timeIsLinear) {
            float endHandleValue = *incomingValueOffset;
            float endComponentValue = *endValue;
            float startHandleValue = *outgoingValueOffset;
            float startComponentValue = *startValue;
            float bezierFraction = gtl_Curve_FindBezierParameter(s_animationSegmentStartTime, s_animationSegmentStartTime + *outgoingTimeOffset, s_animationSegmentEndTime + *incomingTimeOffset, s_animationSegmentEndTime, s_animationSampleTime);
            float complement = one - bezierFraction;
            float fractionSquared = bezierFraction * bezierFraction;
            float complementSquared = complement * complement;
            float endHandleBasis = fractionSquared * complement;
            float startHandleBasis = complementSquared * bezierFraction;
            float endHandleWeight = cubicWeight * endHandleBasis;
            float startHandleWeight = cubicWeight * startHandleBasis;
            float endValueWeight = fractionSquared * bezierFraction + endHandleWeight;
            float startValueWeight = complementSquared * complement + startHandleWeight;
            float interpolatedValue = startHandleValue * startHandleWeight;
            interpolatedValue = startComponentValue * startValueWeight + interpolatedValue;
            interpolatedValue = endHandleValue * endHandleWeight + interpolatedValue;
            interpolatedValue = endComponentValue * endValueWeight + interpolatedValue;
            *outputComponent = interpolatedValue;
        } else {
            // Phase 2: time is linear; classify value controls independently.
            float startComponentValue = *startValue;
            float endComponentValue = *endValue;
            float startHandleValue = *outgoingValueOffset;
            float endHandleValue = *incomingValueOffset;
            float valueTolerance = relativeTolerance * (float)__fabs((double)(endComponentValue - startComponentValue));
            float valueControl1 = startComponentValue + startHandleValue;
            float valueControl2 = endHandleValue + endComponentValue;
            int valueIsLinear = 0;

            if (valueTolerance < minimumTolerance) {
                valueTolerance = minimumTolerance;
            }
            float middleValueStep = valueControl2 - valueControl1;
            float firstValueStep = valueControl1 - startComponentValue;
            if ((float)__fabs((double)((endComponentValue - valueControl2) - middleValueStep)) > valueTolerance) {
                valueIsLinear = 0;
            } else {
                valueIsLinear = !((float)__fabs((double)(middleValueStep - firstValueStep)) > valueTolerance);
            }

            if (!valueIsLinear) {
                // Reuse Vec2_Hermite for one scalar lane: y is zero bits.
                // Stack order: {end tangent,end,start tangent,start,result}.
                // Retain integer zero stores and the call's argument order.
                struct {
                    float value;
                    int zeroBits;
                } hermiteLanes[5];
                *(int*)&hermiteLanes[3].value = 0;
                *(int*)&hermiteLanes[2].value = 0;
                *(int*)&hermiteLanes[1].value = 0;
                *(int*)&hermiteLanes[0].value = 0;
                hermiteLanes[3].zeroBits = 0;
                hermiteLanes[3].value = *startValue;
                hermiteLanes[2].zeroBits = 0;
                hermiteLanes[2].value = *outgoingValueOffset;
                hermiteLanes[1].zeroBits = 0;
                hermiteLanes[1].value = *endValue;
                hermiteLanes[0].zeroBits = 0;
                hermiteLanes[0].value = *incomingValueOffset;
                Vec2_Hermite(&hermiteLanes[4], &hermiteLanes[3], &hermiteLanes[2], &hermiteLanes[1], &hermiteLanes[0], fraction);
                *outputComponent = hermiteLanes[4].value;
            } else {
                *outputComponent = (one - fraction) * *startValue + fraction * *endValue;
            }
        }

        outputComponent++;
        outgoingTimeOffset++;
        incomingTimeOffset++;
        incomingValueOffset++;
        endValue++;
        outgoingValueOffset++;
        startValue++;
    }
    return 1;
}

} // extern "C"
