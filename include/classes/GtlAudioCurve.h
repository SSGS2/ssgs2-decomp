// gtl_AudioCurve_Evaluate in gtl_audio_spatial_and_runtime.cpp owns this view of a piecewise-linear curve.
// gtl_Audio_CalculatePanAndAttenuation checks mode and count, then reads the points before calling Evaluate.
#ifndef CLASSES_GTLAUDIOCURVE_H
#define CLASSES_GTLAUDIOCURVE_H
struct GtlAudioCurvePoint { float x, y; };
struct GtlAudioCurve {
    int mode; // 0x0: nonzero enables the curve in gtl_Audio_CalculatePanAndAttenuation.
    unsigned int count; // 0x4: number of points; Evaluate searches [0, count).
    GtlAudioCurvePoint* points; // 0x8: ordered x/y pairs used for interpolation.
};
#define OFF(T, f, o) typedef char T##_off_##f[((int)&((T*)0)->f == (o)) ? 1 : -1]
OFF(GtlAudioCurvePoint, x, 0x0);
OFF(GtlAudioCurvePoint, y, 0x4);
OFF(GtlAudioCurve, mode, 0x0);
OFF(GtlAudioCurve, count, 0x4);
OFF(GtlAudioCurve, points, 0x8);
#undef OFF
#endif
