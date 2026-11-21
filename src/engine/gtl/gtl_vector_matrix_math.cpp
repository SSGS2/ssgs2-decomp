// Shared vector/matrix library, widescreen camera projection and viewport mapping;
// also contains input-device enumeration/allocation and two platform wrappers.
// Mod lookup (scalar definitions: u80438310_data.cpp):
// - MtxPerspectiveFovRH/LH take VERTICAL fovy in radians and width/height aspect
//   from their caller. Horizontal scale = cot(fovy/2)/aspect; larger aspect widens
//   horizontal coverage at fixed vertical FOV. No global 4/3 or framebuffer size
//   lives here. Trace the caller (main camera vs secondary ribbon) for that knob.
// - Clip distances: nearz/farz in view-distance units. RH looks along -Z, LH +Z;
//   keep 0 < nearz < farz, nonzero aspect and 0 < fovy < pi. No runtime validation.
// - Projection_SetDepthConvention copies one uint32 into shared storage; callers
//   must initialize it before ANY projection builder. Reset clears the pointer;
//   projection after reset without reinitialization dereferences null.
// - Legacy depth mode 1 changes ONLY m[10] in perspective or m[14] in ortho.
//   It does not implement a conventional full symmetric-depth conversion. Keep
//   these coefficients with the backend depth interpretation; do not "correct"
//   the other coefficient as part of an aspect change.
// - WView_BuildViewportTransform consumes uint32 x/y/width/height + float near/far;
//   depth divisor kViewportDepthMaximum = 16777215 (24-bit depth). Its unusual
//   translation/homogeneous terms are literal legacy behavior, documented below.
// - Vec3_ProjectToViewport/UnprojectFromViewport use the same record through a
//   DIFFERENT ortho/inverse composition. Keep origin/extents, depth mode, view,
//   world and projection together. These functions do not set a GX scissor.
// Geometry contracts: all void* math operands refer to contiguous float32 arrays,
// Vec2/3/4 lengths 2/3/4, quaternion XYZW length 4, matrix row-major length 16.
// Row-vector convention v'=v*M, translation indices 12..14. All pointers required
// unless explicitly nullable below. No allocation/retention in math routines;
// outputs belong to callers, and void* returns repeat destination, not status.
// Use disjoint buffers unless exact in-place operation is documented; do not
// infer partial-overlap support from a signature. Interpolation parameters are
// dimensionless and unclamped; vectors must use a common caller-defined space.
// Preserve paired-single rounding/subnormal stores, explicit fused operations,
// volatile access order, optimization pragmas and the legacy inverse anomaly.
// No frame rate, model-load, texture-format, or presentation-resolution policy.
#pragma push
// Translation unit 0x80438310-0x8043B91C (vector/matrix/projection math, 76 functions; shared .sdata2 constants show it is one original file).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"
#include "fp_fused.h"
#include "classes/GtlInputDevice.h"

// Paired-single operations act on adjacent floats; matrix indices are row * 4 + column.
typedef __vec2x32float__ PVec2;

enum InputEnumerationConstants {
    kControllerChannelCount = 4,
    kInputProviderCount = 4,
    kMaxEnumeratedDevices = 16,
    kFactoryDeviceTypeCount = 4,
    kProviderWiiPointer = 0xB,
    kProviderWiiNunchuk = 0xC,
    kProviderWiiClassic = 0xD,
    kProviderGameCube = 0xE,
    kDeviceFilterAll = 0,
    kDeviceFilterPad = 2,
    kDeviceFilterPointer = 3,
    kDeviceFilterKindRange = 4,
    kFirstExactDeviceKind = 0x11,
    kDeviceKindPad = 0x12,
    kDeviceKindAlternatePad = 0x1A,
    kDeviceKindPointer = 0x13,
    kFirstRangeDeviceKind = 0x14,
    kEndRangeDeviceKind = 0x18,
    kEndExactDeviceKind = 0x1D
};

// Only the provider slots used here are exposed; no C++ vtable is generated.
struct InputProviderVtable {
    void* reservedBeforeChannelQuery[6];
    int (*queryChannelStatus)(void*, int);
    void* reservedBeforeKindQuery[2];
    unsigned int (*getDeviceKind)(void*);
};
struct InputProvider { InputProviderVtable* vtable; };

enum FloatRepresentationMasks {
    kFloatExponentMask = 0x7F800000U,
    kFloatSignMask = 0x80000000U
};
// Neutral name: mode 1 changes only one coefficient, not full depth remapping.
enum ProjectionDepthConvention { kLegacyProjectionDepthMode1 = 1 };
static const float kViewportDepthMaximum = 16777215.0f; // 2^24-1 depth-code endpoint.

// Descriptor namespaces (word 1):
// EnumerateDevices: provider 0xB=Wii pointer, 0xC=Nunchuk, 0xD=Classic, 0xE=GC.
// Create: factory selector in [0,4). Channel is word 2 and is in [0,4).
// The ordinal similarity of those four providers is not a conversion rule.
struct DriverEntry {
    // This is a three-word ABI overlay, not a universal device descriptor:
    // EnumerateDevices writes {0, provider ID (0xB..0xE), channel (0..3)}.
    // Create checks word 1 as 0..3, then Ctor copies ONLY words 0 and 1
    // into provider/state. No conversion between these contracts occurs here;
    // do not pass an enumeration record directly to Create on that assumption.
    unsigned int flag;
    unsigned int descriptorWord1;
    unsigned int index;
};

