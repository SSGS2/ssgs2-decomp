// Vector and affine matrix math: dot and cross products, vector lengths and normalizing,
// matrix concatenation and inversion, transforms of points and directions, and nearly-equal tests.

#include "sdk_protos.h"
#include "fp_fused.h"
#include "game/wvector.h"

// Affine storage is four rows of three floats, used with ROW vectors:
//       [ m0 m1 m2 ]   local X basis in the destination space
//       [ m3 m4 m5 ]   local Y basis
//       [ m6 m7 m8 ]   local Z basis
//       [ m9 m10 m11 ] translation
// [x y z 1] * M is a point; [x y z 0] * M is a direction.
// Concat(a,b) forms a*b: transform by a, THEN by b. No space (world/view)
// is intrinsic to these helpers; callers choose the source/destination spaces.
// Parentheses, double accumulators and volatile spills preserve Wii rounding.

extern "C" {

// MSL/math.h classification values; ordering is part of Normalize's <= test.
enum MslFloatClass {
    kMslFloatNaN = 1,
    kMslFloatInfinite = 2
};

extern float kFloat_0_Matrix;
extern float kFloat_1_matrixQuaternion;
extern float kVectorComparisonEpsilon;       // 0.00001f (strict per-axis comparison).
extern float kVectorNormalizationRetryScale; // 100000.0f; rescale before the one retry.
float Math_SquareRootApprox(float value);

// Accumulate a three-component dot product in double precision, then round to float.
static inline float vdot3(float x1, float y1, float z1, float x2, float y2, float z2) {
    volatile float spill[4];
    double r = (double)x1 * (double)x2 + (double)y1 * (double)y2 + (double)z1 * (double)z2;
    spill[0] = r;
    return (float)r;
}

// Remove a float's sign using the original sign-bit test (including signed zero).
static inline float fabsd(float f) {
    union {
        float f;
        int i;
    } u;
    u.f = f;
    if (u.i & 0x80000000)
        f = -f;
    return f;
}

// Perform an ordered strict comparison; an unordered NaN comparison is false.
static inline int lt(float a, float b) {
    return a < b;
}

// Require every absolute component difference to be strictly below the shared epsilon.
static inline int isNear(const float* v, const float* z) {
    union U {
        float f;
        int i;
    };
    U u0, u1, u2;
    float a0, a1, a2;
    u0.f = v[0] - z[0];
    a0 = u0.f;
    if (u0.i & 0x80000000)
        a0 = -a0;
    if (lt(a0, kVectorComparisonEpsilon)) {
        u1.f = v[1] - z[1];
        a1 = u1.f;
        if (u1.i & 0x80000000)
            a1 = -a1;
        if (lt(a1, kVectorComparisonEpsilon)) {
            u2.f = v[2] - z[2];
            a2 = u2.f;
            if (u2.i & 0x80000000)
                a2 = -a2;
            if (lt(a2, kVectorComparisonEpsilon))
                return 1;
        }
    }
    return 0;
}

// Return squared vector length with the dot helper's double accumulation.
static inline float vlen2(const float* a) {
    float x = a[0], y = a[1], z = a[2];
    float x2, y2, z2;
    x2 = x;
    y2 = y;
    z2 = z;
    return vdot3(x, y, z, x2, y2, z2);
}
}

