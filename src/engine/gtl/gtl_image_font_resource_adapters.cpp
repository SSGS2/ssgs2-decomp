// Translation unit 0x803D6468-0x803D7914 (18 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".
// Image/font resource adapters and chunk serialization. Mod lookup: TGA/DDS
// format mapping -> GetDescription; glyph search -> FindGlyphRecord and global
// CompareGlyphCode mode; resource packing/alignment -> RebuildChunks.
// Format records below are ABI views, not portable disk parsers. TGA width and
// height are byte-reversed explicitly. DDS words are already in native order
// here (size must read 0x7C); feeding unconverted little-endian DDS is invalid.
// These routines receive no buffer lengths and do not validate full payloads.

#include "types.h"
#include "MSL/internal/mem.h"
#include "game/stdexcept_local.h"

#pragma use_lmw_stmw on

typedef struct TgaDescOut {
    s32 format; // Native engine format ID; -1 = unsupported (status still 1).
    s32 widthPixels; // Width pixels.
    s32 heightPixels; // Height pixels.
    s32 mipLevelCount; // Mip level count (TGA always 1).
} TgaDescOut;

typedef struct TgaDescPalette {
    s32 paletteFormat; // Native palette format ID, -1 when unsupported/absent.
    s32 entryCountRaw; // Raw palette entry-count word, NOT swapped by this adapter.
} TgaDescPalette;

#pragma pack(push, 1)
typedef struct TgaHeader {
    unsigned char unk0[5]; // +0 ID length, +1 color-map type, +2 image type,
                          // +3..4 color-map origin (unused).
    unsigned short colorMapLengthRaw; // +5 color-map length; consumed without reversal here.
    unsigned char colorMapEntryBits; // +7 color-map entry bits (24/32 supported).
    unsigned char unk8[4]; // +8/+A image origin (unused).
    unsigned short widthLE; // +C width, little endian; explicitly reversed on Wii.
    unsigned short heightLE; // +E height, little endian; explicitly reversed on Wii.
    unsigned char pixelBits; // +10 pixel depth; +11 descriptor omitted from this view.
} TgaHeader;
#pragma pack(pop)

typedef struct GlyphRecord {
    u16 atlasU; // +0 atlas U in texels (BitmapFont_FormatTextVertices: u0).
    u16 atlasV; // +2 atlas V in texels (v0).
    u16 texturePageIndex; // +4 texture page index; bitmap renderer requires <20.
    s16 horizontalBearing; // +6 horizontal bearing, font units; added before quad/pen advance.
    s16 width; // +8 glyph width: atlas texels and unscaled quad width.
    s16 extraAdvance; // +A extra horizontal advance, font units; NOT total pen advance.
} GlyphRecord;
// Native 12-byte record, shared with Glyph in gtl_bitmap_font_draw.cpp. That consumer uses
// header.lineHeight for height, not a seventh per-glyph metric. Its pen equation:
// x0 = penX + bearing*scaleX + globalBearingAdjustment;
// nextPenX = x0 + scaleX*(width + extraAdvance). Keep these effects together
// when changing spacing: bearing also changes the next character's pen position.
typedef char GlyphRecord_size[(sizeof(GlyphRecord) == 12) ? 1 : -1];

typedef struct GlyphEntry {
    u16 code;
    GlyphRecord rec;
} GlyphEntry;

typedef struct GlyphFontHeader {
    unsigned char unk0[0x14];
    s32 lookupMode; // Lookup representation: 0 direct u16 index map, 1 sorted entries.
    unsigned char unk18[8];
    s32 directMapMaxCode; // Direct map maximum accepted code / payload-base multiplier.
    s32 sortedEntryCount; // Sorted GlyphEntry count.
} GlyphFontHeader;

typedef struct GlyphFont {
    unsigned char unk0[4];
    GlyphFontHeader* header;
    unsigned char unk8[4];
    u16* codeTable;
} GlyphFont;

typedef struct ResourceChunk {
    u32 payloadAddressOrKey; // Input: borrowed payload address. Output: logical key (input offset).
    u32 elementCount; // +4 COUNT of elements, copied unchanged to directory +4.
    u32 elementStrideBytes; // +8 BYTES per element; payload bytes = +4 * +8.
    u32 offset; // Input: sort/logical key. Output: aligned byte offset in new buffer.
} ResourceChunk;

typedef struct ResourceChunkBuffer {
    ResourceChunk* base;
    s32 count;
    s32 capacity;
    // Release temporary descriptor storage, never borrowed input payloads.
    ~ResourceChunkBuffer() {
        if (base != 0) {
            char al = 0;
            count = count - count;
            operator delete(base);
        }
    }
} ResourceChunkBuffer;

typedef struct FontHeaderData {
    u8 tag[4];
    s32 unk4;
    s32 unk8;
    s32 recordCount; // Number of 32-byte FontDataRecord entries after 0x28-byte header.
    s32 unk10;
    s32 unk14;
    s32 comparatorModeFlagsA; // OR with +1C installs the global glyph comparator mode.
    s32 comparatorModeFlagsB;
    s32 unk20;
    s32 unk24;
} FontHeaderData;

struct FontDataObject {
    s32 refCount;
};

struct FontDataHeader {
    char unk0[0xC];
    s32 recordCount; // Record count; InitView exposes count*32 bytes starting at header+28.
    char unk10[8];
    s32 comparatorModeFlagsA; // Comparator mode = this word OR +1C; native values, no text decoding.
    s32 comparatorModeFlagsB;
};

struct FontDataView {
    FontDataObject* obj;
    FontDataHeader* hdr;
    u8* glyphs;
    u8* glyphsEnd;
};

