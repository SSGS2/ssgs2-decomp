// GtlOwnedBuffer803F5428: layout view; methods in gtl_media_sample_and_sampling_filter.cpp (dtor_803F5644 and GtlOwnedBuffer_dtor zero count and free data).
// No virtual methods, inheritance, or emitted runtime objects.
#ifndef CLASSES_GTLOWNEDBUFFER803F5428_H
#define CLASSES_GTLOWNEDBUFFER803F5428_H

class GtlOwnedBuffer803F5428 {
public:
    void* data; // 0x0
    char pad_004[0x4];
    int count; // 0x8
};

#define OFF(f, o) typedef char GtlOwnedBuffer803F5428_off_##f[((int)&((GtlOwnedBuffer803F5428*)0)->f == (o)) ? 1 : -1]
OFF(data, 0x0);
OFF(count, 0x8);
#undef OFF

#define AsGtlOwnedBuffer803F5428(p) ((GtlOwnedBuffer803F5428*)(p))

#endif