// Normalize in place, retrying exceptional reciprocal lengths; return this vector.
// Algorithm: preserve exact zero; compute 1/sqrt(dot(v,v)); if that reciprocal
// is NaN/infinite, zero NaN components, discard a near-zero result, or enlarge
// the components and recompute once. Finally multiply by the chosen reciprocal.
// There is no second classification after the retry: infinities can still yield
// NaNs. Do not replace this policy with a generic safe-normalize helper.
WVector& WVector::Normalize() {
    float* value = &x;
    float zv[3];
    float reciprocal;
    float z0 = kFloat_0_Matrix;
    if (z0 == value[0] && z0 == value[1] && z0 == value[2]) {
        value[0] = z0;
        value[1] = z0;
        value[2] = z0;
        return *this;
    }
    // Ordinary path: double dot accumulation, rounded for the square-root call.
    {
        float x0 = value[0];
        float x3 = x0;
        float y1 = value[1];
        float y4 = y1;
        float z2 = value[2];
        float z5 = z2;
        double f10 = (double)x0 * (double)x3;
        f10 = (double)y1 * (double)y4 + f10;
        f10 = (double)z2 * (double)z5 + f10;
        volatile float tmp[4];
        tmp[0] = f10;
        reciprocal = kFloat_1_matrixQuaternion / Math_SquareRootApprox((float)f10);
    }
    // MSL classification codes 1/2 select the NaN/infinity recovery path.
    // Only class-1 components are replaced with zero. If the resulting vector
    // is near zero, multiply it by zero; otherwise scale by 100000 and retry.
    // This is the existing exceptional-input policy, not a finite-length guard.
    if (__fpclassifyf(reciprocal) <= kMslFloatInfinite) {
        float zero = kFloat_0_Matrix;
        float* p = value;
        for (int i = 0; i < 3; ++i, ++p)
            if (__fpclassifyf(*p) == kMslFloatNaN)
                *p = zero;
        zv[0] = kFloat_0_Matrix;
        zv[1] = kFloat_0_Matrix;
        zv[2] = kFloat_0_Matrix;
        if (isNear(value, zv)) {
            float x = value[0];
            float s = kFloat_0_Matrix;
            float y = value[1], z = value[2];
            x *= s;
            y *= s;
            z *= s;
            value[0] = x;
            value[1] = y;
            value[2] = z;
            return *this;
        }
        float x = value[0] * kVectorNormalizationRetryScale, y = value[1] * kVectorNormalizationRetryScale,
              z = value[2] * kVectorNormalizationRetryScale;
        value[0] = x;
        value[1] = y;
        value[2] = z;
        {
            float x0 = value[0];
            float x3 = value[0];
            float y1 = value[1];
            float y4 = value[1];
            float z2 = value[2];
            float z5 = value[2];
            double f10 = (double)x0 * (double)x3;
            f10 = (double)y1 * (double)y4 + f10;
            f10 = (double)z2 * (double)z5 + f10;
            volatile float tmp[4];
            tmp[0] = (float)f10;
            reciprocal = kFloat_1_matrixQuaternion / Math_SquareRootApprox((float)f10);
        }
    }
    // Both paths converge here; these stores also permit in-place normalization.
    {
        float x = value[0], y = value[1], z = value[2];
        x *= reciprocal;
        y *= reciprocal;
        z *= reciprocal;
        value[0] = x;
        value[1] = y;
        value[2] = z;
    }
    return *this;
}

