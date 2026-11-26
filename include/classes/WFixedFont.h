// WFixedFont: offset-preserving view; no constructors, bases, or virtual methods.
// Evidence: WFixedFont_dtor (0x8021C59C) moves nodes to a free list and deletes resources only when borrowedResources is zero; CloneOverlay initializes slotCount=4 and mutex; WFixedFont_LoadMessageResources loads messageBuffer.
#ifndef CLASSES_WFIXEDFONT_H
#define CLASSES_WFIXEDFONT_H

struct WFixedFont {
    void* vtbl; // 0x0
    char pad_004[0xC];
    void* resourceOwner; // 0x10
    char pad_014[0x1C];
    unsigned int unk_30; // 0x30
    unsigned int unk_34; // 0x34
    float glyphScale;    // 0x38
    char pad_03C[0x4];
    unsigned int redChannelMask;   // 0x40
    unsigned int greenChannelMask; // 0x44
    unsigned int blueChannelMask;  // 0x48
    char pad_04C[0x18];
    unsigned char unk_64; // 0x64
    char pad_065[0x3];
    void* activeHead;      // 0x68
    void* activeIterator;  // 0x6C
    void* allocationHead;  // 0x70
    void* recycledHead;    // 0x74
    unsigned int unk_78;   // 0x78
    unsigned int slotMask; // 0x7C
    int slotCount;         // 0x80
    void* slots;           // 0x84
    char mutex[0x18];      // 0x88
    unsigned char unk_A0;  // 0xA0
    unsigned char unk_A1;  // 0xA1
    char pad_0A2[0x2];
    void* resource;                  // 0xA4
    unsigned char borrowedResources; // 0xA8
    char pad_0A9[0x3];
    void* messageResource; // 0xAC
    void* messageDraw;     // 0xB0
    void* messageBuffer;   // 0xB4
};

#define OFF(f, o) typedef char WFixedFont_off_##f[((int)&((WFixedFont*)0)->f == (o)) ? 1 : -1]
OFF(vtbl, 0x0);
OFF(resourceOwner, 0x10);
OFF(unk_30, 0x30);
OFF(unk_34, 0x34);
OFF(glyphScale, 0x38);
OFF(redChannelMask, 0x40);
OFF(greenChannelMask, 0x44);
OFF(blueChannelMask, 0x48);
OFF(unk_64, 0x64);
OFF(activeHead, 0x68);
OFF(activeIterator, 0x6C);
OFF(allocationHead, 0x70);
OFF(recycledHead, 0x74);
OFF(unk_78, 0x78);
OFF(slotMask, 0x7C);
OFF(slotCount, 0x80);
OFF(slots, 0x84);
OFF(mutex, 0x88);
OFF(unk_A0, 0xA0);
OFF(unk_A1, 0xA1);
OFF(resource, 0xA4);
OFF(borrowedResources, 0xA8);
OFF(messageResource, 0xAC);
OFF(messageDraw, 0xB0);
OFF(messageBuffer, 0xB4);
#undef OFF
typedef char WFixedFont_size[(sizeof(WFixedFont) == 0xB8) ? 1 : -1];
#define AsWFixedFont(p) ((WFixedFont*)(p))

#endif // CLASSES_WFIXEDFONT_H
