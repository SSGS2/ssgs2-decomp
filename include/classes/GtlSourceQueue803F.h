// GtlSourceQueue803F: layout view owned by gtl_sampling_filter_queues.cpp.
// Evidence: gtl_CSamplingFilter_ResetAndStart; gtl_CSamplingFilter_AcquireSharedRef; gtl_CSamplingFilter_SetSourceFromSample; gtl_CSamplingFilter_CollectAndCheckInput. No virtual declarations or runtime objects.
#ifndef CLASSES_GTLSOURCEQUEUE803F_H
#define CLASSES_GTLSOURCEQUEUE803F_H

class GtlSourceQueue803F {
public:
    char pad_0[0x4];
    char* vtable; // 0x4; gtl_CSamplingFilter_ResetAndStart
    char pad_8[0x8];
    unsigned int* sharedCounter; // 0x10; gtl_CSamplingFilter_ResetAndStart
    void* source; // 0x14; gtl_CSamplingFilter_ResetAndStart
    char pad_18[0x4];
    unsigned int firstIndex; // 0x1C; gtl_CSamplingFilter_ResetAndStart
    unsigned int size; // 0x20; gtl_CSamplingFilter_ResetAndStart
    unsigned int pagesAddress; // 0x24; gtl_CSamplingFilter_ResetAndStart
    unsigned int firstPage; // 0x28; gtl_CSamplingFilter_ResetAndStart
    unsigned int pageCapacity; // 0x2C; gtl_CSamplingFilter_ResetAndStart
    unsigned int pageCount; // 0x30; gtl_CSamplingFilter_ResetAndStart
};

#define OFF(f, o) typedef char GtlSourceQueue803F_off_##f[((int)&((GtlSourceQueue803F*)0)->f == (o)) ? 1 : -1]
OFF(vtable, 0x4);
OFF(sharedCounter, 0x10);
OFF(source, 0x14);
OFF(firstIndex, 0x1C);
OFF(size, 0x20);
OFF(pagesAddress, 0x24);
OFF(firstPage, 0x28);
OFF(pageCapacity, 0x2C);
OFF(pageCount, 0x30);
#undef OFF

#endif
