// The file layer's 12-byte string. A short string is stored inline and a longer one in a heap buffer.
// This file has the string destructor and the routine that resizes storage to fit a requested length.

#define char_array_guard CharArrayGuard_u80413D24
#include "game/stdexcept_local.h"
#undef char_array_guard
#include "sdk_protos.h"
#include <decomp/cw_bitfields.h>
#include <decomp/cw_string_guard.h>

// Twelve-byte string storage, not a C string pointer. On this big-endian target:
// heap: word 0 = high-bit tag | allocated byte count, word 1 = character count,
//       word 2 = owned buffer; inline: byte 0 low 7 bits = character count,
//       bytes 1..11 = characters including their terminator (10 usable characters).
struct StringRep {
    CW_BITS2(unsigned int isLong : 1, unsigned int rest : 31)
    unsigned int heapLengthOrInlineBytes;
    char* data;
};

extern "C" {
void FileIO_Free(void*);
void* FileIO_Alloc(unsigned int);
void __dl__FPv(void*);
extern char pathAndStringExceptionStrings[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];

enum {
    kInlineStorageBytes = 11,
    kAllocationQuantumBytes = 16,
    kInlineLengthMask = 0x7f
};
static const unsigned int kMaximumStringLength = 0x7ffffffe;

// Free a string's heap buffer and, for deleting > 0, the string object itself.
// Returns self even when freed; does not reset a surviving object's storage fields.
void* FileIOString_dtor(StringRep* self, int deleting) {
    if (self) {
        if (self->isLong)
            FileIO_Free(self->data);
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

// Fit storage to max(requestedLength, current length), preserving text and NUL.
// Unlike a grow-only reserve, this may shrink or return heap text to inline storage.
// self must be a valid writable string; existing character pointers may be invalidated.
void FileIOString_FitCapacityForLength(StringRep* self, unsigned int requestedLength) {
    // Validate the requested character count, then select a byte capacity including NUL.
    if (requestedLength > kMaximumStringLength) {
        std::length_error error(pathAndStringExceptionStrings + 2, pathAndStringExceptionStrings + 0x25);
    }
    unsigned int* words = reinterpret_cast<unsigned int*>(self);
    unsigned char* bytes = reinterpret_cast<unsigned char*>(self);
    unsigned int isLong = words[0] >> 31;
    unsigned int oldLength = isLong ? words[1] : (bytes[0] & kInlineLengthMask);
    unsigned int oldCap = isLong ? (words[0] & 0x7fffffff) : kInlineStorageBytes;
    if (requestedLength < oldLength)
        requestedLength = oldLength;
    unsigned int cap = kInlineStorageBytes;
    if (requestedLength + 1 > kInlineStorageBytes)
        cap = (requestedLength + kAllocationQuantumBytes) & ~(unsigned int)(kAllocationQuantumBytes - 1);
    if (cap == oldCap)
        return;

    // Select the destination before overwriting the representation; old text remains live.
    char* src = isLong ? self->data : reinterpret_cast<char*>(self) + 1;
    char* dst;
    unsigned int nowLong;
    if (cap == kInlineStorageBytes) {
        dst = reinterpret_cast<char*>(self) + 1;
        nowLong = 0;
    } else {
        if (cap > oldCap) {
            dst = static_cast<char*>(FileIO_Alloc(cap));
        } else {
            // Shrinking is optional: keep the old representation if allocation fails.
            try {
                dst = static_cast<char*>(FileIO_Alloc(cap));
            } catch (...) {
                return;
            }
        }
        nowLong = 1;
    }
    // Copy before releasing the old heap buffer; the destination now owns the full text.
    memmove(dst, src, oldLength);
    dst[oldLength] = 0;
    if (isLong)
        FileIO_Free(src);
    // Commit the tag and the length/capacity fields for the selected representation.
    // 0x7fffffff preserves capacity bits; 0x80000000 below preserves the heap tag.
    words[0] = (words[0] & 0x7fffffff) | (nowLong << 31);
    if (nowLong) {
        self->data = dst;
        words[1] = oldLength;
        words[0] = (words[0] & 0x80000000) | cap;
    } else {
        bytes[0] = (bytes[0] & 0x80) | (oldLength & kInlineLengthMask);
    }
}
}
