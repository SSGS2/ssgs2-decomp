#include <decomp/static_init.h>
// Translation unit 0x80403C14-0x80403E80 (6 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

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

// Legacy FileTask spelling: this dispatch view actually describes the active
// file STREAM. It is unrelated to the pooled StreamTask layout below.
struct VTable { void* fn[22]; };
struct FileTask { VTable* vtable; };
typedef int (*StreamCapabilityFunction)(FileTask*);
typedef void (*StreamAsyncCancelFunction)(FileTask*, int, int);
typedef void (*StreamCancelFunction)(FileTask*);

// These are vtable word indices INCLUDING the two ABI header words.
// Verified against DvdFileStream's DVDCancel and DVDCancelAsync implementations.
enum StreamCancelSlot {
    kCanCancelStream = 21, kCanCancelStreamAsync = 10,
    kCancelStreamAsync = 19, kCancelStream = 18
};

// Cancel the borrowed active stream when supported, choosing async cancellation if available.
// No stream is freed or cleared here; both cancellation return values are ignored.
void gtl_FileLoadTask_CancelActiveStream() {
	if (g_activeAudioStream == 0) return;
	if (((StreamCapabilityFunction)((FileTask*)g_activeAudioStream)->vtable->fn[kCanCancelStream])
		((FileTask*)g_activeAudioStream) == 0) return;
	if (((StreamCapabilityFunction)((FileTask*)g_activeAudioStream)->vtable->fn[kCanCancelStreamAsync])
		((FileTask*)g_activeAudioStream) != 0) {
		((StreamAsyncCancelFunction)((FileTask*)g_activeAudioStream)->vtable->fn[kCancelStreamAsync])
			((FileTask*)g_activeAudioStream, 0, 0);
	} else {
		((StreamCancelFunction)((FileTask*)g_activeAudioStream)->vtable->fn[kCancelStream])
			((FileTask*)g_activeAudioStream);
	}
}

#include "sdk_protos.h"
extern void* GtlStreamTaskManager_GetInstance();
extern void* SoundStreamTaskQueue_AllocateTaskBlocking(void*);
extern void* DVDConvertPathToEntrynum(const char*);
extern void SoundStreamTaskQueue_Enqueue(void*, void*, int);
extern void* AudioMessageWorker_GetInstance();
extern void SoundStream_PostMessageOne(void*);

struct Callback { unsigned char pad4[4]; unsigned int flags; unsigned char padC[4]; const char* path; };
// One pooled request: queue links, base file-load arguments, then stream completion data.
// Member names are provisional. The consumer in
// gtl_dvd_file_stream_read_part1.cpp establishes the following roles; pointer-shaped count words are ABI storage.
struct StreamTask {
    void* vtable;
    void* queueNext;
    void* queuePrevious;
    void* ownerId;   // +0x0C: owner ID for cancellation.
    void* requestData;        // +0x10: borrowed destination for the DVD read.
    void* readByteCount;      // +0x14: read byte count, NOT an offset.
    void* fileByteOffset;      // +0x18: file byte offset, NOT a length.
    void* readPriority;        // +0x1C: DVD read priority, initialized to zero.
    void* dvdEntry;           // +0x20: DVD entry number.
    void* completionReceiver;       // +0x24: borrowed completion/channel-buffer receiver.
    void* channelCount; // +0x28: channel count, NOT a callback.
    void* channelBlockBytes;   // +0x2C: bytes per channel block before 32-byte alignment.
    void* headerBytes;        // +0x30: header byte count preceding channel data.
    unsigned char hasChannelHeaders; // +0x34: whether channel headers are present.
};
typedef char StreamTask_flagOffset[((int)&((StreamTask*)0)->hasChannelHeaders == 0x34) ? 1 : -1];
enum StreamLoadResult { kStreamAllocationUnavailable = 1, kStreamRequestQueued = 3 };
enum { kStreamTaskQueuePriority = 2 };
extern void* __vt__Q23gtl12FileLoadTask[];
extern void* __vt__Q33gtl18CSoundStreamSource16StrmDataLoadTask[];

// Populate a pooled DVD stream request, enqueue it, and wake the audio message worker.
// Allocation can sleep. Return 1 if no storage is returned, 3 after enqueueing.
// Ownership of the pooled task passes to the queue; buffer/receiver remain borrowed
// through completion. Counts/offsets keep their original pointer-shaped ABI types.
// The worker supports at most four channels and rounds each channel block to 32 bytes;
// callers must size the staging buffer/header data for that layout (no checks here).
int CSoundStreamSource_StrmCallback_QueueLoadTask(Callback* self, void* requestData, void* byteCount, void* fileOffset, void* channelCount, void* channelBlockBytes, void* headerBytes, unsigned char hasChannelHeaders, void* completionReceiver, void* ownerId) {
	// Phase 1: acquire raw task storage and select heap or inline path text.
	StreamTask* stream = (StreamTask*)SoundStreamTaskQueue_AllocateTaskBlocking(GtlStreamTaskManager_GetInstance());
	if (stream == 0) return kStreamAllocationUnavailable;
	const char* path;
	if ((int)self->flags < 0) path = self->path;
	else path = (const char*)&self->flags + 1;
	// Phase 2: initialize queue/base-task fields before installing the derived vtable.
	stream->queueNext = 0;
	stream->queuePrevious = 0;
	stream->ownerId = ownerId;
	stream->vtable = __vt__Q23gtl12FileLoadTask;
	stream->requestData = requestData;
	stream->readByteCount = byteCount;
	stream->fileByteOffset = fileOffset;
	stream->readPriority = 0;
	stream->dvdEntry = (void*)-1;
	// Preserve the containment test: it advances the START by prefix length even
	// if the matching substring occurs later in the path; it is not a starts-with test.
	if (strstr(path, (const char*)g_streamDvdPathPrefix) != 0) path += strlen((const char*)g_streamDvdPathPrefix);
	stream->dvdEntry = DVDConvertPathToEntrynum(path);
	stream->vtable = __vt__Q33gtl18CSoundStreamSource16StrmDataLoadTask;
	stream->completionReceiver = completionReceiver;
	stream->channelCount = channelCount;
	stream->channelBlockBytes = channelBlockBytes;
	stream->headerBytes = headerBytes;
	stream->hasChannelHeaders = (unsigned char)hasChannelHeaders;
	// Phase 3: publish the fully initialized task, then wake its worker.
	SoundStreamTaskQueue_Enqueue(GtlStreamTaskManager_GetInstance(), stream, kStreamTaskQueuePriority);
	SoundStream_PostMessageOne(AudioMessageWorker_GetInstance());
	return kStreamRequestQueued;
}

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

// Report this callback's constant capability result; the wider interface meaning is unresolved.
int gtl_CSoundStreamSource_StrmCallback_vfunc_1() {
	return 1;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804626AC", _ctors_u80403C14_0, AudioStreamCallback_InitFullTurnConstant);
#pragma pop
