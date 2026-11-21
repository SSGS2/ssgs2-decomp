// GtlMediaSample: layout view; methods in gtl_media_pipeline_filter_manager.cpp (gtl_CMediaSample_Ctor; __vt__Q23gtl12CMediaSample).
// Inferred descriptive type name; no vtable, RTTI, constructors or base classes emitted.
#ifndef CLASSES_GTLMEDIASAMPLE_H
#define CLASSES_GTLMEDIASAMPLE_H

class GtlMediaSample {
public:
    int references; // 0x0
    void* vtbl; // 0x4
    int format; // 0x8
    int data; // 0xC
    int size; // 0x10
};

#define OFF(f, o) typedef char GtlMediaSample_off_##f[((int)&((GtlMediaSample*)0)->f == (o)) ? 1 : -1]
OFF(references, 0x0);
OFF(vtbl, 0x4);
OFF(format, 0x8);
OFF(data, 0xC);
OFF(size, 0x10);
#undef OFF

#endif
