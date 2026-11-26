// Three-component animation interpolation. Track evaluators (gtl_animation_node_evaluate.cpp,
// gtl_animation_model_matrices.cpp, gtl_animation_node_evaluation.cpp, gtl_animation_transform_channels.cpp, gtl_texture_animation_matrices.cpp) select keys and publish absolute
// start/current/end times in s_animationSegmentStartTime, s_animationSampleTime,
// and s_animationSegmentEndTime immediately before dispatch.
// Mod lookup: time-handle inversion is the nonflat branch; value interpolation
// is Bezier, Hermite, or linear below. Shared pool constants retain ABI labels.
// Key schema (16 native floats, 64 bytes; same time unit as the calling track):
// [0..2] incoming time offsets, [3..5] incoming value tangents,
// [6..8] outgoing time offsets, [9..11] outgoing value tangents,
// [12..14] XYZ values, [15] key time. Only a's outgoing and b's incoming
// handles are consumed. XYZ units/space belong to the track, not this adapter.
// Value handles are offsets from endpoint values (Bezier control = value+handle),
// passed unchanged as Hermite tangents on that branch. No duration conversion
// makes these derivatives per second. Time offsets use the track's own clock.

// Embed a scalar in the X lane of Vec2_Hermite. The Y lane contains integer
// zero BITS (+0.0f), never a numeric integer-to-float conversion. Output Y is
// discarded. Keep the four input initializations and call order below.
struct ScalarHermitePair {
    float value;
    int zeroComponentBits;
};

