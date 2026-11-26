// GTL texture pixels: native DXT1/3/5 block decode, image scatter, and per-pixel
// conversion/copy callbacks. No gameplay knobs or texture allocation here.
// Edit index: endpoint interpolation -> DecodeColorBlock; alpha palettes ->
// DecodeDXT5AlphaBlock; block traversal/format dispatch -> DecompressImage;
// channel order/packing -> PixelConvert_*; float range -> ClampByte.
// Format sizing consumer: ImageFormat_GetBitsPerPixel (gtl_image_layout_and_sizing.cpp). Conversion
// dispatch stays in Image_ConvertPixelFormat; this file supplies its callbacks.
// All buffers are borrowed, required and caller-sized. Use disjoint buffers for
// image conversion and channel swaps; swaps are sequential stores, not in-place
// swaps. Simple Copy callbacks inherit Memory_Copy's overlap contract. No errors
// are caught; the image decoder returns 1 even for unsupported DXT type values.

#include "types.h"
#include <decomp/cw_bitfields.h>

struct PixelFloat4 {
    float red;   // red, normalized [0,1] (clamped on conversion)
    float green; // green
    float blue;  // blue
    float alpha; // alpha
};

struct DXT3Block {
    u8 alpha[8];
    u8 color[8];
};
// Same source bytes, different format records: DXT1 is one 8-byte color block;
// DXT3/5 prepend 8 alpha bytes to that color record (16 bytes total). Native
// halfword/word order is intentional; these are not little-endian DDS parsers.
struct DxtColorBlock {
    u16 endpoint0;
    u16 endpoint1;
    u32 selectors;
};
typedef char DxtColorBlock_size[(sizeof(DxtColorBlock) == 8) ? 1 : -1];

struct PixelRGBA8 {
    s8 red;   // red byte, bits represent unsigned 0..255 despite signed storage
    s8 green; // green byte
    s8 blue;  // blue byte
    s8 alpha; // alpha byte (0 transparent, 255 opaque)
};
enum DxtImageType {
    kDxt1 = 0x2E,
    kDxt1Alternate = 0x62,
    kDxt3 = 0x30,
    kDxt3Alternate = 0x64,
    kDxt5 = 0x32,
    kDxt5Alternate = 0x66,
    kPackedAlpha4Destination = 0x21
};

