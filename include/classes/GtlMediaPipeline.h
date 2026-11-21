// GtlMediaPipeline: layout view; methods in gtl_media_pipeline_filter_manager.cpp (gtl_CFilterManager_Update, gtl_MediaPipeline_CreateSamplePoolAndDvdLoader, gtl_CFilterManager_UpdateLoading).
// Inferred descriptive type name; no vtable, RTTI, constructors or base classes emitted.
#ifndef CLASSES_GTLMEDIAPIPELINE_H
#define CLASSES_GTLMEDIAPIPELINE_H

class GtlMediaPipeline {
public:
    void* vtbl; // 0x0
    void* format; // 0x4
    char inputFile[0x34]; // 0x8
    void* sourceByteLengthWord; // 0x3C
    char pad_040[0x4];
    void* output0; // 0x44
    void* output1; // 0x48
    void* showCounter; // 0x4C
    void* stage0; // 0x50
    void* stage1; // 0x54
    void* stage2; // 0x58
    void* stage3; // 0x5C
    void* stage4; // 0x60
    int state; // 0x64
    float showCounterParameter; // 0x68
    int sourceRestartControl; // 0x6C
    unsigned int sampleSize; // 0x70
    unsigned int sampleCount; // 0x74
    void* samplePool; // 0x78
    void* poolSample; // 0x7C
    void* loader; // 0x80
    char* pendingName; // 0x84
    unsigned char startPending; // 0x88
    char pad_089[0x3];
    int outputBuffer; // 0x8C
};

#define OFF(f, o) typedef char GtlMediaPipeline_off_##f[((int)&((GtlMediaPipeline*)0)->f == (o)) ? 1 : -1]
OFF(vtbl, 0x0);
OFF(format, 0x4);
OFF(inputFile, 0x8);
OFF(sourceByteLengthWord, 0x3C);
OFF(output0, 0x44);
OFF(output1, 0x48);
OFF(showCounter, 0x4C);
OFF(stage0, 0x50);
OFF(stage1, 0x54);
OFF(stage2, 0x58);
OFF(stage3, 0x5C);
OFF(stage4, 0x60);
OFF(state, 0x64);
OFF(showCounterParameter, 0x68);
OFF(sourceRestartControl, 0x6C);
OFF(sampleSize, 0x70);
OFF(sampleCount, 0x74);
OFF(samplePool, 0x78);
OFF(poolSample, 0x7C);
OFF(loader, 0x80);
OFF(pendingName, 0x84);
OFF(startPending, 0x88);
OFF(outputBuffer, 0x8C);
#undef OFF

#endif