extern "C" {

static int truncate_angle_turns(float value);

extern void* (**g_inputAllocator)(unsigned int);
void gtl_IInputDevice_Ctor(void*, const void*);

// Validate the descriptor, allocate a generic input device, and construct it with the configured allocator.
// destination receives owned storage on success (1); allocation failure stores null (0).
// Invalid arguments return 0 without writing destination. See DriverEntry's ABI caveat.
int gtl_IInputDevice_Create(const DriverEntry* entry, void** destination);

extern float kFloat_0_MatrixMath;
extern float kFloat_0_5_MatrixMath;
extern float kFloat_3_Math;
extern float kFloat_1_MatrixMath;
extern float kTwoPi_AngleWrap;
extern float kInvTwoPi_AngleWrap;
extern float kFloat_4_712389;
extern float kFloat_3_1415927_MatrixMath;
extern float kHalfPi_MatrixMath;
extern float kFloat_0_0000027557319_rotationPolynomial;
extern float kSinSeriesCoefficient7;
extern float kFloat_0_008333333_805E25F4;
extern float kFloat_neg0_16666667_Trig;
extern float kFloat_2_Math;
extern float kFloat_neg1_MatrixMath; // -1.0f
extern float kFloat_5_CatmullRom; // 5.0f, Catmull-Rom coefficient
extern float kFloat_neg3_CatmullRom; // -3.0f, Catmull-Rom coefficient
extern float kFloat_4_CatmullRom; // 4.0f, Catmull-Rom coefficient
extern float kFloat_0_MatrixAccumulation; // 0.0f
extern double kDouble_SignedIntConversionBias_MatrixMath; // 4503601774854144.0, signed-int conversion bias
extern float kSqrtZeroLowerBound; // -1e-6f, small-value lower bound
extern float kSqrtZeroUpperBound; // 1e-6f, small-value upper bound
extern float kQuaternionTraceThreshold; // 0.01f, quaternion trace threshold
extern float kFloat_0_25_Quaternion; // 0.25f
void Mtx44_TransformVec4(float* out, const float* weights, const float* matrix);
extern unsigned int kMatrixDefaultUpVector[3]; // default up-vector bits
float Vec4_Length(void* v);
void* Vec3_Normalize(void* dst, const void* src);
void Vec3_Cross(void* dst, const void* a, const void* b);
void Vec4_Scale(void* dst, const void* src, float s);
void PSMTX44Transpose(void* dst, const void* src);
void Mtx44_TransformPointPerspective(void* dst, const void* v, const void* m);
void Mtx44_FromQuaternion(void* dst, const void* q);
void Mtx44_InvertLegacyCofactors(void* dst, float* det_out, const void* src);
void* PSMTX44Concat(void* dst, const void* a, const void* b);

// Interpolate two components between startPoint and endPoint using the surrounding control points.
void* Vec2_CatmullRom(void* destination, const void* previousPoint, const void* startPoint, const void* endPoint, const void* nextPoint, float interpolation);

#pragma push
#pragma optimization_level 2
// Blend count 4x4 matrices with scalar weights, accumulating two matrices per iteration.
// data contains count consecutive 16-float matrices; weights contains count scalars.
// Each PVec2 is {m[2*j], m[2*j+1]}: sum0..sum7 cover flat indices
// {0,1}, {2,3}, ... {14,15}. Assigning a scalar to w0 broadcasts to BOTH lanes.
// row is actually a pointer into this packed stream: row[0..7] is matrix A,
// row[8..15] is matrix B. mp += 16 advances two matrices, not sixteen floats.
// The odd tail advances eight pairs. count == 0 writes a zero matrix.
void Mtx44_WeightedSum(register void* out, register const void* data, register const void* weights, register unsigned int count);
#pragma pop

// Build the viewport transform, including the original 24-bit depth conversion and translation formula.
void WView_BuildViewportTransform(void* destination, void* viewport);

// Replace orientation from a forward vector while preserving row lengths and translation; mode zero reuses source up.
float* Matrix_AlignForwardPreservingScaleAndTranslation(float* destination, float* sourceMatrix, float* forwardDirection, int useDefaultUp);

// Rotation builder map, row-major storage and row-vector application v' = v*M:
// X = [1 0 0 0; 0 c s 0; 0 -s c 0; 0 0 0 1]
// Y = [c 0 -s 0; 0 1 0 0; s 0 c 0; 0 0 0 1]
// Z = [c s 0 0; -s c 0 0; 0 0 1 0; 0 0 0 1].
// All builders use s=P(wrap(theta)), c=P(wrap(theta+pi/2)), with
// P(x)=x+k3*x^3+k5*x^5+k7*x^7+k9*x^9. wrap first reduces by whole turns,
// then folds into [-pi/2,pi/2]. Powers and additions keep the literal evaluation
// order below: replacing them with sin/cos or regrouping changes game rounding.
#pragma push
#pragma optimization_level 2
// Build an X-axis rotation in radians using the original range reduction and sine polynomial.
void Mtx44_SetRotationX(float* out, float angleRadians);
#pragma pop

#pragma push
#pragma optimization_level 4
// Build a Y-axis rotation in radians using the original range reduction and sine polynomial.
void Mtx44_SetRotationY(float* out, float angleRadians);
#pragma pop

#pragma push
#pragma optimization_level 4
// Build a Z-axis rotation in radians using the original range reduction and sine polynomial.
void Mtx44_SetRotationZ(float* out, float angleRadians);
#pragma pop

// Normalize the supplied axis, then build its rotation matrix for an angle in radians.
void* Mtx44_SetRotationAxisAngle(void* destination, const void* axis, float angleRadians);

// Compute the legacy 4x4 inverse formula, preserving its nonstandard C22 cofactor.
// A zero determinant leaves BOTH outputs untouched. Source/destination may alias;
// determinantOut is optional, but must not overlap source/destination matrix data.
// For nonsingular input, only out[10] differs algebraically from a conventional
// inverse: the difference is (m31-m30)*m01*m13/det (zero-based matrix indices).
// This was checked by exact polynomial expansion of all 16 cofactors and det;
// keep the anomaly unless intentionally changing game behavior in a separate edit.
void Mtx44_InvertLegacyCofactors(void* destination, float* determinantOut, const void* sourceMatrix);

// Compose optional scaling and rotation about their centers, followed by translation, in the original order.
// Inputs are borrowed Vec3 centers/scale/translation and xyzw quaternions; null omits
// that component. Row-vector product order is Sc^-1 * Sr^-1 * S * Sr * Sc *
// Rc^-1 * R * Rc * T. destination must not overlap input component storage: identity
// is written before components are read. The returned pointer is destination.
// Nonnull scalingRotation must yield a finite, NONSINGULAR matrix through
// Mtx44_FromQuaternion, with finite nonzero determinant in the inverse's actual
// floating-point evaluation; no quaternion normalization/validation is done here.
// Mtx44_InvertLegacyCofactors leaves output untouched at zero determinant, but
// this builder's inverseScalingRotation scratch has no initial value and is
// concatenated unconditionally. Singular input therefore has no defined fallback
// or failure status. A unit rotation quaternion is the intended orientation
// input; merely supplying four finite components does not prove nonsingularity.
// Source callers in gtl_transform_frustum_stopwatch_input.cpp pass null scalingRotation, avoiding this inverse.
void* Mtx44_BuildTransformation(void* destination, const void* scalingCenter, const void* scalingRotation, const void* scale, const void* rotationCenter, const void* rotation, const void* translation);

// Build the view basis from eye/target/up, transpose it, then transform the negated eye into translation.
void* MTXLookAt(void* destination, const void* eyePosition, const void* targetPosition, const void* upDirection);

extern unsigned int inputProviderPriorityOrder[4]; // provider priority order (four provider IDs)
void* InputDriver_GetByProviderId(unsigned int id);

#pragma push
#pragma use_lmw_stmw on
// Enumerate four controller channels in provider priority order, filter device kinds, and invoke the callback.
// Callback records are borrowed stack entries, valid only during the call; copy to retain.
// This emits provider/channel records, not the factory's accepted 0..3 descriptor word.
int InputDriver_EnumerateDevices(int deviceFilter, void (*callback)(void*, int), int callbackArgument);
#pragma pop

float Math_SquareRootApprox(float value);

// Convert a rotation matrix to quaternion components, choosing the largest diagonal when the trace is small.
void Quat_FromMatrix44(void* destination, void* sourceMatrix);

extern unsigned int s_projectionDepthConventionStorage; // leading word stores the copied projection depth convention
extern unsigned int* g_projectionDepthConvention;
extern unsigned char s_projectionDepthConventionInitialized; // projection depth convention initialized flag

// Copy a caller-provided projection depth convention into shared storage and mark it initialized.
int Projection_SetDepthConvention(void* depthConvention);

// Multiply two vectors component by component; read all inputs before storing.
void Vec3_MultiplyComponents(float* out, const float* a, const float* b);

// Return 1 only when both components compare exactly equal (no epsilon).
int Vec2_Equals(const float* a, const float* b);

// Compute ((a*b)*c)*d through the existing concatenator and return dst.
void* Mtx44_Concat4(void* dst, const void* a, const void* b, const void* c, const void* d);

// Evaluate plane normal dot point plus plane[3]; no normal normalization is applied.
float gtl_Plane_EvaluatePoint(const float* a, const float* b);

#pragma dont_inline on
#pragma fp_contract off


// Explicit intrinsic keeps the fused single rounding even with fp_contract off.
float fmaf(float a, float b, float c) {
    return __fmadds(a, b, c);
}

// Truncate turns with signed-int saturation; unordered input follows the original minimum-int path.
static int truncate_angle_turns(float value) {
    if (value >= 2147483648.0f) return 2147483647;
    if (!(value >= -2147483648.0f)) return (-2147483647 - 1);
    return (int)value;
}

// Return success for the input module initialization hook.
int InputDriver_InitStub(void);

// The allocator callbacks take the allocation size/pointer in r3.
extern void* (**g_inputAllocator)(unsigned int);
// Allocate input-driver storage through the configured size callback.
void* __nw__Q23gtl25IInputResourceMemOperatorFUl(unsigned int size);
// Release input-driver storage through the configured allocator free callback.
void __dl__Q23gtl25IInputResourceMemOperatorFPv(void* memory);

// Clear the active projection depth convention pointer and its initialization flag.
void Projection_ResetDepthConvention(void);

// Range reduction truncates toward zero; this is the game's polynomial, not libm sine.
float Math_SinApprox(float angle);

// The phase shift rounds to float before range reduction.
float Math_CosApprox(float angle);

// Return x*x + y*y without a square root.
float Vec2_LengthSquared(const void* input);

// Add two-component vectors, allowing dst to equal either input.
void gtl_Vec2_Add(void* dst, const void* first, const void* second);

// Subtract second from first component by component.
void gtl_Vec2_Subtract(void* dst, const void* first, const void* second);

// Multiply both vector components by the scalar scale.
void Vec2_Scale(void* dst, const void* input, float scale);

// The lane sum groups x and z before adding y.
float Vec3_LengthSquared(const void* input);

// Return the 3D dot product with the original fused x/y partial sum.
float Vec3_Dot(const void* first, const void* second);

// y and z are negated fused subtractions, preserving the game's rounding and zero signs.
void Vec3_Cross(void* dst, const void* first, const void* second);

// Add three-component vectors, reading inputs before writing dst.
void Vec3_Add(void* dst, const void* first, const void* second);

// Subtract the second 3D vector from the first.
void Vec3_Subtract(void* dst, const void* first, const void* second);

// Multiply all three components by scale.
void Vec3_Scale(void* dst, const void* input, float scale);

// Multiplication and addition round separately; do not contract to fused multiply-add.
void Vec3_Lerp(void* dst, const void* first, const void* second, float t);

// Fill all three components with the same scalar.
void Vec3_SetAll(void* dst, float value);

// The paired sums are (x*x + w*w) + (z*z + y*y).
float Vec4_LengthSquared(const void* input);

// Multiply all four components, including w, by scale.
void Vec4_Scale(void* dst, const void* input, float scale);

// Compute first + (second-first)*t for four components; t is not clamped.
void Vec4_Lerp(void* dst, const void* first, const void* second, float t);

// Two independent fused chains are summed, rather than a left-to-right dot product.
static void transform_four(float result[4], const float* v, const float* m, float w) {
    float x = v[0], y = v[1], z = v[2];
    for (int col = 0; col < 4; ++col) {
        float xm = x * m[col], ym = y * m[4 + col];
        float xz = fmaf(z, m[8 + col], xm);
        float yw = fmaf(w, m[12 + col], ym);
        result[col] = xz + yw;
    }
}

// Transform row vector v by matrix: out[c] = sum(v[r]*matrix[4*r+c]).
void Mtx44_TransformVec4(float* out, const float* v, const float* matrix);

// This writes four floats even though its input has three components.
void Mtx44_TransformPointToVec4(void* dst, const void* input, const void* matrix);

#pragma push
#pragma dont_inline off
#pragma fp_contract on
// Avoid extending float lifetimes enough to require callee-saved registers.
#pragma scheduling off
// Each call is one single-precision fused multiply-add on CodeWarrior.
static inline float matrix_madd(float a, float b, float c) {
    return a * b + c;
}

// psq_st flushes subnormal outputs even when scalar arithmetic keeps them.
// Work on the representation so NaN payloads and negative zero stay intact.
// Keep the bits in a separate local: updating the union member spills it again.
static inline unsigned int matrix_store_bits(float value) {
    union { float value; unsigned int bits; } word;
    word.value = value;
    unsigned int bits = word.bits;
    if ((bits & kFloatExponentMask) == 0)
        bits &= kFloatSignMask;
    return bits;
}

// Store one float with paired-single-compatible subnormal flushing.
static inline void matrix_store(volatile float* dst, float value) {
    *(volatile unsigned int*)dst = matrix_store_bits(value);
}

// The two partial dot products round independently before the final sum.
static inline float perspective_component(float x, float y, float z,
                                          const float* m, int col) {
    float xm = x * m[col];
    float ym = y * m[4 + col];
    float xz = matrix_madd(z, m[8 + col], xm);
    float yw = matrix_madd(1.0f, m[12 + col], ym);
    return xz + yw;
}

// The reciprocal is rounded before scaling; z is stored before x and y.
void Mtx44_TransformPointPerspective(void* dst, const void* input, const void* matrix);
#pragma pop

// Zero still participates in the fused translation term (including for infinite matrix entries).
void Mtx44_TransformDirection(void* dst, const void* input, const void* matrix);

// Write all sixteen entries of a 4x4 identity matrix.
void PSMTX44Identity(void* dst);

// Preserve the original pairwise load/store schedule; this is not a buffered transpose.
void PSMTX44Transpose(void* dst, const void* input);

#pragma push
#pragma dont_inline off
#pragma fp_contract on
#pragma scheduling off
// Compute one row/column dot product with the original fused accumulation order.
static inline float concat_element(float a0, float a1, float a2, float a3,
                                   float b0, float b1, float b2, float b3) {
    float p = b0 * a0;
    p = matrix_madd(b1, a1, p);
    p = matrix_madd(b2, a2, p);
    return matrix_madd(b3, a3, p);
}

// Left column pairs are computed first. Later source-row loads follow the game's stores.
void* PSMTX44Concat(void* dst, const void* first, const void* second);
#pragma pop

// Write a diagonal x/y/z scale matrix with homogeneous w=1.
void Mtx44_SetScale(void* dst, float x, float y, float z);

// Write identity plus row-vector translation at flat indices 12,13,14.
void Mtx44_SetTranslation(void* dst, float x, float y, float z);

// Forward the four-component matrix transform without changing argument order.
void Mtx44_TransformVec4Forward(float* dst, const float* v, const float* m);

// Copy two float components, reading both before writing.
void gtl_Vec2_Copy(void* dst, const void* input);

// Copy three float components without converting representations.
void Vector_CopyComponents(void* dst, const void* input);

// Copy four float components, including w.
void Vec4_Copy(void* dst, const void* input);

// Copy one pair at a time, retaining the game's behavior for partially overlapping ranges.
void Mtx44_Copy(void* dst, const void* input);

// Compute (a*b)*c through the existing concatenator and return dst.
void* Mtx44_Concat3(void* dst, const void* a, const void* b, const void* c);

// Forward alignment of the basis while preserving the underlying mode contract.
float* Matrix_AlignForwardPreservingScaleAndTranslationWrapper(float* dst, float* first, float* second, int mode);

extern void* g_revoExVersionString;
// Return the borrowed module version-string address.
void* RevoEX_GetVersionString(void);
void NCDiGetWirelessMacAddress(void*);
// Forward the caller-owned MAC-address output buffer to NCD.
void Network_GetWirelessMacAddress(void* input);

// yaw goes to Y and pitch to X; row-vector application order is Z, then X, then Y.
void* Mtx44_SetEulerRotationZXY(void* dst, float yawRadians, float pitchRadians, float z);

// All quaternion components are read before any matrix entry is written.
void Mtx44_FromQuaternion(void* dst, const void* input);

extern float kFloat_neg2_Hermite; // -2.0f, Hermite coefficient

// Compute endpoint and tangent basis weights with the original single-precision fused rounding.
static void hermite_weights(float* weights, float t) {
    float t2 = t * t, three_t2 = kFloat_3_Math * t2, t3 = t2 * t;
    float h0 = fmaf(kFloat_2_Math, t3, -three_t2);
    float ht0 = -fmaf(kFloat_2_Math, t2, -t3);
    weights[1] = fmaf(kFloat_neg2_Hermite, t3, three_t2);
    weights[2] = t + ht0;
    weights[0] = kFloat_1_MatrixMath + h0;
    weights[3] = t3 - t2;
}

// Arguments are start point, start tangent, end point, end tangent.
void* Vec2_Hermite(void* dst, const void* p0, const void* startTangent, const void* endPoint, const void* t1, float t);

// The same unusual row ordering as the two-component version is intentional.
void* Vec3_Hermite(void* dst, const void* p0, const void* startTangent, const void* endPoint, const void* t1, float t);

// Mode 1 replaces only m[10]; the depth translation is deliberately not doubled.
void MtxPerspectiveFovRH(void* dst, float fovy, float aspect, float nearz, float farz);

// The global depth convention changes only m[10], as in the right-handed version.
void MtxPerspectiveFovLH(void* dst, float fovy, float aspect, float nearz, float farz);

// Mode 1 changes the depth translation only.
void Mtx44_SetOrthoCentered(void* dst, float width, float height, float nearz, float farz);

// Mode 1 changes the depth translation only; depth scale remains 1/(near-far).
void MtxOrthoOffCenter(void* dst, float left, float right, float bottom, float top, float nearz, float farz);

// Unlike MtxOrthoOffCenter, the depth scale uses 1/(far-near), while translation uses near-far.
void Mtx44_SetOrthoOffCenterLH(void* dst, float left, float right, float bottom, float top, float nearz, float farz);

struct MathViewport { unsigned int x, y, width, height; float nearz, farz; };
extern double kDouble_UnsignedIntConversionBias_Viewport; // 4503599627370496.0, unsigned-int conversion bias

// Convert unsigned viewport coordinates with the original double-bias rounding sequence.
static float viewport_integer(unsigned int value) {
    return (float)(((double)value + 4503599627370496.0) - kDouble_UnsignedIntConversionBias_Viewport);
}

// Compose world, view, projection and inverse viewport transforms, then project with perspective division.
void* Vec3_ProjectToViewport(void* dst, void* input, void* viewport, void* projection, void* view, const void* world);

// Inversion happens after composition; do not distribute it across the four matrices.
void* Vec3_UnprojectFromViewport(void* dst, void* input, void* viewport, void* projection, void* view, void* world);

#pragma push
#pragma dont_inline off
#pragma fp_contract on
// The game feeds the raw frsqrte result straight into single-precision fmuls (original Math_SquareRootApprox 0x8043869C-0x804386BC:
// fmuls est*est, fmuls est*0.5, no frsp). A double "estimate * estimate" + frsp rounds differently:
// Gekko's fmuls rounds its second operand to 25 bits first, so the result was 1 ulp off now and then
// (ball roll speeds from CQuadTree_UpdateBallTriangleCollision differed from the original). Keep the estimate in a float register.
#define RSQRTE_ESTIMATE(est, value) asm { frsqrte est, value }

// Estimate square root with one reciprocal-square-root refinement, treating the small signed interval as zero.
float Math_SquareRootApprox(register float value);

// Return the two-component length using the same estimate and small-value handling as Math_SquareRootApprox.
float Vec2_Length(const void* input);

// Return the three-component length using the same estimate and small-value handling as Math_SquareRootApprox.
float Vec3_Length(const void* input);

// Return the four-component length using the same estimate and small-value handling as Math_SquareRootApprox.
float Vec4_Length(void* input);

extern float kVec3NormalizeZeroFallback[3]; // three-component normalization fallback
extern float kVec4NormalizeZeroFallback[4]; // four-component normalization fallback

// Normalize three components with one reciprocal-square-root refinement, copying the fallback for an all-zero input.
void* Vec3_Normalize(void* dst, const void* input);

// Normalize four components with one reciprocal-square-root refinement, copying the fallback for an all-zero input.
void* Vec4_Normalize(void* dst, const void* input);
#pragma pop

} // extern "C"

#pragma pop

