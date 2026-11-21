// Translation unit 0x802CE7B8-0x802CF0B4 (1 function; original file edges are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

// Resumable DEFLATE decoder. Literal/length and distance tables are borrowed;
// blocks owns the circular output/history window. Input and output can pause at
// any state, retaining the partially decoded length/distance in Codes::sub.
enum InflateCodeMode {
    kStartCode = 0, kDecodeLength = 1, kReadLengthExtra = 2,
    kDecodeDistance = 3, kReadDistanceExtra = 4, kCopyMatch = 5,
    kEmitLiteral = 6, kDrainWindow = 7, kCodesDone = 8, kBadCode = 9
};
enum InflateStatus { kInflateOk = 0, kInflateEnd = 1, kInflateStreamError = -2, kInflateDataError = -3 };
enum InflateTableOperation {
    kExtraBitCountMask = 15, kLengthOrDistanceBase = 16,
    kEndOfBlock = 32, kTerminalOrInvalidCode = 64
};
enum {
    kMaximumMatchBytes = 258, kFastDecodeInputBytes = 10, kBitsPerInputByte = 8,
    kInvalidDistanceDiagnosticOffset = 0x1c
};

struct Codes {
    unsigned int mode; // InflateCodeMode; preserved across calls that need more data/space.
    unsigned int len; // Base match length, then length plus extra bits; remaining bytes while copying.
    union {
        struct {
            struct Huft *tree;
            unsigned int need; // Bits required to index the active table/subtable.
        } code;
        unsigned int lit; // Pending literal byte in kEmitLiteral.
        struct {
            unsigned int get; // Extra-bit count, reused for length and distance decoding.
            unsigned int dist; // Back-reference distance, in bytes, for kCopyMatch.
        } copy;
    } sub;
    unsigned char lbits; // Root literal/length table width, in bits.
    unsigned char dbits; // Root distance table width, in bits.
    struct Huft *ltree;
    struct Huft *dtree;
};
struct Huft {
    // exop encodes a literal (0), extra-bit base (16 | bit count), subtable
    // width (no bit 64), block end (64 | 32), or invalid entry (64 without 32).
    // bits is the number consumed at this table level; base is a value/offset.
    unsigned char exop;
    unsigned char bits;
    unsigned short pad;
    unsigned int base;
};
struct Blocks {
    unsigned int mode;
    Codes *codes;
    unsigned char pad8[0x14];
    unsigned int bitk; // Number of valid low-order bits in bitb.
    unsigned int bitb; // Buffered input bits; the next code starts at bit 0.
    unsigned char pad24[4];
    unsigned char *window;
    unsigned char *end;
    unsigned char *read;
    unsigned char *write;
};
// window..end is the wrap interval; read is the flush cursor, write the producer.
// When write precedes read, leave one byte free to distinguish full from empty.
struct Stream {
    unsigned char *next_in;
    unsigned int avail_in;
    unsigned int total_in;
    unsigned char pad[0xc];
    const char *msg;
};
extern int Zlib_InflateFastCodes(unsigned int, unsigned int, Huft *, Huft *, Blocks *, Stream *);
extern int Zlib_InflateFlushWindow(Blocks *, Stream *, int);
extern unsigned int inflateDecodeTableData[];
extern char inflateDiagnosticStrings[];

// Reload the producer cursor and contiguous free space after a flush.
inline void InflateLoadOutput(Blocks* blocks, unsigned char*& outputCursor, unsigned int& outputSpace) {
    outputCursor = blocks->write;
    outputSpace = (unsigned int)(outputCursor < blocks->read ? blocks->read - outputCursor - 1 : blocks->end - outputCursor);
}

// Load the stream's resumable bit/input state and the current output span.
inline void InflateLoadState(Blocks* blocks, Stream* stream, unsigned char*& inputCursor, unsigned int& inputBytes,
                             unsigned int& bitBuffer, unsigned int& bufferedBits, unsigned char*& outputCursor, unsigned int& outputSpace) {
    inputCursor = stream->next_in;
    inputBytes = stream->avail_in;
    bitBuffer = blocks->bitb;
    bufferedBits = blocks->bitk;
    InflateLoadOutput(blocks, outputCursor, outputSpace);
}

