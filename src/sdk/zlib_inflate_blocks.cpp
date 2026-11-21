extern "C" {
extern unsigned char inflateDecodeTableData[];
}
// Translation unit 0x802CF0B4-0x802D0648 (3 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {
extern char s_inflateTableRegionBase[];
struct InflateDiagnostics {
    char invalid_literal_length_code[28];
    char invalid_distance_code[22];
    char invalid_block_type[19];
    char invalid_stored_block_lengths[29];
    char too_many_length_or_distance_symbols[36];
    char oversubscribed_dynamic_bit_lengths_tree[40];
    char incomplete_dynamic_bit_lengths_tree[36];
    char invalid_bit_length_repeat[26];
    char oversubscribed_literal_length_tree[35];
    char incomplete_literal_length_tree[31];
    char oversubscribed_distance_tree[29];
    char incomplete_distance_tree[25];
    char empty_distance_tree_with_lengths[33];
    char text_1_1_3[6];
    char unknown_compression_method[27];
    char invalid_window_size[20];
    char incorrect_header_check[23];
    char need_dictionary[16];
    char incorrect_data_check[21];
    char localizedText[5];
    char localizedText_2[5];
    char localizedText_3[5];
    char localizedText_4[5];
    char UT[3];
    char trailingPadding[7];
};
extern InflateDiagnostics inflateDiagnosticStrings;

// Block mode owns the interpretation of modeStateSlot (+4):
//   kStoredLength/kStoredData: remaining stored bytes (LEN, then decremented).
//   kDynamicHeader/kCodeLengthTree/kDynamicTrees: packed HLIT/HDIST/HCLEN header.
//   kDecodeCodes: owned Codes* from stream->alloc, freed on end-of-block.
// Other modes do not read the word. lengthsAddress separately owns the dynamic
// lengths until tree construction succeeds or malformed data frees it.
// Table roots borrow huffmanWorkspace; windowEnd is one-past, windowRead is the
// oldest unflushed byte. These unsigned address words retain the original 32-bit ABI.
struct DecodeState {
    unsigned mode;
    unsigned modeStateSlot; // Mode-dependent state slot: stored-byte count, dynamic header, or codes-state address.
    unsigned lengthIndex;
    unsigned lengthsAddress; // Address of the dynamic-tree code-length array.
    unsigned bitLengthRootBits;
    unsigned bitLengthTreeAddress; // Address of the temporary code-length decode table.
    unsigned lastBlock;
    unsigned bitCount;
    unsigned bitBuffer;
    unsigned char* huffmanWorkspace;
    unsigned char* windowBegin;
    unsigned windowEnd;
    unsigned windowRead;
    unsigned char* windowWrite;
    unsigned checksumFunction;
    unsigned checksum;
};
// Prefix of z_stream, through allocator context. Output fields are maintained by flush.
struct DecodeStream {
    unsigned char* next;
    unsigned inputRemaining, total;
    unsigned char* nextOutput;
    unsigned outputAvailable, outputTotal;
    const char* msg;
    void* blockState;
    void* (*alloc)(void*, unsigned, unsigned);
    void (*free)(void*, void*);
    void* ctx;
};
extern int Zlib_InflateFlushWindow(DecodeState*, DecodeStream*, int);

typedef DecodeStream Obj;

extern int Zlib_BuildHuffmanDecodeTable(void*, unsigned, unsigned, const char*, const char*, void*, void*, void*, unsigned*, unsigned*);

