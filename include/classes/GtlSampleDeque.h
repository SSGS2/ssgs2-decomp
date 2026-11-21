// GtlSampleDeque: layout view; methods in gtl_media_sample_pool.cpp (SampleDeque_EnsureBackBlocks, SampleDeque_dtor).
// Inferred descriptive type name; no vtable, RTTI, constructors or base classes emitted.
#ifndef CLASSES_GTLSAMPLEDEQUE_H
#define CLASSES_GTLSAMPLEDEQUE_H

class GtlSampleDeque {
public:
    unsigned int position; // 0x0
    unsigned int count; // 0x4
    unsigned int** rows; // 0x8
    unsigned int head; // 0xC
    unsigned int rowCount; // 0x10
    unsigned int capacity; // 0x14
};

#define OFF(f, o) typedef char GtlSampleDeque_off_##f[((int)&((GtlSampleDeque*)0)->f == (o)) ? 1 : -1]
OFF(position, 0x0);
OFF(count, 0x4);
OFF(rows, 0x8);
OFF(head, 0xC);
OFF(rowCount, 0x10);
OFF(capacity, 0x14);
#undef OFF

#endif
