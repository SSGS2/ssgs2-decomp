// Construction and destruction entry points of a streamed sound source (gtl::CSoundStreamSource).
// Init copies the source's metadata, and the destructor variants release its storage.
//
// The other source functions are in gtl_sound_stream_source_part2.cpp through part4.cpp.

#include "classes/SoundStreamSource.h"
#include "sdk_protos.h"

extern "C" {

void* gtl_Audio_Alloc(unsigned int);
void gtl_Audio_Free(void*);
void __dl__FPv(void*);
void gtl_String_ReserveCapacity(void*, unsigned int);
void gtl_String_ReplaceRange(void*, int, int, const char*, const char*, const char*, char*);

extern void* __vt__Q23gtl18CSoundStreamSource[6];
extern void* __vt__Q33gtl18CSoundStreamSource12StrmCallback[6];

enum {
    kStreamConfigWordCount = 7,
    kAdpcmParameterCopyPairs = 24
};

} // extern "C"

// Copy stream metadata and ADPCM parameters, then construct an owned callback name.
// init and config are borrowed; config is 7 words followed by 48 unsigned shorts.
// config[0] holds a valid NUL-terminated name, copied into the callback string.
// Returns this; callback allocation may yield null. String exceptions free the new
// callback allocation and propagate.
gtl::CSoundStreamSource* gtl::CSoundStreamSource::Init(const SoundStreamSourceInit* init, const unsigned int* config) {
    // Phase 1: initialize the source and copy the seven-word configuration prefix.
    enabled = 1;
    vtbl = &__vt__Q23gtl18CSoundStreamSource;
    callback = 0;
    name = init->name;
    sourceData = init->sourceData;
    sampleRate = init->sampleRate;
    format = init->format;
    channelCount = init->channelCount;
    sampleCount = init->sampleCount;
    streamData = init->streamData;
    for (int i = 0; i < kStreamConfigWordCount; ++i)
        this->config[i] = config[i];
    // Phase 2: copy 48 parameter halfwords, two per iteration.
    unsigned short* parameterDest = adpcmChannelParameters - 1;
    // Begin one halfword before each payload because the copy accesses [1].
    const unsigned short* parameterSource = (const unsigned short*)config + kStreamConfigWordCount * 2 - 1;
    for (int i = 0; i < kAdpcmParameterCopyPairs; ++i) {
        parameterDest[1] = parameterSource[1];
        ++parameterDest;
        ++parameterSource;
        parameterDest[1] = parameterSource[1];
        ++parameterDest;
        ++parameterSource;
    }

    // Phase 3: allocate the owned callback, initialize its empty string, and copy the name.
    gtl::CSoundStreamSource::StrmCallback* newCallback =
        (gtl::CSoundStreamSource::StrmCallback*)gtl_Audio_Alloc(sizeof(gtl::CSoundStreamSource::StrmCallback));
    if (newCallback) {
        newCallback->vtbl = &__vt__Q33gtl18CSoundStreamSource12StrmCallback;
        newCallback->flags = 0;
        newCallback->size = 0;
        newCallback->data = 0;
        const char* text = (const char*)this->config[0];
        size_t len = strlen(text);
        try {
            gtl_String_ReserveCapacity(&newCallback->flags, len);
            unsigned char allocatorToken;
            gtl_String_ReplaceRange(&newCallback->flags, 0, 0, text, text + len, (char*)&allocatorToken,
                                    (char*)((char*)&allocatorToken + 1));
        } catch (...) {
            gtl_Audio_Free(newCallback);
            throw;
        }
    }
    callback = (StreamResource*)newCallback;
    return this;
}

extern "C" {

// Allocation cleanup used when callback construction unwinds.
// flag is ignored; this entry always passes storage to the audio allocator.
void dtor_804043A8(void* self, int flag) {
    gtl_Audio_Free(self);
}

// Optionally free an audio-allocated object; a non-deleting call preserves storage.
// flag > 0 requests deallocation; return the original address, possibly freed.
void* dtor_80404368(void* self, int flag) {
    if (self != 0 && flag > 0) {
        gtl_Audio_Free(self);
    }
    return self;
}

// Optionally free an object allocated through the C++ heap.
// flag > 0 requests deallocation; return the original address, possibly freed.
void* dtor_804043AC(void* self, int flag) {
    if (self != 0 && flag > 0) {
        __dl__FPv(self);
    }
    return self;
}

} // extern "C"
