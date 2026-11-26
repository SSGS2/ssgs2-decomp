// Texture and image memory layout: reorders pixel bytes between linear, Morton and tiled order.
// It also sizes images: bytes and bits per element, row pitch and mip chain size for each format.

#include "sdk_protos.h"

// Texture/image memory layout and byte sizing; no gameplay tuning.
// API index:
// - CopyLinearRows (legacy ABI name): bidirectional Morton/linear conversion
//   for ordinary formats; compressed ranges instead copy contiguous bytes.
// - TiledToLinear: hardware tile addressing, with a split-plane four-byte pixel
//   path; this is DIFFERENT from the ordinary Morton conversion above.
// - GetBytesPerElement/GetBitsPerPixel: format encodings -> storage quantities.
//   Format IDs are engine IDs, not GX IDs. Unknown element formats return zero.
// - GetRowPitch/GetMipChainSize: use the shared sizing policies below; return
//   BYTES, never pixels. Mip count zero requests GetMipLevelCount's default.
// Callers own every buffer. Converters allocate nothing and do no bounds checks;
// src/dst must be nonnull, disjoint, and span every computed source/dest offset.
// Integer products and offsets must fit u32; dimensions/counts are not validated.
// There is no error result or partial-write rollback in the conversion APIs.

extern "C" {

void gtl_Image_ReorderCompressedBlocks(unsigned char* src, unsigned char* dst, unsigned int w, unsigned int h, int fmt,
                                       int flags);
unsigned int gtl_Image_GetTiledOffset(unsigned int x, unsigned int y, unsigned int w, unsigned int bpe);
unsigned int gtl_ImageFormat_GetBytesPerElement(unsigned int fmt);
void gtl_Image_Untile8BitPixels(unsigned char* src, unsigned char* dst, unsigned int w, unsigned int h);
unsigned int gtl_Image_GetMipLevelCount(unsigned int w, unsigned int h, int fmt, int mode);

#pragma use_lmw_stmw on
#pragma opt_propagation off
// For each pixel, interleave X/Y bits into a Morton element index. reverse!=0
// copies linear src -> Morton dst; zero copies Morton src -> linear dst.
// A square power-of-two image with known nonzero bytes/element is sufficient for
// dense w*h*bpe storage. Other shapes require storage covering all computed
// Morton offsets; do not assume row-major size alone bounds those accesses.
static inline void CopyMortonRows(unsigned char* src, unsigned char* dst, unsigned int w, unsigned int h, int fmt,
                                  unsigned char reverse) {
    unsigned int x;
    unsigned int y;
    unsigned int bpe;
    unsigned int half;
    unsigned int rowBase;
    unsigned int dstOff, srcOff;
    unsigned int rowOff;
    unsigned int blk;
    unsigned int ux;
    unsigned int uy;
    unsigned int acc;
    unsigned int sq;
    unsigned int qx;
    unsigned int qy;
    bpe = gtl_ImageFormat_GetBytesPerElement(fmt);
    half = w >> 1;
    y = 0;
    rowBase = 0;
    for (; y < h; y++) {
        for (x = 0; x < w; x++) {
            rowOff = bpe * (x + rowBase);
            blk = half;
            ux = x;
            uy = y;
            acc = 0;
            while (blk != 0) {
                qx = ux / blk;
                sq = blk * blk;
                qy = uy / blk;
                acc = (qy * sq * 2) + (acc + (qx * sq));
                ux -= qx * blk;
                uy -= qy * blk;
                blk >>= 1;
            }
            dstOff = acc * bpe;
            if (reverse) {
                srcOff = rowOff;
            } else {
                srcOff = dstOff;
                dstOff = rowOff;
            }
            Memory_Copy(dst + dstOff, src + srcOff, bpe);
        }
        rowBase += w;
    }
}

#pragma use_lmw_stmw reset
#pragma opt_propagation reset

struct Px {
    unsigned char b0, b1, b2, b3;
};
#pragma use_lmw_stmw on
#pragma opt_propagation off

#pragma use_lmw_stmw reset
#pragma opt_common_subs reset

// Round a byte quantity up to a positive alignment; unused by the current paths.
static inline void mip_align(unsigned int& v, int a) {
    int r = (int)v % a;
    if (r != 0) {
        v += a - r;
    } else {
        return;
    }
}

#pragma use_lmw_stmw on

#pragma use_lmw_stmw reset

#pragma use_lmw_stmw on
#pragma opt_propagation off

#pragma use_lmw_stmw reset
#pragma opt_propagation reset

void gtl_Image_ConvertMortonLayout(unsigned char* src, int unused1, int unused2, unsigned char* dst, unsigned int w,
                                   unsigned int h, int unused6, int fmt, unsigned char reverse);
void gtl_Image_Untile8BitPixels(unsigned char* src, unsigned char* dst, unsigned int w, unsigned int h);
void gtl_Image_TiledToLinear(unsigned char* src, int unused1, int unused2, unsigned char* dst, unsigned int w,
                             unsigned int h, int unused6, int fmt);
unsigned int gtl_ImageFormat_GetBytesPerElement(unsigned int fmt);
unsigned int gtl_ImageFormat_GetBitsPerPixel(int fmt);
unsigned int gtl_Image_GetRowPitch(unsigned int w, int fmt, unsigned int mode);
unsigned int gtl_Image_GetMipChainSize(unsigned int w, unsigned int h, int levels, int fmt, int mode);

#pragma push

#pragma opt_propagation off
#pragma use_lmw_stmw on
// Convert ordinary image layout in either direction (see CopyMortonRows), not
// merely a row memcpy. unused1/2/6 are ignored ABI arguments. reverse is ignored
// for formats [0x2E,0x32] and [0x62,0x66]: those copy (w*h*bits)>>3 bytes unchanged,
// with 4 bits for 0x2E/0x62 and 8 for the rest. Preserve floor-to-byte behavior.
// Required buffer sizes and aliasing follow the file contract; no return value.
void gtl_Image_ConvertMortonLayout(unsigned char* src, int unused1, int unused2, unsigned char* dst, unsigned int w,
                                   unsigned int h, int unused6, int fmt, unsigned char reverse) {
    int flag;
    unsigned int area;
    int bits;

    if (fmt >= 0x62) {
        goto hi;
    }
    if (fmt >= 0x33) {
        goto zero;
    }
    if (fmt < 0x2E) {
        goto zero;
    }
    goto one;
hi:
    if (fmt >= 0x67) {
        goto zero;
    }
one:
    flag = 1;
    goto done;
zero:
    flag = 0;
done:
    if (flag) {
        switch (fmt) {
        case 0x1F:
        case 0x21:
        case 0x59:
        case 0x5B:
            bits = 4;
            break;
        case 0x2E:
        case 0x62:
            bits = 4;
            break;
        case 0x2F:
        case 0x30:
        case 0x31:
        case 0x32:
        case 0x63:
        case 0x64:
        case 0x65:
        case 0x66:
            bits = 8;
            break;
        default:
            bits = gtl_ImageFormat_GetBytesPerElement(fmt) * 8;
            break;
        case -1:
            bits = 0;
            break;
        }
        area = h * w;
        Memory_Copy(dst, src, (area * bits) >> 3);
        return;
    }
    CopyMortonRows(src, dst, w, h, fmt, reverse);
}

#pragma push
#pragma dont_inline on
// Copy one byte per pixel from GetTiledOffset addressing to row-major dst[w*h].
// Source storage includes tile padding required by that address generator.
void gtl_Image_Untile8BitPixels(unsigned char* src, unsigned char* dst, unsigned int w, unsigned int h) {
    unsigned int rowOff;
    unsigned int y;
    unsigned int x;

    y = 0;
    rowOff = 0;
    for (; y < h; y++) {
        x = 0;
        for (; x < w; x++) {
            Memory_Copy(dst + (x + rowOff), src + gtl_Image_GetTiledOffset(x, y, w, 1), 1);
        }
        rowOff += w;
    }
}

#pragma pop

// Decode tiled texture storage. unused1/2/6 are ignored. Compressed ranges
// [0x2E,0x32]/[0x62,0x66] delegate to ReorderCompressedBlocks(flags=0).
// Ordinary formats require bpe==1 or the FOUR-byte split-plane representation:
// tiled bytes {off,off+1,off+0x20,off+0x21} become dst channels {3,0,1,2}.
// Every other bpe still takes that four-byte path; it is not a general 2/3/8-byte
// converter. Caller must select a compatible format and provide tile padding.
void gtl_Image_TiledToLinear(unsigned char* src, int unused1, int unused2, unsigned char* dst, unsigned int w,
                             unsigned int h, int unused6, int fmt) {
    int flag;
    unsigned int rowBase;
    unsigned int bpe;
    unsigned int y;
    unsigned int x;
    unsigned int off;
    unsigned char t0;
    unsigned char t3;
    unsigned char t4;
    unsigned char t5;
    unsigned int o;
    unsigned char* p;
    unsigned char* d;
    unsigned char* s0;

    if (fmt >= 0x62) {
        goto hi;
    }
    if (fmt >= 0x33) {
        goto zero;
    }
    if (fmt < 0x2E) {
        goto zero;
    }
    goto one;
hi:
    if (fmt >= 0x67) {
        goto zero;
    }
one:
    flag = 1;
    goto done;
zero:
    flag = 0;
done:
    if (flag) {
        gtl_Image_ReorderCompressedBlocks(src, dst, w, h, fmt, 0);
        return;
    }
    bpe = gtl_ImageFormat_GetBytesPerElement(fmt);
    if (bpe == 1) {
        gtl_Image_Untile8BitPixels(src, dst, w, h);
        return;
    }
    s0 = src;
    y = 0;
    rowBase = 0;
    for (; y < h; y++) {
        for (x = 0; x < w; x++) {
            off = gtl_Image_GetTiledOffset(x, y, w, bpe);
            p = src + off;
            o = bpe * (x + rowBase);
            t5 = s0[off];
            t4 = p[1];
            t3 = p[0x20];
            t0 = p[0x21];
            d = dst + o;
            d[3] = t5;
            d[0] = t4;
            d[1] = t3;
            d[2] = t0;
        }
        rowBase += w;
    }
}

#pragma opt_propagation reset
#pragma use_lmw_stmw reset
// Return format storage bytes per addressable element; unsupported IDs -> 0.
// This is not enough to size sub-byte/compressed formats: use GetBitsPerPixel.
unsigned int gtl_ImageFormat_GetBytesPerElement(unsigned int fmt) {
    switch (fmt) {
    case 1:
    case 63:
        return 4;
    case 8:
        return 2;
    case 30:
    case 88:
        return 1;
    case 31:
    case 89:
        return 1;
    case 37:
    case 95:
        return 1;
    case 34:
    case 92:
        return 1;
    case 28:
    case 72:
        return 1;
    case 36:
        return 2;
    case 14:
    case 76:
        return 4;
    case 15:
    case 77:
        return 4;
    case 21:
    case 83:
        return 2;
    case 20:
        return 2;
    case 5:
        return 2;
    case 80:
        return 2;
    case 82:
        return 2;
    case 55:
    case 107:
        return 2;
    case 52:
    case 104:
        return 4;
    case 2:
    case 64:
        return 4;
    case 0:
    case 62:
        return 3;
    case 13:
    case 75:
        return 3;
    case 70:
        return 2;
    case 56:
    case 108:
        return 4;
    case 16:
    case 78:
        return 4;
    case 17:
    case 79:
        return 4;
    case 54:
    case 106:
        return 4;
    case 44:
    case 96:
        return 2;
    case 25:
        return 8;
    default:
        return 0;
    }
}

// Return 4 for 0x1F/0x21/0x59/0x5B/0x2E/0x62; 8 for 0x2F..0x32 and
// 0x63..0x66; -1 -> 0. Other IDs use GetBytesPerElement*8, possibly zero.
// The branch-shaped encoding is shared by the sizing helper below.
unsigned int gtl_ImageFormat_GetBitsPerPixel(int fmt) {
    if (fmt >= 0x33) {
        goto hi;
    }
    if (fmt == 0x20) {
        goto call;
    }
    if (fmt >= 0x20) {
        goto mid;
    }
    if (fmt == -1) {
        goto zero;
    }
    if (fmt < -1) {
        goto call;
    }
    if (fmt < 0x1F) {
        goto call;
    }
    goto four_a;
mid:
    if (fmt == 0x2E) {
        goto four_b;
    }
    if (fmt >= 0x2E) {
        goto eight_a;
    }
    if (fmt < 0x22) {
        goto four_a;
    }
    goto call;
hi:
    if (fmt == 0x5B) {
        goto four_a;
    }
    if (fmt >= 0x5B) {
        goto hi2;
    }
    if (fmt != 0x59) {
        goto call;
    }
    goto four_a;
hi2:
    if (fmt == 0x62) {
        goto four_b;
    }
    if (fmt < 0x62) {
        goto call;
    }
    if (fmt < 0x67) {
        goto eight_a;
    }
    goto call;
four_a:
    return 4;
four_b:
    return 4;
eight_a:
    return 8;
call:
    return gtl_ImageFormat_GetBytesPerElement(fmt) * 8;
zero:
    return 0;
}

// Expand the format classifier in sizing paths without a runtime API call.
// This duplicates GetBitsPerPixel's format decision order; it is inlined because
// the checked sizing functions originally contained those exact decisions.
// Required bits output, written on all paths; format table is directly above.
// A return-value helper changed compiler data flow and failed equivalence proof;
// the reference output preserves it. Labels keep this format decision tree's
// branch order; callers no longer duplicate six copies of the same tree.
#pragma push
#pragma always_inline on
#pragma opt_propagation off
static inline void GetSizingBitsPerPixel(int fmt, int& bits) {
    if (fmt >= 0x33) {
        goto hi;
    }
    if (fmt == 0x20) {
        goto call;
    }
    if (fmt >= 0x20) {
        goto mid;
    }
    if (fmt == -1) {
        goto zero;
    }
    if (fmt < -1) {
        goto call;
    }
    if (fmt < 0x1F) {
        goto call;
    }
    goto four_a;
mid:
    if (fmt == 0x2E) {
        goto four_b;
    }
    if (fmt >= 0x2E) {
        goto eight_a;
    }
    if (fmt < 0x22) {
        goto four_a;
    }
    goto call;
hi:
    if (fmt == 0x5B) {
        goto four_a;
    }
    if (fmt >= 0x5B) {
        goto hi2;
    }
    if (fmt != 0x59) {
        goto call;
    }
    goto four_a;
hi2:
    if (fmt == 0x62) {
        goto four_b;
    }
    if (fmt < 0x62) {
        goto call;
    }
    if (fmt < 0x67) {
        goto eight_a;
    }
    goto call;
four_a:
    bits = 4;
    goto finishFormatBits;
four_b:
    bits = 4;
    goto finishFormatBits;
eight_a:
    bits = 8;
    goto finishFormatBits;
call:
    bits = gtl_ImageFormat_GetBytesPerElement(fmt) * 8;
    goto finishFormatBits;
zero:
    bits = 0;
finishFormatBits:
    return;
}

#pragma pop

// Mode 3 keeps the original width through mip levels for these format ranges:
// [0x3E,0x40], [0x43,0x4E], [0x50,0x59], [0x62,0x66]. Other formats
// shrink normally and round row width to a power of two. Output is a policy bit,
// not a format validity test. Keep reference output to preserve inlined data flow.
#pragma push
#pragma always_inline on
#pragma opt_propagation off
static inline void GetMode3PowerOfTwoPolicy(int fmt, int& usePowerOfTwo) {
    if (fmt == 0x4F) {
        goto powerOfTwo;
    }
    if (fmt >= 0x4F) {
        goto formatsAtOrAbove4F;
    }
    if (fmt >= 0x41) {
        goto formats41Through4E;
    }
    if (fmt < 0x3E) {
        goto powerOfTwo;
    }
    goto fixedWidth;
formats41Through4E:
    if (fmt < 0x43) {
        goto powerOfTwo;
    }
    goto fixedWidth;
formatsAtOrAbove4F:
    if (fmt >= 0x62) {
        goto formatsAtOrAbove62;
    }
    if (fmt < 0x5A) {
        goto fixedWidth;
    }
    goto powerOfTwo;
formatsAtOrAbove62:
    if (fmt >= 0x67) {
        goto powerOfTwo;
    }
fixedWidth:
    usePowerOfTwo = 0;
    goto finishWidthPolicy;
powerOfTwo:
    usePowerOfTwo = 1;
finishWidthPolicy:
    return;
}
#pragma pop

// Row-width policy map, shared with mip sizing (all alignments are in PIXELS):
//   mode 0/2/9/10/unknown: w unchanged
//   mode 1: ceil multiple of 4
//   mode 3: fixed w for GetMode3PowerOfTwoPolicy's listed ranges; otherwise pow2
//   mode 4/8: next pow2 (at least 1), then align to 128/bits pixels (16 bytes)
//   mode 5/6: next pow2 (at least 1)
//   mode 7: align to tile width 8 for 4/8 bpp, 4 for 16/32 bpp
// Then convert (alignedWidth*bits)>>3; sub-byte rows TRUNCATE, never ceil bytes.
// mode 4/8 requires nonzero bits with 128/bits nonzero; mode 7 requires bpp in
// {4,8,16,32}, otherwise modulo uses zero. Widths/rounding/products must fit both
// u32 and the signed-int remainder intermediates; pow2 growth must not overflow.
// These are storage-layout mode IDs; platform names are not established here.
// The named phase labels preserve compiler branch layout/jump-table metadata;
// structured switch rewrites changed allocated jump-table data and were rejected.
#pragma opt_propagation off
// Return bytes in one row; pure query, no ownership or output parameters.
unsigned int gtl_Image_GetRowPitch(unsigned int w, int fmt, unsigned int mode) {
    unsigned int rowWidthPixels;
    unsigned int powerOfTwoWidth;
    int bits;
    unsigned int pixelsPer16Bytes;
    unsigned int tileWidthPixels;
    unsigned int roundedWidthOrLevelBytes;
    int alignmentRemainder;
    int usePowerOfTwoWidth;

    rowWidthPixels = w;
    switch (mode) {
    case 0:
    case 2:
    case 9:
    case 10:
    default:
        goto rowBytesFromPixelWidth;
    case 1:
        goto fourPixelRow;
    case 3:
        goto formatDependentPowerOfTwoRow;
    case 4:
    case 8:
        goto powerOfTwoThen16ByteRow;
    case 5:
    case 6:
        goto powerOfTwoRow;
    case 7:
        goto hardwareTileWidthRow;
    }

powerOfTwoThen16ByteRow:
    GetSizingBitsPerPixel(fmt, bits);
    powerOfTwoWidth = 1;
grow16ByteRowPowerOfTwo:
    if (rowWidthPixels <= powerOfTwoWidth) {
        goto align16ByteRow;
    }
    powerOfTwoWidth <<= 1;
    goto grow16ByteRowPowerOfTwo;
align16ByteRow:
    pixelsPer16Bytes = 0x80U / bits;
    alignmentRemainder = (int)powerOfTwoWidth % (int)pixelsPer16Bytes;
    rowWidthPixels = powerOfTwoWidth + (pixelsPer16Bytes - alignmentRemainder);
    if (alignmentRemainder != 0) {
        goto rowBytesFromPixelWidth;
    }
    rowWidthPixels = powerOfTwoWidth;
    goto rowBytesFromPixelWidth;

formatDependentPowerOfTwoRow:
    GetMode3PowerOfTwoPolicy(fmt, usePowerOfTwoWidth);
    if (usePowerOfTwoWidth == 0) {
        goto rowBytesFromPixelWidth;
    }
    rowWidthPixels = 1;
growMode3PowerOfTwo:
    if (w <= rowWidthPixels) {
        goto rowBytesFromPixelWidth;
    }
    rowWidthPixels <<= 1;
    goto growMode3PowerOfTwo;
    goto rowBytesFromPixelWidth;

hardwareTileWidthRow:
    GetSizingBitsPerPixel(fmt, bits);
    tileWidthPixels = 0;
    if (bits == 0x10) {
        goto fourPixelTile;
    }
    if (bits >= 0x10) {
        goto test32BitTile;
    }
    if (bits == 8) {
        goto eightPixelTile;
    }
    if (bits >= 8) {
        goto alignTileWidth;
    }
    if (bits != 4) {
        goto alignTileWidth;
    }
    goto eightPixelTile;
test32BitTile:
    if (bits != 0x20) {
        goto alignTileWidth;
    }
    goto fourPixelTile;
eightPixelTile:
    tileWidthPixels = 8;
    goto alignTileWidth;
fourPixelTile:
    tileWidthPixels = 4;
alignTileWidth:
    alignmentRemainder = (int)rowWidthPixels % (int)tileWidthPixels;
    roundedWidthOrLevelBytes = rowWidthPixels + (tileWidthPixels - alignmentRemainder);
    if (alignmentRemainder == 0) {
        roundedWidthOrLevelBytes = rowWidthPixels;
    }
    rowWidthPixels = roundedWidthOrLevelBytes;
    goto rowBytesFromPixelWidth;

powerOfTwoRow:
    rowWidthPixels = 1;
growPowerOfTwoWidth:
    if (w <= rowWidthPixels) {
        goto rowBytesFromPixelWidth;
    }
    rowWidthPixels <<= 1;
    goto growPowerOfTwoWidth;
finishPowerOfTwoWidth:
    goto rowBytesFromPixelWidth;

fourPixelRow:
    alignmentRemainder = (int)w % 4;
    if (alignmentRemainder != 0) {
        goto alignFourPixelWidth;
    }
    goto rowBytesFromPixelWidth;
alignFourPixelWidth:
    rowWidthPixels = w + (4 - alignmentRemainder);
    goto rowBytesFromPixelWidth;

rowBytesFromPixelWidth:
    GetSizingBitsPerPixel(fmt, bits);
    return (rowWidthPixels * bits) >> 3;
}

#pragma use_lmw_stmw on
// Return required mip-chain bytes; pure query, no allocation or buffer writes.
// levels counts base level too: 0 delegates to GetMipLevelCount, explicit values
// must be 1..32 so shifts remain defined. Require positive starting dimensions
// and the same valid format/mode/overflow domains as GetRowPitch above.
// Each level uses max(1,w>>levelIndex) by max(1,h>>levelIndex). Fixed-width mode-3 formats
// KEEP w at every level while height shrinks. All modes apply the row-width
// policy, multiply row bytes by height, and add. Mode 7 rounds the RUNNING TOTAL
// to 32 bytes after EACH level; it does not round height up to a tile height.
// A caller placing mip payloads must preserve that level order and padding and
// allocate at least this returned extent. Hardware tile converters may need
// additional source padding; this sizing API is not an arbitrary-shape validator.
unsigned int gtl_Image_GetMipChainSize(unsigned int w, unsigned int h, int levels, int fmt, int mode) {
    unsigned int levelCount;
    unsigned int totalBytes;
    unsigned int levelIndex;
    unsigned int levelWidthPixels;
    unsigned int levelHeightPixels;
    int bits;
    unsigned int pixelsPer16Bytes;
    unsigned int powerOfTwoWidth;
    unsigned int tileWidthPixels;
    // Reused scratch: rounded PIXEL width in mode 7, then level BYTES at
    // addLevelBytes. No pixel-to-byte value survives that phase boundary.
    unsigned int roundedWidthOrLevelBytes;
    int usePowerOfTwoWidth;
    int alignmentRemainder;

    if (levels == 0) {
        levelCount = gtl_Image_GetMipLevelCount(w, h, fmt, mode);
    } else {
        levelCount = levels;
    }
    totalBytes = 0;
    levelIndex = 0;
    goto hasMoreMipLevels;
sizeNextMipLevel:
    if (mode != 3) {
        goto shrinkMipWidth;
    }
    GetMode3PowerOfTwoPolicy(fmt, usePowerOfTwoWidth);
    if (usePowerOfTwoWidth != 0) {
        goto shrinkMipWidth;
    }
    levelWidthPixels = w;
    goto clampMipWidth;
shrinkMipWidth:
    levelWidthPixels = w >> levelIndex;
clampMipWidth:
    if (levelWidthPixels == 0) {
        levelWidthPixels = 1;
    }
    levelHeightPixels = h >> levelIndex;
    if (levelHeightPixels == 0) {
        levelHeightPixels = 1;
    }
    switch (mode) {
    case 0:
    case 2:
    case 9:
    case 10:
    default:
        goto addLevelBytes;
    case 1:
        goto fourPixelRow;
    case 3:
        goto formatDependentPowerOfTwoRow;
    case 4:
    case 8:
        goto powerOfTwoThen16ByteRow;
    case 5:
    case 6:
        goto powerOfTwoRow;
    case 7:
        goto hardwareTileWidthRow;
    }

powerOfTwoThen16ByteRow:
    GetSizingBitsPerPixel(fmt, bits);
    powerOfTwoWidth = 1;
grow16ByteRowPowerOfTwo:
    if (levelWidthPixels <= powerOfTwoWidth) {
        goto align16ByteRow;
    }
    powerOfTwoWidth <<= 1;
    goto grow16ByteRowPowerOfTwo;
align16ByteRow:
    pixelsPer16Bytes = 0x80U / bits;
    alignmentRemainder = (int)powerOfTwoWidth % (int)pixelsPer16Bytes;
    levelWidthPixels = powerOfTwoWidth + (pixelsPer16Bytes - alignmentRemainder);
    if (alignmentRemainder != 0) {
        goto addLevelBytes;
    }
    levelWidthPixels = powerOfTwoWidth;
    goto addLevelBytes;

formatDependentPowerOfTwoRow:
    GetMode3PowerOfTwoPolicy(fmt, usePowerOfTwoWidth);
    if (usePowerOfTwoWidth == 0) {
        goto addLevelBytes;
    }
    powerOfTwoWidth = 1;
growMode3PowerOfTwo:
    if (levelWidthPixels <= powerOfTwoWidth) {
        goto commitMode3Width;
    }
    powerOfTwoWidth <<= 1;
    goto growMode3PowerOfTwo;
commitMode3Width:
    levelWidthPixels = powerOfTwoWidth;
    goto addLevelBytes;

hardwareTileWidthRow:
    GetSizingBitsPerPixel(fmt, bits);
    tileWidthPixels = 0;
    if (bits == 0x10) {
        goto fourPixelTile;
    }
    if (bits >= 0x10) {
        goto test32BitTile;
    }
    if (bits == 8) {
        goto eightPixelTile;
    }
    if (bits >= 8) {
        goto alignTileWidth;
    }
    if (bits != 4) {
        goto alignTileWidth;
    }
    goto eightPixelTile;
test32BitTile:
    if (bits != 0x20) {
        goto alignTileWidth;
    }
    goto fourPixelTile;
eightPixelTile:
    tileWidthPixels = 8;
    goto alignTileWidth;
fourPixelTile:
    tileWidthPixels = 4;
alignTileWidth:
    alignmentRemainder = (int)levelWidthPixels % (int)tileWidthPixels;
    roundedWidthOrLevelBytes = levelWidthPixels + (tileWidthPixels - alignmentRemainder);
    if (alignmentRemainder == 0) {
        roundedWidthOrLevelBytes = levelWidthPixels;
    }
    levelWidthPixels = roundedWidthOrLevelBytes;
    goto addLevelBytes;

powerOfTwoRow:
    powerOfTwoWidth = 1;
growPowerOfTwoWidth:
    if (levelWidthPixels <= powerOfTwoWidth) {
        goto commitPowerOfTwoWidth;
    }
    powerOfTwoWidth <<= 1;
    goto growPowerOfTwoWidth;
commitPowerOfTwoWidth:
    levelWidthPixels = powerOfTwoWidth;
    goto addLevelBytes;

fourPixelRow:
    alignmentRemainder = (int)levelWidthPixels % 4;
    if (alignmentRemainder == 0) {
        goto addLevelBytes;
    }
    goto alignFourPixelWidth;
alignFourPixelWidth:
    levelWidthPixels += 4 - alignmentRemainder;

addLevelBytes:
    GetSizingBitsPerPixel(fmt, bits);
    roundedWidthOrLevelBytes = (levelWidthPixels * bits) >> 3;
    roundedWidthOrLevelBytes *= levelHeightPixels;
    totalBytes += roundedWidthOrLevelBytes;
    switch (mode) {
    case 7:
        alignmentRemainder = (int)totalBytes % 32;
        if (alignmentRemainder == 0) {
            break;
        } else {
            totalBytes += 32 - alignmentRemainder;
            break;
        }
    }
advanceMipLevel:
    levelIndex++;
hasMoreMipLevels:
    if (levelIndex < levelCount) {
        goto sizeNextMipLevel;
    }
    return totalBytes;
}

#pragma pop

} // extern "C"