extern "C" {

// Replace a with a*b; all results are staged before stores, permitting a == b.
void Matrix_ConcatAffineAssign(float* a, const float* b) {
    volatile float tmp[12];
    float d0, d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11;
    d0 = (a[0] * b[0] + a[1] * b[3]) + a[2] * b[6];
    tmp[0] = d0;
    d1 = (a[0] * b[1] + a[1] * b[4]) + a[2] * b[7];
    tmp[1] = d1;
    d2 = (a[0] * b[2] + a[1] * b[5]) + a[2] * b[8];
    tmp[2] = d2;
    d3 = (a[3] * b[0] + a[4] * b[3]) + a[5] * b[6];
    tmp[3] = d3;
    d4 = (a[3] * b[1] + a[4] * b[4]) + a[5] * b[7];
    tmp[4] = d4;
    d5 = (a[3] * b[2] + a[4] * b[5]) + a[5] * b[8];
    tmp[5] = d5;
    d6 = (a[6] * b[0] + a[7] * b[3]) + a[8] * b[6];
    tmp[6] = d6;
    d7 = (a[6] * b[1] + a[7] * b[4]) + a[8] * b[7];
    tmp[7] = d7;
    d8 = (a[6] * b[2] + a[7] * b[5]) + a[8] * b[8];
    tmp[8] = d8;
    d9 = b[9] + ((a[9] * b[0] + a[10] * b[3]) + a[11] * b[6]);
    tmp[9] = d9;
    d10 = b[10] + ((a[9] * b[1] + a[10] * b[4]) + a[11] * b[7]);
    tmp[10] = d10;
    d11 = b[11] + ((a[9] * b[2] + a[10] * b[5]) + a[11] * b[8]);
    tmp[11] = d11;
    a[0] = d0;
    a[1] = d1;
    a[2] = d2;
    a[3] = d3;
    a[4] = d4;
    a[5] = d5;
    a[6] = d6;
    a[7] = d7;
    a[8] = d8;
    a[9] = d9;
    a[10] = d10;
    a[11] = d11;
}

// Return 1 only when all three absolute differences are strictly below epsilon.
int Vec3_NearlyEqual(const float* lhs, const float* rhs) {
    float tmp[3];
    float dx, dy, dz;
    dx = lhs[0] - rhs[0];
    tmp[0] = dx;
    dx = (*(unsigned int*)&tmp[0] & 0x80000000) ? -dx : dx;
    unsigned int c0 = dx < kVectorComparisonEpsilon;
    if (c0) {
        dy = lhs[1] - rhs[1];
        tmp[1] = dy;
        dy = (*(unsigned int*)&tmp[1] & 0x80000000) ? -dy : dy;
        unsigned int c1 = dy < kVectorComparisonEpsilon;
        if (c1) {
            dz = lhs[2] - rhs[2];
            tmp[2] = dz;
            dz = (*(unsigned int*)&tmp[2] & 0x80000000) ? -dz : dz;
            unsigned int c2 = dz < kVectorComparisonEpsilon;
            if (c2)
                return 1;
        }
    }
    return 0;
}

// Copy an affine matrix, scale its nine basis elements and preserve translation.
void Matrix_CopyScaledBasis(float* dst, const float* src, float scale) {
    float v9, v11, v10;
    v9 = src[9];
    v10 = src[10];
    v11 = src[11];
    float g0[3], g1[3], g2[3];
    float z0 = src[2] * scale;
    float y0 = src[1] * scale;
    float x0 = src[0] * scale;
    float z1 = src[5] * scale;
    float y1 = src[4] * scale;
    float x1 = src[3] * scale;
    float z2 = src[8] * scale;
    float y2 = src[7] * scale;
    float x2 = src[6] * scale;
    g0[0] = x0;
    g0[1] = y0;
    g0[2] = z0;
    dst[0] = g0[0];
    dst[1] = g0[1];
    dst[2] = g0[2];
    g1[0] = x1;
    g1[1] = y1;
    g1[2] = z1;
    dst[3] = g1[0];
    dst[4] = g1[1];
    dst[5] = g1[2];
    g2[0] = x2;
    g2[1] = y2;
    g2[2] = z2;
    dst[6] = g2[0];
    dst[7] = g2[1];
    dst[8] = g2[2];
    dst[9] = v9;
    dst[10] = v10;
    dst[11] = v11;
}

// Scale the nine contiguous basis elements in place; translation is outside this range.
void Matrix3x3_ScaleAssign(float* value, float scale) {
    float a8, a7, a6, a5, a4, a3, a2, a1, a0;
    a0 = value[0] * scale;
    a1 = value[1] * scale;
    a2 = value[2] * scale;
    a3 = value[3] * scale;
    a4 = value[4] * scale;
    a5 = value[5] * scale;
    a6 = value[6] * scale;
    a7 = value[7] * scale;
    a8 = value[8] * scale;
    value[0] = a0;
    value[1] = a1;
    value[2] = a2;
    value[3] = a3;
    value[4] = a4;
    value[5] = a5;
    value[6] = a6;
    value[7] = a7;
    value[8] = a8;
}
}

// Return vector length using the shared approximate square root.
float WVectorLen(const WVector& v) {
    float x0, x3, y1, y4, z2, z5;
    x0 = v.x;
    x3 = v.x;
    y1 = v.y;
    y4 = v.y;
    z2 = v.z;
    z5 = v.z;
    double f10 = (double)x0 * (double)x3;
    f10 = (double)y1 * (double)y4 + f10;
    f10 = (double)z2 * (double)z5 + f10;
    volatile float tmp[4];
    tmp[0] = (float)f10;
    return Math_SquareRootApprox((float)f10);
}

extern "C" {}

// Return this vector's length after double-precision accumulation of its square.
float WVector::Magnitude() const {
    float x0 = x;
    float x3 = x;
    float y1 = y;
    float y4 = y;
    float z2 = z;
    float z5 = z;
    double f10 = (double)x0 * (double)x3;
    f10 = (double)y1 * (double)y4 + f10;
    f10 = (double)z2 * (double)z5 + f10;
    volatile float tmp[4];
    tmp[0] = (float)f10;
    return Math_SquareRootApprox((float)f10);
}
#pragma opt_common_subs reset

