// Title-font texture atlas scanning and fixed-font message-resource creation.
// Mod questions / definition -> consumer:
// - Replace atlas images/cell grid/character order: InitArg ->
//   FontAtlas_InitImagesAndGlyphWidths; CFontNumDraw_SelectFont (hud_distance_to_pin_and_cup_info.cpp)
//   supplies registry configuration; CLetter_OnLoad (golf_letter_overlays.cpp) supplies its
//   embedded configuration. Keep image extents, cell sizes and lookup order equal.
// - Ink threshold: kGlyphInkThreshold=50 -> imagePixel/imageAverage -> measured
//   rightmostInkOffset. Raising it can only exclude more pixels, narrowing or
//   preserving advances; the threshold is a byte average, not opacity or pixels.
// - Digit spacing: initialization scale -> first ten table-0 entries; option==0
//   selects measured table 1. WTitleFont_MeasureStringWidth adds half a cell for
//   unmatched ASCII space; WTitleFont_GetAtlasGlyphAdvance applies glyphScale.
// - Viewport scaling: kScaleToViewport and Arg::viewportWidth -> both measurement
//   functions AND WFont_DrawAtlasGlyph; baseline kFloat_640_font in
//   phase1_shared_data.cpp. This scales advances and draw extents by width/640,
//   independent of atlas texels; coordinate text measurement and layout.
// Resource arrays/advance tables are owned; image names and character bytes are
// borrowed. WTitleFont_ReleaseGlyphResources is the release API. The WFixedFont
// message-resource path uses NewObj/its shared header, not the TitleFont atlas.
#include "classes/WFixedFont.h"

