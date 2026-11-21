// Save banner image setup (translation unit 0x80294D44-0x8029523C).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"
#include "classes/BpTreeLinks.h"

struct LanguageTagStringPool {
    wchar_t japanese[9], english[8], german[7], french[7], spanish[8];
    wchar_t italian[8], dutch[6], simplifiedChinese[13], traditionalChinese[13];
    wchar_t korean[7], empty[1];
};
struct SaveBannerResources {
    void* vtable; void** files; unsigned int fileCount, fileCapacity;
    unsigned int reserved10; char sections[1];
};
struct NandBannerImage {
    char header[0xA0]; unsigned char bannerPixels[0x6000]; unsigned char iconPixels[0x1200];
};
struct TplImage {
    unsigned short height, width; unsigned int format; const void* pixels;
};
struct TplEntry { TplImage* image; };
enum BannerImageConstants {
    kSectionsOffset = (int)&((SaveBannerResources*)0)->sections,
    kRgb5A3 = 5, kBannerTextureIndex = 2, kIconTextureIndex = 1,
    kBannerPixelBytes = 0x6000, kIconPixelBytes = 0x1200, kBytesPerPixel = 2,
    kHeapStringShift = 31, kInlineLengthMask = 0x7F
};

struct Alloc { char c; };

struct Str {
    unsigned int flags;
    unsigned int length;
    const wchar_t* data;
    ~Str();
};

struct Pair_u80294D44 {
    Str key;
    void* value;
};

struct SectionNode { BpTreeLinks links; Pair_u80294D44 entry; };

extern "C" {
#pragma use_lmw_stmw on

unsigned char SCGetLanguage(void*);
void WString_ReserveCapacity(void*, unsigned int);
void WString_ReplaceRange(void*, int, int, const char*, const char*, Alloc);
void* SaveBannerSectionTree_FindInsertionPosition(void*, void*, void**, unsigned char*, unsigned char*);
void* SaveBannerSectionMap_InsertNode(void*, void*, unsigned char, unsigned char, Pair_u80294D44*);
void appFreeUni__FPv(void*);
void NANDInitBanner(void*, void*, const char*, const char*);
void* CSharedFilePtr_GetLoadedBuffer(void*);
void* TPLGet(void*, int);
extern void* s_bannerDefaultSectionValue;
extern unsigned char languageTagStrings[];
}

inline Str::~Str() {
    if (flags >> kHeapStringShift) appFreeUni__FPv((void*)data);
}

// Read the logical length from inline storage or the allocated-string header.
static inline unsigned int WideStringLength(Str* s) {
    if (!(s->flags >> kHeapStringShift)) return *(unsigned char*)s & kInlineLengthMask;
    return s->length;
}

// Locate UTF-16 text without exposing the small-string byte layout to callers.
static inline const wchar_t* WideStringData(Str* s) {
    if (!(s->flags >> kHeapStringShift)) return (const wchar_t*)&s->flags + 1;
    return s->data;
}

// Borrow the smaller string length; ties select the second argument.
static inline const unsigned int& MinLength(const unsigned int& a, const unsigned int& b) {
    return a < b ? a : b;
}

struct LocalizedLine {
    Str text;
};

struct LocalizedSection {
    LocalizedLine* items;
    unsigned int count;
};

