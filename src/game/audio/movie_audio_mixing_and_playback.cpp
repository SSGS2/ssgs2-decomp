// Movie playback audio and video: the decoder's sample queues, mixing of the stereo PCM into the
// DMA buffers, stream volume fades, and drawing the decoded video frame with its GX setup.
// The registry tree cleanup (RotateRegistry) is also here.

#include "classes/GtlMediaSample.h"
#include <decomp/static_init.h>

#include "classes/FloatTween.h"
#include "revolution/GX/GXTypes.h"
#include "classes/CHashPath.h"

// PCM is interleaved signed 16-bit L/R: one stereo frame = four bytes.
// A queue message borrows a persistent three-word descriptor. That descriptor
// owns allocation; cursor/stereoFrames track the mixer's unconsumed portion.
// Ownership of reuse moves freeQueue -> producer -> readyQueue -> mixer -> freeQueue.
struct MovieAudioSampleBuffer {
    void* allocation;
    void* cursor;
    unsigned int stereoFrames;
};
struct MovieAudioFormat {
    char reserved[0x9C];
    unsigned int sampleFrames;
    char tail[4];
};
struct MovieAudioQueueStorage {
    void* freeMessages[6];
    void* readyMessages[6];
    char freeQueue[0x20];
    char readyQueue[0x20];
    char alignmentPadding[0x10];
    char dmaBuffers[2][0x180];
};
// Constant offsets keep all DMA/queue addressing tied to the documented layout.
enum MovieQueueStorageOffset {
    kMovieFreeQueueOffset = (int)&((MovieAudioQueueStorage*)0)->freeQueue,
    kMovieReadyQueueOffset = (int)&((MovieAudioQueueStorage*)0)->readyQueue,
    kMovieReadyMessagesOffset = (int)&((MovieAudioQueueStorage*)0)->readyMessages,
    kMovieDmaBuffersOffset = (int)&((MovieAudioQueueStorage*)0)->dmaBuffers
};
enum MovieAudioConstants {
    kCopyGamma1Point0 = 0,
    kMovieSampleBufferCount = 6,
    kSampleBufferWords = 3,
    kPcmBytesPerStereoFrame = 4,
    kSampleAllocationScale = 4,
    kAudioUnityQ15 = 32768,
    kDmaBufferBytes = 0x180,
    kDmaStereoFrames = 0x60,
    kDmaBufferCount = 2,
    kDmaInitialClearBytes = 0xC0,
    kDecoderReady = 2,
    kDecoderState3 = 3,
    kDecoderState4 = 4,
    kMovieScreenWidth = 640,
    kMovieScreenHeight = 480
};

