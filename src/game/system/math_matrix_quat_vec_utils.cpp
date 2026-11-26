// Math helpers for 2D and 3D vectors, affine and 4x4 matrices, quaternions, planes and
// bounding boxes (axis-aligned and oriented). Functions copy, combine and test these values,
// and build rotation matrices from Euler angles and from an axis and angle.

#include <decomp/static_init.h>
#include "game/types.h"
#include "fp_fused.h"

extern "C" {

extern const float kFloat_6_2831855;
extern float s_twoPi_8004CF18;
extern const float kFloat_0_Matrix;
extern const float kFloat_0_5_Quaternion;
extern const float kFloat_1_matrixQuaternion;
extern const float kFloat_0_05_MatrixQuaternion;
extern const float kFloat_1_5707964_Quaternion;

// Returns double: the original rounds the result with frsp f28, f1 at 0x8004D848.
extern double acos(double);
extern float Math_SinApprox(float);
extern float Math_CosApprox(float);

struct Mat4_8004CF18 {
    float v[16];
};
struct EulerAngles {
    float x, y, z;
};
extern void Matrix_SetBasisElements(void*, float, float, float, float, float, float, float, float, float);
// Build the affine rotation from Euler X/Y/Z angles, preserving the original multiply and rounding order.
void Matrix_FromEulerAngles(Mat4_8004CF18* dst, const EulerAngles* angles) {
    struct Affine {
        float v[12];
    } combinedXY, rotationX, rotationY, rotationZ;
    float sinZ, negativeSinZ, cosZ;
    cosZ = Math_CosApprox(angles->z);
    negativeSinZ = -Math_SinApprox(angles->z);
    sinZ = Math_SinApprox(angles->z);
    float cosZForBasis = Math_CosApprox(angles->z);
    Matrix_SetBasisElements(&rotationZ, cosZForBasis, sinZ, kFloat_0_Matrix, negativeSinZ, cosZ, kFloat_0_Matrix,
                            kFloat_0_Matrix, kFloat_0_Matrix, kFloat_1_matrixQuaternion);
    Matrix_SetBasisElements(&rotationY, Math_CosApprox(angles->y), kFloat_0_Matrix, Math_SinApprox(angles->y),
                            kFloat_0_Matrix, kFloat_1_matrixQuaternion, kFloat_0_Matrix, -Math_SinApprox(angles->y),
                            kFloat_0_Matrix, Math_CosApprox(angles->y));
    Matrix_SetBasisElements(&rotationX, kFloat_1_matrixQuaternion, kFloat_0_Matrix, kFloat_0_Matrix, kFloat_0_Matrix,
                            Math_CosApprox(angles->x), -Math_SinApprox(angles->x), kFloat_0_Matrix,
                            Math_SinApprox(angles->x), Math_CosApprox(angles->x));
    combinedXY.v[0] =
        rotationX.v[2] * rotationY.v[6] + (rotationX.v[0] * rotationY.v[0] + rotationX.v[1] * rotationY.v[3]);
    combinedXY.v[1] =
        rotationX.v[2] * rotationY.v[7] + (rotationX.v[0] * rotationY.v[1] + rotationX.v[1] * rotationY.v[4]);
    combinedXY.v[2] =
        rotationX.v[2] * rotationY.v[8] + (rotationX.v[0] * rotationY.v[2] + rotationX.v[1] * rotationY.v[5]);
    combinedXY.v[3] =
        rotationX.v[5] * rotationY.v[6] + (rotationX.v[3] * rotationY.v[0] + rotationX.v[4] * rotationY.v[3]);
    combinedXY.v[4] =
        rotationX.v[5] * rotationY.v[7] + (rotationX.v[3] * rotationY.v[1] + rotationX.v[4] * rotationY.v[4]);
    combinedXY.v[5] =
        rotationX.v[5] * rotationY.v[8] + (rotationX.v[3] * rotationY.v[2] + rotationX.v[4] * rotationY.v[5]);
    combinedXY.v[6] =
        rotationX.v[8] * rotationY.v[6] + (rotationX.v[6] * rotationY.v[0] + rotationX.v[7] * rotationY.v[3]);
    combinedXY.v[7] =
        rotationX.v[8] * rotationY.v[7] + (rotationX.v[6] * rotationY.v[1] + rotationX.v[7] * rotationY.v[4]);
    combinedXY.v[8] =
        rotationX.v[8] * rotationY.v[8] + (rotationX.v[6] * rotationY.v[2] + rotationX.v[7] * rotationY.v[5]);
    combinedXY.v[9] = rotationY.v[9] + (rotationX.v[11] * rotationY.v[6] +
                                        (rotationX.v[9] * rotationY.v[0] + rotationX.v[10] * rotationY.v[3]));
    combinedXY.v[10] = rotationY.v[10] + (rotationX.v[11] * rotationY.v[7] +
                                          (rotationX.v[9] * rotationY.v[1] + rotationX.v[10] * rotationY.v[4]));
    combinedXY.v[11] = rotationY.v[11] + (rotationX.v[11] * rotationY.v[8] +
                                          (rotationX.v[9] * rotationY.v[2] + rotationX.v[10] * rotationY.v[5]));
    dst->v[0] =
        combinedXY.v[2] * rotationZ.v[6] + (combinedXY.v[0] * rotationZ.v[0] + combinedXY.v[1] * rotationZ.v[3]);
    dst->v[1] =
        combinedXY.v[2] * rotationZ.v[7] + (combinedXY.v[0] * rotationZ.v[1] + combinedXY.v[1] * rotationZ.v[4]);
    dst->v[2] =
        combinedXY.v[2] * rotationZ.v[8] + (combinedXY.v[0] * rotationZ.v[2] + combinedXY.v[1] * rotationZ.v[5]);
    dst->v[3] =
        combinedXY.v[5] * rotationZ.v[6] + (combinedXY.v[3] * rotationZ.v[0] + combinedXY.v[4] * rotationZ.v[3]);
    dst->v[4] =
        combinedXY.v[5] * rotationZ.v[7] + (combinedXY.v[3] * rotationZ.v[1] + combinedXY.v[4] * rotationZ.v[4]);
    dst->v[5] =
        combinedXY.v[5] * rotationZ.v[8] + (combinedXY.v[3] * rotationZ.v[2] + combinedXY.v[4] * rotationZ.v[5]);
    dst->v[6] =
        combinedXY.v[8] * rotationZ.v[6] + (combinedXY.v[6] * rotationZ.v[0] + combinedXY.v[7] * rotationZ.v[3]);
    dst->v[7] =
        combinedXY.v[8] * rotationZ.v[7] + (combinedXY.v[6] * rotationZ.v[1] + combinedXY.v[7] * rotationZ.v[4]);
    dst->v[8] =
        combinedXY.v[8] * rotationZ.v[8] + (combinedXY.v[6] * rotationZ.v[2] + combinedXY.v[7] * rotationZ.v[5]);
    dst->v[9] = rotationZ.v[9] + (combinedXY.v[11] * rotationZ.v[6] +
                                  (combinedXY.v[9] * rotationZ.v[0] + combinedXY.v[10] * rotationZ.v[3]));
    dst->v[10] = rotationZ.v[10] + (combinedXY.v[11] * rotationZ.v[7] +
                                    (combinedXY.v[9] * rotationZ.v[1] + combinedXY.v[10] * rotationZ.v[4]));
    dst->v[11] = rotationZ.v[11] + (combinedXY.v[11] * rotationZ.v[8] +
                                    (combinedXY.v[9] * rotationZ.v[2] + combinedXY.v[10] * rotationZ.v[5]));
}

struct CQuaternion {
    float v[4];
};
// Interpolate quaternions along the shortest arc, using linear weights for nearly equal orientations.
void Quat_Slerp(CQuaternion* dst, const CQuaternion* start, const CQuaternion* end, float fraction) {
    // 0x8004D794/0x8004D7A8: fmuls f1 = start[1]*end[1], then fmadds start[0]*end[0] + f1 (fused order matters for rounding).
    float dot = start->v[1] * end->v[1];
    dot = start->v[0] * end->v[0] + dot;
    dot = start->v[2] * end->v[2] + dot;
    dot = start->v[3] * end->v[3] + dot;
    float adjustedStart[4];
    if (dot < kFloat_0_Matrix) {
        adjustedStart[0] = -start->v[0];
        adjustedStart[1] = -start->v[1];
        adjustedStart[2] = -start->v[2];
        adjustedStart[3] = -start->v[3];
        dot = -dot;
    } else {
        adjustedStart[0] = start->v[0];
        adjustedStart[1] = start->v[1];
        adjustedStart[2] = start->v[2];
        adjustedStart[3] = start->v[3];
    }
    float startWeight, endWeight;
    if (kFloat_0_05_MatrixQuaternion < kFloat_1_matrixQuaternion + dot) {
        if (kFloat_1_matrixQuaternion - dot < kFloat_0_05_MatrixQuaternion) {
            startWeight = kFloat_1_matrixQuaternion - fraction;
            endWeight = fraction;
        } else {
            float angle = (float)acos(dot); // frsp at 0x8004D848
            float sine = Math_SinApprox(angle);
            startWeight = Math_SinApprox(angle * (kFloat_1_matrixQuaternion - fraction)) / sine;
            endWeight = Math_SinApprox(angle * fraction) / sine;
        }
        dst->v[3] = startWeight * adjustedStart[3] + endWeight * end->v[3];
        dst->v[2] = startWeight * adjustedStart[2] + endWeight * end->v[2];
        dst->v[1] = startWeight * adjustedStart[1] + endWeight * end->v[1];
        dst->v[0] = startWeight * adjustedStart[0] + endWeight * end->v[0];
    } else {
        float orthogonalStartWeight =
            Math_SinApprox(kFloat_1_5707964_Quaternion * (kFloat_1_matrixQuaternion - fraction));
        float orthogonalEndWeight = Math_SinApprox(kFloat_1_5707964_Quaternion * fraction);
        dst->v[3] = orthogonalStartWeight * adjustedStart[3] + orthogonalEndWeight * adjustedStart[2];
        dst->v[1] = orthogonalStartWeight * adjustedStart[1] + orthogonalEndWeight * adjustedStart[0];
        dst->v[0] = orthogonalStartWeight * adjustedStart[0] - orthogonalEndWeight * adjustedStart[1];
        dst->v[2] = orthogonalStartWeight * adjustedStart[2] - orthogonalEndWeight * adjustedStart[3];
    }
}

struct Vec2_8004D5EC {
    float x;
    float y;
};

typedef Vec3f Vec3_8004DF6C;

struct CAabb {
    float v[6];
};
// Test all three min/max intervals; touching faces count as overlap.
int AABB_Overlaps(const CAabb* a, const CAabb* b) {
    const float* ap = a->v;
    const float* bp = b->v;
    for (int i = 0; i < 3; ++i, ++ap, ++bp) {
        if (ap[0] > bp[3] || ap[3] < bp[0])
            return 0;
    }
    return 1;
}

// Copy bounds and add the translation to both corners, preserving the original load/store order.
void AABB_CopyTranslated(CAabb* dst, const CAabb* src, const Vec3_8004DF6C* offset) {
    float maxZ;
    float y;
    float offsetZ;
    float minZ;
    float minY;
    float maxY;
    float minX;

    *dst = *src;
    __typeof__(offset->y) firstYRead = offset->y;
    float maxX;
    __typeof__(firstYRead) secondYRead = offset->y;
    maxX = (dst->v[3] + offset->x);
    minX = dst->v[0] + offset->x;
    float yValue = offset->y;
    y = yValue;
    float yValue2 = y;
    maxY = dst->v[4] + yValue2;
    minY = dst->v[1] + y;
    offsetZ = offset->z;
    maxZ = dst->v[5] + offsetZ;
    minZ = (dst->v[2] + offsetZ);
    dst->v[3] = maxX;
    dst->v[0] = minX;

    dst->v[4] = maxY;
    dst->v[1] = minY;
    dst->v[5] = maxZ;
    dst->v[2] = minZ;
}

// Copy all three source components to the destination.
void Vec3_CopySourceToDestination(Vec3_8004DF6C* src, Vec3_8004DF6C* dst) {
    float z;
    float y;
    float x;
    x = src->x;
    y = src->y;
    z = src->z;
    dst->x = x;
    dst->y = y;
    dst->z = z;
}

// Add the right-hand vector to this vector.
void Vec2_AddAssign(Vec2_8004D5EC* p, const Vec2_8004D5EC* rhs) {
    p->x += rhs->x;
    p->y += rhs->y;
}

// Return the component-wise sum of two vectors.
Vec2_8004D5EC Vec2_Add(const Vec2_8004D5EC* a, const Vec2_8004D5EC* b) {
    Vec2_8004D5EC result;
    result.y = a->y + b->y;
    result.x = a->x + b->x;
    return result;
}

// Return the component-wise difference of two vectors.
Vec2_8004D5EC Vec2_Subtract(const Vec2_8004D5EC* a, const Vec2_8004D5EC* b) {
    Vec2_8004D5EC result;
    result.y = a->y - b->y;
    result.x = a->x - b->x;
    return result;
}

struct Vec6_8004DA70 {
    float v[6];
};

// Copy the minimum and maximum bounds corners.
void AABB_Copy(Vec6_8004DA70* dst, const Vec6_8004DA70* src) {
    float maxZ, maxY, maxX, minZ, minY, minX;
    minX = src->v[0];
    minY = src->v[1];
    minZ = src->v[2];
    maxX = src->v[3];
    maxY = src->v[4];
    maxZ = src->v[5];
    dst->v[0] = minX;
    dst->v[1] = minY;
    dst->v[2] = minZ;
    dst->v[3] = maxX;
    dst->v[4] = maxY;
    dst->v[5] = maxZ;
}

// Return a vector multiplied by the scalar.
Vec2_8004D5EC Vec2_ScaleByFloat(const Vec2_8004D5EC* p, float scale) {
    Vec2_8004D5EC result;
    result.y = scale * p->y;
    result.x = scale * p->x;
    return result;
}

// Copy both vector components.
void Vec2_Copy(Vec2_8004D5EC* dst, const Vec2_8004D5EC* src) {
    dst->x = src->x;
    dst->y = src->y;
}

// Multiply both vector components in place.
void Vec2_ScaleAssign(Vec2_8004D5EC* p, float scale) {
    p->x *= scale;
    p->y *= scale;
}

struct Vec4_8004D664 {
    float x;
    float y;
    float z;
    float w;
};

struct FloatStorage12_8004DB20 {
    float v[12];
}; // Four Vec3 slots; bounds readers also use its first six floats.
struct CMatrix4x4 {
    float v[16];
}; // Full homogeneous matrix storage.
struct Temp_8004DB84 {
    float x, y, z;
    Temp_8004DB84() {}
    Temp_8004DB84(float a, float b, float c) {
        x = a;
        y = b;
        z = c;
    }
    Temp_8004DB84 operator*(float s) const {
        Temp_8004DB84 r;
        r.x = x * s;
        r.y = y * s;
        r.z = z * s;
        return r;
    }
};
// Convert min/max corners into a center and three axis-aligned half-extent vectors.
void Bounds_BuildCenterAndHalfExtentAxes(CMatrix4x4* dst, const FloatStorage12_8004DB20* src) {
    float zero, minZOrHalf, sourceComponent, maxX, sumX, maxY, sumZ, sumY, extentX, unused, extentY, extentZ;
    zero = kFloat_0_Matrix;
    sourceComponent = src->v[5];
    minZOrHalf = src->v[2]; // Max Z and min Z.
    maxY = src->v[4];
    sumX = src->v[1];
    sumZ = sourceComponent + minZOrHalf;
    extentZ = sourceComponent - minZOrHalf;
    maxX = src->v[3];
    sumY = maxY + sumX;
    sourceComponent = src->v[0];
    extentY = maxY - sumX;
    minZOrHalf = kFloat_0_5_Quaternion; // Reuse the min-Z temporary as the half scale.
    sumX = maxX + sourceComponent;
    extentX = maxX - sourceComponent;
    *(Temp_8004DB84*)&dst->v[0] = Temp_8004DB84(sumX, sumY, sumZ) * minZOrHalf;
    *(Temp_8004DB84*)&dst->v[3] = Temp_8004DB84(extentX, zero, zero) * minZOrHalf;
    *(Temp_8004DB84*)&dst->v[6] = Temp_8004DB84(zero, extentY, zero) * minZOrHalf;
    *(Temp_8004DB84*)&dst->v[9] = Temp_8004DB84(zero, zero, extentZ) * minZOrHalf;
}
extern void* memcpy(void*, const void*, unsigned int);

struct CPlane {
    float x, y, z, w;
};

// Copy the normal and plane-distance term, returning the destination.
Vec4_8004D664* Plane_Copy(Vec4_8004D664* dst, const Vec4_8004D664* src) {
    memcpy(dst, src, sizeof(Vec3f));
    dst->w = src->w;
    return dst;
}

// Copy four contiguous three-float affine vectors.
void Matrix_CopyAffineElements(FloatStorage12_8004DB20* dst, const FloatStorage12_8004DB20* src) {
    float translationZ, translationY, translationX, basisZZ, basisZY, basisZX, basisYZ, basisYY, basisYX, basisXZ,
        basisXY, basisXX;
    basisXX = src->v[0];
    basisXY = src->v[1];
    basisXZ = src->v[2];
    basisYX = src->v[3];
    basisYY = src->v[4];
    basisYZ = src->v[5];
    basisZX = src->v[6];
    basisZY = src->v[7];
    basisZZ = src->v[8];
    translationX = src->v[9];
    translationY = src->v[10];
    translationZ = src->v[11];
    dst->v[0] = basisXX;
    dst->v[1] = basisXY;
    dst->v[2] = basisXZ;
    dst->v[3] = basisYX;
    dst->v[4] = basisYY;
    dst->v[5] = basisYZ;
    dst->v[6] = basisZX;
    dst->v[7] = basisZY;
    dst->v[8] = basisZZ;
    dst->v[9] = translationX;
    dst->v[10] = translationY;
    dst->v[11] = translationZ;
}

// Copy translation first, followed by the three basis vectors.
void Matrix_MoveLastVectorToFront(FloatStorage12_8004DB20* dst, const FloatStorage12_8004DB20* src) {
    float basisZZ, basisZY, basisZX, basisYZ, basisYY, basisYX, basisXZ, basisXY, basisXX, translationZ, translationY,
        translationX;
    translationX = src->v[9];
    translationY = src->v[10];
    translationZ = src->v[11];
    basisXX = src->v[0];
    basisXY = src->v[1];
    basisXZ = src->v[2];
    basisYX = src->v[3];
    basisYY = src->v[4];
    basisYZ = src->v[5];
    basisZX = src->v[6];
    basisZY = src->v[7];
    basisZZ = src->v[8];
    dst->v[0] = translationX;
    dst->v[1] = translationY;
    dst->v[2] = translationZ;
    dst->v[3] = basisXX;
    dst->v[4] = basisXY;
    dst->v[5] = basisXZ;
    dst->v[6] = basisYX;
    dst->v[7] = basisYY;
    dst->v[8] = basisYZ;
    dst->v[9] = basisZX;
    dst->v[10] = basisZY;
    dst->v[11] = basisZZ;
}

// Scale affine vectors 1, 2 and 3 by X, Y and Z, leaving vector 0 unchanged.
void AffineVectors_ScaleTrailingThree(FloatStorage12_8004DB20* matrix, const Vec3_8004DF6C* scale) {
    float scaledTranslationZ, scaledTranslationY, scaledTranslationX;
    float scaledRow2Z, scaledRow2Y, scaledRow2X;
    float scaledRow1Z, scaledRow1Y, scaledRow1X;
    float x = scale->x;
    float z;
    float y = scale->y;
    z = scale->z;
    scaledRow1X = matrix->v[3] * x;
    scaledRow1Y = matrix->v[4] * x;
    scaledRow1Z = matrix->v[5] * x;
    scaledRow2X = matrix->v[6] * y;
    scaledRow2Y = matrix->v[7] * y;
    scaledRow2Z = matrix->v[8] * y;
    scaledTranslationX = matrix->v[9] * z;
    scaledTranslationY = matrix->v[10] * z;
    scaledTranslationZ = matrix->v[11] * z;
    matrix->v[3] = scaledRow1X;
    matrix->v[4] = scaledRow1Y;
    matrix->v[5] = scaledRow1Z;
    matrix->v[6] = scaledRow2X;
    matrix->v[7] = scaledRow2Y;
    matrix->v[8] = scaledRow2Z;
    matrix->v[9] = scaledTranslationX;
    matrix->v[10] = scaledTranslationY;
    matrix->v[11] = scaledTranslationZ;
}

// Insert homogeneous zero/one components into four three-float affine vectors.
void Matrix_ExpandAffineTo4x4(const FloatStorage12_8004DB20* src, CMatrix4x4* dst) {
    float one;
    float translationZ;
    float translationY;
    float translationX;
    float basisZZ;
    float basisZY;
    float basisZX;
    float basisYZ;
    float basisYY;
    float basisYX;
    float zero;
    float basisXZ;
    zero = kFloat_0_Matrix;
    float basisXY;
    float basisXX;
    basisXX = src->v[0];
    basisXY = src->v[1];
    basisXZ = src->v[2];
    basisYX = src->v[3];
    basisYY = src->v[4];
    basisYZ = src->v[5];
    basisZX = src->v[6];
    basisZY = src->v[7];
    basisZZ = src->v[8];
    translationX = src->v[9];
    translationY = src->v[10];
    translationZ = src->v[11];

    one = kFloat_1_matrixQuaternion;
    dst->v[0] = basisXX;
    dst->v[1] = basisXY;
    dst->v[2] = basisXZ;
    dst->v[3] = zero;
    dst->v[4] = basisYX;
    dst->v[5] = basisYY;
    dst->v[6] = basisYZ;
    dst->v[7] = zero;
    dst->v[8] = basisZX;
    dst->v[9] = basisZY;
    dst->v[10] = basisZZ;
    dst->v[11] = zero;
    dst->v[12] = translationX;
    dst->v[13] = translationY;
    dst->v[14] = translationZ;
    dst->v[15] = one;
}

// Drop homogeneous components from a sixteen-float matrix into twelve affine elements.
void Matrix_ExtractAffineFrom4x4(const CMatrix4x4* src, FloatStorage12_8004DB20* dst) {
    float translationZ, translationY, translationX, basisZZ, basisZY, basisZX, basisYZ, basisYY, basisYX, basisXZ,
        basisXY, basisXX;
    basisXX = src->v[0];
    basisXY = src->v[1];
    basisXZ = src->v[2];
    basisYX = src->v[4];
    basisYY = src->v[5];
    basisYZ = src->v[6];
    basisZX = src->v[8];
    basisZY = src->v[9];
    basisZZ = src->v[10];
    translationX = src->v[12];
    translationY = src->v[13];
    translationZ = src->v[14];
    dst->v[0] = basisXX;
    dst->v[1] = basisXY;
    dst->v[2] = basisXZ;
    dst->v[3] = basisYX;
    dst->v[4] = basisYY;
    dst->v[5] = basisYZ;
    dst->v[6] = basisZX;
    dst->v[7] = basisZY;
    dst->v[8] = basisZZ;
    dst->v[9] = translationX;
    dst->v[10] = translationY;
    dst->v[11] = translationZ;
}

// Returns dst (r3 is left untouched, blr at 0x8004D70C); its caller passes it on (0x80066430).
// Build a plane from its normal and a point, with distance equal to their negative dot product.
CPlane* Plane_FromNormalAndPoint(CPlane* dst, const Vec3_8004DF6C* normal, const Vec3_8004DF6C* point) {
    float normalX = normal->x;
    float pointX = point->x;
    double dotDouble = (double)normalX * (double)pointX;
    float normalY = normal->y;
    float pointY = point->y;
    float normalZ = normal->z;
    float pointZ = point->z;
    dotDouble = (double)normalY * (double)pointY + dotDouble;
    dst->x = normalX;
    dst->y = normalY;
    dotDouble = (double)normalZ * (double)pointZ + dotDouble;
    dst->z = normalZ;
    float dot = (float)dotDouble;
    dst->w = -dot;
    return dst;
}

struct MatrixStorage16_8004D4C8 {
    float v[16];
}; // Axis-angle scratch holds a full 4x4; basis-only setters write fewer elements.
extern void
Mtx44_SetRotationAxisAngle(MatrixStorage16_8004D4C8*, const void* axis,
                           float); // (dst, axis, angle): the original passes r4 through untouched (0x8004D4C8)
extern float Math_CosApprox(float);
extern float Math_SinApprox(float);

enum RotationAxis {
    kRotateX = 0,
    kRotateY = 1,
    kRotateZ = 2
};

// Build an X, Y or Z rotation; unknown axis values follow the Y path.
void Matrix_SetAxisRotation(MatrixStorage16_8004D4C8* matrix, float angle, signed char axis) {
    float cosine = Math_CosApprox(angle);
    float sine = Math_SinApprox(angle);
    switch (axis) {
    case kRotateX:
        Matrix_SetBasisElements(matrix, kFloat_1_matrixQuaternion, kFloat_0_Matrix, kFloat_0_Matrix, kFloat_0_Matrix,
                                cosine, -sine, kFloat_0_Matrix, sine, cosine);
        break;
    case kRotateZ:
        Matrix_SetBasisElements(matrix, cosine, sine, kFloat_0_Matrix, -sine, cosine, kFloat_0_Matrix, kFloat_0_Matrix,
                                kFloat_0_Matrix, kFloat_1_matrixQuaternion);
        break;
    case kRotateY:
    default:
        Matrix_SetBasisElements(matrix, cosine, kFloat_0_Matrix, sine, kFloat_0_Matrix, kFloat_1_matrixQuaternion,
                                kFloat_0_Matrix, -sine, kFloat_0_Matrix, cosine);
        break;
    }
}

struct OrientedBounds {
    Vec3f center;
    Vec3f halfExtentAxes[3]; // Each vector has direction and half extent combined.
};

#pragma push
#pragma optimization_level 1
// Test a point against all three oriented half-extent axes using absolute projections.
int OBB_ContainsPoint(const OrientedBounds* bounds, const float* point) {
    float xWork, yWork, zWork, axisX, axisY, axisZ, centerXOrSquaredExtent, projectionMagnitude, pointComponent;
    double dotDouble;
    volatile float deltaX, deltaY, deltaZ;
    int inside;
    // Keep double accumulation and volatile deltas: rounding and reload order are observable.
    projectionMagnitude = point[0];
    centerXOrSquaredExtent = bounds->center.x;
    axisX = bounds->halfExtentAxes[0].x;
    xWork = projectionMagnitude - centerXOrSquaredExtent;
    pointComponent = point[1];
    centerXOrSquaredExtent = bounds->center.y;
    dotDouble = (double)xWork * (double)axisX;
    axisY = bounds->halfExtentAxes[0].y;
    projectionMagnitude = bounds->center.z;
    yWork = pointComponent - centerXOrSquaredExtent;
    pointComponent = point[2];
    axisZ = bounds->halfExtentAxes[0].z;
    zWork = pointComponent - projectionMagnitude;
    centerXOrSquaredExtent = kFloat_0_Matrix;
    dotDouble = (double)yWork * (double)axisY + dotDouble;
    deltaX = xWork;
    deltaY = yWork;
    dotDouble = (double)zWork * (double)axisZ + dotDouble;
    deltaZ = zWork;
    projectionMagnitude = (float)dotDouble;
    projectionMagnitude = projectionMagnitude > kFloat_0_Matrix ? projectionMagnitude : -projectionMagnitude;
    xWork = bounds->halfExtentAxes[0].x;
    yWork = bounds->halfExtentAxes[0].y;
    axisX = xWork;
    zWork = bounds->halfExtentAxes[0].z;
    axisY = yWork;
    axisZ = zWork;
    dotDouble = (double)xWork * (double)axisX;
    dotDouble = (double)yWork * (double)axisY + dotDouble;
    dotDouble = (double)zWork * (double)axisZ + dotDouble;
    centerXOrSquaredExtent = (float)dotDouble;
    inside = projectionMagnitude <= centerXOrSquaredExtent;
    // Repeat the projection test for the next half-extent axis only while inside.
    if (inside) {
        xWork = deltaX;
        axisX = bounds->halfExtentAxes[1].x;
        yWork = deltaY;
        dotDouble = (double)xWork * (double)axisX;
        axisY = bounds->halfExtentAxes[1].y;
        zWork = deltaZ;
        axisZ = bounds->halfExtentAxes[1].z;
        centerXOrSquaredExtent = kFloat_0_Matrix;
        dotDouble = (double)yWork * (double)axisY + dotDouble;
        dotDouble = (double)zWork * (double)axisZ + dotDouble;
        projectionMagnitude = (float)dotDouble;
        projectionMagnitude = projectionMagnitude > kFloat_0_Matrix ? projectionMagnitude : -projectionMagnitude;
        xWork = bounds->halfExtentAxes[1].x;
        yWork = bounds->halfExtentAxes[1].y;
        axisX = xWork;
        zWork = bounds->halfExtentAxes[1].z;
        axisY = yWork;
        axisZ = zWork;
        dotDouble = (double)xWork * (double)axisX;
        dotDouble = (double)yWork * (double)axisY + dotDouble;
        dotDouble = (double)zWork * (double)axisZ + dotDouble;
        centerXOrSquaredExtent = (float)dotDouble;
        inside = projectionMagnitude <= centerXOrSquaredExtent;
    }
    if (inside) {
        xWork = deltaX;
        axisX = bounds->halfExtentAxes[2].x;
        yWork = deltaY;
        dotDouble = (double)xWork * (double)axisX;
        axisY = bounds->halfExtentAxes[2].y;
        zWork = deltaZ;
        axisZ = bounds->halfExtentAxes[2].z;
        centerXOrSquaredExtent = kFloat_0_Matrix;
        dotDouble = (double)yWork * (double)axisY + dotDouble;
        dotDouble = (double)zWork * (double)axisZ + dotDouble;
        projectionMagnitude = (float)dotDouble;
        projectionMagnitude = projectionMagnitude > kFloat_0_Matrix ? projectionMagnitude : -projectionMagnitude;
        xWork = bounds->halfExtentAxes[2].x;
        yWork = bounds->halfExtentAxes[2].y;
        axisX = xWork;
        zWork = bounds->halfExtentAxes[2].z;
        axisY = yWork;
        axisZ = zWork;
        dotDouble = (double)xWork * (double)axisX;
        dotDouble = (double)yWork * (double)axisY + dotDouble;
        dotDouble = (double)zWork * (double)axisZ + dotDouble;
        centerXOrSquaredExtent = (float)dotDouble;
        inside = projectionMagnitude <= centerXOrSquaredExtent;
    }
    return inside;
}

#pragma pop
// Build an axis-angle rotation with negated angle and copy its basis elements.
void Matrix_SetNegativeAxisAngleRotation(MatrixStorage16_8004D4C8* matrix, const void* axis, float angle) {
    MatrixStorage16_8004D4C8 temp;
    float value;
    Mtx44_SetRotationAxisAngle(&temp, axis, -angle);
    value = temp.v[10];
    Matrix_SetBasisElements(matrix, temp.v[0], temp.v[1], temp.v[2], temp.v[4], temp.v[5], temp.v[6], temp.v[8],
                            temp.v[9], value);
}

// Return the cursor vector multiplied by the scalar.
Vec2_8004D5EC CursorVec2_Scale(const Vec2_8004D5EC* p, float scale) {
    Vec2_8004D5EC result;
    result.y = scale * p->y;
    result.x = scale * p->x;
    return result;
}

// Evaluate normal dot point plus distance for a plane supplied first.
float Plane_EvaluatePoint(const Vec4_8004D664* plane, const Vec4_8004D664* point) {
    float sum = plane->y * point->y;
    sum = FP_FMADDS(plane->x, point->x, sum);
    sum = FP_FMADDS(plane->z, point->z, sum);
    return plane->w + sum;
}

// Evaluate normal dot point plus distance for a plane supplied second.
float Plane_EvaluatePointReversed(const Vec4_8004D664* point, const Vec4_8004D664* plane) {
    float sum = plane->y * point->y;
    sum = FP_FMADDS(plane->x, point->x, sum);
    sum = FP_FMADDS(plane->z, point->z, sum);
    return plane->w + sum;
}

// Copy the three vector components into the output.
void Vec3_CopyToOutput(Vec3_8004DF6C* src, Vec3_8004DF6C* dst) {
    float z;
    float y;
    float x;
    x = src->x;
    y = src->y;
    z = src->z;
    dst->x = x;
    dst->y = y;
    dst->z = z;
}

extern float Math_SquareRootApprox(float);

// Initialize this math module's full-turn angle constant.
void MatrixQuaternion_InitializeAngleConstant() {
    s_twoPi_8004CF18 = kFloat_6_2831855;
}

// Clear both vector components.
void Vec2_Zero(Vec2_8004D5EC* p) {
    float zero = kFloat_0_Matrix;
    p->y = zero;
    p->x = zero;
}

// Return the vector length using the game's square-root approximation.
float Vec2_CalculateLength(Vec2_8004D5EC* p) {
    return Math_SquareRootApprox(p->x * p->x + p->y * p->y);
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80461FD0", _ctors_u8004CF18_0, MatrixQuaternion_InitializeAngleConstant);
#pragma pop
