// GtlPagedQueue803F: layout view owned by gtl_sampling_filter_queues.cpp.
// Evidence: GtlPagedQueue803F_GetBackSpareCount: remaining slots = ((pageCapacity-1)<<7) - firstIndex - size. No virtual declarations or runtime objects.
#ifndef CLASSES_GTLPAGEDQUEUE803F_H
#define CLASSES_GTLPAGEDQUEUE803F_H

class GtlPagedQueue803F {
public:
    unsigned int firstIndex; // 0x0; GtlPagedQueue803F_GetBackSpareCount: remaining slots = ((pageCapacity-1)<<7) - firstIndex - size
    unsigned int size; // 0x4; GtlPagedQueue803F_GetBackSpareCount: remaining slots = ((pageCapacity-1)<<7) - firstIndex - size
    char pad_8[0x8];
    unsigned int pageCapacity; // 0x10; GtlPagedQueue803F_GetBackSpareCount: remaining slots = ((pageCapacity-1)<<7) - firstIndex - size
};

#define OFF(f, o) typedef char GtlPagedQueue803F_off_##f[((int)&((GtlPagedQueue803F*)0)->f == (o)) ? 1 : -1]
OFF(firstIndex, 0x0);
OFF(size, 0x4);
OFF(pageCapacity, 0x10);
#undef OFF

#endif
