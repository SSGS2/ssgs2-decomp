// GtlSamplingFilter: layout view; methods in gtl_media_sample_and_sampling_filter.cpp (gtl_CSamplingFilter_Ctor, gtl_CSamplingFilter_Process, gtl_CSamplingFilter_dtor).
// No virtual methods, inheritance, or emitted runtime objects.
#ifndef CLASSES_GTLSAMPLINGFILTER_H
#define CLASSES_GTLSAMPLINGFILTER_H

class GtlSamplingFilter {
public:
    char pad_000[0x4];
    void* vtable; // 0x4
    char pad_008[0x8];
    int sample; // 0x10
    int inputSamplePool; // 0x14
    unsigned char started; // 0x18
    char pad_019[0x3];
    unsigned int queueStart; // 0x1C
    unsigned int queueSize; // 0x20
    unsigned int queueMap; // 0x24
    unsigned int queueMapStart; // 0x28
    unsigned int queueMapSize; // 0x2C
    unsigned int queueMapCapacity; // 0x30
};

#define OFF(f, o) typedef char GtlSamplingFilter_off_##f[((int)&((GtlSamplingFilter*)0)->f == (o)) ? 1 : -1]
OFF(vtable, 0x4);
OFF(sample, 0x10);
OFF(inputSamplePool, 0x14);
OFF(started, 0x18);
OFF(queueStart, 0x1C);
OFF(queueSize, 0x20);
OFF(queueMap, 0x24);
OFF(queueMapStart, 0x28);
OFF(queueMapSize, 0x2C);
OFF(queueMapCapacity, 0x30);
#undef OFF

#define AsGtlSamplingFilter(p) ((GtlSamplingFilter*)(p))

#endif