#pragma use_lmw_stmw on
extern void* memcpy(void*, const void*, unsigned);
extern int Zlib_InflateCodes(DecodeState*, DecodeStream*, int);
// ABI argument map (addresses remain int-sized; all outputs are caller storage):
//   literalCount, distanceCount       input symbol counts
//   lengthsAddress                   unsigned[literalCount + distanceCount]
//   literalBitsAddress/distanceBitsAddress   unsigned* requested/actual root widths
//   literalTreeAddress/distanceTreeAddress   CodeEntry** roots into workspace
//   workspaceAddress                 CodeEntry[1440], borrowed mutable arena
//   stream                           allocator/context and diagnostic destination
int Zlib_InflateBuildDynamicTrees(int, int, int, int, int, int, int, int, Obj*);
typedef DecodeStream BlockStream;
// An 8-byte decode entry. bits consumes bits at THIS table level; val is a
// literal/symbol, a length/distance base, or a relative subtable displacement.
// op encoding (test terminal flags before treating it as a subtable width):
//   0x00 literal/simple symbol; 0x50+N base with N following extra bits;
//   0x60 end-of-block; 0xC0 invalid/unassigned code;
//   1..15 subtable index width, val measured in entries FROM THE LINK ENTRY.
// bits and op are bytes; pad is not a semantic field of the Huffman code.
struct CodeEntry { unsigned char op, bits; unsigned short pad; unsigned val; };
// Address-relative view across adjacent zlib constants, beyond the base label's own extent.
// Only field offsets are used; this does not declare or resize any shared data object.
struct InflateTablePool {
    char prefix[0xa8];
    unsigned bitMasks[18];
    CodeEntry fixedLiteralLength[512];
    CodeEntry fixedDistance[32];
    unsigned codeLengthOrder[19];
    char beforeLengthBases[0x30];
    unsigned lengthBases[31];
    unsigned lengthExtraBits[32];
    unsigned distanceBases[30];
    unsigned distanceExtraBits[30];
};
#define INFLATE_OFFSET(field) ((int)&((InflateTablePool*)0)->field)
enum InflateBlockMode {
    kBlockType = 0, kStoredLength = 1, kStoredData = 2, kDynamicHeader = 3,
    kCodeLengthTree = 4, kDynamicTrees = 5, kDecodeCodes = 6,
    kDrainWindow = 7, kBlockDone = 8, kBlockBad = 9
};
enum ZlibResult { kZOk = 0, kZStreamEnd = 1, kZStreamError = -2, kZDataError = -3, kZMemoryError = -4, kZBufferError = -5 };
enum { kMaxCodeBits = 15, kCodeLengthSymbolCount = 19, kHuffmanWorkspaceEntries = 1440, kCodesStart = 0 };
enum DeflateFieldWidth {
    kBlockHeaderBits = 3, kStoredLengthAndComplementBits = 32,
    kDynamicCountsBits = 14, kEncodedCodeLengthBits = 3
};
// RFC 1951 dynamic header: HLIT[4:0], HDIST[9:5], HCLEN[13:10].
// Count biases are symbols, root widths are bits, workspace sizes are entries.
enum DeflateDynamicHeader {
    kDynamicHeaderMask = 0x3FFF, kDynamicCountMask = 31,
    kDistanceCountShift = 5, kCodeLengthCountShift = 10,
    kLiteralCountBias = 257, kDistanceCountBias = 1,
    kCombinedCountBias = 258, kCodeLengthCountBias = 4,
    kLargestCountField = 29, kCodeLengthValueMask = 7,
    kCodeLengthRootBits = 7, kLiteralRootBits = 9, kDistanceRootBits = 6,
    kFixedDistanceRootBits = 5, kLiteralSortScratchSymbols = 288
};
enum DeflateRepeatCode {
    kRepeatPreviousLength = 16, kRepeatZeroShort = 17, kRepeatZeroLong = 18,
    kLongRepeatExtraBits = 7, kLongRepeatMinimum = 11, kShortRepeatMinimum = 3,
    kRepeatCodeToExtraBitBias = 14
};
enum DeflateByteLayout { kStoredLengthMask = 0xffff, kStoredLengthBits = 16, kByteBits = 8, kCodesStateBytes = 28 };

enum DeflateBlockEncoding { kStoredEncoding = 0, kFixedEncoding = 1, kDynamicEncoding = 2, kReservedEncoding = 3 };
enum HuffmanOperation { kHuffLiteral = 0, kHuffBaseWithExtra = 0x50, kHuffEndOfBlock = 0x60, kHuffInvalid = 0xC0 };
struct Codes {
    unsigned mode;
    char pad[12];
    unsigned char literalRootBits, distanceRootBits;
    char pad2[2];
    void* literalTree;
    void* distanceTree;
};
// Leave one byte free before the read cursor. An inline function changes code generation.
#define WINDOW_SPACE(block, cursor) \
    ((cursor) < (unsigned char*)(block)->windowRead \
        ? (unsigned char*)(block)->windowRead - (cursor) - 1 \
        : (unsigned char*)(block)->windowEnd - (cursor))
// total counts newly consumed input, so read the old next BEFORE updating next.
inline void CommitInflateCursors(DecodeState* block, DecodeStream* stream,
        unsigned bitBuffer, unsigned bufferedBits, unsigned inputRemaining,
        unsigned char* inputCursor, unsigned char* windowCursor) {
    block->bitBuffer = bitBuffer;
    block->bitCount = bufferedBits;
    stream->inputRemaining = inputRemaining;
    stream->total += inputCursor - stream->next;
    stream->next = inputCursor;
    block->windowWrite = windowCursor;
}