// Definitions follow the original text-address order.
extern "C" {

#pragma push
// Validate the factory descriptor and return a caller-owned input device (1), or
// 0 on failure; invalid args preserve output, allocation failure writes null.
// Release with gtl_IInputDevice_dtor(device,1) (gtl_texture_projection_input_device.cpp), the deleting
// destructor at the device vtable's first callable slot. Do not use global delete:
// storage came from g_inputAllocator[0] and the destructor returns it through
// __dl__Q23gtl25IInputResourceMemOperatorFPv -> g_inputAllocator[1]. Keep that
// allocator pair alive/unchanged until every device has been destroyed.
int gtl_IInputDevice_Create(const DriverEntry* entry, void** destination) {
    if (!destination) return 0;
    if (!entry) return 0;
    if ((int)entry->descriptorWord1 < 0 || (int)entry->descriptorWord1 >= kFactoryDeviceTypeCount) return 0;
    void* object = g_inputAllocator[0](sizeof(GtlInputDevice));
    if (object) gtl_IInputDevice_Ctor(object, entry);
    *destination = object;
    if (!object) return 0;
    return 1;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Visit matching connected channels using stack-borrowed DriverEntry descriptors;
// callback must consume/copy each record synchronously; see provider ID caveat above.
// callback may be null; callbackArgument is passed unchanged. Always returns 1,
// including no matches. Filters 0=all, 2=pad, 3=pointer, 4=kind range; 1 and 5..16
// reject all, 0x11..0x1c request exact kind, all other values reject all. Providers
// must exist in the configured order; pad providers compete per channel, pointer
// entries bypass channel-status testing. This does not report a connected count.
int InputDriver_EnumerateDevices(int deviceFilter, void (*callback)(void*, int), int callbackArgument) {
    int entryFlags;
    unsigned int entryProviderId;
    unsigned int entryChannel;
    unsigned int* providerOrder;
    int providerIds[kInputProviderCount];
    DriverEntry devices[kMaxEnumeratedDevices];
    DriverEntry* volatile devicesEnd = devices;
    providerOrder = inputProviderPriorityOrder;
    unsigned int providerIndex;
    int channel;
    unsigned int padAlreadyAdded;
    int providerId;
    DriverEntry* entry;
    InputProvider* provider;
    unsigned int deviceKind;
    int matchesFilter;

    providerIds[0] = providerOrder[0];
    providerIds[1] = providerOrder[1];
    providerIds[2] = providerOrder[2];
    providerIds[3] = providerOrder[3];
    // Build at most one competing pad entry per channel, plus independent providers.
    for (channel = 0; channel < kControllerChannelCount; channel++) {
        padAlreadyAdded = 0;
        for (providerIndex = 0; providerIndex < kInputProviderCount; providerIndex++) {
            providerId = providerIds[providerIndex];
            provider = (InputProvider*)InputDriver_GetByProviderId(providerId);
            entryFlags = 0;
            entryProviderId = providerId;
            entryChannel = channel;
            if (providerId != kProviderWiiPointer) {
                if (provider->vtable->queryChannelStatus(provider, channel) != 0) {
                    continue;
                }
                // A single switch scope lets break skip pad arbitration while continue skips the provider.
                switch (0) { default:
                    // These three pad providers compete for one entry per channel.
                    if (providerId == kProviderGameCube) {
                    } else if (providerId == kProviderWiiNunchuk) {
                    } else if (providerId != kProviderWiiClassic) {
                        break;
                    }
                    if (padAlreadyAdded != 0) {
                        continue;
                    }
                    padAlreadyAdded = 1;
                }
            }
            if ((unsigned int)(devicesEnd - devices) < kMaxEnumeratedDevices) {
                DriverEntry* temp = devicesEnd;
                temp->flag = entryFlags;
                temp->descriptorWord1 = entryProviderId;
                temp->index = entryChannel;
                devicesEnd++;
            }
        }
    }

    // Filter by provider-reported device kind, then publish the stack records.
    for (entry = devices; entry != devicesEnd; entry++) {
        provider = (InputProvider*)InputDriver_GetByProviderId(entry->descriptorWord1);
        deviceKind = provider->vtable->getDeviceKind(provider);
        matchesFilter = 0;
        switch (deviceFilter) {
        case kDeviceFilterAll:
            matchesFilter = 1;
            break;
        case 1:
            break;
        case kDeviceFilterPad:
            if (deviceKind == kDeviceKindPad || deviceKind == kDeviceKindAlternatePad) {
                matchesFilter = 1;
            }
            break;
        case kDeviceFilterPointer:
            if (deviceKind == kDeviceKindPointer) {
                matchesFilter = 1;
            }
            break;
        case kDeviceFilterKindRange:
            if ((int)deviceKind >= kFirstRangeDeviceKind && (int)deviceKind < kEndRangeDeviceKind) {
                matchesFilter = 1;
            }
            break;
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
            break;
        default:
            if (deviceFilter >= kFirstExactDeviceKind && deviceFilter < kEndExactDeviceKind) {
                if ((unsigned int)deviceFilter == provider->vtable->getDeviceKind(provider)) {
                    matchesFilter = 1;
                }
            }
            break;
        }
        if (matchesFilter != 0 && callback != 0) {
            callback(entry, callbackArgument);
        }
    }
    return 1;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Satisfy input-module initialization without allocating or probing hardware.
int InputDriver_InitStub(void) { return 1; }
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Allocate size bytes through the required configured input allocator.
void* __nw__Q23gtl25IInputResourceMemOperatorFUl(unsigned int size) { return g_inputAllocator[0](size); }
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Return input-device storage to its configured matching free callback.
void __dl__Q23gtl25IInputResourceMemOperatorFPv(void* memory) {
    ((void (*)(void*))g_inputAllocator[1])(memory);
}
#pragma pop

#pragma push
// Copy a required readable uint32 depth mode; null returns 0 without changing
// globals, otherwise publishes owned scalar storage and returns 1. No enum check.
int Projection_SetDepthConvention(void* depthConvention)
{
    if (!depthConvention) return 0;

    unsigned int val = *(unsigned int*)depthConvention;
    s_projectionDepthConventionStorage = val;
    g_projectionDepthConvention = &s_projectionDepthConventionStorage;
    s_projectionDepthConventionInitialized = 1;

    return 1;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Clear the depth-mode pointer/initialized flag; reinitialize before projection.
void Projection_ResetDepthConvention(void) {
    g_projectionDepthConvention = 0;
    s_projectionDepthConventionInitialized = 0;
}
#pragma pop

#pragma push
#pragma dont_inline off
#pragma fp_contract on
// Estimate sqrt(value) with Gekko refinement; [-1e-6,1e-6) yields zero, while
// other invalid/negative inputs follow the hardware estimate (no error status).
float Math_SquareRootApprox(register float value) {
    if (value >= kSqrtZeroLowerBound && value < kSqrtZeroUpperBound)
        return kFloat_0_MatrixMath;
    register float estimate;
    RSQRTE_ESTIMATE(estimate, value);
    float square = estimate * estimate;
    float half_estimate = estimate * kFloat_0_5_MatrixMath;
    float correction = FP_FNMSUBS(square, value, kFloat_3_Math);
    float refined = correction * half_estimate;
    return kFloat_1_MatrixMath / refined;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Approximate sine of radians with whole-turn folding and fused polynomial terms.
// Caller domain: finite angle with float(kInvTwoPi_AngleWrap*angle) in
// [-2147483648,2147483648). Outside it, truncate_angle_turns saturates rather
// than reducing turns; NaN follows its INT_MIN branch. No error is returned.
// This conversion bound is not an accuracy guarantee: very large finite angles
// lose phase precision even inside it. Rotation callers use small angles (e.g.
// quaternion interpolation in math_matrix_quat_vec_utils.cpp); a broader error bound is unestablished.
float Math_SinApprox(float angle) {
    float turns = kInvTwoPi_AngleWrap * angle;
    int integral = truncate_angle_turns(turns);
    float whole = (float)(((double)integral + 4503601774854144.0) - kDouble_SignedIntConversionBias_MatrixMath);
    float x = -fmaf(kTwoPi_AngleWrap, whole, -angle);
    if (x < kFloat_0_MatrixMath) x = x + kTwoPi_AngleWrap;
    if (x > kFloat_4_712389) x = x - kTwoPi_AngleWrap;
    else if (x > kHalfPi_MatrixMath) x = kFloat_3_1415927_MatrixMath - x;
    float x2 = x * x;
    float x3 = x2 * x;
    float x5 = x3 * x2;
    float a = fmaf(kFloat_neg0_16666667_Trig, x3, x);
    float x7 = x5 * x2;
    float b = fmaf(kFloat_0_008333333_805E25F4, x5, a);
    float x9 = x7 * x2;
    float c = fmaf(kSinSeriesCoefficient7, x7, b);
    return fmaf(kFloat_0_0000027557319_rotationPolynomial, x9, c);
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Approximate cosine via a float-rounded pi/2 shift before sine range reduction.
// Same finite turn-conversion domain as Math_SinApprox, applied to the FLOAT
// rounded (pi/2+angle). Shift overflow/NaN is outside the accepted domain.
float Math_CosApprox(float angle) {
    float shifted = kHalfPi_MatrixMath + angle;
    float turns = kInvTwoPi_AngleWrap * shifted;
    int integral = truncate_angle_turns(turns);
    float whole = (float)(((double)integral + 4503601774854144.0) - kDouble_SignedIntConversionBias_MatrixMath);
    float x = -fmaf(kTwoPi_AngleWrap, whole, -shifted);
    if (x < kFloat_0_MatrixMath) x = x + kTwoPi_AngleWrap;
    if (x > kFloat_4_712389) x = x - kTwoPi_AngleWrap;
    else if (x > kHalfPi_MatrixMath) x = kFloat_3_1415927_MatrixMath - x;
    float x2 = x * x;
    float x3 = x2 * x;
    float x5 = x3 * x2;
    float a = fmaf(kFloat_neg0_16666667_Trig, x3, x);
    float x7 = x5 * x2;
    float b = fmaf(kFloat_0_008333333_805E25F4, x5, a);
    float x9 = x7 * x2;
    float c = fmaf(kSinSeriesCoefficient7, x7, b);
    return fmaf(kFloat_0_0000027557319_rotationPolynomial, x9, c);
}
#pragma pop

#pragma push
#pragma dont_inline off
#pragma fp_contract on
// Return two-component magnitude with the same small-square cutoff/estimate as sqrt.
float Vec2_Length(const void* input) {
    register float value = Vec2_LengthSquared(input);
    if (value >= kSqrtZeroLowerBound && value < kSqrtZeroUpperBound)
        return kFloat_0_MatrixMath;
    register float estimate;
    RSQRTE_ESTIMATE(estimate, value);
    float square = estimate * estimate;
    float half_estimate = estimate * kFloat_0_5_MatrixMath;
    float correction = kFloat_3_Math - square * value;
    float refined = correction * half_estimate;
    return kFloat_1_MatrixMath / refined;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Return squared magnitude of two floats in the caller's common coordinate space.
float Vec2_LengthSquared(const void* input) {
    const float* v = (const float*)input;
    float x2 = v[0] * v[0], y2 = v[1] * v[1];
    return x2 + y2;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Add two XY vectors; exact in-place destination may equal either input.
void gtl_Vec2_Add(void* dst, const void* first, const void* second) {
    const float* a = (const float*)first; const float* b = (const float*)second;
    float x = a[0] + b[0], y = a[1] + b[1];
    float* out = (float*)dst; out[0] = x; out[1] = y;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Subtract second XY from first XY; exact in-place destination is supported.
void gtl_Vec2_Subtract(void* dst, const void* first, const void* second) {
    const float* a = (const float*)first; const float* b = (const float*)second;
    float x = a[0] - b[0], y = a[1] - b[1];
    float* out = (float*)dst; out[0] = x; out[1] = y;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Scale two components, supporting destination == input.
void Vec2_Scale(void* dst, const void* input, float scale) {
    const float* v = (const float*)input;
    float x = v[0] * scale, y = v[1] * scale;
    float* out = (float*)dst; out[0] = x; out[1] = y;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Interpolate XY from endpoint/tangent data; read all control points before output.
void* Vec2_Hermite(void* dst, const void* p0, const void* startTangent, const void* endPoint, const void* t1, float t) {
    float matrix[16], weights[4], result[4];
    hermite_weights(weights, t); PSMTX44Identity(matrix);
    Memory_Copy(matrix, p0, 8); Memory_Copy(matrix + 4, endPoint, 8);
    Memory_Copy(matrix + 8, startTangent, 8); Memory_Copy(matrix + 12, t1, 8);
    Mtx44_TransformVec4(result, weights, matrix);
    float* out = (float*)dst; out[0] = result[0]; out[1] = result[1]; return dst;
}
#pragma pop

#pragma push
// Interpolate XY between start/end using previous/next points; no t clamp.
void* Vec2_CatmullRom(void* destination, const void* previousPoint, const void* startPoint, const void* endPoint, const void* nextPoint, float interpolation) {
    struct {
        float result[4];
        float weights[4];
        float matrix[16];
    } stack;
    register float* matrix = stack.matrix;
    register float two = kFloat_2_Math;
    PVec2 zero = kFloat_0_MatrixMath;
    register float t = interpolation;
    register float one = kFloat_1_MatrixMath;
    matrix[0] = one;
    register float t2 = t * t;
    register float t3 = t2 * t;
    register float w0 = kFloat_neg1_MatrixMath * t + (kFloat_neg1_MatrixMath * t3 + two * t2);
    register float w1 = two + (kFloat_3_Math * t3 - kFloat_5_CatmullRom * t2);
    register float w2 = t + (kFloat_neg3_CatmullRom * t3 + kFloat_4_CatmullRom * t2);
    register float w3 = t3 - t2;
    register float half = kFloat_0_5_MatrixMath;
    w1 *= half;
    w0 *= half;
    w3 *= half;
    w2 *= half;
    *(PVec2*)(matrix + 1) = zero;
    *(PVec2*)(matrix + 3) = zero;
    matrix[5] = one;
    stack.weights[0] = w0;
    stack.weights[1] = w1;
    stack.weights[2] = w2;
    stack.weights[3] = w3;
    *(PVec2*)(matrix + 6) = zero;
    *(PVec2*)(matrix + 8) = zero;
    matrix[10] = one;
    *(PVec2*)(matrix + 11) = zero;
    *(PVec2*)(matrix + 13) = zero;
    matrix[15] = one;

    Memory_Copy(matrix, previousPoint, 8);
    Memory_Copy(stack.matrix + 4, startPoint, 8);
    Memory_Copy(stack.matrix + 8, endPoint, 8);
    Memory_Copy(stack.matrix + 12, nextPoint, 8);

    Mtx44_TransformVec4(stack.result, stack.weights, matrix);

    Vec2* out = (Vec2*)destination;
    out->x = stack.result[0];
    out->y = stack.result[1];
    return destination;
}
#pragma pop

#pragma push
#pragma dont_inline off
#pragma fp_contract on
// Return XYZ magnitude using Gekko refinement and the small-squared-length cutoff.
float Vec3_Length(const void* input) {
    register float value = Vec3_LengthSquared(input);
    if (value >= kSqrtZeroLowerBound && value < kSqrtZeroUpperBound)
        return kFloat_0_MatrixMath;
    register float estimate;
    RSQRTE_ESTIMATE(estimate, value);
    float square = estimate * estimate;
    float half_estimate = estimate * kFloat_0_5_MatrixMath;
    float correction = kFloat_3_Math - square * value;
    float refined = correction * half_estimate;
    return kFloat_1_MatrixMath / refined;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Return XYZ squared length with fused X/Z partial sum before adding Y squared.
float Vec3_LengthSquared(const void* input) {
    const float* v = (const float*)input;
    float x2 = v[0] * v[0], y2 = v[1] * v[1];
    float xz = fmaf(v[2], v[2], x2);
    return xz + y2;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Dot two XYZ vectors; preserve the fused X/Y sum then separate Z addition.
float Vec3_Dot(const void* first, const void* second) {
    const float* a = (const float*)first; const float* b = (const float*)second;
    float yy = a[1] * b[1], zz = a[2] * b[2];
    float xy = fmaf(a[0], b[0], yy);
    return xy + zz;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Cross first with second, allowing destination == either input; preserve negated
// fused Y/Z results, including their zero signs.
void Vec3_Cross(void* dst, const void* first, const void* second) {
    const float* a = (const float*)first; const float* b = (const float*)second;
    float bx = b[0], by = b[1], az = a[2], ax = a[0], ay = a[1], bz = b[2];
    float bxaz = bx * az, byaz = by * az, byax = by * ax;
    float yneg = fmaf(ax, bz, -bxaz);
    float x = fmaf(ay, bz, -byaz);
    float zneg = fmaf(ay, bx, -byax);
    float* out = (float*)dst; out[0] = x; out[1] = -yneg; out[2] = -zneg;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Add XYZ vectors after reading all components; exact in-place output is supported.
void Vec3_Add(void* dst, const void* first, const void* second) {
    const float* a = (const float*)first; const float* b = (const float*)second;
    float x = a[0] + b[0], y = a[1] + b[1], z = a[2] + b[2];
    float* out = (float*)dst; out[0] = x; out[1] = y; out[2] = z;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Subtract second XYZ from first, supporting exact in-place output.
void Vec3_Subtract(void* dst, const void* first, const void* second) {
    const float* a = (const float*)first; const float* b = (const float*)second;
    float x = a[0] - b[0], y = a[1] - b[1], z = a[2] - b[2];
    float* out = (float*)dst; out[0] = x; out[1] = y; out[2] = z;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Scale XYZ, supporting destination == input.
void Vec3_Scale(void* dst, const void* input, float scale) {
    const float* v = (const float*)input;
    float x = v[0] * scale, y = v[1] * scale, z = v[2] * scale;
    float* out = (float*)dst; out[0] = x; out[1] = y; out[2] = z;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Interpolate XYZ without clamping t or fusing multiply/add; inputs may equal dst.
void Vec3_Lerp(void* dst, const void* first, const void* second, float t) {
    const float* a = (const float*)first; const float* b = (const float*)second;
    float ax = a[0], ay = a[1], az = a[2];
    float dx = b[0] - ax, dy = b[1] - ay, dz = b[2] - az;
    float tx = dx * t, ty = dy * t, tz = dz * t;
    float x = ax + tx, y = ay + ty, z = az + tz;
    float* out = (float*)dst; out[0] = x; out[1] = y; out[2] = z;
}
#pragma pop

#pragma push
#pragma dont_inline off
#pragma fp_contract on
// Normalize XYZ with one Gekko refinement; exactly zero copies the shared fallback.
// Supports in-place output; no near-zero cutoff or invalid-input status.
void* Vec3_Normalize(void* dst, const void* input) {
    const float* v = (const float*)input;
    float zero = kFloat_0_MatrixMath;
    if (zero == v[0] && zero == v[1] && zero == v[2]) {
        Vector_CopyComponents(dst, kVec3NormalizeZeroFallback);
        return dst;
    }
    float x = v[0], y = v[1], z = v[2];
    float x2 = x * x, y2 = y * y;
    float xz = fmaf(z, z, x2);
    register float value = xz + y2;
    register float estimate;
    RSQRTE_ESTIMATE(estimate, value);
    float square = estimate * estimate;
    float half_estimate = estimate * kFloat_0_5_MatrixMath;
    float correction = FP_FNMSUBS(square, value, kFloat_3_Math);
    float refined = correction * half_estimate;
    float rx = x * refined, ry = y * refined, rz = z * refined;
    volatile float* out = (volatile float*)dst;
    out[0] = rx; out[1] = ry; out[2] = rz;
    return dst;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Interpolate XYZ with endpoint/tangent basis; all controls read before output.
void* Vec3_Hermite(void* dst, const void* p0, const void* startTangent, const void* endPoint, const void* t1, float t) {
    float matrix[16], weights[4], result[4];
    hermite_weights(weights, t); PSMTX44Identity(matrix);
    Memory_Copy(matrix, p0, 12); Memory_Copy(matrix + 4, endPoint, 12);
    Memory_Copy(matrix + 8, startTangent, 12); Memory_Copy(matrix + 12, t1, 12);
    Mtx44_TransformVec4(result, weights, matrix);
    float* out = (float*)dst; out[0] = result[0]; out[1] = result[1]; out[2] = result[2]; return dst;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Transform XYZ with implicit W=1 into FOUR output floats; no perspective divide.
void Mtx44_TransformPointToVec4(void* dst, const void* input, const void* matrix) {
    float result[4];
    transform_four(result, (const float*)input, (const float*)matrix, 1.0f);
    float* out = (float*)dst;
    out[0] = result[0]; out[1] = result[1]; out[2] = result[2]; out[3] = result[3];
}
#pragma pop

#pragma push
#pragma dont_inline off
#pragma fp_contract on
#pragma scheduling off
// Transform XYZ with W=1, divide by resulting W, store XYZ in Z/X/Y order.
// Source/destination may coincide; W=0 has no failure guard. Preserve scalar
// reciprocal rounding and paired-store-compatible subnormal flushing.
void Mtx44_TransformPointPerspective(void* dst, const void* input, const void* matrix) {
    const float* v = (const float*)input;
    const float* m = (const float*)matrix;
    float vx = v[0], vy = v[1], vz = v[2];
    float rx = perspective_component(vx, vy, vz, m, 0);
    float ry = perspective_component(vx, vy, vz, m, 1);
    float rz = perspective_component(vx, vy, vz, m, 2);
    float rw = perspective_component(vx, vy, vz, m, 3);
    float inverse = 1.0f / rw;
    float z = rz * inverse, x = rx * inverse, y = ry * inverse;
    volatile float* out = (volatile float*)dst;
    matrix_store(out + 2, z); matrix_store(out, x); matrix_store(out + 1, y);
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Transform XYZ with implicit W=0, retaining zero-times-translation arithmetic.
void Mtx44_TransformDirection(void* dst, const void* input, const void* matrix) {
    float result[4];
    transform_four(result, (const float*)input, (const float*)matrix, kFloat_0_MatrixMath);
    float* out = (float*)dst; out[0] = result[0]; out[1] = result[1]; out[2] = result[2];
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Project model XYZ to viewport XYZ through world*view*projection*inverse(ortho).
// All records required; dst may equal input. Legacy ortho bounds are x,width,
// height,y AS STORED (not x+width/y+height), so nonzero origins need care. No
// singularity/W=0 status is returned; keep dimensions/depth interval nondegenerate.
void* Vec3_ProjectToViewport(void* dst, void* input, void* viewport, void* projection, void* view, const void* world) {
    const MathViewport* vp = (const MathViewport*)viewport;
    float viewport_matrix[16], combined[16];
    float x = viewport_integer(vp->x), width = viewport_integer(vp->width);
    float height = viewport_integer(vp->height), y = viewport_integer(vp->y);
    float nearz = vp->nearz, farz = vp->farz;
    Mtx44_SetOrthoOffCenterLH(viewport_matrix, x, width, height, y, nearz, farz);
    Mtx44_InvertLegacyCofactors(viewport_matrix, 0, viewport_matrix);
    PSMTX44Concat(combined, world, view); PSMTX44Concat(combined, combined, projection);
    PSMTX44Concat(combined, combined, viewport_matrix);
    Mtx44_TransformPointPerspective(dst, input, combined); return dst;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Unproject viewport XYZ by inverting ProjectToViewport's composed matrix; same
// required buffers and legacy bounds. No inverse-success status: caller must
// provide nonsingular transforms. Result is in world's INPUT/model space.
void* Vec3_UnprojectFromViewport(void* dst, void* input, void* viewport, void* projection, void* view, void* world) {
    const MathViewport* vp = (const MathViewport*)viewport;
    float viewport_matrix[16], combined[16];
    float x = viewport_integer(vp->x), width = viewport_integer(vp->width);
    float height = viewport_integer(vp->height), y = viewport_integer(vp->y);
    float nearz = vp->nearz, farz = vp->farz;
    Mtx44_SetOrthoOffCenterLH(viewport_matrix, x, width, height, y, nearz, farz);
    Mtx44_InvertLegacyCofactors(viewport_matrix, 0, viewport_matrix);
    PSMTX44Concat(combined, world, view); PSMTX44Concat(combined, combined, projection);
    PSMTX44Concat(combined, combined, viewport_matrix);
    Mtx44_InvertLegacyCofactors(combined, 0, combined);
    Mtx44_TransformPointPerspective(dst, input, combined); return dst;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Fill XYZ with value; dst requires three writable floats.
void Vec3_SetAll(void* dst, float value) {
    float* out = (float*)dst; out[0] = value; out[1] = value; out[2] = value;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract on
// Return XYZW magnitude including W, using Gekko refinement/small-square cutoff.
float Vec4_Length(void* input) {
    register float value = Vec4_LengthSquared(input);
    if (value >= kSqrtZeroLowerBound && value < kSqrtZeroUpperBound)
        return kFloat_0_MatrixMath;
    register float estimate;
    RSQRTE_ESTIMATE(estimate, value);
    float square = estimate * estimate;
    float half_estimate = estimate * kFloat_0_5_MatrixMath;
    float correction = kFloat_3_Math - square * value;
    float refined = correction * half_estimate;
    return kFloat_1_MatrixMath / refined;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Return XYZW squared length with paired grouping (X^2+W^2)+(Z^2+Y^2).
float Vec4_LengthSquared(const void* input) {
    const float* v = (const float*)input;
    float x2 = v[0] * v[0], y2 = v[1] * v[1], z2 = v[2] * v[2], w2 = v[3] * v[3];
    float xw = x2 + w2, zy = z2 + y2;
    return xw + zy;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Scale XYZW including homogeneous W; exact in-place destination is supported.
void Vec4_Scale(void* dst, const void* input, float scale) {
    const float* v = (const float*)input;
    float x = v[0] * scale, y = v[1] * scale, z = v[2] * scale, w = v[3] * scale;
    float* out = (float*)dst; out[0] = x; out[1] = y; out[2] = z; out[3] = w;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Interpolate four components with separate multiply/add rounding and unclamped t.
void Vec4_Lerp(void* dst, const void* first, const void* second, float t) {
    const float* a = (const float*)first; const float* b = (const float*)second;
    float av[4], result[4];
    int i;
    for (i = 0; i < 4; ++i) av[i] = a[i];
    for (i = 0; i < 4; ++i) {
        float delta = b[i] - av[i];
        float scaled = delta * t;
        result[i] = av[i] + scaled;
    }
    for (i = 0; i < 4; ++i) ((float*)dst)[i] = result[i];
}
#pragma pop

#pragma push
#pragma dont_inline off
#pragma fp_contract on
// Normalize all four components; exactly zero copies shared fallback, in-place OK.
void* Vec4_Normalize(void* dst, const void* input) {
    const float* v = (const float*)input;
    float zero = kFloat_0_MatrixMath;
    if (zero == v[0] && zero == v[1] && zero == v[2] && zero == v[3]) {
        Vec4_Copy(dst, kVec4NormalizeZeroFallback);
        return dst;
    }
    float x = v[0], y = v[1], z = v[2], w = v[3];
    float x2 = x * x, y2 = y * y, z2 = z * z, w2 = w * w;
    float xw = x2 + w2, zy = z2 + y2;
    register float value = xw + zy;
    register float estimate;
    RSQRTE_ESTIMATE(estimate, value);
    float square = estimate * estimate;
    float half_estimate = estimate * kFloat_0_5_MatrixMath;
    float correction = kFloat_3_Math - square * value;
    float refined = correction * half_estimate;
    float rx = x * refined, ry = y * refined, rz = z * refined, rw = w * refined;
    volatile float* out = (volatile float*)dst;
    out[0] = rx; out[1] = ry; out[2] = rz; out[3] = rw;
    return dst;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Transform row-vector XYZW without dividing by W; all input read before output.
void Mtx44_TransformVec4(float* out, const float* v, const float* matrix) {
    float result[4];
    float w = v[3];
    transform_four(result, v, matrix, w);
    out[0] = result[0]; out[1] = result[1]; out[2] = result[2]; out[3] = result[3];
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Write all sixteen floats of an identity matrix in the original store sequence.
void PSMTX44Identity(void* dst) {
    volatile float* m = (volatile float*)dst; float z = kFloat_0_MatrixMath, one = kFloat_1_MatrixMath;
    m[1] = z; m[2] = z; m[0] = one; m[3] = z; m[4] = z;
    m[5] = one; m[6] = z; m[7] = z; m[8] = z; m[9] = z;
    m[10] = one; m[11] = z; m[12] = z; m[13] = z; m[14] = z; m[15] = one;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Transpose a 4x4 matrix with staged pair loads/stores; exact in-place is supported,
// but arbitrary partial overlap is not a buffered-transpose contract.
void PSMTX44Transpose(void* dst, const void* input) {
    volatile float* d = (volatile float*)dst; const volatile float* s = (const volatile float*)input;
    float a0 = s[0], a1 = s[1], b0 = s[4], b1 = s[5];
    float a2 = s[2], a3 = s[3];
    d[0] = a0; d[1] = b0;
    float b2 = s[6], b3 = s[7];
    d[4] = a1; d[5] = b1;
    float c0 = s[8], c1 = s[9];
    d[8] = a2; d[9] = b2;
    float e0 = s[12], e1 = s[13];
    d[12] = a3; d[13] = b3;
    float c2 = s[10], c3 = s[11];
    d[2] = c0; d[3] = e0;
    float e2 = s[14], e3 = s[15];
    d[6] = c1; d[7] = e1;
    d[10] = c2; d[11] = e2; d[14] = c3; d[15] = e3;
}
#pragma pop

#pragma push
#pragma dont_inline off
#pragma fp_contract on
#pragma scheduling off
// Multiply first*second in row-vector order, returning dst; dst may equal either
// input exactly. Paired-store bit handling and staged reads preserve game rounding.
void* PSMTX44Concat(void* dst, const void* first, const void* second) {
    volatile unsigned int* d = (volatile unsigned int*)dst;
    const volatile float* a = (const volatile float*)first;
    const volatile float* b = (const volatile float*)second;
    // Cache finished results as store bits, freeing the floating-point registers.
    float b00 = b[0], b01 = b[1], b10 = b[4], b11 = b[5];
    float b20 = b[8], b21 = b[9], b30 = b[12], b31 = b[13];
    float a0 = a[0], a1 = a[1], a2 = a[2], a3 = a[3];
    unsigned int l00 = matrix_store_bits(concat_element(a0, a1, a2, a3, b00, b10, b20, b30));
    unsigned int l01 = matrix_store_bits(concat_element(a0, a1, a2, a3, b01, b11, b21, b31));
    a0 = a[4]; a1 = a[5]; a2 = a[6]; a3 = a[7];
    unsigned int l10 = matrix_store_bits(concat_element(a0, a1, a2, a3, b00, b10, b20, b30));
    unsigned int l11 = matrix_store_bits(concat_element(a0, a1, a2, a3, b01, b11, b21, b31));
    a0 = a[8]; a1 = a[9]; a2 = a[10]; a3 = a[11];
    unsigned int l20 = matrix_store_bits(concat_element(a0, a1, a2, a3, b00, b10, b20, b30));
    unsigned int l21 = matrix_store_bits(concat_element(a0, a1, a2, a3, b01, b11, b21, b31));
    a0 = a[12]; a1 = a[13]; a2 = a[14]; a3 = a[15];
    unsigned int l30 = matrix_store_bits(concat_element(a0, a1, a2, a3, b00, b10, b20, b30));
    float l31_value = concat_element(a0, a1, a2, a3, b01, b11, b21, b31);

    float b02 = b[2], b03 = b[3], b12 = b[6], b13 = b[7];
    float b22 = b[10], b23 = b[11], b32 = b[14], b33 = b[15];
    // b is now dead, leaving an integer register for the eighth left result.
    unsigned int l31 = matrix_store_bits(l31_value);
    a0 = a[0]; a1 = a[1]; a2 = a[2]; a3 = a[3];
    float r00_value = concat_element(a0, a1, a2, a3, b02, b12, b22, b32);
    float r01_value = concat_element(a0, a1, a2, a3, b03, b13, b23, b33);
    float a10 = a[4], a11 = a[5], a12 = a[6], a13 = a[7];
    d[0] = l00; d[1] = l01;
    // Convert after releasing l00/l01, keeping this leaf in volatile registers.
    unsigned int r00 = matrix_store_bits(r00_value);
    unsigned int r01 = matrix_store_bits(r01_value);
    float r10_value = concat_element(a10, a11, a12, a13, b02, b12, b22, b32);
    float r11_value = concat_element(a10, a11, a12, a13, b03, b13, b23, b33);
    // These reads must precede d[4..5], but need not overlap the row-1 arithmetic.
    a0 = a[8]; a1 = a[9];
    d[4] = l10; d[5] = l11;
    unsigned int r10 = matrix_store_bits(r10_value);
    unsigned int r11 = matrix_store_bits(r11_value);
    a2 = a[10]; a3 = a[11];
    d[8] = l20; d[9] = l21;
    unsigned int r20 = matrix_store_bits(concat_element(a0, a1, a2, a3, b02, b12, b22, b32));
    unsigned int r21 = matrix_store_bits(concat_element(a0, a1, a2, a3, b03, b13, b23, b33));
    a10 = a[12]; a11 = a[13];
    d[12] = l30; d[13] = l31;
    a12 = a[14]; a13 = a[15];
    d[2] = r00; d[3] = r01;
    unsigned int r30 = matrix_store_bits(concat_element(a10, a11, a12, a13, b02, b12, b22, b32));
    unsigned int r31 = matrix_store_bits(concat_element(a10, a11, a12, a13, b03, b13, b23, b33));
    d[6] = r10; d[7] = r11;
    d[10] = r20; d[11] = r21;
    d[14] = r30; d[15] = r31;
    return dst;
}
#pragma pop

#pragma push
// Invert with the legacy C22 cofactor anomaly documented at the declaration;
// exact-zero determinant leaves destination AND optional determinantOut untouched.
// In-place matrix allowed; determinantOut must not alias either matrix.
void Mtx44_InvertLegacyCofactors(void* destination, float* determinantOut, const void* sourceMatrix) {
    const float* src = (const float*)sourceMatrix;
    float* out = (float*)destination;
    unsigned int i;

    // Mathematical map: mRC means src[4*R+C] (zero-based), so m12=src[6],
    // m11=src[5], m23=src[11], m13=src[7]. pIJ is src[I]*src[J], NOT a minor.
    // The determinant below expands 24 signed permutation products, grouped into
    // twelve positive terms then twelve subtractions. Preserve its floating-point order.
    register float m12 = src[6];
    register float m11 = src[5];
    register float m23 = src[11];
    register float m13 = src[7];
    float p06 = src[0] * m12;
    float p05 = src[0] * m11;
    float p07 = src[0] * m13;
    float p16 = src[1] * m12;
    float p14 = src[1] * src[4];
    float p17 = src[1] * m13;
    float p24 = src[2] * src[4];
    float p25 = src[2] * m11;
    float p27 = src[2] * m13;
    float p34 = src[3] * src[4];
    float p35 = src[3] * m11;
    float p36 = src[3] * m12;

    float det = src[12] * (src[9] * p36)
        + (src[14] * (src[8] * p35)
            + (src[13] * (src[10] * p34)
                + (src[13] * (src[8] * p27)
                    + (src[12] * (m23 * p25)
                        + (src[15] * (src[9] * p24)
                            + (src[12] * (src[10] * p17)
                                + (src[15] * (src[8] * p16)
                                    + (src[14] * (m23 * p14)
                                        + (src[14] * (src[9] * p07)
                                            + (src[15] * (src[10] * p05)
                                                + src[13] * (m23 * p06)))))))))))
        - src[14] * (m23 * p05)
        - src[15] * (src[9] * p06)
        - src[13] * (src[10] * p07)
        - src[15] * (src[10] * p14)
        - src[12] * (m23 * p16)
        - src[14] * (src[8] * p17)
        - src[13] * (m23 * p24)
        - src[15] * (src[8] * p25)
        - src[12] * (src[9] * p27)
        - src[14] * (src[9] * p34)
        - src[12] * (src[10] * p35)
        - src[13] * (src[8] * p36);

    if (kFloat_0_MatrixMath == det) {
        return;
    }
    if (determinantOut != 0) {
        *determinantOut = det;
    }

    // Snapshot the input before writing any cofactors, including for in-place use.
    // Adjugate layout: out[4*r+c] corresponds to signed minor C[c,r];
    // the later reciprocal scales all entries by 1/det. Keep the literal terms:
    // out[10] contains a repeated matrix.v[13] in the existing formula, so this
    // code must not be replaced by a textbook inverse during cleanup.
    struct Matrix44 { float v[16]; } matrix = *(const Matrix44*)sourceMatrix;
    // Adjugate row 0: C00, C10, C20, C30.
    out[0] = matrix.v[14] * (matrix.v[7] * matrix.v[9])
        + (matrix.v[15] * (matrix.v[5] * matrix.v[10])
            + matrix.v[13] * (matrix.v[6] * matrix.v[11]))
        - matrix.v[14] * (matrix.v[5] * matrix.v[11])
        - matrix.v[15] * (matrix.v[6] * matrix.v[9])
        - matrix.v[13] * (matrix.v[7] * matrix.v[10]);
    out[1] = matrix.v[13] * (matrix.v[3] * matrix.v[10])
        + (matrix.v[14] * (matrix.v[1] * matrix.v[11])
            + matrix.v[15] * (matrix.v[2] * matrix.v[9]))
        - matrix.v[15] * (matrix.v[1] * matrix.v[10])
        - matrix.v[13] * (matrix.v[2] * matrix.v[11])
        - matrix.v[14] * (matrix.v[3] * matrix.v[9]);
    out[2] = matrix.v[14] * (matrix.v[3] * matrix.v[5])
        + (matrix.v[15] * (matrix.v[1] * matrix.v[6])
            + matrix.v[13] * (matrix.v[2] * matrix.v[7]))
        - matrix.v[14] * (matrix.v[1] * matrix.v[7])
        - matrix.v[15] * (matrix.v[2] * matrix.v[5])
        - matrix.v[13] * (matrix.v[3] * matrix.v[6]);
    out[3] = matrix.v[9] * (matrix.v[3] * matrix.v[6])
        + (matrix.v[10] * (matrix.v[1] * matrix.v[7])
            + matrix.v[11] * (matrix.v[2] * matrix.v[5]))
        - matrix.v[11] * (matrix.v[1] * matrix.v[6])
        - matrix.v[9] * (matrix.v[2] * matrix.v[7])
        - matrix.v[10] * (matrix.v[3] * matrix.v[5]);
    // Adjugate row 1: C01, C11, C21, C31.
    out[4] = matrix.v[12] * (matrix.v[7] * matrix.v[10])
        + (matrix.v[14] * (matrix.v[4] * matrix.v[11])
            + matrix.v[15] * (matrix.v[6] * matrix.v[8]))
        - matrix.v[15] * (matrix.v[4] * matrix.v[10])
        - matrix.v[12] * (matrix.v[6] * matrix.v[11])
        - matrix.v[14] * (matrix.v[7] * matrix.v[8]);
    out[5] = matrix.v[14] * (matrix.v[3] * matrix.v[8])
        + (matrix.v[15] * (matrix.v[0] * matrix.v[10])
            + matrix.v[12] * (matrix.v[2] * matrix.v[11]))
        - matrix.v[14] * (matrix.v[0] * matrix.v[11])
        - matrix.v[15] * (matrix.v[2] * matrix.v[8])
        - matrix.v[12] * (matrix.v[3] * matrix.v[10]);
    out[6] = matrix.v[12] * (matrix.v[3] * matrix.v[6])
        + (matrix.v[14] * (matrix.v[0] * matrix.v[7])
            + matrix.v[15] * (matrix.v[2] * matrix.v[4]))
        - matrix.v[15] * (matrix.v[0] * matrix.v[6])
        - matrix.v[12] * (matrix.v[2] * matrix.v[7])
        - matrix.v[14] * (matrix.v[3] * matrix.v[4]);
    out[7] = matrix.v[10] * (matrix.v[3] * matrix.v[4])
        + (matrix.v[11] * (matrix.v[0] * matrix.v[6])
            + matrix.v[8] * (matrix.v[2] * matrix.v[7]))
        - matrix.v[10] * (matrix.v[0] * matrix.v[7])
        - matrix.v[11] * (matrix.v[2] * matrix.v[4])
        - matrix.v[8] * (matrix.v[3] * matrix.v[6]);
    // Adjugate row 2: C02, C12, C22, C32.
    out[8] = matrix.v[13] * (matrix.v[7] * matrix.v[8])
        + (matrix.v[15] * (matrix.v[4] * matrix.v[9])
            + matrix.v[12] * (matrix.v[5] * matrix.v[11]))
        - matrix.v[13] * (matrix.v[4] * matrix.v[11])
        - matrix.v[15] * (matrix.v[5] * matrix.v[8])
        - matrix.v[12] * (matrix.v[7] * matrix.v[9]);
    out[9] = matrix.v[12] * (matrix.v[3] * matrix.v[9])
        + (matrix.v[13] * (matrix.v[0] * matrix.v[11])
            + matrix.v[15] * (matrix.v[1] * matrix.v[8]))
        - matrix.v[15] * (matrix.v[0] * matrix.v[9])
        - matrix.v[12] * (matrix.v[1] * matrix.v[11])
        - matrix.v[13] * (matrix.v[3] * matrix.v[8]);
    // C22 anomaly: the third positive product below uses v[13], not v[12].
    out[10] = matrix.v[13] * (matrix.v[3] * matrix.v[4])
        + (matrix.v[15] * (matrix.v[0] * matrix.v[5])
            + matrix.v[13] * (matrix.v[1] * matrix.v[7]))
        - matrix.v[13] * (matrix.v[0] * matrix.v[7])
        - matrix.v[15] * (matrix.v[1] * matrix.v[4])
        - matrix.v[12] * (matrix.v[3] * matrix.v[5]);
    out[11] = matrix.v[8] * (matrix.v[3] * matrix.v[5])
        + (matrix.v[9] * (matrix.v[0] * matrix.v[7])
            + matrix.v[11] * (matrix.v[1] * matrix.v[4]))
        - matrix.v[11] * (matrix.v[0] * matrix.v[5])
        - matrix.v[8] * (matrix.v[1] * matrix.v[7])
        - matrix.v[9] * (matrix.v[3] * matrix.v[4]);
    // Adjugate row 3: C03, C13, C23, C33.
    out[12] = matrix.v[12] * (matrix.v[6] * matrix.v[9])
        + (matrix.v[13] * (matrix.v[4] * matrix.v[10])
            + matrix.v[14] * (matrix.v[5] * matrix.v[8]))
        - matrix.v[14] * (matrix.v[4] * matrix.v[9])
        - matrix.v[12] * (matrix.v[5] * matrix.v[10])
        - matrix.v[13] * (matrix.v[6] * matrix.v[8]);
    out[13] = matrix.v[13] * (matrix.v[2] * matrix.v[8])
        + (matrix.v[14] * (matrix.v[0] * matrix.v[9])
            + matrix.v[12] * (matrix.v[1] * matrix.v[10]))
        - matrix.v[13] * (matrix.v[0] * matrix.v[10])
        - matrix.v[14] * (matrix.v[1] * matrix.v[8])
        - matrix.v[12] * (matrix.v[2] * matrix.v[9]);
    out[14] = matrix.v[12] * (matrix.v[2] * matrix.v[5])
        + (matrix.v[13] * (matrix.v[0] * matrix.v[6])
            + matrix.v[14] * (matrix.v[1] * matrix.v[4]))
        - matrix.v[14] * (matrix.v[0] * matrix.v[5])
        - matrix.v[12] * (matrix.v[1] * matrix.v[6])
        - matrix.v[13] * (matrix.v[2] * matrix.v[4]);
    out[15] = matrix.v[9] * (matrix.v[2] * matrix.v[4])
        + (matrix.v[10] * (matrix.v[0] * matrix.v[5])
            + matrix.v[8] * (matrix.v[1] * matrix.v[6]))
        - matrix.v[9] * (matrix.v[0] * matrix.v[6])
        - matrix.v[10] * (matrix.v[1] * matrix.v[4])
        - matrix.v[8] * (matrix.v[2] * matrix.v[5]);
    // Final phase: broadcast 1/det to paired lanes and scale the adjugate in place.
    float f6 = kFloat_1_MatrixMath / det;
    PVec2 scale = f6;
    PVec2* pairs = (PVec2*)out;
    {
        PVec2 a = pairs[0];
        PVec2 b = pairs[1];
        PVec2 c = a * scale;
        PVec2 d = b * scale;
        pairs[0] = c;
        pairs[1] = d;
    }
    {
        PVec2 a = pairs[2];
        PVec2 b = pairs[3];
        PVec2 c = a * scale;
        PVec2 d = b * scale;
        pairs[2] = c;
        pairs[3] = d;
    }
    {
        PVec2 a = pairs[4];
        PVec2 b = pairs[5];
        PVec2 c = a * scale;
        PVec2 d = b * scale;
        pairs[4] = c;
        pairs[5] = d;
    }
    {
        PVec2 a = pairs[6];
        PVec2 b = pairs[7];
        PVec2 c = a * scale;
        PVec2 d = b * scale;
        pairs[6] = c;
        pairs[7] = d;
    }
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Write an axis scale matrix (dimensionless XYZ multipliers), clearing translation.
void Mtx44_SetScale(void* dst, float x, float y, float z) {
    volatile float* m = (volatile float*)dst; float zero = kFloat_0_MatrixMath, one = kFloat_1_MatrixMath;
    m[0] = x; m[1] = zero; m[2] = zero; m[3] = zero; m[4] = zero;
    m[5] = y; m[6] = zero; m[7] = zero; m[8] = zero; m[9] = zero;
    m[10] = z; m[11] = zero; m[12] = zero; m[13] = zero; m[14] = zero; m[15] = one;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Write row-vector identity plus XYZ translation in the caller's distance units.
void Mtx44_SetTranslation(void* dst, float x, float y, float z) {
    volatile float* m = (volatile float*)dst; float zero = kFloat_0_MatrixMath, one = kFloat_1_MatrixMath;
    m[12] = x; m[13] = y; m[14] = z;
    m[0] = one; m[1] = zero; m[2] = zero; m[3] = zero;
    m[4] = zero; m[5] = one; m[6] = zero; m[7] = zero;
    m[8] = zero; m[9] = zero; m[10] = one; m[11] = zero; m[15] = one;
}
#pragma pop

#pragma push
#pragma optimization_level 2
// Build X rotation using the folded sine polynomial, not libm sin/cos; radians.
void Mtx44_SetRotationX(float* out, float angleRadians) {
    register float a = angleRadians - kTwoPi_AngleWrap * (float)(int)(kInvTwoPi_AngleWrap * angleRadians);
    if (a < kFloat_0_MatrixMath) {
        a += kTwoPi_AngleWrap;
    }
    if (a > kFloat_4_712389) {
        a -= kTwoPi_AngleWrap;
    } else if (a > kHalfPi_MatrixMath) {
        a = kFloat_3_1415927_MatrixMath - a;
    }

    register float a2 = a * a;
    register float t = kHalfPi_MatrixMath + angleRadians;
    register float a3 = a2 * a;
    register float a5 = a3 * a2;
    register float a7 = a5 * a2;
    register float a9 = a7 * a2;
    register float s = kFloat_0_0000027557319_rotationPolynomial * a9 + (kSinSeriesCoefficient7 * a7 + (kFloat_0_008333333_805E25F4 * a5 + (kFloat_neg0_16666667_Trig * a3 + a)));
    register float b = t - kTwoPi_AngleWrap * (float)(int)(kInvTwoPi_AngleWrap * t);
    if (b < kFloat_0_MatrixMath) {
        b += kTwoPi_AngleWrap;
    }
    if (b > kFloat_4_712389) {
        b -= kTwoPi_AngleWrap;
    } else if (b > kHalfPi_MatrixMath) {
        b = kFloat_3_1415927_MatrixMath - b;
    }

    register float b2 = b * b;
    register float zero = kFloat_0_MatrixMath;
    register PVec2 zeros = zero;
    register float one = kFloat_1_MatrixMath;
    register float ns = -s;
    register float k3 = kFloat_neg0_16666667_Trig;
    register float b3 = b2 * b;
    register float k5 = kFloat_0_008333333_805E25F4;
    register float k7 = kSinSeriesCoefficient7;
    register float b5 = b3 * b2;
    register float k9 = kFloat_0_0000027557319_rotationPolynomial;
    register float c = k3 * b3 + b;
    *(PVec2*)(out + 2) = zeros;
    register float b7 = b5 * b2;
    c = k5 * b5 + c;
    out[4] = zero;
    register float b9 = b7 * b2;
    *(PVec2*)(out + 11) = zeros;
    c = k7 * b7 + c;
    out[0] = one;
    out[1] = zero;
    c = k9 * b9 + c;
    *(PVec2*)(out + 13) = zeros;
    *(PVec2*)(out + 7) = zeros;
    out[15] = one;
    out[5] = c;
    out[6] = s;
    out[9] = ns;
    out[10] = c;
}
#pragma pop

#pragma push
#pragma optimization_level 4
// Build Y rotation with the polynomial/range reduction described above; radians.
void Mtx44_SetRotationY(float* out, float angleRadians) {
    register float a = angleRadians - kTwoPi_AngleWrap * (float)(int)(kInvTwoPi_AngleWrap * angleRadians);
    if (a < kFloat_0_MatrixMath) {
        a += kTwoPi_AngleWrap;
    }
    if (a > kFloat_4_712389) {
        a -= kTwoPi_AngleWrap;
    } else if (a > kHalfPi_MatrixMath) {
        a = kFloat_3_1415927_MatrixMath - a;
    }

    register float a2 = a * a;
    register float t = kHalfPi_MatrixMath + angleRadians;
    register float a3 = a * a2;
    register float a5 = a3 * a2;
    register float a7 = a5 * a2;
    register float a9 = a7 * a2;
    register float s = kFloat_0_0000027557319_rotationPolynomial * a9 + (kSinSeriesCoefficient7 * a7 + (kFloat_0_008333333_805E25F4 * a5 + (kFloat_neg0_16666667_Trig * a3 + a)));

    register float b = t - kTwoPi_AngleWrap * (float)(int)(kInvTwoPi_AngleWrap * t);
    if (b < kFloat_0_MatrixMath) {
        b += kTwoPi_AngleWrap;
    }
    if (b > kFloat_4_712389) {
        b -= kTwoPi_AngleWrap;
    } else if (b > kHalfPi_MatrixMath) {
        b = kFloat_3_1415927_MatrixMath - b;
    }

    register float b2 = b * b;
    register float zero = kFloat_0_MatrixMath;
    register PVec2 zeros = kFloat_0_MatrixMath;
    register float negs = -s;
    register float one = kFloat_1_MatrixMath;
    register float k3 = kFloat_neg0_16666667_Trig;
    register float b3 = b2 * b;
    register float k5 = kFloat_0_008333333_805E25F4;
    register float k7 = kSinSeriesCoefficient7;
    register float k9 = kFloat_0_0000027557319_rotationPolynomial;
    register float b5 = b3 * b2;
    *(PVec2*)(out + 6) = zeros;
    register float c = k3 * b3 + b;
    *(PVec2*)(out + 12) = zeros;
    register float b7 = b5 * b2;
    c = k5 * b5 + c;
    out[2] = negs;
    out[3] = zero;
    register float b9 = b7 * b2;
    out[4] = zero;
    out[5] = one;
    c = k7 * b7 + c;
    out[8] = s;
    out[9] = zero;
    c = k9 * b9 + c;
    out[14] = zero;
    out[15] = one;
    out[0] = c;
    out[1] = zero;
    out[10] = c;
    out[11] = zero;
}
#pragma pop

#pragma push
#pragma optimization_level 4
// Build Z rotation with the polynomial/range reduction described above; radians.
void Mtx44_SetRotationZ(float* out, float angleRadians) {
    register float a = angleRadians - kTwoPi_AngleWrap * (float)(int)(kInvTwoPi_AngleWrap * angleRadians);
    if (a < kFloat_0_MatrixMath) {
        a += kTwoPi_AngleWrap;
    }
    if (a > kFloat_4_712389) {
        a -= kTwoPi_AngleWrap;
    } else if (a > kHalfPi_MatrixMath) {
        a = kFloat_3_1415927_MatrixMath - a;
    }

    register float a2 = a * a;
    register float t = kHalfPi_MatrixMath + angleRadians;
    register float a3 = a2 * a;
    register float a5 = a3 * a2;
    register float a7 = a5 * a2;
    register float a9 = a7 * a2;
    register float s = kFloat_0_0000027557319_rotationPolynomial * a9 + (kSinSeriesCoefficient7 * a7 + (kFloat_0_008333333_805E25F4 * a5 + (kFloat_neg0_16666667_Trig * a3 + a)));

    register float b = t - kTwoPi_AngleWrap * (float)(int)(kInvTwoPi_AngleWrap * t);
    if (b < kFloat_0_MatrixMath) {
        b += kTwoPi_AngleWrap;
    }
    if (b > kFloat_4_712389) {
        b -= kTwoPi_AngleWrap;
    } else if (b > kHalfPi_MatrixMath) {
        b = kFloat_3_1415927_MatrixMath - b;
    }

    float zero = kFloat_0_MatrixMath;
    float one = kFloat_1_MatrixMath;
    float negs = -s;
    float b2 = b * b;
    float b3 = b2 * b;
    float b5 = b3 * b2;
    float b7 = b5 * b2;
    float b9 = b7 * b2;
    float c = kFloat_0_0000027557319_rotationPolynomial * b9 + (kSinSeriesCoefficient7 * b7 + (kFloat_0_008333333_805E25F4 * b5 + (kFloat_neg0_16666667_Trig * b3 + b)));
    out[2] = zero;
    out[3] = zero;
    out[6] = zero;
    out[7] = zero;
    out[8] = zero;
    out[9] = zero;
    out[12] = zero;
    out[13] = zero;
    out[10] = one;
    out[11] = zero;
    out[14] = zero;
    out[15] = one;
    out[0] = c;
    out[1] = s;
    out[4] = negs;
    out[5] = c;
}
#pragma pop

#pragma push
// Normalize required XYZ axis then build its radian rotation; zero axis uses
// Vec3_Normalize's fallback rather than a failure status. Returns destination.
void* Mtx44_SetRotationAxisAngle(void* destination, const void* axis, float angleRadians) {
    register float b, a, a9, a7, a5, a3, a2;
    register float one, k, zero, yz, xz, xy;
    b = angleRadians;
    a = angleRadians;
    a -= kTwoPi_AngleWrap * (float)truncate_angle_turns(kInvTwoPi_AngleWrap * a);
    if (a < kFloat_0_MatrixMath) {
        a += kTwoPi_AngleWrap;
    }
    if (a > kFloat_4_712389) {
        a -= kTwoPi_AngleWrap;
    } else if (a > kHalfPi_MatrixMath) {
        a = kFloat_3_1415927_MatrixMath - a;
    }

    a2 = a * a;
    a3 = a2 * a;
    a5 = a3 * a2;
    a7 = a5 * a2;
    a9 = a7 * a2;
    float s = kFloat_0_0000027557319_rotationPolynomial * a9 + (kSinSeriesCoefficient7 * a7 + (kFloat_0_008333333_805E25F4 * a5 + (kFloat_neg0_16666667_Trig * a3 + a)));

    b = kHalfPi_MatrixMath + b;
    b = b - kTwoPi_AngleWrap * (float)truncate_angle_turns(kInvTwoPi_AngleWrap * b);
    if (b < kFloat_0_MatrixMath) {
        b += kTwoPi_AngleWrap;
    }
    if (b > kFloat_4_712389) {
        b -= kTwoPi_AngleWrap;
    } else if (b > kHalfPi_MatrixMath) {
        b = kFloat_3_1415927_MatrixMath - b;
    }

    float b2 = b * b;
    float b3 = b2 * b;
    float b5 = b3 * b2;
    float b7 = b5 * b2;
    float b9 = b7 * b2;
    float p2 = kFloat_0_008333333_805E25F4 * b5 + (kFloat_neg0_16666667_Trig * b3 + b);
    float p1 = kSinSeriesCoefficient7 * b7 + p2;
    float c = kFloat_0_0000027557319_rotationPolynomial * b9 + p1;

    float v[3];
    Vec3_Normalize(v, axis);
    one = kFloat_1_MatrixMath;
    k = one - c;
    zero = kFloat_0_MatrixMath;
    xy = v[0] * v[1];
    xz = v[0] * v[2];
    yz = v[1] * v[2];

    float* out = (float*)destination;
    out[0] = k * (v[0] * v[0]) + c;
    out[1] = xy * k + v[2] * s;
    out[2] = xz * k - v[1] * s;
    out[3] = zero;
    out[4] = xy * k - v[2] * s;
    out[5] = k * (v[1] * v[1]) + c;
    out[6] = yz * k + v[0] * s;
    out[7] = zero;
    out[8] = xz * k + v[1] * s;
    out[9] = yz * k - v[0] * s;
    out[10] = k * (v[2] * v[2]) + c;
    out[11] = zero;
    out[12] = zero;
    out[13] = zero;
    out[14] = zero;
    out[15] = one;
    return destination;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Build a rotation from XYZW quaternion without normalizing it; no input validation.
void Mtx44_FromQuaternion(void* dst, const void* input) {
    const float* q = (const float*)input;
    float x = q[0], y = q[1], w = q[3], z = q[2];
    float xx = x * x, yy = y * y, wy = w * y, wz = w * z, zz = z * z, wx = w * x;
    float xyplus = fmaf(x, y, wz), xyminus = fmaf(x, y, -wz);
    float yzsum = yy + zz, xzsum = xx + zz;
    float xzminus = fmaf(x, z, -wy), xzplus = fmaf(x, z, wy);
    float yzplus = fmaf(y, z, wx), yzminus = fmaf(y, z, -wx), xysum = xx + yy;
    float zero = kFloat_0_MatrixMath, one = kFloat_1_MatrixMath, two = kFloat_2_Math;
    volatile float* m = (volatile float*)dst;
    m[3] = zero; m[7] = zero; m[11] = zero; m[12] = zero; m[13] = zero;
    m[0] = -fmaf(two, yzsum, -one);
    m[1] = two * xyplus; m[2] = two * xzminus; m[4] = two * xyminus;
    m[5] = -fmaf(two, xzsum, -one); m[6] = two * yzplus;
    m[8] = two * xzplus; m[9] = two * yzminus;
    m[10] = -fmaf(two, xysum, -one); m[14] = zero; m[15] = one;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Compose Z*X*Y rotations from yaw(Y), pitch(X), and Z angle, all in radians.
void* Mtx44_SetEulerRotationZXY(void* dst, float yawRadians, float pitchRadians, float z) {
    float rotationX[16], rotationY[16], mz[16];
    Mtx44_SetRotationX(rotationX, pitchRadians); Mtx44_SetRotationY(rotationY, yawRadians); Mtx44_SetRotationZ(mz, z);
    Mtx44_Concat3(dst, mz, rotationX, rotationY); return dst;
}
#pragma pop

#pragma push
// Compose optional centered scale/rotation/translation; destination required,
// other pointers nullable (omitted operation). Centers/scale/translation are XYZ;
// rotations are XYZW quaternions. See the declaration's scalingRotation contract:
// its generated matrix must be nonsingular; an inverse failure leaves uninitialized
// local scratch that is still concatenated, with no error return or rollback.
// Destination must not overlap inputs: identity is written first.
void* Mtx44_BuildTransformation(void* destination, const void* scalingCenter, const void* scalingRotation, const void* scale, const void* rotationCenter, const void* rotation, const void* translation) {
    float* out = (float*)destination;
    float scalingCenterMatrix[16];
    float inverseScalingCenter[16];
    float scalingRotationMatrix[16];
    float inverseScalingRotation[16];
    float rotationCenterMatrix[16];
    float inverseRotationCenter[16];
    float scaleMatrix[16];
    float rotationMatrix[16];
    float translationMatrix[16];
    static const float oneZero[2] = {1.0f, 0.0f};
    static const float zeroOne[2] = {0.0f, 1.0f};
    PVec2 zeroPair = kFloat_0_MatrixMath;
    float one = kFloat_1_MatrixMath;
    out[0] = one;
    *(PVec2*)(out + 1) = zeroPair;
    *(PVec2*)(out + 3) = zeroPair;
    out[5] = one;
    *(PVec2*)(out + 6) = zeroPair;
    *(PVec2*)(out + 8) = zeroPair;
    out[10] = one;
    *(PVec2*)(out + 11) = zeroPair;
    *(PVec2*)(out + 13) = zeroPair;
    out[15] = one;

    // Phase 1: move into the scale center and scale-orientation frame, then scale.
    if (scalingCenter != 0) {
        const float* p = (const float*)scalingCenter;
        register float x, y, z;
        z = p[2];
        y = p[1];
        x = p[0];
        register float* m = scalingCenterMatrix;
        scalingCenterMatrix[12] = x;
        scalingCenterMatrix[13] = y;
        scalingCenterMatrix[14] = z;
        PVec2 oz = *(const PVec2*)oneZero;
        PVec2 zo = *(const PVec2*)zeroOne;
        *(PVec2*)m = oz;
        *(PVec2*)(m + 2) = zeroPair;
        *(PVec2*)(m + 4) = zo;
        *(PVec2*)(m + 6) = zeroPair;
        *(PVec2*)(m + 8) = zeroPair;
        *(PVec2*)(m + 10) = oz;
        scalingCenterMatrix[15] = one;
        Mtx44_InvertLegacyCofactors(inverseScalingCenter, 0, scalingCenterMatrix);
        PSMTX44Concat(out, out, inverseScalingCenter);
    }
    if (scalingRotation != 0) {
        Mtx44_FromQuaternion(scalingRotationMatrix, scalingRotation);
        // inverseScalingRotation is uninitialized until a successful inverse.
        // Do not assume zero determinant supplies identity or preserves out.
        Mtx44_InvertLegacyCofactors(inverseScalingRotation, 0, scalingRotationMatrix);
        PSMTX44Concat(out, out, inverseScalingRotation);
    }
    if (scale != 0) {
        const float* p = (const float*)scale;
        register float x, y, z;
        z = p[2];
        y = p[1];
        x = p[0];
        PVec2 zero = 0.0f;
        float one = kFloat_1_MatrixMath;
        register float* m = scaleMatrix;
        m[0] = x;
        *(PVec2*)(m + 1) = zero;
        *(PVec2*)(m + 3) = zero;
        m[5] = y;
        *(PVec2*)(m + 6) = zero;
        *(PVec2*)(m + 8) = zero;
        m[10] = z;
        *(PVec2*)(m + 11) = zero;
        *(PVec2*)(m + 13) = zero;
        m[15] = one;
        PSMTX44Concat(out, out, m);
    }
    // Phase 2: return from the scale frame, then move into the rotation-center frame.
    if (scalingRotation != 0) {
        PSMTX44Concat(out, out, scalingRotationMatrix);
    }
    if (scalingCenter != 0) {
        PSMTX44Concat(out, out, scalingCenterMatrix);
    }
    if (rotationCenter != 0) {
        const float* p = (const float*)rotationCenter;
        register float x, y, z;
        float one = kFloat_1_MatrixMath;
        PVec2 zero = 0.0f;
        PVec2 oz = *(const PVec2*)oneZero;
        PVec2 zo = *(const PVec2*)zeroOne;
        z = p[2];
        y = p[1];
        x = p[0];
        rotationCenterMatrix[12] = x;
        rotationCenterMatrix[13] = y;
        rotationCenterMatrix[14] = z;
        *(PVec2*)rotationCenterMatrix = oz;
        *(PVec2*)(rotationCenterMatrix + 2) = zero;
        *(PVec2*)(rotationCenterMatrix + 4) = zo;
        *(PVec2*)(rotationCenterMatrix + 6) = zero;
        *(PVec2*)(rotationCenterMatrix + 8) = zero;
        *(PVec2*)(rotationCenterMatrix + 10) = oz;
        rotationCenterMatrix[15] = one;
        Mtx44_InvertLegacyCofactors(inverseRotationCenter, 0, rotationCenterMatrix);
        PSMTX44Concat(out, out, inverseRotationCenter);
    }
    // Phase 3: apply rotation, undo its center shift and append final translation.
    if (rotation != 0) {
        Mtx44_FromQuaternion(rotationMatrix, rotation);
        PSMTX44Concat(out, out, rotationMatrix);
    }
    if (rotationCenter != 0) {
        PSMTX44Concat(out, out, rotationCenterMatrix);
    }
    if (translation != 0) {
        const float* p = (const float*)translation;
        register float x, y, z;
        float one = kFloat_1_MatrixMath;
        PVec2 zero = 0.0f;
        PVec2 oz = *(const PVec2*)oneZero;
        PVec2 zo = *(const PVec2*)zeroOne;
        z = p[2];
        y = p[1];
        x = p[0];
        translationMatrix[12] = x;
        translationMatrix[13] = y;
        translationMatrix[14] = z;
        *(PVec2*)translationMatrix = oz;
        *(PVec2*)(translationMatrix + 2) = zero;
        *(PVec2*)(translationMatrix + 4) = zo;
        *(PVec2*)(translationMatrix + 6) = zero;
        *(PVec2*)(translationMatrix + 8) = zero;
        *(PVec2*)(translationMatrix + 10) = oz;
        translationMatrix[15] = one;
        PSMTX44Concat(out, out, translationMatrix);
    }
    return destination;
}
#pragma pop

#pragma push
// Build a right-handed world-to-view matrix (forward maps toward -Z) from XYZ
// eye/target/up. Require distinct eye/target and nonparallel up for a useful view;
// degeneracy follows normalization fallback, with no error result. Keep basis
// construction, in-place transpose, then transformed negative-eye translation.
void* MTXLookAt(void* destination, const void* eyePosition, const void* targetPosition, const void* upDirection) {
    const float* eye = (const float*)eyePosition;
    const float* at = (const float*)targetPosition;
    float* out = (float*)destination;
    float forward[3];
    float normalizedUp[3];
    float side[3];
    float correctedUp[3];
    float negativeEye[3];

    *(PVec2*)forward = *(const PVec2*)at - *(const PVec2*)eye;
    forward[2] = at[2] - eye[2];
    Vec3_Normalize(forward, forward);
    Vec3_Normalize(normalizedUp, upDirection);

    side[0] = forward[1] * normalizedUp[2] - normalizedUp[1] * forward[2];
    side[1] = -(forward[0] * normalizedUp[2] - normalizedUp[0] * forward[2]);
    side[2] = -(forward[1] * normalizedUp[0] - normalizedUp[1] * forward[0]);
    Vec3_Normalize(side, side);

    correctedUp[0] = side[1] * forward[2] - forward[1] * side[2];
    correctedUp[1] = -(side[0] * forward[2] - forward[0] * side[2]);
    correctedUp[2] = -(side[1] * forward[0] - forward[1] * side[0]);
    Vec3_Normalize(correctedUp, correctedUp);

    float first = side[0];
    float zero = kFloat_0_MatrixMath;
    out[0] = first;
    float one = kFloat_1_MatrixMath;
    out[1] = side[1];
    out[2] = side[2];
    out[3] = zero;
    out[4] = correctedUp[0];
    out[5] = correctedUp[1];
    out[6] = correctedUp[2];
    out[7] = zero;
    out[8] = -forward[0];
    out[9] = -forward[1];
    float nz = -forward[2];
    out[11] = zero;
    out[12] = zero;
    out[10] = nz;
    out[13] = zero;
    out[14] = zero;
    out[15] = one;

    PSMTX44Transpose(destination, destination);
    float negone = kFloat_neg1_MatrixMath;
    *(PVec2*)negativeEye = *(const PVec2*)eye * negone;
    negativeEye[2] = eye[2] * negone;
    Mtx44_TransformPointPerspective(out + 12, negativeEye, destination);
    return destination;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Build RH (-Z-forward) perspective; fovy radians, aspect width/height, clips in
// view units. Writes float[16]; requires initialized depth convention. Preserve
// half-angle tan(double)->float, reciprocal order, and mode-1 coefficient anomaly.
void MtxPerspectiveFovRH(void* dst, float fovy, float aspect, float nearz, float farz) {
    float halfangle = kFloat_0_5_MatrixMath * fovy;
    float tangent = (float)tan((double)halfangle);
    float zero = kFloat_0_MatrixMath, one = kFloat_1_MatrixMath;
    float depth = nearz - farz, product = nearz * farz;
    float cotangent = one / tangent;
    volatile float* m = (volatile float*)dst;
    m[11] = kFloat_neg1_MatrixMath; m[1] = zero; m[2] = zero; m[3] = zero; m[4] = zero;
    float xscale = cotangent / aspect;
    m[6] = zero; m[5] = cotangent; m[7] = zero; m[8] = zero; m[9] = zero;
    float zscale = farz / depth;
    m[0] = xscale; m[12] = zero; m[13] = zero; m[15] = zero;
    float ztranslation = product / depth;
    m[10] = zscale; m[14] = ztranslation;
    if (*g_projectionDepthConvention == kLegacyProjectionDepthMode1) { float sum = nearz + farz; m[10] = sum / depth; }
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Build LH (+Z-forward) perspective with the same radians/aspect/clip contracts;
// changes sign of W/depth denominator relative to RH, not the vertical FOV policy.
void MtxPerspectiveFovLH(void* dst, float fovy, float aspect, float nearz, float farz) {
    float halfangle = kFloat_0_5_MatrixMath * fovy;
    float tangent = (float)tan((double)halfangle);
    float zero = kFloat_0_MatrixMath, one = kFloat_1_MatrixMath;
    float negative_near = -nearz, depth = farz - nearz;
    volatile float* m = (volatile float*)dst; m[1] = zero;
    float cotangent = one / tangent;
    m[11] = one; m[2] = zero; m[3] = zero; m[4] = zero; m[6] = zero;
    float product = negative_near * farz;
    m[5] = cotangent;
    float xscale = cotangent / aspect;
    m[7] = zero; m[8] = zero; m[9] = zero; m[12] = zero; m[13] = zero;
    float zscale = farz / depth;
    m[0] = xscale; m[15] = zero;
    float ztranslation = product / depth;
    m[10] = zscale; m[14] = ztranslation;
    if (*g_projectionDepthConvention == kLegacyProjectionDepthMode1) { float sum = nearz + farz; m[10] = sum / depth; }
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Write centered orthographic projection; nonzero width/height, near != far,
// initialized depth convention required. Extents and clips use the caller's units.
void Mtx44_SetOrthoCentered(void* dst, float width, float height, float nearz, float farz) {
    float two = kFloat_2_Math, depth = nearz - farz, zero = kFloat_0_MatrixMath, one = kFloat_1_MatrixMath;
    float sx = two / width; volatile float* m = (volatile float*)dst;
    m[1] = zero; m[15] = one; m[2] = zero; m[3] = zero;
    float sy = two / height;
    m[0] = sx; m[4] = zero; m[6] = zero; m[7] = zero; m[8] = zero;
    float sz = one / depth;
    m[9] = zero; m[11] = zero; m[5] = sy; m[12] = zero; m[13] = zero;
    float tz = nearz / depth; m[10] = sz; m[14] = tz;
    if (*g_projectionDepthConvention == kLegacyProjectionDepthMode1) { float sum = nearz + farz; m[14] = sum / depth; }
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Map off-center bounds to orthographic coordinates, with RH depth denominator;
// bounds/clips must differ and depth convention must be initialized. No checks.
void MtxOrthoOffCenter(void* dst, float left, float right, float bottom, float top, float nearz, float farz) {
    float depth = nearz - farz, zero = kFloat_0_MatrixMath, one = kFloat_1_MatrixMath;
    float width = right - left, two = kFloat_2_Math, height = top - bottom;
    float sx = two / width; volatile float* m = (volatile float*)dst;
    m[15] = one; m[1] = zero; m[2] = zero; m[3] = zero; m[4] = zero;
    float sy = two / height;
    m[0] = sx; m[6] = zero; m[7] = zero; m[8] = zero; m[9] = zero;
    float sz = one / depth; m[5] = sy; m[11] = zero;
    float tz = nearz / depth; m[10] = sz;
    float xsum = left + right; m[14] = tz;
    float xden = left - right, ysum = top + bottom, yden = bottom - top;
    float tx = xsum / xden, ty = ysum / yden;
    m[12] = tx; m[13] = ty;
    if (*g_projectionDepthConvention == kLegacyProjectionDepthMode1) { float sum = nearz + farz; m[14] = sum / depth; }
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Map off-center bounds with LH depth scale; legacy translation still uses
// near-far. Same nondegeneracy/global-depth preconditions as other ortho builders.
void Mtx44_SetOrthoOffCenterLH(void* dst, float left, float right, float bottom, float top, float nearz, float farz) {
    float depth = nearz - farz, zero = kFloat_0_MatrixMath, one = kFloat_1_MatrixMath;
    float reverse_depth = farz - nearz, width = right - left, two = kFloat_2_Math;
    float sz = one / reverse_depth; volatile float* m = (volatile float*)dst;
    m[15] = one; m[1] = zero; m[2] = zero; m[3] = zero; m[4] = zero;
    float tz = nearz / depth; m[10] = sz;
    m[6] = zero; m[7] = zero; m[8] = zero; m[9] = zero;
    float height = top - bottom; m[14] = tz;
    float xsum = left + right, xden = left - right; m[11] = zero;
    float sx = two / width, sy = two / height; m[0] = sx;
    float tx = xsum / xden; m[5] = sy;
    float ysum = top + bottom; m[12] = tx;
    float yden = bottom - top; m[13] = ysum / yden;
    if (*g_projectionDepthConvention == kLegacyProjectionDepthMode1) { float sum = nearz + farz; m[14] = sum / depth; }
}
#pragma pop

#pragma push
// Recover XYZW quaternion from a rotation matrix, choosing trace or largest-axis
// branch; no orthogonality check or quaternion renormalization. Keep branch ties.
void Quat_FromMatrix44(void* destination, void* sourceMatrix) {
    float* out = (float*)destination;
    const float* m = (const float*)sourceMatrix;
    float trace = kFloat_1_MatrixMath + (m[10] + (m[0] + m[5]));
    float r;
    float s;
    float h;
    float inv;
    float q0;
    float q1;
    float q2;
    float q3;

    if (trace > kQuaternionTraceThreshold) {
        s = Math_SquareRootApprox(trace);
        h = kFloat_0_5_MatrixMath * s;
        inv = kFloat_0_25_Quaternion / h;
        q0 = inv * (m[6] - m[9]);
        q1 = inv * (m[8] - m[2]);
        q2 = inv * (m[1] - m[4]);
        q3 = h;
    } else if (m[0] > m[5]) {
        if (m[0] > m[10]) {
            r = ((kFloat_1_MatrixMath + m[0]) - m[5]) - m[10];
            s = Math_SquareRootApprox(r);
            h = kFloat_0_5_MatrixMath * s;
            inv = kFloat_0_25_Quaternion / h;
            q0 = h;
            q1 = inv * (m[1] + m[4]);
            q2 = inv * (m[2] + m[8]);
            q3 = inv * (m[6] - m[9]);
        } else {
            r = ((kFloat_1_MatrixMath + m[10]) - m[0]) - m[5];
            s = Math_SquareRootApprox(r);
            h = kFloat_0_5_MatrixMath * s;
            inv = kFloat_0_25_Quaternion / h;
            q0 = inv * (m[8] + m[2]);
            q1 = inv * (m[9] + m[6]);
            q2 = h;
            q3 = inv * (m[1] - m[4]);
        }
    } else if (m[5] > m[10]) {
        r = ((kFloat_1_MatrixMath + m[5]) - m[10]) - m[0];
        s = Math_SquareRootApprox(r);
        h = kFloat_0_5_MatrixMath * s;
        inv = kFloat_0_25_Quaternion / h;
        q0 = inv * (m[1] + m[4]);
        q1 = h;
        q2 = inv * (m[6] + m[9]);
        q3 = inv * (m[8] - m[2]);
    } else {
        r = ((kFloat_1_MatrixMath + m[10]) - m[0]) - m[5];
        s = Math_SquareRootApprox(r);
        h = kFloat_0_5_MatrixMath * s;
        inv = kFloat_0_25_Quaternion / h;
        q0 = inv * (m[8] + m[2]);
        q1 = inv * (m[9] + m[6]);
        q2 = h;
        q3 = inv * (m[1] - m[4]);
    }
    out[0] = q0;
    out[1] = q1;
    out[2] = q2;
    out[3] = q3;
}
#pragma pop

#pragma push
// Evaluate plane[0..2] dot pointXYZ + plane[3], not normalized signed distance.
float gtl_Plane_EvaluatePoint(const float* a, const float* b) {
    return a[3] + (a[0] * b[0] + a[1] * b[1] + a[2] * b[2]);
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Forward XYZW transformation to the shared implementation without argument swaps.
void Mtx44_TransformVec4Forward(float* dst, const float* v, const float* m) { Mtx44_TransformVec4(dst, v, m); }
#pragma pop

#pragma push
// Replace basis orientation while retaining source row lengths/translation;
// forwardDirection is XYZ, useDefaultUp==0 borrows source up, nonzero uses shared
// up vector. useDefaultUp==0 with nonpositive up-row length returns NULL and leaves
// destination untouched; other degeneracies follow normalization fallback.
// Row lengths include FOUR components (Vec4_Length), not just affine XYZ. The
// destination's fourth row-components are scaled before being cleared below;
// retain that original access order even though their values are overwritten.
// Required destination admission: lanes [3], [7], [11] must already contain
// initialized finite floats before entry. The Vec3 writes below initialize only
// XYZ, then Vec4_Scale reads/multiplies all four lanes before the final zero
// stores. A fresh uninitialized output matrix is not an admitted destination;
// initialize it (normally affine lanes=0) or pass an initialized matrix in place.
// The billboard callers in gtl_animation_model_matrices.cpp/gtl_animation_node_evaluation.cpp use that in-place form. Keep the
// fourth-lane reads even though they do not contribute to the final matrix.
// Success returns destination; exact in-place sourceMatrix is supported.
float* Matrix_AlignForwardPreservingScaleAndTranslation(float* destination, float* sourceMatrix, float* forwardDirection, int useDefaultUp) {
    float rightScale = Vec4_Length(sourceMatrix);
    float upScale = Vec4_Length(sourceMatrix + 4);
    float forwardScale = Vec4_Length(sourceMatrix + 8);
    unsigned int upBits[3];
    float* up = (float*)upBits;

    upBits[0] = kMatrixDefaultUpVector[0];
    upBits[1] = kMatrixDefaultUpVector[1];
    upBits[2] = kMatrixDefaultUpVector[2];
    if (useDefaultUp == 0) {
        if (upScale <= kFloat_0_MatrixAccumulation) {
            return 0;
        }
        float c4 = sourceMatrix[4];
        float c5 = sourceMatrix[5];
        float c6 = sourceMatrix[6];
        up[0] = c4;
        up[1] = c5;
        up[2] = c6;
    }
    float* upRow = destination + 4;
    float* forwardRow = destination + 8;

    Vec3_Normalize(forwardRow, forwardDirection);
    Vec3_Cross(destination, up, forwardRow);
    Vec3_Normalize(destination, destination);
    Vec3_Cross(upRow, forwardRow, destination);
    Vec3_Normalize(upRow, upRow);
    Vec4_Scale(destination, destination, rightScale);
    Vec4_Scale(upRow, upRow, upScale);
    Vec4_Scale(forwardRow, forwardRow, forwardScale);

    float zero = kFloat_0_MatrixAccumulation;
    float c12 = sourceMatrix[12];
    float c13 = sourceMatrix[13];
    float c14 = sourceMatrix[14];
    float c15 = sourceMatrix[15];
    destination[3] = zero;
    destination[7] = zero;
    destination[11] = zero;
    destination[12] = c12;
    destination[13] = c13;
    destination[14] = c14;
    destination[15] = c15;
    return destination;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Copy XY after reading both components, supporting exact in-place operation.
void gtl_Vec2_Copy(void* dst, const void* input) {
    const float* s = (const float*)input; float x = s[0], y = s[1];
    float* d = (float*)dst; d[0] = x; d[1] = y;
}
#pragma pop

#pragma push
// Compare XY with exact float equality (NaNs unequal, signed zeros equal).
int Vec2_Equals(const float* a, const float* b) {
    int result = 0;
    if (a[0] == b[0] && a[1] == b[1]) {
        result = 1;
    }
    return result;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Copy XYZ as floats without a coordinate conversion; exact in-place allowed.
void Vector_CopyComponents(void* dst, const void* input) {
    const float* s = (const float*)input; float x = s[0], y = s[1], z = s[2];
    float* d = (float*)dst; d[0] = x; d[1] = y; d[2] = z;
}
#pragma pop

#pragma push
// Multiply XYZ componentwise after all reads, permitting exact in-place output.
void Vec3_MultiplyComponents(float* out, const float* a, const float* b)
{
    float a0 = a[0];
    float b0 = b[0];
    float a1 = a[1];
    float b1 = b[1];
    float a2 = a[2];
    float b2 = b[2];

    float prod0 = a0 * b0;
    float prod1 = a1 * b1;
    float prod2 = a2 * b2;

    out[0] = prod0;
    out[1] = prod1;
    out[2] = prod2;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Copy XYZW after reading all four components; exact in-place allowed.
void Vec4_Copy(void* dst, const void* input) {
    const float* s = (const float*)input; float x = s[0], y = s[1], z = s[2], w = s[3];
    float* d = (float*)dst; d[0] = x; d[1] = y; d[2] = z; d[3] = w;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Copy 16 matrix floats pairwise with volatile order; in-place allowed, partial
// overlaps retain this sequence rather than memmove semantics.
void Mtx44_Copy(void* dst, const void* input) {
    volatile float* d = (volatile float*)dst; const volatile float* s = (const volatile float*)input;
    for (int i = 0; i < 16; i += 2) {
        float x = s[i], y = s[i + 1]; d[i] = x; d[i + 1] = y;
    }
}
#pragma pop

#pragma push
// Build the legacy WView matrix from a required 24-byte MathViewport. Destination
// requires 16 floats, disjoint from viewport. X/Y origins are multiplied by half
// extents at indices 3/7/15; do not replace them with conventional additive origin
// offsets. Depth uses (near-far)/(2^24-1), with near/(2^24-1) translation.
void WView_BuildViewportTransform(void* destination, void* viewport) {
    unsigned int* in = (unsigned int*)viewport;
    float* out = (float*)destination;
    const unsigned int height = in[3], x = in[0], width = in[2], y = in[1];
    const float znear = ((float*)viewport)[4], zfar = ((float*)viewport)[5];
    float halfWidth, xOriginTerm, heightForYScale, heightForOrigin, originY;
    halfWidth = (float)width / 2.0f;
    xOriginTerm = ((float)width / 2.0f) * (float)x;
    heightForYScale = (float)height;
    heightForOrigin = (float)height;
    originY = (float)y;
    out[0] = halfWidth;
    out[1] = 0.0f;
    out[2] = 0.0f;
    out[3] = xOriginTerm;
    out[4] = 0.0f;
    out[5] = -(heightForYScale / 2.0f);
    out[6] = 0.0f;
    out[8] = 0.0f;
    out[9] = 0.0f;
    out[10] = (znear - zfar) / kViewportDepthMaximum;
    out[7] = -(heightForOrigin / 2.0f) * originY;
    out[11] = 0.0f;
    out[12] = (float)width / 2.0f;
    out[13] = (float)height / 2.0f;
    out[14] = znear / kViewportDepthMaximum;
    out[15] = 1.0f + (((float)width / 2.0f) * (float)x + ((float)height / 2.0f) * (float)y);
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Compose (a*b)*c into dst; c must survive the first write (disjoint from dst).
void* Mtx44_Concat3(void* dst, const void* a, const void* b, const void* c) {
    PSMTX44Concat(dst, a, b); PSMTX44Concat(dst, dst, c); return dst;
}
#pragma pop

#pragma push
// Compose ((a*b)*c)*d; later operands c/d must not overlap the written destination.
void* Mtx44_Concat4(void* dst, const void* a, const void* b, const void* c, const void* d) {
    PSMTX44Concat(dst, a, b);
    PSMTX44Concat(dst, dst, c);
    PSMTX44Concat(dst, dst, d);
    return dst;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Forward basis alignment with unchanged source/forward/mode contract.
float* Matrix_AlignForwardPreservingScaleAndTranslationWrapper(float* dst, float* first, float* second, int mode) {
    return Matrix_AlignForwardPreservingScaleAndTranslation(dst, first, second, mode);
}
#pragma pop

#pragma push
#pragma optimization_level 2
// Accumulate count packed matrices with count scalar weights into float[16];
// count==0 writes zero. All reads precede output stores; paired lanes/odd tail
// and weight order must be preserved (see packed-stream schema at declaration).
// Lane map (row-major matrix): sum0 -> out[0,1] (m00,m01), sum1 -> [2,3]
// (m02,m03), sum2 -> [4,5] (m10,m11), sum3 -> [6,7] (m12,m13), sum4 -> [8,9]
// (m20,m21), sum5 -> [10,11] (m22,m23), sum6 -> [12,13] (m30,m31), sum7 ->
// [14,15] (m32,m33). row[0..7] are one matrix; row[8..15] the next. w0
// broadcasts one scalar to both lanes. Each lane adds matrices in input order;
// a/b/c/d/e are only scheduled loads, never a transpose or cross-lane operation.
void Mtx44_WeightedSum(register void* out, register const void* data, register const void* weights, register unsigned int count) {
    register PVec2* mp = (PVec2*)data;
    register float* wp = (float*)weights;
    register unsigned int n = count;
    float zero = kFloat_0_MatrixAccumulation;
    register PVec2 sum1 = zero, sum2 = zero, sum3 = zero, sum4 = zero;
    register PVec2 sum5 = zero, sum6 = zero, sum7 = zero, sum0 = zero;
    register PVec2 w0;
    register PVec2 a, b, c, d, e;

    // One pass permits an early exit after an even number of matrices.
    do {
        if (n > 0U) {
            unsigned int half = n >> 1;
            if (half != 0U) {
                for (unsigned int loops = 0; loops < half; ++loops) {
                    w0 = wp[0];
                    register PVec2* row = mp;
                    a = row[0];
                    b = row[1];
                    c = row[2];
                    sum0 = sum0 + a * w0;
                    a = row[3];
                    sum1 = sum1 + b * w0;
                    d = row[4];
                    sum2 = sum2 + c * w0;
                    b = row[5];
                    c = row[6];
                    sum3 = sum3 + a * w0;
                    e = row[7];
                    sum4 = sum4 + d * w0;
                    sum5 = sum5 + b * w0;
                    a = row[8];
                    sum6 = sum6 + c * w0;
                    sum7 = sum7 + e * w0;
                    w0 = wp[1];
                    b = row[9];
                    c = row[10];
                    sum0 = sum0 + a * w0;
                    a = row[11];
                    d = row[12];
                    sum1 = sum1 + b * w0;
                    b = row[13];
                    sum2 = sum2 + c * w0;
                    c = row[14];
                    sum3 = sum3 + a * w0;
                    e = row[15];
                    sum4 = sum4 + d * w0;
                    wp += 2;
                    sum5 = sum5 + b * w0;
                    mp += 16;
                    sum6 = sum6 + c * w0;
                    sum7 = sum7 + e * w0;
                }
                n &= 1U;
                if (n == 0U) break;
            }
            do {
                w0 = *wp++;
                register PVec2* row = mp;
                a = row[0];
                b = row[1];
                c = row[2];
                sum0 = sum0 + a * w0;
                a = row[3];
                sum1 = sum1 + b * w0;
                d = row[4];
                sum2 = sum2 + c * w0;
                b = row[5];
                c = row[6];
                sum3 = sum3 + a * w0;
                e = row[7];
                sum4 = sum4 + d * w0;
                sum5 = sum5 + b * w0;
                mp += 8;
                sum6 = sum6 + c * w0;
                sum7 = sum7 + e * w0;
            } while (--n != 0U);
        }
    } while (0);
    register PVec2* dst = (PVec2*)out;
    // Commit all eight lane pairs after reading the inputs; out may alias an input matrix.
    dst[0] = sum0; dst[1] = sum1; dst[2] = sum2; dst[3] = sum3;
    dst[4] = sum4; dst[5] = sum5; dst[6] = sum6; dst[7] = sum7;
    return;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Borrow the module's static version string; caller must not free it.
void* RevoEX_GetVersionString(void) { return g_revoExVersionString; }
#pragma pop

#pragma push
#pragma dont_inline on
#pragma fp_contract off
// Forward a required caller-owned six-byte MAC output buffer to NCD; no local status.
void Network_GetWirelessMacAddress(void* input) { NCDiGetWirelessMacAddress(input); }
#pragma pop

}