extern "C" {

// Copy twelve affine elements, loading the entire source before writing the destination.
void Matrix_CopyAffine(float* dst, const float* src) {
    float l, k, j, i, h, g, f, e, d, c, b, a;
    a = src[0];
    b = src[1];
    c = src[2];
    d = src[3];
    e = src[4];
    f = src[5];
    g = src[6];
    h = src[7];
    i = src[8];
    j = src[9];
    k = src[10];
    l = src[11];
    dst[0] = a;
    dst[1] = b;
    dst[2] = c;
    dst[3] = d;
    dst[4] = e;
    dst[5] = f;
    dst[6] = g;
    dst[7] = h;
    dst[8] = i;
    dst[9] = j;
    dst[10] = k;
    dst[11] = l;
}
}

// Return the component-wise sum without modifying either input.
WVector operator+(const WVector& lhs, const WVector& rhs) {
    float z = lhs.z + rhs.z;
    float y = lhs.y + rhs.y;
    float x = lhs.x + rhs.x;
    return WVector(x, y, z);
}

extern "C" {}

// Return the component-wise difference lhs-rhs.
WVector operator-(const WVector& lhs, const WVector& rhs) {
    float z = lhs.z - rhs.z;
    float y = lhs.y - rhs.y;
    float x = lhs.x - rhs.x;
    return WVector(x, y, z);
}

extern "C" {}

// Add another vector in place and return this vector.
WVector& WVector::operator+=(const WVector& other) {
    float nx = x + other.x;
    float ny = y + other.y;
    float nz = z + other.z;
    x = nx;
    y = ny;
    z = nz;
    return *this;
}

extern "C" {}

#pragma opt_lifetimes on
// Return the dot product with double accumulation and one final float rounding.
float operator*(const WVector& lhs, const WVector& rhs) {
    float x0, x3, x1, x4, x2, x5;
    x0 = lhs.x;
    x3 = rhs.x;
    x1 = lhs.y;
    x4 = rhs.y;
    x2 = lhs.z;
    x5 = rhs.z;
    double result = (double)x0 * (double)x3;
    result = (double)x1 * (double)x4 + result;
    result = (double)x2 * (double)x5 + result;
    volatile float tmp[4];
    tmp[0] = (float)result;
    return (float)result;
}
#pragma opt_lifetimes reset

