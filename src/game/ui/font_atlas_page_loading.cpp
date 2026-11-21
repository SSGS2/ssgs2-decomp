#include "game/string.h"
#include "classes/FontAtlas.h"

// Translation unit 0x800A7CA4-0x800A7E64 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

// Private ABI storage: both Obj types hold a narrow string; HeapObj is a font-data view.
// Keep the separate string types and the explicit local aggregate layout for code generation.
struct Obj16 { unsigned char data[12]; };
struct Obj10 { unsigned char data[12]; };
struct HeapObj { unsigned char data[16]; HeapObj(void*); };
// Header +0x0C is the number of atlas pages, +0x10 is the copied line height.
struct InitResult { unsigned char pad[12]; int count; int lineHeight; unsigned char tail[16]; };

HeapObj* gtl_FontData_InitView(HeapObj*, void*);
void gtl_FontData_CopyHeader(HeapObj*, void*);
void gtl_FontData_GetRecord(HeapObj*, int, void*);
int CHashPath_FindPath(void*);
void MWString_ConstructCString(Obj16*, int);
int MWString_ReverseFindCString(Obj16*, const char*, int);
void MWString_AssignCString(Obj16*, const char*);
int MWString_GetLength(void*);
void MWString_EraseRange(Obj16*, const char*, int);
void MWString_CopyConstruct(Obj10*, Obj16*);
void MWString_AppendCString(Obj10*, void*);
void FontAtlasPage_InitEmpty(void*);
int MWString_FindCString(Obj10*, const char*, int);
int MWString_GetCString(Obj10*);
// Shared declaration mismatch: this call site passes (atlas, path address, page output).
// The definition in party_goods_item_display.cpp uses (texture, stream, format); do not infer a stream here.
void Texture_CreateFromStream(void*, int, void*);
void ImageResource_LoadFileAndDecode(int, void*, void*, void*, void*);
void FontAtlasPageVector_PushBack(void*, void*);

extern const char texturePathStrings[];

// Bind the font reader to the atlas's existing buffer (no buffer copy here).
inline HeapObj::HeapObj(void* arg) { gtl_FontData_InitView(this, arg); }

#pragma opt_loop_invariants off
// Build a font-data view and append each referenced page using paths relative to the font file.
// self/fontBuffer and arg (a path object resolved by CHashPath_FindPath) must be valid.
// The atlas retains the allocated reader and appended page handles; temporary strings are scoped.
// Existing pages are not cleared. Returns 1 without checking individual load results.
int FontAtlas_LoadPages(void* self, void* arg) {
    struct Locals {
        unsigned char item[4]; // Loaded page handle passed to the page vector.
        unsigned char a[4]; // Secondary image output (palette in the shared decoder).
        Obj10 part; // Directory plus this page's filename.
        Obj16 text; // Directory prefix, including its trailing slash when present.
        unsigned char smallBuf[0x18]; // Secondary decoder metadata.
        unsigned char attr[0x20]; // Page record; leading bytes form the filename.
        unsigned char bigBuf[0x28]; // Primary decoder metadata.
        InitResult result; // Font header copy.
    } local;
    // Publish the reader and line height before loading the referenced pages.
    HeapObj* fontReader = new HeapObj(AsFontAtlas(self)->fontBuffer);
    AsFontAtlas(self)->fontData = fontReader;
    gtl_FontData_CopyHeader(fontReader, &local.result);
    AsFontAtlas(self)->lineHeight = local.result.lineHeight;

    // Retain the directory through the last slash, or use an empty relative prefix.
    MWString_ConstructCString(&local.text, CHashPath_FindPath(arg));
    struct StringCleanup {
        NString* string;
        // Destroy the temporary narrow string on normal exit and exception unwinding.
        ~StringCleanup() { string->~NString(); }
    } cleanupText = {(NString*)&local.text};
    enum {
        kPathSeparatorOffset = 0x4b, // "/"
        kEmptyDirectoryOffset = 0x4d, // ""
        kTgaSuffixOffset = 0x4e, // "tga" (no dot)
        kExtensionSearchTailBytes = 4,
        kSearchFromEnd = -1
    };
    unsigned int slashPosition = MWString_ReverseFindCString(&local.text, texturePathStrings + kPathSeparatorOffset, kSearchFromEnd);
    if (slashPosition == -1) {
        MWString_AssignCString(&local.text, texturePathStrings + kEmptyDirectoryOffset);
    } else {
        // EraseRange's pointer-shaped second argument carries a character index here.
        MWString_EraseRange(&local.text, (const char*)(slashPosition + 1), MWString_GetLength(&local.text) - slashPosition);
    }
    // Construct each page path, dispatch the load, then append the resulting handle.
    for (int pageIndex = 0; pageIndex < local.result.count; ++pageIndex) {
        gtl_FontData_GetRecord((HeapObj*)AsFontAtlas(self)->fontData, pageIndex, local.attr);
        MWString_CopyConstruct(&local.part, &local.text);
        StringCleanup cleanupPart = {(NString*)&local.part};
        MWString_AppendCString(&local.part, local.attr);
        FontAtlasPage_InitEmpty(local.item);
        // This is a case-sensitive search for "tga" in the last four characters,
        // not a validated file extension test. Preserve that dispatch rule.
        unsigned int tgaPosition = MWString_FindCString(&local.part, texturePathStrings + kTgaSuffixOffset, MWString_GetLength(&local.part) - kExtensionSearchTailBytes);
        if (tgaPosition != -1) {
            Texture_CreateFromStream(self, MWString_GetCString(&local.part), local.item);
        } else {
            // Initialize the decoder's existing +0x0C/+0x10 control words only;
            // the other metadata bytes are produced/consumed by the shared loader.
            *(int*)(local.bigBuf + 0xc) = 0;
            *(int*)(local.smallBuf + 0x10) = 0;
            int pagePathAddress = MWString_GetCString(&local.part);
            ImageResource_LoadFileAndDecode(pagePathAddress, local.item, local.a, local.bigBuf, local.smallBuf);
        }
        FontAtlasPageVector_PushBack(AsFontAtlas(self)->pages, local.item);
    }
    return 1;
}

} // extern "C"