extern "C" {

// Select localized banner text, then validate and copy the banner and icon textures.
int SaveBanner_InitializeLocalizedImage(SaveBannerResources* self, NandBannerImage* banner, void* flags) {
    const char* languageKey;
    unsigned char language = SCGetLanguage(self);
    languageKey = 0;
    switch (language) {
    case 0: languageKey = (const char*)languageTagStrings + (int)&((LanguageTagStringPool*)0)->japanese; break;
    case 1: languageKey = (const char*)languageTagStrings + (int)&((LanguageTagStringPool*)0)->english; break;
    case 2: languageKey = (const char*)languageTagStrings + (int)&((LanguageTagStringPool*)0)->german; break;
    case 3: languageKey = (const char*)languageTagStrings + (int)&((LanguageTagStringPool*)0)->french; break;
    case 4: languageKey = (const char*)languageTagStrings + (int)&((LanguageTagStringPool*)0)->spanish; break;
    case 5: languageKey = (const char*)languageTagStrings + (int)&((LanguageTagStringPool*)0)->italian; break;
    case 6: languageKey = (const char*)languageTagStrings + (int)&((LanguageTagStringPool*)0)->dutch; break;
    case 7: languageKey = (const char*)languageTagStrings + (int)&((LanguageTagStringPool*)0)->simplifiedChinese; break;
    case 8: languageKey = (const char*)languageTagStrings + (int)&((LanguageTagStringPool*)0)->traditionalChinese; break;
    case 9: languageKey = (const char*)languageTagStrings + (int)&((LanguageTagStringPool*)0)->korean; break;
    }
    const char* bannerTitle = (const char*)languageTagStrings + (int)&((LanguageTagStringPool*)0)->empty;
    const char* bannerSubtitle = bannerTitle;
    if (languageKey) {
        LocalizedSection* section;
        {
        Str sectionKey;
        sectionKey.flags = 0;
        sectionKey.length = 0;
        sectionKey.data = 0;
        unsigned int keyLength = wcslen((const wchar_t*)languageKey);
        WString_ReserveCapacity(&sectionKey, keyLength);
        Alloc al;
        WString_ReplaceRange(&sectionKey, 0, 0, languageKey, languageKey + keyLength * 2, al);
        unsigned int storedLength, comparisonLength, requestedLength, storedKeyLength;
        void* found;
        unsigned char insertFlags, insertSide;
        void* hint = SaveBannerSectionTree_FindInsertionPosition((char*)self + kSectionsOffset, &sectionKey, &found, &insertSide, &insertFlags);
        Pair_u80294D44* entry;
        int comparison = -1;
        if (found) {
            entry = &((SectionNode*)found)->entry;
            const wchar_t* requestedText;
            if (!(sectionKey.flags >> kHeapStringShift)) {
                requestedText = (const wchar_t*)&sectionKey.flags + 1;
                requestedLength = *(unsigned char*)&sectionKey & kInlineLengthMask;
            } else {
                requestedText = sectionKey.data;
                requestedLength = sectionKey.length;
            }
            storedKeyLength = WideStringLength(&entry->key);
            const wchar_t* storedText;
            if (!(entry->key.flags >> kHeapStringShift)) {
                storedText = (const wchar_t*)&entry->key.flags + 1;
                comparisonLength = *(unsigned char*)&entry->key & kInlineLengthMask;
            } else {
                storedText = entry->key.data;
                comparisonLength = entry->key.length;
            }
            storedLength = MinLength(storedKeyLength, comparisonLength);
            comparison = wmemcmp(storedText, requestedText, (size_t)MinLength(requestedLength, storedLength));
            if (comparison == 0) {
                if (storedLength < requestedLength) comparison = -1; else if (storedLength == requestedLength) comparison = 0; else comparison = 1;
            }
        }
        if (comparison < 0) {
            Pair_u80294D44 newEntry;
            if (!(sectionKey.flags >> kHeapStringShift)) {
                newEntry.key.flags = sectionKey.flags;
                newEntry.key.length = sectionKey.length;
                newEntry.key.data = sectionKey.data;
            } else {
                newEntry.key.flags = 0;
                newEntry.key.length = 0;
                newEntry.key.data = 0;
                WString_ReserveCapacity(&newEntry.key, sectionKey.length);
                Alloc al2;
                WString_ReplaceRange(&newEntry.key, 0, 0, (const char*)sectionKey.data,
                            (const char*)sectionKey.data + sectionKey.length * 2, al2);
            }
            newEntry.value = s_bannerDefaultSectionValue;
            entry = &((SectionNode*)SaveBannerSectionMap_InsertNode((char*)self + kSectionsOffset, hint, insertSide, insertFlags, &newEntry))->entry;
        }
        section = (LocalizedSection*)entry->value;
        }
        if (section) {
            if (section->count >= 2) {
                bannerSubtitle = (const char*)WideStringData(&section->items[1].text);
            }
            if (section->count >= 1) {
                bannerTitle = (const char*)WideStringData(&section->items[0].text);
            }
        }
    }
    NANDInitBanner(banner, flags, bannerTitle, bannerSubtitle);
    void* fileBuffer = 0;
    if (self->fileCount > kBannerTextureIndex) fileBuffer = self->files[kBannerTextureIndex];
    if (fileBuffer) {
        fileBuffer = CSharedFilePtr_GetLoadedBuffer(fileBuffer);
    } else {
        fileBuffer = 0;
    }
    if (!fileBuffer) return 0;
    TplEntry* texture = (TplEntry*)TPLGet(fileBuffer, 0);
    if (!texture || !texture->image) return 0;
    TplImage* image = texture->image;
    if (image->format != kRgb5A3 || (image->height * image->width * kBytesPerPixel) != kBannerPixelBytes) return 0;
    Memory_Copy(banner->bannerPixels, image->pixels,
                (size_t)(image->height * image->width * kBytesPerPixel));
    fileBuffer = 0;
    if (self->fileCount > kIconTextureIndex) fileBuffer = self->files[kIconTextureIndex];
    if (fileBuffer) {
        fileBuffer = CSharedFilePtr_GetLoadedBuffer(fileBuffer);
    } else {
        fileBuffer = 0;
    }
    if (!fileBuffer) return 0;
    texture = (TplEntry*)TPLGet(fileBuffer, 0);
    if (!texture || !texture->image) return 0;
    image = texture->image;
    if (image->format != kRgb5A3 || (image->height * image->width * kBytesPerPixel) != kIconPixelBytes) return 0;
    Memory_Copy(banner->iconPixels, image->pixels,
                (size_t)(image->height * image->width * kBytesPerPixel));
    return 1;
}

typedef char banner_sections_at_14[(kSectionsOffset == 0x14) ? 1 : -1];
typedef char banner_icons_at_60A0[((int)&((NandBannerImage*)0)->iconPixels == 0x60A0) ? 1 : -1];
typedef char banner_empty_language_at_AC[((int)&((LanguageTagStringPool*)0)->empty == 0xAC) ? 1 : -1];
typedef char banner_entry_at_0C[((int)&((SectionNode*)0)->entry == 0x0C) ? 1 : -1];
typedef char banner_wide_string_size[(sizeof(Str) == 12) ? 1 : -1];

} // extern "C"
