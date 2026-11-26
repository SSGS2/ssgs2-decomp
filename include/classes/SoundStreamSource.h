// gtl::CSoundStreamSource: the source of a streamed sound. It holds the metadata copied from the
// sound definition, the ADPCM parameter block, and the owned stream callback. The nested
// StrmCallback is the callback object; StrmDataLoadTask is the queued DVD read request.
#ifndef CLASSES_SOUNDSTREAMSOURCE_H
#define CLASSES_SOUNDSTREAMSOURCE_H

// Object owned through the callback pointer; its class has no header yet.
struct StreamResource {
    virtual ~StreamResource();
};

// Completion receiver of a stream load task; its class has no header yet.
struct StreamLoadReceiver {
    virtual void done(int result);
};

// Borrowed output of GetStreamInfo; defined by the player source.
struct StreamInfoOut;

// Metadata for a new source. Metadata is copied as-is; pointer slots are shallow copies.
struct SoundStreamSourceInit {
    void* name;
    void* sourceData;
    void* sampleRate;
    unsigned short format;
    unsigned short channelCount;
    void* sampleCount;
    void* streamData;
};

namespace gtl {

class CSoundStreamSource {
public:
    // Callback object owned by a source. Bit 31 of flags means data is owned; otherwise the name is inline.
    class StrmCallback {
    public:
        int vfunc_1();
        int QueueLoadTask(void* requestData, void* byteCount, void* fileOffset, void* channelCount,
                          void* channelBlockBytes, void* headerBytes, unsigned char hasChannelHeaders,
                          void* completionReceiver, void* ownerId);
        void CancelOwnerTasks(void* ownerKey);

        void* vtbl;         // 0x00
        unsigned int flags; // 0x04
        unsigned int size;  // 0x08
        char* data;         // 0x0C
    };

    // Queued DVD read of a stream block, with the channel layout of the destination.
    class StrmDataLoadTask {
    public:
        void* Destroy(int deleteFlag);
        void Execute();
        void Cancel();

        void* vtbl;                   // 0x00
        void* queueNext;              // 0x04
        void* queuePrevious;          // 0x08
        void* ownerId;                // 0x0C
        char* buffer;                 // 0x10
        int size;                     // 0x14
        int offset;                   // 0x18
        int readPriority;             // 0x1C
        void* dvdEntryNumber;         // 0x20
        StreamLoadReceiver* callback; // 0x24
        int channels;                 // 0x28
        int blockSize;                // 0x2C
        int headerSize;               // 0x30
        unsigned char hasHeader;      // 0x34
    };

    CSoundStreamSource* Init(const SoundStreamSourceInit* init, const unsigned int* config);
    CSoundStreamSource* Destroy(int deleteFlag);
    int GetStreamCallback(void** sourceOutput);
    int BuildPlaybackHeader(void* stream) const;
    int GetStreamInfo(StreamInfoOut* out);

    unsigned int enabled;                      // 0x00
    void* vtbl;                                // 0x04
    void* name;                                // 0x08
    void* sourceData;                          // 0x0C
    void* sampleRate;                          // 0x10
    unsigned short format;                     // 0x14
    unsigned short channelCount;               // 0x16
    void* sampleCount;                         // 0x18
    void* streamData;                          // 0x1C
    unsigned int config[7];                    // 0x20, config[0] is the NUL-terminated stream name
    unsigned short adpcmChannelParameters[48]; // 0x3C
    StreamResource* callback;                  // 0x9C
};

} // namespace gtl

#define CSOUNDSTREAMSOURCE_OFF(f, o) \
    typedef char CSoundStreamSource_off_##f[((int)&((gtl::CSoundStreamSource*)0)->f == (o)) ? 1 : -1]
CSOUNDSTREAMSOURCE_OFF(enabled, 0x00);
CSOUNDSTREAMSOURCE_OFF(vtbl, 0x04);
CSOUNDSTREAMSOURCE_OFF(sampleRate, 0x10);
CSOUNDSTREAMSOURCE_OFF(config, 0x20);
CSOUNDSTREAMSOURCE_OFF(adpcmChannelParameters, 0x3C);
CSOUNDSTREAMSOURCE_OFF(callback, 0x9C);
#undef CSOUNDSTREAMSOURCE_OFF

#endif
