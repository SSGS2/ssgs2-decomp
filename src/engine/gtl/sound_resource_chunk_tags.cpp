// Sound resource chunk tags: the four-character tags (GSND, HEAD, DATA and the others) that
// identify sound resource chunks, packed from the shared string pool at startup. The same
// startup call sets the sound resource's full-turn constant.

#include <decomp/static_init.h>

extern "C" {

extern float kFloat_6_2831855;
extern unsigned char soundResourceFormatStrings[];
extern float s_soundResourceTau;
extern unsigned int s_soundResourceTagGSND;
extern unsigned int s_soundChunkTagHEAD;
extern unsigned int s_soundResourceDataChunkTag;
extern unsigned int s_soundBsicChunkTag;
extern unsigned int s_audioNameChunkTag;
extern unsigned int s_audioChunkTag_EV_C;
extern unsigned int s_audioChunkTag_EQ_C;
extern unsigned int s_audioChunkTag_RO_C;
extern unsigned int s_soundResourceEmitChunkTag;
extern unsigned int s_soundResourceTagGCEX;

struct SoundTagStrings {
    unsigned char exceptionMessages[0xB8];
    unsigned char sound[5], head[5], data[5], basic[5], name[5];
    // Legacy field names below expand abbreviations without established semantics.
    // Actual bytes are "EV-C", "EQ-C", "RO-C", "EMIT", "GCEX" (each plus NUL).
    unsigned char tagEV_C[5], tagEQ_C[5], tagRO_C[5], emit[5], extension[5];
};

enum {
    kTagSound = (int)&((SoundTagStrings*)0)->sound,
    kTagHead = (int)&((SoundTagStrings*)0)->head,
    kTagData = (int)&((SoundTagStrings*)0)->data,
    kTagBasic = (int)&((SoundTagStrings*)0)->basic,
    kTagName = (int)&((SoundTagStrings*)0)->name,
    kTagEV_C = (int)&((SoundTagStrings*)0)->tagEV_C,
    kTagEQ_C = (int)&((SoundTagStrings*)0)->tagEQ_C,
    kTagRO_C = (int)&((SoundTagStrings*)0)->tagRO_C,
    kTagEmit = (int)&((SoundTagStrings*)0)->emit,
    kTagExtension = (int)&((SoundTagStrings*)0)->extension
};

// Pack four pool bytes into a big-endian tag word; do not include the NUL byte.
// Known tags are ASCII. Preserve the original signed extension for bytes 1..3.
inline unsigned int PackSoundTag(int poolOffset) {
    return (signed char)soundResourceFormatStrings[poolOffset + 3] |
           ((signed char)soundResourceFormatStrings[poolOffset + 2] << 8) |
           ((soundResourceFormatStrings[poolOffset] << 24) |
            ((signed char)soundResourceFormatStrings[poolOffset + 1] << 16));
}

// Initialize tau and the GSND container/chunk tags from the shared string pool.
// Startup-only writes to process-global caches; no allocation or ownership transfer.
// Tag spelling, rather than an inferred interpretation, defines the file contract:
// GSND, HEAD, DATA, BSIC, NAME, EV-C, EQ-C, RO-C, EMIT, GCEX.
void SoundResource_InitializeChunkTags(void) {
    // Publish the angle and the first five tag words directly.
    s_soundResourceTau = kFloat_6_2831855;
    s_soundResourceTagGSND = PackSoundTag(kTagSound);
    s_soundChunkTagHEAD = PackSoundTag(kTagHead);
    s_soundResourceDataChunkTag = PackSoundTag(kTagData);
    s_soundBsicChunkTag = PackSoundTag(kTagBasic);
    s_audioNameChunkTag = PackSoundTag(kTagName);
    // Stage the remaining words before publishing; preserve the original load/store order.
    unsigned int emitTag = PackSoundTag(kTagEmit);
    unsigned int evCTag = PackSoundTag(kTagEV_C);
    unsigned int eqCTag = PackSoundTag(kTagEQ_C);
    unsigned int roCTag = PackSoundTag(kTagRO_C);
    unsigned int extensionTag = PackSoundTag(kTagExtension);
    s_audioChunkTag_EV_C = evCTag;
    s_audioChunkTag_EQ_C = eqCTag;
    s_audioChunkTag_RO_C = roCTag;
    s_soundResourceEmitChunkTag = emitTag;
    s_soundResourceTagGCEX = extensionTag;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462724", _ctors_u8043E858_0, SoundResource_InitializeChunkTags);
#pragma pop