// Publish all consumed input and buffered output before calling another decoder or returning.
inline void InflateSaveState(Blocks* blocks, Stream* stream, unsigned char* inputCursor, unsigned int inputBytes,
                             unsigned int bitBuffer, unsigned int bufferedBits, unsigned char* outputCursor) {
    blocks->bitb = bitBuffer;
    blocks->bitk = bufferedBits;
    stream->avail_in = inputBytes;
    stream->total_in += inputCursor - stream->next_in;
    stream->next_in = inputCursor;
    blocks->write = outputCursor;
}

// Flush pending window bytes to the caller and reload the newly available output span.
inline void InflateFlushOutput(Blocks* blocks, Stream* stream, int& result, unsigned char*& outputCursor, unsigned int& outputSpace) {
    blocks->write = outputCursor;
    result = Zlib_InflateFlushWindow(blocks, stream, result);
    InflateLoadOutput(blocks, outputCursor, outputSpace);
}

// Wrap the producer only when the beginning of the window is not occupied by unread bytes.
inline void InflateWrapOutput(Blocks* blocks, unsigned char*& outputCursor, unsigned int& outputSpace) {
    if (outputCursor == blocks->end && blocks->read != blocks->window) {
        outputCursor = blocks->window;
        outputSpace = (unsigned int)(outputCursor < blocks->read ? blocks->read - outputCursor - 1 : blocks->end - outputCursor);
    }
}

// Consume low-order bits; DEFLATE input is accumulated least-significant bit first.
inline void InflateDropBits(unsigned int& bitBuffer, unsigned int& bufferedBits, unsigned int count) {
    bitBuffer >>= count;
    bufferedBits -= count;
}

// Emit one byte into a previously reserved output span.
inline void InflateWriteByte(unsigned char*& outputCursor, unsigned int& outputSpace, unsigned int value) {
    *outputCursor++ = (unsigned char)value;
    outputSpace--;
}

// Save cursors and flush on every decoder exit; partial code state stays resumable.
inline int InflateLeave(Blocks* blocks, Stream* stream, int result, unsigned char* inputCursor, unsigned int inputBytes,
                        unsigned int bitBuffer, unsigned int bufferedBits, unsigned char* outputCursor) {
    InflateSaveState(blocks, stream, inputCursor, inputBytes, bitBuffer, bufferedBits, outputCursor);
    return Zlib_InflateFlushWindow(blocks, stream, result);
}

