// gtl::CSoundBufferSource: a sound sample source. It copies the sound's metadata at creation and holds a
// reference on the shared sample block. The playback binding reads some of the same words under other
// names; those are the anonymous unions below.
#ifndef CLASSES_GTL_CSOUNDBUFFERSOURCE_H
#define CLASSES_GTL_CSOUNDBUFFERSOURCE_H

// Sound metadata copied at creation.
struct SoundCopyData {
    void* name;
    void* format;
    unsigned int sampleRate;
    unsigned short sampleFormat;
    unsigned short channelCount;
    unsigned int sampleCount;
    void* loopData;
};

// Shared sample block: the reference-counted owner, the sample data and its byte count.
struct SoundRefData {
    void* sampleOwner;
    void* sampleData;
    unsigned int sampleBytes;
};

// Sample metadata reported by GetBufferInfo.
struct SoundBufferInfo {
    unsigned int name;
    unsigned int format;
    unsigned int sampleRate;
    unsigned short sampleFormat;
    unsigned short channelCount;
    unsigned int sampleCount;
    unsigned int loopData;
    unsigned int sampleOwner;
};

namespace gtl {

class CSoundBufferSource {
public:
    // Copy the metadata and take a reference on the shared sample block; returns this.
    CSoundBufferSource* Init(const SoundCopyData* from, const SoundRefData* shared);
    // Release the sample block; frees this when flags is positive. Returns this.
    void* Destroy(int flags);

    // Fill the playback header of soundBuffer from this source and flush its channel data.
    static int BindToPlaybackBuffer(CSoundBufferSource* __restrict self, void* soundBuffer);
    // Copy the metadata to out. Returns 0 for a null out.
    int GetBufferInfo(SoundBufferInfo* out) const;

    int referenceCount; // 0x0
    void* vtable;       // 0x4
    union {
        void* name;             // 0x8
        unsigned int byteTotal; // 0x8: playback binding reads the source byte count here
    };
    void* format; // 0xC
    union {
        unsigned int sampleRate;     // 0x10
        unsigned int playbackFormat; // 0x10: playback binding stores the format from here
    };
    unsigned short sampleFormat; // 0x14
    unsigned short channelCount; // 0x16
    union {
        unsigned int sampleCount;  // 0x18
        unsigned int playbackRate; // 0x18: playback binding copies the rate from here
    };
    union {
        void* loopData;              // 0x1C
        unsigned int playbackLength; // 0x1C: playback binding copies the length from here
    };
    union {
        void* sampleOwner; // 0x20: reference-counted owner of the samples
        void* dataBlock;   // 0x20: playback binding reads its base pointer from here
    };
    void* sampleData;         // 0x24: first entry of the offset table
    unsigned int sampleBytes; // 0x28
};

#define GTL_CSOUNDBUFFERSOURCE_OFF(f, o) \
    typedef char gtl_CSoundBufferSource_off_##f[((int)&((CSoundBufferSource*)0)->f == (o)) ? 1 : -1]
GTL_CSOUNDBUFFERSOURCE_OFF(vtable, 0x4);
GTL_CSOUNDBUFFERSOURCE_OFF(sampleOwner, 0x20);
GTL_CSOUNDBUFFERSOURCE_OFF(sampleData, 0x24);
GTL_CSOUNDBUFFERSOURCE_OFF(sampleBytes, 0x28);
#undef GTL_CSOUNDBUFFERSOURCE_OFF

} // namespace gtl

#endif // CLASSES_GTL_CSOUNDBUFFERSOURCE_H
