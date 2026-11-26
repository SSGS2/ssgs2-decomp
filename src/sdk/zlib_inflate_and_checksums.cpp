#include "classes/InflateWindow802CE62C.h"

// z_stream layout used by this zlib version (32-bit fields on Gekko).
struct ZlibStreamView {
    unsigned char* nextInput;
    unsigned int availableInput, totalInput;
    unsigned char* nextOutput;
    unsigned int availableOutput, totalOutput;
    unsigned char* message;
    void* state;
    void* allocate;
    void* release;
    void* allocatorContext;
    int dataType;
    unsigned int checksum;
};
struct ArchiveInputView {
    unsigned char usesCallbacks, memoryBacked;
    char padding02[6];
    unsigned char readFailed;
    char padding09[3];
    unsigned int callbackPosition;
    unsigned char eof;
    char padding11[3];
    void* input;
    union {
        unsigned int size;
        void* callbacks;
    };
    unsigned int position;
};
// Outer zlib wrapper state: method/flags -> dictionary -> blocks -> checksum.
// blocks points to a DIFFERENT allocation with its own block-mode state machine.
// check is reused for the CMF byte, completed checksum and bad-state recovery marker.
struct InflateRootView {
    unsigned int mode, check, neededChecksum, rawStream, windowBits;
    unsigned char* blocks;
};
// Same 0x40-byte block allocation viewed by InflateState802D and the shared
// InflateWindow header below. +0x1c/+0x20 are bit count/reservoir;
// +0x28..+0x3c are the window and checksum fields. codeState/temporaryTree
// are mode-dependent owned allocations, not outer-state pointers.
struct InflateBlocksView {
    int mode;
    void* codeState;
    unsigned int reserved08;
    void* temporaryTree;
    char padding10[0xC];
    unsigned int bitCount, bitBuffer;
    void* decodeTables;
};
typedef void* (*InflateAllocator)(void*, unsigned int, unsigned int);
typedef void (*InflateDeallocator)(void*, void*);
// Access the stream's owned outer-state pointer without confusing it with its blocks.
static inline unsigned char*& InflateRoot(unsigned char* stream) {
    return *(unsigned char**)&((ZlibStreamView*)stream)->state;
}
// Dispatch through zlib's caller-supplied allocation callback and context.
static inline void* AllocateInflate(unsigned char* stream, unsigned int count, unsigned int size) {
    return ((InflateAllocator)((ZlibStreamView*)stream)->allocate)(((ZlibStreamView*)stream)->allocatorContext, count,
                                                                   size);
}
// Return an inflater allocation through the matching release callback.
static inline void FreeInflate(unsigned char* stream, void* allocation) {
    ((InflateDeallocator)((ZlibStreamView*)stream)->release)(((ZlibStreamView*)stream)->allocatorContext, allocation);
}
// Consume one compressed byte and update both input counters.
static inline unsigned int NextInflateByte(unsigned char* stream) {
    ((ZlibStreamView*)stream)->availableInput--;
    ((ZlibStreamView*)stream)->totalInput++;
    return *(((ZlibStreamView*)stream)->nextInput)++;
}
enum ZlibStatus {
    kZlibOk = 0,
    kZlibEnd = 1,
    kZlibNeedDictionary = 2,
    kZlibStreamError = -2,
    kZlibDataError = -3,
    kZlibMemoryError = -4,
    kZlibBufferError = -5,
    kZlibVersionError = -6
};
enum InflateModes {
    kReadMethod,
    kReadFlags,
    kReadDictionaryByte3,
    kReadDictionaryByte2,
    kReadDictionaryByte1,
    kReadDictionaryByte0,
    kAwaitDictionary,
    kInflateBlocks,
    kReadChecksumByte3,
    kReadChecksumByte2,
    kReadChecksumByte1,
    kReadChecksumByte0,
    kInflateDone,
    kInflateBad
};
enum InflateSizes {
    kWindowBytes = 0x8000,
    kWindowBits = 15,
    kDeflateMethod = 8,
    kFinishFlush = 4
};
// The block decoder has its own mode zero, distinct from the outer zlib header reader.
enum InflateBlockModes {
    kReadBlockType = 0,
    kBuildBitLengthTree = 4,
    kBuildDynamicTrees = 5,
    kDecodeBlockCodes = 6
};
enum ZlibHeaderBits {
    kCompressionMethodMask = 0x0F,
    kWindowInfoShift = 4,
    kWindowExponentBias = 8,
    kPresetDictionaryFlag = 0x20,
    kHeaderCheckDivisor = 31
};
enum InflateFastLimits {
    kMaximumMatchBytes = 258,
    kFastInputBytes = 10
};
enum ZlibLimits {
    kAdlerChunkSize = 5552,
    kAdlerModulus = 65521
};