#pragma use_lmw_stmw on
// Decode literals and overlapping back-references, pausing on input/output exhaustion.
// blocks/stream and their tables/window must be valid. result is the caller's pending flush status;
// returns 1 at block end, -3 for bad data, -2 for an invalid mode, otherwise the
// flush status (including need-more-input/output). No allocations or ownership transfers.
int Zlib_InflateCodes(Blocks *blocks, Stream *stream, int result) {
    unsigned int lookupBits;
    Huft *entry;
    unsigned int entryOperation;
    unsigned int bitBuffer;
    unsigned int bufferedBits;
    unsigned char *inputCursor;
    unsigned int inputBytes;
    unsigned char *outputCursor;
    unsigned int outputSpace;
    unsigned char *matchCursor;
    Codes *codes = blocks->codes;

    InflateLoadState(blocks, stream, inputCursor, inputBytes, bitBuffer, bufferedBits, outputCursor, outputSpace);
    while (1) switch (codes->mode) {
    case kStartCode:
        // Phase 1: use the bulk decoder only with its guaranteed input/output margin.
        if (outputSpace >= kMaximumMatchBytes && inputBytes >= kFastDecodeInputBytes) {
            InflateSaveState(blocks, stream, inputCursor, inputBytes, bitBuffer, bufferedBits, outputCursor);
            result = Zlib_InflateFastCodes(codes->lbits, codes->dbits, codes->ltree, codes->dtree, blocks, stream);
            InflateLoadState(blocks, stream, inputCursor, inputBytes, bitBuffer, bufferedBits, outputCursor, outputSpace);
            if (result != 0) {
                codes->mode = result == kInflateEnd ? kDrainWindow : kBadCode;
                break;
            }
        }
        codes->sub.code.need = codes->lbits;
        codes->sub.code.tree = codes->ltree;
        codes->mode = kDecodeLength;
        // Fall through: decode the first literal/length symbol without another loop.
    case kDecodeLength:
        // Phase 2: resolve literal, match length, subtable, block end, or invalid code.
        lookupBits = codes->sub.code.need;
        // Refill low-order bits; preserve this mode if the input stops here.
        while (bufferedBits < lookupBits) {
            if (inputBytes) result = kInflateOk;
            else { return InflateLeave(blocks, stream, result, inputCursor, inputBytes, bitBuffer, bufferedBits, outputCursor); }
            inputBytes--;
            bitBuffer |= (unsigned int)(*inputCursor++) << bufferedBits;
            bufferedBits += kBitsPerInputByte;
        }
        entry = codes->sub.code.tree + ((unsigned int)bitBuffer & inflateDecodeTableData[lookupBits]);
        InflateDropBits(bitBuffer, bufferedBits, entry->bits);
        entryOperation = (unsigned int)entry->exop;
        if (entryOperation == 0) {
            codes->sub.lit = entry->base;
            codes->mode = kEmitLiteral;
            break;
        }
        if (entryOperation & kLengthOrDistanceBase) {
            codes->sub.copy.get = entryOperation & kExtraBitCountMask;
            codes->len = entry->base;
            codes->mode = kReadLengthExtra;
            break;
        }
        if ((entryOperation & kTerminalOrInvalidCode) == 0) {
            codes->sub.code.need = entryOperation;
            codes->sub.code.tree = entry + entry->base;
            break;
        }
        if (entryOperation & kEndOfBlock) {
            codes->mode = kDrainWindow;
            break;
        }
        codes->mode = kBadCode;
        stream->msg = inflateDiagnosticStrings;
        result = kInflateDataError;
        return InflateLeave(blocks, stream, result, inputCursor, inputBytes, bitBuffer, bufferedBits, outputCursor);
    case kReadLengthExtra:
        // Complete the match length before reusing the union for the distance table.
        lookupBits = codes->sub.copy.get;
        // Refill low-order bits; preserve this mode if the input stops here.
        while (bufferedBits < lookupBits) {
            if (inputBytes) result = kInflateOk;
            else { return InflateLeave(blocks, stream, result, inputCursor, inputBytes, bitBuffer, bufferedBits, outputCursor); }
            inputBytes--;
            bitBuffer |= (unsigned int)(*inputCursor++) << bufferedBits;
            bufferedBits += kBitsPerInputByte;
        }
        codes->len += (unsigned int)bitBuffer & inflateDecodeTableData[lookupBits];
        InflateDropBits(bitBuffer, bufferedBits, lookupBits);
        codes->sub.code.need = codes->dbits;
        codes->sub.code.tree = codes->dtree;
        codes->mode = kDecodeDistance;
        // Fall through: length is complete and the distance table is selected.
    case kDecodeDistance:
        // Phase 3: decode a distance base/subtable, rejecting invalid terminal codes.
        lookupBits = codes->sub.code.need;
        // Refill low-order bits; preserve this mode if the input stops here.
        while (bufferedBits < lookupBits) {
            if (inputBytes) result = kInflateOk;
            else { return InflateLeave(blocks, stream, result, inputCursor, inputBytes, bitBuffer, bufferedBits, outputCursor); }
            inputBytes--;
            bitBuffer |= (unsigned int)(*inputCursor++) << bufferedBits;
            bufferedBits += kBitsPerInputByte;
        }
        entry = codes->sub.code.tree + ((unsigned int)bitBuffer & inflateDecodeTableData[lookupBits]);
        InflateDropBits(bitBuffer, bufferedBits, entry->bits);
        entryOperation = (unsigned int)entry->exop;
        if (entryOperation & kLengthOrDistanceBase) {
            codes->sub.copy.get = entryOperation & kExtraBitCountMask;
            codes->sub.copy.dist = entry->base;
            codes->mode = kReadDistanceExtra;
            break;
        }
        if ((entryOperation & kTerminalOrInvalidCode) == 0) {
            codes->sub.code.need = entryOperation;
            codes->sub.code.tree = entry + entry->base;
            break;
        }
        codes->mode = kBadCode;
        stream->msg = inflateDiagnosticStrings + kInvalidDistanceDiagnosticOffset;
        result = kInflateDataError;
        return InflateLeave(blocks, stream, result, inputCursor, inputBytes, bitBuffer, bufferedBits, outputCursor);
    case kReadDistanceExtra:
        lookupBits = codes->sub.copy.get;
        // Refill low-order bits; preserve this mode if the input stops here.
        while (bufferedBits < lookupBits) {
            if (inputBytes) result = kInflateOk;
            else { return InflateLeave(blocks, stream, result, inputCursor, inputBytes, bitBuffer, bufferedBits, outputCursor); }
            inputBytes--;
            bitBuffer |= (unsigned int)(*inputCursor++) << bufferedBits;
            bufferedBits += kBitsPerInputByte;
        }
        codes->sub.copy.dist += (unsigned int)bitBuffer & inflateDecodeTableData[lookupBits];
        InflateDropBits(bitBuffer, bufferedBits, lookupBits);
        codes->mode = kCopyMatch;
        // Fall through: distance and length are now complete, both measured in bytes.
    case kCopyMatch:
        // Phase 4: reconstruct a match from the circular history. Copy one byte at
        // a time because source and destination can overlap (e.g. repeated bytes).
        matchCursor = outputCursor - codes->sub.copy.dist;
        while (matchCursor < blocks->window) matchCursor += blocks->end - blocks->window;
        while (codes->len) {
            if (outputSpace == 0) {
                InflateWrapOutput(blocks, outputCursor, outputSpace);
                if (outputSpace == 0) {
                    InflateFlushOutput(blocks, stream, result, outputCursor, outputSpace);
                    InflateWrapOutput(blocks, outputCursor, outputSpace);
                    if (outputSpace == 0) { return InflateLeave(blocks, stream, result, inputCursor, inputBytes, bitBuffer, bufferedBits, outputCursor); }
                }
            }
            result = kInflateOk;
            InflateWriteByte(outputCursor, outputSpace, *matchCursor++);
            if (matchCursor == blocks->end) matchCursor = blocks->window;
            codes->len--;
        }
        codes->mode = kStartCode;
        break;
    case kEmitLiteral:
        // A literal also waits for output space without consuming its pending value.
        if (outputSpace == 0) {
            InflateWrapOutput(blocks, outputCursor, outputSpace);
            if (outputSpace == 0) {
                InflateFlushOutput(blocks, stream, result, outputCursor, outputSpace);
                InflateWrapOutput(blocks, outputCursor, outputSpace);
                if (outputSpace == 0) { return InflateLeave(blocks, stream, result, inputCursor, inputBytes, bitBuffer, bufferedBits, outputCursor); }
            }
        }
        result = kInflateOk;
        InflateWriteByte(outputCursor, outputSpace, codes->sub.lit);
        codes->mode = kStartCode;
        break;
    case kDrainWindow:
        // Phase 5: return a whole look-ahead byte, then drain all pending output
        // before reporting block completion. A partial drain stays in this mode.
        if (bufferedBits > kBitsPerInputByte - 1) { bufferedBits -= kBitsPerInputByte; inputBytes++; inputCursor--; }
        InflateFlushOutput(blocks, stream, result, outputCursor, outputSpace);
        if (blocks->read != blocks->write) return InflateLeave(blocks, stream, result, inputCursor, inputBytes, bitBuffer, bufferedBits, outputCursor);
        codes->mode = kCodesDone;
        // Fall through: no output remains buffered for this block.
    case kCodesDone:
        result = kInflateEnd;
        return InflateLeave(blocks, stream, result, inputCursor, inputBytes, bitBuffer, bufferedBits, outputCursor);
    case kBadCode:
        result = kInflateDataError;
        return InflateLeave(blocks, stream, result, inputCursor, inputBytes, bitBuffer, bufferedBits, outputCursor);
    default:
        result = kInflateStreamError;
        return InflateLeave(blocks, stream, result, inputCursor, inputBytes, bitBuffer, bufferedBits, outputCursor);
    }
}

} // extern "C"