extern "C" {

extern float s_animationSegmentStartTime;
extern float s_animationSampleTime;
extern float s_animationSegmentEndTime;
extern float kFloat_1_GtlModel;
extern float kCurveRelativeSpacingTolerance;
extern float kCurveMinimumSpacingTolerance;
extern float kBezierCubicCoefficient;
// Roles (original scalar pool): C4=1, CC=0.01 relative flatness tolerance,
// D0=0.0001 absolute tolerance floor, D8=3 Bernstein coefficient. Increasing
// either tolerance admits more curves to Hermite/linear paths; it is a change
// of approximation policy, not just speed. Safe tuning ranges are unestablished.
// Preserve float loads, double fabs conversions and expression/evaluation order.

extern float gtl_Curve_FindBezierParameter(float x0, float x1, float x2, float x3, float t);
extern void Vec2_Hermite(void* dst, const void* p0, const void* startTangent, const void* endPoint, const void* t1,
                         float t);

// Requires two complete keys and three writable output floats, nonoverlapping
// with either key. t is normalized segment progress, normally [0,1]; no clamp.
// Returns 1 after writing XYZ; no allocation, validation or recoverable failure.
// Time-flatness compares handle spacings, then value-flatness chooses Hermite
// vs linear. A curved time axis solves against the GLOBAL absolute time triple,
// whereas the other paths use t. Callers must refresh that triple for this
// segment and serialize evaluations; replacing it with t changes the curve.
int gtl_Animation_InterpolateBezierVec3(float* out, const float* a, const float* b, float t) {
    float bernsteinThree = kBezierCubicCoefficient;
    float absoluteTolerance = kCurveMinimumSpacingTolerance;
    float relativeTolerance = kCurveRelativeSpacingTolerance;
    float endValueTangent;
    float endValue;
    float startValue;
    unsigned int component;
    const float* endKey = b;
    float* componentOutput = out;
    const float* startTimeHandle = a + 6;
    const float* endTimeHandle = endKey;
    const float* endValueHandle = endKey + 3;
    const float* endValueComponent = endKey + 12;
    const float* startValueHandle = a + 9;
    const float* startValueComponent = a + 12;
    const float* startKey = a;
    float one = kFloat_1_GtlModel;

    for (component = 0; component < 3; component++) {
        float incomingTimeOffset, outgoingTimeOffset, firstTimeControl, endTime, startTime, secondTimeControl,
            timeTolerance;
        startTime = startKey[15];
        endTime = endKey[15];
        outgoingTimeOffset = *startTimeHandle;
        incomingTimeOffset = *endTimeHandle;
        firstTimeControl = startTime + outgoingTimeOffset;
        secondTimeControl = incomingTimeOffset + endTime;
        timeTolerance = relativeTolerance * (float)__fabs((double)(endTime - startTime));
        int timeHandlesLinear = 0;

        if (timeTolerance < absoluteTolerance) {
            timeTolerance = absoluteTolerance;
        }
        float middleTimeSpacing = secondTimeControl - firstTimeControl;
        float firstTimeSpacing = firstTimeControl - startTime;
        if ((float)__fabs((double)((endTime - secondTimeControl) - middleTimeSpacing)) > timeTolerance) {
            timeHandlesLinear = 0;
        } else {
            timeHandlesLinear = !((float)__fabs((double)(middleTimeSpacing - firstTimeSpacing)) > timeTolerance);
        }

        if (!timeHandlesLinear) {
            endValueTangent = *endValueHandle;
            endValue = *endValueComponent;
            float startValueTangent = *startValueHandle;
            startValue = *startValueComponent;
            float curveParameter = gtl_Curve_FindBezierParameter(
                s_animationSegmentStartTime, s_animationSegmentStartTime + *startTimeHandle,
                s_animationSegmentEndTime + *endTimeHandle, s_animationSegmentEndTime, s_animationSampleTime);
            float oneMinusParameter = one - curveParameter;
            float parameterSquared = curveParameter * curveParameter;
            float oneMinusSquared = oneMinusParameter * oneMinusParameter;
            float endWeightBase = parameterSquared * oneMinusParameter;
            float startWeightBase = oneMinusSquared * curveParameter;
            float endTangentWeight = bernsteinThree * endWeightBase;
            float startTangentWeight = bernsteinThree * startWeightBase;
            float endValueWeight = parameterSquared * curveParameter + endTangentWeight;
            float startValueWeight = oneMinusSquared * oneMinusParameter + startTangentWeight;
            float componentValue = startValueTangent * startTangentWeight;
            componentValue = startValue * startValueWeight + componentValue;
            componentValue = endValueTangent * endTangentWeight + componentValue;
            componentValue = endValue * endValueWeight + componentValue;
            *componentOutput = componentValue;
        } else {
            float startValue = *startValueComponent;
            float endValue = *endValueComponent;
            float startValueTangent = *startValueHandle;
            float endValueTangent = *endValueHandle;
            float valueTolerance = relativeTolerance * (float)__fabs((double)(endValue - startValue));
            float firstValueControl = startValue + startValueTangent;
            float secondValueControl = endValueTangent + endValue;
            int valueHandlesLinear = 0;

            if (valueTolerance < absoluteTolerance) {
                valueTolerance = absoluteTolerance;
            }
            float middleValueSpacing = secondValueControl - firstValueControl;
            float firstValueSpacing = firstValueControl - startValue;
            if ((float)__fabs((double)((endValue - secondValueControl) - middleValueSpacing)) > valueTolerance) {
                valueHandlesLinear = 0;
            } else {
                valueHandlesLinear =
                    !((float)__fabs((double)(middleValueSpacing - firstValueSpacing)) > valueTolerance);
            }

            if (!valueHandlesLinear) {
                // Slots: end tangent, end value, start tangent, start value,
                // result. Reversed storage is independent of Hermite arg order.
                ScalarHermitePair hermite[5];
                *(int*)&hermite[3].value = 0;
                *(int*)&hermite[2].value = 0;
                *(int*)&hermite[1].value = 0;
                *(int*)&hermite[0].value = 0;
                hermite[3].zeroComponentBits = 0;
                hermite[3].value = *startValueComponent;
                hermite[2].zeroComponentBits = 0;
                hermite[2].value = *startValueHandle;
                hermite[1].zeroComponentBits = 0;
                hermite[1].value = *endValueComponent;
                hermite[0].zeroComponentBits = 0;
                hermite[0].value = *endValueHandle;
                Vec2_Hermite(&hermite[4], &hermite[3], &hermite[2], &hermite[1], &hermite[0], t);
                *componentOutput = hermite[4].value;
            } else {
                *componentOutput = (one - t) * *startValueComponent + t * *endValueComponent;
            }
        }

        endTimeHandle++;
        endValueHandle++;
        endValueComponent++;
        startValueHandle++;
        startValueComponent++;
        componentOutput++;
        startTimeHandle++;
    }
    return 1;
}

} // extern "C"
