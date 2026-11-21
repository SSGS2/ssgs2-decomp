// gtl bitmap font setup and glyph geometry; also exports animation-key searches.
// Mod/API lookup:
// - Font file versus built-in font: LoadResourceBuffer; Initialize publishes the
//   shared header, texture pages and vertex buffer. Font view parsing is provided
//   by gtl_FontData_InitView/CopyHeader/GetRecord (gtl_image_font_resource_adapters.cpp).
// - Glyph position/UV/color: WriteGlyphQuad, GlyphRect and GlyphVertex* schemas.
//   UV inputs are texels; output is normalized by the texture-page dimensions.
// - Texture pages: LoadTexturePages; external filenames versus built-in 4-bit
//   texels/palette. Format/stride consumers are in gtl_texture_vertex_formats.cpp; tiled RGBA upload is
//   Texture_ConvertRGBA8ToTiledARGBPlanes. Keep dimensions, format and bytes/texel
//   together; allocation sizes are bytes, page/animation counts are entries.
// - Animation search: LowerBoundKey{96,64,48,32,16,8}, using the time offsets
//   below; empty input returns last and equal keys return the first equal key.
// Global companions: s_bitmapFontResourceBuffer owns the resource buffer, s_bitmapFontVertexBuffer the
// vertex buffer; s_bitmapFontState stores page handles/descriptions and copied header;
// s_builtinFontMetadata is built-in metadata, s_builtinFontTexels texels, s_builtinFontPalette palette.
// These are process-wide font resources: caller serializes setup and drawing.
// Translation unit 0x803E4220-0x803E56D8 (10 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/stdexcept_local.h"
#include "sdk_protos.h"

struct CFileLocal {
    char data[0x18];
    ~CFileLocal();
};

// Animation keys: a key of N bytes whose time is the float at TimeOffset.
template <int Size, int TimeOffset> struct AnimationKey {
    char before[TimeOffset];
    f32 time;
    char after[Size - TimeOffset - 4];
};

// Compare stored key times without converting precision or units.
template <class Key> static inline bool AnimationKey_NotAfter(const Key& a, const Key& b) { return a.time <= b.time; }

// std::lower_bound over keys by time: the first key whose time is not before `key`'s.
// Borrowed [first,last) is sorted ascending by finite stored float times; key is
// required. Does not write any key or allocate. Units remain those of this track.
template <class Key> static inline Key* AnimationKey_LowerBound(Key* first, Key* last, const Key* key) {
    int count = last - first;
    while (count > 0) {
        Key* middle;
        int half;
        half = count / 2;
        middle = first + half;
        if (!AnimationKey_NotAfter(*key, *middle)) {
            first = middle + 1;
            count -= half + 1;
        } else {
            count = half;
        }
    }
    return first;
}

typedef AnimationKey<0x60, 0x50> AnimationKey96;
typedef AnimationKey<0x40, 0x3C> AnimationKey64;
typedef AnimationKey<0x30, 0x28> AnimationKey48;
typedef AnimationKey<0x20, 0x14> AnimationKey32;
typedef AnimationKey<0x10, 0xC> AnimationKey16;
typedef AnimationKey<0x8, 0x4> AnimationKey8;