#include "game/singletons.h"
#include "sdk_protos.h"
#include <decomp/cw_bitfields.h>
#include "classes/CAppFontResource.h"
#include "classes/CMsgDraw.h"
#include "classes/WOverlay.h"
extern "C" {
extern "C" void* __nwa__FUl(unsigned int);
extern "C" void* CreateSharedTextureResource__FPCci(void*, int);
extern "C" void* ISharedResource_GetPaletteObject(void*);
extern "C" int Get__15ISharedResourceFv(void*);
extern "C" void* ISharedResource_GetPaletteObject(void*);
extern "C" void* ISharedResource_GetLocked(void*);
extern "C" void Texture_GetLevelDescription(void*, int, void*);
extern "C" unsigned int gtl_ImageFormat_FromPlatformFormat(void*, int);
extern "C" void* gtl_Image_GetRowPitch(void*, unsigned int, int);
extern "C" void* Texture_CalculateMipStorageSize(void*, void*, int, void*);
extern "C" int Texture_LockLevel(void*, int, void*, int, int);
extern "C" int TextureFormat_IsPlatformCMPR_C8OrRGBA8(void*);
extern "C" void gtl_Image_CopyFromPlatform7Layout(void*, void*, void*, int, void*, void*, int, void*);
extern "C" void* gtl_ImageFormat_ToSwizzled(void*);
extern "C" unsigned int gtl_ImageFormat_GetBytesPerElement(void*);
extern "C" void Texture_UnlockLevel(void*, int);
extern "C" void* TexturePalette_GetDescription(void*, void*);
extern "C" void* TexturePalette_GetEntryCountForType(void*);
extern "C" int TexturePalette_GetData(void*, void*, int);
extern "C" void TexturePalette_Flush(void*);
extern "C" void CubeResourceStorage_dtor(void*, int);
extern "C" void appFreeUni__FPv(void*);
extern "C" char __vt__Q23std11logic_error[];
extern "C" char s_sharedCharDeleterVtable[];
extern "C" char __vt__Q23std12length_error[];
struct FontMarkupStringPool {
    char markupTokens[0x38];
    char lengthError[0x14];
    char lengthErrorType[0x48];
};
extern "C" FontMarkupStringPool fontMarkupStrings;
extern "C" void __dla__FPv(void*);
extern "C" void* __nw__FUl(unsigned int);
extern "C" void __dl__FPv(void*);
extern "C" void* appMallocUni__FUl(unsigned int);
extern "C" void PairVector_InitReserved(void*);
extern "C" void MessageDrawPairList_Dtor(void*);
extern "C" void PointerIntVector_AppendPair(void*, void*, int);
extern "C" void Sprite_DrawClippedTexturedQuad(void*, void*, void*, float*, float*, float, unsigned int, unsigned int,
                                               int, int);
extern "C" unsigned int __vt__5WFont[];
extern "C" unsigned int __vt__10WFixedFont[];
extern "C" unsigned int s_messageDrawPairList[];
extern "C" unsigned int s_messageDrawListDtorRegistration[];
extern "C" signed char s_pointerPairListInitialized;
extern "C" float kFloat_1_FixedFont;
extern "C" float kFloat_640_font;
extern "C" const float kFloat_0_Font;
extern "C" const float kFloat_0_5_FontSpacing;

// Atlas contract: cfg and its image-name array/paths are borrowed for the full
// synchronous initialization; character bytes remain borrowed until release.
// The stored imageNames address is not used by later recovered atlas APIs.
// Dimensions/cell sizes are positive pixels;
// the character table is a NUL-terminated byte lookup in atlas traversal order.
// Glyph indices address images in sequence, then cells left-to-right/top-to-bottom.
// This code assumes every queried glyph/palette index and format is supported.
// glyphAdvances[0] uses configured digit widths for the first ten glyphs;
// glyphAdvances[1] uses measured rightmost ink offsets for every glyph.
// advanceMode = (option == 0) selects measured table 1 when option is zero.
// Owned arrays use new[]/delete[]; retained native interfaces use packed refs.
enum FontOptions {
    kScaleToViewport = 0x100,
    kGlyphInkThreshold = 50,
    kDigitCount = 10,
    kAdvanceTableCount = 2,
    kSpaceByte = 0x20
};
enum AtlasStorageProtocol {
    kAtlasPlatform = 7,
    kAtlasLevelZero = 0,
    kAtlasMipLevelCount = 1,
    kAtlasIndexBytes1 = 1,
    kAtlasIndexBytes2 = 2,
    kAtlasIndexBytes4 = 4,
    kAtlasTwoPlanePaletteFormatSuffix = 6
};
// These describe which bytes imageAverage samples, without guessing a platform format.
enum InkByteOrder {
    kAverageFourBytes = 0,
    kSkipByteZero = 1,
    kUseFirstThreeBytes = 2
};
// Observed image-format codes; no unverified platform/color names are assigned.
enum AtlasInkFormatCode {
    kInkFormatBase = 0x3e,
    kInkCode3E = 0x3e,
    kInkCode3F = 0x3f,
    kInkCode41 = 0x41,
    kInkCode4B = 0x4b,
    kInkCode4C = 0x4c,
    kInkCode4E = 0x4e
};

// WTitleFont atlas implementation: owns image interfaces and advance arrays.
// This is independent of WFixedFont's message-resource layout (NewObj below).
struct Obj {
    char padding0[0x28];
    unsigned int flags; // 0x28
    char padding1[0x38 - 0x2C];
    float glyphScale; // 0x38
    char padding2[0xA0 - 0x3C];
    void** imageNames;      // 0xA0
    int imageCount;         // 0xA4
    int glyphWidth;         // 0xA8
    int glyphHeight;        // 0xAC
    int atlasWidth;         // 0xB0
    int atlasHeight;        // 0xB4
    int configuredRows;     // 0xB8
    int characterTable;     // 0xBC
    void* imageResources;   // 0xC0
    void* glyphAdvances[2]; // 0xC4
    int advanceMode;        // 0xCC
    void* characters;       // 0xD0
    int glyphCount;         // 0xD4
    int glyphsPerImage;     // 0xD8
    int glyphsPerRow;       // 0xDC
    int loadedImageCount;   // 0xE0
};

// WFixedFont implementation: owns a CAppFontResource/CMsgDraw pair, not an atlas.
struct NewObj {
    void** vtable;
    char padding4[0x2C];
    int baseValue30;
    int baseValue34;
    float glyphScale;
    char padding3c[4];
    unsigned int color40;
    unsigned int color44;
    unsigned int color48;
    char padding4c[0x1C];
    int activeHead;
    int activeIterator;
    int allocationHead;
    int recycledHead;
    int allocationBlockSize;
    int slotMask;
    int slotCount;
    void* slots;
    char padding88[0x20];
    unsigned char borrowedResources;
    char paddinga9[3];
    void* messageResource;
    void* messageDraw;
    int atlasHeight;
};

struct Arg {
    char padding0[0x28];
    unsigned int flags; // 0x28
    char padding1[0x188 - 0x2C];
    float viewportWidth; // 0x188
};

// width/height: glyph cell pixels; totalW/totalH: configured atlas pixels.
// rows is retained metadata; characterLookupAddress is an address (not an index).
// Initialization configuration for WTitleFont atlas images; all pointers borrowed.
struct InitArg {
    void** images;
    int count;
    int width;
    int height;
    int totalW;
    int totalH;
    int rows;
    int characterLookupAddress;
};
void WTitleFont_ReleaseGlyphResources(Obj*);

extern "C" void __dt__Q23std12length_errorFv(void*);
extern "C" void* appMallocDummy__FUl(unsigned int);
extern "C" void* memcpy(void*, const void*, unsigned int);
extern "C" void* memset(void*, int, unsigned int);

#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_propagation off
#pragma opt_strength_reduction off
// Retain a texture resource using its packed 16-bit reference count.
static inline void addRef(void* p) {
    struct Ref {
        CW_BITS2(unsigned int count : 16, unsigned int flags : 16)
    };
    ++((Ref*)p)->count;
}

// Release one reference and return the remaining packed reference count.
static inline int releaseRef(void* p) {
    struct Ref {
        CW_BITS2(unsigned int count : 16, unsigned int flags : 16)
    };
    --((Ref*)p)->count;
    return ((Ref*)p)->count;
}

// Scratch block used while growing the image byte vector inline (the compiler-matching form of
// "allocate bigger buffer, copy the old bytes, swap, free the old buffer"): `block` is the allocation
// held by the temporary, `used` a running byte count, `capacity` its size, `tailOffset` where the old bytes land.
struct VecTmp {
    unsigned char* block;
    unsigned int used;
    unsigned int capacity;
    unsigned int* owner;
    unsigned int tailOffset;
    // Free whichever allocation the growth swap left in this temporary.
    ~VecTmp() {
        if (block != 0) {
            used = 0;
            appFreeUni__FPv(block);
        }
    }
};

struct ImgBuffer {
    unsigned char* begin;
    unsigned int size;
    unsigned int cap;
    // Start with an empty owned linear pixel buffer.
    ImgBuffer() : begin(0), size(0), cap(0) {}
    // Release only the linear copy, never the borrowed lock pointer.
    ~ImgBuffer() {
        if (begin != 0) {
            size -= (begin + size) - begin;
            appFreeUni__FPv(begin);
        }
    }
};

// Per-image scan owner: retains texture/palette interfaces and owns both CPU
// copies. data aliases buffer.begin after a successful level lock/conversion.
struct ImgData {
    void* texture;                  // 0x00
    unsigned int pixelFormat;       // 0x04
    int w;                          // 0x08
    int h;                          // 0x0C
    int flag;                       // 0x10
    int reserved14;                 // 0x14
    unsigned char* data;            // 0x18
    ImgBuffer buffer;               // 0x1C
    unsigned int pixelElementBytes; // 0x28: BYTES PER PIXEL/INDEX, never row pitch
    unsigned int requiredBytes;     // 0x2C
    void* palette;                  // 0x30
    unsigned char* paletteBytes;    // 0x34
    unsigned int paletteFormat;     // 0x38
    unsigned int padding3c;         // 0x3C
    unsigned int paletteCount;      // 0x40
    unsigned int paletteEntryBytes; // 0x44: bytes per decoded palette entry

    // Initialize owned buffer/palette and retained-interface handles to null.
    ImgData()
        : texture(0), data(0), pixelElementBytes(0), requiredBytes(0), palette(0), paletteBytes(0), paletteCount(0),
          paletteEntryBytes(0) {}

    // Release retained native interfaces and the copied palette bytes.
    ~ImgData() {
        void* p = texture;
        if (p != 0) {
            if (releaseRef(p) == 0)
                CubeResourceStorage_dtor(p, 1);
            texture = 0;
        }
        if (paletteBytes != 0) {
            __dla__FPv(paletteBytes);
            paletteBytes = 0;
        }
        p = palette;
        if (p != 0) {
            if (releaseRef(p) == 0)
                CubeResourceStorage_dtor(p, 1);
            palette = 0;
        }
    }
};

// Texture description filled in by the engine's texture query: platform pixel-format code at +0,
// width/height of the top mip level at +0x1C/+0x20 (other words unused here).
struct ImgInfo {
    int platformFormat; // 0x00
    int v4;             // 0x04
    int v8;             // 0x08
    int vC;             // 0x0C
    int v10;            // 0x10
    int v14;            // 0x14
    int v18;            // 0x18
    int w;              // 0x1C
    int h;              // 0x20
};

// Result of locking a texture level: pointer to the raw pixel data at +4.
struct BufInfo {
    int v0;       // 0x00
    void* pixels; // 0x04
};

// Palette description: platform entry-format code at +0, palette type code at +8 (used to look up the entry count).
struct PalInfo {
    int platformFormat; // 0x00
    int v4;             // 0x04
    int typeCode;       // 0x08
    int vC;
    int v10;
};

struct LenErr {
    void* vt;
    void* data;
    void* rep;
};
struct StrObj {
    void* buf;
    void* flag;
    // Guard a newly allocated exception message until the shared rep takes it.
    StrObj(void* b, void* f) : buf(b), flag(f) {}
    // Delete the message unless release() transferred ownership.
    ~StrObj() {
        if (buf)
            __dla__FPv(buf);
    }
    // Transfer message ownership to the exception representation.
    void* release() {
        void* p = buf;
        buf = 0;
        return p;
    }
};

// Construct and throw a length_error while guarding the temporary message buffer.
static inline void ThrowImageLengthError() {
    LenErr le;
    char flag_ = 0;
    void* msg_ = fontMarkupStrings.lengthError;
    le.vt = __vt__Q23std11logic_error;
    unsigned int len_ = strlen((char*)msg_) + 1;
    void* buf_ = __nwa__FUl(len_ * 2 - len_);
    le.data = buf_;
    {
        StrObj guard_(buf_, &flag_);
        void* rep_ = __nw__FUl(0x10);
        if (rep_ != 0) {
            ((int*)rep_)[1] = 1;
            ((int*)rep_)[2] = 1;
            *(void**)rep_ = s_sharedCharDeleterVtable;
            ((void**)rep_)[3] = buf_;
        }
        le.rep = rep_;
        guard_.release();
    }
    strcpy((char*)le.data, (char*)msg_);
    le.vt = __vt__Q23std12length_error;
    __throw(fontMarkupStrings.lengthErrorType, &le, (void*)__dt__Q23std12length_errorFv);
}

// Return the NEW total allocation size (not a capacity increase request) when a byte vector of size `capacity`
// must grow by `need` more bytes: grows by max(need, 60% / 50% of the old size by tier), throws length_error on overflow.
static inline unsigned int imageCapacity(unsigned int capacity, const unsigned int& need) {
    if (need > 0xFFFFFFFFu - capacity)
        ThrowImageLengthError();
    if (capacity < 0x55555555u) {
        unsigned int amount = (capacity + 1) * 3 / 5;
        const unsigned int* selected = amount < need ? &need : &amount;
        return capacity + *selected;
    } else if (capacity < 0xAAAAAAAAu) {
        unsigned int amount = (capacity + 1) >> 1;
        const unsigned int* selected = amount < need ? &need : &amount;
        return capacity + *selected;
    }
    return 0xFFFFFFFFu;
}

// Resolve a pixel address, expanding palette indices when the image is indexed.
// No x/y bounds check. Unsupported index width or index>=paletteCount returns
// null; the scan immediately passes that null to imageAverage, which dereferences
// it. This is NOT a recoverable malformed-atlas status. The loader must supply
// valid decoded pixels, a complete palette and in-range indices for every cell.
static inline unsigned char* imagePixel(ImgData* d, int pixelX, int pixelY) {
    // Linear rows contain w elements; pixelElementBytes is the element size in bytes.
    int pixelByteOffset = (pixelX + pixelY * d->w) * (int)d->pixelElementBytes;
    unsigned char* p;
    if (d->palette != 0) {
        unsigned int idx;
        int indexByteOffset = pixelByteOffset;
        switch (d->pixelElementBytes) {
        case kAtlasIndexBytes1:
            idx = *(unsigned char*)(d->data + indexByteOffset);
            break;
        case kAtlasIndexBytes2:
            idx = *(unsigned short*)(d->data + indexByteOffset);
            break;
        case kAtlasIndexBytes4:
            idx = *(unsigned int*)(d->data + indexByteOffset);
            break;
        default:
            p = 0;
            return p;
        }
        if (d->paletteCount <= idx)
            p = 0;
        else
            p = d->paletteBytes + d->paletteEntryBytes * idx;
    } else {
        p = d->data + pixelByteOffset;
    }
    return p;
}
// Average the selected three or four pixel bytes for the ink-threshold test.
// p must expose 3 bytes for kUseFirstThreeBytes, 4 for the other orders; no null
// handling. Palette/texture entry format and byte order must satisfy this size.
static inline int imageAverage(unsigned char* p, int format) {
    int avg;
    switch (format) {
    case kUseFirstThreeBytes:
        avg = (p[0] + p[1] + p[2]) / 3;
        break;
    case kSkipByteZero:
        avg = (p[1] + p[2] + p[3]) / 3;
        break;
    default:
        avg = ((p[0] + p[2]) + (p[1] + p[3])) / 4;
        break;
    }
    return avg;
}

// Local role aliases leave the existing shared layout names intact.
typedef Obj TitleFontAtlasView;
typedef InitArg TitleFontAtlasConfig;

// Return bytes per linear row; the legacy engine ABI transports width/return through void*.
static inline unsigned int AtlasLinearRowPitchBytes(int widthPixels, unsigned int imageFormat) {
    return (unsigned int)gtl_Image_GetRowPitch((void*)widthPixels, imageFormat, kAtlasPlatform);
}

// Return total mip storage in bytes, using the platform format rather than the converted format.
static inline unsigned int AtlasMipStorageBytes(int widthPixels, int heightPixels, int platformFormat) {
    return (unsigned int)Texture_CalculateMipStorageSize((void*)widthPixels, (void*)heightPixels, kAtlasMipLevelCount,
                                                         (void*)platformFormat);
}

// Load atlas images and scan glyph cells to build fixed and measured advance tables.
// Entry contract (unchecked): self has initialized resource fields suitable for
// ReleaseGlyphResources; cfg is required and does not alias self. count>=0;
// images contains count valid NUL-terminated resource paths. width/height are
// positive cell texels; totalW/totalH are at least one cell and fit every loaded
// level-zero image. Exact divisibility is NOT required: integer division and the
// <=extent-cell loops both discard partial right/bottom cells. All products
// (cells, glyphs, bytes) must fit signed int and the allocator's 32-bit sizes.
// rows is copied metadata, not the loop's row count. Character lookup is required
// for later measurement: NUL-terminated bytes, with each found index<glyphCount.
// cfg and its images/path bytes may expire on return: only this synchronous scan
// dereferences that name array. self->imageNames retains the address as metadata,
// with no ownership or later recovered reader. CLetter_OnLoad uses a stack-local
// one-name array, demonstrating this call-only lifetime; a future consumer/reload
// must not reuse that stored address after return. Character lookup differs:
// MeasureStringWidth reads it later, so keep those bytes until release/replacement.
// option==0 selects measured spacing; other values select digit-configured spacing.
// scale is a finite digit-width multiplier; no clamp or tested safe range exists.
// Failure contract: old resources are released FIRST. Resource readiness spins
// without timeout. Null textures, failed level locks, unsupported conversions,
// incomplete palette data or bad indices have no checked error/rollback path:
// scanning may read null/uninitialized data. Allocation exceptions propagate;
// local ImgData cleans its own copies/refs, not the partially replaced font.
// Only a fully completed call publishes usable tables; no success status exists.
void FontAtlas_InitImagesAndGlyphWidths(TitleFontAtlasView* self, TitleFontAtlasConfig* cfg, int option, float scale) {
    // Phase 1: discard old resources and allocate both tables for the new atlas.
    WTitleFont_ReleaseGlyphResources(self);
    self->imageNames = cfg->images;
    int imageCount = cfg->count;
    self->imageCount = imageCount;
    int cellWidth = cfg->width;
    self->glyphWidth = cellWidth;
    int cellHeight = cfg->height;
    self->glyphHeight = cellHeight;
    int atlasWidth = cfg->totalW;
    self->atlasWidth = atlasWidth;
    int columnsPerImage = atlasWidth / cellWidth;
    int atlasHeight = cfg->totalH;
    self->atlasHeight = atlasHeight;
    self->configuredRows = cfg->rows;
    self->characterTable = cfg->characterLookupAddress;
    int rowsPerImage = atlasHeight / cellHeight;
    self->glyphsPerRow = columnsPerImage;
    self->loadedImageCount = imageCount;
    int cellsPerImage = columnsPerImage * rowsPerImage;
    self->glyphsPerImage = cellsPerImage;
    self->glyphCount = imageCount * cellsPerImage;
    self->characters = (void*)cfg->characterLookupAddress;
    self->imageResources = __nwa__FUl(imageCount * 4);
    Obj* slot = self;
    for (int tableIndex = 0; tableIndex < kAdvanceTableCount; tableIndex++, slot = (Obj*)((char*)slot + 4))
        slot->glyphAdvances[0] = __nwa__FUl(self->glyphCount * 4);
    self->advanceMode = option == 0;

    // Phase 2: load each image and retain its pixel/palette interfaces for scanning.
    int glyphIndex = 0;
    int advanceByteOffset = 0;
    int imageIndex = 0;
    int imageOffset = 0;
    if (cfg->count > 0) {
        for (; imageIndex < cfg->count; imageIndex++, imageOffset += 4) {
            void* imageResource = CreateSharedTextureResource__FPCci(*(void**)((char*)cfg->images + imageOffset), 0);
            *(void**)((char*)self->imageResources + imageOffset) = imageResource;
            while (Get__15ISharedResourceFv(*(void**)((char*)self->imageResources + imageOffset)) == 0) {
            }
            void* palette = ISharedResource_GetPaletteObject(*(void**)((char*)self->imageResources + imageOffset));
            void* texture = ISharedResource_GetLocked(*(void**)((char*)self->imageResources + imageOffset));

            ImgData scanImage;
            ImgInfo textureDescription;
            if (texture != 0) {
                scanImage.texture = texture;
                addRef(texture);
            }
            if (palette != 0) {
                scanImage.palette = palette;
                addRef(palette);
            }

            // Phase 3: copy level zero to a linear CPU buffer. Native platform-7
            // layouts are converted before any per-pixel glyph queries.
            Texture_GetLevelDescription(texture, kAtlasLevelZero, &textureDescription);
            unsigned int format = (unsigned int)gtl_ImageFormat_FromPlatformFormat(
                (void*)textureDescription.platformFormat, kAtlasPlatform);
            scanImage.h = textureDescription.h;
            scanImage.pixelFormat = format;
            scanImage.w = textureDescription.w;
            scanImage.flag = 1;
            // These legacy void* parameters/returns carry integer units in the Wii ABI.
            // Keep row pitch (bytes per row) distinct from scanImage.pixelElementBytes (bytes per pixel).
            unsigned int linearRowPitchBytes = AtlasLinearRowPitchBytes(textureDescription.w, scanImage.pixelFormat);
            scanImage.requiredBytes =
                AtlasMipStorageBytes(textureDescription.w, textureDescription.h, textureDescription.platformFormat);

            BufInfo lockedLevel;
            if (Texture_LockLevel(texture, kAtlasLevelZero, &lockedLevel, 0, 0) != 0) {
                // Phase 3a: resize the owned linear copy before conversion.
                if (scanImage.requiredBytes > scanImage.buffer.size) {
                    unsigned int bytesToAppend = scanImage.requiredBytes - scanImage.buffer.size;
                    // 0x8021B01C unit: fill in place or replace the buffer, then join.
                    do {
                        if (bytesToAppend > scanImage.buffer.cap ||
                            scanImage.buffer.size > scanImage.buffer.cap - bytesToAppend) {
                            imageCapacity(scanImage.buffer.cap,
                                          (scanImage.buffer.size + bytesToAppend) - scanImage.buffer.cap);
                        } else {
                            memset(scanImage.buffer.begin + scanImage.buffer.size, 0, bytesToAppend);
                            scanImage.buffer.size = scanImage.buffer.size + bytesToAppend;
                            break;
                        }

                        {
                            VecTmp growth;
                            growth.block = 0;
                            growth.used = 0;
                            growth.capacity = 0;
                            growth.owner = &scanImage.buffer.cap;
                            growth.tailOffset = 0;
                            unsigned int replacementCapacity = imageCapacity(
                                scanImage.buffer.cap, (scanImage.buffer.size + bytesToAppend) - scanImage.buffer.cap);
                            growth.block = (unsigned char*)appMallocDummy__FUl(replacementCapacity);
                            growth.capacity = replacementCapacity;
                            growth.tailOffset = scanImage.buffer.size;
                            memset(growth.used + (growth.block + scanImage.buffer.size), 0, bytesToAppend);
                            unsigned char* previousPixels = scanImage.buffer.begin;
                            unsigned char* previousEnd = previousPixels + scanImage.buffer.size;
                            unsigned int previousSize = previousEnd - previousPixels;
                            growth.used = growth.used + bytesToAppend;
                            growth.tailOffset = growth.tailOffset - previousSize;
                            memcpy(growth.block + growth.tailOffset, previousPixels, previousSize);
                            memset(previousPixels, 0, previousSize);
                            unsigned char* replacementSizeAsPointer = (unsigned char*)growth.used + previousSize;
                            unsigned char* replacementPixels = growth.block;
                            unsigned int previousCapacity = scanImage.buffer.cap;
                            scanImage.buffer.begin = replacementPixels;
                            scanImage.buffer.cap = growth.capacity;
                            growth.capacity = previousCapacity;
                            growth.block = previousPixels;
                            scanImage.buffer.size = (unsigned int)replacementSizeAsPointer;
                            growth.used = 0;
                        }
                    } while (0);
                } else if (scanImage.requiredBytes < scanImage.buffer.size) {
                    scanImage.buffer.size -= scanImage.buffer.size - scanImage.requiredBytes;
                }
                scanImage.data = scanImage.buffer.begin;
                if (TextureFormat_IsPlatformCMPR_C8OrRGBA8((void*)textureDescription.platformFormat) != 0) {
                    gtl_Image_CopyFromPlatform7Layout(
                        lockedLevel.pixels, (void*)textureDescription.w, (void*)textureDescription.h, 0, scanImage.data,
                        (void*)linearRowPitchBytes, 0, (void*)textureDescription.platformFormat);
                    scanImage.pixelFormat = (unsigned int)gtl_ImageFormat_ToSwizzled((void*)scanImage.pixelFormat);
                } else {
                    memcpy(scanImage.data, lockedLevel.pixels, scanImage.requiredBytes);
                }
                Texture_UnlockLevel(texture, kAtlasLevelZero);
            }

            scanImage.pixelElementBytes =
                (unsigned int)gtl_ImageFormat_GetBytesPerElement((void*)scanImage.pixelFormat);
            // Phase 3b: indexed images also need a linear palette with interleaved entry bytes.
            if (scanImage.palette != 0) {
                PalInfo paletteDescription;
                TexturePalette_GetDescription(scanImage.palette, &paletteDescription);
                scanImage.paletteFormat = (unsigned int)gtl_ImageFormat_FromPlatformFormat(
                    (void*)paletteDescription.platformFormat, kAtlasPlatform);
                if (TextureFormat_IsPlatformCMPR_C8OrRGBA8((void*)paletteDescription.platformFormat) != 0)
                    scanImage.paletteFormat = (unsigned int)gtl_ImageFormat_ToSwizzled((void*)scanImage.paletteFormat);
                scanImage.paletteCount =
                    (unsigned int)TexturePalette_GetEntryCountForType((void*)paletteDescription.typeCode);
                scanImage.paletteEntryBytes =
                    (unsigned int)gtl_ImageFormat_GetBytesPerElement((void*)scanImage.paletteFormat);
                unsigned int paletteByteCount = scanImage.paletteEntryBytes * scanImage.paletteCount;
                if (scanImage.paletteBytes != 0) {
                    __dla__FPv(scanImage.paletteBytes);
                    scanImage.paletteBytes = 0;
                }
                scanImage.paletteBytes = (unsigned char*)__nwa__FUl(paletteByteCount);
                void* paletteSourceBytes; // borrowed palette data; scanImage owns only its copy
                if (TexturePalette_GetData(scanImage.palette, &paletteSourceBytes, 0) != 0) {
                    if (TextureFormat_IsPlatformCMPR_C8OrRGBA8((void*)paletteDescription.platformFormat) != 0) {
                        if ((unsigned int)(paletteDescription.platformFormat + 0x80000000u) ==
                            kAtlasTwoPlanePaletteFormatSuffix) {
                            // Plane 0 supplies the first halfword of every entry;
                            // plane 1 supplies the second. Destination entries are interleaved.
                            unsigned short* firstPlane = (unsigned short*)paletteSourceBytes;
                            unsigned int tableIndex = 0;
                            unsigned short* firstHalfword = firstPlane;
                            unsigned short* destinationEntry = (unsigned short*)scanImage.paletteBytes;
                            for (; tableIndex < scanImage.paletteCount; tableIndex++) {
                                destinationEntry[0] = *firstHalfword;
                                firstHalfword += 1;
                                destinationEntry[1] = firstPlane[tableIndex + scanImage.paletteCount];
                                destinationEntry += 2;
                            }
                        }
                    } else {
                        Memory_Copy(scanImage.paletteBytes, paletteSourceBytes, paletteByteCount);
                    }
                    TexturePalette_Flush(scanImage.palette);
                }
            }

            // Phase 4: choose the ink byte order, then scan the configured cell grid.
            // Code -> averaged byte indices: 3e/41/4b/4e -> 0,1,2;
            // 3f/4c -> 1,2,3; all other codes -> 0,1,2,3.
            // Byte order is observed; color/alpha channel names are unresolved.
            int channelOrder = kAverageFourBytes;
            unsigned int effectiveFormatCode = scanImage.palette != 0 ? scanImage.paletteFormat : scanImage.pixelFormat;
            switch (effectiveFormatCode - kInkFormatBase) {
            case kInkCode3E - kInkFormatBase:
            case kInkCode41 - kInkFormatBase:
            case kInkCode4B - kInkFormatBase:
            case kInkCode4E - kInkFormatBase:
                channelOrder = kUseFirstThreeBytes;
                break;
            case kInkCode3F - kInkFormatBase:
            case kInkCode4C - kInkFormatBase:
                channelOrder = kSkipByteZero;
                break;
            }

            for (int y = 0; y <= self->atlasHeight - self->glyphHeight; y += self->glyphHeight) {
                for (int x = 0; x <= self->atlasWidth - self->glyphWidth; x += self->glyphWidth) {
                    int xEnd = x + self->glyphWidth;
                    int yEnd = y + self->glyphHeight;
                    int rightmostInkOffset = 0;
                    for (int pixelY = y; pixelY < yEnd; pixelY++) {
                        int pixelX = x + rightmostInkOffset;
                        int rowPixelOffset = pixelY * scanImage.w;
                        for (; pixelX < xEnd; pixelX++) {
                            unsigned char* pixelBytes = imagePixel(&scanImage, pixelX, pixelY);
                            int inkAverage = imageAverage(pixelBytes, channelOrder);
                            if (inkAverage > kGlyphInkThreshold && pixelX - x > rightmostInkOffset)
                                rightmostInkOffset = pixelX - x;
                        }
                    }
                    // Store the rightmost ink offset, not an inclusive pixel count.
                    // In table 0 only the first ten glyphs use cellWidth * scale.
                    *(float*)((char*)self->glyphAdvances[1] + advanceByteOffset) = (float)rightmostInkOffset;
                    float configuredAdvance;
                    if (glyphIndex < kDigitCount)
                        configuredAdvance = (float)self->glyphWidth * scale;
                    else
                        configuredAdvance = (float)rightmostInkOffset;
                    *(float*)((char*)self->glyphAdvances[0] + advanceByteOffset) = configuredAdvance;
                    glyphIndex++;
                    advanceByteOffset += 4;
                }
            }
        }
    }
}

#pragma opt_strength_reduction reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma opt_common_subs off
// idx must be in [0, glyphCount); arg may be null to use glyphScale alone.
// Return a glyph advance scaled for the font and optional viewport width.
float WTitleFont_GetAtlasGlyphAdvance(Obj* self, Arg* arg, int idx) {
    int flag = 0;
    if (arg != 0 && (self->flags & kScaleToViewport))
        flag = 1;
    float f1;
    if (flag) {
        float t = arg->viewportWidth;
        f1 = self->glyphScale;
        f1 = f1 * t / kFloat_640_font;
    } else {
        f1 = self->glyphScale;
    }
    int i = self->advanceMode;
    float* arr = (float*)self->glyphAdvances[i];
    f1 = arr[idx] * f1;
    return f1;
}
#pragma opt_common_subs reset

// Release atlas textures and both advance tables, then clear the character lookup.
void WTitleFont_ReleaseGlyphResources(Obj* self) {
    if (self->imageResources != 0) {
        for (int i = 0; i < self->loadedImageCount; i++) {
            void* p = ((void**)self->imageResources)[i];
            if (p != 0) {
                if (p != 0) {
                    struct VObj {
                        virtual void v0() = 0;
                        virtual void v1() = 0;
                        virtual void v2() = 0;
                        virtual void v3() = 0;
                    };
                    ((VObj*)p)->v1();
                    ((void**)self->imageResources)[i] = 0;
                }
            }
        }
    }
    if (self->imageResources != 0) {
        __dla__FPv(self->imageResources);
        self->imageResources = 0;
    }
    for (int i = 0; i < kAdvanceTableCount; i++) {
        if (self->glyphAdvances[i] != 0) {
            __dla__FPv(self->glyphAdvances[i]);
            self->glyphAdvances[i] = 0;
        }
    }
    self->characters = 0;
}

// Measure a string from the atlas character lookup, using a half-cell space width.
float WTitleFont_MeasureStringWidth(Obj* self, Arg* arg, const char* str) {
    float unscaledAdvance;
    void* found;
    char* base;
    signed char c;
    const char* p;
    int idx;
    float* arr;
    float cellWidth;
    unscaledAdvance = kFloat_0_Font;
    p = str;
    while ((c = *p++) != 0) {
        base = (char*)self->characters;
        found = strchr(base, c);
        if (found != 0) {
            idx = (char*)found - base;
            arr = (float*)self->glyphAdvances[self->advanceMode];
            unscaledAdvance += arr[idx];
        } else if (c == kSpaceByte) {
            cellWidth = (float)self->glyphWidth;
            unscaledAdvance = unscaledAdvance + kFloat_0_5_FontSpacing * cellWidth;
        }
    }
    int flag = 0;
    if (arg != 0 && (self->flags & kScaleToViewport))
        flag = 1;
    float effectiveScale;
    if (flag) {
        float viewportWidth = arg->viewportWidth;
        effectiveScale = self->glyphScale;
        effectiveScale = effectiveScale * viewportWidth / kFloat_640_font;
    } else {
        effectiveScale = self->glyphScale;
    }
    return unscaledAdvance * effectiveScale;
}

// Initialize the fixed-font node pool, message resources, and shared draw registration.
NewObj* WFixedFont_Ctor(NewObj* self) {
    ((WOverlay*)self)->__ct();
#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_strength_reduction off
    void* mtx;
    int zero = 0;
    self->baseValue30 = zero;
    self->vtable = (void**)__vt__5WFont;
    self->slots = (void*)zero;
    OSInitMutex((OSMutex*)(AsWFixedFont(self)->mutex));
    mtx = AsWFixedFont(self)->mutex;
    OSLockMutex((OSMutex*)mtx);
    self->activeHead = zero;
    int n = 4;
    int cond = n > 0;
    self->recycledHead = zero;
    self->allocationHead = zero;
    self->allocationBlockSize = n;
    self->activeIterator = zero;
    self->slotCount = n;
    if (cond) {
        self->slots = appMallocUni__FUl(n * 4);
        self->slotMask = self->slotCount - 1;
        int i = 0;
        int off = 0;
        while (i < self->slotCount) {
            void* base = self->slots;
            i = i + 1;
            *(int*)((char*)base + off) = zero;
            off = off + 4;
        }
    } else {
        self->slots = (void*)zero;
    }
    OSUnlockMutex((OSMutex*)mtx);
    ((void (*)(NewObj*, int))(*(void***)self)[3])(self, 0x2200);
    self->glyphScale = kFloat_1_FixedFont;
    AsWFixedFont(self)->unk_64 = 0;
    self->color40 = 0x00FF0000;
    self->color44 = 0x0000FF00;
    self->color48 = 0xFF;
    self->baseValue34 = 1;
    self->vtable = (void**)__vt__10WFixedFont;
    AsWFixedFont(self)->borrowedResources = 0;
    // Initialize the remaining font mode/status controls separately from resource ownership.
    (*(int*)&AsWFixedFont(self)->unk_30) = 0;
    AsWFixedFont(self)->unk_A1 = 0;
    (*(int*)&AsWFixedFont(self)->resource) = 0;
    AsWFixedFont(self)->unk_A0 = 0;
    (*(int*)&AsWFixedFont(self)->messageBuffer) = 0;
    void* p = __nw__FUl(0x1C);
    if (p != 0)
        ((CAppFontResource*)p)->__ct();
    self->messageResource = p;
    p = __nw__FUl(0x4BC);
    if (p != 0)
        ((CMsgDraw*)p)->__ct(self->messageResource);
    int lbl0 = ((int&)Sing_805DC650());
    self->messageDraw = p;
    if (lbl0 == 0) {
        if (!s_pointerPairListInitialized) {
            PairVector_InitReserved(s_messageDrawPairList);
            __register_global_object(s_messageDrawPairList, (void*)MessageDrawPairList_Dtor,
                                     s_messageDrawListDtorRegistration);
            s_pointerPairListInitialized = 1;
        }
        ((int&)Sing_805DC650()) = (int)s_messageDrawPairList;
    }
    PointerIntVector_AppendPair((void*)((int&)Sing_805DC650()), self->messageDraw, 0);
    return self;
}
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma opt_common_subs reset
#pragma opt_propagation reset

// screenX/screenY and the resulting rectangle are in viewport pixels.
// UV arrays contain (u, v, width, height), not opposite-corner coordinates.
// c1/c2 are forwarded draw colors; idx must be a valid atlas glyph.
// Draw one atlas cell and return its scaled advance.
float WFont_DrawAtlasGlyph(Obj* self, Arg* arg, int idx, float screenX, float screenY, int c1, int c2) {
    float width;
    float heightScale;
    float uvOriginAndExtent[4];
    float advanceScale;
    int cellIndex = idx % self->glyphsPerImage;

    uvOriginAndExtent[0] = (float)(self->glyphWidth * (cellIndex % self->glyphsPerRow)) / (float)self->atlasWidth;
    uvOriginAndExtent[1] = (float)(self->glyphHeight * (cellIndex / self->glyphsPerRow)) / (float)self->atlasHeight;
    uvOriginAndExtent[2] = (float)self->glyphWidth / (float)self->atlasWidth;
    uvOriginAndExtent[3] = (float)self->glyphHeight / (float)self->atlasHeight;

    unsigned int flag = 0;
    if (arg != 0 && (self->flags & kScaleToViewport))
        flag = 1;

    if (flag) {
        float viewportWidthForHeight = arg->viewportWidth;
        heightScale = (self->glyphScale * viewportWidthForHeight) / kFloat_640_font;
    } else {
        heightScale = self->glyphScale;
    }
    flag = 0;

    float drawHeight = (float)self->glyphHeight * heightScale;
    if (arg != 0 && (self->flags & kScaleToViewport))
        flag = 1;
    float widthScale;
    if (flag) {
        float viewportWidthForWidth = arg->viewportWidth;
        widthScale = (self->glyphScale * viewportWidthForWidth) / kFloat_640_font;
    } else {
        widthScale = self->glyphScale;
    }

    float drawWidth = (float)self->glyphWidth * widthScale;
    float screenRect[4];
    screenRect[0] = screenX;
    screenRect[2] = drawWidth;
    screenRect[1] = screenY;
    screenRect[3] = drawHeight;

    Sprite_DrawClippedTexturedQuad(self, arg, ((void**)self->imageResources)[idx / self->glyphsPerImage],
                                   uvOriginAndExtent, screenRect, kFloat_0_Font, (unsigned int)c1, (unsigned int)c2, 0,
                                   1);

    flag = 0;
    if (0 != arg && (self->flags & kScaleToViewport))
        flag = 1;

    if (!(!(flag))) {
        float viewportWidthForAdvance = arg->viewportWidth;
        advanceScale = self->glyphScale * viewportWidthForAdvance / kFloat_640_font;
    } else {
        advanceScale = self->glyphScale;
    }
    width = ((float*)self->glyphAdvances[self->advanceMode])[idx];
    return width * advanceScale;
}

// p7/p8 are forwarded colors; p9 is unused. Input is a NUL-terminated byte string.
// Preserve the existing fallback: an unrecognized non-space byte advances by
// the preceding character's advance (zero initially). Space uses half a cell
// without glyph/viewport scaling here; the measurement routine scales its sum.
// Draw each recognized atlas character and return the horizontal distance advanced.
float WTitleFont_DrawString(Obj* self, Arg* arg, float screenX, float screenY, const char* chars, int p7, int p8,
                            int p9) {
    float initialX = screenX;
    float baselineY = screenY;
    float previousAdvance = kFloat_0_Font;
    float penX = initialX;
    float referenceViewportWidth = kFloat_640_font;
    float spaceWidthFactor = kFloat_0_5_FontSpacing;
    signed char c;
    const char* cp = chars;
    while ((c = *cp++) != 0) {
        char* base = (char*)self->characters;
        void* found = strchr(base, c);
        if (found != 0) {
            int idx = (char*)found - base;
            int cellIndex = idx % self->glyphsPerImage;
            float screenRect[4];
            float uvOriginAndExtent[4] = {
                (float)(self->glyphWidth * (cellIndex % self->glyphsPerRow)) / (float)self->atlasWidth,
                (float)(self->glyphHeight * (cellIndex / self->glyphsPerRow)) / (float)self->atlasHeight,
                (float)self->glyphWidth / (float)self->atlasWidth, (float)self->glyphHeight / (float)self->atlasHeight};
            int flag = 0;
            if (arg != 0 && (self->flags & kScaleToViewport))
                flag = 1;
            float scale;
            if (flag) {
                float t = arg->viewportWidth;
                scale = self->glyphScale;
                scale = scale * t / referenceViewportWidth;
            } else {
                scale = self->glyphScale;
            }
            float drawHeight = (float)self->glyphHeight * scale;
            flag = 0;
            if (arg != 0 && (self->flags & kScaleToViewport))
                flag = 1;
            if (flag) {
                float t = arg->viewportWidth;
                scale = self->glyphScale;
                scale = scale * t / referenceViewportWidth;
            } else {
                scale = self->glyphScale;
            }
            screenRect[0] = penX;
            screenRect[1] = baselineY;
            screenRect[2] = (float)self->glyphWidth * scale;
            screenRect[3] = drawHeight;
            typedef void (*Fn9)(void*, void*, void*, float*, float*, unsigned int, unsigned int, int, int, float);
            ((Fn9)Sprite_DrawClippedTexturedQuad)(self, arg, ((void**)self->imageResources)[idx / self->glyphsPerImage],
                                                  uvOriginAndExtent, screenRect, (unsigned int)p7, (unsigned int)p8, 0,
                                                  1, kFloat_0_Font);
            flag = 0;
            if (arg != 0 && (self->flags & kScaleToViewport))
                flag = 1;
            if (flag) {
                float t = arg->viewportWidth;
                scale = self->glyphScale;
                scale = scale * t / referenceViewportWidth;
            } else {
                scale = self->glyphScale;
            }
            previousAdvance = ((float*)self->glyphAdvances[self->advanceMode])[idx] * scale;
        } else if (c == kSpaceByte) {
            previousAdvance = spaceWidthFactor * (float)self->glyphWidth;
        }
        penX = penX + previousAdvance;
    }
    return penX - initialX;
}
} // extern "C"
