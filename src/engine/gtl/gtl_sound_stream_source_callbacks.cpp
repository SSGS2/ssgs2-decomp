// Callbacks for streamed sounds (gtl::CSoundStreamSource): queuing a DVD load task for a stream block,
// cancelling the active stream from a file load task, and the constants these callbacks use.
//
// The source's own functions are in gtl_sound_stream_source_part1.cpp through part4.cpp.

#include <decomp/static_init.h>
#include "classes/SoundStreamSource.h"
#include "classes/gtl_FileLoadTask.h"

extern "C" {

extern float kFloat_6_2831855;
float s_audioStreamCallbackFullTurnAngle;
void* g_activeAudioStream;
extern void* g_streamDvdPathPrefix;

void __dl__FPv(void*);

// Optionally deallocate this trivial object; the returned address may already be freed.
void* dtor_80403C60(void* this_, int deleteFlag) {
    if (this_ != 0 && deleteFlag > 0) {
        __dl__FPv(this_);
    }
    return this_;
}

// The active file stream, viewed through its vtable. Slot indices include the two ABI header words.
struct VTable {
    void* fn[22];
};
struct FileTask {
    VTable* vtable;
};
typedef int (*StreamCapabilityFunction)(FileTask*);
typedef void (*StreamAsyncCancelFunction)(FileTask*, int, int);
typedef void (*StreamCancelFunction)(FileTask*);

enum StreamCancelSlot {
    kCanCancelStream = 21,
    kCanCancelStreamAsync = 10,
    kCancelStreamAsync = 19,
    kCancelStream = 18
};

} // extern "C"

// Cancel the borrowed active stream when supported, choosing async cancellation if available.
// No stream is freed or cleared here; both cancellation return values are ignored.
void gtl::FileLoadTask::CancelActiveStream() {
    if (g_activeAudioStream == 0)
        return;
    if (((StreamCapabilityFunction)((FileTask*)g_activeAudioStream)->vtable->fn[kCanCancelStream])(
            (FileTask*)g_activeAudioStream) == 0)
        return;
    if (((StreamCapabilityFunction)((FileTask*)g_activeAudioStream)->vtable->fn[kCanCancelStreamAsync])(
            (FileTask*)g_activeAudioStream) != 0) {
        ((StreamAsyncCancelFunction)((FileTask*)g_activeAudioStream)->vtable->fn[kCancelStreamAsync])(
            (FileTask*)g_activeAudioStream, 0, 0);
    } else {
        ((StreamCancelFunction)((FileTask*)g_activeAudioStream)->vtable->fn[kCancelStream])(
            (FileTask*)g_activeAudioStream);
    }
}

#include "sdk_protos.h"
extern void* GtlStreamTaskManager_GetInstance();
extern void* SoundStreamTaskQueue_AllocateTaskBlocking(void*);
extern void* DVDConvertPathToEntrynum(const char*);
extern void SoundStreamTaskQueue_Enqueue(void*, void*, int);
extern void* AudioMessageWorker_GetInstance();
extern void SoundStream_PostMessageOne(void*);

enum StreamLoadResult {
    kStreamAllocationUnavailable = 1,
    kStreamRequestQueued = 3
};
enum {
    kStreamTaskQueuePriority = 2
};
extern void* __vt__Q23gtl12FileLoadTask[];
extern void* __vt__Q33gtl18CSoundStreamSource16StrmDataLoadTask[];

// Populate a pooled DVD stream request, enqueue it, and wake the audio message worker.
// Allocation can sleep. Return 1 if no storage is returned, 3 after enqueueing.
// Ownership of the pooled task passes to the queue; buffer/receiver remain borrowed
// through completion. The worker supports at most four channels and rounds each channel
// block to 32 bytes; callers must size the staging buffer and header data for that layout.
int gtl::CSoundStreamSource::StrmCallback::QueueLoadTask(void* requestData, void* byteCount, void* fileOffset,
                                                         void* channelCount, void* channelBlockBytes, void* headerBytes,
                                                         unsigned char hasChannelHeaders, void* completionReceiver,
                                                         void* ownerId) {
    // Phase 1: acquire raw task storage and select heap or inline path text.
    gtl::CSoundStreamSource::StrmDataLoadTask* stream =
        (gtl::CSoundStreamSource::StrmDataLoadTask*)SoundStreamTaskQueue_AllocateTaskBlocking(
            GtlStreamTaskManager_GetInstance());
    if (stream == 0)
        return kStreamAllocationUnavailable;
    const char* path;
    if ((int)flags < 0)
        path = data;
    else
        path = (const char*)&flags + 1;
    // Phase 2: initialize queue/base-task fields before installing the derived vtable.
    stream->queueNext = 0;
    stream->queuePrevious = 0;
    stream->ownerId = ownerId;
    stream->vtbl = __vt__Q23gtl12FileLoadTask;
    stream->buffer = (char*)requestData;
    stream->size = (int)byteCount;
    stream->offset = (int)fileOffset;
    stream->readPriority = 0;
    stream->dvdEntryNumber = (void*)-1;
    // Preserve the containment test: it advances the START by prefix length even
    // if the matching substring occurs later in the path; it is not a starts-with test.
    if (strstr(path, (const char*)g_streamDvdPathPrefix) != 0)
        path += strlen((const char*)g_streamDvdPathPrefix);
    stream->dvdEntryNumber = DVDConvertPathToEntrynum(path);
    stream->vtbl = __vt__Q33gtl18CSoundStreamSource16StrmDataLoadTask;
    stream->callback = (StreamLoadReceiver*)completionReceiver;
    stream->channels = (int)channelCount;
    stream->blockSize = (int)channelBlockBytes;
    stream->headerSize = (int)headerBytes;
    stream->hasHeader = (unsigned char)hasChannelHeaders;
    // Phase 3: publish the fully initialized task, then wake its worker.
    SoundStreamTaskQueue_Enqueue(GtlStreamTaskManager_GetInstance(), stream, kStreamTaskQueuePriority);
    SoundStream_PostMessageOne(AudioMessageWorker_GetInstance());
    return kStreamRequestQueued;
}

extern "C" {

// Initialize this translation unit's full-turn angle constant.
void AudioStreamCallback_InitFullTurnConstant() {
    s_audioStreamCallbackFullTurnAngle = kFloat_6_2831855;
}

// Optionally deallocate this trivial object; positive deleteFlag invalidates the result.
void* dtor_80403C20(void* this_, int deleteFlag) {
    if (this_ != 0 && deleteFlag > 0) {
        __dl__FPv(this_);
    }
    return this_;
}

} // extern "C"

// Report this callback's constant capability result.
int gtl::CSoundStreamSource::StrmCallback::vfunc_1() {
    return 1;
}

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804626AC", _ctors_u80403C14_0, AudioStreamCallback_InitFullTurnConstant);
#pragma pop
