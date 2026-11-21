// State consumed by GameMD5_Update (0x8029DD8C) and GameMD5_Final (0x8029DE94).
// Layout only; the final two block words receive the 64-bit input bit count.
#ifndef CLASSES_MD5CONTEXT_H
#define CLASSES_MD5CONTEXT_H
struct MD5Context {
    unsigned int digest[4];  // 0x0: GameMD5_Transform accumulates the four state words
    unsigned int bitCountLow; // 0x10: GameMD5_Update adds len << 3
    unsigned int bitCountHigh; // 0x14: carry and len >> 29
    unsigned int blockWords[16]; // 0x18: pending 64-byte transform block
};
#define OFF(f, o) typedef char MD5Context_off_##f[((int)&((MD5Context*)0)->f == (o)) ? 1 : -1]
OFF(digest, 0);
OFF(bitCountLow, 0x10);
OFF(bitCountHigh, 0x14);
OFF(blockWords, 0x18);
#undef OFF
#endif