extern "C" {

// Return the three-component dot product with the original x/y/z accumulation order.
float Vec3_DotProduct(const float* lhs, const float* rhs) {
    float f2 = lhs[2];
    float f0 = lhs[0];

    float f1 = lhs[1];
    float f3 = rhs[0];
    double f10 = (double)f0 * (double)f3;

    float f4 = rhs[1];
    f10 = (double)f1 * (double)f4 + f10;

    volatile float tmp[4];
    f10 = (double)f2 * (double)(rhs[2]) + f10;
    tmp[0] = (float)f10;
    return (float)f10;
}

// Transform a point (implicit w=1), including the matrix translation row.
// dst/value may be the same three-float vector; matrix supplies all twelve floats.
// This point operation includes translation; Matrix_TransformDirection uses w=0.
void* Matrix_TransformPoint(float* dst, const float* value, const float* matrix) {
    typedef __vec2x32float__ Pair;
    union Packed {
        Pair pair;
        float f[2];
    } a, b, v, w, out;
    a.pair = *(const Pair*)(matrix + 0);
    b.pair = *(const Pair*)(matrix + 3);
    Pair c = *(const Pair*)(matrix + 6);
    Pair d = *(const Pair*)(matrix + 9);
    v.f[0] = value[0];
    v.f[1] = value[0];
    w.f[0] = value[2];
    w.f[1] = value[2];
    out.pair = w.pair * c + v.pair * a.pair;
    v.f[0] = value[1];
    v.f[1] = value[1];
    // The original rounds y*m before adding the translation lane.
    volatile Pair ym = v.pair * b.pair;
    out.pair = out.pair + (ym + d);
    float x = out.f[0], y = out.f[1];
    volatile float ymz = value[1] * matrix[5];
    float z = (value[2] * matrix[8] + value[0] * matrix[2]) + (ymz + matrix[11]);
    dst[1] = y;
    dst[0] = x;
    dst[2] = z;
    return dst;
}

// Transform a direction (implicit w=0), retaining the original zero*translation terms.
// clang-format off
#ifdef __MWERKS__
asm void Matrix_TransformDirection(float* dst, const float* value, const float* matrix) {
    nofralloc
    stwu r1, -64(r1)
    stfd f31, 48(r1)
    psq_st f31, 56(r1), 0, 0
    stfd f30, 32(r1)
    psq_st f30, 40(r1), 0, 0
    psq_l f0, 0(r5), 0, 0
    li r0, 0
    psq_l f1, 12(r5), 0, 0
    stw r0, 8(r1)
    ps_merge00 f2, f0, f1
    psq_l f10, 8(r5), 1, 0
    ps_merge11 f4, f0, f1
    psq_l f11, 20(r5), 1, 0
    stw r0, 12(r1)
    psq_l f0, 24(r5), 0, 0
    psq_l f1, 36(r5), 0, 0
    ps_merge00 f6, f10, f11
    ps_merge11 f8, f10, f11
    psq_l f10, 32(r5), 1, 0
    psq_l f11, 44(r5), 1, 0
    ps_merge00 f3, f0, f1
    ps_merge11 f5, f0, f1
    psq_l f0, 0(r4), 0, 0
    psq_l f1, 8(r4), 1, 0
    ps_merge00 f7, f10, f11
    psq_l f13, 8(r1), 0, 0
    ps_merge11 f9, f10, f11
    ps_mul f10, f0, f2
    ps_merge00 f1, f1, f13
    ps_mul f2, f0, f4
    ps_madd f11, f1, f3, f10
    ps_madd f3, f1, f5, f2
    ps_mul f10, f0, f6
    ps_sum0 f12, f11, f11, f11
    ps_mul f2, f0, f8
    ps_madd f11, f1, f7, f10
    ps_sum1 f13, f3, f12, f3
    ps_madd f3, f1, f9, f2
    ps_sum0 f4, f11, f11, f11
    psq_st f13, 16(r1), 0, 0
    ps_sum1 f13, f3, f4, f3
    lfs f30, 20(r1)
    lfs f31, 16(r1)
    stfs f30, 4(r3)
    psq_st f13, 24(r1), 0, 0
    lfs f30, 24(r1)
    stfs f31, 0(r3)
    stfs f30, 8(r3)
    psq_l f31, 56(r1), 0, 0
    lfd f31, 48(r1)
    psq_l f30, 40(r1), 0, 0
    lfd f30, 32(r1)
    addi r1, r1, 64
    blr
}
#else
// clang-format on
void Matrix_TransformDirection(float* dst, const float* value, const float* matrix) {
    // Same 4x3 row-major layout as Matrix_TransformPoint (rows m[0..2],
    // m[3..5], m[6..8], translation m[9..11]), but with w = 0: the original
    // loads (v2, 1.0) and merges it with a zeroed stack pair (+0x18 li r0,0;
    // +0x20/+0x34 stw r0,0x8/0xc(r1); +0x64 psq_l f13,0x8(r1); +0x70
    // ps_merge00 f1,f1,f13), so the translation row is multiplied by 0.
    // x = (v2*m6 + v0*m0) + (0*m9 + v1*m3) (+0x6c ps_mul, +0x78 ps_madd,
    // +0x84 ps_sum0); y, z likewise with m1/m4/m7/m10 and m2/m5/m8/m11.
    // Stores at +0x110/+0x11c/+0x120: dst[1], dst[0], dst[2].
    float x = value[0], y = value[1], z = value[2];
    float w = 0.0f;
    float ox = (z * matrix[6] + x * matrix[0]) + (w * matrix[9] + y * matrix[3]);
    float oy = (z * matrix[7] + x * matrix[1]) + (w * matrix[10] + y * matrix[4]);
    float oz = (z * matrix[8] + x * matrix[2]) + (w * matrix[11] + y * matrix[5]);
    dst[1] = oy;
    dst[0] = ox;
    dst[2] = oz;
}
#endif

// Store a*b (apply a then b); stage all twelve results so dst may alias either input.
void Matrix_ConcatAffine(float* dst, const float* a, const float* b) {
    float x0 = a[0] * b[0] + a[1] * b[3] + a[2] * b[6];
    float x1 = a[0] * b[1] + a[1] * b[4] + a[2] * b[7];
    float x2 = a[0] * b[2] + a[1] * b[5] + a[2] * b[8];
    float x3 = a[3] * b[0] + a[4] * b[3] + a[5] * b[6];
    float x4 = a[3] * b[1] + a[4] * b[4] + a[5] * b[7];
    float x5 = a[3] * b[2] + a[4] * b[5] + a[5] * b[8];
    float x6 = a[6] * b[0] + a[7] * b[3] + a[8] * b[6];
    float x7 = a[6] * b[1] + a[7] * b[4] + a[8] * b[7];
    float x8 = a[6] * b[2] + a[7] * b[5] + a[8] * b[8];
    float x9 = b[9] + (a[9] * b[0] + a[10] * b[3] + a[11] * b[6]);
    float x10 = b[10] + (a[9] * b[1] + a[10] * b[4] + a[11] * b[7]);
    float x11 = b[11] + (a[9] * b[2] + a[10] * b[5] + a[11] * b[8]);
    dst[0] = x0;
    dst[1] = x1;
    dst[2] = x2;
    dst[3] = x3;
    dst[4] = x4;
    dst[5] = x5;
    dst[6] = x6;
    dst[7] = x7;
    dst[8] = x8;
    dst[9] = x9;
    dst[10] = x10;
    dst[11] = x11;
}

// Invert the basis by cofactors and set translation to -t*inverseBasis; no singularity check.
void Matrix_InvertAffine(float* dst, const float* m) {
    // Basis labels: [a b c; d e f; g h i]. A/B/C are the first-column
    // cofactors of the inverse numerator: ei-fh, fg-di, dh-eg.
    // det = a*A + b*B + c*C. A zero determinant is divided into 1 directly;
    // singular/NaN inputs therefore retain the original IEEE exceptional results.
    volatile float spill[4];
    float inv, tz, tx, ty;
    float bi, c, a, b, B, d, A, i, h;
    i = m[8];
    d = m[3];
    float di = d * i;
    b = m[1];
    bi = b * i;
    float bd = b * d;
    float f = m[5];
    h = m[7];
    float g = m[6];
    float e = m[4];
    c = m[2];
    a = m[0];
    B = f * g - di;
    A = e * i - f * h;
    float C = d * h - e * g;
    spill[2] = C;
    spill[0] = A;
    spill[1] = B;
    // Phase 1: obtain 1/det. Preserve these spills and arithmetic association.
    inv = kFloat_1_matrixQuaternion / (a * A + b * B + c * C);
    float r10, r11, r12, r00, r01, r02, r20, r21, r22;
    // Phase 2: write the adjugate/det basis; rRC denotes inverse row R, column C.
    r10 = inv * B;
    r11 = inv * (a * i - c * g);
    r12 = inv * (c * d - a * f);
    tx = m[9];
    ty = m[10];
    tz = m[11];
    dst[3] = r10;
    dst[5] = r12;
    dst[4] = r11;
    r00 = inv * A;
    r01 = inv * (c * h - bi);
    r02 = inv * (b * f - c * e);
    dst[0] = r00;
    dst[1] = r01;
    dst[2] = r02;
    r20 = inv * C;
    r21 = inv * (b * g - a * h);
    r22 = inv * (a * e - b * d);
    dst[6] = r20;
    dst[7] = r21;
    dst[8] = r22;
    float t2, t1, t0;
    // Phase 3: translation cancels the original offset in the inverse space.
    t0 = -(tx * r00 + ty * r10 + tz * r20);
    t1 = -(tx * r01 + ty * r11 + tz * r21);
    t2 = -(tx * r02 + ty * r12 + tz * r22);
    dst[9] = t0;
    dst[10] = t1;
    dst[11] = t2;
}
}