// Input exhaustion and window stalls below commit all cursors and flush explicitly.
// Resume a DEFLATE block, preserving bit/input/window cursors whenever input or output stalls.
// block/input are borrowed mutable decoder state. Return zlib status (enum above);
// result is the incoming stall status, promoted to OK when an input byte is read.
// No input/output ownership transfer. Window/table allocations use input->alloc/free.
// Unread bits occupy the low bufferedBits; new bytes append above them and
// decoding consumes the least-significant bits first. Each early
// return commits bitBuffer/count, input remaining/total/next and windowWrite.
// CommitInflateCursors centralizes publication; each suspension still names its
// own flush/status. Retrying the current mode resumes from those saved cursors.
// Memory failure retains the dynamic-length array for retry; malformed input
// frees it and enters kBlockBad. Codes-state allocation transfers to modeStateSlot.
// Resume map (the switch stores its next mode BEFORE a possible suspension):
//   TYPE         needs 3 bits; chooses STORED_LENGTH / DYNAMIC_HEADER / CODES.
//   STORED_LENGTH needs byte-aligned LEN+NLEN; STORED_DATA copies bounded chunks.
//   DYNAMIC_HEADER allocates lengths; CODE_LENGTH_TREE builds the temporary decoder.
//   DYNAMIC_TREES fills lengths, builds both roots, allocates Codes, frees lengths.
//   CODES        hands off committed cursors; non-end status suspends in CODES.
//   DRAIN        flushes until read==write; DONE/BAD are terminal statuses.
// Input/space exhaustion keeps the current phase. Allocation errors are retryable;
// data errors set BAD after releasing any dynamic lengths owned by that path.
// Deliberate fallthrough: DYNAMIC_HEADER -> CODE_LENGTH_TREE -> DYNAMIC_TREES
// -> CODES -> DRAIN -> DONE. break returns to the outer switch, not to the caller.
// State progression: header -> stored bytes OR fixed codes OR dynamic counts /
// code-length tree / literal-distance trees -> codes -> drain -> done.
// Dynamic states deliberately fall through once their current phase completes.
int Zlib_InflateBlocks(DecodeState* block, DecodeStream* input, int result) {
    unsigned* bitMasks;
    unsigned* codeLengthOrder;
    char* tableBytes;
    Codes* codes;
    unsigned mode, literalCountMinus257, distanceCountMinus1;
    unsigned* codeLengths;
    void* workspace;
    void* temporaryWork;
    unsigned totalLengths;
    CodeEntry* entry;
    unsigned lengthCode, entryBits, repeatExtraBits, repeatCount, lengthIndex, repeatedLength;
    unsigned literalBits, distanceBits;
    void* literalTable;
    void* distanceTable;
    unsigned literalRootBits, distanceRootBits;
    void* literalTree;
    void* distanceTree;
    unsigned workspaceUsed;
    BlockStream* stream;
    unsigned required;
    int treeResult;
    unsigned windowAvailable;
    register unsigned bitBuffer;
    register unsigned bufferedBits;
    register unsigned char* inputCursor;
    register unsigned inputRemaining;
    register unsigned char* windowCursor;
    stream=(BlockStream*)input;
    tableBytes=s_inflateTableRegionBase;
    windowCursor=block->windowWrite;
    inputCursor=stream->next;
    inputRemaining=stream->inputRemaining;
    bitBuffer=block->bitBuffer;
    bufferedBits=block->bitCount;
    windowAvailable=WINDOW_SPACE(block, windowCursor);
    codeLengthOrder=(unsigned*)(((char*)(tableBytes + INFLATE_OFFSET(codeLengthOrder))));
    bitMasks=(unsigned*)(((char*)(tableBytes + INFLATE_OFFSET(bitMasks))));
    for(;;) switch(block->mode) {
    case kBlockType:
        {
            // BFINAL:1 then BTYPE:2. Stored blocks discard remaining bits to a byte boundary.
            while (bufferedBits < (unsigned)(kBlockHeaderBits)) {
                if (inputRemaining) result = 0;
                else {
                    CommitInflateCursors(block, stream, bitBuffer, bufferedBits, inputRemaining, inputCursor, windowCursor);
                    return Zlib_InflateFlushWindow(block, input, (result));
                }
                --inputRemaining;
                bitBuffer |= (unsigned)*inputCursor++ << bufferedBits;
                bufferedBits += kByteBits;
            }
            block->lastBlock=bitBuffer&1;
            switch((bitBuffer>>1)&3) {
            case kStoredEncoding:
                block->mode=kStoredLength;
                bitBuffer>>=3;
                bufferedBits-=3;
                bitBuffer>>=(bufferedBits&7);
                bufferedBits-=bufferedBits&7;
                break;
            case kFixedEncoding:
                {
                    codes=(Codes*)stream->alloc(stream->ctx,1,kCodesStateBytes);
                    if(codes) {
                        codes->mode=kCodesStart;
                        codes->literalRootBits=kLiteralRootBits;
                        codes->distanceRootBits=kFixedDistanceRootBits;
                        codes->literalTree=((char*)(tableBytes + INFLATE_OFFSET(fixedLiteralLength)));
                        codes->distanceTree=((char*)(tableBytes + INFLATE_OFFSET(fixedDistance)));
                    }
                    block->modeStateSlot=(unsigned)codes;
                    if(!codes) {
                        CommitInflateCursors(block, stream, bitBuffer, bufferedBits, inputRemaining, inputCursor, windowCursor);
                        return Zlib_InflateFlushWindow(block, input, (kZMemoryError));
                    }
                    block->mode=kDecodeCodes;
                    bitBuffer>>=3;
                    bufferedBits-=3;
                    break;
                }
            case kDynamicEncoding:
                block->mode=kDynamicHeader;
                bitBuffer>>=3;
                bufferedBits-=3;
                break;
            case kReservedEncoding:
                block->mode=kBlockBad;
                stream->msg=inflateDiagnosticStrings.invalid_block_type;
                bitBuffer>>=3;
                bufferedBits-=3;
                {
                    CommitInflateCursors(block, stream, bitBuffer, bufferedBits, inputRemaining, inputCursor, windowCursor);
                    return Zlib_InflateFlushWindow(block, input, (kZDataError));
                }
            }
            break;
        }
    case kStoredLength:
        {
            // Byte-aligned LEN:16 and NLEN:16 must be one's complements.
            while (bufferedBits < (unsigned)(kStoredLengthAndComplementBits)) {
                if (inputRemaining) result = 0;
                else {
                    CommitInflateCursors(block, stream, bitBuffer, bufferedBits, inputRemaining, inputCursor, windowCursor);
                    return Zlib_InflateFlushWindow(block, input, (result));
                }
                --inputRemaining;
                bitBuffer |= (unsigned)*inputCursor++ << bufferedBits;
                bufferedBits += kByteBits;
            }
            if((bitBuffer&kStoredLengthMask)!=(~bitBuffer>>kStoredLengthBits)) {
                block->mode=kBlockBad;
                stream->msg=inflateDiagnosticStrings.invalid_stored_block_lengths;
                {
                    CommitInflateCursors(block, stream, bitBuffer, bufferedBits, inputRemaining, inputCursor, windowCursor);
                    return Zlib_InflateFlushWindow(block, input, (kZDataError));
                }
            }
            block->modeStateSlot=bitBuffer&kStoredLengthMask;
            bufferedBits=0;
            bitBuffer=0;
            mode=kBlockType;
            if(block->modeStateSlot) mode=kStoredData;
            else {
                mode=kBlockType;
                if(block->lastBlock) mode=kDrainWindow;
            }
            block->mode=mode;
            break;
        }
    case kStoredData:
        {
            // Copy min(stored bytes, input remaining, contiguous window room).
            // Full window: wrap if legal, flush, retry wrap, then suspend if still full.
            if(!inputRemaining) {
                CommitInflateCursors(block, stream, bitBuffer, bufferedBits, inputRemaining, inputCursor, windowCursor);
                return Zlib_InflateFlushWindow(block, input, (result));
            }
            if (!windowAvailable) {
                if (windowCursor == (unsigned char*)block->windowEnd && (unsigned char*)block->windowRead != block->windowBegin) {
                    windowCursor = block->windowBegin;
                    windowAvailable = WINDOW_SPACE(block, windowCursor);
                }
                if (!windowAvailable) {
                    block->windowWrite = windowCursor;
                    result = Zlib_InflateFlushWindow(block, input, result);
                    windowCursor = block->windowWrite;
                    windowAvailable = WINDOW_SPACE(block, windowCursor);
                    if (windowCursor == (unsigned char*)block->windowEnd && (unsigned char*)block->windowRead != block->windowBegin) {
                        windowCursor = block->windowBegin;
                        windowAvailable = WINDOW_SPACE(block, windowCursor);
                    }
                    if (!windowAvailable) {
                        CommitInflateCursors(block, stream, bitBuffer, bufferedBits, inputRemaining, inputCursor, windowCursor);
                        return Zlib_InflateFlushWindow(block, input, (result));
                    }
                }
            }
            required=block->modeStateSlot;
            result=0;
            if(required>inputRemaining) required=inputRemaining;
            if(required>windowAvailable) required=windowAvailable;
            memcpy(windowCursor,inputCursor,required);
            inputCursor+=required;
            inputRemaining-=required;
            windowCursor+=required;
            windowAvailable-=required;
            if((block->modeStateSlot-=required)==0) {
                mode=kBlockType;
                if(block->lastBlock) mode=kDrainWindow;
                block->mode=mode;
            }
            break;
        }
    case kDynamicHeader:
        {
            // HLIT:5 => literal/length count +257; HDIST:5 => distance count +1;
            // HCLEN:4 => code-length alphabet count +4. Packed header is retained
            // while later states fill the lengths array in DEFLATE permutation order.
            while (bufferedBits < (unsigned)(kDynamicCountsBits)) {
                if (inputRemaining) result = 0;
                else {
                    CommitInflateCursors(block, stream, bitBuffer, bufferedBits, inputRemaining, inputCursor, windowCursor);
                    return Zlib_InflateFlushWindow(block, input, (result));
                }
                --inputRemaining;
                bitBuffer |= (unsigned)*inputCursor++ << bufferedBits;
                bufferedBits += kByteBits;
            }
            block->modeStateSlot=bitBuffer&kDynamicHeaderMask;
            literalCountMinus257=block->modeStateSlot&kDynamicCountMask;
            if(literalCountMinus257>kLargestCountField || ((block->modeStateSlot>>kDistanceCountShift)&kDynamicCountMask)>kLargestCountField) {
                block->mode=kBlockBad;
                stream->msg=inflateDiagnosticStrings.too_many_length_or_distance_symbols;
                {
                    CommitInflateCursors(block, stream, bitBuffer, bufferedBits, inputRemaining, inputCursor, windowCursor);
                    return Zlib_InflateFlushWindow(block, input, (kZDataError));
                }
            }
            distanceCountMinus1=(block->modeStateSlot>>kDistanceCountShift)&kDynamicCountMask;
            block->lengthsAddress=(unsigned)stream->alloc(stream->ctx,literalCountMinus257+distanceCountMinus1+kCombinedCountBias,4);
            if(!block->lengthsAddress) {
                CommitInflateCursors(block, stream, bitBuffer, bufferedBits, inputRemaining, inputCursor, windowCursor);
                return Zlib_InflateFlushWindow(block, input, (kZMemoryError));
            }
            block->lengthIndex=0;
            bitBuffer>>=14;
            bufferedBits-=14;
            block->mode=kCodeLengthTree;
        }
    // Phase: read the permuted code-length alphabet before decoding the lengths array.
    case kCodeLengthTree:
        {
            // Read HCLEN+4 three-bit lengths, zero the other symbols (19 total),
            // then build the small tree that decodes the actual tree lengths.
            while(block->lengthIndex < (block->modeStateSlot>>kCodeLengthCountShift)+kCodeLengthCountBias) {
                while (bufferedBits < (unsigned)(kEncodedCodeLengthBits)) {
                    if (inputRemaining) result = 0;
                    else {
                        CommitInflateCursors(block, stream, bitBuffer, bufferedBits, inputRemaining, inputCursor, windowCursor);
                        return Zlib_InflateFlushWindow(block, input, (result));
                    }
                    --inputRemaining;
                    bitBuffer |= (unsigned)*inputCursor++ << bufferedBits;
                    bufferedBits += kByteBits;
                }
                ((unsigned*)block->lengthsAddress)[codeLengthOrder[block->lengthIndex++]]=bitBuffer&kCodeLengthValueMask;
                bitBuffer>>=3;
                bufferedBits-=3;
            }
            while(block->lengthIndex<kCodeLengthSymbolCount) ((unsigned*)block->lengthsAddress)[codeLengthOrder[block->lengthIndex++]]=0;
            workspace=block->huffmanWorkspace;
            block->bitLengthRootBits=kCodeLengthRootBits;
            codeLengths=(unsigned*)block->lengthsAddress;
            workspaceUsed=0;
            temporaryWork=stream->alloc(stream->ctx,kCodeLengthSymbolCount,4);
            if(!temporaryWork) treeResult=-4;
            else {
                treeResult=Zlib_BuildHuffmanDecodeTable(codeLengths,kCodeLengthSymbolCount,kCodeLengthSymbolCount,0,0,&block->bitLengthTreeAddress,&block->bitLengthRootBits,workspace,&workspaceUsed,(unsigned*)temporaryWork);
                if(treeResult==-3) stream->msg=inflateDiagnosticStrings.oversubscribed_dynamic_bit_lengths_tree;
                else if(treeResult==-5 || block->bitLengthRootBits==0) {
                    stream->msg=inflateDiagnosticStrings.incomplete_dynamic_bit_lengths_tree;
                    treeResult=-3;
                }
                stream->free(stream->ctx,temporaryWork);
            }
            if(treeResult) {
                if(treeResult==-3) {
                    stream->free(stream->ctx,(void*)block->lengthsAddress);
                    block->mode=kBlockBad;
                }
                {
                    CommitInflateCursors(block, stream, bitBuffer, bufferedBits, inputRemaining, inputCursor, windowCursor);
                    return Zlib_InflateFlushWindow(block, input, (treeResult));
                }
            }
            block->lengthIndex=0;
            block->mode=kDynamicTrees;
        }
    // Phase: expand literal/repeat symbols, then commit the two final tree roots.
    case kDynamicTrees:
        {
            // Codes 0..15 are literal lengths. 16 repeats the previous length
            // 3..6 times (2 extra bits; illegal at index 0); 17 repeats zero
            // 3..10 times (3 bits); 18 repeats zero 11..138 times (7 bits).
            // A repeat may cross the literal/distance boundary but not totalLengths.
            while((lengthIndex=block->lengthIndex) < (totalLengths=kCombinedCountBias+(block->modeStateSlot&kDynamicCountMask)+((block->modeStateSlot>>kDistanceCountShift)&kDynamicCountMask))) {
                required=block->bitLengthRootBits;
                while (bufferedBits < (unsigned)(required)) {
                    if (inputRemaining) result = 0;
                    else {
                        CommitInflateCursors(block, stream, bitBuffer, bufferedBits, inputRemaining, inputCursor, windowCursor);
                        return Zlib_InflateFlushWindow(block, input, (result));
                    }
                    --inputRemaining;
                    bitBuffer |= (unsigned)*inputCursor++ << bufferedBits;
                    bufferedBits += kByteBits;
                }
                entry=(CodeEntry*)block->bitLengthTreeAddress+(bitBuffer&bitMasks[required]);
                lengthCode=entry->val;
                entryBits=entry->bits;
                if(lengthCode<kRepeatPreviousLength) {
                    bitBuffer>>=entryBits;
                    bufferedBits-=entryBits;
                    ((unsigned*)block->lengthsAddress)[block->lengthIndex++]=lengthCode;
                }
                else {
                    repeatExtraBits=lengthCode==kRepeatZeroLong ? kLongRepeatExtraBits : lengthCode-kRepeatCodeToExtraBitBias;
                    repeatCount=lengthCode==kRepeatZeroLong ? kLongRepeatMinimum : kShortRepeatMinimum;
                    while (bufferedBits < (unsigned)(entryBits+repeatExtraBits)) {
                        if (inputRemaining) result = 0;
                        else {
                            CommitInflateCursors(block, stream, bitBuffer, bufferedBits, inputRemaining, inputCursor, windowCursor);
                            return Zlib_InflateFlushWindow(block, input, (result));
                        }
                        --inputRemaining;
                        bitBuffer |= (unsigned)*inputCursor++ << bufferedBits;
                        bufferedBits += kByteBits;
                    }
                    bitBuffer>>=entryBits;
                    bufferedBits-=entryBits;
                    repeatCount+=bitBuffer&bitMasks[repeatExtraBits];
                    bitBuffer>>=repeatExtraBits;
                    bufferedBits-=repeatExtraBits;
                    if(lengthIndex+repeatCount>totalLengths || (lengthCode==kRepeatPreviousLength && lengthIndex<1)) {
                        stream->free(stream->ctx,(void*)block->lengthsAddress);
                        block->mode=kBlockBad;
                        stream->msg=inflateDiagnosticStrings.invalid_bit_length_repeat;
                        {
                            CommitInflateCursors(block, stream, bitBuffer, bufferedBits, inputRemaining, inputCursor, windowCursor);
                            return Zlib_InflateFlushWindow(block, input, (kZDataError));
                        }
                    }
                    repeatedLength=lengthCode==kRepeatPreviousLength ? ((unsigned*)block->lengthsAddress)[lengthIndex-1] : 0;
                    do {
                        ((unsigned*)block->lengthsAddress)[lengthIndex++]=repeatedLength;
                    }
                    while(--repeatCount);
                    block->lengthIndex=lengthIndex;
                }
            }
            // Temporary length decoder is no longer used. Its workspace may now be
            // overwritten by the literal/distance tables; lengths remain owned until Codes exists.
            block->bitLengthTreeAddress=0;
            literalRootBits=kLiteralRootBits;
            distanceRootBits=kDistanceRootBits;
            treeResult=Zlib_InflateBuildDynamicTrees(
                (block->modeStateSlot&kDynamicCountMask)+kLiteralCountBias, // input literal/length count
                ((block->modeStateSlot>>kDistanceCountShift)&kDynamicCountMask)+kDistanceCountBias, // input distance count
                (int)block->lengthsAddress, // input lengths, literal/length then distance
                (int)&literalRootBits, (int)&distanceRootBits, // in/out widths in bits
                (int)&literalTree, (int)&distanceTree, // output roots, borrowing workspace
                (int)block->huffmanWorkspace, (Obj*)stream);
            if(treeResult) {
                if((unsigned)treeResult==(unsigned)-3) {
                    stream->free(stream->ctx,(void*)block->lengthsAddress);
                    block->mode=kBlockBad;
                }
                {
                    CommitInflateCursors(block, stream, bitBuffer, bufferedBits, inputRemaining, inputCursor, windowCursor);
                    return Zlib_InflateFlushWindow(block, input, (treeResult));
                }
            }
            distanceTable=distanceTree;
            literalTable=literalTree;
            distanceBits=distanceRootBits;
            literalBits=literalRootBits;
            codes=(Codes*)stream->alloc(stream->ctx,1,kCodesStateBytes);
            if(codes) {
                codes->mode=kCodesStart;
                codes->literalRootBits=literalBits;
                codes->distanceRootBits=distanceBits;
                codes->literalTree=literalTable;
                codes->distanceTree=distanceTable;
            }
            if(!codes) {
                CommitInflateCursors(block, stream, bitBuffer, bufferedBits, inputRemaining, inputCursor, windowCursor);
                return Zlib_InflateFlushWindow(block, input, (kZMemoryError));
            }
            block->modeStateSlot=(unsigned)codes;
            stream->free(stream->ctx,(void*)block->lengthsAddress);
            block->mode=kDecodeCodes;
        }
        // Fall through: final roots and Codes are ready; reload is unnecessary here.
    case kDecodeCodes:
        {
            // Hand off committed cursors to the literal/match decoder. On block
            // end free only its codes-state object; reload cursors before continuing.
            CommitInflateCursors(block, stream, bitBuffer, bufferedBits, inputRemaining, inputCursor, windowCursor);
            treeResult=Zlib_InflateCodes(block,input,result);
            if(treeResult!=1) return Zlib_InflateFlushWindow(block,input,treeResult);
            result=0;
            stream->free(stream->ctx,(void*)block->modeStateSlot);
            windowCursor=block->windowWrite;
            inputCursor=stream->next;
            inputRemaining=stream->inputRemaining;
            bitBuffer=block->bitBuffer;
            bufferedBits=block->bitCount;
            windowAvailable=WINDOW_SPACE(block, windowCursor);
            if(!block->lastBlock) {
                block->mode=kBlockType;
                break;
            }
            block->mode=kDrainWindow;
        }
        // Fall through: last block ended, but pending window output still belongs to the stream.
    case kDrainWindow:
        // Last block still has pending output: done is reached only after read==write.
        block->windowWrite=windowCursor;
        result=Zlib_InflateFlushWindow(block,input,result);
        windowCursor=block->windowWrite;
        if((unsigned char*)block->windowRead!=windowCursor) {
            CommitInflateCursors(block, stream, bitBuffer, bufferedBits, inputRemaining, inputCursor, windowCursor);
            return Zlib_InflateFlushWindow(block, input, (result));
        }
        block->mode=kBlockDone;
        // Fall through only after all pending output has reached nextOutput.
    case kBlockDone:
        {
            CommitInflateCursors(block, stream, bitBuffer, bufferedBits, inputRemaining, inputCursor, windowCursor);
            return Zlib_InflateFlushWindow(block, input, (kZStreamEnd));
        }
    case kBlockBad:
        {
            CommitInflateCursors(block, stream, bitBuffer, bufferedBits, inputRemaining, inputCursor, windowCursor);
            return Zlib_InflateFlushWindow(block, input, (kZDataError));
        }
    default:
        {
            CommitInflateCursors(block, stream, bitBuffer, bufferedBits, inputRemaining, inputCursor, windowCursor);
            return Zlib_InflateFlushWindow(block, input, (kZStreamError));
        }
    }
}