extern "C" {

void __ct__9CFileCtorFPCcUi(void* self, const char* name, unsigned int mode);
s32 gtl_CFile_GetFileSize(void* self);
void gtl_CFile_Read(void* self, void* buffer, u32 length);
void gtl_CFile_dtor(void* self, s32 flags);
void gtl_IG3DXBuffer_Create(s32 size, void** out, s32 flags);
void* Object_GetField8(void* obj);
u32 Object_GetFieldC(void* obj);
extern void* s_bitmapFontResourceBuffer;
extern u8 s_builtinFontMetadata[];
extern float lbl_805E20F0;
void gtl_PixelCopy_CopyPackedColor(void* dst, void* src);
void __dl__FPv(void* p);
void FontDataView_dtor(void* self, s32 flags);
void* memset(void* dst, s32 value, u32 size);
void gtl_FontData_InitView(void* self, void* buffer);
void gtl_FontData_CopyHeader(void* self, void* src);
void gtl_BitmapFont_LoadTexturePages(void* self, const char* name);
void gtl_FontData_GetRecord(void* self, s32 index, void* out);
void Texture_Create2D(s32 width, s32 height, s32 mips, s32 a, u32 format, s32 b, void* out, s32 c);
void Texture_GetLevelDescription(void* tex, s32 level, void* desc);
void TexturePalette_GetDescription(void* palette, void* desc);
void ImageResource_LoadFileAndDecode(void* name, void* tex, void* palette, void* out, s32 flags);
s32 TextureFormat_GetRGBA8OrIA8BytesPerTexel(u32 format);
void Texture_LockLevel(void* tex, s32 level, void* out, s32 a, s32 b);
void Texture_ConvertRGBA8ToTiledARGBPlanes(void* src, void* dst, s32 a, s32 pitch, s32 w, s32 h, s32 b, s32 bpt);
void Texture_UnlockLevel(void* tex, s32 level);
void gtl_PixelCopy_Copy4Bytes_1(void* dst, void* src);
void* __nw__FUl(u32 size);
void* memcpy(void* dst, const void* src, u32 size);
extern u8 s_bitmapFontState[];
extern u32 s_builtinFontTexels[];
extern u32 s_builtinFontPalette[];
extern char s_vectorLengthErrorText[];
void VertexBuffer_Create(s32 size, s32 a, s32 format, s32 b, void** out, s32 c);
void StdString_ReserveCapacity(void* value, u32 size);
void StdString_ReplaceRange(void* self, u32 position, u32 eraseCount, const char* first, const char* last, u8* alloc);
void FileIO_SplitPath(const char* path, char* drive, char* dir, char* stem, char* ext);
extern u8 s_bitmapFontHeaderStorage[];
extern u8 s_bitmapFontPageBatches[];
extern void* s_bitmapFontVertexBuffer;
extern s32 s_bitmapFontInitialized;
extern char __vt__14filenamestring[];

// Close the local file even when resource loading unwinds; leave stack storage.
inline CFileLocal::~CFileLocal() { gtl_CFile_dtor(this, -1); }

// Preserve the unsigned dimension-to-binary32 conversion used in UV division.
static inline f32 gtl_U32ToF32(u32 v) { return (f32)v; }

struct NameField {
    u32 flags;
    u32 heapLen;
    char* heapPtr;
};

// Establish an empty inline narrow string; does not free an existing allocation.
static inline void NameField_Clear(NameField* f) {
    f->flags = 0;
    f->heapLen = 0;
    f->heapPtr = 0;
}

// Read byte length from the inline tag or long-string length word.
static inline u32 NameField_Len(NameField* f) { return (f->flags >> 31) == 0 ? (*(u8*)f & 0x7F) : f->heapLen; }

// Borrow NUL-terminated narrow text; invalidated by mutation/destruction of f.
static inline char* NameField_Data(NameField* f) { return (f->flags >> 31) == 0 ? (char*)f + 1 : f->heapPtr; }

// Return borrowed byte range and length through d/l for either string form.
static inline void NameField_Range(NameField* f, char*& d, u32& l) {
    if ((f->flags >> 31) == 0) {
        l = *(u8*)f & 0x7F;
        d = (char*)f + 1;
    } else {
        d = f->heapPtr;
        l = f->heapLen;
    }
}

// Release heap text only; inline text is embedded and requires no free.
static inline void NameField_Free(NameField* f) {
    if (f->flags >> 31) __dl__FPv(f->heapPtr);
}

struct NameStr {
    NameField input;
    NameField drive;
    NameField directory;
    NameField stem;
    NameField extension;
    NameField driveDirectory;
    NameField unk48;
    NameField fileName;
    void* vtable;
    ~NameStr();
};

// Destroy filename components in reverse member order, releasing heap strings.
inline NameStr::~NameStr() {
    NameField_Free(&fileName);
    NameField_Free(&unk48);
    NameField_Free(&driveDirectory);
    NameField_Free(&extension);
    NameField_Free(&stem);
    NameField_Free(&directory);
    NameField_Free(&drive);
    NameField_Free(&input);
}

struct FontDataLocal {
    char data[0x10];
    ~FontDataLocal();
};

// Release the temporary font view's references; leave the shared font resource.
inline FontDataLocal::~FontDataLocal() { FontDataView_dtor(this, -1); }

#pragma opt_propagation off
// Find the first key >= key.time in the borrowed sorted range; see common contract.
AnimationKey96* gtl_Animation_LowerBoundKey96(AnimationKey96* first, AnimationKey96* last, const AnimationKey96* key) {
    return AnimationKey_LowerBound(first, last, key);
}

// Same lower-bound contract for 64-byte records, time at +0x3C.
AnimationKey64* gtl_Animation_LowerBoundKey64(AnimationKey64* first, AnimationKey64* last, const AnimationKey64* key) {
    return AnimationKey_LowerBound(first, last, key);
}

// Same lower-bound contract for 48-byte records, time at +0x28.
AnimationKey48* gtl_Animation_LowerBoundKey48(AnimationKey48* first, AnimationKey48* last, const AnimationKey48* key) {
    return AnimationKey_LowerBound(first, last, key);
}

// Same lower-bound contract for 32-byte records, time at +0x14.
AnimationKey32* gtl_Animation_LowerBoundKey32(AnimationKey32* first, AnimationKey32* last, const AnimationKey32* key) {
    return AnimationKey_LowerBound(first, last, key);
}

// Same lower-bound contract for 16-byte records, time at +0xC.
AnimationKey16* gtl_Animation_LowerBoundKey16(AnimationKey16* first, AnimationKey16* last, const AnimationKey16* key) {
    return AnimationKey_LowerBound(first, last, key);
}

// Same lower-bound contract for 8-byte records, time at +4.
AnimationKey8* gtl_Animation_LowerBoundKey8(AnimationKey8* first, AnimationKey8* last, const AnimationKey8* key) {
    return AnimationKey_LowerBound(first, last, key);
}

// Publish an owned IG3DXBuffer in s_bitmapFontResourceBuffer. pathAddress is the Wii ABI's
// integer-encoded borrowed NUL-terminated filename; zero selects built-in data.
// External path: open mode 2, allocate file-size bytes, read the buffer's length.
// No open/read/create status is checked, and no previous global buffer is released
// here: caller must arrange initial/retired ownership before setup/reload. Local
// CFile closes on return/unwind; the global buffer survives for font-view use.
// Built-in conversion region: offset 0x48 followed by 0x58 blocks of 16 bytes,
// ending at 0x5C8. In each block, reverse the TWO halfwords of its four words;
// this is not a full byte swap. External files bypass that conversion and must
// already be accepted by FontData. This routine checks no magic/version/length.
// Failure is unchecked/exceptional, with no transactional restoration of global
// state; a return proves neither a valid font nor successful file I/O.
void gtl_BitmapFont_LoadResourceBuffer(s32 pathAddress) {
    if (pathAddress != 0) {
        CFileLocal file;
        __ct__9CFileCtorFPCcUi(&file, (const char*)pathAddress, 2);
        s32 size = gtl_CFile_GetFileSize(&file);
        gtl_IG3DXBuffer_Create(size, &s_bitmapFontResourceBuffer, 0);
        u32 len = Object_GetFieldC(s_bitmapFontResourceBuffer);
        gtl_CFile_Read(&file, Object_GetField8(s_bitmapFontResourceBuffer), len);
        return;
    }
    gtl_IG3DXBuffer_Create(0x5C8, &s_bitmapFontResourceBuffer, 0);
    u8* base = (u8*)Object_GetField8(s_bitmapFontResourceBuffer);
    Memory_Copy(base, s_builtinFontMetadata, 0x5C8);
    u16 t;
    u16* hi;
    u16* lo;
    u8* p;
    p = base + 0x48;
    for (s32 i = 0; i < 0x58; i++) {
        lo = (u16*)p;
        hi = (u16*)(p + 4);
        while (lo < --hi) {
            t = *lo;
            *lo++ = *hi;
            *hi = t;
        }
        hi = (u16*)(p + 8);
        lo = (u16*)(p += 4);
        while (lo < --hi) {
            t = *lo;
            *lo++ = *hi;
            *hi = t;
        }
        hi = (u16*)(p + 8);
        lo = (u16*)(p += 4);
        while (lo < --hi) {
            t = *lo;
            *lo++ = *hi;
            *hi = t;
        }
        hi = (u16*)(p + 8);
        lo = (u16*)(p += 4);
        while (lo < --hi) {
            t = *lo;
            *lo++ = *hi;
            *hi = t;
        }
        p += 4;
    }
}
#pragma opt_propagation reset

struct GlyphRect {
    f32 x0, y0, x1, y1;
    // Snapshot a borrowed rectangle before any output writes; order is significant.
    GlyphRect(const GlyphRect& o) {
        x0 = o.x0;
        y0 = o.y0;
        x1 = o.x1;
        y1 = o.y1;
    }
};

// Native vertex/descriptor schemas (four-byte alignment, not serialized pointers).
struct GlyphVertexColorUv { f32 x, y, z; u32 color; f32 u, v; }; // 24 bytes
struct GlyphVertexUv { f32 x, y, z, u, v; };                  // 20 bytes
struct GlyphPageDimensions { char reserved[0x1C]; u32 width, height; };
enum GlyphOutputFormat {
    kGlyphXyzColorUv = 0x11F005, kGlyphXyzUv = 0x110005,
    kGlyphTriangleStrip = 0x98
};
// Write four strip vertices in (x0,y0), (x1,y0), (x0,y1), (x1,y1) order.
// Required disjoint writable vertices: 96 bytes with color, 80 without;
// pageDescription and both rectangle pointers required for supported formats.
// Positive page dimensions are texels. uvRect is in texels; positionRect and z
// are passed through in the caller's drawing coordinate space (no projection).
// Packed color is converted by PixelCopy only in the colored layout. Its UVs
// add 0.5 texel before normalization; uncolored UVs have no half-texel bias.
// unusedMetadata is an ignored ABI pointer. Unsupported primitive/format leaves
// output untouched; void return has no status. No ownership transfer/allocation.
// The float-word stores below implement these two schemas in the original order;
// replacing them with whole-vertex assignments changes store scheduling. In the
// colored format words 3/9/15/21 are color bits, never floating-point values.
void gtl_BitmapFont_WriteGlyphQuad(void* vertices, s32 vertexFormat, s32 primitive, void* pageDescription, void* unusedMetadata, void* uvRect, void* positionRect, s32 packedColor, f32 z) {
    s32 colorCopy = packedColor;
    f32* d = (f32*)vertices;
    GlyphPageDimensions* m = (GlyphPageDimensions*)pageDescription;
    if (primitive != kGlyphTriangleStrip) {
        return;
    }
    switch (vertexFormat) {
    case kGlyphXyzColorUv: {
        GlyphRect pos(*(GlyphRect*)positionRect);
        GlyphRect uv(*(GlyphRect*)uvRect);
        d[0] = pos.x0;
        d[1] = pos.y0;
        d[2] = z;
        gtl_PixelCopy_CopyPackedColor(d + 3, &colorCopy);
        d[4] = uv.x0;
        d[5] = uv.y0;
        d[6] = pos.x1;
        d[7] = pos.y0;
        d[8] = z;
        gtl_PixelCopy_CopyPackedColor(d + 9, &colorCopy);
        d[10] = uv.x1;
        d[11] = uv.y0;
        d[12] = pos.x0;
        d[13] = pos.y1;
        d[14] = z;
        gtl_PixelCopy_CopyPackedColor(d + 15, &colorCopy);
        d[16] = uv.x0;
        d[17] = uv.y1;
        d[18] = pos.x1;
        d[19] = pos.y1;
        d[20] = z;
        gtl_PixelCopy_CopyPackedColor(d + 21, &colorCopy);
        u32 pageHeight = m->height;
        u32 pageWidth = m->width;
        d[4] = (0.5f + d[4]) / gtl_U32ToF32(pageWidth);
        d[5] = (0.5f + d[5]) / gtl_U32ToF32(pageHeight);
        d[10] = (0.5f + d[10]) / gtl_U32ToF32(pageWidth);
        d[11] = (0.5f + d[11]) / gtl_U32ToF32(pageHeight);
        d[16] = (0.5f + d[16]) / gtl_U32ToF32(pageWidth);
        d[17] = (0.5f + d[17]) / gtl_U32ToF32(pageHeight);
        d[22] = (0.5f + uv.x1) / gtl_U32ToF32(pageWidth);
        d[23] = (0.5f + uv.y1) / gtl_U32ToF32(pageHeight);
        return;
    }
    case kGlyphXyzUv: {
        u32 pageHeight = m->height;
        u32 pageWidth = m->width;
        GlyphRect pos2(*(GlyphRect*)positionRect);
        GlyphRect uv2(*(GlyphRect*)uvRect);
        d[3] = uv2.x0;
        d[4] = uv2.y0;
        d[2] = z;
        d[7] = z;
        d[12] = z;
        d[17] = z;
        d[8] = uv2.x1;
        d[9] = uv2.y0;
        d[13] = uv2.x0;
        d[14] = uv2.y1;
        d[18] = uv2.x1;
        d[19] = uv2.y1;
        d[0] = pos2.x0;
        d[1] = pos2.y0;
        d[5] = pos2.x1;
        d[6] = pos2.y0;
        d[10] = pos2.x0;
        d[11] = pos2.y1;
        d[15] = pos2.x1;
        d[16] = pos2.y1;
        d[3] = uv2.x0 / gtl_U32ToF32(pageWidth);
        d[4] = uv2.y0 / gtl_U32ToF32(pageHeight);
        d[8] = uv2.x1 / gtl_U32ToF32(pageWidth);
        d[9] = uv2.y0 / gtl_U32ToF32(pageHeight);
        d[13] = uv2.x0 / gtl_U32ToF32(pageWidth);
        d[14] = uv2.y1 / gtl_U32ToF32(pageHeight);
        d[18] = uv2.x1 / gtl_U32ToF32(pageWidth);
        d[19] = uv2.y1 / gtl_U32ToF32(pageHeight);
        return;
    }
    }
}

#pragma opt_unroll_loops off
// Clear one 260-byte path buffer; expanded paired stores preserve its word order.
static inline void gtl_ZeroBuf104(char* buf) {
    u32* p = (u32*)buf - 1;
    u32 n = 0x20;
    do {
        p[1] = 0;
        p[2] = 0;
        p += 2;
    } while (--n);
    p[1] = 0;
}

// Set up the shared font from an optional borrowed filename (null = built-in).
// Path buffers require components/combined directory to fit 0x104 bytes including
// NUL; no truncation/bounds validation. Temporary NameStr owns component strings,
// FontDataLocal retains its view until return. Globals remain owned by the font
// subsystem; this is setup, not an atomic hot-reload/release API. No failure status
// is checked. Initialized latch is set after texture/buffer calls, not proof that
// every page load succeeded. Volatile tag-byte pairs model allocator ABI scratch,
// not user settings; preserve their reads and order when editing path assembly.
void gtl_BitmapFont_Initialize(const char* arg0) {
    volatile u32 d1, s1, s2, d2, s3, d3, s4, d4, s5, d5, s6, d6, s7, d7, s8, d8, s9, d9;
    char bufE[0x104];
    FontDataLocal fd;
    char* name;
    gtl_BitmapFont_LoadResourceBuffer((s32)arg0);
    gtl_FontData_InitView(&fd, s_bitmapFontResourceBuffer);
    gtl_FontData_CopyHeader(&fd, s_bitmapFontHeaderStorage);
    name = 0;
    gtl_ZeroBuf104(bufE);
    if (arg0 != 0) {
        NameStr str;
        NameField_Clear(&str.input);
        u32 len = strlen(arg0);
        u32 n;
        StdString_ReserveCapacity(&str.input, len);
        *(u8*)&d1 = *(volatile u8*)&s1;
        StdString_ReplaceRange(&str.input, 0, 0, arg0, arg0 + len, (u8*)&d1);
        str.drive.flags = 0;
        str.vtable = __vt__14filenamestring;
        str.drive.heapLen = 0;
        str.drive.heapPtr = 0;
        NameField_Clear(&str.directory);
        NameField_Clear(&str.stem);
        NameField_Clear(&str.extension);
        NameField_Clear(&str.driveDirectory);
        NameField_Clear(&str.unk48);
        NameField_Clear(&str.fileName);
        char bufA[0x104];
        char bufB[0x104];
        char bufC[0x104];
        char bufD[0x104];
        gtl_ZeroBuf104(bufA);
        gtl_ZeroBuf104(bufB);
        gtl_ZeroBuf104(bufC);
        gtl_ZeroBuf104(bufD);
        FileIO_SplitPath(NameField_Data(&str.input), bufA, bufB, bufC, bufD);

        len = NameField_Len(&str.drive);
        *(u8*)&d2 = *(volatile u8*)&s2;
        n = strlen(bufA);
        StdString_ReplaceRange(&str.drive, 0, len, bufA, bufA + n, (u8*)&d2);

        len = NameField_Len(&str.directory);
        *(u8*)&d3 = *(volatile u8*)&s3;
        n = strlen(bufB);
        StdString_ReplaceRange(&str.directory, 0, len, bufB, bufB + n, (u8*)&d3);

        len = NameField_Len(&str.stem);
        *(u8*)&d4 = *(volatile u8*)&s4;
        n = strlen(bufC);
        StdString_ReplaceRange(&str.stem, 0, len, bufC, bufC + n, (u8*)&d4);

        len = NameField_Len(&str.extension);
        *(u8*)&d5 = *(volatile u8*)&s5;
        n = strlen(bufD);
        StdString_ReplaceRange(&str.extension, 0, len, bufD, bufD + n, (u8*)&d5);

        s32 sh = !(str.drive.flags >> 31);
        if ((sh ? (*(u8*)&str.drive & 0x7F) : str.drive.heapLen) != 0) {
            u32 l2 = sh ? (*(u8*)&str.drive & 0x7F) : str.drive.heapLen;
            u32 idx = l2 - 1;
            char* dp = sh ? (char*)&str.drive + 1 : str.drive.heapPtr;
            dp[idx] = 0;
        }

        len = NameField_Len(&str.driveDirectory);
        *(u8*)&d6 = *(volatile u8*)&s6;
        n = strlen(bufA);
        StdString_ReplaceRange(&str.driveDirectory, 0, len, bufA, bufA + n, (u8*)&d6);

        len = NameField_Len(&str.driveDirectory);
        *(u8*)&d7 = *(volatile u8*)&s7;
        n = strlen(bufB);
        StdString_ReplaceRange(&str.driveDirectory, len, 0, bufB, bufB + n, (u8*)&d7);

        if (!(str.fileName.flags >> 31) && !(str.stem.flags >> 31)) {
            str.fileName.flags = str.stem.flags;
            str.fileName.heapLen = str.stem.heapLen;
            str.fileName.heapPtr = str.stem.heapPtr;
        } else {
            len = (str.fileName.flags >> 31) ? str.fileName.heapLen : (*(u8*)&str.fileName & 0x7F);
            char* sd;
            NameField_Range(&str.stem, sd, n);
            *(u8*)&d8 = *(volatile u8*)&s8;
            StdString_ReplaceRange(&str.fileName, 0, len, sd, sd + n, (u8*)&d8);
        }

        len = NameField_Len(&str.fileName);
        char* ed;
        NameField_Range(&str.extension, ed, n);
        *(u8*)&d9 = *(volatile u8*)&s9;
        StdString_ReplaceRange(&str.fileName, len, 0, ed, ed + n, (u8*)&d9);

        strcpy(bufE, NameField_Data(&str.driveDirectory));
        name = bufE;
    } else {
        name = 0;
    }
    gtl_BitmapFont_LoadTexturePages(&fd, name);
    VertexBuffer_Create(0x48000, 0, 0x11F005, 0, (void**)&s_bitmapFontVertexBuffer, 0);
    memset(s_bitmapFontPageBatches, 0, 0x4000);
    s_bitmapFontInitialized = 1;
}
#pragma opt_unroll_loops reset

struct FontRecord {
    char namePrefix[0x18]; // Prefix also used as page filename when fetched to recName.
    s32 texWidth;    // +0x18: texels
    s32 texHeight;   // +0x1C: texels
};

struct TexLevelDesc {
    u32 format;
    char reserved04[0x18];
    u32 width;
    u32 height;
};

struct ByteVector {
    u8* data;
    u32 size;
    u32 capacity;
};

struct TexLock {
    void* data; // Borrowed writable level storage, valid until UnlockLevel.
    u32 pitch;  // Byte pitch supplied by lock; pass unchanged to tiled converter.
};

struct TexLoadFrame {
    TexLock lock;
    char reserved08[0x18];
    ByteVector vec;
    NameField s;
    char unk38[0x18];
    char recName[0x20];
    FontRecord rec;
    TexLevelDesc lvl;
    u32 imgInfo[4];
};

enum FontPageStorage {
    kFontPaletteSlotsOffset = 0x4050,
    kFontCopiedHeaderOffset = 0x40A0,
    kFontTextureDescriptionsOffset = 0x40C8,
    kFontPaletteDescriptionsOffset = 0x4398,
    kFontTextureDescriptionBytes = 0x24,
    kFontPaletteDescriptionBytes = 0x14,
    kFontPlatformRgba8 = 0x80000006,
    kBuiltinFontPackedWords = 0x1000,
    kExpandedBytesPerPackedWord = 0x20
};
// Populate shared page handles and descriptions. Required self is a live FontData
// view backed by the resource buffer; name is a borrowed NUL-terminated directory
// prefix (already includes separators), null selects the built-in atlas.
// Global byte layout relative to s_bitmapFontState: texture slots +0, palette slots
// +4050, copied font header +40A0 (page count at header +C), texture descriptors
// +40C8 (36 bytes each), palette descriptors +4398 (20 bytes each). Each handle
// is a Wii 4-byte pointer. Page count must fit all these preallocated regions;
// this function does not check their limits. GetRecord writes one 32-byte page
// record, whose +18/+1C fields are width/height in texels; external filename starts
// at +0 and must be NUL terminated within its 24-byte prefix. Keep schema aligned
// with FontData_GetRecord and CopyHeader (gtl_image_font_resource_adapters.cpp) for a different font format.
// That view uses a 0x28-byte header, count at +C, and 32-byte records at +0x28;
// its glyph lookup separately uses header offsets +0x20/+0x24 (gtl_image_font_resource_adapters.cpp). The built-
// in swap region described at LoadResourceBuffer is NOT the page-record schema.
// External path: concatenate directory+record name, decode into owned texture/
// palette slots, then describe nonnull handles. A null decoded texture/palette is
// tolerated and its old description stays untouched. No success result/rollback.
// Built-in path: create one platform RGBA8 texture (0x80000006 => 4 bytes/texel,
// gtl_texture_vertex_formats.cpp). Decode 4096 packed words: reverse each word's four bytes, then expand
// each byte's LOW nibble followed by HIGH nibble through the 16-entry RGBA palette.
// Each word emits 8 texels/32 bytes => 0x20000 bytes total. The returned texture
// dimensions/format must provide at least that much temporary storage. bpt==0 or
// a smaller allocation is unchecked; those are invalid inputs, not empty-font
// success. Tiled upload uses lock.pitch, then unlocks and frees temporary bytes.
// No previous global handles are released here. Caller owns subsystem lifecycle,
// serializes rendering/setup, and supplies successful create/lock allocations.
// There is no RAII guard for the temporary byte vector or lock on exceptions;
// no promise of cleanup on decoder/converter/allocation failure is made here.
#pragma use_lmw_stmw on
void gtl_BitmapFont_LoadTexturePages(void* self, const char* name) {
    u8 tag10;
    u8 t14;
    u8 t18;
    u8 tag1C;
    TexLoadFrame f;
    u8* base = s_bitmapFontState;
    gtl_FontData_GetRecord(self, 0, &f.rec);
    if (name != 0) {
        u8* cnt = base + kFontCopiedHeaderOffset;
        if (*(s32*)(cnt + 0xC) > 0) {
            u8 t1C = tag1C;
            u8 t10 = tag10;
            void** palSlot = (void**)(base + kFontPaletteSlotsOffset);
            void** texSlot = (void**)base;
            u8* texDesc = base + kFontTextureDescriptionsOffset;
            u8* palDesc = base + kFontPaletteDescriptionsOffset;
            for (s32 i = 0; i < *(s32*)(cnt + 0xC); i++) {
                gtl_FontData_GetRecord(self, i, f.recName);
                NameField& s = f.s;
                NameField_Clear(&s);
                u32 n = strlen(name);
                StdString_ReserveCapacity(&s, n);
                t18 = t1C;
                StdString_ReplaceRange(&s, 0, 0, name, name + n, &t18);
                u32 pos = NameField_Len(&s);
                t14 = t10;
                u32 m = strlen(f.recName);
                StdString_ReplaceRange(&s, pos, 0, f.recName, f.recName + m, &t14);
                f.imgInfo[2] = 0;
                f.imgInfo[3] = 0;
                ImageResource_LoadFileAndDecode(NameField_Data(&s), texSlot, palSlot, f.imgInfo, 0);
                if (*texSlot != 0) {
                    Texture_GetLevelDescription(*texSlot, 0, texDesc);
                }
                if (*palSlot != 0) {
                    TexturePalette_GetDescription(*palSlot, palDesc);
                }
                NameField_Free(&s);
                texSlot++;
                palSlot++;
                texDesc += kFontTextureDescriptionBytes;
                palDesc += kFontPaletteDescriptionBytes;
            }
        }
    } else {
        Texture_Create2D(f.rec.texWidth, f.rec.texHeight, 1, 0, kFontPlatformRgba8, 0, base, 0);
        Texture_GetLevelDescription(*(void**)base, 0, base + kFontTextureDescriptionsOffset);
        TexLevelDesc& lvl = f.lvl;
        Texture_GetLevelDescription(*(void**)base, 0, &lvl);
        s32 bpt = TextureFormat_GetRGBA8OrIA8BytesPerTexel(lvl.format);
        u32 total = bpt * lvl.width * lvl.height;
        ByteVector& vec = f.vec;
        vec.data = 0;
        vec.size = 0;
        vec.capacity = 0;
        if (total > 0) {
            if (vec.size + total > 0xFFFFFFFFU - vec.capacity) {
                std::length_error err(s_vectorLengthErrorText, s_vectorLengthErrorText + 0x14);
            }
            if (vec.capacity < 0x55555555U) {
            } else if (vec.capacity < 0xAAAAAAAAU) {
            }
            u32 needed = (vec.size + total) - vec.capacity;
            if (needed > 0xFFFFFFFFU - vec.capacity) {
                std::length_error err(s_vectorLengthErrorText, s_vectorLengthErrorText + 0x14);
            }
            u32 cap = vec.capacity;
            u32 newCap;
            if (cap < 0x55555555U) {
                u32 growth = ((cap + 1) * 4 - (cap + 1)) / 5;
                newCap = cap + (growth < needed ? needed : growth);
            } else if (cap < 0xAAAAAAAAU) {
                u32 growth = (cap + 1) >> 1;
                newCap = cap + (growth < needed ? needed : growth);
            } else {
                newCap = 0xFFFFFFFFU;
            }
            u8* newData = (u8*)__nw__FUl(newCap);
            memset(newData + vec.size, 0, total);
            memcpy(newData, vec.data, vec.size);
            memset(vec.data, 0, vec.size);
            u8* oldData = vec.data;
            vec.data = newData;
            vec.capacity = newCap;
            vec.size = vec.size + total;
            if (oldData != 0) {
                __dl__FPv(oldData);
            }
        }
        u32 word;
        for (u32 i = 0; i < kBuiltinFontPackedWords; i++) {
            word = s_builtinFontTexels[i];
            u8* lo = (u8*)&word;
            u8* hi = lo + 4;
            while (lo < --hi) {
                u8 t = *lo;
                *lo++ = *hi;
                *hi = t;
            }
            u8* bytes = (u8*)&word;
            u8* dst = vec.data + i * kExpandedBytesPerPackedWord;
            for (s32 k = 0; k < 4; k++) {
                u8* at = dst;
                dst += 4;
                gtl_PixelCopy_Copy4Bytes_1(at, (u8*)s_builtinFontPalette + ((bytes[k] & 0xF) << 2));
                at = dst;
                dst += 4;
                gtl_PixelCopy_Copy4Bytes_1(at, (u8*)s_builtinFontPalette + ((bytes[k] & 0xF0) >> 2));
            }
        }
        TexLock& lock = f.lock;
        Texture_LockLevel(*(void**)base, 0, &lock, 0, 0);
        Texture_ConvertRGBA8ToTiledARGBPlanes(vec.data, lock.data, 0, lock.pitch, lvl.width, lvl.height, 0, bpt);
        Texture_UnlockLevel(*(void**)base, 0);
        if (vec.data != 0) {
            vec.size = 0;
            __dl__FPv(vec.data);
        }
    }
}

} // extern "C"
