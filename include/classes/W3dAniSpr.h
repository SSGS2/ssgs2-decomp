// W3dAniSpr: explicit layout view; no inheritance, virtual methods, or RTTI.
// Field comments cite the original function addresses that establish their use.
#ifndef CLASSES_W3DANISPR_H
#define CLASSES_W3DANISPR_H

#include "game/types.h"
#include "game/wvector.h"

class W3dAniSpr {
public:
    void* vtbl; // 0x0: meaning unknown; retained offset name
    char pad_004[0xC];
    int resourceManager; // 0x10: W3dAniSpr_LoadSpritesInOneTexture: ResourceManager_LoadBitmap first argument
    int bitmapHead; // 0x14: W3dAniSpr_Ctor, CleanupList: Head
    int bitmapCursor; // 0x18: W3dAniSpr_Ctor, CleanupList: Cursor
    int bitmapBlocks; // 0x1C: W3dAniSpr_Ctor, CleanupList: Blocks
    int bitmapFree; // 0x20: W3dAniSpr_Ctor, CleanupList: Free
    int bitmapBlockSize; // 0x24: W3dAniSpr_Ctor, CleanupList: BlockSize
    char pad_028[0x4];
    int bitmapBucketCount; // 0x2C: W3dAniSpr_Ctor, CleanupList: BucketCount
    int bitmapBuckets; // 0x30: W3dAniSpr_Ctor, CleanupList: Buckets
    char bitmapMutex[0x18]; // 0x34: W3dAniSpr_Ctor (0x80217E9C) initializes; dtor locks bitmap list here
    int frameHead; // 0x4C: W3dAniSpr_Ctor, CleanupList: Head
    int frameCursor; // 0x50: W3dAniSpr_Ctor, CleanupList: Cursor
    int frameBlocks; // 0x54: W3dAniSpr_Ctor, CleanupList: Blocks
    int frameFree; // 0x58: W3dAniSpr_Ctor, CleanupList: Free
    int frameBlockSize; // 0x5C: W3dAniSpr_Ctor, CleanupList: BlockSize
    char pad_060[0x4];
    int frameBucketCount; // 0x64: W3dAniSpr_Ctor, CleanupList: BucketCount
    int frameBuckets; // 0x68: W3dAniSpr_Ctor, CleanupList: Buckets
    char frameMutex[0x18]; // 0x6C: W3dAniSpr_Ctor initializes; W3dAniSpr_DrawFrame locks frame list here
    int bitmapCount; // 0x84: W3dAniSpr_LoadSpritesInOneTexture: incremented after appending a bitmap
    int frameCount; // 0x88: W3dAniSpr_LoadSpritesInOneTexture/8021962C: incremented per grid cell
    float left; // 0x8C: W3dAniSpr_SetSizeAndPivot: -horizontal pivot
    float right; // 0x90: W3dAniSpr_SetSizeAndPivot: width minus horizontal pivot
    float top; // 0x94: W3dAniSpr_SetSizeAndPivot: -vertical pivot
    float bottom; // 0x98: W3dAniSpr_SetSizeAndPivot: height minus vertical pivot
    WVector position; // 0x9C: W3dAniSpr_SetPosition: vector assignment; W3dAniSpr_DrawFrame: added to transformed corner
    char pad_0A8[0x18];
    float vertex0U; // 0xC0: frame's left U coordinate
    float vertex0V; // 0xC4: frame's top V coordinate
    char pad_0C8[0x30];
    float vertex1U; // 0xF8: frame's right U coordinate
    float vertex1V; // 0xFC: frame's top V coordinate
    char pad_100[0x30];
    float vertex2U; // 0x130: frame's right U coordinate
    float vertex2V; // 0x134: frame's bottom V coordinate
    char pad_138[0x30];
    float vertex3U; // 0x168: frame's left U coordinate
    float vertex3V; // 0x16C: frame's bottom V coordinate
    char pad_170[0x18];
    void* vertex0; // 0x188: W3dAniSpr_Ctor/80218A10: corner vertex pointer
    void* vertex1; // 0x18C: W3dAniSpr_Ctor/80218A10: corner vertex pointer
    void* vertex2; // 0x190: W3dAniSpr_Ctor/80218A10: corner vertex pointer
    void* vertex3; // 0x194: W3dAniSpr_Ctor/80218A10: corner vertex pointer
};

#define W3DANISPR_OFF(f, o) typedef char W3dAniSpr_off_##f[((int)&((W3dAniSpr*)0)->f == (o)) ? 1 : -1]
W3DANISPR_OFF(vtbl, 0x0);
W3DANISPR_OFF(resourceManager, 0x10);
W3DANISPR_OFF(bitmapHead, 0x14);
W3DANISPR_OFF(bitmapCursor, 0x18);
W3DANISPR_OFF(bitmapBlocks, 0x1C);
W3DANISPR_OFF(bitmapFree, 0x20);
W3DANISPR_OFF(bitmapBlockSize, 0x24);
W3DANISPR_OFF(bitmapBucketCount, 0x2C);
W3DANISPR_OFF(bitmapBuckets, 0x30);
W3DANISPR_OFF(bitmapMutex, 0x34);
W3DANISPR_OFF(frameHead, 0x4C);
W3DANISPR_OFF(frameCursor, 0x50);
W3DANISPR_OFF(frameBlocks, 0x54);
W3DANISPR_OFF(frameFree, 0x58);
W3DANISPR_OFF(frameBlockSize, 0x5C);
W3DANISPR_OFF(frameBucketCount, 0x64);
W3DANISPR_OFF(frameBuckets, 0x68);
W3DANISPR_OFF(frameMutex, 0x6C);
W3DANISPR_OFF(bitmapCount, 0x84);
W3DANISPR_OFF(frameCount, 0x88);
W3DANISPR_OFF(left, 0x8C);
W3DANISPR_OFF(right, 0x90);
W3DANISPR_OFF(top, 0x94);
W3DANISPR_OFF(bottom, 0x98);
W3DANISPR_OFF(position, 0x9C);
W3DANISPR_OFF(vertex0U, 0xC0);
W3DANISPR_OFF(vertex0V, 0xC4);
W3DANISPR_OFF(vertex1U, 0xF8);
W3DANISPR_OFF(vertex1V, 0xFC);
W3DANISPR_OFF(vertex2U, 0x130);
W3DANISPR_OFF(vertex2V, 0x134);
W3DANISPR_OFF(vertex3U, 0x168);
W3DANISPR_OFF(vertex3V, 0x16C);
W3DANISPR_OFF(vertex0, 0x188);
W3DANISPR_OFF(vertex1, 0x18C);
W3DANISPR_OFF(vertex2, 0x190);
W3DANISPR_OFF(vertex3, 0x194);
#undef W3DANISPR_OFF

#endif
