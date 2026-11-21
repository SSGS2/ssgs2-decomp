// GTL Hermite animation callback: interpolate three components from two key records.
// API lookup: InterpolateHermiteVec3 selects outgoing(start)/incoming(end)
// tangents; Vec3_Hermite (gtl_vector_matrix_math.cpp) computes the basis and writes XYZ.
// Track consumers normalize key time to a segment weight before interpolation
// (see texture tracks in gtl_texture_animation_matrices.cpp). This callback has no seconds/frame-rate knob.
// Translation unit 0x803E02C0-0x803E02F0 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

void* Vec3_Hermite(void* dst, const void* p0, const void* startTangent, const void* endPoint, const void* t1, float t);

// Used prefix of a native, 4-byte-aligned key: float32 XYZ vectors in 16-byte
// lanes. The fourth word of each lane is not read by this callback. This view
// describes the 0x2C bytes required here; it does not set the track's key stride.
struct HermiteVec3KeyView {
    float value[3]; unsigned int unusedValueWord;       // +0x00
    float incoming[3]; unsigned int unusedIncomingWord; // +0x10
    float outgoing[3];                                // +0x20
};

// Required dst float[3], start/end readable key prefixes; no ownership transfer.
// weight is dimensionless, ordinarily [0,1], unclamped (outside extrapolates).
// Values and tangents share the track's component units/space; tangents are
// consumed directly as segment controls, with no key-duration rescale here.
// Vec3_Hermite reads all controls before writing, so dst may alias either key's
// value. No pointer/range validation or failure status: writes XYZ, returns 1.
int gtl_Animation_InterpolateHermiteVec3(void* dst, void* startKey, void* endKey, float weight) {
    Vec3_Hermite(dst, startKey, ((HermiteVec3KeyView*)startKey)->outgoing, endKey, ((HermiteVec3KeyView*)endKey)->incoming, weight);
    return 1;
}

} // extern "C"
