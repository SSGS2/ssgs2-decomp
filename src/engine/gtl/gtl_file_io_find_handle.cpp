// Translation unit 0x8041C290-0x8041C598 (6 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {
extern void FileIO_Free(void *p);
extern void __dl__FPv(void *p);
extern void *FileIO_InvokeAllocatorCallback3(void *p);
// This symbol forwards to FileIo_InitializeDevices callback slot +0xC.
// Its callers here pass existing storage for disposal; the callback's contract is unresolved.
extern void FileIOString_FitCapacityForLength(void *a, unsigned int b);
extern void FileIOString_ReplaceRangeWithAllocator(void *a, unsigned int b, unsigned int c, const char *d, const char *e, void *f);
extern char *strchr(const char *s, int c);
extern unsigned int strlen(const char *s);

// Disposal-only view: +0 owns storage, +4 is reset before release. The
// element type and remaining container layout are unresolved; this is NOT
// the find-handle/string layout below. deleting > 0 also releases the wrapper.
struct dtor_8041C294_Layout { void *field00; int field04; };

// Release the FileIO-owned buffer and optionally its containing allocation; return self.
void* dtor_8041C294(dtor_8041C294_Layout *self, int deleting) {
    if (self != 0) {
        // Original 0x8041C294 / 0x8041C514: shared destructor continuation.
        do {
            if (self == 0) break;
            if (self == 0) break;
            if (self == 0) break;
            if (self == 0) break;
            if (self->field00 != 0) {
                void *data = self->field00;
                self->field04 -= ((char*)self + self->field04) - (char*)self;
                FileIO_Free(data);
            }
        } while (0);
        if (deleting > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Release a FileIO allocation (no wrapper or element destruction).
void dtor_8041C290(void *p) { FileIO_Free(p); }
// Release a FileIO allocation for this destructor specialization.
void dtor_8041C510(void *p) { FileIO_Free(p); }
// Release a FileIO allocation for the find-result specialization.
void dtor_8041C594(void *p) { FileIO_Free(p); }

struct dtor_8041C514_Layout { void *field00; int field04; };

// Reset the buffer extent, dispose through callback slot 3, optionally delete self, and return self.
void* FindHandleNames_dtor_8041C514(dtor_8041C514_Layout *self, int deleting) {
    if (self != 0) {
        // Original 0x8041C294 / 0x8041C514: shared destructor continuation.
        do {
            if (self == 0) break;
            if (self == 0) break;
            if (self == 0) break;
            if (self == 0) break;
            if (self->field00 != 0) {
                void *data = self->field00;
                self->field04 -= ((char*)self + self->field04) - (char*)self;
                FileIO_InvokeAllocatorCallback3(data);
            }
        } while (0);
        if (deleting > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Find-handle +4 begins a 12-byte small-string representation for the pattern:
// big-endian word bit 31 set => length at +8, heap pointer at +0xC;
// otherwise byte +4 holds the length (low 7 bits), inline bytes start at +5.
// +0 is not used here. The stored pattern is borrowed, never modified/freed.
struct gtl_CFindHandleFMEM_Layout { void *field00; int field04; char pad[0x04]; void *field0C; };
enum FindPatternBits { kHeapStringTag = 0x80000000u, kInlineLengthMask = 0x7f };

// Return 1 for a complete pattern match, 0 otherwise; path is a borrowed NUL-terminated candidate.
// Dialect: case-sensitive bytes, '?' consumes one byte, '*' skips to the FIRST
// occurrence of the next literal byte (no backtracking; '*?' searches for '?').
// Runs of '*' collapse; a trailing '*' scans to NUL, but '*' cannot consume an
// already-empty candidate because the loop checks NUL before interpreting '*'.
// If path contains no dot ANYWHERE, append '.' to the temporary candidate.
int gtl_CFindHandleFMEM_MatchesCandidate(gtl_CFindHandleFMEM_Layout *self, const char *path) {
    // Phase 1: own a temporary candidate string; a heap representation must be freed on either exit.
    char buf[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
    char initialAllocatorTag = 0;
    char copyAllocatorTag = initialAllocatorTag;
    char suffixAllocatorTag = 0;
    char unusedAllocatorTag = 0;
    unsigned int len = strlen(path);
    FileIOString_FitCapacityForLength(buf, len);
    char b14 = (char)buf[0]; // placeholder not used
    (void)b14;
    FileIOString_ReplaceRangeWithAllocator(buf, 0, 0, path, path + len, &copyAllocatorTag);

    if (strchr(path, 0x2e) == 0) {
        unsigned int v;
        unsigned int first = *(unsigned int *)buf;
        if (first & kHeapStringTag) {
            v = *(unsigned int *)(buf + 4);
        } else {
            v = (unsigned char)buf[0] & kInlineLengthMask;
        }
        unsigned int dotlen = strlen(".");
        (void)dotlen;
        FileIOString_ReplaceRangeWithAllocator(buf, v, 0, ".", "." + 1, &suffixAllocatorTag);
    }

    // Phase 2: decode borrowed pattern and owned candidate independently (both use the same tag).
    char *pat;
    if (self->field04 & kHeapStringTag) {
        pat = (char *)self->field0C;
    } else {
        pat = (char *)self + 5;
    }
    char *cur;
    unsigned int flag;
    unsigned int first = *(unsigned int *)buf;
    if (first & kHeapStringTag) {
        cur = (char *)*(unsigned int *)(buf + 8);
        flag = 1;
    } else {
        cur = buf + 1;
        flag = 0;
    }

    char *patternCursor = pat;
    char *candidateCursor = cur;

    // Phase 3: monotonic cursors; a failed suffix never retries an earlier '*' choice.
    for (;;) {
        signed char s1 = (signed char)*patternCursor;
        if (s1 == 0 || (signed char)*candidateCursor == 0) {
            if (s1 != 0) break;
            signed char s2b = (signed char)*candidateCursor;
            if (s2b != 0) break;
            if (flag) FileIO_Free(*(void **)(buf + 8));
            return 1;
        }

        signed char a = (signed char)*patternCursor;
        signed char b = (signed char)*candidateCursor;
        if (a != b && a != '?') {
            if (a == '*') {
                // Continue with star handling below.
            } else {
                break;
            }
        } else {
            patternCursor++;
            candidateCursor++;
        }

        if (*patternCursor != '*') continue;
        do { patternCursor++; } while (*patternCursor == '*');
        {
            signed char target = (signed char)*patternCursor;
            while (1) {
                signed char qc = (signed char)*candidateCursor;
                if (qc == target) break;
                if (qc == 0) break;
                candidateCursor++;
            }
        }
    }
    if (flag) FileIO_Free(*(void **)(buf + 8));
    return 0;

    // Allocator tags are addressable ABI scratch, with no additional behavior here.
    (void)suffixAllocatorTag; (void)copyAllocatorTag;
}
} // extern "C"