// Build bounded multilevel Huffman tables from code lengths; reject oversubscribed or incomplete trees.
// Require symbolCount > 0 and lengths in 0..15. lengthsInput and bases/extras
// are borrowed arrays; sortedSymbolsInput is caller-provided scratch. rootBits
// is in/out (requested width, then clamped); rootTable points INTO workspace.
// workspaceUsed is in/out entries consumed, not bytes: both trees share a
// 1440-entry arena. No allocation/free here. Return 0 success, -3 invalid tree
// or arena overflow, -5 incomplete tree (single-code trees are accepted).
// Worked example: lengths A=1,B=2,C=3,D=3 produce canonical codes
// A=0, B=10, C=110, D=111. With rootBits=2, LSB-first indexing places A
// at root[0] AND root[2] (one bit consumed), B at root[1] (two bits),
// and a link at root[3] (bits=2, op=1). The link's val is child-root[3];
// the next input bit selects child[0]=C or child[1]=D (bits=1 each).
// Replication fills every root slot sharing a shorter prefix. tableStack and
// offsetsOrPrefixes permit returning to a parent when that prefix changes.
int Zlib_BuildHuffmanDecodeTable(void* lengthsInput, unsigned symbolCount, unsigned simpleSymbolCount, const char* basesInput, const char* extrasInput,
                 void* rootTableOutput, void* rootBitsOutput, void* workspaceInput, unsigned* workspaceUsed, unsigned* sortedSymbolsInput) {
    unsigned* sortedSymbols = sortedSymbolsInput;
    unsigned lengthCounts[16];
    CodeEntry* tableStack[16];
    unsigned offsetsOrPrefixes[16];
    union { CodeEntry entry; unsigned words[2]; } entryStorage;
    unsigned* codeLengths = (unsigned*)lengthsInput;
    const unsigned* baseValues = (const unsigned*)basesInput;
    const unsigned* extraBits = (const unsigned*)extrasInput;
    CodeEntry** rootTable = (CodeEntry**)rootTableOutput;
    unsigned* rootBits = (unsigned*)rootBitsOutput;
    CodeEntry* workspace = (CodeEntry*)workspaceInput;
    register unsigned remainingAtLength;
    register unsigned codeSpace;
    register int maximumLength;
    register int tableLevel;
    register unsigned cursor;
    register unsigned code;
    register unsigned* symbolCursor;
    register int codeLength;
    int bitsPerTable;
    register CodeEntry* currentTable;
    register int bitsDecoded;
    register unsigned* countCursor;
    register int unusedCodes;
    unsigned tableSize;

    // Count code lengths, then bound the root table to the shortest and longest used lengths.
    for (code = 0; code < kMaxCodeBits + 1; code++) lengthCounts[code] = 0;
    unsigned count = symbolCount;
    symbolCursor = codeLengths;
    do { lengthCounts[*symbolCursor++]++; } while (--count);
    if (lengthCounts[0] == symbolCount) { *rootTable = 0; *rootBits = 0; return 0; }
    bitsPerTable = *rootBits;
    for (cursor = 1; cursor <= 15; cursor++) if (lengthCounts[cursor]) break;
    codeLength = cursor;
    if (bitsPerTable < cursor) bitsPerTable = cursor;
    for (code = 15; code; code--) if (lengthCounts[code]) break;
    maximumLength = code;
    if (bitsPerTable > code) bitsPerTable = code;
    *rootBits = bitsPerTable;
    for (unusedCodes = 1 << cursor; cursor < code; cursor++, unusedCodes <<= 1)
        if ((unusedCodes -= lengthCounts[cursor]) < 0) return -3;
    if ((unusedCodes -= lengthCounts[code]) < 0) return -3;
    lengthCounts[code] += unusedCodes;
    // Sort symbol numbers by code length; offsets later store prefixes for parent tables.
    offsetsOrPrefixes[1] = cursor = 0;
    symbolCursor = lengthCounts + 1; countCursor = offsetsOrPrefixes + 2;
    while (--code) { cursor += *symbolCursor++; *countCursor++ = cursor; }
    symbolCursor = codeLengths; code = 0;
    do { if ((cursor = *symbolCursor++) != 0) sortedSymbols[offsetsOrPrefixes[cursor]++] = code; } while (++code < symbolCount);
    symbolCount = offsetsOrPrefixes[maximumLength];
    offsetsOrPrefixes[0] = code = 0;
    tableLevel = -1;
    tableStack[0] = currentTable = 0;
    bitsDecoded = -bitsPerTable;
    tableSize = 0;
    symbolCursor = sortedSymbols;
    // Emit replicated decode entries and descend or unwind subtables as prefixes change.
    for (; codeLength <= maximumLength; codeLength++) {
        remainingAtLength = lengthCounts[codeLength];
        while (remainingAtLength--) {
            while (codeLength > bitsDecoded + bitsPerTable) {
                tableLevel++;
                bitsDecoded += bitsPerTable;
                tableSize = maximumLength - bitsDecoded;
                tableSize = tableSize > bitsPerTable ? bitsPerTable : tableSize;
                cursor = codeLength - bitsDecoded;
                if ((codeSpace = 1 << cursor) > remainingAtLength + 1) {
                    codeSpace -= remainingAtLength + 1;
                    countCursor = lengthCounts + codeLength;
                    if (cursor < tableSize)
                        while (++cursor < tableSize) {
                            if ((codeSpace <<= 1) <= *++countCursor) break;
                            codeSpace -= *countCursor;
                        }
                }
                tableSize = 1 << cursor;
                if (*workspaceUsed + tableSize > kHuffmanWorkspaceEntries) return -3;
                tableStack[tableLevel] = currentTable = workspace + *workspaceUsed;
                *workspaceUsed += tableSize;
                if (tableLevel) {
                    offsetsOrPrefixes[tableLevel] = code;
                    entryStorage.entry.bits = (unsigned char)bitsPerTable;
                    entryStorage.entry.op = (unsigned char)cursor;
                    cursor = code >> (bitsDecoded - bitsPerTable);
                    entryStorage.entry.val = currentTable - tableStack[tableLevel-1] - cursor;
                    unsigned* link = (unsigned*)(tableStack[tableLevel-1] + cursor);
                    link[0] = entryStorage.words[0];
                    link[1] = entryStorage.words[1];
                } else *rootTable = currentTable;
            }
            entryStorage.entry.bits = (unsigned char)(codeLength - bitsDecoded);
            if (symbolCursor >= sortedSymbols + symbolCount) entryStorage.entry.op = kHuffInvalid;
            else if (*symbolCursor < simpleSymbolCount) {
                // Literal=0, end-of-block=0x60; enum operands change this ternary's code generation.
                entryStorage.entry.op = *symbolCursor < 256 ? 0 : 96;
                entryStorage.entry.val = *symbolCursor++;
            } else {
                entryStorage.entry.op = (unsigned char)(extraBits[*symbolCursor - simpleSymbolCount] + kHuffBaseWithExtra);
                entryStorage.entry.val = baseValues[*symbolCursor++ - simpleSymbolCount];
            }
            cursor = code >> bitsDecoded;
            codeSpace = 1 << (codeLength - bitsDecoded);
            for (; cursor < tableSize; cursor += codeSpace) {
                ((unsigned*)&currentTable[cursor])[0] = entryStorage.words[0];
                ((unsigned*)&currentTable[cursor])[1] = entryStorage.words[1];
            }
            cursor = 1 << (codeLength - 1);
            while (code & cursor) { code ^= cursor; cursor >>= 1; }
            code ^= cursor;
            codeSpace = (1 << bitsDecoded) - 1;
            while ((code & codeSpace) != offsetsOrPrefixes[tableLevel]) {
                tableLevel--; bitsDecoded -= bitsPerTable; codeSpace = (1 << bitsDecoded) - 1;
            }
        }
    }
    return unusedCodes != 0 && maximumLength != 1 ? -5 : 0;
}

