// GtlSamplePool: layout view; methods in gtl_media_sample_pool.cpp (SamplePool_Ctor, SamplePool_Dtor).
// Inferred descriptive type name; no vtable, RTTI, constructors or base classes emitted.
#ifndef CLASSES_GTLSAMPLEPOOL_H
#define CLASSES_GTLSAMPLEPOOL_H

#include "classes/GtlSampleDeque.h"

class GtlSamplePool {
public:
    char* storage;            // 0x0
    unsigned int sampleSize;  // 0x4
    unsigned int sampleCount; // 0x8
    GtlSampleDeque available; // 0xC
    GtlSampleDeque queued;    // 0x24
    GtlSampleDeque returned;  // 0x3C
};

#define OFF(f, o) typedef char GtlSamplePool_off_##f[((int)&((GtlSamplePool*)0)->f == (o)) ? 1 : -1]
OFF(storage, 0x0);
OFF(sampleSize, 0x4);
OFF(sampleCount, 0x8);
OFF(available, 0xC);
OFF(queued, 0x24);
OFF(returned, 0x3C);
#undef OFF

#endif
