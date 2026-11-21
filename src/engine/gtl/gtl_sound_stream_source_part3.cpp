// Translation unit 0x804044DC-0x8040465C (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

struct StreamHeader {
    u8 format, loop, channels, reserved;
    u16 rate, flags;
    u32 samples, start, zero, end, current, field1c, size, field24, aligned, field2c;
    u16 adpcmChannelParameters[28];
};

// Header scalar fields occupy 0x30 bytes. adpcmChannelParameters holds
// mixed ADPCM decoder state, not just filter coefficients. Each destination
// channel has 28 halfwords; each source channel has 24. Only the channel windows
// move in the loop below, so scalar fields are written exactly once.

// ADPCM source channels occupy 24 halfwords; playback channels occupy 28.
struct SourceChannelBlock { u16 parameters[24]; };
struct PlaybackChannelBlock { u16 parameters[28]; };
enum {
    kStreamAlignment = 32, kPlaybackChannelSlots = 2, kCopiedAdpcmHalfwords = 22,
    kAdpcmState20 = 20, kAdpcmState21 = 21, kAdpcmState22 = 22,
    kAdpcmTrailingState = 23
};

namespace gtl {
struct CSoundStreamSource {
    u8 pad00[0x10];
    u32 rate;
    u16 field14, channels;
    u32 samples, start, field20, current, flags, end, field30, field34, size;
    u16 adpcmChannelParameters[48];
};
}

extern "C" {
StreamHeader* gtl_ISoundStream_GetPlaybackHeader(void*);

// Build the playback header and copy both ADPCM blocks, clearing three decoder-state halfwords.
// self is borrowed; stream must supply writable header storage for both channels.
// Returns 1 unconditionally, with no validation of pointers or source channel count.
int gtl_CSoundStreamSource_BuildPlaybackHeader(const gtl::CSoundStreamSource* self, void* stream) {
    StreamHeader* out = gtl_ISoundStream_GetPlaybackHeader(stream);
    // Copy playback metadata; size/aligned are byte counts with 32-byte alignment.
    // Legacy sample/block field names are retained; exact units of each are unresolved here.
    // Format/loop are forced to zero; rate/flags narrow to 16 bits, channels to 8 bits.
    out->format = 0;
    out->loop = 0;
    out->channels = self->channels;
    out->reserved = 0;
    out->rate = self->rate;
    out->flags = self->flags;
    out->samples = self->samples;
    out->start = self->start;
    out->zero = 0;
    out->end = self->end;
    out->current = self->current;
    out->field1c = self->field30;
    out->size = self->size;
    out->field24 = self->field34;
    out->aligned = (self->size + kStreamAlignment - 1) & ~(kStreamAlignment - 1);
    out->field2c = 0;
    // Both channel records are processed even for a mono source. Copy 0..21 and 23,
    // then reset 20..22; 24..27 in the destination are left untouched.
    for (u32 channel = 0; channel < kPlaybackChannelSlots; ++channel) {
        for (u32 halfword = 0; halfword < kCopiedAdpcmHalfwords; ++halfword)
            out->adpcmChannelParameters[halfword] = self->adpcmChannelParameters[halfword];
        out->adpcmChannelParameters[kAdpcmTrailingState] = self->adpcmChannelParameters[kAdpcmTrailingState];
        out->adpcmChannelParameters[kAdpcmState20] = 0;
        out->adpcmChannelParameters[kAdpcmState21] = 0;
        out->adpcmChannelParameters[kAdpcmState22] = 0;
        // Advance the channel windows; no second complete source/header object is implied.
        self = (const gtl::CSoundStreamSource*)((const SourceChannelBlock*)self + 1);
        out = (StreamHeader*)((PlaybackChannelBlock*)out + 1);
    }
    return 1;
}

} // extern "C"
