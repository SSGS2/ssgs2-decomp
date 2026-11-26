// Media pipeline manager: owns the five filter stages, the DVD loader, the sample pool and the output samples.
// Methods are in gtl_media_pipeline_filters.cpp and gtl_media_pipeline_filter_manager.cpp.
// No virtual functions are declared here; the vtable comes from the original data.
#ifndef CLASSES_GTLMEDIAPIPELINE_H
#define CLASSES_GTLMEDIAPIPELINE_H

namespace gtl {

// Two numeric words (sample size, sample count) given to the constructor. They are values, not pointers.
struct CFilterManagerConfig {
    unsigned int sampleSize;
    unsigned int sampleCount;
};

class CFilterManager {
public:
    // Setup and teardown.
    // Initialize pipeline resources and the configured sample size and count.
    void Init(const CFilterManagerConfig* config);
    // Release both reference-counted output samples and the pending file name; free storage when flag > 0.
    void* Destroy(int flag);
    // Unregister the pipeline, then invoke its deleting destructor.
    void UnregisterAndDelete();
    // Release pipeline-owned samples, pool, format, DVD loader, and output references.
    void ReleaseResources();
    // Release the show counter's reference and clear the pipeline's pointer.
    void ReleaseShowCounter();

    // File and pipeline control.
    // Copy and open the DVD path, allocate output samples, then report loading state.
    int OpenFile(const char* path, int flag);
    // Close the DVD file, release pipeline resources, and report state 0.
    int CloseFile();
    // Validate the filter type and optionally destroy the previous filter in that stage.
    int SetFilter(int index, void* filter, int replace);
    // Transition filters to state 3 and notify the show counter before reporting the state.
    int StartRunning();
    // Transition to state 4, recycle samples, and reset the show counter.
    int ResetPipeline();
    // Transition filters to state 5 and notify the show counter before reporting the state.
    int Suspend();
    // Advance loading and each filter stage, transfer referenced samples, and stop on a stage error.
    int Update();
    // Poll DVD loading, configure the filters once data arrives, and finish a pending start.
    int UpdateLoading();
    // Issue a supported transition command to all five filters in order.
    int TransitionFilters(int requestedState);
    // Update the split filter and forward output ports 0/1 to primary/secondary sinks.
    int ForwardSplitOutputs(void* splitFilter, void* primarySink, void* secondarySink);
    // Store the pipeline state and report success.
    int SetState(int state);
    // Copy the current pipeline state to the caller.
    int GetState(int* outState);

    // Output and format access.
    // Copy the active media format only while the pipeline has a usable format.
    int CopyFormat(void* output);
    // Return an added primary-sample reference only while the primary filter has usable output.
    int GetPrimaryOutput(void* output);
    // Return an added secondary-sample reference only while the secondary filter is output-ready.
    int GetSecondaryOutput(void* output);
    // Replace the primary sample buffer, freeing old data only when internally owned.
    void SetOutputBuffer(int buffer);

    // Timing and restart control.
    // Store the pipeline playback value.
    int SetShowCounterParameter(float value);
    // Copy the pipeline timing-control scalar to caller storage.
    int GetShowCounterParameter(float* outTimingValue);
    // Store the pipeline control word without interpreting caller policy.
    int SetSourceRestartControl(int controlValue);
    // Return the pipeline source-restart control word through caller storage.
    int GetSourceRestartControl(int* outControlValue);

    // Virtual slots of the original class with no recovered purpose yet.
    // Return whether the supplied status is nonzero.
    int vfunc_12(int value);
    // Default successful pipeline hook.
    int vfunc_13();
    // Default successful pipeline hook.
    int vfunc_14();
    // Default successful pipeline hook.
    int vfunc_15();
    // Test for source completion while the transform waits for more input.
    int vfunc_28();

    void* vtbl;                 // 0x0
    void* format;               // 0x4
    char inputFile[0x34];       // 0x8, DVD file handle
    void* sourceByteLengthWord; // 0x3C
    char pad_040[0x4];
    void* output0;              // 0x44
    void* output1;              // 0x48
    void* showCounter;          // 0x4C
    void* stage0;               // 0x50
    void* stage1;               // 0x54
    void* stage2;               // 0x58
    void* stage3;               // 0x5C
    void* stage4;               // 0x60
    int state;                  // 0x64
    float showCounterParameter; // 0x68
    int sourceRestartControl;   // 0x6C
    unsigned int sampleSize;    // 0x70
    unsigned int sampleCount;   // 0x74
    void* samplePool;           // 0x78
    void* poolSample;           // 0x7C
    void* loader;               // 0x80
    char* pendingName;          // 0x84
    unsigned char startPending; // 0x88
    char pad_089[0x3];
    int outputBuffer; // 0x8C
};

} // namespace gtl

#define GTL_CFILTERMANAGER_OFF(f, o) \
    typedef char gtl_CFilterManager_off_##f[((int)&((gtl::CFilterManager*)0)->f == (o)) ? 1 : -1]
GTL_CFILTERMANAGER_OFF(vtbl, 0x0);
GTL_CFILTERMANAGER_OFF(format, 0x4);
GTL_CFILTERMANAGER_OFF(inputFile, 0x8);
GTL_CFILTERMANAGER_OFF(sourceByteLengthWord, 0x3C);
GTL_CFILTERMANAGER_OFF(output0, 0x44);
GTL_CFILTERMANAGER_OFF(output1, 0x48);
GTL_CFILTERMANAGER_OFF(showCounter, 0x4C);
GTL_CFILTERMANAGER_OFF(stage0, 0x50);
GTL_CFILTERMANAGER_OFF(stage1, 0x54);
GTL_CFILTERMANAGER_OFF(stage2, 0x58);
GTL_CFILTERMANAGER_OFF(stage3, 0x5C);
GTL_CFILTERMANAGER_OFF(stage4, 0x60);
GTL_CFILTERMANAGER_OFF(state, 0x64);
GTL_CFILTERMANAGER_OFF(showCounterParameter, 0x68);
GTL_CFILTERMANAGER_OFF(sourceRestartControl, 0x6C);
GTL_CFILTERMANAGER_OFF(sampleSize, 0x70);
GTL_CFILTERMANAGER_OFF(sampleCount, 0x74);
GTL_CFILTERMANAGER_OFF(samplePool, 0x78);
GTL_CFILTERMANAGER_OFF(poolSample, 0x7C);
GTL_CFILTERMANAGER_OFF(loader, 0x80);
GTL_CFILTERMANAGER_OFF(pendingName, 0x84);
GTL_CFILTERMANAGER_OFF(startPending, 0x88);
GTL_CFILTERMANAGER_OFF(outputBuffer, 0x8C);
#undef GTL_CFILTERMANAGER_OFF

#endif
