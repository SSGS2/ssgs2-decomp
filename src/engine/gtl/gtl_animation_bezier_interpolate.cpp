// Scalar animation interpolation (Bezier/Hermite); edit curve shape here, not
// the animation update cadence. Callers in gtl_animation_node_evaluate.cpp/gtl_animation_model_matrices.cpp/gtl_animation_node_evaluation.cpp choose
// adjacent keys and publish the three time globals below before dispatching.
// Six-float key schema (24 bytes, native floats), for both a and b:
//   [0] incoming time-handle delta, [1] incoming value-handle delta,
//   [2] outgoing time-handle delta, [3] outgoing value-handle delta,
//   [4] scalar value, [5] key time in the caller's animation time domain.
// Handles are offsets from their endpoint, not absolute control points.
// Value units depend on the track (no game-distance or angle conversion here).
// The normal interval is a.time < b.time and normalized t in [0,1]; callers
// handle endpoint keys. This routine neither clamps t nor validates the interval.
// Lookup: nonlinear time spacing -> FindBezierParameter; linear time spacing ->
// Hermite when value spacing is nonlinear, otherwise straight value interpolation.

extern "C" {

extern float s_animationSegmentStartTime;    // interval start time, set by animation evaluator
extern float s_animationSampleTime;          // absolute sample time, set by animation evaluator
extern float s_animationSegmentEndTime;      // interval end time, set by animation evaluator
extern float kFloat_1_GtlModel;              // 1.0f: complement weights
extern float kCurveRelativeSpacingTolerance; // 0.01f: relative linear-spacing tolerance
extern float kCurveMinimumSpacingTolerance;  // 0.0001f: absolute tolerance floor, in tested axis units
extern float kBezierCubicCoefficient;        // 3.0f: cubic Bernstein coefficient

extern float gtl_Curve_FindBezierParameter(float x0, float x1, float x2, float x3, float t);
extern void* Vec2_Hermite(void* dst, const void* p0, const void* startTangent, const void* endPoint, const void* t1,
                          float t);

#pragma opt_lifetimes off
// Required borrowed a/b each expose all six floats; out exposes one writable
// float and is written only after input calculations. Returns 1 unconditionally.
// No allocations. The mutable time globals make this API non-reentrant: publish
// all three for the same key interval/sample and serialize interpolation calls.
// The explicit t is normalized; nonlinear-time inversion uses the ABSOLUTE
// global sample time instead. Keep these two inputs consistent when calling.
// Spacing is considered linear if adjacent control-point differences agree
// within max(0.01*abs(endpoint span),0.0001), first in time then in value.
// Larger tolerances route more curves through cheaper Hermite/linear branches;
// these are shared pools, not independent tuning settings. Safe mod ranges are
// unestablished. Preserve float operation/order, double fabs conversions and
// opt_lifetimes: algebraic simplification may change rounding and branch choice.
int gtl_Animation_InterpolateBezierFloat(float* out, const float* a, const float* b, float t) {
    int hasLinearSpacing;
    const float* startOutgoingTime = a + 2;
    const float* endIncomingValue = b + 1;
    const float* endValue = b + 4;
    const float* startOutgoingValue = a + 3;
    const float* startValue = a + 4;
    float one;
    float three;
    float endValueDelta;
    float endValueCopy;
    float startValueDelta;
    float startValueCopy;
    float startTime;
    float endTime;
    float outgoingTimeDelta;
    float incomingTimeDelta;
    float spacingTolerance;
    float span;
    float timeControl0;
    float timeControl1;
    float timeControl2;
    float timeControl3;
    float firstSpacing;
    float middleSpacing;
    float lastSpacing;
    float curveParameter;
    float parameterSquared;
    float complement;
    float complementSquared;
    float endCubicTerm;
    float startCubicTerm;
    float endHandleWeight;
    float startHandleWeight;
    float endValueWeight;
    float startValueWeight;
    float interpolatedValue;
    float valueControl0;
    float valueControl1;
    float valueControl2;
    float valueControl3;
    float hermiteResult[2];

    // Phase 1: time-axis control points and equal-spacing test.
    startTime = a[5];
    endTime = b[5];
    span = endTime - startTime;
    outgoingTimeDelta = a[2];
    incomingTimeDelta = b[0];
    timeControl0 = startTime;
    timeControl1 = startTime + outgoingTimeDelta;
    timeControl2 = incomingTimeDelta + endTime;
    timeControl3 = endTime;
    one = kFloat_1_GtlModel;
    three = kBezierCubicCoefficient;
    float relativeSpacingTolerance = kCurveRelativeSpacingTolerance;

    spacingTolerance = relativeSpacingTolerance * (float)__fabs((double)span);
    float absoluteSpacingTolerance = kCurveMinimumSpacingTolerance;
    if (spacingTolerance < absoluteSpacingTolerance) {
        spacingTolerance = absoluteSpacingTolerance;
    }

    middleSpacing = timeControl2 - timeControl1;
    lastSpacing = timeControl3 - timeControl2;
    firstSpacing = timeControl1 - timeControl0;
    if ((float)__fabs((double)(lastSpacing - middleSpacing)) > spacingTolerance) {
        hasLinearSpacing = 0;
    } else {
        hasLinearSpacing = !((float)__fabs((double)(middleSpacing - firstSpacing)) > spacingTolerance);
    }

    if (hasLinearSpacing == 0) {
        // Phase 2a: invert time, then evaluate value using that curve parameter.
        float timeHandle1 = s_animationSegmentStartTime + startOutgoingTime[0];
        endValueDelta = endIncomingValue[0];
        float timeHandle2 = s_animationSegmentEndTime + b[0];
        endValueCopy = endValue[0];
        startValueDelta = startOutgoingValue[0];
        startValueCopy = startValue[0];
        curveParameter = gtl_Curve_FindBezierParameter(s_animationSegmentStartTime, timeHandle1, timeHandle2,
                                                       s_animationSegmentEndTime, s_animationSampleTime);
        complement = one - curveParameter;
        parameterSquared = curveParameter * curveParameter;
        complementSquared = complement * complement;
        endCubicTerm = parameterSquared * complement;
        startCubicTerm = complementSquared * curveParameter;
        endHandleWeight = three * endCubicTerm;
        startHandleWeight = three * startCubicTerm;
        endValueWeight = parameterSquared * curveParameter + endHandleWeight;
        startValueWeight = complementSquared * complement + startHandleWeight;
        interpolatedValue = startValueDelta * startHandleWeight;
        interpolatedValue = startValueCopy * startValueWeight + interpolatedValue;
        interpolatedValue = endValueDelta * endHandleWeight + interpolatedValue;
        interpolatedValue = endValueCopy * endValueWeight + interpolatedValue;
        out[0] = interpolatedValue;
    } else {
        // Phase 2b: time is linear; test the value axis using the same policy.
        valueControl0 = startValue[0];
        valueControl1 = startValue[0] + startOutgoingValue[0];
        valueControl2 = endIncomingValue[0] + endValue[0];
        valueControl3 = endValue[0];
        spacingTolerance = relativeSpacingTolerance * (float)__fabs((double)(endValue[0] - startValue[0]));
        if (spacingTolerance < absoluteSpacingTolerance) {
            spacingTolerance = absoluteSpacingTolerance;
        }

        lastSpacing = valueControl3 - valueControl2;
        middleSpacing = valueControl2 - valueControl1;
        firstSpacing = valueControl1 - valueControl0;
        if ((float)__fabs((double)(lastSpacing - middleSpacing)) > spacingTolerance) {
            hasLinearSpacing = 0;
        } else {
            hasLinearSpacing = !((float)__fabs((double)(middleSpacing - firstSpacing)) > spacingTolerance);
        }

        if (hasLinearSpacing == 0) {
            float hermiteStart[2] = {0.0f, 0.0f};
            float hermiteStartTangent[2] = {0.0f, 0.0f};
            float hermiteEnd[2] = {0.0f, 0.0f};
            float hermiteEndTangent[2] = {0.0f, 0.0f};
            // Embed the scalar in Vec2.x; the synthetic second axis stays zero.
            hermiteEnd[0] = endValue[0];
            hermiteStart[0] = startValue[0];
            hermiteStartTangent[0] = startOutgoingValue[0];
            hermiteEndTangent[0] = endIncomingValue[0];
            Vec2_Hermite(hermiteResult, hermiteStart, hermiteStartTangent, hermiteEnd, hermiteEndTangent, t);
            out[0] = hermiteResult[0];
        } else {
            out[0] = (one - t) * startValue[0] + t * endValue[0];
        }
    }
    return 1;
}
#pragma opt_lifetimes reset

} // extern "C"
