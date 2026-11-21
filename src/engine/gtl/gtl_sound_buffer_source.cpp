// Translation unit 0x80403A8C-0x80403C14 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"
#pragma use_lmw_stmw on
#pragma scheduling on

enum { kDspDecoderStateHalfwords = 23 };

// Playback channels are 0x44 bytes apart. The state-only view below advances
// by the same stride from the first channel's decoder state at +0x14.
struct Channel {
    void* samples;
    char padBeforeDecoderState[0x10];
    unsigned short state[kDspDecoderStateHalfwords];
    char padChannelEnd[2];
};
struct ChannelState {
    unsigned short state[kDspDecoderStateHalfwords];
    char pad[22];
};
struct Buffer {
    // +0x00: playback flags/channel count; +0x04/+0x08/+0x0c: format, rate, length.
    // rate/length are forwarded unchanged; their units are not inferred from this copy alone.
    unsigned char playbackFlag0, playbackFlag1, channels, padHeader;
    unsigned int format, rate, length;
    Channel channel[1]; // Variable-length tail with one entry per channel.
};
struct Data {
    char padBase[8];
    char* base;
};
namespace gtl {
struct CSoundBufferSource {
    char padBase[8];
    unsigned int size; // Aggregate source bytes, divided by the output channel count for cache flush.
    char padBeforeFormat[4];
    unsigned int format;
    unsigned short formatAuxiliary, channels;
    unsigned int rate, length;
    Data* data;
    unsigned int offsets[1]; // Variable tail: byte offsets from data->base to each DSP header.
};
}

// DSP channel data has a 0x60-byte header; 23 halfwords beginning at 0x1c
// supply the decoder coefficients/history used by the playback channel.
struct DspChannelData {
    char padBeforeDecoderState[0x1c];
    unsigned short decoderState[kDspDecoderStateHalfwords];
    char padBeforeSamples[0x16];
    char samples[1];
};
static const unsigned int kDspSampleOffset = (unsigned int)&((DspChannelData*)0)->samples;

extern "C" {
Buffer* gtl_ISoundBuffer_GetPlaybackHeader(void*);

// Fill a playback header and flush/copy each source channel before audio DMA reads it.
// Returns 0 for a null soundBuffer, otherwise 1. self/data/header and all channel tails must be valid.
// Output sample pointers borrow the source allocation; no samples are copied or ownership transferred.
// Each DSP header is 0x60 bytes; channel ordering follows offsets[], without a lane permutation.
// Source offsets/header storage must fit all declared channels; no format or bounds validation occurs here.
int CSoundBufferSource_BindToPlaybackBuffer(gtl::CSoundBufferSource* __restrict self, void* soundBuffer) {
    char* channelData;
    gtl::CSoundBufferSource* offsetTableCursor;
    Buffer* playbackChannelCursor;
    ChannelState* state;
    Buffer* buffer;
    char* base;
    int channelIndex;
    unsigned char zero;
    if (!soundBuffer) return 0;
    // Publish the common playback format and reset both playback flag bytes.
    buffer = gtl_ISoundBuffer_GetPlaybackHeader(soundBuffer);
    zero = 0;
    buffer->playbackFlag0 = zero;
    buffer->playbackFlag1 = zero;
    buffer->channels = self->channels;
    buffer->format = (unsigned short)self->format;
    buffer->rate = self->rate;
    buffer->length = self->length;
    base = self->data->base;
    state = (ChannelState*)buffer->channel[0].state;
    offsetTableCursor = self;
    playbackChannelCursor = buffer;
    // Walk the offset tail (4-byte stride) and playback channels (0x44-byte stride) together.
    for (channelIndex = 0; channelIndex < buffer->channels; ++channelIndex) {
        channelData = base + offsetTableCursor->offsets[0];
        DCFlushRange(channelData, self->size / buffer->channels);
        playbackChannelCursor->channel[0].samples = channelData + kDspSampleOffset;
        // Copy all 46 decoder-state bytes verbatim; preserve the explicit load/store order.
        state->state[0] = ((DspChannelData*)channelData)->decoderState[0];
        state->state[1] = ((DspChannelData*)channelData)->decoderState[1];
        state->state[2] = ((DspChannelData*)channelData)->decoderState[2];
        state->state[3] = ((DspChannelData*)channelData)->decoderState[3];
        state->state[4] = ((DspChannelData*)channelData)->decoderState[4];
        state->state[5] = ((DspChannelData*)channelData)->decoderState[5];
        state->state[6] = ((DspChannelData*)channelData)->decoderState[6];
        state->state[7] = ((DspChannelData*)channelData)->decoderState[7];
        state->state[8] = ((DspChannelData*)channelData)->decoderState[8];
        state->state[9] = ((DspChannelData*)channelData)->decoderState[9];
        state->state[10] = ((DspChannelData*)channelData)->decoderState[10];
        state->state[11] = ((DspChannelData*)channelData)->decoderState[11];
        state->state[12] = ((DspChannelData*)channelData)->decoderState[12];
        state->state[13] = ((DspChannelData*)channelData)->decoderState[13];
        state->state[14] = ((DspChannelData*)channelData)->decoderState[14];
        state->state[15] = ((DspChannelData*)channelData)->decoderState[15];
        state->state[16] = ((DspChannelData*)channelData)->decoderState[16];
        state->state[17] = ((DspChannelData*)channelData)->decoderState[17];
        state->state[18] = ((DspChannelData*)channelData)->decoderState[18];
        state->state[19] = ((DspChannelData*)channelData)->decoderState[19];
        state->state[20] = ((DspChannelData*)channelData)->decoderState[20];
        state->state[21] = ((DspChannelData*)channelData)->decoderState[21];
        state->state[22] = ((DspChannelData*)channelData)->decoderState[22];
        ++state;
        offsetTableCursor = (gtl::CSoundBufferSource*)((char*)offsetTableCursor + sizeof(self->offsets[0]));
        playbackChannelCursor = (Buffer*)((char*)playbackChannelCursor + sizeof(Channel));
    }
    return 1;
}

} // extern "C"