// Build literal/length and distance trees in shared workspace, freeing temporary storage on every exit.
// Address-valued int parameters preserve the 32-bit ABI; lengths are concatenated
// literalCount then distanceCount. Tree/root-bit outputs borrow the supplied arena.
// Return 0 on two valid trees; -4 allocation failure; -3 malformed/incomplete tree.
int Zlib_InflateBuildDynamicTrees(int literalCount, int distanceCount, int lengthsAddress, int literalBitsAddress, int distanceBitsAddress, int literalTreeAddress, int distanceTreeAddress, int workspaceAddress, Obj* stream) {
    int workspaceUsed = 0;
    char* tableBytes = s_inflateTableRegionBase;
    int distanceResult;
    int literalResult;
    void* temporaryWork = stream->alloc(stream->ctx, kLiteralSortScratchSymbols, 4);
    if (temporaryWork == 0) return -4;

    literalResult = Zlib_BuildHuffmanDecodeTable((void*)lengthsAddress, literalCount, kLiteralCountBias, ((char*)(tableBytes + INFLATE_OFFSET(lengthBases))), ((char*)(tableBytes + INFLATE_OFFSET(lengthExtraBits))), (void*)literalTreeAddress, (void*)literalBitsAddress, (void*)workspaceAddress, (unsigned*)&workspaceUsed, (unsigned*)temporaryWork);
    // Phase 1: a nonempty literal/length tree is required before the distance tree.
    if (!(literalResult == 0 && *(int*)literalBitsAddress != 0)) {
        if (literalResult == -3) {
            stream->msg = inflateDiagnosticStrings.oversubscribed_literal_length_tree;
        } else if (literalResult != -4) {
            literalResult = -3;
            stream->msg = inflateDiagnosticStrings.incomplete_literal_length_tree;
        }
        stream->free(stream->ctx, temporaryWork);
        return literalResult;

    }
    // Phase 2: append the distance tree in the same arena; an empty distance
    // tree is legal only when there are no match-length symbols.
    distanceResult = Zlib_BuildHuffmanDecodeTable((char*)lengthsAddress + literalCount * 4, distanceCount, 0, ((char*)(tableBytes + INFLATE_OFFSET(distanceBases))), ((char*)(tableBytes + INFLATE_OFFSET(distanceExtraBits))), (void*)distanceTreeAddress, (void*)distanceBitsAddress, (void*)workspaceAddress, (unsigned*)&workspaceUsed, (unsigned*)temporaryWork);
    if (distanceResult != 0 || (*(int*)distanceBitsAddress == 0 && (unsigned int)literalCount > kLiteralCountBias)) {
        if (distanceResult == -3) {
            stream->msg = inflateDiagnosticStrings.oversubscribed_distance_tree;
        } else if (distanceResult == -5) {
            distanceResult = -3;
            stream->msg = inflateDiagnosticStrings.incomplete_distance_tree;
        } else if (distanceResult != -4) {
            distanceResult = -3;
            stream->msg = inflateDiagnosticStrings.empty_distance_tree_with_lengths;
        }
        stream->free(stream->ctx, temporaryWork);
        return distanceResult;
    }
    stream->free(stream->ctx, temporaryWork);
    return 0;
}

} // extern "C"
