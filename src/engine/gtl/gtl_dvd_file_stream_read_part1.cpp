// Sound stream reads from DVD: opening the stream for a load task, the runtime type checks
// that find a DVD file stream, and the read path used by gtl::CSoundStreamSource's data
// load task.

#pragma gcc_extensions on
// Preserve the compiler's non-returning exception helper declaration.
#define __throw(a, b, c) __throw(a, b, c) __attribute__((noreturn))
#include "sdk_protos.h"
#undef __throw
#include <revolution/OS/OSCache.h>
#include "classes/GtlDvdFileStream.h"
#include "classes/SoundStreamSource.h"

enum StreamReadConstants {
    kReadCancelled = -3,
    kSeekFromStart = 0,
    kCacheLineBytes = 32,
    kMaxStreamChannels = 4,
    kChannelHeaderBytes = 8
};
#pragma use_lmw_stmw on
#pragma deadcode on

struct TypeInfo {
    TypeInfo* parent;
};
struct Stream {
    virtual TypeInfo* type();
    virtual void v0c();
    virtual void close();
    virtual int read(void*, int);
    virtual void v18();
    virtual void v1c();
    virtual void v20();
    virtual void v24();
    virtual int check28();
    virtual int check2c();
    virtual void v30();
    virtual void v34();
    virtual void v38();
    virtual void v3c();
    virtual void v40();
    virtual void seek(int, int);
    virtual void v48();
    virtual void v4c();
    virtual int check50();
    char pad[0x64];
    int priority; // Same offset as DvdFileStream::priority.
};
// The load task being executed; its fields are declared in classes/SoundStreamSource.h.
typedef gtl::CSoundStreamSource::StrmDataLoadTask Task;
typedef StreamLoadReceiver Callback;
extern "C" {
extern Stream s_audioDvdFileStreamStorage;
extern Stream* g_activeAudioStream;
extern TypeInfo s_dvdFileStreamTypeInfo;
void* GtlStreamTaskManager_GetInstance();
void AudioWorkQueue_CancelOwnerTasks(void*, void*);
void* gtl_StreamLoadTask_GetChannelBlockData(Callback*, int);
void StreamLoadTask_SetChannelStates(Callback*, int, unsigned short*);
void* memcpy(void*, const void*, unsigned long);
}

// Construct the shared DVD stream and reject failed virtual readiness/capability checks.
// The meanings of check50/check2c/check28 are not individually established here.
static inline int openStream(Task* task) {
    Stream* stream = &s_audioDvdFileStreamStorage;
    if (stream) {
        try {
            ((gtl::nw4r::ut::DvdFileStream*)stream)->Init(task->dvdEntryNumber);
        } catch (...) {
            __throw(0, 0, 0);
        }
    }
    g_activeAudioStream = stream;
    if (!stream)
        return 0;
    if (!stream->check50() || !g_activeAudioStream->check2c() || !g_activeAudioStream->check28()) {
        g_activeAudioStream->close();
        g_activeAudioStream = 0;
        return 0;
    }
    return 1;
}
// Walk the runtime type chain to support derived DVD stream types.
static inline int hasType(TypeInfo* type, TypeInfo* wanted) {
    while (type) {
        if (type == wanted)
            return 1;
        type = type->parent;
    }
    return 0;
}
// Return the stream only when its runtime type derives from DvdFileStream.
static inline Stream* castStream(Stream* stream) {
    TypeInfo* wanted = &s_dvdFileStreamTypeInfo;
    if (hasType(stream->type(), wanted))
        return stream;
    return 0;
}
// Seek to an absolute byte offset and read size bytes; return the stream's raw result.
// Only a DVD-derived stream receives readPriority; the shared stream must be serialized.
static inline int readStream(void* buffer, int size, int offset, Task* task) {
    if (!g_activeAudioStream)
        return 0;
    Stream* stream = castStream(g_activeAudioStream);
    if (stream)
        ((gtl::nw4r::ut::DvdFileStream*)stream)->priority = task->readPriority;
    g_activeAudioStream->seek(offset, kSeekFromStart);
    return g_activeAudioStream->read(buffer, size);
}
// Read a block containing consecutive, cache-aligned channel payloads and notify completion.
// Caller supplies cache-safe staging storage covering headerSize plus all rounded
// payloads, and writable callback destinations of at least the rounded block size.
// done(0) means open/read failure; done(1) follows any nonnegative read (even a short
// read). Cancellation (-3) skips notification. The callback may be null.
// hasHeader requires a positive headerSize with an 8-byte record per channel:
// otherwise the state array passed on completion contains uninitialized entries.
void gtl::CSoundStreamSource::StrmDataLoadTask::Execute() {
    // Phase 1: invalidate DVD staging memory, then open/read/close the shared stream.
    DCInvalidateRange(buffer, size);
    if (!openStream(this)) {
        if (callback)
            callback->done(0);
        return;
    }
    int result = readStream(buffer, size, offset, this);
    g_activeAudioStream->close();
    g_activeAudioStream = 0;
    // Phase 2: cancellation stops quietly; other read errors cancel this owner's queue.
    if (result == kReadCancelled)
        return;
    if (result < 0) {
        AudioWorkQueue_CancelOwnerTasks(GtlStreamTaskManager_GetInstance(), ownerId);
        if (callback)
            callback->done(0);
        return;
    }
    // Phase 3: copy each channel's rounded payload and collect its leading state word.
    unsigned short headers[kMaxStreamChannels];
    int channel = 0;
    int alignedBlockBytes;
    int headerOffset = 0;
    unsigned short* header = headers;
    for (; channel < channels; headerOffset += kChannelHeaderBytes, ++header, ++channel) {
        alignedBlockBytes = (blockSize + kCacheLineBytes - 1) & ~(kCacheLineBytes - 1);
        int payloadStart = headerSize;
        int channelOffset = payloadStart + alignedBlockBytes * channel;
        if (hasHeader && payloadStart > 0)
            // The stream block header is big-endian on disc (state halfword).
            *header = *(unsigned short*)(buffer + headerOffset);
        if (callback) {
            char* source = buffer + channelOffset;
            void* dest = gtl_StreamLoadTask_GetChannelBlockData(callback, channel);
            if (source && dest) {
                memcpy(dest, source, alignedBlockBytes);
                DCFlushRange(dest, alignedBlockBytes);
            }
        }
    }
    // Phase 4: publish channel states before reporting successful completion.
    if (callback) {
        if (hasHeader)
            StreamLoadTask_SetChannelStates(callback, channels, headers);
        callback->done(1);
    }
}