struct D6894RefCountedBase { s32 refCount; };

struct D6894RefCounted : D6894RefCountedBase { 
virtual ~D6894RefCounted();
};

struct D6894Holder { D6894RefCounted* obj; };

typedef struct DdsPixelFormatEntry {
    unsigned char pixelFormat[0x20];
    s32 format;
} DdsPixelFormatEntry;

typedef struct DdsPixelFormatTable {
    DdsPixelFormatEntry entries[10];
} DdsPixelFormatTable;

typedef struct DdsHeader {
    u32 size;
    u32 flags;
    u32 height;
    u32 width;
    u32 pitch;
    u32 depth;
    u32 mipCount;
    unsigned char unk1C[0x2C];
    unsigned char pixelFormat[0x20];
} DdsHeader;

struct GenericResourceAllocTable {
    void* (*alloc)(u32 size);
    void (*free)(void* p, GenericResourceAllocTable* self);
};

typedef struct FontDataRecord {
    u8 bytes[24];
    s32 unk18;
    s32 unk1C;
} FontDataRecord;

extern "C" {

extern char s_lengthErrorInfo[];
void* Object_GetField8(void* obj);
void* Object_GetFieldC(void* obj);
void gtl_IG3DXBuffer_Create(s32 size, void* owner, s32 flags);
ResourceChunk* gtl_ResourceFile_GetChunkByIndex(void* file, void* chunks, s32 index);

void Memory_CopyAndReverseStridedBlocks(void* destination, void* source, unsigned int packedLayout, unsigned int rowStride, unsigned int rowCount);
// The callee reads the engine format as its eighth argument (r10), which the original six-argument call leaves
// live.
void gtl_Image_TiledToLinear(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5);
void gtl_Image_ConvertMortonLayout(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6, s32 arg7, s32 arg8);
s32 gtl_ImageFormat_GetBytesPerElement(s32 arg0);
s32 gtl_FontData_CompareGlyphCode(const void* a, const void* b);
void* bsearch(const void* key, const void* base, size_t nmemb, size_t size, s32 (*compar)(const void*, const void*));

s32 gtl_Image_GetMipChainSize(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4);


s32 gtl_FontData_FindGlyphRecord(const GlyphFont* arg0, s32 arg1, GlyphRecord* arg2);
extern s32 s_glyphComparatorMode;


#pragma opt_propagation off
#pragma opt_common_subs off

#pragma opt_propagation reset
#pragma opt_common_subs reset

extern s32 s_sortPivotSequence;
void gtl_ResourceChunks_SortThreeByOffset(ResourceChunk** a, ResourceChunk** b, ResourceChunk** c, s32 ctx);
void gtl_ResourceChunks_SortByOffsetRecursive(ResourceChunk** lo, ResourceChunk** hi, s32 ctx);

#pragma opt_lifetimes off
#pragma opt_propagation off

#pragma opt_lifetimes reset
#pragma opt_propagation reset

struct Flag {
    volatile u8 v;
    u8 pad[3];
};

struct ChunkIter {
    ResourceChunk* volatile p;
    // Observe the stored iterator each time; compaction depends on read order.
    ResourceChunk* cur() { return p; }
    // Advance one 16-byte descriptor with a visible iterator write.
    void step() { p = p + 1; }
};

extern void* (*s_genericResourceAllocatorFunctions)(u32 size);


extern DdsPixelFormatTable s_ddsPixelFormatTable;

void gtl_Image_CopyFromPlatformLayout(s32 sourcePixels, s32 widthPixels, s32 heightPixels, s32 sourceWord3, s32 destinationPixels, s32 destinationWord5, s32 destinationWord6, s32 engineFormat, s32 sourceLayout);
s32 gtl_ImageTGA_GetImageData(void** arg0, const TgaHeader* arg1);
s32 gtl_ImageTGA_GetPaletteData(void** arg0, TgaHeader* arg1);
s32 gtl_ImageTGA_GetDescription(TgaDescOut* arg0, TgaDescPalette* arg1, const TgaHeader* arg2);
FontDataView* gtl_FontData_InitView(FontDataView* self, FontDataObject* obj);
D6894Holder* FontDataView_dtor(D6894Holder* self, s32 flags);
void gtl_FontData_CopyHeader(const GlyphFont* font, FontHeaderData* out);
s32 gtl_FontData_FindGlyph(const GlyphFont* font, s32 code, GlyphRecord* out);
s32 gtl_FontData_GetRecord(const FontDataView* self, s32 index, FontDataRecord* out);
s32 gtl_FontData_CompareGlyphCode(const void* a, const void* b);
s32 gtl_FontData_FindGlyphRecord(const GlyphFont* arg0, s32 arg1, GlyphRecord* arg2);
s32 gtl_ImageDDS_IsValid(const u32* data);
s32 gtl_ImageDDS_GetDescription(TgaDescOut* arg0, TgaDescPalette* arg1, unsigned char* arg2);
s32 gtl_ImageDDS_GetImageData(void** out, const u8* data);
void* gtl_GenericResource_Allocate(u32 size);
void gtl_GenericResource_Free(void* p);
void gtl_ResourceFile_RebuildChunks(void** arg0, void* arg1, s32 arg2, s32* arg3);
void gtl_ResourceChunks_SortByOffset(ResourceChunk** lo, ResourceChunk** hi, s32 ctx);

#pragma push

// Copy ONE image level out of a platform layout; no allocation/status result.
// sourcePixels/destinationPixels are borrowed 32-bit addresses, disjoint and
// caller-sized; width/height are nonnegative texel counts, products must fit u32.
// engineFormat is a GTL format, sourceLayout is a GSR layout selector (not a
// GX format): 3=type4/Morton, 5=type1/contiguous, 7=type5/Wii tiles. Other layouts
// return without writing. Layout5 copies floor(width*height*bitsPerPixel/8)
// bytes, including sub-byte formats; -1/unknown-byte formats copy zero bytes.
// Layout3 delegates to ConvertMortonLayout(reverse=0); sourceWord3,
// destinationWord5/6 are forwarded ABI words ignored by that implementation.
// Square power-of-two dimensions suffice for dense ordinary Morton storage;
// other shapes need storage covering its generated Morton offsets. Compressed
// formats copy contiguous bytes. See gtl_image_layout_and_sizing.cpp for exact domains/padding.
// Layout7 forwards (src,word5,word3,dst,width,height) ONLY SIX arguments even
// though gtl_image_layout_and_sizing.cpp's TiledToLinear reads an eighth format argument. Thus this
// code does not establish the format seen by that callee; word6/engineFormat
// are not forwarded on this path. Preserve that ABI discrepancy for a behavior
// audit. Tile output cannot safely be inferred from this wrapper alone.
void gtl_Image_CopyFromPlatformLayout(s32 sourcePixels, s32 widthPixels, s32 heightPixels, s32 sourceWord3, s32 destinationPixels, s32 destinationWord5, s32 destinationWord6, s32 engineFormat, s32 sourceLayout) {
    u32 bitsPerPixel;
    u32 pixelCount;

    switch (sourceLayout) {
    case 7:
        gtl_Image_TiledToLinear(sourcePixels, destinationWord5, sourceWord3, destinationPixels, widthPixels, heightPixels);
        return;
    case 5:
        switch (engineFormat) {
        case 0x1F:
        case 0x21:
        case 0x59:
        case 0x5B:
            bitsPerPixel = 4;
            break;
        case 0x2E:
        case 0x62:
            bitsPerPixel = 4;
            break;
        case 0x2F:
        case 0x30:
        case 0x31:
        case 0x32:
        case 0x63:
        case 0x64:
        case 0x65:
        case 0x66:
            bitsPerPixel = 8;
            break;
        default:
            bitsPerPixel = gtl_ImageFormat_GetBytesPerElement(engineFormat) * 8;
            break;
        case -1:
            bitsPerPixel = 0;
            break;
        }
        pixelCount = heightPixels * widthPixels;
        memcpy((void*) destinationPixels, (const void*) sourcePixels, (u32) (pixelCount * bitsPerPixel) >> 3);
        return;
    case 3:
        sourceLayout = 0;
        gtl_Image_ConvertMortonLayout(sourcePixels, destinationWord5, sourceWord3, destinationPixels, widthPixels, heightPixels, destinationWord6, engineFormat, sourceLayout);
        return;
    }
}

// Map palette-entry bit depth to engine format; unsupported/no palette -> -1.
static inline s32 TgaPaletteFormat(const TgaHeader* header) {
    switch (header->colorMapEntryBits) {
    case 32:
        return 0x3F;
    case 24:
        return 0x3E;
    case 0:
        return -1;
    default:
        return -1;
    }
}
#pragma push
#pragma opt_propagation off
// Return borrowed pixel start after 18-byte header, ID bytes and palette bytes.
// Header/out required; no RLE decoding or size validation; status always 1.
// Palette length is used as stored (no endian reversal), unlike dimensions.
s32 gtl_ImageTGA_GetImageData(void** arg0, const TgaHeader* arg1) {
    s32 fmt = TgaPaletteFormat(arg1);
    s32 size;
    if (fmt == -1) {
        size = 0;
    } else {
        size = gtl_Image_GetMipChainSize(arg1->colorMapLengthRaw, 1, 1, fmt, 0);
    }
    char* base = (char*)arg1 + arg1->unk0[0];
    *arg0 = base + size + 0x12;
    return 1;
}
#pragma pop

// Return borrowed palette start for color-map type1; otherwise explicitly null.
// Header/out required, no validation/allocation; always returns 1.
s32 gtl_ImageTGA_GetPaletteData(void** arg0, TgaHeader* arg1) {
    if (arg1->unk0[1] != 1) {
        *arg0 = 0;
        return 1;
    }
    *arg0 = (unsigned char*) arg1 + arg1->unk0[0] + 0x12;
    return 1;
}

// Describe pixels (8/16/24/32 bits) and optional palette (24/32 bits). Required
// arg0/header; arg1 nullable. Unsupported depths write format -1 and still
// return 1. Width/height alone are reversed from file order; palette count is not.
s32 gtl_ImageTGA_GetDescription(TgaDescOut* imageDescription, TgaDescPalette* paletteDescription, const TgaHeader* header) {
    s32 pixelFormat;
    s32 paletteEntryCount;
    u32 paletteFormat;
    struct {
        unsigned short width;
        unsigned short height;
    } dimensions;

    u8 bpp = header->pixelBits;
    switch (bpp) {
    case 32:
        pixelFormat = 0x3F;
        break;
    case 24:
        pixelFormat = 0x3E;
        break;
    case 16:
        pixelFormat = 0x46;
        break;
    case 8:
        pixelFormat = 0x58;
        break;
    default:
        pixelFormat = -1;
        break;
    }
    imageDescription->format = pixelFormat;
    u16 width = header->widthLE;
    imageDescription->mipLevelCount = 1;
    dimensions.width = width;
    u16 height = header->heightLE;
    dimensions.height = height;
    Memory_CopyAndReverseStridedBlocks(&dimensions.width, &dimensions.width, 0x20001, 2, 1);
    Memory_CopyAndReverseStridedBlocks(&dimensions.height, &dimensions.height, 0x20001, 2, 1);
    imageDescription->widthPixels = (s32) dimensions.width;
    imageDescription->heightPixels = (s32) dimensions.height;
    if (paletteDescription != 0) {
        u32 pbits = header->colorMapEntryBits;
        switch (pbits) {
        case 32:
            paletteFormat = 0x3F;
            break;
        case 24:
            paletteFormat = 0x3E;
            break;
        case 0:
            paletteFormat = -1;
            break;
        default:
            paletteFormat = -1;
            break;
        }
        paletteEntryCount = header->colorMapLengthRaw;
        paletteDescription->paletteFormat = paletteFormat;
        paletteDescription->entryCountRaw = paletteEntryCount;
    }
    return 1;
}

// Retain required obj; borrow its +8 header/data for this view. Set glyph byte
// range [header+28, +count*32), install global comparator mode, return self.
// FontDataView_dtor drops the retained owner and resets that shared mode.
FontDataView* gtl_FontData_InitView(FontDataView* self, FontDataObject* obj) {
    u8* glyphs;
    FontDataHeader* hdr;
    FontDataHeader* glyphHeader;

    self->obj = obj;
    self->hdr = 0;
    self->glyphs = 0;
    self->glyphsEnd = 0;
    obj->refCount++;
    hdr = (FontDataHeader*) Object_GetField8(self->obj);
    self->hdr = hdr;
    s_glyphComparatorMode = hdr->comparatorModeFlagsA | hdr->comparatorModeFlagsB;
    glyphHeader = self->hdr;
    glyphs = (u8*) glyphHeader + 0x28;
    self->glyphs = glyphs;
    self->glyphsEnd = glyphs + (glyphHeader->recordCount << 5);
    return self;
}

// Nullable holder; nonnull requires a live retained obj. Drop ref, delete at
// zero, clear global font mode; positive flags delete holder storage as well.
D6894Holder* FontDataView_dtor(D6894Holder* self, s32 flags) {
    if (self != 0) {
        D6894RefCounted* d = self->obj;
        if (--d->refCount == 0) {
            delete d;
        }
        s_glyphComparatorMode = 0;
        if (flags > 0) {
            operator delete(self);
        }
    }
    return self;
}

// Copy the native 40-byte header verbatim; font/header/out required, no retain.
void gtl_FontData_CopyHeader(const GlyphFont* font, FontHeaderData* out) {
    const FontHeaderData* h = (const FontHeaderData*) font->header;
    out->tag[0] = h->tag[0];
    out->tag[1] = h->tag[1];
    out->tag[2] = h->tag[2];
    out->tag[3] = h->tag[3];
    out->unk4 = h->unk4;
    out->unk8 = h->unk8;
    out->recordCount = h->recordCount;
    out->unk10 = h->unk10;
    out->unk14 = h->unk14;
    out->comparatorModeFlagsA = h->comparatorModeFlagsA;
    out->comparatorModeFlagsB = h->comparatorModeFlagsB;
    out->unk20 = h->unk20;
    out->unk24 = h->unk24;
}

// Forward code lookup; same lifetime/mode/unchanged-on-miss contract as FindGlyphRecord.
s32 gtl_FontData_FindGlyph(const GlyphFont* font, s32 code, GlyphRecord* out) {
    return gtl_FontData_FindGlyphRecord(font, code, out);
}

// Copy one native 32-byte record; index>=0 required (only upper bound tested).
// Out required; returns 0 without writing when index>=count, else copies and returns 1.
s32 gtl_FontData_GetRecord(const FontDataView* self, s32 index, FontDataRecord* out) {
    const u8* src;
    u8* dst;
    u8 b0;
    u8 b1;
    const FontDataRecord* rec;

    if (self->hdr->recordCount <= index) {
        return 0;
    }
    rec = (const FontDataRecord*) self->glyphs + index;
    *out = *rec;
    return 1;
}

// bsearch comparator: both pointers require a leading native u16 code. All four
// recognized modes (0x11/21/12/22) currently compare unsigned numeric code units
// identically, with no endian conversion or text decoding. Their encoding names
// are unproven. InitView writes header(+18)|header(+1C) to s_glyphComparatorMode;
// holder destruction resets it to 0. Unknown mode compares everything equal,
// so searching with no active compatible view can return an arbitrary entry.
// One global mode is shared by all fonts: serialize view init/destroy/search.
s32 gtl_FontData_CompareGlyphCode(const void* a, const void* b) {
    switch (s_glyphComparatorMode) {
    case 0x11: {
        u16 kb;
        u16 ka;
        ka = *(const u16*) a;
        kb = *(const u16*) b;
        if (ka < kb) {
            return -1;
        }
        return ka > kb;
    }
    case 0x21: {
        u16 kb;
        u16 ka;
        ka = *(const u16*) a;
        kb = *(const u16*) b;
        if (ka < kb) {
            return -1;
        }
        return ka > kb;
    }
    case 0x12: {
        u16 kb;
        u16 ka;
        ka = *(const u16*) a;
        kb = *(const u16*) b;
        if (ka < kb) {
            return -1;
        }
        return ka > kb;
    }
    case 0x22: {
        u16 kb;
        u16 ka;
        ka = *(const u16*) a;
        kb = *(const u16*) b;
        if (ka < kb) {
            return -1;
        }
        return ka > kb;
    }
    default:
        return 0;
    }
}

#pragma opt_common_subs off
#pragma opt_propagation off
// Font/header/table/out required and borrowed. Header +14 selects representation:
// 0: u16 direct indices, count at +20, zero index means absent; upper check is
// code > count (equal is accepted), then six native halfwords at 2*count+2+12*idx.
// 1: sorted 14-byte GlyphEntry array, length +24, comparator mode described above;
// code narrows to u16. Other modes/misses return 0 and leave output unchanged.
// Six GlyphRecord fields are transported verbatim; their atlas/page/bearing/
// width/extra-advance schema and pen equation are above, proven by the bitmap
// renderer in gtl_bitmap_font_draw.cpp. Keep signedness; this is not UTF-8 decoding.
// Existing out bytes are read to initialize search scratch, even on a miss.
s32 gtl_FontData_FindGlyphRecord(const GlyphFont* font, s32 code, GlyphRecord* outGlyph) {
    const GlyphFontHeader* hdr;
    s32 maximumCode;
    const u16* codeOrEntryTable;
    s32 glyphIndex;
    const u8* metricArray;
    s32 glyphByteOffset;
    const GlyphRecord* metrics;
    GlyphEntry* matchingEntry;
    u8* src;
    u8* dst;
    u8 b1;
    u8 b0;
    u8 f1;
    u8 f0;
    u8 g1;
    u8 g0;

    hdr = font->header;
    switch (hdr->lookupMode) {
    case 0:
        maximumCode = hdr->directMapMaxCode;
        if ((u32) maximumCode < code) {
            return 0;
        }
        codeOrEntryTable = font->codeTable;
        glyphIndex = codeOrEntryTable[code];
        if (glyphIndex == 0) {
            return 0;
        }
        metricArray = (const u8*) codeOrEntryTable + maximumCode * 2;
        glyphByteOffset = glyphIndex * 12;
        metricArray = metricArray + 2;
        outGlyph->atlasU = *(u16*) (metricArray + glyphByteOffset);
        metrics = (const GlyphRecord*) (metricArray + glyphByteOffset);
        outGlyph->atlasV = metrics->atlasV;
        outGlyph->texturePageIndex = metrics->texturePageIndex;
        outGlyph->horizontalBearing = metrics->horizontalBearing;
        outGlyph->width = metrics->width;
        outGlyph->extraAdvance = metrics->extraAdvance;
        return 1;
    case 1: {
        GlyphEntry key = {0};
        key.code = code;
        src = (u8*) outGlyph;
        dst = (u8*) &key.rec;
        f0 = src[0];
        f1 = src[1];
        dst[0] = f0;
        dst[1] = f1;
        g0 = src[2];
        g1 = src[3];
        dst[2] = g0;
        dst[3] = g1;
        b0 = src[4];
        b1 = src[5];
        dst[4] = b0;
        dst[5] = b1;
        b1 = src[6];
        b0 = src[7];
        dst[6] = b1;
        dst[7] = b0;
        b0 = src[8];
        dst[8] = b0;
        b0 = src[9];
        dst[9] = b0;
        b0 = src[10];
        dst[10] = b0;
        b0 = src[11];
        dst[11] = b0;
        matchingEntry = (GlyphEntry*) bsearch(&key, font->codeTable, hdr->sortedEntryCount, sizeof(GlyphEntry), gtl_FontData_CompareGlyphCode);
        if (matchingEntry != 0) {
            outGlyph->atlasU = matchingEntry->rec.atlasU;
            outGlyph->atlasV = matchingEntry->rec.atlasV;
            outGlyph->texturePageIndex = matchingEntry->rec.texturePageIndex;
            outGlyph->horizontalBearing = matchingEntry->rec.horizontalBearing;
            outGlyph->width = matchingEntry->rec.width;
            outGlyph->extraAdvance = matchingEntry->rec.extraAdvance;
            return 1;
        }
    }
    default:
        return 0;
    }
}

#pragma opt_common_subs reset
#pragma opt_propagation reset
// Test only the native magic word 0x20534444, not length/header/payload validity.
s32 gtl_ImageDDS_IsValid(const u32* data) {
    return *data == 0x20534444;
}

// Required converted DDS/header/out; optional palette arg1 is UNUSED/untouched.
// size!=0x7C nulls the local header then dereferences it: not a graceful failure.
// Match 32-byte pixel-format tuple against s_ddsPixelFormatTable's 10 entries, else -1;
// flag0x20000 enables stored mipCount, else1. No byte swapping, always status1.
s32 gtl_ImageDDS_GetDescription(TgaDescOut* arg0, TgaDescPalette* arg1, unsigned char* arg2) {
    DdsHeader* hdr;
    DdsPixelFormatTable table;
    u32 i;
    DdsPixelFormatEntry* entry;
    s32 fmt;

    hdr = (DdsHeader*) (arg2 + 4);
    if (hdr->size != 0x7C) {
        hdr = 0;
    }
    table = s_ddsPixelFormatTable;
    entry = table.entries;
    for (i = 0; i < 10; i++) {
        if (memcmp(hdr->pixelFormat, entry->pixelFormat, 0x20) == 0) {
            fmt = entry->format;
            goto found;
        }
        entry++;
    }
    fmt = -1;
found:
    arg0->format = fmt;
    arg0->widthPixels = hdr->width;
    arg0->heightPixels = hdr->height;
    if (hdr->flags & 0x20000) {
        arg0->mipLevelCount = hdr->mipCount;
    } else {
        arg0->mipLevelCount = 1;
    }
    return 1;
}

// Borrow start at data+4+0x7C for native size0x7C. Invalid size instead produces
// address0x7C and still returns1; caller must validate before dereferencing.
s32 gtl_ImageDDS_GetImageData(void** out, const u8* data) {
    const u32* header;
    header = (const u32*) (data + 4);
    if (*header != 0x7C) {
        header = 0;
    }
    *out = (void*) ((const u8*) header + 0x7C);
    return 1;
}

// Allocate bytes through current process-wide resource callback; caller owns result.
void* gtl_GenericResource_Allocate(u32 size) {
    return s_genericResourceAllocatorFunctions(size);
}

// Release callback-allocated storage with matching live allocator table; no null guard.
void gtl_GenericResource_Free(void* p) {
    GenericResourceAllocTable* t = (GenericResourceAllocTable*) &s_genericResourceAllocatorFunctions;
    t->free(p, t);
}

// Pack borrowed descriptors/payloads into a new owned IG3DXBuffer in *outBuffer.
// outBuffer/headerMetadata required; metadata supplies two native words.
// sourceChunks contains chunkCount descriptors; count/products/sums must fit.
// No overlap with output allocation. No status/rollback: create/alloc results
// are unchecked; exceptions release descriptors only, and a published output can remain.
// Policy/phase map:
// 1 allocate/init/copy: own descriptor scratch; never own source payloads.
// 2 sort by input offset key; skip invalid prefix, compact valid descriptors in
//   place (all of address, element count, stride bytes must be nonzero), truncate tail.
// 3 size: 0x20 header + 16-byte descriptors + each payload rounded up to 16.
// 4 publish buffer/header; pass A assigns logical keys and aligned output offsets;
//   pass B copies payloads. Padding bytes are not explicitly initialized.
// Flag/volatile iterator scratch preserves observed load/store and unwind shape:
// compactRead scans to first invalid, scanEnd/sourceEnd mark the old end,
// compactWrite is the write cursor; *Snapshot/Flag locals preserve scaffolding
// accesses, never output format fields. sortTagScratch retains an unspecified
// incoming byte; the sort treats context opaquely. Do not invent a flag meaning.
// ResourceChunk.offset changes meaning only in OUTPUT descriptors; descriptors retain
// input keys and payload addresses for both passes. Output header tag/version
// are 0x66667267/0x013240D4 in native word order; this function does no swapping.
// Initialization/compaction retain named test/body labels: a fully structured
// rewrite inverted two branch predicates and failed conservative equivalence.
// The labels spell out pre-test loops and retain the volatile iterator rereads;
// sizing and the two output passes below use verified ordinary loops instead.
void gtl_ResourceFile_RebuildChunks(void** outBuffer, void* sourceChunks, s32 chunkCount, s32* headerMetadata) {
    ResourceChunkBuffer descriptors;
    ResourceChunk tmp;
    volatile ResourceChunk zeroDescriptorScratch;
    Flag allocationTag;
    Flag initializationTag;
    ResourceChunk* sortLo;
    ResourceChunk* sortHi;
    ResourceChunk* initializationCursor;
    ResourceChunk* descriptorEnd;
    ResourceChunk* remainingDescriptor;
    ResourceChunk* compactedEnd;
    ResourceChunk* outputChunk;
    ResourceChunk* previousOutputChunk;
    ResourceChunk* sourceDescriptor;
    ResourceChunk* firstInvalid;
    s32* outputHeader;
    s32 chunkIndex;
    s32 outputByteCount;
    s32 alignmentRemainder;
    u8* payloadDestination;
    Flag sortTagScratch;
    Flag sortTag;
    Flag scanTag;
    Flag eraseTag;
    ResourceChunk* volatile firstInvalidSnapshot;
    ChunkIter compactRead;
    ChunkIter scanEnd;
    ResourceChunk* volatile eraseEndSnapshot;
    ResourceChunk* volatile descriptorEndSnapshot;
    ResourceChunk* volatile descriptorBeginSnapshot;
    ResourceChunk* volatile sortEndSnapshot;
    ResourceChunk* volatile sortBeginSnapshot;
    ResourceChunk* volatile eraseLastSnapshot;
    ChunkIter sourceEnd;
    ChunkIter compactWrite;
    ResourceChunk* volatile eraseFirstSnapshot;

    descriptors.base = 0;
    descriptors.count = 0;
    descriptors.capacity = 0;
    if (chunkCount == 0) {
        goto copyAndSortDescriptors;
    }
    allocationTag.v = 0;
    if ((u32) chunkCount <= 0x0FFFFFFFU) {
        goto allocateDescriptors;
    }
    {
        std::length_error err(s_lengthErrorInfo, s_lengthErrorInfo + 0x14);
    }
allocateDescriptors:
    descriptors.base = (ResourceChunk*) operator new(chunkCount * 16);
    descriptors.capacity = chunkCount;
    chunkIndex = chunkCount;
    initializationCursor = descriptors.base + descriptors.count;
    initializationTag.v = 0;
    goto testInitializationRemaining;
initializeNextDescriptor:
    zeroDescriptorScratch.payloadAddressOrKey = 0;
    zeroDescriptorScratch.elementCount = 0;
    zeroDescriptorScratch.elementStrideBytes = 0;
    zeroDescriptorScratch.offset = 0;
    try {
        if (initializationCursor != 0) {
            initializationCursor->payloadAddressOrKey = 0;
            initializationCursor->elementCount = 0;
            initializationCursor->elementStrideBytes = 0;
            initializationCursor->offset = 0;
        }
    } catch (...) {
        throw;
    }
    chunkIndex -= 1;
    initializationCursor++;
    descriptors.count += 1;
testInitializationRemaining:
    if (chunkIndex != 0) {
        goto initializeNextDescriptor;
    }
copyAndSortDescriptors:
    memcpy(descriptors.base, sourceChunks, (u32) (chunkCount << 4));
    descriptorEnd = descriptors.base + descriptors.count;
    sortBeginSnapshot = descriptors.base;
    sortTag.v = sortTagScratch.v;
    sortEndSnapshot = descriptorEnd;
    sortHi = descriptorEnd;
    sortLo = descriptors.base;
    gtl_ResourceChunks_SortByOffset(&sortLo, &sortHi, (s32) &sortTag);

    scanTag.v = 0;
    descriptorEndSnapshot = descriptors.base + descriptors.count;
    sourceEnd.p = descriptors.base + descriptors.count;
    descriptorBeginSnapshot = descriptors.base;
    scanEnd.p = descriptors.base + descriptors.count;
    compactRead.p = descriptors.base;
    goto testValidPrefix;
advanceValidPrefix:
    compactRead.step();
testValidPrefix:
    if (compactRead.cur() == scanEnd.cur()) {
        goto beginCompaction;
    }
    {
        s32 invalid = 0;
        ResourceChunk* candidate = compactRead.cur();
        if (candidate->elementStrideBytes == 0 || candidate->elementCount == 0 || candidate->payloadAddressOrKey == 0) {
            invalid = 1;
        }
        if (invalid == 0) {
            goto advanceValidPrefix;
        }
    }
beginCompaction:
    firstInvalidSnapshot = compactRead.cur();
    compactWrite.p = compactRead.cur();
    if (compactRead.cur() == sourceEnd.cur()) {
        goto truncateCompactedTail;
    }
    remainingDescriptor = compactRead.cur() + 1;
    goto testCompactionRemaining;
compactNextDescriptor:
    {
        s32 invalid = 0;
        if (remainingDescriptor->elementStrideBytes == 0 || remainingDescriptor->elementCount == 0 || remainingDescriptor->payloadAddressOrKey == 0) {
            invalid = 1;
        }
        if (invalid == 0) {
            ResourceChunk* destinationDescriptor = compactWrite.cur();
            destinationDescriptor->payloadAddressOrKey = remainingDescriptor->payloadAddressOrKey;
            compactWrite.step();
            destinationDescriptor->elementCount = remainingDescriptor->elementCount;
            destinationDescriptor->elementStrideBytes = remainingDescriptor->elementStrideBytes;
            destinationDescriptor->offset = remainingDescriptor->offset;
        }
    }
    remainingDescriptor++;
testCompactionRemaining:
    if (remainingDescriptor != sourceEnd.cur()) {
        goto compactNextDescriptor;
    }
truncateCompactedTail:
    compactedEnd = compactWrite.cur();
    descriptorEnd = descriptors.base + descriptors.count;
    eraseTag.v = 0;
    eraseEndSnapshot = descriptorEnd;
    eraseLastSnapshot = descriptorEnd;
    eraseFirstSnapshot = compactedEnd;
    if (compactedEnd == descriptorEnd) {
        goto sizeOutputBuffer;
    }
    memmove(compactedEnd, descriptorEnd, (((descriptors.base + descriptors.count) - descriptorEnd) / 16) * 16);
    descriptors.count = descriptors.count - (descriptorEnd - compactedEnd);
sizeOutputBuffer:
    outputByteCount = 0x20 + (descriptors.count << 4);
    alignmentRemainder = outputByteCount % 16;
    if (alignmentRemainder != 0) {
        outputByteCount += 16 - alignmentRemainder;
    }
    chunkIndex = 0;
    sourceDescriptor = descriptors.base;
    while ((u32) chunkIndex < (u32) descriptors.count) {
        outputByteCount += sourceDescriptor->elementCount * sourceDescriptor->elementStrideBytes;
        alignmentRemainder = outputByteCount % 16;
        if (alignmentRemainder != 0) {
            outputByteCount += 16 - alignmentRemainder;
        }
        sourceDescriptor++;
        chunkIndex++;
    }
    gtl_IG3DXBuffer_Create(outputByteCount, outBuffer, 0);

    outputHeader = (s32*) Object_GetField8(*outBuffer);
    outputHeader[0] = 0x20;
    outputHeader[3] = (s32) Object_GetFieldC(*outBuffer);
    outputHeader[4] = headerMetadata[0];
    outputHeader[1] = 0x66667267;
    outputHeader[2] = headerMetadata[1];
    outputHeader[6] = 0x013240D4;
    outputHeader[5] = descriptors.count;

    chunkIndex = 0;
    sourceDescriptor = descriptors.base;
    // Pass A: metadata only; payload bytes are copied after every offset is known.
    while ((u32) chunkIndex < (u32) descriptors.count) {
        outputChunk = gtl_ResourceFile_GetChunkByIndex(outputHeader, (void*) outputHeader[3], chunkIndex);
        outputChunk->elementCount = sourceDescriptor->elementCount;
        outputChunk->elementStrideBytes = sourceDescriptor->elementStrideBytes;
        outputChunk->payloadAddressOrKey = sourceDescriptor->offset;
        if (chunkIndex == 0) {
            outputByteCount = outputChunk->offset = (descriptors.count << 4) + 0x20;
            alignmentRemainder = outputByteCount % 16;
            if (alignmentRemainder != 0) {
                outputByteCount += 16 - alignmentRemainder;
            }
            outputChunk->offset = outputByteCount;
        } else {
            previousOutputChunk = gtl_ResourceFile_GetChunkByIndex(outputHeader, (void*) outputHeader[3], chunkIndex - 1);
            outputByteCount = outputChunk->offset = previousOutputChunk->offset + (sourceDescriptor - 1)->elementCount * (sourceDescriptor - 1)->elementStrideBytes;
            alignmentRemainder = outputByteCount % 16;
            if (alignmentRemainder != 0) {
                outputByteCount += 16 - alignmentRemainder;
            }
            outputChunk->offset = outputByteCount;
        }
        sourceDescriptor++;
        chunkIndex++;
    }

    chunkIndex = 0;
    sourceDescriptor = descriptors.base;
    // Pass B: output owns copies; the borrowed input buffers are never released.
    while ((u32) chunkIndex < (u32) descriptors.count) {
        outputChunk = gtl_ResourceFile_GetChunkByIndex(outputHeader, (void*) outputHeader[3], chunkIndex);
        outputByteCount = outputChunk->offset;
        payloadDestination = (u8*) Object_GetField8(*outBuffer) + outputByteCount;
        if (sourceDescriptor->payloadAddressOrKey != 0) {
            memcpy(payloadDestination, (void*) sourceDescriptor->payloadAddressOrKey, sourceDescriptor->elementCount * sourceDescriptor->elementStrideBytes);
        } else {
            outputChunk->offset = 0;
        }
        sourceDescriptor++;
        chunkIndex++;
    }

}

// Sort a valid half-open descriptor range by ascending unsigned offset, in place.
// Small ranges select minima; large ones partition/recurse. Equal keys are not
// stable. Updates global pivot sequence s_sortPivotSequence. ctx is opaque scaffolding
// forwarded to helpers, not an owner. No allocation or payload modification.
void gtl_ResourceChunks_SortByOffset(ResourceChunk** pBegin, ResourceChunk** pEnd, s32 ctx)
{
    ResourceChunk* last;
    for (;;) {
        int n = *pEnd - *pBegin;
        if (n <= 1) {
            return;
        }
        if (n <= 20) {
            ResourceChunk* end = *(ResourceChunk* volatile*)pEnd;
            ResourceChunk* cur = *(ResourceChunk* volatile*)pBegin;
            if (cur != end) {
                last = end - 1;
                if (cur != last) {
                    while (cur != last) {
                        ResourceChunk* min = cur;
                        if (cur != end) {
                            for (ResourceChunk* p = cur + 1; p != end; p++) {
                                if (p->offset < min->offset) {
                                    min = p;
                                }
                            }
                        }
                        if (min != cur) {
                            ResourceChunk tmp = *min;
                            *min = *cur;
                            *cur = tmp;
                        }
                        cur++;
                    }
                }
            }
            return;
        }

        int v = s_sortPivotSequence;
        ResourceChunk* m1 = *(ResourceChunk* volatile*)pBegin + (n / 4 + v % 5);
        int k = v + 1;
        if (k >= 5) {
            k = -4;
        }
        int next = k + 1;
        s_sortPivotSequence = next;
        int t = n * 3 / 4;
        ResourceChunk* m2 = *pBegin + (t + k % 5);
        if (next >= 5) {
            s_sortPivotSequence = k - 8;
        }
        last = *pEnd - 1;
        ResourceChunk* m3 = last;
        gtl_ResourceChunks_SortThreeByOffset(&m1, &m2, &m3, ctx);

        ResourceChunk* p = *pBegin;
        while (p->offset < last->offset) {
            p++;
        }
        ResourceChunk* q = last;
        do {
            q--;
            if (q == p) {
                break;
            }
        } while (q->offset >= last->offset);
        if (p < q) {
            ResourceChunk tmp = *p;
            *p = *q;
            p++;
            *q = tmp;
            for (;;) {
                while (p->offset < last->offset) {
                    p++;
                }
                do {
                    q--;
                } while (q->offset >= last->offset);
                if (q - p <= 0) {
                    break;
                }
                ResourceChunk tmp2 = *p;
                *p = *q;
                p++;
                *q = tmp2;
            }
        }

        if (p == *pBegin) {
            ResourceChunk tmp = *p;
            ResourceChunk* lp = *(ResourceChunk* volatile*)pEnd - 1;
            *p = *last;
            p++;
            *last = tmp;            if (!((*pBegin)->offset < lp->offset)) {
                ResourceChunk* e = *pEnd;
                while (p != e && (*pBegin)->offset >= p->offset) {
                    p++;
                }
                if (p < lp) {
                    ResourceChunk tmp2 = *p;
                    *p = *lp;
                    *lp = tmp2;
                }
            }
            if (p < lp) {
                for (;;) {
                    while ((*pBegin)->offset >= p->offset) {
                        p++;
                    }
                    do {
                        lp--;
                    } while ((*pBegin)->offset < lp->offset);
                    if (lp - p <= 0) {
                        break;
                    }
                    ResourceChunk tmp3 = *p;
                    *p = *lp;
                    p++;
                    *lp = tmp3;
                }
            }
            *pBegin = p;
        } else {
            int left = p - *pBegin;
            int right = *pEnd - p;
            if (left < right) {
                ResourceChunk* t1 = *pBegin;
                ResourceChunk* t2 = p;
                gtl_ResourceChunks_SortByOffsetRecursive(&t1, &t2, ctx);
                *pBegin = p;
            } else {
                ResourceChunk* t1 = p;
                ResourceChunk* t2 = *pEnd;
                gtl_ResourceChunks_SortByOffsetRecursive(&t1, &t2, ctx);
                *pEnd = p;
            }
        }
    }
}

#pragma pop

} // extern "C"
