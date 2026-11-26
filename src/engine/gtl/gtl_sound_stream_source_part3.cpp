// Playback header of a streamed sound source (gtl::CSoundStreamSource): BuildPlaybackHeader writes
// the scalar fields and the channel windows into the stream's playback header.
//
// Other source functions are in gtl_sound_stream_source_part1.cpp, part2.cpp and part4.cpp.

#include "classes/SoundStreamSource.h"
#include "classes/GtlSoundStreamInterface.h"
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

// Playback header written into the stream: scalar fields, then two channel windows of 28 halfwords.
struct StreamHeader {
    u8 format, loop, channels, reserved;
    u16 rate, flags;
    u32 samples, start, zero, end, current, field1c, size, field24, aligned, field2c;
    u16 adpcmChannelParameters[28];
};

// ADPCM source channels occupy 24 halfwords; playback channels occupy 28.
struct SourceChannelBlock {
    u16 parameters[24];
};
struct PlaybackChannelBlock {
    u16 parameters[28];
};
enum {
    kStreamAlignment = 32,
    kPlaybackChannelSlots = 2,
    kCopiedAdpcmHalfwords = 22,
    kAdpcmState20 = 20,
    kAdpcmState21 = 21,
    kAdpcmState22 = 22,
    kAdpcmTrailingState = 23
};

// Build the playback header and copy both ADPCM blocks, clearing three decoder-state halfwords.
// stream must supply writable header storage for both channels. Returns 1 unconditionally.
// config[1..6] supply the current, flags, end, field30, field34 and size words.
int gtl::CSoundStreamSource::BuildPlaybackHeader(void* stream) const {
    StreamHeader* out = (StreamHeader*)((gtl::ISoundStream*)stream)->GetPlaybackHeader();
    // Copy playback metadata; size/aligned are byte counts with 32-byte alignment.
    out->format = 0;
    out->loop = 0;
    out->channels = channelCount;
    out->reserved = 0;
    out->rate = (u32)sampleRate;
    out->flags = config[2];
    out->samples = (u32)sampleCount;
    out->start = (u32)streamData;
    out->zero = 0;
    out->end = config[3];
    out->current = config[1];
    out->field1c = config[4];
    out->size = config[6];
    out->field24 = config[5];
    out->aligned = (config[6] + kStreamAlignment - 1) & ~(kStreamAlignment - 1);
    out->field2c = 0;
    // Both channel records are processed even for a mono source. Copy 0..21 and 23,
    // then reset 20..22; 24..27 in the destination are left untouched.
    const gtl::CSoundStreamSource* self = this;
    for (u32 channel = 0; channel < kPlaybackChannelSlots; ++channel) {
        for (u32 halfword = 0; halfword < kCopiedAdpcmHalfwords; ++halfword)
            out->adpcmChannelParameters[halfword] = self->adpcmChannelParameters[halfword];
        out->adpcmChannelParameters[kAdpcmTrailingState] = self->adpcmChannelParameters[kAdpcmTrailingState];
        out->adpcmChannelParameters[kAdpcmState20] = 0;
        out->adpcmChannelParameters[kAdpcmState21] = 0;
        out->adpcmChannelParameters[kAdpcmState22] = 0;
        // Advance to the next channel window in both the source and the header.
        self = (const gtl::CSoundStreamSource*)((const SourceChannelBlock*)self + 1);
        out = (StreamHeader*)((PlaybackChannelBlock*)out + 1);
    }
    return 1;
}