// Subtract another vector in place and return this vector.
WVector& WVector::operator-=(const WVector& other) {
    float nx = x - other.x;
    float ny = y - other.y;
    float nz = z - other.z;
    x = nx;
    y = ny;
    z = nz;
    return *this;
}

extern "C" {

// Negate each component in place.
void Vec3_NegateInPlace(float* value) {
    value[0] = -value[0];
    value[1] = -value[1];
    value[2] = -value[2];
}

// Store src*(1/scale); zero scale follows IEEE division without validation.
void Vec3_DivideScalar(float* dst, float scale, const float* src) {
    float reciprocal = kFloat_1_matrixQuaternion / scale;
    float z = src[2] * reciprocal;
    float y = src[1] * reciprocal;
    float x = src[0] * reciprocal;
    dst[2] = z;
    dst[0] = x;
    dst[1] = y;
}

// Divide in place using a single reciprocal, preserving the original rounding.
void Vec3_DivideAssign(float* value, float scale) {
    float reciprocal = kFloat_1_matrixQuaternion / scale;
    float x = value[0] * reciprocal;
    float y = value[1] * reciprocal;
    float z = value[2] * reciprocal;
    value[0] = x;
    value[1] = y;
    value[2] = z;
}
}

// Return a scaled vector without modifying src.
WVector operator*(const WVector& src, float scale) {
    float x, y, z;
    z = src.z;
    y = src.y;
    x = src.x;
    z *= scale;
    y *= scale;
    x *= scale;
    return WVector(x, y, z);
}

