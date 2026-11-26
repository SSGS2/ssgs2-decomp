// GtlMediaFormat: layout view; methods in gtl_media_pipeline_filter_manager.cpp (gtl_MediaFormat_Equals, gtl_MediaPipeline_CreateSamplePoolAndDvdLoader).
// Inferred descriptive type name; no vtable, RTTI, constructors or base classes emitted.
#ifndef CLASSES_GTLMEDIAFORMAT_H
#define CLASSES_GTLMEDIAFORMAT_H

class GtlMediaFormat {
public:
    int unk_0;                        // 0x0
    unsigned int sourceByteLength;    // 0x4
    char name[0x80];                  // 0x8
    float showCounterFormatParameter; // 0x88
    unsigned int initialReadOffset;   // 0x8C
    unsigned int sampleBufferSize;    // 0x90
    char unk_94[0x4];
    unsigned short unk_98;                   // 0x98
    unsigned short unk_9A;                   // 0x9A
    unsigned int transformWorkspaceCount;    // 0x9C
    unsigned int splitPacketHeaderWordCount; // 0xA0
};

#define OFF(f, o) typedef char GtlMediaFormat_off_##f[((int)&((GtlMediaFormat*)0)->f == (o)) ? 1 : -1]
OFF(unk_0, 0x0);
OFF(sourceByteLength, 0x4);
OFF(name, 0x8);
OFF(showCounterFormatParameter, 0x88);
OFF(initialReadOffset, 0x8C);
OFF(sampleBufferSize, 0x90);
OFF(unk_98, 0x98);
OFF(unk_9A, 0x9A);
OFF(transformWorkspaceCount, 0x9C);
OFF(splitPacketHeaderWordCount, 0xA0);
#undef OFF

#endif
