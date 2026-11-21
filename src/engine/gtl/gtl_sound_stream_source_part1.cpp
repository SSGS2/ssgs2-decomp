#include "classes/SoundStreamSource.h"
// Translation unit 0x80404214-0x804043EC (4 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".
#include "sdk_protos.h"

extern "C" {

void* gtl_Audio_Alloc(unsigned int);
void gtl_Audio_Free(void*);
void __dl__FPv(void*);
void gtl_String_ReserveCapacity(void*, unsigned int);
void gtl_String_ReplaceRange(void*, int, int, const char*, const char*, const char*, char*);

extern void* __vt__Q23gtl18CSoundStreamSource[6];
extern void* __vt__Q33gtl18CSoundStreamSource12StrmCallback[6];

enum { kStreamConfigWordCount = 7, kAdpcmParameterCopyPairs = 24 };

struct StreamCallback {
    void* vtable;
    unsigned int nameStorage[3]; // gtl string representation; initialized empty before assignment.
};

struct SoundStreamSourceInit {
    // Metadata is copied verbatim. Pointer-typed slots describe 32-bit storage;
    // they do not imply allocation, retention, or ownership of the pointed-to data.
    void* name;
    void* sourceData; // Copied to +0x0C; the owning format is unresolved.
    void* sampleRate;
    unsigned short format; // Copied to +0x14; encoding values are unresolved.
    unsigned short channelCount;
    void* sampleCount;
    void* streamData; // Copied to +0x1C; the owning format is unresolved.
};


// Copy stream metadata and ADPCM parameters, then construct an owned callback name.
// init/config are borrowed; config is 7 words followed by 48 unsigned shorts.
// config[0] holds a valid NUL-terminated name, copied into the callback string.
// Return self; callback allocation may yield null. String exceptions free the new
// callback allocation and propagate. Metadata pointers are shallow copies.
void* gtl_CSoundStreamSource_Ctor(SoundStreamSource* self, const SoundStreamSourceInit* init, const unsigned int* config) {
    // Phase 1: initialize the wrapper and copy the seven-word configuration prefix.
    self->enabled = 1;
    self->vtable = &__vt__Q23gtl18CSoundStreamSource;
    self->callback = 0;
    self->name = init->name;
    self->sourceData = init->sourceData;
    self->sampleRate = init->sampleRate;
    self->format = init->format;
    self->channelCount = init->channelCount;
    self->sampleCount = init->sampleCount;
    self->streamData = init->streamData;
    for (int i = 0; i < kStreamConfigWordCount; ++i) self->config[i] = config[i];
    // Phase 2: copy 48 parameter halfwords, two per iteration. These are not
    // necessarily 24 ADPCM coefficient pairs; the individual word roles are unresolved.
    unsigned short* parameterDest = self->adpcmChannelParameters - 1;
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
    void* callback = gtl_Audio_Alloc(sizeof(StreamCallback));
    if (callback) {
        ((StreamCallback*)callback)->vtable = &__vt__Q33gtl18CSoundStreamSource12StrmCallback;
        ((StreamCallback*)callback)->nameStorage[0] = 0;
        ((StreamCallback*)callback)->nameStorage[1] = 0;
        ((StreamCallback*)callback)->nameStorage[2] = 0;
        const char* name = (const char*)self->config[0];
        size_t len = strlen(name);
        try {
            gtl_String_ReserveCapacity(((StreamCallback*)callback)->nameStorage, len);
            unsigned char allocatorToken;
            gtl_String_ReplaceRange(((StreamCallback*)callback)->nameStorage, 0, 0, name, name + len, (char*)&allocatorToken, (char*)((char*)&allocatorToken + 1));
        } catch (...) {
            gtl_Audio_Free(callback);
            throw;
        }
    }
    self->callback = callback;
    return self;
}

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