extern "C" {}

// Return a scaled vector for scalar-first multiplication.
WVector operator*(float scale, const WVector& src) {
    float x, y, z;
    z = src.z;
    y = src.y;
    x = src.x;
    z *= scale;
    y *= scale;
    x *= scale;
    return WVector(x, y, z);
}

extern "C" {}

// Scale each component in place and return this vector.
WVector& WVector::operator*=(float scale) {
    float z, y, x;
    x = this->x;
    y = this->y;
    z = this->z;
    x *= scale;
    y *= scale;
    z *= scale;
    this->x = x;
    this->y = y;
    this->z = z;
    return *this;
}

extern "C" {

// Set all three components to the shared zero constant.
void Vec3_Zero(float* value) {
    float zero = kFloat_0_Matrix;
    value[2] = zero;
    value[1] = zero;
    value[0] = zero;
}
}

// Copy all components and return this vector, including for self-assignment.
WVector& WVector::operator=(const WVector& src) {
    float z, y, x;
    x = src.x;
    y = src.y;
    z = src.z;
    this->x = x;
    this->y = y;
    this->z = z;
    return *this;
}

extern "C" {

// Return x*x+y*y+z*z with this helper's original float evaluation order.
float Vec3_CalculateLengthSquared(const float* value) {
    return value[2] * value[2] + (value[0] * value[0] + value[1] * value[1]);
}

// Set the three source-axis basis rows in destination coordinates; clear translation.
// Argument order is X row, Y row, Z row, each in XYZ component order.
void Matrix_SetBasisElements(float* value, float xBasisX, float xBasisY, float xBasisZ, float yBasisX, float yBasisY,
                             float yBasisZ, float zBasisX, float zBasisY, float zBasisZ) {
    float zero = kFloat_0_Matrix;
    value[0] = xBasisX;
    value[1] = xBasisY;
    value[2] = xBasisZ;
    value[3] = yBasisX;
    value[4] = yBasisY;
    value[5] = yBasisZ;
    value[6] = zBasisX;
    value[7] = zBasisY;
    value[8] = zBasisZ;
    value[11] = zero;
    value[10] = zero;
    value[9] = zero;
}

// Set the affine identity: unit basis and zero translation.
void Matrix_SetIdentityAffine(float* value) {
    float zero = kFloat_0_Matrix;
    float one = kFloat_1_matrixQuaternion;
    value[11] = zero;
    value[8] = one;
    value[4] = one;
    value[0] = one;
    value[10] = zero;
    value[9] = zero;
    value[7] = zero;
    value[6] = zero;
    value[5] = zero;
    value[3] = zero;
    value[2] = zero;
    value[1] = zero;
}

// Store lhs cross rhs; dst must not alias either input because stores are interleaved.
void Vector_CrossProduct(float* dst, const float* lhs, const float* rhs) {
    dst[2] = FP_FMSUBS(lhs[0], rhs[1], lhs[1] * rhs[0]);
    dst[1] = FP_FMSUBS(lhs[2], rhs[0], lhs[0] * rhs[2]);
    dst[0] = FP_FMSUBS(lhs[1], rhs[2], lhs[2] * rhs[1]);
}

} // extern "C"
