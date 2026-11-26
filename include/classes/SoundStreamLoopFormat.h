// Embedded format at CSoundStreamPlayer +0x548, used by
// gtl_ISoundStream_Play (0x804022BC) before gtl_nw4r_detail_StrmPlayer_PreparePlayback receives its address.
#ifndef CLASSES_SOUNDSTREAMLOOPFORMAT_H
#define CLASSES_SOUNDSTREAMLOOPFORMAT_H
struct SoundStreamLoopFormat {
    char pad_000[8];
    unsigned int loopStart; // +0x08: copied from player's loopStart
    unsigned int loopEnd;   // +0x0C: copied from player's loopEnd
    char pad_010[4];
    unsigned int blockCount;            // +0x14: ceil(loopEnd / samplesPerBlock)
    unsigned int blockBytes;            // +0x18: copied to lastBlockBytes when count changes
    unsigned int samplesPerBlock;       // +0x1C: divisor for loop-end sample count
    unsigned int lastBlockBytes;        // +0x20: size rounded for lastBlockAlignedBytes
    unsigned int lastBlockSamples;      // +0x24: loopEnd % samplesPerBlock, zero replaced by full block
    unsigned int lastBlockAlignedBytes; // +0x28: (lastBlockBytes + 31) & ~31
};
#define OFF(f, o) typedef char SoundStreamLoopFormat_off_##f[((int)&((SoundStreamLoopFormat*)0)->f == (o)) ? 1 : -1]
OFF(loopStart, 0x08);
OFF(loopEnd, 0x0C);
OFF(blockCount, 0x14);
OFF(blockBytes, 0x18);
OFF(samplesPerBlock, 0x1C);
OFF(lastBlockBytes, 0x20);
OFF(lastBlockSamples, 0x24);
OFF(lastBlockAlignedBytes, 0x28);
#undef OFF
#define AsSoundStreamLoopFormat(p) ((SoundStreamLoopFormat*)(p))
#endif
