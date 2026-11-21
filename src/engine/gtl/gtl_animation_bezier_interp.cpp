// gtl animation: evaluate four independent Bezier value channels between two keys.
// Mod lookup: edit key value/handle arrays below for curve shape; time spacing
// comes from the track evaluator in gtl_animation_node_evaluate.cpp/gtl_animation_model_matrices.cpp. Those callers set the
// shared time globals before dispatching here. No allocation or gameplay knobs.
// Translation unit 0x803E0BCC-0x803E0EE8 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

struct BezierKey {
    // Native float32 key, 0x54 bytes, four channels; handle components are
    // OFFSETS from the key, not absolute control points or slopes.
    float incomingTimeOffset[4];  // +00: end control time = b.time + this
    float incomingValueOffset[4]; // +10: end control value = b.value + this
    float outgoingTimeOffset[4];  // +20: start control time = a.time + this
    float outgoingValueOffset[4]; // +30: start control value = a.value + this
    float value[4];               // +40: endpoint value, in track channel units
    float time;                   // +50: track time; no seconds conversion here
};

struct Vec2 {
    float x;
    float y;
};

extern float kFloat_1_GtlModel; // unit weight (1)
extern float kCurveRelativeSpacingTolerance; // 0.01f: relative curvature tolerance, shared by interpolators
extern float kCurveMinimumSpacingTolerance; // 0.0001f: minimum absolute curvature tolerance, same pool
extern float kBezierCubicCoefficient; // cubic Bernstein coefficient (3)
extern float s_animationSegmentStartTime; // selected segment's start time
extern float s_animationSampleTime; // requested absolute track time
extern float s_animationSegmentEndTime; // selected segment's end time

extern float gtl_Curve_FindBezierParameter(float x0, float x1, float x2, float x3, float t);
extern void* Vec2_Hermite(void* dst, const void* p0, const void* startTangent, const void* endPoint, const void* t1, float t);

// Write four float32 values to required out[4]; return status 1 on every path.
// a/b are borrowed complete keys; t is normalized segment time (normally 0..1),
// not absolute time. No clamping, validation or allocation. For predictable
// evaluation out must not overlap either key; a and b may refer to the same key.
// Nonlinear time uses shared absolute times above, so calls are not reentrant;
// callers must keep them consistent with a/b and t, including extrapolation.
// Per channel: nonlinear time -> solve cubic time then evaluate cubic value;
// linear time + nonlinear value -> Hermite; both linear -> endpoint blend.
// Curvature is linear when both adjacent-control-spacing differences are within
// max(relativeTolerance*abs(endpointSpan), minimumTolerance). Changing either
// tolerance affects time AND value decisions here and sibling interpolators.
// Larger tolerances select the Hermite/linear shortcuts more often. Constants
// are float32 at 805E20CC/D0; minimum tolerance is used in each tested channel's
// time or value units. The relative scalar is dimensionless. No safe edit range
// is established; preserve the two distinct settings rather than merging them.
// Keep float operations, fabs double conversions and accumulation order intact.
int gtl_Animation_InterpolateBezierVec4(float* out, BezierKey* a, BezierKey* b, float t) {
    unsigned int i;
    float* pOut = out;
    float three = kBezierCubicCoefficient;
    float minTol = kCurveMinimumSpacingTolerance;
    float tolScale = kCurveRelativeSpacingTolerance;
    float* startTimeHandle = a->outgoingTimeOffset;
    float* endTimeHandle = b->incomingTimeOffset;
    float* endValueHandle = b->incomingValueOffset;
    float* endValue = b->value;
    float* startValueHandle = a->outgoingValueOffset;
    float* startValue = a->value;
    float one = kFloat_1_GtlModel;

    for (i = 0; i < 4; i++) {
        float dt;
        float tol;
        float p0;
        float p1;
        float p2;
        float p3;
        float d1;
        float d2;
        int linearTime;
        int linearValue;

        // p0..p3 first hold time control points, then value control points below.
        p0 = a->time;
        p1 = a->time + *startTimeHandle;
        p2 = b->time + *endTimeHandle;
        p3 = b->time;
        dt = b->time - a->time;
        tol = tolScale * (float)__fabs((double)dt);
        if (tol < minTol) {
            tol = minTol;
        }

        float pm = p1 - p0;
        d1 = (p3 - p2) - (p2 - p1);
        linearTime = 0;
        if ((float)__fabs((double)d1) > tol) {
            linearTime = 0;
        } else {
            d2 = (p2 - p1) - pm;
            linearTime = !((float)__fabs((double)d2) > tol);
        }

        if (!linearTime) {
            float x1 = s_animationSegmentStartTime + *startTimeHandle;
            float x2 = s_animationSegmentEndTime + *endTimeHandle;
            float incomingValue = *endValueHandle;
            float endPoint = *endValue;
            float outgoingValue = *startValueHandle;
            float startPoint = *startValue;
            float s = gtl_Curve_FindBezierParameter(s_animationSegmentStartTime, x1, x2, s_animationSegmentEndTime, s_animationSampleTime);
            float u = one - s;
            float s2 = s * s;
            float u2 = u * u;
            float w2 = three * (s2 * u);
            float w1 = three * (u2 * s);
            float w0 = u2 * u + w1;
            float w3 = s2 * s + w2;
            // Fold endpoint contributions from the relative value handles into
            // w0/w3. This is the offset-handle form of the Bernstein polynomial.
            float v = outgoingValue * w1;

            v = startPoint * w0 + v;
            v = incomingValue * w2 + v;
            v = endPoint * w3 + v;
            *pOut = v;
        } else {
            p0 = *startValue;
            p3 = *endValue;
            p1 = p0 + *startValueHandle;
            dt = p3 - p0;
            p2 = *endValueHandle + p3;
            tol = tolScale * (float)__fabs((double)dt);
            if (tol < minTol) {
                tol = minTol;
            }

            float pmv = p1 - p0;
            d1 = (p3 - p2) - (p2 - p1);
            linearValue = 0;
            if ((float)__fabs((double)d1) > tol) {
                linearValue = 0;
            } else {
                d2 = (p2 - p1) - pmv;
                linearValue = !((float)__fabs((double)d2) > tol);
            }

            if (!linearValue) {
                // Hermite receives value offsets verbatim as tangents. Its
                // unused Y channel stays zero; do not rescale handles by three.
                Vec2 res;
                Vec2 q0 = {};
                Vec2 q1 = {};
                Vec2 q2 = {};
                Vec2 q3 = {};
                q0.x = *startValue;
                q1.x = *startValueHandle;
                q2.x = *endValue;
                q3.x = *endValueHandle;
                Vec2_Hermite(&res, &q0, &q1, &q2, &q3, t);
                *pOut = res.x;
            } else {
                *pOut = *startValue * (one - t) + t * *endValue;
            }
        }

        pOut++;
        startTimeHandle++;
        startValueHandle++;
        startValue++;
        endTimeHandle++;
        endValueHandle++;
        endValue++;
    }

    return 1;
}

} // extern "C"
