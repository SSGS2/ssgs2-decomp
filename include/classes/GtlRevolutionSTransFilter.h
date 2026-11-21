// gtl::CRevolutionSTransFilter: partial non-virtual layout from
// gtl_CRevolutionSTransFilter_AllocateBuffers in gtl_revolution_strans_filter_alloc.cpp.
#ifndef CLASSES_GTLREVOLUTIONSTRANSFILTER_H
#define CLASSES_GTLREVOLUTIONSTRANSFILTER_H

class GtlRevolutionSTransFilter {
public:
    char pad_000[0x1C];
    unsigned int segmentSize; // 0x803F9EF8-0x803F9EFC: allocation size is twice this value
    void* buffer;             // 0x803F9F18: result of gtl_Alloc
    void* firstSegment;       // 0x803F9F20: starts at buffer
    void* secondSegment;      // 0x803F9F30-0x803F9F34: buffer + segmentSize
};

#define GTLREVOLUTIONSTRANSFILTER_OFF(f, o) typedef char GtlRevolutionSTransFilter_off_##f[((int)&((GtlRevolutionSTransFilter*)0)->f == (o)) ? 1 : -1]
GTLREVOLUTIONSTRANSFILTER_OFF(segmentSize, 0x1C);
GTLREVOLUTIONSTRANSFILTER_OFF(buffer, 0x20);
GTLREVOLUTIONSTRANSFILTER_OFF(firstSegment, 0x24);
GTLREVOLUTIONSTRANSFILTER_OFF(secondSegment, 0x28);
#undef GTLREVOLUTIONSTRANSFILTER_OFF

#endif