extern "C" {
extern float kFloat_6_2831855;
extern const float kFloat_0_RenderTexture;
extern float kMovieProjectionFarZ;
extern float s_movieAudioFullTurn;
extern char s_streamVolumeFadeTween[0x20];
extern void* s_movieSampleBuffers[18];

extern char rotateRegistryVtable[];
extern void* s_audioMixPreserveStereo;

void FloatTween_Init(void*, float);
int FloatTween_Update(void*, float);
void gtl_Media_CreateFilterManager(void*, void*);
void Audio_InitializeDMAMixingQueues(void*);
extern volatile float s_audioMasterVolumeScale;
extern int s_audioVolumeQ15;
extern float kFloat_32768_SwingTimer;
extern void* g_resourceManager;

unsigned int OSEnableInterrupts();
void OSRestoreInterrupts(unsigned int);
unsigned int OSDisableInterrupts();
void DCFlushRange(void*, unsigned int);
void FloatTween_Start(void*, int, float, float);
int AICheckInit();
void* Memory_Clear(void*, unsigned int);
void OSInitMessageQueue(void*, void*, int);
void* AudioInterface_SetDmaCallback(void*);
void Audio_MixNextDMABuffer();
void __dl__FPv(void*);
void __dla__FPv(void*);
extern int s_audioDmaBufferIndex;
extern int g_currentAudioDmaBuffer;
extern int g_audioMixSourceBuffer;
extern int g_audioMixCurrentBuffer;
extern int s_previousAudioDmaCallback;
extern char s_movieAudioQueueStorage[0x100];
extern char s_audioDmaBuffers[0x300];
extern char s_movieSampleFreeQueue[0x20];
extern char s_movieSampleReadyQueue[0x20];
int AIGetDMAStartAddr();
void AIInitDMA(void*, int);
void DCInvalidateRange(void*, unsigned int);
void Audio_MixQueuedStereoSamples(void*, void*, int);
int OSReceiveMessage(void*, void**, int);
void Memory_Copy(void*, void*, unsigned int);
void OSSendMessage(void*, void*, int);
void MovieRenderer_DrawMediaSample(void*, void*);
void Memory_Copy(void*, void*, unsigned int);
void MovieRenderer_SetupGXState(void*, unsigned short, unsigned short);
void Media_DrawThreePlaneTextureQuad(void*, void*, void*, void*, int, int, int, int, unsigned short, unsigned short);
void GXSetAlphaUpdate(int);
void GXSetNumTexGens(int);
void GXSetNumChans(int);
void GXSetNumTevStages(int);
void GXSetTevSwapModeTable(int, int, int, int, int);
void GXSetTevOrder(int, int, int, int);
void GXSetTevKColorSel(int, int);
void GXSetTevColorIn(int, int, int, int, int);
void GXSetTevColorOp(int, int, int, int, int, int);
void GXSetTevAlphaIn(int, int, int, int, int);
void GXSetTevAlphaOp(int, int, int, int, int, int);
void* __nwa__FUl(unsigned int);
void C_MTXOrtho(void*, float, float, float, float, float, float);
void PSMTXIdentity(void*);
void GXSetProjection(void*, int);
void GXLoadPosMtxImm(void*, int);
void GXSetCurrentMtx(int);
void GXSetZMode(int, int, int);
void GXSetBlendMode(int, int, int, int);
void GXSetAlphaCompare(int, int, int, int, int);
void GXSetColorUpdate(int);
void GXSetDispCopyGamma(int);
void Engine_SetRenderStateList(void*, int);
void GXSetTexCoordGen2(int, int, int, int, int, int);
void GXInvalidateTexAll();
void GXClearVtxDesc();
void GXSetVtxDesc(int, int);
void GXSetVtxAttrFmt(int, int, int, int, int);
void GXSetTevKAlphaSel(int, int);
void GXSetTevColorS10(int, void*);
void GXSetTevKColor(int, void*);
struct MovieRenderStatePairs {
    int v[18];
};
extern MovieRenderStatePairs kMovieRenderStatePairs;
extern unsigned short kMovieYuvInitialColorR;
extern unsigned short kMovieYuvInitialColorG;
extern unsigned short kMovieYuvInitialColorB;
extern unsigned short kMovieYuvInitialColorA;
extern unsigned char kMovieYuvKonst0R;
extern unsigned char kMovieYuvKonst0G;
extern unsigned char kMovieYuvKonst0B;
extern unsigned char kMovieYuvKonst0A;
extern unsigned char kMovieYuvKonst1R;
extern unsigned char kMovieYuvKonst1G;
extern unsigned char kMovieYuvKonst1B;
extern unsigned char kMovieYuvKonst1A;
extern unsigned char kMovieYuvKonst2R;
extern unsigned char kMovieYuvKonst2G;
extern unsigned char kMovieYuvKonst2B;
extern unsigned char kMovieYuvKonst2A;

struct MovieStartView {
    void* pad00;
    void* manager;
    int pad08;
    int state;
};
struct MovieStartDecoder {
    virtual void m2();
    virtual void m3();
    virtual void m4();
    virtual void m5();
    virtual void getState(int*);
    virtual void configure(void*);
    virtual void m8();
    virtual int start();
    virtual void m10();
    virtual void m11();
    virtual void m12();
    virtual void m13();
    virtual void m14();
    virtual void m15();
    virtual void m16();
    virtual void m17();
    virtual void getAudioFormat(void*);
};
#pragma push
#pragma use_lmw_stmw on
#pragma opt_common_subs off
#pragma opt_loop_invariants off
// Configure a ready decoder, allocate six reusable PCM buffers, then start playback.
int Movie_ConfigureAndStartDecoder(MovieStartView* self, void* arg) {
    // Phase 1: require decoder state 2 or 4, configure it and obtain its format.
    // Return 0 on rejection; otherwise forward start()'s result unchanged.
    int type;
    MovieAudioFormat format;
    int ok;
    MovieStartDecoder* mgr = (MovieStartDecoder*)self->manager;
    if (mgr == 0) {
        ok = 0;
    } else {
        mgr->getState(&type);
        ok = 1;
        if (type != kDecoderReady && type != kDecoderState4)
            ok = 0;
    }
    if (ok == 0)
        return 0;
    ((MovieStartDecoder*)self->manager)->configure(arg);
    ((MovieStartDecoder*)self->manager)->getAudioFormat(&format);
    // Phase 2: replace the six descriptor-owned allocations. Caller must already
    // have quiesced consumers of any old sample buffers before reconfiguration.
    int i = 0;
    void* zero = (void*)i;
    for (i = 0; i < kMovieSampleBufferCount; i++) {
        int idx = i * kSampleBufferWords;
        if (s_movieSampleBuffers[idx] != 0) {
            __dla__FPv(s_movieSampleBuffers[idx]);
            s_movieSampleBuffers[idx] = zero;
        }
    }
    int formatFrameCount = (*(int*)&format.sampleFrames);
    if (formatFrameCount != 0) {
        void* data;
        int j;
        // Existing sizing contract: clear 4*N bytes but allocate 16*N bytes.
        // The extra factor of four is observed behavior; its rationale is unknown.
        // Do not shrink the allocation or expand the clear in a readability edit.
        int pcmClearBytes = formatFrameCount * kPcmBytesPerStereoFrame;
        for (j = 0; j < kMovieSampleBufferCount; j++) {
            int idx = j * kSampleBufferWords;
            Memory_Clear(&s_movieSampleBuffers[idx], 0x0C);
            data = __nwa__FUl(pcmClearBytes * kSampleAllocationScale);
            Memory_Clear(data, pcmClearBytes);
            s_movieSampleBuffers[idx] = data;
            OSSendMessage(s_movieSampleFreeQueue, &s_movieSampleBuffers[idx], 0);
        }
    }
    // Phase 3: publish readiness and hand control back to the decoder.
    self->state = 1;
    return ((MovieStartDecoder*)self->manager)->start();
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Resolve Y/U/V sample planes, draw a full-screen movie quad, then restore the base TEV setup.
void MovieRenderer_DrawMediaSample(void* self, void* resource) {
    if (resource == 0)
        return;
    struct ResInfo {
        void* field00;
        char pad0x94[0x94];
        unsigned short width;
        unsigned short height;
        char pad0xa0[8];
    };
    ResInfo info;
    ((gtl::CMediaSample*)resource)->CopyFormat((void**)&info);
    void* data;
    ((gtl::CMediaSample*)resource)->GetData((int*)&data);
    if (data == 0)
        return;
    struct PlanarVideoSample {
        unsigned int offsets[6];
    };
    PlanarVideoSample* planes = (PlanarVideoSample*)data;
    void* yPlane = (char*)data + planes->offsets[1];
    void* uPlane = (char*)data + planes->offsets[3];
    void* vPlane = (char*)data + planes->offsets[5];
    unsigned short height = info.height;
    unsigned short width = info.width;
    MovieRenderer_SetupGXState(self, width, height);
    Media_DrawThreePlaneTextureQuad(self, yPlane, uPlane, vPlane, 0, 0, width, height, kMovieScreenWidth,
                                    kMovieScreenHeight);
    GXSetAlphaUpdate(GX_TRUE);
    GXSetNumTexGens(1);
    GXSetNumChans(1);
    GXSetNumTevStages(1);
    GXSetTevSwapModeTable(GX_TEV_SWAP1, GX_CH_RED, GX_CH_ALPHA, GX_CH_BLUE, GX_CH_ALPHA);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_KONST, GX_CC_ONE, GX_CC_RASC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
}
#pragma pop

// Return the movie interface's constant zero result.
int Movie_ReturnZero() {
    return 0;
}

// Chain the previous AI callback, swap DMA buffers, and mix queued stereo PCM into the next buffer.
void Audio_MixNextDMABuffer() {
    // AI callback context: preserve the previous completed DMA source before
    // chaining the old callback, which may schedule a new hardware buffer.
    if (g_currentAudioDmaBuffer != 0)
        g_audioMixSourceBuffer = g_currentAudioDmaBuffer;
    if (s_previousAudioDmaCallback != 0)
        ((void (*)())s_previousAudioDmaCallback)();
    int current = AIGetDMAStartAddr();
    // AI reports a physical address; +0x80000000 obtains its cached CPU alias.
    g_currentAudioDmaBuffer = current + 0x80000000;
    int oldIndex = s_audioDmaBufferIndex;
    int index = oldIndex ^ 1;
    s_audioDmaBufferIndex = index;
    AIInitDMA(s_audioDmaBuffers + index * kDmaBufferBytes, kDmaBufferBytes);
    // Program the next 0x180-byte / 96-frame stereo buffer before doing CPU work.
    // The original protocol ENABLES interrupts while mixing and restores the
    // previous mask afterward; this is not a critical-section lock.
    unsigned int interrupts = OSEnableInterrupts();
    if (g_audioMixSourceBuffer != 0)
        DCInvalidateRange((void*)g_audioMixSourceBuffer, kDmaBufferBytes);
    Audio_MixQueuedStereoSamples(s_audioDmaBuffers + s_audioDmaBufferIndex * kDmaBufferBytes,
                                 (void*)g_audioMixSourceBuffer, kDmaStereoFrames);
    DCFlushRange(s_audioDmaBuffers + s_audioDmaBufferIndex * kDmaBufferBytes, kDmaBufferBytes);
    OSRestoreInterrupts(interrupts);
}

// Copy decoded stereo PCM into a free reusable buffer and queue it for the audio mixer.
// resource/data are borrowed for this call. Blocking receives/sends transfer the
// descriptor's reuse rights; the decoder's sample bytes are copied, never retained.
void Audio_QueueMediaSampleForMixing(void* self, void* resource) {
    if (resource == 0)
        return;
    void* data = 0;
    ((gtl::CMediaSample*)resource)->GetData((int*)&data);
    if (data == 0)
        return;
    void* queueMessage;
    void* sampleBuffer = OSReceiveMessage(s_movieSampleFreeQueue, &queueMessage, 1) == 1 ? queueMessage : 0;
    if (sampleBuffer == 0)
        return;
    struct AudioSampleDescriptor {
        unsigned int stereoFrames;
        unsigned int dataOffset;
    };
    AudioSampleDescriptor* sample = (AudioSampleDescriptor*)data;
    unsigned int pcmBytes = sample->stereoFrames * kPcmBytesPerStereoFrame;
    void* pcmData = (char*)sample + sample->dataOffset;
    DCFlushRange(pcmData, pcmBytes);
    Memory_Copy(((MovieAudioSampleBuffer*)sampleBuffer)->allocation, pcmData, pcmBytes);
    ((MovieAudioSampleBuffer*)sampleBuffer)->cursor = ((MovieAudioSampleBuffer*)sampleBuffer)->allocation;
    ((MovieAudioSampleBuffer*)sampleBuffer)->stereoFrames = sample->stereoFrames;
    OSSendMessage(s_movieSampleReadyQueue, sampleBuffer, 1);
}

// Decoder vtable slots used by this unit (slot numbers are vtable indices; the rest are unused here):
//   slot 17 = getVideoSample(out ref), slot 18 = getAudioSample(out ref), both hand back a
//   reference-counted sample object through *out (null when nothing is ready).
// Sample objects are reference counted: Ref::count is the first word and vtable slot 2 is
// the deleting destructor (called with flag 1) that runs when the last reference drops.
struct MovieDrawView {
    void* audioMixingEnabled;
    void* decoder;
    void* playbackReferences;
};
struct MovieSampleDecoder {
    virtual void m0();
    virtual void m1();
    virtual void m2();
    virtual void m3();
    virtual void m4();
    virtual void m5();
    virtual void m6();
    virtual void m7();
    virtual void m8();
    virtual void m9();
    virtual void m10();
    virtual void m11();
    virtual void m12();
    virtual void m13();
    virtual void m14();
    virtual void m15();
    virtual void m16();
    virtual void getVideoSample(void**);
    virtual void getAudioSample(void**);
    virtual void m19(void**);
};
// Draw the current video sample and queue its audio sample, releasing both acquired references.
int MovieRenderer_DrawCurrentSamples(MovieDrawView* self) {
    if (self->decoder == 0)
        return 0;
    struct Ref {
        int count;
        void** vtable;
    };
    Ref* buf = 0;
    // Acquire: video sample -> draw -> release (drop one reference, destroy at zero).
    ((MovieSampleDecoder*)self->decoder)->getVideoSample((void**)&buf);
    MovieRenderer_DrawMediaSample(self, buf);
    if (buf != 0) {
        Ref* p = buf;
        if (--p->count == 0) {
            ((void (*)(void*, int))p->vtable[2])(p, 1);
        }
        buf = 0;
    }
    if (self->audioMixingEnabled == 0)
        return 0;
    // Same acquire/consume/release pattern for the audio sample; the release below is the
    // same refcount drop as above, written out again by the original code.
    ((MovieSampleDecoder*)self->decoder)->getAudioSample((void**)&buf);
    Audio_QueueMediaSampleForMixing(self, buf);
    if (buf != 0) {
        Ref* p = buf;
        if (--p->count == 0) {
            ((void (*)(void*, int))p->vtable[2])(p, 1);
        }
    }
    return 0;
}

// Start a stream-volume fade, or apply an immediate master-scaled Q15 volume.
void Audio_SetStreamVolumeWithFade(float targetVolume, float fadeSeconds) {
    unsigned int interruptMask;
    float scaled;
    if (fadeSeconds > kFloat_0_RenderTexture) {
        FloatTween_Start(s_streamVolumeFadeTween, -1, targetVolume, fadeSeconds);
        return;
    }
    fadeSeconds = kFloat_0_RenderTexture;
    FloatTween_Start(s_streamVolumeFadeTween, -1, targetVolume, fadeSeconds);
    float masterVolume = s_audioMasterVolumeScale;
    float streamVolume = ((FloatTween*)s_streamVolumeFadeTween)->value; // Audio_SetStreamVolumeWithFade: tween +4
    scaled = masterVolume * streamVolume;
    interruptMask = OSEnableInterrupts();
    int volumeQ15 = (int)(kFloat_32768_SwingTimer * scaled);
    s_audioVolumeQ15 = volumeQ15;
    if (volumeQ15 < 0)
        s_audioVolumeQ15 = 0;
    else if (volumeQ15 > kAudioUnityQ15)
        s_audioVolumeQ15 = kAudioUnityQ15;
    OSRestoreInterrupts(interruptMask);
}

#pragma push
#pragma use_lmw_stmw on
#pragma opt_common_subs off
// Clear the DMA buffers, initialize sample queues, and install the mixing callback if AI is ready.
void Audio_InitializeDMAMixingQueues(void* self) {
    // Phase 1: leave the owner unchanged if AI has not initialized. Otherwise
    // clear only 0xC0 of each 0x180-byte DMA slot, exactly as the original does.
    char* base = s_movieAudioQueueStorage;
    if (AICheckInit() == 0)
        return;
    char* dmaBuffers = ((char*)(base + kMovieDmaBuffersOffset));
    // Startup clears only the first 0xC0 bytes of each 0x180-byte DMA buffer.
    int offset = 0;
    s_audioDmaBufferIndex = 0;
    int counter = 0;
    do {
        Memory_Clear(dmaBuffers + offset, kDmaInitialClearBytes);
        DCFlushRange(dmaBuffers + offset, kDmaInitialClearBytes);
        counter = counter + 1;
        offset = offset + kDmaBufferBytes;
    } while (counter < kDmaBufferCount);
    int zero = 0;
    g_currentAudioDmaBuffer = zero;
    g_audioMixSourceBuffer = zero;
    OSInitMessageQueue(((char*)(base + kMovieFreeQueueOffset)), base, kMovieSampleBufferCount);
    OSInitMessageQueue(((char*)(base + kMovieReadyQueueOffset)), base + kMovieReadyMessagesOffset,
                       kMovieSampleBufferCount);
    g_audioMixCurrentBuffer = zero;
    // Phase 2: publish both queues before installing our callback atomically with
    // respect to AI interrupts; retain the old callback for chaining/restoration.
    counter = OSDisableInterrupts();
    s_previousAudioDmaCallback = (int)AudioInterface_SetDmaCallback((void*)Audio_MixNextDMABuffer);
    OSRestoreInterrupts(counter);
    *(int*)self = 1;
}
#pragma pop

// Slot map for the decoder as used here: isStopped() is vtable slot 2 (polled until it returns
// nonzero) and release() is slot 20 (destroys the decoder object). The other slots are not
// called from this unit and are left unnamed (m<N> = slot N).
struct MovieCloseView {
    int pad0;
    void* decoder;
    int pad08;
    int sampleBuffersReady;
};
struct MovieCloseDecoder {
    virtual void m0();
    virtual void m1();
    virtual int isStopped();
    virtual void m3();
    virtual int m4();
    virtual void m5();
    virtual void m6();
    virtual void m7();
    virtual void m8();
    virtual void m9();
    virtual void m10();
    virtual void m11();
    virtual void m12();
    virtual void m13();
    virtual void m14();
    virtual void m15();
    virtual void m16();
    virtual void m17();
    virtual void m18();
    virtual void m19();
    virtual void release();
    virtual void m21();
    virtual void m22();
};
#pragma push
#pragma use_lmw_stmw on
// Wait for decoder stop, free PCM buffers, restore the prior AI callback and release the decoder.
void MovieAudio_ReleaseSampleBuffersAndManager(MovieCloseView* self) {
    // Phase 1: busy-poll until the decoder reports stopped; no timeout/yield.
    // Callers must stop production first. This path assumes the mix callback
    // cannot consume freed descriptors during the subsequent buffer-release loop.
    if (self->decoder != 0) {
        while (!((MovieCloseDecoder*)self->decoder)->isStopped()) {
        }
    }
    for (int i = 0; i < kMovieSampleBufferCount; ++i) {
        int index = i * kSampleBufferWords;
        if (s_movieSampleBuffers[index] != 0) {
            __dla__FPv(s_movieSampleBuffers[index]);
            s_movieSampleBuffers[index] = 0;
        }
    }
    // Phase 2: restore the saved callback while interrupts are disabled, then
    // clear ready state and release the decoder. A null saved callback is skipped.
    unsigned int mask = OSDisableInterrupts();
    if (s_previousAudioDmaCallback != 0)
        AudioInterface_SetDmaCallback((void*)s_previousAudioDmaCallback);
    OSRestoreInterrupts(mask);
    s_previousAudioDmaCallback = 0;
    MovieCloseDecoder* manager = (MovieCloseDecoder*)self->decoder;
    self->sampleBuffersReady = 0;
    if (manager != 0) {
        manager->release();
        self->decoder = 0;
    }
}
#pragma pop

// Clear the four words of initial movie playback state.
void MoviePlaybackState_Init(int* p) {
    p[0] = 0;
    p[1] = 0;
    p[2] = 0;
    p[3] = 0;
}

// Initialize the stream-volume tween and this unit's angle constant.
void MovieAudio_InitializeVolumeTween() {
    s_movieAudioFullTurn = kFloat_6_2831855;
    FloatTween_Init(s_streamVolumeFadeTween, kFloat_0_RenderTexture);
}

struct RotateRegistryFields {
    void* vtable;       // 0x00
    void* owner;        // 0x04
    int treeHeader;     // 0x08
    int treeRoot;       // 0x0C
    void* treeSentinel; // 0x10
};

struct MovieStateDecoder {
    virtual void m0();
    virtual void m1();
    virtual void m2();
    virtual void m3();
    virtual void getState(int*);
};

struct MoviePlaybackDecoder {
    virtual void m0();
    virtual void m1();
    virtual void m2();
    virtual void m3();
    virtual void m4();
    virtual void m5();
    virtual void m6();
    virtual void pause();
    virtual void stop();
    virtual void resume();
    virtual void m10();
};

struct MovieQueryView {
    void* vtable;
    MovieStateDecoder* decoder;
};

struct MoviePlaybackView {
    void* vtable;
    MoviePlaybackDecoder* decoder;
    int playbackReferences;
    int sampleBuffersReady;
};

struct MovieBufferView {
    void* vtable;
    MoviePlaybackDecoder* decoder;
    int playbackReferences;
    int sampleBuffersReady;
};

// Report whether the attached decoder is in state 2 or 4.
int Movie_IsDecoderReadyOrState4(MovieQueryView* self) {
    MovieStateDecoder* p = self->decoder;
    if (p == 0)
        return 0;
    int decoderState;
    p->getState(&decoderState);
    return decoderState == kDecoderReady || decoderState == kDecoderState4;
}

// Resume on the first playback reference when sample buffers are ready, then increment the count.
void Movie_AcquirePlaybackReference(MoviePlaybackView* self) {
    MoviePlaybackDecoder* p = self->decoder;
    if (p != 0) {
        if (self->playbackReferences == 0 && self->sampleBuffersReady != 0)
            p->resume();
        self->playbackReferences++;
    }
}

#pragma push
#pragma use_lmw_stmw on
// Stop the decoder, free reusable PCM buffers, and clear the buffer-ready flag.
void Movie_ReleaseSampleBuffers(MovieBufferView* self) {
    // This shorter path requests stop but does not wait or restore the callback;
    // callers must ensure no producer/mixer can still access the freed allocations.
    MoviePlaybackDecoder* p = self->decoder;
    if (p != 0)
        p->stop();
    for (int i = 0; i < kMovieSampleBufferCount; i++) {
        int idx = i * kSampleBufferWords;
        if (s_movieSampleBuffers[idx] != 0) {
            __dla__FPv(s_movieSampleBuffers[idx]);
            s_movieSampleBuffers[idx] = 0;
        }
    }
    self->sampleBuffersReady = 0;
}
#pragma pop

// Initialize the owner and empty registry tree, including its self-referencing sentinel.
void RotateRegistry_Init(RotateRegistryFields* self, void* owner) {
    self->vtable = (void*)rotateRegistryVtable;
    self->owner = owner;
    self->treeHeader = 0;
    self->treeRoot = 0;
    self->treeSentinel = (void*)&self->treeRoot;
}

// Preserve stereo when nonzero; zero folds the queued stereo samples to mono.
// Store the flag under the original interrupt mask protocol.
void AudioMix_SetPreserveStereo(void* preserveStereo) {
    unsigned int mask = OSEnableInterrupts();
    s_audioMixPreserveStereo = preserveStereo;
    OSRestoreInterrupts(mask);
}

// Free this audio object only for a deleting destructor call.
void* MoviePlayback_DeleteIfFlagged(void* self, int flag) {
    if (self != 0) {
        if (flag > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Clamp the playback reference count at zero and pause when the last ready reference is released.
void Movie_ReleasePlaybackReference(MoviePlaybackView* self) {
    MoviePlaybackDecoder* p = (MoviePlaybackDecoder*)self->decoder;
    if (p == 0)
        return;
    int cnt = self->playbackReferences;
    cnt = cnt - 1;
    self->playbackReferences = cnt;
    if (cnt > 0)
        return;
    int zero = 0;
    self->playbackReferences = zero;
    if (self->sampleBuffersReady == 0)
        return;
    p->pause();
}

// Query whether the decoder reports state 3; an absent decoder returns false.
int Movie_IsDecoderState3(MoviePlaybackView* self) {
    MovieStateDecoder* p = (MovieStateDecoder*)self->decoder;
    if (p == 0)
        return 0;
    int decoderState;
    p->getState(&decoderState);
    return decoderState == kDecoderState3;
}

// Query whether the decoder reports state 4; an absent decoder returns false.
int Movie_IsDecoderState4(MoviePlaybackView* self) {
    MovieStateDecoder* p = (MovieStateDecoder*)self->decoder;
    if (p == 0)
        return 0;
    int decoderState;
    p->getState(&decoderState);
    return decoderState == kDecoderState4;
}
// Apply master times stream volume as a saturated Q15 mixer gain.
void MovieAudio_SetMasterVolume(float value) {
    float masterVolume = (float)(double)value;
    s_audioMasterVolumeScale = value;
    float streamVolume = ((FloatTween*)s_streamVolumeFadeTween)->value; // MovieAudio_SetMasterVolume: tween +4
    float scaled = masterVolume * streamVolume;
    unsigned int interruptMask = OSEnableInterrupts();
    int volumeQ15 = (int)(kFloat_32768_SwingTimer * scaled);
    s_audioVolumeQ15 = volumeQ15;
    if (volumeQ15 < 0)
        s_audioVolumeQ15 = 0;
    else if (volumeQ15 > kAudioUnityQ15)
        s_audioVolumeQ15 = kAudioUnityQ15;
    OSRestoreInterrupts(interruptMask);
}

struct MovieOpenView {
    int pad0;
    void* manager;
    int playbackReferences;
};
struct MovieDecoderCreateArgs {
    int a;
    int b;
};
struct MovieOpenDecoder {
    virtual void m0();
    virtual int openResource(void*, int);
};
// Create a media filter manager, initialize mixing, and open the resolved movie resource.
int Movie_OpenResource(MovieOpenView* self, int arg, int a, int b) {
    MovieDecoderCreateArgs args;
    args.a = a;
    args.b = b;
    gtl_Media_CreateFilterManager(&self->manager, &args);
    if (self->manager == 0)
        return 0;
    Audio_InitializeDMAMixingQueues(self);
    self->playbackReferences = 0;
    void* resource = ((CHashPath*)g_resourceManager)->FindPathForwarder((const char*)arg);
    return ((MovieOpenDecoder*)self->manager)->openResource(resource, 1);
}

// Advance the stream-volume tween and apply the saturated master-scaled Q15 gain.
void Audio_UpdateStreamVolumeFade(float elapsedSeconds) {
    if (FloatTween_Update(s_streamVolumeFadeTween, elapsedSeconds) == 0)
        return;
    float masterVolume = s_audioMasterVolumeScale;
    float streamVolume = ((FloatTween*)s_streamVolumeFadeTween)->value; // Audio_UpdateStreamVolumeFade: tween +4
    float scaled = masterVolume * streamVolume;
    unsigned int interruptMask = OSEnableInterrupts();
    int volumeQ15 = (int)(kFloat_32768_SwingTimer * scaled);
    s_audioVolumeQ15 = volumeQ15;
    if (volumeQ15 < 0)
        s_audioVolumeQ15 = 0;
    else if (volumeQ15 > kAudioUnityQ15)
        s_audioVolumeQ15 = kAudioUnityQ15;
    OSRestoreInterrupts(interruptMask);
}

void GXInitTexObj(void*, void*, unsigned int, unsigned int, int, int, int, int);
void GXInitTexObjLOD(void*, int, int, float, float, float, int, int, int);
void GXLoadTexObj(void*, int);
void GXBegin(int, int, int);
// Load full-resolution Y and half-resolution U/V textures, then emit a screen-space GX quad.
void Media_DrawThreePlaneTextureQuad(void* renderer, void* yPlane, void* uPlane, void* vPlane, int screenX, int screenY,
                                     int textureWidth, int textureHeight, unsigned short screenWidth,
                                     unsigned short screenHeight) {
    // Planes are borrowed YUV 4:2:0 bytes (even texture dimensions expected).
    // Screen coordinates/sizes are pixels; texture coordinates are integer 0/1.
    char yTexture[0x20];
    char uTexture[0x20];
    char vTexture[0x20];
    GXInitTexObj(yTexture, yPlane, (unsigned int)textureWidth, (unsigned int)textureHeight, GX_TF_I8, GX_CLAMP,
                 GX_CLAMP, GX_FALSE);
    GXInitTexObjLOD(yTexture, GX_NEAR, GX_NEAR, kFloat_0_RenderTexture, kFloat_0_RenderTexture, kFloat_0_RenderTexture,
                    GX_FALSE, GX_FALSE, GX_ANISO_1);
    GXLoadTexObj(yTexture, GX_TEXMAP0);
    int chromaHeight = textureHeight >> 1;
    int chromaWidth = (unsigned short)textureWidth >> 1;
    GXInitTexObj(uTexture, uPlane, (unsigned int)chromaWidth, (unsigned int)chromaHeight, GX_TF_I8, GX_CLAMP, GX_CLAMP,
                 GX_FALSE);
    GXInitTexObjLOD(uTexture, GX_NEAR, GX_NEAR, kFloat_0_RenderTexture, kFloat_0_RenderTexture, kFloat_0_RenderTexture,
                    GX_FALSE, GX_FALSE, GX_ANISO_1);
    GXLoadTexObj(uTexture, GX_TEXMAP1);
    GXInitTexObj(vTexture, vPlane, (unsigned int)chromaWidth, (unsigned short)chromaHeight, GX_TF_I8, GX_CLAMP,
                 GX_CLAMP, GX_FALSE);
    GXInitTexObjLOD(vTexture, GX_NEAR, GX_NEAR, kFloat_0_RenderTexture, kFloat_0_RenderTexture, kFloat_0_RenderTexture,
                    GX_FALSE, GX_FALSE, GX_ANISO_1);
    GXLoadTexObj(vTexture, GX_TEXMAP2);
    GXBegin(GX_QUADS, GX_VTXFMT7, 4);
    // GX FIFO emits, per vertex, XYZ as signed-16 bit patterns followed by ST
    // as unsigned-16 values. Keep volatile write order: each store feeds hardware.
    volatile unsigned short* fifo = (volatile unsigned short*)0xCC008000;
    *fifo = screenX;
    *fifo = screenY;
    *fifo = 0;
    *fifo = 0;
    *fifo = 0;
    *fifo = screenX + screenWidth;
    *fifo = screenY;
    *fifo = 0;
    *fifo = 1;
    *fifo = 0;
    *fifo = screenX + screenWidth;
    *fifo = screenY + screenHeight;
    *fifo = 0;
    *fifo = 1;
    *fifo = 1;
    *fifo = screenX;
    *fifo = screenY + screenHeight;
    *fifo = 0;
    *fifo = 0;
    *fifo = 1;
}

struct RegistryTreeNode {
    void* left;
    void* right;
};
struct RotateRegistryTreeView {
    char prefix[8];
    int header;
    RegistryTreeNode* root;
};
struct RotateRegistryVtableView {
    char prefix[0x10];
    void (*releaseEntries)(void*);
};
// Borrow the embedded tree header used as allocator/context for recursive cleanup.
static inline void* RotateRegistryTreeHeader(void* object) {
    return (void*)&((RotateRegistryTreeView*)object)->header;
}
void RBTree_DestroySubtree(void*, void*);
void appFreeUni__FPv(void*);
#pragma push
#pragma use_lmw_stmw on
// Invoke the registry cleanup hook, destroy its tree, and optionally free the owner.
void* RotateRegistry_dtor(void* object, int flag) {
    // The registry cleanup hook handles entries before nodes are freed. The
    // following three levels are an unrolled post-order traversal; deeper
    // branches delegate to RBTree_DestroySubtree. This is registry cleanup,
    // independent of decoder/audio stop sequencing. The embedded header survives.
    if (object != 0) {
        *(void**)object = rotateRegistryVtable;
        ((RotateRegistryVtableView*)rotateRegistryVtable)->releaseEntries(object);
        RegistryTreeNode* top = (RegistryTreeNode*)((RotateRegistryTreeView*)object)->root;
        if (top != 0) {
            RegistryTreeNode* g;
            RegistryTreeNode* p;
            RegistryTreeNode* c;
            g = (RegistryTreeNode*)top->left;
            if (g != 0) {
                p = (RegistryTreeNode*)g->left;
                if (p != 0) {
                    c = (RegistryTreeNode*)p->left;
                    if (c != 0) {
                        if (c->left != 0)
                            RBTree_DestroySubtree(RotateRegistryTreeHeader(object), c->left);
                        if (c->right != 0)
                            RBTree_DestroySubtree(RotateRegistryTreeHeader(object), c->right);
                        appFreeUni__FPv(c);
                    }
                    c = (RegistryTreeNode*)p->right;
                    if (c != 0) {
                        if (c->left != 0)
                            RBTree_DestroySubtree(RotateRegistryTreeHeader(object), c->left);
                        if (c->right != 0)
                            RBTree_DestroySubtree(RotateRegistryTreeHeader(object), c->right);
                        appFreeUni__FPv(c);
                    }
                    appFreeUni__FPv(p);
                }
                p = (RegistryTreeNode*)g->right;
                if (p != 0) {
                    c = (RegistryTreeNode*)p->left;
                    if (c != 0) {
                        if (c->left != 0)
                            RBTree_DestroySubtree(RotateRegistryTreeHeader(object), c->left);
                        if (c->right != 0)
                            RBTree_DestroySubtree(RotateRegistryTreeHeader(object), c->right);
                        appFreeUni__FPv(c);
                    }
                    c = (RegistryTreeNode*)p->right;
                    if (c != 0) {
                        if (c->left != 0)
                            RBTree_DestroySubtree(RotateRegistryTreeHeader(object), c->left);
                        if (c->right != 0)
                            RBTree_DestroySubtree(RotateRegistryTreeHeader(object), c->right);
                        appFreeUni__FPv(c);
                    }
                    appFreeUni__FPv(p);
                }
                appFreeUni__FPv(g);
            }
            g = (RegistryTreeNode*)top->right;
            if (g != 0) {
                p = (RegistryTreeNode*)g->left;
                if (p != 0) {
                    c = (RegistryTreeNode*)p->left;
                    if (c != 0) {
                        if (c->left != 0)
                            RBTree_DestroySubtree(RotateRegistryTreeHeader(object), c->left);
                        if (c->right != 0)
                            RBTree_DestroySubtree(RotateRegistryTreeHeader(object), c->right);
                        appFreeUni__FPv(c);
                    }
                    c = (RegistryTreeNode*)p->right;
                    if (c != 0) {
                        if (c->left != 0)
                            RBTree_DestroySubtree(RotateRegistryTreeHeader(object), c->left);
                        if (c->right != 0)
                            RBTree_DestroySubtree(RotateRegistryTreeHeader(object), c->right);
                        appFreeUni__FPv(c);
                    }
                    appFreeUni__FPv(p);
                }
                p = (RegistryTreeNode*)g->right;
                if (p != 0) {
                    c = (RegistryTreeNode*)p->left;
                    if (c != 0) {
                        if (c->left != 0)
                            RBTree_DestroySubtree(RotateRegistryTreeHeader(object), c->left);
                        if (c->right != 0)
                            RBTree_DestroySubtree(RotateRegistryTreeHeader(object), c->right);
                        appFreeUni__FPv(c);
                    }
                    c = (RegistryTreeNode*)p->right;
                    if (c != 0) {
                        if (c->left != 0)
                            RBTree_DestroySubtree(RotateRegistryTreeHeader(object), c->left);
                        if (c->right != 0)
                            RBTree_DestroySubtree(RotateRegistryTreeHeader(object), c->right);
                        appFreeUni__FPv(c);
                    }
                    appFreeUni__FPv(p);
                }
                appFreeUni__FPv(g);
            }
            appFreeUni__FPv(top);
        }
        if (flag > 0)
            __dl__FPv(object);
    }
    return object;
}
#pragma pop
// Configure orthographic GX drawing and four TEV stages that turn planar YUV into RGB.
void MovieRenderer_SetupGXState(void* self, unsigned short width, unsigned short height) {
    // Projection uses source-image pixel dimensions; the emitted quad separately
    // supplies its destination dimensions. Texture maps 0/1/2 hold Y/U/V.
    (void)self;
    float mtx[16];
    C_MTXOrtho(mtx, kFloat_0_RenderTexture, (float)height, kFloat_0_RenderTexture, (float)width, kFloat_0_RenderTexture,
               kMovieProjectionFarZ);
    GXSetProjection(mtx, GX_ORTHOGRAPHIC);
    float mtx2[12];
    PSMTXIdentity(mtx2);
    GXLoadPosMtxImm(mtx2, GX_PNMTX0);
    GXSetCurrentMtx(GX_PNMTX0);
    GXSetZMode(GX_TRUE, GX_ALWAYS, GX_FALSE);
    GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GXSetColorUpdate(GX_TRUE);
    GXSetAlphaUpdate(GX_FALSE);
    GXSetDispCopyGamma(kCopyGamma1Point0);
    MovieRenderStatePairs arr = kMovieRenderStatePairs;
    Engine_SetRenderStateList(&arr, 9);
    GXSetNumChans(1);
    GXSetNumTexGens(2);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    GXInvalidateTexAll();
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT7, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
    GXSetVtxAttrFmt(GX_VTXFMT7, GX_VA_TEX0, GX_TEX_ST, GX_U16, 0);
    GXSetNumTevStages(4);
    // Stages 0 and 1 combine chroma; stage 2 adds luma; stage 3 applies the final bias.
    // Per-channel equations implied by the TEV inputs below (map1 = U, map2 = V, map0 = Y;
    // C0/A0 = TEVREG0, K0/K1/K2 = the three konst colors; the numeric constants live in
    // data and are not restated here). TEV out = d + (1-c)*a + c*b, bias 0:
    //   stage 0  C = C0 + U*K0               A = A0 - Ua*K0a
    //   stage 1  C = 2*(C + V*K1)            A = A - Va*K1a
    //   stage 2  C = C + Y                   A = A + Ya        (both clamped)
    //   stage 3  out = A*(1-K2) + C*K2       (APREV is used as a color input: lerp by K2)
    // Together these form the YUV-to-RGB matrix; the konst colors set its coefficients.
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_C0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_A0);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
    GXSetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
    GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP2, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_CPREV);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_2, GX_FALSE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_APREV);
    GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
    GXSetTevKColorSel(GX_TEVSTAGE1, GX_TEV_KCSEL_K1);
    GXSetTevKAlphaSel(GX_TEVSTAGE1, GX_TEV_KASEL_K1_A);
    GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_TEXC, GX_CC_ONE, GX_CC_CPREV);
    GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE2, GX_CA_TEXA, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
    GXSetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevOrder(GX_TEVSTAGE3, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE3, GX_CC_APREV, GX_CC_CPREV, GX_CC_KONST, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE3, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevKColorSel(GX_TEVSTAGE3, GX_TEV_KCSEL_K2);
    unsigned short h0 = kMovieYuvInitialColorR;
    unsigned short h1 = kMovieYuvInitialColorG;
    unsigned short h2 = kMovieYuvInitialColorB;
    unsigned short h3 = kMovieYuvInitialColorA;
    unsigned short bufh[4] = {h0, h1, h2, h3};
    GXSetTevColorS10(GX_TEVREG0, bufh);
    unsigned char b0 = kMovieYuvKonst0R;
    unsigned char b1 = kMovieYuvKonst0G;
    unsigned char b2 = kMovieYuvKonst0B;
    unsigned char b3 = kMovieYuvKonst0A;
    unsigned char bufb0[4] = {b0, b1, b2, b3};
    GXSetTevKColor(GX_KCOLOR0, bufb0);
    unsigned char c0 = kMovieYuvKonst1R;
    unsigned char c1 = kMovieYuvKonst1G;
    unsigned char c2 = kMovieYuvKonst1B;
    unsigned char c3 = kMovieYuvKonst1A;
    unsigned char bufb1[4] = {c0, c1, c2, c3};
    GXSetTevKColor(GX_KCOLOR1, bufb1);
    unsigned char d0 = kMovieYuvKonst2R;
    unsigned char d1 = kMovieYuvKonst2G;
    unsigned char d2 = kMovieYuvKonst2B;
    unsigned char d3 = kMovieYuvKonst2A;
    unsigned char bufb2[4] = {d0, d1, d2, d3};
    GXSetTevKColor(GX_KCOLOR2, bufb2);
    GXSetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_ALPHA);
}
} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462304", _ctors_u8025D2A4_0, MovieAudio_InitializeVolumeTween);
#pragma pop
