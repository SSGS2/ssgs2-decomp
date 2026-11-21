// WVector: the game's 3-float vector. static_R.sel exports the real mangled names of the API
// (__pl/__mi/__ml/__as/Magnitude/Normalize/WVectorLen/CalcDeltaAngle); the data layout is x, y, z.
// The game passes WVector BY VALUE as a pointer to a copy (12-byte struct), see GetGroundPoint__9CQuadTreeF7WVectorbPib.
#ifndef GAME_WVECTOR_H
#define GAME_WVECTOR_H

class WVector {
public:
    float x, y, z;

    WVector() {}
    WVector(float nx, float ny, float nz) : x(nx), y(ny), z(nz) {}

    WVector& operator=(const WVector& other);   // __as__7WVectorFRC7WVector (sel)
    WVector& operator+=(const WVector& other);  // __apl__7WVectorFRC7WVector (inferred, not in the sel)
    WVector& operator-=(const WVector& other);  // __ami__7WVectorFRC7WVector (inferred)
    WVector& operator*=(float scale);           // __amu__7WVectorFf (inferred)
    WVector& Normalize();                       // Normalize__7WVectorFv (sel)
    float Magnitude() const;                    // Magnitude__7WVectorCFv (sel)
};

WVector operator+(const WVector& a, const WVector& b);   // __pl__FRC7WVectorRC7WVector (sel)
WVector operator-(const WVector& a, const WVector& b);   // __mi__FRC7WVectorRC7WVector (sel)
WVector operator*(const WVector& v, float s);            // __ml__FRC7WVectorf (inferred)
WVector operator*(float s, const WVector& v);            // __ml__FfRC7WVector (sel)
float operator*(const WVector& a, const WVector& b);     // __ml__FRC7WVectorRC7WVector (sel, dot product)
float WVectorLen(const WVector& v);                      // WVectorLen__FRC7WVector (sel)
float CalcDeltaAngle(const WVector& a, const WVector& b);// CalcDeltaAngle__FRC7WVectorRC7WVector (sel; defined in raw_file_stream_and_matrix_utils.cpp)

#endif