extern "C" {

struct InflateDiagnosticPool {
    unsigned char invalidLiteralLength[0x1C];
    unsigned char invalidDistance[0x16];
    unsigned char blockDiagnostics[0x185 - 0x32];
    unsigned char version[6];
    unsigned char unknownMethod[0x1B];
    unsigned char invalidWindowSize[0x14];
    unsigned char incorrectHeaderCheck[0x17];
    unsigned char needDictionary[0x10];
    unsigned char incorrectDataCheck[0x15];
    unsigned char archivePathStrings[0x214 - 0x1F6];
};
extern InflateDiagnosticPool inflateDiagnosticStrings;
// The runtime allocator multiplies the two arguments and zeroes the allocation.
void* Runtime_AllocateZeroedArray(unsigned int count, unsigned int elementBytes);
void* free(void* a);
int Zlib_InflateBlocks(void* a, void* b, int c);
void* memcpy(void* dst, const void* src, unsigned int n);
void* __nw__FUl(unsigned long);

unsigned int Zlib_Adler32(unsigned int a, const unsigned char* b, unsigned int c);
// Allocate a zeroed array for zlib; the default allocator ignores its context.
void* Zlib_DefaultAllocate(void* allocatorContext, unsigned int count, unsigned int elementBytes) {
    return Runtime_AllocateZeroedArray(count, elementBytes);
}

// Free a zlib allocation while preserving the allocator context return value.
void* Zlib_DefaultFree(void* allocatorContext, void* allocation) {
    void* keep = allocatorContext;
    free(allocation);
    return keep;
}

#pragma push
#pragma use_lmw_stmw off
// Allocate and reset an inflater for raw DEFLATE with a 32 KiB window.
int Zlib_InitRawInflate(void* self) {
    unsigned char* p = (unsigned char*)self;
    unsigned char* q = inflateDiagnosticStrings.version;
    if (q == 0 || (signed char)*(volatile unsigned char*)q !=
                      (signed char)*(volatile unsigned char*)(inflateDiagnosticStrings.version))
        return kZlibVersionError;
    if (p == 0)
        return kZlibStreamError;
    (*(unsigned int*)&((ZlibStreamView*)p)->message) = 0;
    if ((*(unsigned int*)&((ZlibStreamView*)p)->allocate) == 0) {
        (*(unsigned int*)&((ZlibStreamView*)p)->allocatorContext) = 0;
        ((ZlibStreamView*)p)->allocate = (void*)Zlib_DefaultAllocate;
    }
    if ((*(unsigned int*)&((ZlibStreamView*)p)->release) == 0)
        ((ZlibStreamView*)p)->release = (void*)Zlib_DefaultFree;
    typedef void* (*Alloc)(void*, unsigned int, unsigned int);
    typedef void (*Free)(void*, void*);
    typedef unsigned int (*Init)(unsigned int, const unsigned char*, unsigned int);
    struct Root {
        int mode;
        unsigned int check, need, nowrap, wbits;
        unsigned char* node;
    };
    struct Reset {
        // Release temporary decode state, empty the window and reset the optional checksum.
        static inline void run(unsigned char* node, unsigned char* p) {
            if ((unsigned int)((InflateBlocksView*)node)->mode - kBuildBitLengthTree <= 1)
                FreeInflate(p, ((InflateBlocksView*)node)->temporaryTree);
            if (((InflateBlocksView*)node)->mode == kDecodeBlockCodes)
                FreeInflate(p, ((InflateBlocksView*)node)->codeState);
            ((InflateBlocksView*)node)->mode = kReadBlockType;
            ((InflateBlocksView*)node)->bitCount = 0;
            ((InflateBlocksView*)node)->bitBuffer = 0;
            unsigned int window = (*(unsigned int*)&AsInflateWindow(node)->windowBegin);
            (*(unsigned int*)&AsInflateWindow(node)->writeCursor) = window;
            (*(unsigned int*)&AsInflateWindow(node)->readCursor) = window;
            Init init = (*(Init*)&AsInflateWindow(node)->checksumCallback);
            if (init) {
                unsigned int result = init(0, 0, 0);
                (*(unsigned int*)&AsInflateWindow(node)->checksum) = result;
                ((ZlibStreamView*)p)->checksum = result;
            }
        }
    };
    struct Blocks {
        // Allocate tables and circular window, unwinding earlier allocations on failure.
        static inline unsigned char* create(unsigned char* p, Init adlerfn) {
            unsigned char* node = (unsigned char*)AllocateInflate(p, 1, 0x40);
            if (!node)
                return node;
            ((InflateBlocksView*)node)->decodeTables = AllocateInflate(p, 8, 0x5a0);
            if (!((InflateBlocksView*)node)->decodeTables) {
                FreeInflate(p, node);
                return 0;
            }
            (*(void**)&AsInflateWindow(node)->windowBegin) = AllocateInflate(p, 1, 0x8000);
            if (!(*(void**)&AsInflateWindow(node)->windowBegin)) {
                FreeInflate(p, ((InflateBlocksView*)node)->decodeTables);
                FreeInflate(p, node);
                return 0;
            }
            (*(void**)&AsInflateWindow(node)->windowEnd) =
                (unsigned char*)(*(volatile unsigned int*)&AsInflateWindow(node)->windowBegin) + 0x8000;
            (*(Init*)&AsInflateWindow(node)->checksumCallback) = adlerfn;
            ((InflateBlocksView*)node)->mode = kReadBlockType;
            Reset::run(node, p);
            return node;
        }
    };
    // Phase 1: allocate the outer wrapper, then its independently owned block state.
    void* root = AllocateInflate(p, 1, 0x18);
    ((ZlibStreamView*)p)->state = root;
    if (!root)
        return kZlibMemoryError;
    ((Root*)root)->node = 0;
    ((InflateRootView*)InflateRoot(p))->rawStream = 0;
    ((InflateRootView*)InflateRoot(p))->rawStream = 1;
    ((InflateRootView*)InflateRoot(p))->windowBits = 15;
    unsigned char* node = Blocks::create(p, ((InflateRootView*)InflateRoot(p))->rawStream ? 0 : Zlib_Adler32);
    ((InflateRootView*)InflateRoot(p))->blocks = node;
    if (!node) {
        if (p && InflateRoot(p) && (Free)((ZlibStreamView*)p)->release) {
            node = ((InflateRootView*)InflateRoot(p))->blocks;
            if (node) {
                Reset::run(node, p);
                FreeInflate(p, (*(void**)&AsInflateWindow(node)->windowBegin));
                FreeInflate(p, ((InflateBlocksView*)node)->decodeTables);
                FreeInflate(p, node);
            }
            FreeInflate(p, InflateRoot(p));
            InflateRoot(p) = 0;
        }
        return kZlibMemoryError;
    }
    // Phase 2: zero stream counters; raw DEFLATE starts at blocks, bypassing the header.
    if (p) {
        int mode;
        Root* state = (Root*)InflateRoot(p);
        if (state) {
            mode = 0;
            ((ZlibStreamView*)p)->totalOutput = 0;
            ((ZlibStreamView*)p)->totalInput = 0;
            (*(unsigned int*)&((ZlibStreamView*)p)->message) = 0;
            if (state->nowrap)
                mode = 7;
            state->mode = mode;
            Reset::run(((Root*)InflateRoot(p))->node, p);
        }
    }
    return 0;
}
#pragma pop

extern unsigned int crc32TableAndUnzipVersion[];
extern unsigned int inflateDecodeTableData[];

// Fast-loop view of the SAME block allocation, starting meaningful fields at +0x1c.
struct InflateState802D {
    unsigned char pad00[0x1C];
    unsigned int bits;
    unsigned int hold;
    unsigned char pad24[4];
    unsigned char* window;
    unsigned char* windowEnd;
    unsigned char* read;
    unsigned char* write;
    void* checksumCallback;
    unsigned int checksum;
};
typedef ZlibStreamView Stream802D;
// Huffman entry: bits consumed; op=0 literal, bit 4 length/distance with low-nibble
// extra bits, bit 6 terminal/invalid, bit 5 end-of-block; otherwise op is subtable bits.
struct Code802D {
    unsigned char op, bits;
    unsigned short pad;
    unsigned int val;
};

#pragma push
#pragma use_lmw_stmw on
// Consume zlib header, DEFLATE blocks, and optional Adler checksum incrementally.
// Mutates stream input/output counters and retained state. Returns ZlibStatus;
// a no-input return preserves the current mode, so callers can refill and resume.
// Flush 4 maps a would-block status to -5. Case fallthrough consumes consecutive fields.
int Zlib_Inflate(unsigned char* self, register int bufferStatus) {
    if (self == 0 || ((ZlibStreamView*)self)->state == 0 || *(void**)self == 0)
        return kZlibStreamError;
    bufferStatus = (bufferStatus == kFinishFlush) ? (int)kZlibBufferError : (int)kZlibOk;
    int result = kZlibBufferError;
    struct RootState {
        unsigned int mode;
        unsigned long check;
        unsigned int need, nowrap, wbits;
        unsigned char* node;
    };
    struct Reset {
        // Save the completed checksum, release decode state and reset the window/checksum cursors.
        static inline void run(unsigned char* node, unsigned char* self, unsigned long* check) {
            if (check)
                *check = (*(unsigned int*)&AsInflateWindow(node)->checksum);
            typedef void (*Free)(void*, void*);
            if ((unsigned int)((InflateBlocksView*)node)->mode - kBuildBitLengthTree <= 1)
                ((Free)((ZlibStreamView*)self)->release)(((ZlibStreamView*)self)->allocatorContext,
                                                         ((InflateBlocksView*)node)->temporaryTree);
            if (((InflateBlocksView*)node)->mode == kDecodeBlockCodes)
                ((Free)((ZlibStreamView*)self)->release)(((ZlibStreamView*)self)->allocatorContext,
                                                         ((InflateBlocksView*)node)->codeState);
            ((InflateBlocksView*)node)->mode = kReadBlockType;
            ((InflateBlocksView*)node)->bitCount = 0;
            ((InflateBlocksView*)node)->bitBuffer = 0;
            unsigned int window = (*(unsigned int*)&AsInflateWindow(node)->windowBegin);
            (*(unsigned int*)&AsInflateWindow(node)->writeCursor) = window;
            (*(unsigned int*)&AsInflateWindow(node)->readCursor) = window;
            typedef unsigned int (*Init)(unsigned int, const unsigned char*, unsigned int);
            Init init = (*(Init*)&AsInflateWindow(node)->checksumCallback);
            if (init) {
                unsigned int result = init(0, 0, 0);
                (*(unsigned int*)&AsInflateWindow(node)->checksum) = result;
                ((ZlibStreamView*)self)->checksum = result;
            }
        }
    };
    for (;;) {
        unsigned char* root = InflateRoot(self);
        switch (((InflateRootView*)root)->mode) {
        case kReadMethod: {
            if (((ZlibStreamView*)self)->availableInput == 0)
                return result;
            result = bufferStatus;
            unsigned int byte = NextInflateByte(self);
            ((InflateRootView*)root)->check = byte;
            // CMF low nibble CM=8 is DEFLATE; high nibble CINFO encodes log2(window)-8.
            if ((byte & kCompressionMethodMask) != kDeflateMethod) {
                ((InflateRootView*)InflateRoot(self))->mode = kInflateBad;
                ((ZlibStreamView*)self)->message = inflateDiagnosticStrings.unknownMethod;
                ((InflateRootView*)InflateRoot(self))->check = 5;
                continue;
            }
            root = InflateRoot(self);
            if ((((InflateRootView*)root)->check >> kWindowInfoShift) + kWindowExponentBias >
                ((InflateRootView*)root)->windowBits) {
                ((InflateRootView*)root)->mode = kInflateBad;
                ((ZlibStreamView*)self)->message = inflateDiagnosticStrings.invalidWindowSize;
                ((InflateRootView*)InflateRoot(self))->check = 5;
                continue;
            }
            ((InflateRootView*)root)->mode = kReadFlags;
        }
        case kReadFlags: {
            if (((ZlibStreamView*)self)->availableInput == 0)
                return result;
            unsigned int byte = NextInflateByte(self);
            result = bufferStatus;
            root = InflateRoot(self);
            // CMF:FLG is big-endian and divisible by 31; FLG bit 5 requests a dictionary.
            if (((((InflateRootView*)root)->check << 8) + byte) % kHeaderCheckDivisor != 0) {
                ((InflateRootView*)root)->mode = kInflateBad;
                ((ZlibStreamView*)self)->message = inflateDiagnosticStrings.incorrectHeaderCheck;
                ((InflateRootView*)InflateRoot(self))->check = 5;
                continue;
            }
            if (!(byte & kPresetDictionaryFlag)) {
                ((InflateRootView*)root)->mode = kInflateBlocks;
                continue;
            }
            ((InflateRootView*)root)->mode = kReadDictionaryByte3;
        }
        case kReadDictionaryByte3:
            if (((ZlibStreamView*)self)->availableInput == 0)
                return result;
            result = bufferStatus;
            ((InflateRootView*)InflateRoot(self))->neededChecksum = (unsigned int)NextInflateByte(self) << 24;
            ((InflateRootView*)InflateRoot(self))->mode = kReadDictionaryByte2;
        case kReadDictionaryByte2:
            if (((ZlibStreamView*)self)->availableInput == 0)
                return result;
            result = bufferStatus;
            ((InflateRootView*)InflateRoot(self))->neededChecksum += (unsigned int)NextInflateByte(self) << 16;
            ((InflateRootView*)InflateRoot(self))->mode = kReadDictionaryByte1;
        case kReadDictionaryByte1:
            if (((ZlibStreamView*)self)->availableInput == 0)
                return result;
            result = bufferStatus;
            ((InflateRootView*)InflateRoot(self))->neededChecksum += (unsigned int)NextInflateByte(self) << 8;
            ((InflateRootView*)InflateRoot(self))->mode = kReadDictionaryByte0;
        case kReadDictionaryByte0:
            if (((ZlibStreamView*)self)->availableInput == 0)
                return result;
            ((InflateRootView*)InflateRoot(self))->neededChecksum += NextInflateByte(self);
            root = InflateRoot(self);
            ((ZlibStreamView*)self)->checksum = ((InflateRootView*)root)->neededChecksum;
            ((InflateRootView*)root)->mode = kAwaitDictionary;
            return 2;
        case kAwaitDictionary:
            ((InflateRootView*)root)->mode = kInflateBad;
            ((ZlibStreamView*)self)->message = inflateDiagnosticStrings.needDictionary;
            ((InflateRootView*)InflateRoot(self))->check = 0;
            return kZlibStreamError;
        case kInflateBlocks: {
            result = Zlib_InflateBlocks(((InflateRootView*)root)->blocks, self, result);
            if (result == -3) {
                ((InflateRootView*)InflateRoot(self))->mode = kInflateBad;
                ((InflateRootView*)InflateRoot(self))->check = 0;
                continue;
            }
            if (result == 0)
                result = bufferStatus;
            if (result != 1)
                return result;
            result = bufferStatus;
            Reset::run(((RootState*)InflateRoot(self))->node, self, &((RootState*)InflateRoot(self))->check);
            root = InflateRoot(self);
            if (((InflateRootView*)root)->rawStream) {
                ((InflateRootView*)root)->mode = kInflateDone;
                continue;
            }
            ((InflateRootView*)root)->mode = kReadChecksumByte3;
        }
        case kReadChecksumByte3:
            if (((ZlibStreamView*)self)->availableInput == 0)
                return result;
            result = bufferStatus;
            ((InflateRootView*)InflateRoot(self))->neededChecksum = (unsigned int)NextInflateByte(self) << 24;
            ((InflateRootView*)InflateRoot(self))->mode = kReadChecksumByte2;
        case kReadChecksumByte2:
            if (((ZlibStreamView*)self)->availableInput == 0)
                return result;
            result = bufferStatus;
            ((InflateRootView*)InflateRoot(self))->neededChecksum += (unsigned int)NextInflateByte(self) << 16;
            ((InflateRootView*)InflateRoot(self))->mode = kReadChecksumByte1;
        case kReadChecksumByte1:
            if (((ZlibStreamView*)self)->availableInput == 0)
                return result;
            result = bufferStatus;
            ((InflateRootView*)InflateRoot(self))->neededChecksum += (unsigned int)NextInflateByte(self) << 8;
            ((InflateRootView*)InflateRoot(self))->mode = kReadChecksumByte0;
        case kReadChecksumByte0:
            if (((ZlibStreamView*)self)->availableInput == 0)
                return result;
            result = bufferStatus;
            ((InflateRootView*)InflateRoot(self))->neededChecksum += NextInflateByte(self);
            root = InflateRoot(self);
            if (((InflateRootView*)root)->check != ((InflateRootView*)root)->neededChecksum) {
                ((InflateRootView*)root)->mode = kInflateBad;
                ((ZlibStreamView*)self)->message = inflateDiagnosticStrings.incorrectDataCheck;
                ((InflateRootView*)InflateRoot(self))->check = 5;
                continue;
            }
            ((InflateRootView*)root)->mode = kInflateDone;
        case kInflateDone:
            return 1;
        case kInflateBad:
            return kZlibDataError;
        default:
            return kZlibStreamError;
        }
    }
}
#pragma pop

// Save decoder cursors, returning unused whole bytes to the input stream.
static inline int FinishInflateFastCodes(int result, Stream802D* const& strm, InflateState802D* const& state,
                                         unsigned int& avail, unsigned int& bits, const unsigned int& hold,
                                         unsigned char*& in, unsigned char*& out, unsigned int& back) {
    back = strm->availableInput - avail;
    if ((bits >> 3) < back)
        back = bits >> 3;
    state->hold = hold;
    bits -= back << 3;
    in -= back;
    state->bits = bits;
    avail += back;
    strm->totalInput += (unsigned int)(in - strm->nextInput);
    strm->availableInput = avail;
    strm->nextInput = in;
    state->write = out;
    return result;
}

#pragma push
#pragma use_lmw_stmw on
// Decode literal/length and distance Huffman codes into the circular output window.
// Caller must provide >=10 input bytes and >=258 contiguous free window bytes
// before entry (the first do-loop iteration is unconditional). Trees and window
// are borrowed. hold's low bits are next to decode; bits counts valid reservoir bits;
// in/avail are input cursor/remaining bytes; out/left are window cursor/free bytes.
// Forward byte copies intentionally support overlapping LZ77 matches; memcpy cannot.
// FinishInflateFastCodes commits cursors and returns prefetched whole bytes to input.
// Return 0 to resume the slow path, 1 at block end, -3 for invalid codes.
int Zlib_InflateFastCodes(register unsigned int lbits, register unsigned int dbits, Code802D* ltree, Code802D* dtree,
                          InflateState802D* state, Stream802D* strm) {
    Code802D* here;
    unsigned int op;
    unsigned int hold;
    unsigned int bits;
    unsigned char* in;
    unsigned int avail;
    unsigned char* out;
    unsigned int left;
    unsigned int dmask;
    unsigned int lmask;
    unsigned int len;
    unsigned int dist;
    unsigned char* from;
    in = strm->nextInput;
    avail = strm->availableInput;
    hold = state->hold;
    bits = state->bits;
    out = state->write;
    unsigned char* read = AsInflateWindow(state)->readCursor;
    left = out < read ? (unsigned int)(read - out) - 1 : (unsigned int)(AsInflateWindow(state)->windowEnd - out);
    lmask = inflateDecodeTableData[lbits];
    dmask = inflateDecodeTableData[dbits];
    unsigned int back;
    // Refill reservoir -> decode literal or match -> copy match -> recheck fast-loop margins.
    do {
        while (bits < 20) {
            --avail;
            hold |= (unsigned int)*in++ << bits;
            bits += 8;
        }
        here = ltree + (hold & lmask);
        op = here->op;
        if (op == 0) {
            *out++ = (unsigned char)here->val;
            --left;
            hold >>= here->bits;
            bits -= here->bits;
        } else
            for (;;) {
                hold >>= here->bits;
                bits -= here->bits;
                if (op & 16) {
                    op &= 15;
                    len = here->val + (hold & inflateDecodeTableData[op]);
                    bits -= op;
                    hold >>= op;
                    while (bits < 15) {
                        --avail;
                        hold |= (unsigned int)*in++ << bits;
                        bits += 8;
                    }
                    here = dtree + (hold & dmask);
                    op = here->op;
                    for (;;) {
                        hold >>= here->bits;
                        bits -= here->bits;
                        if (op & 16) {
                            op &= 15;
                            while (bits < op) {
                                --avail;
                                hold |= (unsigned int)*in++ << bits;
                                bits += 8;
                            }
                            unsigned int extra = hold & inflateDecodeTableData[op];
                            bits -= op;
                            hold >>= op;
                            dist = here->val + extra;
                            left -= len;
                            from = out - dist;
                            unsigned char* window = state->window;
                            if (from < window) {
                                unsigned char* end = AsInflateWindow(state)->windowEnd;
                                do {
                                    from += end - window;
                                } while (from < window);
                                unsigned int count = end - from;
                                if (len > count) {
                                    len -= count;
                                    do {
                                        *out++ = *from++;
                                    } while (--count);
                                    from = state->window;
                                    do {
                                        *out++ = *from++;
                                    } while (--len);
                                } else {
                                    *out++ = *from++;
                                    *out++ = *from++;
                                    len -= 2;
                                    do {
                                        *out++ = *from++;
                                    } while (--len);
                                }
                            } else {
                                *out++ = *from++;
                                *out++ = *from++;
                                len -= 2;
                                do {
                                    *out++ = *from++;
                                } while (--len);
                            }
                            break;
                        } else if (!(op & 64)) {
                            here += here->val;
                            here += hold & inflateDecodeTableData[op];
                            op = here->op;
                        } else {
                            ((ZlibStreamView*)strm)->message = inflateDiagnosticStrings.invalidDistance;
                            return FinishInflateFastCodes(-3, strm, state, avail, bits, hold, in, out, back);
                        }
                    }
                    break;
                } else if (!(op & 64)) {
                    here += here->val;
                    here += hold & inflateDecodeTableData[op];
                    op = here->op;
                    if (op == 0) {
                        *out++ = (unsigned char)here->val;
                        --left;
                        hold >>= here->bits;
                        bits -= here->bits;
                        break;
                    }
                } else if (op & 32) {
                    return FinishInflateFastCodes(1, strm, state, avail, bits, hold, in, out, back);
                } else {
                    ((ZlibStreamView*)strm)->message = inflateDiagnosticStrings.invalidLiteralLength;
                    return FinishInflateFastCodes(-3, strm, state, avail, bits, hold, in, out, back);
                }
            }
    } while (left >= kMaximumMatchBytes && avail >= kFastInputBytes);
    return FinishInflateFastCodes(0, strm, state, avail, bits, hold, in, out, back);
}
#pragma pop

// Update a ZIP CRC-32, processing eight input bytes per unrolled iteration.
unsigned int Zlib_Crc32(unsigned int crc, const unsigned char* data, unsigned int len) {
    if (data == 0)
        return 0;
    unsigned int* table = crc32TableAndUnzipVersion;
    unsigned int c = crc ^ 0xFFFFFFFFu;
    unsigned int ctr = len >> 3;
    unsigned int rem = len;
    if (rem >= 8) {
        for (unsigned int i = 0; i < ctr; ++i) {
            unsigned char b0 = data[0];
            unsigned int t1 = c >> 8;
            unsigned char b1 = data[1];
            rem -= 8;
            unsigned int x0 = (c ^ b0) & 0xFF;
            unsigned char b2 = data[2];
            unsigned int v0 = table[x0];
            unsigned char b3 = data[3];
            c = t1 ^ v0;
            unsigned char b4 = data[4];
            unsigned int x1 = (c ^ b1) & 0xFF;
            unsigned char b5 = data[5];
            unsigned int v1 = table[x1];
            unsigned int t2 = c >> 8;
            unsigned char b6 = data[6];
            c = t2 ^ v1;
            unsigned char b7 = data[7];
            unsigned int x2 = (c ^ b2) & 0xFF;
            unsigned int v2 = table[x2];
            unsigned int t3 = c >> 8;
            c = t3 ^ v2;
            unsigned int x3 = (c ^ b3) & 0xFF;
            unsigned int v3 = table[x3];
            unsigned int t4 = c >> 8;
            c = t4 ^ v3;
            unsigned int x4 = (c ^ b4) & 0xFF;
            unsigned int v4 = table[x4];
            unsigned int t5 = c >> 8;
            c = t5 ^ v4;
            unsigned int x5 = (c ^ b5) & 0xFF;
            unsigned int v5 = table[x5];
            unsigned int t6 = c >> 8;
            c = t6 ^ v5;
            unsigned int x6 = (c ^ b6) & 0xFF;
            unsigned int v6 = table[x6];
            unsigned int t7 = c >> 8;
            c = t7 ^ v6;
            unsigned int x7 = (c ^ b7) & 0xFF;
            unsigned int v7 = table[x7];
            unsigned int t8 = c >> 8;
            c = t8 ^ v7;
            data += 8;
        }
    }
    if (rem != 0) {
        unsigned int* t = crc32TableAndUnzipVersion;
        do {
            unsigned char bb = data[0];
            unsigned int idx = (c ^ bb) & 0xFF;
            unsigned int v = t[idx];
            unsigned int sh = c >> 8;
            c = sh ^ v;
            data += 1;
            rem -= 1;
        } while (rem != 0);
    }
    return c ^ 0xFFFFFFFFu;
}

// Create an archive input context for a supported memory or callback source.
void* ArchiveReader_CreateInputContext(void* a, void* b, unsigned int c, void* out) {
    if (c != 1 && c != 2 && c != 3) {
        *(unsigned int*)out = 0x10000;
        return 0;
    }
    unsigned int zero = 0;
    *(unsigned int*)out = 0;
    void* obj = __nw__FUl(0x20);
    ((unsigned char*)obj)[0] = 0;
    ((unsigned char*)obj)[1] = 1;
    ((unsigned char*)obj)[0x10] = 0;
    ((ArchiveInputView*)obj)->input = a;
    ((ArchiveInputView*)obj)->callbacks = b;
    ((ArchiveInputView*)obj)->position = 0;
    ((ArchiveInputView*)obj)->callbackPosition = 0;
    *(unsigned int*)out = 0;
    return obj;
}

#pragma push
#pragma use_lmw_stmw on
// Assemble four little-endian bytes; callback read failure returns -1 and zero output.
// Requires position<=size. A short memory-only read does NOT signal failure here:
// the existing code reuses r30; callers must ensure four bytes remain for a valid value.
int ZipStream_ReadU32LE(void* self, unsigned int* out) {
    unsigned char* obj = (unsigned char*)self;
    unsigned int pos = ((ArchiveInputView*)obj)->position;
    unsigned int size = ((ArchiveInputView*)obj)->size;
    unsigned int n = 1;
    if (pos + 1 > size)
        n = size - pos;
    unsigned int r30 = n;
    unsigned char sb0;
    memcpy(&sb0, (unsigned char*)((ArchiveInputView*)obj)->input + pos, n);
    ((ArchiveInputView*)obj)->position = pos + n;
    int result;
    if (n == 1) {
        r30 = sb0;
        result = 0;
    } else {
        if (((ArchiveInputView*)obj)->usesCallbacks && ((ArchiveInputView*)obj)->readFailed)
            result = -1;
        else
            result = 0;
    }
    unsigned int acc = r30;
    if (result == 0) {
        pos = ((ArchiveInputView*)obj)->position;
        n = 1;
        if (pos + 1 > size)
            n = size - pos;
        unsigned char sb1;
        memcpy(&sb1, (unsigned char*)((ArchiveInputView*)obj)->input + pos, n);
        ((ArchiveInputView*)obj)->position = pos + n;
        if (n == 1) {
            r30 = sb1;
            result = 0;
        } else {
            if (((ArchiveInputView*)obj)->usesCallbacks && ((ArchiveInputView*)obj)->readFailed)
                result = -1;
            else
                result = 0;
        }
    }
    acc += r30 << 8;
    if (result == 0) {
        pos = ((ArchiveInputView*)obj)->position;
        n = 1;
        if (pos + 1 > size)
            n = size - pos;
        unsigned char sb2;
        memcpy(&sb2, (unsigned char*)((ArchiveInputView*)obj)->input + pos, n);
        ((ArchiveInputView*)obj)->position = pos + n;
        if (n == 1) {
            r30 = sb2;
            result = 0;
        } else {
            if (((ArchiveInputView*)obj)->usesCallbacks && ((ArchiveInputView*)obj)->readFailed)
                result = -1;
            else
                result = 0;
        }
    }
    acc += r30 << 16;
    if (result == 0) {
        pos = ((ArchiveInputView*)obj)->position;
        n = 1;
        if (pos + 1 > size)
            n = size - pos;
        unsigned char sb3;
        memcpy(&sb3, (unsigned char*)((ArchiveInputView*)obj)->input + pos, n);
        ((ArchiveInputView*)obj)->position = pos + n;
        if (n == 1) {
            r30 = sb3;
            result = 0;
        } else {
            if (((ArchiveInputView*)obj)->usesCallbacks && ((ArchiveInputView*)obj)->readFailed)
                result = -1;
            else
                result = 0;
        }
    }
    acc += r30 << 24;
    *out = (result == 0) ? acc : 0;
    return result;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Update Adler-32 in bounded chunks to avoid overflowing its two sums.
unsigned int Zlib_Adler32(unsigned int adler, const unsigned char* data, unsigned int len) {
    unsigned int a = adler & 0xFFFF;
    unsigned int b = adler >> 16;
    if (data == 0)
        return 1;
    unsigned int n = len;
    while (n > 0) {
        int k = (n < kAdlerChunkSize) ? (int)n : kAdlerChunkSize;
        n -= (unsigned int)k;
        while (k >= 16) {
            a += data[0];
            b += a;
            a += data[1];
            b += a;
            a += data[2];
            b += a;
            a += data[3];
            b += a;
            a += data[4];
            b += a;
            a += data[5];
            b += a;
            a += data[6];
            b += a;
            a += data[7];
            b += a;
            a += data[8];
            b += a;
            a += data[9];
            b += a;
            a += data[10];
            b += a;
            a += data[11];
            b += a;
            a += data[12];
            b += a;
            a += data[13];
            b += a;
            a += data[14];
            b += a;
            a += data[15];
            b += a;
            data += 16;
            k -= 16;
        }
        if (k != 0)
            do {
                a += *data++;
                b += a;
            } while (--k);
        a %= kAdlerModulus;
        b %= kAdlerModulus;
    }
    return (b << 16) | a;
}
#pragma pop

} // extern "C"