extern "C" {

extern const float kNormalizedChannelLowerClamp; // 0: normalized lower clamp
extern const float kNormalizedChannelUpperClamp; // 1: normalized upper clamp
extern const float kByteChannelMaximum;          // 255: float/RGB565 -> byte scale
extern const float kDxtAlphaByteMaximum;         // 255: DXT3 alpha byte maximum
extern const float kDxtAlphaNibbleMaximum;       // 15: four-bit alpha maximum (also packed output)

void* memset(void*, int, unsigned int);
void Memory_Copy(void* dst, const void* src, u32 size);
extern const float kRgb565FiveBitMaximum; // 31: five-bit RGB565 component maximum
extern const float kRgb565SixBitMaximum;  // 63: six-bit green maximum
s32 gtl_ImageFormat_GetBitsPerPixel(s32 format);
extern const f32 kDxtThreeColorMidpoint; // 1/2: three-color midpoint
extern const f32 kDxtNearEndpointWeight; // float32 2/3: near-endpoint weight
extern const f32 kDxtFarEndpointWeight;  // float32 1/3: far-endpoint weight
// These are shared conversion coefficients, not independent visual tuning knobs.
// Preserve float multiply/add order and cast/truncation, including signed byte stores.

void gtl_PixelConvert_RGB565ToRGBA8(u8* dst, const u16* src);

// Decode 8 input bytes into 16 row-major RGBA pixels (4x4). +0/+2 are native
// u16 RGB565 endpoints, +4 native u32 selectors, low two bits select first pixel.
// Supply the game's resource byte order; no DDS/little-endian normalization here.
// threeColorMode!=0: selector2 midpoint, selector3 transparent. Zero: selectors
// 2/3 use 2:1/1:2 interpolation, all opaque. Caller decides mode from endpoints;
// endpoint equality deliberately takes FOUR-color mode in DecompressImage.
void gtl_DXT_DecodeColorBlock(PixelRGBA8* out, const u8* blockBytes, s32 threeColorMode) {
    u8* outputBytes = (u8*)out;
    const f32& twoThirds = kDxtNearEndpointWeight;
    const f32& oneThird = kDxtFarEndpointWeight;
    const f32& half = kDxtThreeColorMidpoint;
    u8* pixelBytes;
    s32 selectorShift;
    const u8* endpoint1 = blockBytes + 2;
    u32 pixelIndex;
    pixelIndex = 0;
    selectorShift = 0;
    pixelBytes = outputBytes;

    do {
        s32 selector = (((const DxtColorBlock*)blockBytes)->selectors >> selectorShift) & 3;
        switch (selector) {
        case 0:
            gtl_PixelConvert_RGB565ToRGBA8(pixelBytes, (const u16*)blockBytes);
            break;
        case 1:
            gtl_PixelConvert_RGB565ToRGBA8(pixelBytes, (const u16*)endpoint1);
            break;
        case 2:
            if (threeColorMode != 0) {
                u8 c1[4];
                u8 c0[4];
                gtl_PixelConvert_RGB565ToRGBA8(c0, (const u16*)blockBytes);
                gtl_PixelConvert_RGB565ToRGBA8(c1, (const u16*)endpoint1);
                pixelBytes[2] = (s8)(((f32)c0[2] * half) + ((f32)c1[2] * half));
                pixelBytes[1] = (s8)(((f32)c0[1] * half) + ((f32)c1[1] * half));
                pixelBytes[0] = (u8)(s32)(((f32)c0[0] * half) + ((f32)c1[0] * half));
                pixelBytes[3] = 0xFF;
            } else {
                u8 c1[4];
                u8 c0[4];
                gtl_PixelConvert_RGB565ToRGBA8(c0, (const u16*)blockBytes);
                gtl_PixelConvert_RGB565ToRGBA8(c1, (const u16*)endpoint1);
                pixelBytes[2] = (s8)(((f32)c0[2] * twoThirds) + ((f32)c1[2] * oneThird));
                pixelBytes[1] = (s8)(((f32)c0[1] * twoThirds) + ((f32)c1[1] * oneThird));
                pixelBytes[0] = (u8)(s32)(((f32)c0[0] * twoThirds) + ((f32)c1[0] * oneThird));
                pixelBytes[3] = 0xFF;
            }
            break;
        case 3:
            if (threeColorMode != 0) {
                pixelBytes[2] = 0;
                pixelBytes[1] = 0;
                pixelBytes[0] = 0;
                pixelBytes[3] = 0;
            } else {
                u8 c1[4];
                u8 c0[4];
                gtl_PixelConvert_RGB565ToRGBA8(c0, (const u16*)blockBytes);
                gtl_PixelConvert_RGB565ToRGBA8(c1, (const u16*)endpoint1);
                pixelBytes[2] = (s8)(((f32)c0[2] * oneThird) + ((f32)c1[2] * twoThirds));
                pixelBytes[1] = (s8)(((f32)c0[1] * oneThird) + ((f32)c1[1] * twoThirds));
                pixelBytes[0] = (u8)(s32)(((f32)c0[0] * oneThird) + ((f32)c1[0] * twoThirds));
                pixelBytes[3] = 0xFF;
            }
            break;
        }
        pixelBytes += 4;
        pixelIndex += 1;
        selectorShift += 2;
    } while (pixelIndex < 0x10U);
}

void gtl_Image_ConvertPixelFormat(u8* dst, PixelRGBA8* src, s32 format, s32 a, s32 b, s32 c);

// Select the larger referenced component without changing comparison/NaN policy.
static inline const float& FloatMax(const float& a, const float& b) {
    return a < b ? b : a;
}

// Select the smaller referenced component; reference lifetime stays with caller.
static inline const float& FloatMin(const float& a, const float& b) {
    return b < a ? b : a;
}

// Clamp normalized finite input to [0,1], scale by 255; caller truncates to byte.
// Alpha template keeps the original load order of the two pool values.
template <bool Alpha> static inline float ClampByte(const float& input) {
    float one;
    float zero;
    float value;
    if (Alpha) {
        zero = kNormalizedChannelLowerClamp;
        one = kNormalizedChannelUpperClamp;
    } else {
        one = kNormalizedChannelUpperClamp;
        zero = kNormalizedChannelLowerClamp;
    }
    value = input;
    float selected = FloatMin(FloatMax(value, zero), one);
    float scale = kByteChannelMaximum;
    return scale * selected;
}

#pragma push
#pragma use_lmw_stmw on
// Replace only alpha in 16 RGBA pixels; RGB is untouched. Endpoints a0>a1 make
// six /7 interpolants; otherwise four /5 interpolants plus 0/255. Integer biases
// +3/+2 round to nearest. Native u32 loads select three-bit indices from two
// groups of eight; load at +5 reads through +8, so provide at least NINE readable
// bytes, as in the enclosing 16-byte DXT5 block. Loads may be unaligned: retain
// this legacy source representation rather than treating it as a portable DDS API.
void gtl_DXT_DecodeDXT5AlphaBlock(PixelRGBA8* outputPixels, const u8* sourceAlphaBlock) {
    u8 alphaPalette[8];
    u8 endpoint0 = sourceAlphaBlock[0];
    u8 endpoint1 = sourceAlphaBlock[1];
    alphaPalette[0] = endpoint0;
    alphaPalette[1] = endpoint1;
    if (endpoint0 > endpoint1) {
        alphaPalette[2] = (6 * endpoint0 + endpoint1 + 3) / 7u;
        alphaPalette[3] = (5 * endpoint0 + 2 * endpoint1 + 3) / 7u;
        alphaPalette[4] = (4 * endpoint0 + 3 * endpoint1 + 3) / 7u;
        alphaPalette[5] = (3 * endpoint0 + 4 * endpoint1 + 3) / 7u;
        alphaPalette[6] = (2 * endpoint0 + 5 * endpoint1 + 3) / 7u;
        alphaPalette[7] = (endpoint0 + 6 * endpoint1 + 3) / 7u;
    } else {
        alphaPalette[6] = 0;
        alphaPalette[7] = 0xFF;
        alphaPalette[2] = (4 * endpoint0 + endpoint1 + 2) / 5u;
        alphaPalette[3] = (3 * endpoint0 + 2 * endpoint1 + 2) / 5u;
        alphaPalette[4] = (2 * endpoint0 + 3 * endpoint1 + 2) / 5u;
        alphaPalette[5] = (endpoint0 + 4 * endpoint1 + 2) / 5u;
    }
    for (u32 i = 0; i < 16; ++i) {
        u32 bits = *(const u32*)(sourceAlphaBlock + 2 + 3 * (i >> 3));
        outputPixels->alpha = alphaPalette[(bits >> (3 * (i & 7) + 8)) & 7];
        ++outputPixels;
    }
}
#pragma pop

// Decode a row-major grid of 4x4 blocks, scatter to a linear tightly packed image.
// width/height are texels, positive multiples of four; products must fit u32.
// src needs width*height/16 blocks: 8 bytes for DXT1, 16 for DXT3/5. Base must
// support native u16/u32 color loads and the alpha decoder's unaligned loads.
// dst needs (GetBitsPerPixel(format)*width*height)>>3 bytes. format is the
// DESTINATION converter's domain: 0x21 packs alpha nibbles (even X LOW nibble,
// odd X HIGH nibble: shift is 0 for even X, 4 for odd X);
// other format words are passed to Image_ConvertPixelFormat with (1,5,0).
// In the converter (gtl_image_format_dispatch.cpp, destination layout 0), a source layout of 5
// returns without writing, so non-0x21 destinations stay cleared by memset. The call and its
// argument order stay as they are; cleanups must not change this behavior.
// type is a SOURCE format from DxtImageType, not a
// destination format. DXT3/5 always use opaque four-color interpolation before
// replacing alpha. No crop/mip/tile/pitch handling, no bounds/error checks.
// Clears dst first; unsupported type leaves block scratch undefined, then still
// scatters it and returns 1. Validate formats before calling, not via this result.
s32 gtl_DXT_DecompressImage(u8* dst, s32 format, u8* src, u32 width, u32 height, s32 type) {
    PixelRGBA8 decodedPixels[16];
    u32 alphaPixelIndex;
    u32 blockCount;
    u32 blocksPerRow;
    u32 blockIndex;
    u32 texelCount;
    u32 destinationBitsPerPixel;
    u32 ctr;
    // Parallel format cursors, not three allocations or successive phases.
    // Only the cursor selected by type is read in each iteration. All advance
    // together so DXT1 addresses blockIndex*8 and DXT3/5 blockIndex*16. Keeping
    // those increments preserves the original loop's instruction sequence.
    u8* colorBlocks8;
    u8* alphaColorBlocks16;
    DXT3Block* explicitAlphaBlocks16;
    PixelRGBA8* decodedPixel;
    u32 blockColumn;
    u32 blockRow;
    u32 blockPixelIndex;
    u32 imageX;
    u32 imageY;
    u32 destinationByteOffset;
    PixelRGBA8* alphaOutput;

    texelCount = width * height;
    destinationBitsPerPixel = gtl_ImageFormat_GetBitsPerPixel(format);
    memset(dst, 0, (destinationBitsPerPixel * texelCount) >> 3);
    colorBlocks8 = src;
    alphaColorBlocks16 = src;
    explicitAlphaBlocks16 = (DXT3Block*)src;
    blockCount = texelCount >> 4;
    blocksPerRow = width >> 2;
    for (blockIndex = 0; blockIndex < blockCount; blockIndex++) {
        switch (type) {
        case kDxt1:
        case kDxt1Alternate:
            if (((DxtColorBlock*)colorBlocks8)->endpoint0 < ((DxtColorBlock*)colorBlocks8)->endpoint1) {
                gtl_DXT_DecodeColorBlock(decodedPixels, colorBlocks8, 1);
            } else {
                gtl_DXT_DecodeColorBlock(decodedPixels, colorBlocks8, 0);
            }
            break;
        case kDxt3:
        case kDxt3Alternate:
            gtl_DXT_DecodeColorBlock(decodedPixels, alphaColorBlocks16 + 8, 0);
            alphaOutput = decodedPixels;
            for (alphaPixelIndex = 0; alphaPixelIndex < 16; alphaPixelIndex += 8) {
                s8 t0 =
                    (s8)((kDxtAlphaByteMaximum * (f32)(u32)((explicitAlphaBlocks16->alpha[(alphaPixelIndex + 0) >> 1] >>
                                                             (((alphaPixelIndex + 0) & 1) * 4)) &
                                                            0xF)) /
                         kDxtAlphaNibbleMaximum);
                s8 t1 =
                    (s8)((kDxtAlphaByteMaximum * (f32)(u32)((explicitAlphaBlocks16->alpha[(alphaPixelIndex + 1) >> 1] >>
                                                             (((alphaPixelIndex + 1) & 1) * 4)) &
                                                            0xF)) /
                         kDxtAlphaNibbleMaximum);
                s8 t2 =
                    (s8)((kDxtAlphaByteMaximum * (f32)(u32)((explicitAlphaBlocks16->alpha[(alphaPixelIndex + 2) >> 1] >>
                                                             (((alphaPixelIndex + 2) & 1) * 4)) &
                                                            0xF)) /
                         kDxtAlphaNibbleMaximum);
                s8 t3 =
                    (s8)((kDxtAlphaByteMaximum * (f32)(u32)((explicitAlphaBlocks16->alpha[(alphaPixelIndex + 3) >> 1] >>
                                                             (((alphaPixelIndex + 3) & 1) * 4)) &
                                                            0xF)) /
                         kDxtAlphaNibbleMaximum);
                s8 t4 =
                    (s8)((kDxtAlphaByteMaximum * (f32)(u32)((explicitAlphaBlocks16->alpha[(alphaPixelIndex + 4) >> 1] >>
                                                             (((alphaPixelIndex + 4) & 1) * 4)) &
                                                            0xF)) /
                         kDxtAlphaNibbleMaximum);
                alphaOutput[0].alpha = t0;
                alphaOutput[1].alpha = t1;
                alphaOutput[2].alpha = t2;
                alphaOutput[3].alpha = t3;
                alphaOutput[4].alpha = t4;
                s8 t5 =
                    (s8)((kDxtAlphaByteMaximum * (f32)(u32)((explicitAlphaBlocks16->alpha[(alphaPixelIndex + 5) >> 1] >>
                                                             (((alphaPixelIndex + 5) & 1) * 4)) &
                                                            0xF)) /
                         kDxtAlphaNibbleMaximum);
                s8 t6 =
                    (s8)((kDxtAlphaByteMaximum * (f32)(u32)((explicitAlphaBlocks16->alpha[(alphaPixelIndex + 6) >> 1] >>
                                                             (((alphaPixelIndex + 6) & 1) * 4)) &
                                                            0xF)) /
                         kDxtAlphaNibbleMaximum);
                s8 t7 =
                    (s8)((kDxtAlphaByteMaximum * (f32)(u32)((explicitAlphaBlocks16->alpha[(alphaPixelIndex + 7) >> 1] >>
                                                             (((alphaPixelIndex + 7) & 1) * 4)) &
                                                            0xF)) /
                         kDxtAlphaNibbleMaximum);
                alphaOutput[5].alpha = t5;
                alphaOutput[6].alpha = t6;
                alphaOutput[7].alpha = t7;
                alphaOutput += 8;
            }
            break;
        case kDxt5:
        case kDxt5Alternate:
            gtl_DXT_DecodeColorBlock(decodedPixels, alphaColorBlocks16 + 8, 0);
            gtl_DXT_DecodeDXT5AlphaBlock(decodedPixels, alphaColorBlocks16);
            break;
        }

        blockRow = blockIndex / blocksPerRow;
        blockColumn = blockIndex - blockRow * blocksPerRow;
        decodedPixel = decodedPixels;
        for (blockPixelIndex = 0; blockPixelIndex < 16; blockPixelIndex++) {
            imageX = (blockPixelIndex & 3) + blockColumn * 4;
            imageY = (blockPixelIndex >> 2) + blockRow * 4;
            destinationByteOffset = (destinationBitsPerPixel * (imageX + imageY * width)) >> 3;
            if (destinationBitsPerPixel == 4 && format == kPackedAlpha4Destination) {
                dst[destinationByteOffset] =
                    dst[destinationByteOffset] |
                    (u8)((u8)(s32)(kDxtAlphaNibbleMaximum * ((f32)(u8)decodedPixel->alpha / kDxtAlphaByteMaximum))
                         << ((-(imageX & 1)) & 4));
            } else {
                gtl_Image_ConvertPixelFormat(dst + destinationByteOffset, decodedPixel, format, 1, 5, 0);
            }
            decodedPixel++;
        }
        colorBlocks8 += 8;
        alphaColorBlocks16 += 16;
        explicitAlphaBlocks16 += 1;
    }
    return 1;
}

// Per-pixel copy and conversion callbacks of the image converter (dst, src).
// Copy one four-byte pixel through Memory_Copy (same callback in three table slots).
void gtl_PixelCopy_Copy4Bytes_0(void* dst, const void* src) {
    Memory_Copy(dst, src, 4);
}
// Second four-byte pixel-copy entry; identical operation, distinct ABI symbol.
void gtl_PixelCopy_Copy4Bytes_1(void* dst, const void* src) {
    Memory_Copy(dst, src, 4);
}
// Copy one single-channel byte through Memory_Copy.
void gtl_PixelCopy_Copy1Byte(void* dst, const void* src) {
    Memory_Copy(dst, src, 1);
}
// Copy one 16-byte pixel payload, without interpreting components.
void gtl_PixelCopy_Copy16Bytes(void* dst, const void* src) {
    Memory_Copy(dst, src, 16);
}
// Third four-byte pixel-copy entry.
void gtl_PixelCopy_Copy4Bytes_2(void* dst, const void* src) {
    Memory_Copy(dst, src, 4);
}
// Copy one aligned native packed-color word; exact source/destination alias is safe.
void gtl_PixelCopy_CopyPackedColor(u32* dst, const u32* src) {
    *dst = *src;
}

// Convert four normalized RGBA floats to byte bits; alpha is stored first.
void gtl_PixelConvert_Float4ToRGBA8(PixelRGBA8* dst, const PixelFloat4* src) {
    dst->alpha = (s8)ClampByte<true>(src->alpha);
    dst->red = (s8)ClampByte<false>(src->red);
    dst->green = (s8)ClampByte<false>(src->green);
    dst->blue = (s8)ClampByte<false>(src->blue);
}

// Convert four bytes by exchanging red/blue; preserve green/alpha, disjoint buffers.
void gtl_PixelConvert_SwapRB32_0(u8* dst, const u8* src) {
    dst[0] = src[2];
    dst[1] = src[1];
    dst[2] = src[0];
    dst[3] = src[3];
}

// Same four-byte red/blue conversion, separate dispatch-table entry.
void gtl_PixelConvert_SwapRB32_1(u8* dst, const u8* src) {
    dst[0] = src[2];
    dst[1] = src[1];
    dst[2] = src[0];
    dst[3] = src[3];
}

// Unsupported/no-work dispatch entry: ignores both pointers, leaves dst untouched.
void gtl_PixelConvert_NoOp_0(u8* dst, const u8* src) {}

// declaration order (two bytes), unlike the explicit byte masks in Unpack4444.
struct Pixel565 {
    CW_BITS3(u16 r : 5, u16 g : 6, u16 b : 5)
};

struct Pixel4444 {
    CW_BITS2(u8 r : 4, u8 g : 4)
    CW_BITS2(u8 b : 4, u8 a : 4)
};

// Truncate each RGBA byte to its high nibble, write the native two-byte record.
void gtl_PixelConvert_RGBA8ToPacked4444(Pixel4444* dst, const u8* src) {
    dst->b = src[2] >> 4;
    dst->g = src[1] >> 4;
    dst->r = src[0] >> 4;
    dst->a = src[3] >> 4;
}

// Second no-work dispatch entry; both arguments unused, no output writes.
void gtl_PixelConvert_NoOp_1(u8* dst, const u8* src) {}

// Convert BGRX to RGBA, force opaque alpha (four-byte input/output).
void gtl_PixelConvert_BGRX8ToRGBA8(u8* dst, const u8* src) {
    dst[0] = src[2];
    dst[1] = src[1];
    dst[2] = src[0];
    dst[3] = 0xFF;
}

// Copy one channel byte; caller selects its role through the format table.
void gtl_PixelConvert_CopyByte0(u8* dst, const u8* src) {
    dst[0] = src[0];
}

// Copy four RGBA bytes in alpha/blue/green/red order; exact alias is safe.
void gtl_PixelConvert_CopyRGBA8(u8* dst, const u8* src) {
    dst[3] = src[3];
    dst[2] = src[2];
    dst[1] = src[1];
    dst[0] = src[0];
}

// Exchange red/blue in four bytes, alpha first; not safe for exact in-place use.
void gtl_PixelConvert_SwapRB32_2(u8* dst, const u8* src) {
    dst[3] = src[3];
    dst[2] = src[0];
    dst[1] = src[1];
    dst[0] = src[2];
}

// Rotate four RGBA bytes to ARGB; preserve this sequential write order.
void gtl_PixelConvert_RGBA8ToARGB8_0(u8* dst, const u8* src) {
    dst[0] = src[3];
    dst[3] = src[2];
    dst[2] = src[1];
    dst[1] = src[0];
}

// Rotate RGBX to ARGB and replace X with opaque alpha.
void gtl_PixelConvert_RGBX8ToARGB8_0(u8* dst, const u8* src) {
    dst[0] = 0xFF;
    dst[1] = src[0];
    dst[2] = src[1];
    dst[3] = src[2];
}

// Expand bytes [GB,AR] to RGBA by shifting nibbles left, without low-bit fill:
// outputs are multiples of 16 (maximum 240), deliberately not replicated to 255.
void gtl_PixelConvert_Unpack4444(u8* dst, const u8* src) {
    dst[0] = (src[1] & 0xF) << 4;
    dst[3] = src[1] & 0xF0;
    dst[2] = (src[0] & 0xF) << 4;
    dst[1] = src[0] & 0xF0;
}

// Same RGBX-to-ARGB mapping as entry0; distinct table slot and store order.
void gtl_PixelConvert_RGBX8ToARGB8_1(u8* dst, const u8* src) {
    dst[0] = 0xFF;
    dst[3] = src[2];
    dst[2] = src[1];
    dst[1] = src[0];
}

// Copy RGB, discard X and force opaque alpha (four bytes).
void gtl_PixelConvert_RGBX8ToRGBA8(u8* dst, const u8* src) {
    dst[3] = 0xFF;
    dst[2] = src[2];
    dst[1] = src[1];
    dst[0] = src[0];
}

// Copy exactly three RGB bytes; any fourth destination byte is untouched.
void gtl_PixelConvert_CopyRGB8(u8* dst, const u8* src) {
    dst[2] = src[2];
    dst[1] = src[1];
    dst[0] = src[0];
}

// Exchange red/blue in a three-byte RGB/BGR pixel; disjoint buffers required.
void gtl_PixelConvert_SwapRB24(u8* dst, const u8* src) {
    dst[0] = src[2];
    dst[1] = src[1];
    dst[2] = src[0];
}

// Four-byte red/blue exchange used by another conversion-table entry.
void gtl_PixelConvert_SwapRB32_3(u8* dst, const u8* src) {
    dst[0] = src[2];
    dst[1] = src[1];
    dst[2] = src[0];
    dst[3] = src[3];
}

// Rotate RGBA to ARGB, alpha first; four-byte disjoint input/output.
void gtl_PixelConvert_RGBA8ToARGB8_1(u8* dst, const u8* src) {
    dst[3] = src[2];
    dst[2] = src[1];
    dst[1] = src[0];
    dst[0] = src[3];
}

// Copy RGB into ARGB positions, then force opaque alpha.
void gtl_PixelConvert_RGBX8ToARGB8_2(u8* dst, const u8* src) {
    dst[3] = src[2];
    dst[2] = src[1];
    dst[1] = src[0];
    dst[0] = 0xFF;
}

// Expand native RGB565 to RGBA with float multiply/divide then truncation;
// alpha=255. Input is one aligned u16, output four bytes; no bit replication.
void gtl_PixelConvert_RGB565ToRGBA8(u8* dst, const u16* src) {
    u16 c = *src;
    u32 b = c & 0x1F;
    u32 g = (c >> 5) & 0x3F;
    u32 r = (c >> 11) & 0x1F;
    dst[2] = kByteChannelMaximum * (f32)b / kRgb565FiveBitMaximum;
    dst[1] = kByteChannelMaximum * (f32)g / kRgb565SixBitMaximum;
    dst[0] = kByteChannelMaximum * (f32)r / kRgb565FiveBitMaximum;
    dst[3] = 0xFF;
}

// Extract alpha from a four-byte RGBA pixel to one destination byte.
void gtl_PixelConvert_CopyAlphaByte(u8* dst, const u8* src) {
    dst[0] = src[3];
}

} // extern "C"
