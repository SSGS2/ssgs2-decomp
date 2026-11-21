#include <decomp/ct_member.h>
#include "MSL/error_message.h"
#include "MSL/vector.h"
// Vector destroyed by dtor_800A9544 holds borrowed UTF-16 line pointers (scalar tag below);
// its size reset loads the size twice (form 2).
struct Elem_800A9544 { unsigned int words[1]; };
// Skip per-element destruction for the scalar glyph-storage tag.
namespace std { template <> struct __vec_destroys_elements<Elem_800A9544> { enum { value = 2 }; static void destroy(Elem_800A9544*) {} }; }
typedef std::vector<Elem_800A9544, app::app_allocator<Elem_800A9544> > Elem_800A9544Vector;
namespace exception_runtime {
extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12length_errorFv(void*, int);
extern const char scriptAliasExceptionStrings[];
}
}

// Translation unit 0x800A9430-0x800AE594 (44 functions; boundaries inferred by decomp-toolkit from extab).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#define GAME_WSTRING_DECLARATIONS_ONLY
#include "game/wstring.h"

#include "game/string.h"
#include "classes/CMsgDraw.h"
#include <decomp/cw_bitfields.h>

// Borrow the checked renderer layout; keep this cast macro to retain float-register lifetimes.
#define AsCMsgDraw(p) ((CMsgDraw*)(p))

// Member offsets retain the compiler's byte-addition scheduling for embedded containers.
enum MessageMemberOffset {
    kMsgRevealDelayTagCountOffset = (int)&((CMsgDraw*)0)->revealDelayTagCount,
    kMsgRevealDelayTreeRootOffset = (int)&((CMsgDraw*)0)->revealDelayTreeRoot,
    kMsgSoundTagCountOffset = (int)&((CMsgDraw*)0)->soundTagCount,
    kMsgSoundTagTreeRootOffset = (int)&((CMsgDraw*)0)->soundTagTreeRoot,
    kMsgGlyphCountOffset = (int)&((CMsgDraw*)0)->glyphCount,
    kMsgNextLineYOffset = (int)&((CMsgDraw*)0)->nextLineY,
    kMsgRevealDelayFramesOffset = (int)&((CMsgDraw*)0)->revealDelayFrames,
    kMsgLastSoundGlyphCountOffset = (int)&((CMsgDraw*)0)->lastSoundGlyphCount,
    kMsgRevealSoundStorageOffset = (int)&((CMsgDraw*)0)->revealSoundStorage,
    kMsgRevealSoundHeapDataOffset = (int)&((CMsgDraw*)0)->revealSoundHeapData,
    kMsgRevealSoundHeapLengthOffset = (int)&((CMsgDraw*)0)->revealSoundHeapLength,
    kMsgCurrentColorOffset = (int)&((CMsgDraw*)0)->currentColor,
    kMsgDefaultColorOffset = (int)&((CMsgDraw*)0)->defaultColor,
    kMsgLineIndexOffset = (int)&((CMsgDraw*)0)->lineIndex,
    kMsgDefaultRevealSoundStorageOffset = (int)&((CMsgDraw*)0)->defaultRevealSoundStorage,
    kMsgLinePointersOffset = (int)&((CMsgDraw*)0)->linePointers,
    kMsgContentStorageOffset = (int)&((CMsgDraw*)0)->contentStorage,
    kMsgLineCountOffset = (int)&((CMsgDraw*)0)->lineCount,
    kMsgDefaultRevealSoundHeapDataOffset = (int)&((CMsgDraw*)0)->defaultRevealSoundHeapData,
    kMsgContentHeapLengthOffset = (int)&((CMsgDraw*)0)->contentHeapLength,
    kMsgContentHeapDataOffset = (int)&((CMsgDraw*)0)->contentHeapData
};

// Cached atlas quad: page, screen bounds, UV bounds, and packed RGBA.
struct MessageGlyph {
    int atlasPage;
    float left, top, right, bottom;
    float u0, v0, u1, v1;
    unsigned int color;
};
typedef char MessageGlyph_size[(sizeof(MessageGlyph) == 0x28) ? 1 : -1];

// Formatting escapes: DS= reveal step, SS= sound, S= scale, W= wait, C= RGB, A= alignment.
enum MessageTag {
    kTagRevealStep = 0, kTagSound = 1, kTagScale = 2,
    kTagWaitFrames = 3, kTagColor = 4, kTagAlignment = 5
};
enum NumericToken {
    kMinusToken = 0, kDecimalToken = 1, kFirstDigitToken = 2, kEndNumericToken = 12
};
// Unsigned code-unit bounds retain the original unsigned comparison instructions.
static const unsigned int kFirstDecimalCode128 = 0x30u;
static const unsigned int kLastDecimalCode128 = 0x39u;
static const unsigned int kSpaceCode128 = 0x20u;
enum MessageTextConstants {
    kFormattingTagCount = 6,
    kFormatEscape = '\\', kFormatTagOpen = '<', kFormatTagClose = '>',
    kFormatSegmentBreak = 'B', kFormatNewline = '\n',
    kFullWidthGlyphThreshold = 0x3000,
    kCenterAlignmentTokenOffset = 8, kRightAlignmentTokenOffset = 0x16,
    kSegmentBreakTokenOffset = 0x22,
    kNoRevealSoundTokenOffset = 0x4B, kRevealSoundAliasOffset = 0x4F
};
enum MessageDrawing {
    kGlyphCapacity = 700, kGlyphStride = sizeof(MessageGlyph),
    kVertexStride = 24, kTriangleStrip = 0x98, kQuadVertexCount = 4
};
struct MessageFormattingToken { unsigned short* text; int id; };

// The low bit in parentAndColor is the red/black tag; key/value follow the links.
struct FormatTreeNode {
    FormatTreeNode* left;
    FormatTreeNode* right;
    unsigned int parentAndColor;
    int glyphIndex;
};
// Narrow strings use the top bit for heap ownership and the low seven bits for inline length.
struct SoundFormatTreeNode {
    FormatTreeNode links;
    unsigned int stringStorage, heapLength;
    char* heapData;
};
// Cleanup needs only the two child links. Payload destruction differs by tree:
// delay nodes contain integers; sound nodes own a tagged narrow-string value.
// Both trees use glyph indices as keys, and &root as the end/empty sentinel.
struct FormatCleanupNode { FormatCleanupNode* left; FormatCleanupNode* right; };
enum FormatTreeLayout {
    kFormatNodeKeyOffset = (int)&((FormatTreeNode*)0)->glyphIndex,
    kFormatNodeValueOffset = sizeof(FormatTreeNode),
    kSoundNodeHeapDataOffset = (int)&((SoundFormatTreeNode*)0)->heapData,
    kTaggedStringInlineMask = 0x7F
};


extern "C" {

extern char s_textParserDispatchTable[12]; // Shared WTextParser dispatch table; not a character pool.
extern char __vt__8CMsgDraw[];
extern const char scriptAliasExceptionStrings[];
extern MessageFormattingToken messageFormattingTokens[];
extern const char s_messageGlyphRenderStates[];
extern const int s_messageGlyphTextureStageParameters[];
extern char kIdentityMatrix4x4[];
extern void* g_soundManager;

extern const float kFloat_0_MessageDraw;
extern const float kFloat_4096_Message;
extern const float kFloat_1_Message;
extern const float kMessageRevealSoundVolume;
extern const float kMessageRevealSoundRange;
extern const float kMessageDefaultLayoutWidth;
extern const float kMessageDefaultLayoutHeight;
extern const float kFloat_0_1_Message;
extern const float kFloat_10_MsgText;
extern const float kFloat_2_MessageDraw;
extern int s_defaultRevealWaitFrames;
extern char messageTextAlignmentTagStrings;

void __dl__FPv(void* p);
void __dla__FPv(void* p);
int CMsgDraw_ResetTextState(char* self);

void* dtor_800A9544(char* self, int flag);
void* SoundTagTree_dtor(char* p, int flag);
void* RBTreeMap_dtor(void* p, int flag);
void appFreeUni__FPv(void* p);
void TextFormatTree_FreeStringSubtree(void* self, void* value);
void RBTree_FreeTrivialSubtree(void* self, void* value);
void* RBTree_RebalanceAfterErase(void* p, void* q);
void String_ReplaceRange(void* arg0, int arg1, int arg2, unsigned char* arg3,
                unsigned char* arg4, unsigned char* arg5);
void* IntegerKeyTree_FindInsertionPosition(void* arg0, int* arg1, void** arg2, unsigned char* arg3,
                  unsigned char* arg4);
void* BpEntryTree_InsertNode(void* arg0, void* arg1, unsigned char arg2,
                  unsigned char arg3, void* arg4);
void* __nwa__FUl(unsigned int size);
void String_ReserveCapacity(void);
int strcmp(const char* left, const char* right);
void* CMsgDrawSoundTagMap_InsertNode(void* tree, void* slot, unsigned char left,
                  unsigned char right, void* value);
void* CMsgDrawFormattedTextMap_FindInsertionParent(void* tree, int* key, void** parentOut,
                  unsigned char* leftOut, unsigned char* rightOut);
void* FontAtlas_GetFontData(void*);
void FontAtlas_GetPageDescriptor(void*, void*, unsigned int);
int FontAtlas_GetLineHeight(void*);
void gtl_FontData_FindGlyph(void*, unsigned int, void*);
void Engine_SetTextureStageParameterList(int, int*, int);
void Engine_SetRenderStateList(void*, int);
void Engine_SetType7Resource(int);
void SetTransform(int, void*);
int SoundManager_IsAliasPrefixOrStreamStemActive(void*, const char*);
void Sound_StopByAlias(void*, const char*);
void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);
void FontAtlas_BindPage(void*, void*);
void gtl_PixelCopy_CopyPackedColor(void*, void*);
void Engine_SetFlexibleVertexFormat(int);
void* GX_GetPrimitiveCount(int, int);
void DrawPrimitiveUP(int, void*, void*, int);
void* MWString_GetLength(void*);
void* MWString_GetCString(void*);
void MWString_AssignCString(void*, void*);
int CMsgDraw_MeasureText(char*, unsigned short*, float*);
int CMsgDraw_FindFormattingToken(unsigned short*, unsigned short*, int*);
int CMsgDraw_ClearFormattingAndRevealAll(void*);
int CMsgDraw_SetRevealStep(char*, int, float);
void* dtor_800ADB90(void*, int);
void WTextParser_Init(void**);
void* WString_FindChar(void*, unsigned short, unsigned short*, void*);
unsigned short* WTextParser_ReadFloat(void*, unsigned short*, float*);
unsigned short* WTextParser_ReadToken(void*, unsigned short*, unsigned short*, unsigned short*);
void* CMsgDraw_GetOrInsertFormattedTextEntry(void*, int*);
void String_AssignWithInlineFastPath(unsigned int*, const unsigned int*);
void MWString_PushBackChar(void*, signed char);
void CMsgDrawFormattedTextMap_Find(void**, void*, int*);
void CMsgDrawFormattedTextMap_GetEndIterator(void**, char*);
int CMsgDrawFormattedTextMap_IteratorsNotEqual(int*, int*);
void* CMsgDrawFormattedTextMap_CopyIterator(void**, void**);
void CMsgDraw_EraseFormattingMapEntry(void**, void*, void**);
void* TextFormatIntMap_GetOrInsertValue(void*, int*);
void Storage_ClearThreeWords(void*);
bool CMsgDraw_AppendGlyph(char*, unsigned int, float);
bool CMsgDraw_SetGlyphScale(char*, float*);
int* CMsgDraw_ParseNumericArgument(void*, unsigned short*, int*);
unsigned short* MsgText_FindWideSubstringEnd(void*, unsigned short*, unsigned short*, unsigned short*);
int wcslen(unsigned short*);
void WString_ReplaceRange(void*, int, unsigned int, unsigned short*, unsigned short*,
                 unsigned char*);
void* appMallocDummy__FUl(unsigned int);
unsigned int strlen(const char*);
char* strcpy(char*, const char*);

#pragma opt_common_subs off
// Destroy the lowest unrolled delay-tree level; deeper nodes use recursive cleanup.
static inline void FormatTree_FreeDelayLeaf(void* self, FormatCleanupNode* node_) {
    if (node_) {
        FormatCleanupNode* value_ = node_->left;
        if (value_) RBTree_FreeTrivialSubtree(((char*)&AsCMsgDraw(self)->revealDelayTagCount), value_);
        value_ = node_->right;
        if (value_) RBTree_FreeTrivialSubtree(((char*)&AsCMsgDraw(self)->revealDelayTagCount), value_);
        appFreeUni__FPv(node_);
    }
}

// Release a sound node's owned narrow string before the node itself is freed.
static inline void FormatTree_FreeSoundValue(FormatCleanupNode* node) {
    if ((char*)node + kFormatNodeKeyOffset != 0 && (char*)node + kFormatNodeValueOffset != 0 &&
        (*(unsigned int*)((char*)node + kFormatNodeValueOffset) >> 31))
        appFreeUni__FPv(*(void**)((char*)node + kSoundNodeHeapDataOffset));
}

// Destroy the lowest unrolled sound level, including its owned string value.
static inline void FormatTree_FreeSoundLeaf(void* self, FormatCleanupNode* node_) {
    typedef FormatCleanupNode Node;
    if (node_) {
        __typeof__(node_->left) savedLeftChild = node_->left;
        Node* value_ = savedLeftChild;
        if (value_) TextFormatTree_FreeStringSubtree(((char*)&AsCMsgDraw(self)->soundTagCount), value_);
        value_ = node_->right;
        if (value_) TextFormatTree_FreeStringSubtree(((char*)&AsCMsgDraw(self)->soundTagCount), value_);
        FormatTree_FreeSoundValue(node_);
        appFreeUni__FPv(node_);
    }
}

// Clear pending wait/sound tags and reveal the whole message immediately.
int CMsgDraw_ClearFormattingAndRevealAll(void* self) {
    typedef FormatCleanupNode Node;
    float fullyRevealed;
    float revealStep;
    revealStep = kFloat_0_MessageDraw;
    fullyRevealed = kFloat_4096_Message;
    AsCMsgDraw(self)->revealStep = revealStep;
    AsCMsgDraw(self)->revealPosition = fullyRevealed;
    Node* root = *(Node**)&AsCMsgDraw(self)->revealDelayTreeRoot;

// Phase 1: delay-tree postorder = left descendants, right descendants, node.
// Leaf/Mid/Parent each expand one level; the leaf helper recurses below that.
// Repeated child reads and caller-local node slots preserve CodeWarrior scheduling.
// Mid/Parent macros retain caller-local lifetimes; leaf/payload work is isolated
// in typed helpers above. Do not replace the child reloads with cached links.
#define CLEAN_A_LEAF(n) \
    { Node* node_ = (n); \
    FormatTree_FreeDelayLeaf(self, node_); }
#define CLEAN_A_MID(n) \
    { Node* node_ = (n); \
    if (node_) { \
        Node* child_ = node_->left; \
        CLEAN_A_LEAF(child_); __typeof__(node_->right) savedRightChild = node_->right; \
        child_ = savedRightChild; \
        CLEAN_A_LEAF(child_); \
        appFreeUni__FPv(node_); \
    } }
#define CLEAN_A_PARENT(n) \
    { Node* node_ = (n); \
    if (node_) { \
        Node* child_ = node_->left; \
        CLEAN_A_MID(child_); __typeof__(node_->right) unusedRightChild = node_->right; __typeof__(unusedRightChild) savedRightChild = node_->right; \
        child_ = savedRightChild; \
        CLEAN_A_MID(child_); \
        appFreeUni__FPv(node_); \
    } }
    if (root) {
        CLEAN_A_PARENT(root->left);
        CLEAN_A_PARENT(root->right);
        appFreeUni__FPv(root);
        *(void**)&AsCMsgDraw(self)->revealDelayTagCount = 0;
        *(void**)&AsCMsgDraw(self)->revealDelayTreeRoot = 0;
        (*(volatile void**)&AsCMsgDraw(self)->firstRevealDelayNode) = (void*)((volatile char*)self + kMsgRevealDelayTreeRootOffset);
    }
#undef CLEAN_A_MID
#undef CLEAN_A_PARENT
#undef CLEAN_A_LEAF

    // Phase 2: sound-tree postorder additionally releases each owned string before
    // freeing its node. Clear count/root and restore the embedded sentinel last.
    root = *(Node**)&AsCMsgDraw(self)->soundTagTreeRoot;
#define CLEAN_B_STRING(n) FormatTree_FreeSoundValue(n);
#define CLEAN_B_LEAF(n) \
    { Node* node_ = (n); \
    FormatTree_FreeSoundLeaf(self, node_); }
#define CLEAN_B_MID(n) \
    { Node* node_ = (n); \
    if (node_) { __typeof__(node_->left) savedLeftChild = node_->left; \
        Node* child_ = savedLeftChild; \
        CLEAN_B_LEAF(child_); \
        child_ = node_->right; \
        CLEAN_B_LEAF(child_); \
        CLEAN_B_STRING(node_); \
        appFreeUni__FPv(node_); \
    } }
#define CLEAN_B_PARENT(n) \
    { Node* node_ = (n); \
    if (node_) { __typeof__(node_->left) unusedLeftChild = node_->left; __typeof__(node_->left) savedLeftChild = node_->left; __typeof__(node_->right) unusedRightChild1 = node_->right; __typeof__(node_->right) savedRightChild = node_->right; __typeof__(node_->right) unusedRightChild2 = node_->right; \
        Node* child_ = savedLeftChild; \
        CLEAN_B_MID(child_); __typeof__(savedRightChild) restoredRightChild = node_->right; \
        child_ = restoredRightChild; \
        CLEAN_B_MID(child_); \
        CLEAN_B_STRING(node_); \
        appFreeUni__FPv(node_); \
    } }
    if (root) {
        CLEAN_B_PARENT(root->left);
        CLEAN_B_PARENT(root->right);
        CLEAN_B_STRING(root);
        appFreeUni__FPv(root);
        *(void**)&AsCMsgDraw(self)->soundTagCount = 0;
        *(void**)&AsCMsgDraw(self)->soundTagTreeRoot = 0;
        (*(volatile void**)&AsCMsgDraw(self)->firstSoundTagNode) = (void*)((volatile char*)self + kMsgSoundTagTreeRootOffset);
    }
#undef CLEAN_B_MID
#undef CLEAN_B_PARENT
#undef CLEAN_B_LEAF
#undef CLEAN_B_STRING
    return 1;
}
#pragma opt_common_subs reset

// Reset cursor, glyphs, reveal timing, and formatting trees while retaining owned buffers.
int CMsgDraw_ResetTextState(char* self) {
    // Phase 1: reset glyph/reveal output while retaining the allocated buffers.
    typedef FormatCleanupNode Node;

    float zero = kFloat_0_MessageDraw;
    float originY;
    float originX;
    originX = AsCMsgDraw(self)->originX;
    originY = AsCMsgDraw(self)->originY;
    AsCMsgDraw(self)->cursorX = originX;
    AsCMsgDraw(self)->cursorY = originY;
    AsCMsgDraw(self)->textHeight = zero;
    AsCMsgDraw(self)->textWidth = zero;
    *(void**)(self + kMsgGlyphCountOffset) = 0;
    AsCMsgDraw(self)->revealPosition = zero;
    *(void**)(self + kMsgNextLineYOffset) = 0;
    *(void**)(self + kMsgRevealDelayFramesOffset) = 0;
    *(void**)(self + kMsgLastSoundGlyphCountOffset) = 0;

    // Phase 2: retire old wait/sound tags before restoring default reveal state.
    Node* root = *(Node**)(self + kMsgRevealDelayTreeRootOffset);

// Clear integer wait tags in postorder, then restore the empty-tree sentinel.
#define CLEAN_A_LEAF(n) \
    { Node* node_ = (n); \
    FormatTree_FreeDelayLeaf(self, node_); }
#define CLEAN_A_MID(n) \
    { Node* node_ = (n); \
    if (node_) { \
        Node* child_ = node_->left; \
        CLEAN_A_LEAF(child_); __typeof__(node_->right) savedRightChild = node_->right; \
        child_ = savedRightChild; \
        CLEAN_A_LEAF(child_); \
        appFreeUni__FPv(node_); \
    } }
#define CLEAN_A_PARENT(n) \
    { Node* node_ = (n); \
    if (node_) { \
        Node* child_ = node_->left; \
        CLEAN_A_MID(child_); __typeof__(node_->right) unusedRightChild = node_->right; __typeof__(unusedRightChild) savedRightChild = node_->right; \
        child_ = savedRightChild; \
        CLEAN_A_MID(child_); \
        appFreeUni__FPv(node_); \
    } }
    if (root) {
        CLEAN_A_PARENT(root->left);
        CLEAN_A_PARENT(root->right);
        appFreeUni__FPv(root);
        *(void**)(self + kMsgRevealDelayTagCountOffset) = 0;
        *(void**)(self + kMsgRevealDelayTreeRootOffset) = 0;
        (*(volatile void**)&AsCMsgDraw(self)->firstRevealDelayNode) = (void*)(((char*)&AsCMsgDraw(self)->revealDelayTreeRoot));
    }
#undef CLEAN_A_MID
#undef CLEAN_A_PARENT
#undef CLEAN_A_LEAF

    // Clear owned sound-name values in postorder, then restore that tree's sentinel.
    root = (Node*)*(void**)(self + kMsgSoundTagTreeRootOffset);
#define CLEAN_B_STRING(n) FormatTree_FreeSoundValue(n);
#define CLEAN_B_LEAF(n) \
    { Node* node_ = (n); \
    FormatTree_FreeSoundLeaf(self, node_); }
#define CLEAN_B_MID(n) \
    { Node* node_ = (n); \
    if (node_) { __typeof__(node_->left) savedLeftChild = node_->left; \
        Node* child_ = savedLeftChild; \
        CLEAN_B_LEAF(child_); \
        child_ = node_->right; \
        CLEAN_B_LEAF(child_); \
        CLEAN_B_STRING(node_); \
        appFreeUni__FPv(node_); \
    } }
#define CLEAN_B_PARENT(n) \
    { Node* node_ = (n); \
    if (node_) { __typeof__(node_->left) unusedLeftChild = node_->left; __typeof__(node_->left) savedLeftChild = node_->left; __typeof__(node_->right) unusedRightChild1 = node_->right; __typeof__(node_->right) savedRightChild = node_->right; __typeof__(node_->right) unusedRightChild2 = node_->right; \
        Node* child_ = savedLeftChild; \
        CLEAN_B_MID(child_); __typeof__(savedRightChild) restoredRightChild = node_->right; \
        child_ = restoredRightChild; \
        CLEAN_B_MID(child_); \
        CLEAN_B_STRING(node_); \
        appFreeUni__FPv(node_); \
    } }
    if (root) {
        CLEAN_B_PARENT(root->left);
        CLEAN_B_PARENT(root->right);
        CLEAN_B_STRING(root);
        appFreeUni__FPv(root);
        *(void**)(self + kMsgSoundTagCountOffset) = 0;
        *(void**)(self + kMsgSoundTagTreeRootOffset) = 0;
        (*(volatile void**)&AsCMsgDraw(self)->firstSoundTagNode) = (void*)(((char*)&AsCMsgDraw(self)->soundTagTreeRoot));
    }
#undef CLEAN_B_MID
#undef CLEAN_B_PARENT
#undef CLEAN_B_LEAF
#undef CLEAN_B_STRING

    // Reset the current reveal sound and layout defaults without releasing the
    // retained string/glyph buffers. Heap string length becomes zero in place.
    unsigned int flags = *(unsigned int*)(self + kMsgRevealSoundStorageOffset);
    if (!(flags >> 31)) {

        *(unsigned char*)(self + 0x9d) = 0;
        *(unsigned char*)(self + kMsgRevealSoundStorageOffset) = (unsigned char)((*(unsigned char*)(self + kMsgRevealSoundStorageOffset)) & -128);
    } else {
        **(unsigned char**)(self + kMsgRevealSoundHeapDataOffset) = 0;
        *(void**)(self + kMsgRevealSoundHeapLengthOffset) = 0;
    }
    int emptyLineCount = AsCMsgDraw(self)->lineCount;
    emptyLineCount = emptyLineCount - emptyLineCount;
    AsCMsgDraw(self)->glyphScaleY = kFloat_1_Message;
    *(void**)(self + kMsgCurrentColorOffset) = *(void**)(self + kMsgDefaultColorOffset);
    AsCMsgDraw(self)->glyphScaleX = kFloat_1_Message;
    AsCMsgDraw(self)->lineCount = emptyLineCount;
    *(void**)(self + kMsgLineIndexOffset) = (void*)-1;
    return 1;
}

// Build glyph quads and per-glyph reveal/sound tags from UTF-16 text and formatting escapes.
// Input is borrowed, null-terminated UTF-16. Grammar: backslash + '<' introduces
// a tag ending at '>'; DS/SS/S/W/C/A dispatch to reveal/sound/scale/wait/RGB/align.
// A bare backslash is consumed. Unknown tags are skipped, except the segment-break
// marker which returns success early. Glyph-capacity exhaustion also returns 1;
// null input returns 0. Missing delimiters are delegated to WTextParser, with no
// local validation/recovery; callers must provide well-formed formatting input.
// Maps are keyed by current glyphCount: integer wait frames and owned narrow sound
// names. Get-or-insert returns a value address, not a node. Byte-pointer member
// casts retain the embedded-container ABI; pointers stored in value are parser
// cursors/output slots, not retained source-text ownership.
int CMsgDraw_ParseFormattedText(char* self, unsigned short* text) {
    // Phase 1: validate the segment and install its optional default sound.
    unsigned short soundNameToken[0x100];
    int soundName[3];
    float lineExtent[2];
    if (0 == text)
        return 0;

    if (MWString_GetLength(((char*)&AsCMsgDraw(self)->defaultRevealSoundStorage)) != 0) {
        void* value = MWString_GetCString(self + kMsgDefaultRevealSoundStorageOffset);
        MWString_AssignCString(CMsgDraw_GetOrInsertFormattedTextEntry(((char*)&AsCMsgDraw(self)->soundTagCount), (int*)(((char*)&AsCMsgDraw(self)->glyphCount))), value);
    }

    float glyphScale[2];
    void* parser;
    void* value;
    float revealStep;
    int waitFrames;
    void* existingWaitTag;
    void* endWaitTag;
    void* eraseResult;
    void* eraseIterator;

    int rgba[3];

    // Phase 2: measure the line and tokenize formatting/glyph input.
    WTextParser_Init(&parser);
     CMsgDraw_MeasureText(self, text, lineExtent);
    AsCMsgDraw(self)->nextLineY = (int)(AsCMsgDraw(self)->cursorY + lineExtent[1]);
    AsCMsgDraw(self)->parsedLineCount = 1; // original +0x28

    for (; 0 != text && *text != 0;) {
        unsigned short ch = *text;
        if (ch == kFormatNewline) {
            text += 1;

            AsCMsgDraw(self)->cursorX = (AsCMsgDraw(self)->originX); AsCMsgDraw(self)->cursorY = (float)AsCMsgDraw(self)->nextLineY;
            AsCMsgDraw(self)->nextLineY = (int)(AsCMsgDraw(self)->cursorY + lineExtent[1]);
            AsCMsgDraw(self)->parsedLineCount = AsCMsgDraw(self)->parsedLineCount + 1; // original +0x28
            CMsgDraw_MeasureText(self, text, lineExtent);
            continue;
        }
        if (ch == kFormatEscape) {
            if (text[1] == kFormatTagOpen) {
                unsigned short* tag = (text + 2);
                text = (unsigned short*)WString_FindChar(&parser, kFormatTagClose, tag, 0);
                value = 0;
                switch (CMsgDraw_FindFormattingToken(tag, text, (int*)&value)) {
                case kTagRevealStep:
                    revealStep = 1.0f;
                    value = WTextParser_ReadFloat(&parser, (unsigned short*)value, &revealStep);
                    if (revealStep < 1e-5f) CMsgDraw_ClearFormattingAndRevealAll(self);
                    else CMsgDraw_SetRevealStep(self, 0, revealStep);
                    break;
                case kTagSound:
                    // Convert UTF-16 token code units to narrow bytes, then copy
                    // the temporary string into the glyph-indexed owned value.
                    soundNameToken[0] = 0;
                    value = WTextParser_ReadToken(&parser, (unsigned short*)value, text, soundNameToken);
                    Storage_ClearThreeWords(soundName);
                    try {
                    for (unsigned short* p = soundNameToken; *p != 0; ++p)
                        MWString_PushBackChar(soundName, (signed char)*p);
                    String_AssignWithInlineFastPath((unsigned int*)CMsgDraw_GetOrInsertFormattedTextEntry(self + kMsgSoundTagCountOffset, (int*)(self + kMsgGlyphCountOffset)),
                                (unsigned int*)soundName);
                    } catch (...) {
                        ((NString*)(soundName))->~NString();
                        throw;
                    }
                    ((NString*)(soundName))->~NString();
                    break;
                case kTagScale:
                    value = WTextParser_ReadFloat(&parser, (unsigned short*)value, &glyphScale[0]);
                    value = WTextParser_ReadFloat(&parser, (unsigned short*)value, &glyphScale[1]);
                    CMsgDraw_SetGlyphScale(self, glyphScale);
                    break;
                case kTagWaitFrames:
                    // Replace a previous wait at this glyph rather than accumulate.
                    value = CMsgDraw_ParseNumericArgument(&parser, (unsigned short*)value, &waitFrames);
                    CMsgDrawFormattedTextMap_Find((void**)&existingWaitTag, self + kMsgRevealDelayTagCountOffset, (int*)(self + kMsgGlyphCountOffset));
                    CMsgDrawFormattedTextMap_GetEndIterator(&endWaitTag, self + kMsgRevealDelayTagCountOffset);
                    if (CMsgDrawFormattedTextMap_IteratorsNotEqual((int*)&existingWaitTag, (int*)&endWaitTag)) {
                        CMsgDraw_EraseFormattingMapEntry(&eraseResult, self + kMsgRevealDelayTagCountOffset, (void**)CMsgDrawFormattedTextMap_CopyIterator(&eraseIterator, &existingWaitTag));
                    }
                    int parsedWaitFrames = waitFrames;
                    *(int*)TextFormatIntMap_GetOrInsertValue(self + kMsgRevealDelayTagCountOffset, (int*)(self + kMsgGlyphCountOffset)) = parsedWaitFrames;
                    break;
                case kTagColor:
                    value = (void*)CMsgDraw_ParseNumericArgument(&parser, (unsigned short*)value, &rgba[0]);
                    value = (void*)CMsgDraw_ParseNumericArgument(&parser, (unsigned short*)value, &rgba[1]);
                    value = (void*)CMsgDraw_ParseNumericArgument(&parser, (unsigned short*)value, &rgba[2]);
                    *(unsigned int*)&AsCMsgDraw(self)->currentColor = ((rgba[0] & 0xff) << 24) |
                        ((rgba[1] & 0xff) << 16) | ((rgba[2] & 0xff) << 8) | 0xff;
                    break;
                case kTagAlignment:
                    if (MsgText_FindWideSubstringEnd(&parser, (unsigned short*)(&messageTextAlignmentTagStrings + kCenterAlignmentTokenOffset),
                                    (unsigned short*)value, text))
                        AsCMsgDraw(self)->cursorX = AsCMsgDraw(self)->originX +
                            AsCMsgDraw(self)->layoutWidth * 0.5f - 0.5f * lineExtent[0];
                    else if (MsgText_FindWideSubstringEnd(&parser, (unsigned short*)(&messageTextAlignmentTagStrings + kRightAlignmentTokenOffset),
                                         (unsigned short*)value, text))
                        AsCMsgDraw(self)->cursorX = AsCMsgDraw(self)->originX +
                            AsCMsgDraw(self)->layoutWidth - lineExtent[0];
                    break;
                default:
                    value = MsgText_FindWideSubstringEnd(&parser, (unsigned short*)(&messageTextAlignmentTagStrings + kSegmentBreakTokenOffset),
                                        tag, text);
                    if (0 != value) {
                        dtor_800ADB90(&parser, -1);
                        return 1;
                    }
                    break;
                }
            }
            text = (1 + text);
            continue;
        }

        unsigned short* scan = text;
        unsigned short* found = 0;
        int index = 0;
        while (*scan != 0) {
            unsigned short c = *scan;
            if (c == kFormatNewline || c == kFormatEscape) {
                found = text + index;
                if (*found != kFormatEscape) break;
                if (found[1] != kFormatTagOpen) break;
                if (found[2] != kFormatSegmentBreak) break;
                dtor_800ADB90(&parser, -1);
                return 1;
            }
            CMsgDraw_AppendGlyph(self, c, lineExtent[1]);
            if (AsCMsgDraw(self)->glyphCount >= kGlyphCapacity) {
                dtor_800ADB90(&parser, -1);
                return 1;
            }
            scan = (scan + 1);
            index = index + 1;
        }
        text = found;
    }
    dtor_800ADB90(&parser, -1);
    return 1;
}

// Draw the revealed glyph prefix and play its configured reveal sound when that prefix advances.
int Text_DrawGlyphs(char* self) {
    // Phase 1: honor draw suppression and configure the glyph render state.
    unsigned int renderStatePairCount = 13;
    if (AsCMsgDraw(self)->drawEnabled == 0) {
        return 0;
    }
    if (AsCMsgDraw(self)->drawSuppressed != 0) {
        return 0;
    }

    *(volatile int*)&AsCMsgDraw(self)->drawSuppressed = 1;
    int renderStates[26];
    int* projectionDst = renderStates - 1;
    int* projectionSrc = (int*)s_messageGlyphRenderStates - 1;
    while (renderStatePairCount-- > 0) {
        int second = projectionSrc[2];
        int first = projectionSrc[1];

        projectionSrc += 2;
        projectionDst[1] = first;
        projectionDst[2] = second;
        projectionDst += 2;
    }

    int* table = (int*)s_messageGlyphTextureStageParameters;
    int setup[10]; setup[0] = table[0];
    setup[1] = table[1];
    setup[2] = table[2];
    setup[3] = table[3];
    setup[4] = table[4];
    setup[5] = table[5];
    setup[6] = table[6];
    setup[7] = table[7];
    setup[8] = table[8];
    setup[9] = table[9];
    Engine_SetTextureStageParameterList(0, setup, 5);
    Engine_SetRenderStateList(renderStates, 13);
    Engine_SetType7Resource(0);
    SetTransform(2, kIdentityMatrix4x4);

    // Phase 2: limit rendering to the revealed prefix and draw its cached glyphs.
    int count = (int)AsCMsgDraw(self)->revealPosition;
    int* countPtr;
    if (AsCMsgDraw(self)->glyphCount < count) {
        countPtr = (int*)((char*)self + kMsgGlyphCountOffset);
    } else {
        countPtr = &count;
    }
    int maxCount = *countPtr;
    unsigned int selected;
    if (!((*(unsigned int*)&AsCMsgDraw(self)->revealSoundStorage >> 31) == 0)) {
        selected = *(unsigned int*)&AsCMsgDraw(self)->revealSoundHeapLength;
    } else {
        selected = *(unsigned char*)&AsCMsgDraw(self)->revealSoundStorage & kTaggedStringInlineMask;
    }
    if (selected > 1u) { if (AsCMsgDraw(self)->lastSoundGlyphCount != maxCount) {
        const char* name = scriptAliasExceptionStrings + kRevealSoundAliasOffset;
        void* manager = g_soundManager;
        if (SoundManager_IsAliasPrefixOrStreamStemActive(manager, name) != 0) {
            Sound_StopByAlias(g_soundManager, name);
        }
        if ((*(unsigned int*)&AsCMsgDraw(self)->revealSoundStorage >> 31) == 0) {
            selected = (unsigned int)(self + 0x9d);
        } else {
            selected = *(unsigned int*)&AsCMsgDraw(self)->revealSoundHeapData;
        }
        Sound_PlayByName(g_soundManager, (const char*)selected,
                    0, 0, (int)name, 0, kMessageRevealSoundVolume,
                    kMessageRevealSoundRange, kFloat_1_Message);
        AsCMsgDraw(self)->lastSoundGlyphCount = maxCount;
    } }

    int offset;
    int index = 0;
    struct Vertex {
        float x, y, z;
        unsigned char color[4];
        float u; float v;
    };
    if (maxCount > 0) {
      offset = 0;
      while (maxCount > index) {
        FontAtlas_BindPage(AsCMsgDraw(self)->fontAtlas,
                    *(void**)(((char*)AsCMsgDraw(self)->glyphs) + offset));
        char* item = ((char*)AsCMsgDraw(self)->glyphs) + offset;
        int pad[2] = {};
        Vertex verts[4] = {};
            verts[0].x = ((MessageGlyph*)item)->left;
        verts[0].y = ((MessageGlyph*)item)->top;
        verts[0].u = ((MessageGlyph*)item)->u0;
        verts[0].v = ((MessageGlyph*)item)->v0;
        verts[1].x = ((MessageGlyph*)item)->left;
        verts[1].y = ((MessageGlyph*)item)->bottom;
        verts[1].u = ((MessageGlyph*)item)->u0;
        verts[1].v = ((MessageGlyph*)item)->v1;
        verts[2].x = ((MessageGlyph*)item)->right;
        verts[2].y = ((MessageGlyph*)item)->top;
        verts[2].u = ((MessageGlyph*)item)->u1;
        verts[2].v = ((MessageGlyph*)item)->v0;
        verts[3].x = ((MessageGlyph*)item)->right;
        verts[3].y = ((MessageGlyph*)item)->bottom;
        verts[3].u = ((MessageGlyph*)item)->u1;
        verts[3].v = ((MessageGlyph*)item)->v1;
        gtl_PixelCopy_CopyPackedColor(verts[3].color, &((MessageGlyph*)item)->color);
        verts[2].color[0] = verts[3].color[0];
        verts[2].color[1] = verts[3].color[1];
        verts[2].color[2] = verts[3].color[2];
        verts[2].color[3] = verts[3].color[3];
        verts[1].color[0] = verts[3].color[0];
        verts[1].color[1] = verts[3].color[1];
        verts[1].color[2] = verts[3].color[2];
        verts[1].color[3] = verts[3].color[3];
        verts[0].color[0] = verts[3].color[0];
        verts[0].color[1] = verts[3].color[1];
        verts[0].color[2] = verts[3].color[2];
        verts[0].color[3] = verts[3].color[3];
        Engine_SetFlexibleVertexFormat(0x120000 - 0xffb);
        Engine_SetType7Resource(0);
        void* buffer = GX_GetPrimitiveCount(kQuadVertexCount, kTriangleStrip);
        DrawPrimitiveUP(kTriangleStrip, buffer, verts, kVertexStride);
        offset += kGlyphStride;
        index += 1;
      }
    }
    return 1;
}

// Release glyph storage, strings, line pointers, and formatting trees; optionally delete the object.
void* CMsgDraw_dtor(char* self, int flag) {
    if (self != 0) {
        *(void**)self = &__vt__8CMsgDraw;
        CMsgDraw_ResetTextState(self);
        __dla__FPv(AsCMsgDraw(self)->glyphs);
        ((NString*)((char*)&AsCMsgDraw(self)->defaultRevealSoundStorage))->~NString();
        ((NString*)((char*)&AsCMsgDraw(self)->revealSoundStorage))->~NString();
        SoundTagTree_dtor((char*)&AsCMsgDraw(self)->soundTagCount, -1);
        RBTreeMap_dtor((char*)&AsCMsgDraw(self)->revealDelayTagCount, -1);
        dtor_800A9544((char*)&AsCMsgDraw(self)->linePointers, -1);
        reinterpret_cast<WString*>((char*)&AsCMsgDraw(self)->contentStorage)->~WString();
        if (flag > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

#pragma use_lmw_stmw on
// Destroy the integer wait-tag tree using the original unrolled postorder traversal.
void* RBTreeMap_dtor(void* arg0, int flag) {
    // Phase 1: release the unrolled integer-format tree below its header.
    char* self;
    self = (char*)arg0;
    if (self == 0) return self;
    {
        char* root = *(char**)(self + 4);
        if (root != 0) {
            char* node = *(char**)root;
            if (node != 0) {
                char* child = *(char**)node;
                if (child != 0) {
                    char* leaf = *(char**)child;
                    if (leaf != 0) {
                        if (*(void**)leaf != 0) RBTree_FreeTrivialSubtree(self, *(void**)leaf);
                        if (*(void**)(leaf + 4) != 0) RBTree_FreeTrivialSubtree(self, *(void**)(leaf + 4));
                        appFreeUni__FPv(leaf);
                    }
                    leaf = *(char**)(child + 4);
                    if (leaf != 0) {
                        if (*(void**)leaf != 0) RBTree_FreeTrivialSubtree(self, *(void**)leaf);
                        if (*(void**)(leaf + 4) != 0) RBTree_FreeTrivialSubtree(self, *(void**)(leaf + 4));
                        appFreeUni__FPv(leaf);
                    }
                    appFreeUni__FPv(child);
                }
                child = *(char**)(node + 4);
                if (child != 0) {
                    char* leaf = *(char**)child;
                    if (leaf != 0) {
                        if (*(void**)leaf != 0) RBTree_FreeTrivialSubtree(self, *(void**)leaf);
                        if (*(void**)(leaf + 4) != 0) RBTree_FreeTrivialSubtree(self, *(void**)(leaf + 4));
                        appFreeUni__FPv(leaf);
                    }
                    leaf = *(char**)(child + 4);
                    if (leaf != 0) {
                        if (*(void**)leaf != 0) RBTree_FreeTrivialSubtree(self, *(void**)leaf);
                        if (*(void**)(leaf + 4) != 0) RBTree_FreeTrivialSubtree(self, *(void**)(leaf + 4));
                        appFreeUni__FPv(leaf);
                    }
                    appFreeUni__FPv(child);
                }
                appFreeUni__FPv(node);
            }
            node = *(char**)(root + 4);
            if (node != 0) {
                char* child = *(char**)node;
                if (child != 0) {
                    char* leaf = *(char**)child;
                    if (leaf != 0) {
                        if (*(void**)leaf != 0) RBTree_FreeTrivialSubtree(self, *(void**)leaf);
                        if (*(void**)(leaf + 4) != 0) RBTree_FreeTrivialSubtree(self, *(void**)(leaf + 4));
                        appFreeUni__FPv(leaf);
                    }
                    leaf = *(char**)(child + 4);
                    if (leaf != 0) {
                        if (*(void**)leaf != 0) RBTree_FreeTrivialSubtree(self, *(void**)leaf);
                        if (*(void**)(leaf + 4) != 0) RBTree_FreeTrivialSubtree(self, *(void**)(leaf + 4));
                        appFreeUni__FPv(leaf);
                    }
                    appFreeUni__FPv(child);
                }
                child = *(char**)(node + 4);
                if (child != 0) {
                    char* leaf = *(char**)child;
                    if (leaf != 0) {
                        if (*(void**)leaf != 0) RBTree_FreeTrivialSubtree(self, *(void**)leaf);
                        if (*(void**)(leaf + 4) != 0) RBTree_FreeTrivialSubtree(self, *(void**)(leaf + 4));
                        appFreeUni__FPv(leaf);
                    }
                    leaf = *(char**)(child + 4);
                    if (leaf != 0) {
                        if (*(void**)leaf != 0) RBTree_FreeTrivialSubtree(self, *(void**)leaf);
                        if (*(void**)(leaf + 4) != 0) RBTree_FreeTrivialSubtree(self, *(void**)(leaf + 4));
                        appFreeUni__FPv(leaf);
                    }
                    appFreeUni__FPv(child);
                }
                appFreeUni__FPv(node);
            }
            appFreeUni__FPv(root);
        }
        if (flag > 0) __dl__FPv(self);
    }
    return self;
}
#pragma use_lmw_stmw reset

// Destroy the sound-tag tree, including each node's owned narrow string.
void* SoundTagTree_dtor(char* self, int flag) {
    // Phase 1: release the owned sound-format tree below its header.
    struct Cleanup {
        // Release one sound node's owned heap string, then the node allocation.
        static void node(char* p) {
            if (p + 0xc != 0 && p + 0x10 != 0 && (*(unsigned int*)(p + 0x10) >> 31) != 0)
                appFreeUni__FPv(*(void**)(p + 0x18));
            appFreeUni__FPv(p);
        }
        // Recursively clear both descendants below the unrolled cleanup depth.
        static void leaf(char* owner, char* p) {
            if (p == 0) return;
            if (*(void**)p != 0) TextFormatTree_FreeStringSubtree(owner, *(void**)p);
            if (*(void**)(p + 4) != 0) TextFormatTree_FreeStringSubtree(owner, *(void**)(p + 4));
            node(p);
        }
        // Clear one lower level left-to-right, then release its parent payload.
        static void lower(char* owner, char* p) {
            if (p) {

                leaf(owner, *(char**)p);
                leaf(owner, *(char**)((4 + p)));
                node(p);
            }
        }
        // Clear a lower level with caller-local child slots for original scheduling.
        static void lower2(char* owner, char* p) {
            char* child;
            if (0 != p) {
                child = *(char**)p;

                leaf(owner, child);
                child = *(char**)((p + 4));
                leaf(owner, child);
                node(p);
            }
        }
        // Clear an upper level using the original asymmetric child expansions.
        static void upper(char* owner, char* p) {
            if (p != 0) {
                // original 0x800A98D4 / 0x800A99C8: frees the subtree at +0x0 before the one at +0x4
                lower2(owner, *(char**)p);
                lower(owner, *(char**)(p + 4));
                node(p);
            }
        }
        // Clear an upper level using the same child-slot expansion on both sides.
        static void upper2(char* owner, char* p) {
            if (p != 0) {
                lower2(owner, *(char**)p);
                lower2(owner, *(char**)(p + 4));
                node(p);
            }
        }
        // Enter bounded postorder cleanup; null roots require no work.
        static void top(char* owner, char* p) {
            if (p != 0) {
                upper(owner, *(char**)p);
                upper2(owner, *(char**)(4 + p));
                node(p);
            }
        }
    };
    if (self != 0) {
    if (self != 0) {
    if (self != 0) {
    if (self) {
    if (self != 0) {
    if (self != 0) {
    Cleanup::top(self, *(char**)(self + 4));
    }
    }
    }
    }
    }
        if (0 < flag) __dl__FPv(self);
    }
    return self;
}

// Consume one delay leaf and its deeper subtrees; other segment nodes remain live.
static inline void FreeNextLineDelayLeaf128(char* self, char* leaf) {
    if (leaf) {
        if (*(void**)leaf) RBTree_FreeTrivialSubtree(((char*)&AsCMsgDraw(self)->revealDelayTagCount), *(void**)leaf);
        if (*(void**)(leaf + 4)) RBTree_FreeTrivialSubtree(((char*)&AsCMsgDraw(self)->revealDelayTagCount), *(void**)(leaf + 4));
        appFreeUni__FPv(leaf);
    }
}

// Consume one sound leaf and its deeper subtrees; other segment nodes remain live.
static inline void FreeNextLineSoundLeaf128(char* self, char* leaf) {
    if (leaf) {
        if (*(void**)leaf) TextFormatTree_FreeStringSubtree(((char*)&AsCMsgDraw(self)->soundTagCount), *(void**)leaf);
        if (*(void**)(leaf + 4)) TextFormatTree_FreeStringSubtree(((char*)&AsCMsgDraw(self)->soundTagCount), *(void**)(leaf + 4));
        if ((leaf + 0xc) != 0 && (leaf + 0x10) != 0 && ((*(unsigned int*)(leaf + 0x10) >> 31) != 0))
        appFreeUni__FPv(*(void**)(leaf + 0x18));
        appFreeUni__FPv(leaf);
    }
}

// Advance the message segment after retiring its delay and sound metadata.
int CMsgDraw_NextLine(char* self) {
    // Phase 1: validate the destination before changing any segment state.
    int index = AsCMsgDraw(self)->lineIndex + 1;
    if (index < 0 || (unsigned int)index >= *(unsigned int*)(self + kMsgLineCountOffset)) return 0;

    char* root = *(char**)(self + kMsgRevealDelayTreeRootOffset);
    AsCMsgDraw(self)->lineIndex = index;
    AsCMsgDraw(self)->textHeight = kFloat_0_MessageDraw;
    AsCMsgDraw(self)->cursorX = AsCMsgDraw(self)->originX;
    AsCMsgDraw(self)->cursorY = AsCMsgDraw(self)->originY;
    AsCMsgDraw(self)->textWidth = kFloat_0_MessageDraw;
    AsCMsgDraw(self)->glyphCount = 0;
    AsCMsgDraw(self)->revealPosition = kFloat_0_MessageDraw;
    AsCMsgDraw(self)->nextLineY = 0;
    AsCMsgDraw(self)->revealDelayFrames = 0;
    AsCMsgDraw(self)->lastSoundGlyphCount = 0;
    // Phase 2: retire old formatting metadata; nodes own their heap-backed sound names.
    if (root) {
        char* n1 = *(char**)root;
        char* n2;
        char* leaf;
        if (n1) {
            n2 = *(char**)n1;
            if (n2) {
                leaf = *(char**)n2;
                FreeNextLineDelayLeaf128(self, leaf);
                leaf = *(char**)((n2 + 4));
                FreeNextLineDelayLeaf128(self, leaf);
                appFreeUni__FPv(n2);
            }
            n2 = *(char**)(n1 + 4);
            if (n2) {
                leaf = *(char**)n2;
                FreeNextLineDelayLeaf128(self, leaf);
                leaf = *(char**)((4 + n2));
                FreeNextLineDelayLeaf128(self, leaf);
                appFreeUni__FPv(n2);
            }
            appFreeUni__FPv(n1);
        }
        n1 = *(char**)(4 + root);
        if (n1) {
            n2 = *(char**)n1;
            if (n2) {
                leaf = *(char**)n2;
                FreeNextLineDelayLeaf128(self, leaf);
                leaf = *(char**)(n2 + 4);
                FreeNextLineDelayLeaf128(self, leaf);
                appFreeUni__FPv(n2);
            }
            n2 = *(char**)((n1 + 4));
            if (n2) {
                leaf = *(char**)n2;
                FreeNextLineDelayLeaf128(self, leaf);
                leaf = *(char**)(4 + n2);
                FreeNextLineDelayLeaf128(self, leaf);
                appFreeUni__FPv(n2);
            }
            appFreeUni__FPv(n1);
        }
        appFreeUni__FPv(root);
        *(void**)(self + kMsgRevealDelayTagCountOffset) = 0;
        *(void**)(self + kMsgRevealDelayTreeRootOffset) = 0;
        AsCMsgDraw(self)->firstRevealDelayNode = ((char*)&AsCMsgDraw(self)->revealDelayTreeRoot);
    }
    char* tree = *(char**)(self + kMsgSoundTagTreeRootOffset);
    if (tree) {
        char* n2;
        char* n1 = *(char**)tree;
        char* leaf;
        if (n1) {
            n2 = *(char**)n1;
            if (n2) {
                leaf = *(char**)n2;
                FreeNextLineSoundLeaf128(self, leaf);
                leaf = *(char**)(n2 + 4);
                FreeNextLineSoundLeaf128(self, leaf);
                if ((n2 + 0xc) != 0 && (n2 + 0x10) != 0 && ((*(unsigned int*)(n2 + 0x10) >> 31) != 0))
                    appFreeUni__FPv(*(void**)(n2 + 0x18));
                appFreeUni__FPv(n2);
            }
            n2 = *(char**)(n1 + 4);
            if (n2) {
                leaf = *(char**)n2;
                FreeNextLineSoundLeaf128(self, leaf);
                leaf = *(char**)((n2 + 4));
                FreeNextLineSoundLeaf128(self, leaf);
                if ((n2 + 0xc) != 0 && (n2 + 0x10) != 0 && ((*(unsigned int*)(n2 + 0x10) >> 31) != 0))
                    appFreeUni__FPv(*(void**)(n2 + 0x18));
                appFreeUni__FPv(n2);
            }
            if ((n1 + 0xc) != 0 && (n1 + 0x10) != 0 && ((*(unsigned int*)(n1 + 0x10) >> 31) != 0))
                appFreeUni__FPv(*(void**)(n1 + 0x18));
            appFreeUni__FPv(n1);
        }
        n1 = *(char**)((tree + 4));
        if (n1) {
            n2 = *(char**)n1;
            if (n2) {
                leaf = *(char**)n2;
                FreeNextLineSoundLeaf128(self, leaf);
                leaf = *(char**)((n2 + 4));
                FreeNextLineSoundLeaf128(self, leaf);
                if ((n2 + 0xc) != 0 && (n2 + 0x10) != 0 && ((*(unsigned int*)(n2 + 0x10) >> 31) != 0))
                    appFreeUni__FPv(*(void**)(n2 + 0x18));
                appFreeUni__FPv(n2);
            }
            n2 = *(char**)((n1 + 4));
            if (n2) {
                leaf = *(char**)n2;
                FreeNextLineSoundLeaf128(self, leaf);
                leaf = *(char**)(4 + n2);
                FreeNextLineSoundLeaf128(self, leaf);
                if ((n2 + 0xc) != 0 && (n2 + 0x10) != 0 && ((*(unsigned int*)(n2 + 0x10) >> 31) != 0))
                    appFreeUni__FPv(*(void**)(n2 + 0x18));
                appFreeUni__FPv(n2);
            }
            if ((n1 + 0xc) != 0 && (n1 + 0x10) != 0 && ((*(unsigned int*)(n1 + 0x10) >> 31) != 0))
                appFreeUni__FPv(*(void**)(n1 + 0x18));
            appFreeUni__FPv(n1);
        }
        if ((tree + 0xc) != 0 && (tree + 0x10) != 0 && ((*(unsigned int*)(tree + 0x10) >> 31) != 0))
            appFreeUni__FPv(*(void**)(tree + 0x18));
        appFreeUni__FPv(tree);
        *(void**)(self + kMsgSoundTagCountOffset) = 0; *(void**)(self + kMsgSoundTagTreeRootOffset) = 0;
        AsCMsgDraw(self)->firstSoundTagNode = ((char*)&AsCMsgDraw(self)->soundTagTreeRoot);
    }
    if (!(!(!(!(!(!(((*(unsigned int*)(self + kMsgRevealSoundStorageOffset) >> 31) == 0)))))))) {

        *(unsigned char*)(self + 0x9d) = 0;
        *(unsigned char*)(self + kMsgRevealSoundStorageOffset) = (unsigned char)((*(unsigned char*)(self + kMsgRevealSoundStorageOffset)) & 0x80);
    } else {
        **(unsigned char**)(self + kMsgRevealSoundHeapDataOffset) = 0;
        AsCMsgDraw(self)->revealSoundHeapLength = 0;
    }

    // Phase 3: restore defaults and publish the selected segment.
    AsCMsgDraw(self)->revealStep = kFloat_1_Message;
    AsCMsgDraw(self)->revealPosition = (float)(int)(AsCMsgDraw(self)->revealPosition);
    AsCMsgDraw(self)->currentColor = AsCMsgDraw(self)->defaultColor;
    CMsgDraw_ParseFormattedText(self, *(unsigned short**)(*(char**)(self + kMsgLinePointersOffset) + (index * 4)));
    return 1;
}

// Replace the sound name inserted at the start of each parsed message.
void CMsgDraw_SetDefaultRevealSound(char* self, char* text) {
    unsigned int word = *(unsigned int*)(self + kMsgDefaultRevealSoundStorageOffset);
    if ((word >> 31) == 0) {
        *(unsigned char*)(self + 0x4ad) = 0;
        int value = *(unsigned char*)(self + kMsgDefaultRevealSoundStorageOffset);
        value &= -128;
        *(unsigned char*)(self + kMsgDefaultRevealSoundStorageOffset) = (unsigned char)value;
    } else {
        char* ptr = *(char**)(self + kMsgDefaultRevealSoundHeapDataOffset);
        *ptr = 0;
        AsCMsgDraw(self)->defaultRevealSoundHeapLength = 0;
    }
    if (text != 0) {
        unsigned int state = *(unsigned int*)(self + kMsgDefaultRevealSoundStorageOffset);
        int capacity;
        if ((state >> 31) == 0) {
            capacity = *(unsigned char*)(self + kMsgDefaultRevealSoundStorageOffset) & kTaggedStringInlineMask;
        } else {
            capacity = AsCMsgDraw(self)->defaultRevealSoundHeapLength;
        }
        unsigned char scratch[8];
        scratch[0] = scratch[4];
        unsigned int length = strlen(text);
        String_ReplaceRange(((char*)&AsCMsgDraw(self)->defaultRevealSoundStorage), 0, capacity,
                    (unsigned char*)text, (unsigned char*)text + length,
                    scratch);
    }
}

#pragma opt_propagation off
// Erase a tree iterator, repair its first-node sentinel, and return the successor.
void CMsgDraw_EraseFormattingMapEntry(void** result, void* tree, void** nodePtr) {
    struct N {
        N* left;
        N* right;
        unsigned parent;
    };
    struct T {
        int count;
        void* base;
        N* first;
    };
    N* n = (N*)*nodePtr;
    T* t = (T*)tree;
    N* node = n;
    if (n == t->first) {
        if (n->right != 0) {
            N* q = n->right;
            while (q->left != 0) q = q->left;
            n = q;
        } else {
            N* p;
            for (;;) {
                p = (N*)(n->parent & ~1u);
                if (n == p->left) break;
                n = p;
            }
            n = p;
        }
        t->first = n;
    }
    void* value = RBTree_RebalanceAfterErase(node, t->base);
    appFreeUni__FPv(node);
    t->count -= 1;
    *result = value;
}
#pragma opt_propagation reset

// Find a UTF-16 character before the optional end pointer or NUL terminator.
void* WString_FindChar(void* arg0, unsigned short arg1, unsigned short* list,
                  void* arg3) {
    char* cursor;
    char* scan;
    int index;
    if (list == 0) {
        return 0;
    }
    cursor = (char*)list;
    scan = (char*)list;
    index = 0;
    while (*(unsigned short*)scan != 0) {
        if (arg3 == cursor) {
            return 0;
        }
        if (arg1 == *(unsigned short*)scan) {
            return (char*)list + index * 2;
        }
        cursor += 2;
        scan += 2;
        index++;
    }
    return 0;
}

// Find a bounded UTF-16 substring and return the pointer just past its match.
unsigned short* MsgText_FindWideSubstringEnd(void* arg0, unsigned short* text,
                           unsigned short* candidate,
                           unsigned short* match) {
    if (candidate == 0) return 0;
    // 0x800ADBD0: retry from the next candidate after a mismatch.
    for (;;) {
        unsigned short first = *text;
        unsigned short* scan = candidate;
        unsigned short* scanCopy = candidate;
        int index = 0;
        while (*scanCopy != 0 || (candidate = 0, false)) {
            if (match == scan) {
                candidate = 0;
                break;
            }
            if (*scanCopy == first) {
                candidate += index;
                break;
            }
            scan++;
            scanCopy++;
            index++;
        }
        if (candidate == 0) return 0;
        unsigned short* left = candidate + 1;
        unsigned short* right = text + 1;
        int mismatch = 0;
        while (*right != 0) {
            if (*left != *right) { mismatch = 1; break; }
            left++;
            right++;
        }
        if (!mismatch) return left;
        candidate++;
    }
}

// Initialize message layout, reveal state, owned strings, and a fixed-capacity glyph buffer.
void* CMsgDraw_Ctor(char* self, void* fontAtlas) {
    float defaultHeight, defaultWidth, unitScale, zero, fullyRevealed;
    zero = kFloat_0_MessageDraw;
    unitScale = kFloat_1_Message;
    fullyRevealed = kFloat_4096_Message;
    defaultWidth = kMessageDefaultLayoutWidth;
    defaultHeight = kMessageDefaultLayoutHeight;
    *(void**)self = &__vt__8CMsgDraw;
    AsCMsgDraw(self)->fontAtlas = fontAtlas;
    AsCMsgDraw(self)->contentStorage = 0;
    AsCMsgDraw(self)->contentHeapLength = 0;
    AsCMsgDraw(self)->contentHeapData = 0;
    AsCMsgDraw(self)->linePointers = 0;
    AsCMsgDraw(self)->lineCount = 0;
    AsCMsgDraw(self)->lineCapacity = 0;
    AsCMsgDraw(self)->lineIndex = -1;
    AsCMsgDraw(self)->unk_2C = 0; // Reserved draw-state word; only its initial value is established here.
    AsCMsgDraw(self)->revealPosition = fullyRevealed;
    AsCMsgDraw(self)->revealStep = zero;
    AsCMsgDraw(self)->defaultColor = -1;
    AsCMsgDraw(self)->currentColor = -1;
    AsCMsgDraw(self)->colorMultiplier = -1;
    AsCMsgDraw(self)->revealDelayFrames = 0;
    AsCMsgDraw(self)->revealDelayTagCount = 0;
    AsCMsgDraw(self)->revealDelayTreeRoot = 0;
    AsCMsgDraw(self)->firstRevealDelayNode = (char*)self + kMsgRevealDelayTreeRootOffset;
    AsCMsgDraw(self)->soundTagCount = 0;
    AsCMsgDraw(self)->soundTagTreeRoot = 0;
    AsCMsgDraw(self)->firstSoundTagNode = (char*)self + kMsgSoundTagTreeRootOffset;
    AsCMsgDraw(self)->drawSuppressed = 0;
    AsCMsgDraw(self)->nextLineY = 0;
    AsCMsgDraw(self)->revealSoundStorage = 0;
    AsCMsgDraw(self)->revealSoundHeapLength = 0;
    AsCMsgDraw(self)->revealSoundHeapData = 0;
    AsCMsgDraw(self)->lastSoundGlyphCount = 0;
    AsCMsgDraw(self)->defaultRevealSoundStorage = 0;
    AsCMsgDraw(self)->defaultRevealSoundHeapLength = 0;
    AsCMsgDraw(self)->defaultRevealSoundHeapData = 0;
    AsCMsgDraw(self)->drawEnabled = 1;
    AsCMsgDraw(self)->originY = zero;
    AsCMsgDraw(self)->originX = zero;
    AsCMsgDraw(self)->textHeight = zero;
    AsCMsgDraw(self)->textWidth = zero;
    AsCMsgDraw(self)->glyphScaleY = unitScale;
    AsCMsgDraw(self)->glyphScaleX = unitScale;
    AsCMsgDraw(self)->cursorX = zero;
    AsCMsgDraw(self)->cursorY = zero;
    AsCMsgDraw(self)->layoutWidth = defaultWidth;
    AsCMsgDraw(self)->layoutHeight = defaultHeight;
    AsCMsgDraw(self)->glyphCount = 0;
    AsCMsgDraw(self)->glyphs = __nwa__FUl(kGlyphCapacity * kGlyphStride);
    return self;
}

// Find or insert a glyph-index sound tag and return its narrow-string value.
void* CMsgDraw_GetOrInsertFormattedTextEntry(void* tree, int* key) {
    void* parent;
    unsigned char left;
    unsigned char right;
    void* slot = CMsgDrawFormattedTextMap_FindInsertionParent(tree, key, &parent, &left, &right);
    void* result = slot;
    if (parent != 0) {
        int nodeKey = ((FormatTreeNode*)parent)->glyphIndex;
        if (*key <= nodeKey) {
            result = (char*)parent + kFormatNodeKeyOffset;
            return (char*)result + 4;
        }
    }
    int value[4];
    value[0] = *key;
    value[1] = 0;
    value[2] = 0;
    value[3] = 0;
    volatile int extra[3];
    extra[0] = 0;
    extra[1] = 0;
    extra[2] = 0;
    result = CMsgDrawSoundTagMap_InsertNode(tree, slot, left, right, value);
    if ((value[1] >> 31) != 0) {
        appFreeUni__FPv((void*)value[3]);
    }
    if ((extra[0] >> 31) != 0) {
        appFreeUni__FPv((void*)extra[2]);
    }
    return (char*)result + kFormatNodeValueOffset;
}

// Destroy the owned string in a temporary key/string pair; optionally delete the pair.
void* dtor_800AE528(char* self, int flag) {
    if (self != 0) {
        if ((char*)(self + 4) != 0) {
            if ((*(unsigned int*)(self + 4) >> 31) != 0) {
                appFreeUni__FPv(*(void**)(self + 0xC));
            }
        }
        if (flag > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

#pragma opt_common_subs off
#pragma opt_propagation off
#pragma opt_strength_reduction off
// Release the line-pointer vector's storage; its strings remain owned by the text buffer.
void* dtor_800A9544(char* self, int flag) {
    return ::std::__vector_deleting_dtor((Elem_800A9544Vector*)self, flag);
}
#pragma opt_common_subs reset
#pragma opt_propagation reset
#pragma opt_strength_reduction reset

// Overwrite the packed RGBA of every cached glyph.
void CMsgDraw_SetAllGlyphColors(char* self, int rgba) {
    int glyphOffset;
    int glyphIndex;
    glyphIndex = 0;
    glyphOffset = 0;
    while (glyphIndex < AsCMsgDraw(self)->glyphCount) {
        char* base = ((char*)AsCMsgDraw(self)->glyphs);
        glyphIndex = glyphIndex + 1;
        ((MessageGlyph*)(base + glyphOffset))->color = rgba;
        glyphOffset = glyphOffset + kGlyphStride;
    }
}

// Delete a text-parser wrapper when requested; it owns no additional resources.
void* dtor_800ADB90(void* self, int flag) {
    if (self != 0 && flag > 0) {
        __dl__FPv(self);
    }
    return self;
}

// Set glyphs revealed per update, restarting or snapping the current reveal position.
int CMsgDraw_SetRevealStep(char* self, int restartReveal, float glyphsPerUpdate) {
    AsCMsgDraw(self)->revealStep = glyphsPerUpdate;
    if (restartReveal != 0) {
        AsCMsgDraw(self)->revealPosition = kFloat_0_MessageDraw;
    } else {
        float revealPosition = AsCMsgDraw(self)->revealPosition;
        int wholeGlyphs = (int)revealPosition;
        AsCMsgDraw(self)->revealPosition = (float)wholeGlyphs;
    }
    return 1;
}

// Select the previous text segment and rebuild its glyphs and formatting state.
int CMsgDraw_PreviousLine(char* self) {
    struct ShortFlags {
        CW_BITS2(
            unsigned char isLong : 1,
            unsigned char size : 7)
    };
    struct StrNode {
        char unk0[0x10];
        CW_BITSP1(unsigned int, 31, unsigned int isLong : 1)
    };
    struct Node {
        Node* left;
        Node* right;
    };

    int index = AsCMsgDraw(self)->lineIndex - 1;
    if (index < 0 || (unsigned int)index >= *(unsigned int*)(self + kMsgLineCountOffset))
        return 0;

    float f0 = kFloat_0_MessageDraw;
    float f1;
    float f2;
    f2 = AsCMsgDraw(self)->originX;
    f1 = AsCMsgDraw(self)->originY;
    AsCMsgDraw(self)->lineIndex = index;
    AsCMsgDraw(self)->cursorX = f2;
    AsCMsgDraw(self)->cursorY = f1;
    AsCMsgDraw(self)->textHeight = f0;
    AsCMsgDraw(self)->textWidth = f0;
    *(void**)(self + kMsgGlyphCountOffset) = 0;
    AsCMsgDraw(self)->revealPosition = f0;
    *(void**)(self + kMsgNextLineYOffset) = 0;
    *(void**)(self + kMsgRevealDelayFramesOffset) = 0;
    *(void**)(self + kMsgLastSoundGlyphCountOffset) = 0;

    Node* root = *(Node**)(self + kMsgRevealDelayTreeRootOffset);

#define CLEAN_A_LEAF(n) \
    { Node* node_ = (n); \
    if (node_) { \
        Node* value_ = node_->left; \
        if (value_) RBTree_FreeTrivialSubtree(((char*)&AsCMsgDraw(self)->revealDelayTagCount), value_); \
        value_ = node_->right; \
        if (value_) RBTree_FreeTrivialSubtree(((char*)&AsCMsgDraw(self)->revealDelayTagCount), value_); \
        appFreeUni__FPv(node_); \
    } }
#define CLEAN_A_MID(n) \
    { Node* node_ = (n); \
    if (node_) { \
        Node* child_ = node_->left; \
        CLEAN_A_LEAF(child_); __typeof__(node_->right) savedRightChild = node_->right; \
        child_ = savedRightChild; \
        CLEAN_A_LEAF(child_); \
        appFreeUni__FPv(node_); \
    } }
#define CLEAN_A_PARENT(n) \
    { Node* node_ = (n); \
    if (node_) { \
        Node* child_ = node_->left; \
        CLEAN_A_MID(child_); __typeof__(node_->right) unusedRightChild = node_->right; __typeof__(unusedRightChild) savedRightChild = node_->right; \
        child_ = savedRightChild; \
        CLEAN_A_MID(child_); \
        appFreeUni__FPv(node_); \
    } }
    if (root) {
        CLEAN_A_PARENT(root->left);
        CLEAN_A_PARENT(root->right);
        appFreeUni__FPv(root);
        *(void**)(self + kMsgRevealDelayTagCountOffset) = 0;
        *(void**)(self + kMsgRevealDelayTreeRootOffset) = 0;
        AsCMsgDraw(self)->firstRevealDelayNode = (void*)(((char*)&AsCMsgDraw(self)->revealDelayTreeRoot));
    }
#undef CLEAN_A_MID
#undef CLEAN_A_PARENT
#undef CLEAN_A_LEAF

    root = *(Node**)(self + kMsgSoundTagTreeRootOffset);
#define CLEAN_B_STRING(n) \
    if ((char*)(n) + kFormatNodeKeyOffset != 0 && (char*)(n) + kFormatNodeValueOffset != 0 && \
        ((StrNode*)(n))->isLong) \
        appFreeUni__FPv(*(void**)((char*)(n) + kSoundNodeHeapDataOffset));
#define CLEAN_B_LEAF(n) \
    { Node* node_ = (n); \
    if (node_) { __typeof__(node_->left) savedLeftChild = node_->left; \
        Node* value_ = savedLeftChild; \
        if (value_) TextFormatTree_FreeStringSubtree(((char*)&AsCMsgDraw(self)->soundTagCount), value_); \
        value_ = node_->right; \
        if (value_) TextFormatTree_FreeStringSubtree(((char*)&AsCMsgDraw(self)->soundTagCount), value_); \
        CLEAN_B_STRING(node_); \
        appFreeUni__FPv(node_); \
    } }
#define CLEAN_B_MID(n) \
    { Node* node_ = (n); \
    if (node_) { __typeof__(node_->left) savedLeftChild = node_->left; \
        Node* child_ = savedLeftChild; \
        CLEAN_B_LEAF(child_); \
        child_ = node_->right; \
        CLEAN_B_LEAF(child_); \
        CLEAN_B_STRING(node_); \
        appFreeUni__FPv(node_); \
    } }
#define CLEAN_B_PARENT(n) \
    { Node* node_ = (n); \
    if (node_) { __typeof__(node_->left) unusedLeftChild = node_->left; __typeof__(node_->left) savedLeftChild = node_->left; __typeof__(node_->right) unusedRightChild1 = node_->right; __typeof__(node_->right) savedRightChild = node_->right; __typeof__(node_->right) unusedRightChild2 = node_->right; \
        Node* child_ = savedLeftChild; \
        CLEAN_B_MID(child_); __typeof__(savedRightChild) restoredRightChild = node_->right; \
        child_ = restoredRightChild; \
        CLEAN_B_MID(child_); \
        CLEAN_B_STRING(node_); \
        appFreeUni__FPv(node_); \
    } }
    if (root) {
        CLEAN_B_PARENT(root->left);
        CLEAN_B_PARENT(root->right);
        CLEAN_B_STRING(root);
        appFreeUni__FPv(root);
        *(void**)(self + kMsgSoundTagCountOffset) = 0;
        *(void**)(self + kMsgSoundTagTreeRootOffset) = 0;
        AsCMsgDraw(self)->firstSoundTagNode = (void*)(((char*)&AsCMsgDraw(self)->soundTagTreeRoot));
    }
#undef CLEAN_B_MID
#undef CLEAN_B_PARENT
#undef CLEAN_B_LEAF
#undef CLEAN_B_STRING

    if (!(*(unsigned int*)(self + kMsgRevealSoundStorageOffset) >> 31)) {
        *(unsigned char*)(self + 0x9d) = 0;
        ((ShortFlags*)(((char*)&AsCMsgDraw(self)->revealSoundStorage)))->size = 0;
    } else {
        **(unsigned char**)(self + kMsgRevealSoundHeapDataOffset) = 0;
        *(void**)(self + kMsgRevealSoundHeapLengthOffset) = 0;
    }
    // Phase 3: restore defaults and publish the selected segment.
    AsCMsgDraw(self)->revealStep = kFloat_1_Message;
    AsCMsgDraw(self)->revealPosition = (float)(int)AsCMsgDraw(self)->revealPosition;
    *(void**)(self + kMsgCurrentColorOffset) = *(void**)(self + kMsgDefaultColorOffset);
    CMsgDraw_ParseFormattedText(self, (*(unsigned short***)(self + kMsgLinePointersOffset))[AsCMsgDraw(self)->lineIndex]);
    return 1;
}

// Move both the message origin and its current drawing cursor.
bool Text_SetPosition(char* self, float* position) {
    float y;
    float x;
    x = position[0];
    y = position[1];
    AsCMsgDraw(self)->originX = x;
    AsCMsgDraw(self)->originY = y;
    AsCMsgDraw(self)->cursorX = x;
    AsCMsgDraw(self)->cursorY = y;
    return true;
}

extern char __vt__Q23std11logic_error[], s_sharedCharDeleterVtable[], __vt__Q23std12length_error[];
void* memcpy(void*, const void*, unsigned long);
void* memset(void*, int, unsigned long);
void* __nw__FUl(unsigned long);
void __throw(void*, void*, void*);
extern "C" void __dt__Q23std9exceptionFv(void*, int);
struct LineAllocation {
    char* pointer; char* allocator;
    // Guard newly allocated exception text until shared ownership is established.
    LineAllocation(char* p, char* a) : pointer(p), allocator(a) {}
    // Free text if construction failed before ownership transfer.
    ~LineAllocation() { if (pointer) __dla__FPv(pointer); }
};
struct LineControlBase {
    void* table; unsigned strong, weak;
    // Initialize the exception-message control block's two reference counts.
    LineControlBase() { strong = 1; weak = 1; table = s_sharedCharDeleterVtable; }
};
struct LineControl : LineControlBase {
    char* pointer;
    // Attach the owned message allocation to its shared control block.
    LineControl(char* p) { pointer = p; }
};
struct LineErrorBase {
    void* table;
    // Install the base logic_error interface before message construction.
    LineErrorBase() { table = __vt__Q23std11logic_error; }
    // Destroy the exception base without deleting the enclosing stack object.
    ~LineErrorBase() { __dt__Q23std9exceptionFv(this, -1); }
};
namespace std {
struct length_error : LineErrorBase {
    ::std::__error_message<LineAllocation, LineControl> message_;
    // Construct the shared-message error used when the line-pointer vector grows too far.
    length_error() {
        const char* message = scriptAliasExceptionStrings;
        message_.assign(message);
        table = __vt__Q23std12length_error;
    }
    DECOMP_CT_MEMBER(length_error)
};
}
struct LinePtrVector { unsigned short** volatile data; unsigned size, capacity; };
struct LinePtrBuffer {
    unsigned short** data; volatile unsigned size; unsigned capacity; unsigned* owner; volatile unsigned offset;
    // Start a growth temporary; offset marks the beginning of its live slots.
    LinePtrBuffer(unsigned* p) : data(0), size(0), capacity(0), owner(p), offset(0) {}
    // Release vector storage only; line pointers refer into the message's text.
    ~LinePtrBuffer() { if (data) { size = 0; appFreeUni__FPv(data); } }
};
// Choose the larger capacity increment without creating a temporary reference.
inline const unsigned& LineMax(const unsigned& a, const unsigned& b) { return a < b ? b : a; }

// Find the next line-separator token and return the pointer after it.
inline unsigned short* LineSearch(unsigned short* p, unsigned short* sep) {
    if (p == 0) return 0;
    unsigned short c = *sep;
    while (*p != 0) {
        unsigned short* hit;
        if (p == 0) {
            hit = 0;
        } else {
            unsigned short* a = p;
            unsigned short* b = p;
            int i = 0;
            hit = 0;
            while (*b != 0) {
                if (a == 0) { hit = 0; break; }
                if (c == *b) { hit = p + i; break; }
                a++; b++; i++;
            }
        }
        if (hit == 0) return 0;
        p = hit + 1;
        unsigned short* q = sep + 1;
        int mismatch = 0;
        while (*q != 0) {
            if (*p != *q) { mismatch = 1; break; }
            ++p; ++q;
        }
        if (!mismatch) return p;
        p = hit + 1;
    }
    return 0;
}

// Replace the owned UTF-16 text using the string's current inline/heap representation.
inline void LineAssign(char* self, unsigned short* text, unsigned int capacity) {
    struct Fill { unsigned char value; };
    Fill source;
    Fill terminator = source;
    unsigned int length = wcslen(text);
    unsigned short* end = text + length;
    WString_ReplaceRange(((char*)self + kMsgContentStorageOffset), 0, capacity, text, end, &terminator.value);
}

#pragma push
#pragma use_lmw_stmw on
// Own a copy of the text, index its separator-delimited segments, and parse the first segment.
int Text_SetContent(char* self, unsigned short* text) {
    // Phase 1: copy the content and reset the line-pointer vector.
    unsigned int capacity;
    if ((*(unsigned int*)(self + kMsgContentStorageOffset) >> 31) == 0)
        capacity = *(unsigned char*)(self + kMsgContentStorageOffset) & kTaggedStringInlineMask;
    else
        capacity = *(unsigned int*)(self + kMsgContentHeapLengthOffset);
    LineAssign(self, text, capacity);

    unsigned short* begin;
    if ((*(unsigned int*)(self + kMsgContentStorageOffset) >> 31) == 0)
        begin = (unsigned short*)(self + 0x0e);
    else
        begin = *(unsigned short**)(self + kMsgContentHeapDataOffset);

    unsigned int oldSize = *(volatile unsigned int*)(self + kMsgLineCountOffset);
    *(unsigned int*)(self + kMsgLineCountOffset) = oldSize - oldSize;
    // 0x800A9430 unit: append each line while a line start remains.
    // Phase 2: append borrowed starts of each segment, growing only pointer storage.
    while (begin != 0) {
        LinePtrVector* v = (LinePtrVector*)(((char*)&AsCMsgDraw(self)->linePointers));
        unsigned int size = v->size;
        if (size < v->capacity) {
            unsigned short** slots = v->data;
            unsigned int next = 1 + size;
            *(volatile unsigned*)&v->size = next;
            (slots + next)[-1] = begin;
        } else {
            if (0x3FFFFFFFu - v->capacity < 1) {
                union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
            std::length_error* error = (std::length_error*)storage.bytes;
                error->__ct();
            exception_runtime::__throw(exception_runtime::scriptAliasExceptionStrings + 0x14, error,
                    (void*)exception_runtime::__dt__Q23std12length_errorFv);
            }
            unsigned needed = 1 + v->size - v->capacity;
            LinePtrBuffer temp(&v->capacity);
            unsigned cap = v->capacity;
            if (needed > 0x3FFFFFFFu - cap) {
                union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
            std::length_error* error = (std::length_error*)storage.bytes;
                error->__ct();
            exception_runtime::__throw(exception_runtime::scriptAliasExceptionStrings + 0x14, error,
                    (void*)exception_runtime::__dt__Q23std12length_errorFv);
            }
            if (cap < 0x15555555u) {
                cap += LineMax((3 * (cap + 1)) / 5, needed);
            } else if (cap < 0x2AAAAAAAu) {
                cap += LineMax((cap + 1) / 2, needed);
            } else cap = 0x3FFFFFFF;
            if (cap > 0x3FFFFFFFu) {
                union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
            std::length_error* error = (std::length_error*)storage.bytes;
                error->__ct();
            exception_runtime::__throw(exception_runtime::scriptAliasExceptionStrings + 0x14, error,
                    (void*)exception_runtime::__dt__Q23std12length_errorFv);
            }
            unsigned short** data = (unsigned short**)appMallocDummy__FUl(cap * 4);
            unsigned int oldSize = v->size;
            temp.offset = oldSize;
            (data + oldSize)[temp.size++] = begin;
            unsigned short** oldData = v->data;
            int count = ((char*)(oldData + v->size) - (char*)oldData) / 4;
            temp.capacity = cap;
            temp.data = data;
            int offset = oldSize - count;
            temp.offset = offset;
            memcpy(data + offset, oldData, count * 4);
            memset(oldData, 0, count * 4);
            unsigned int newSize = temp.size + count;
            temp.size = newSize;
            unsigned short** tmp = v->data;
            unsigned tcap = v->capacity;
            v->size = 0;
            v->capacity = temp.capacity;
            temp.capacity = tcap;
            v->data = temp.data;
            temp.data = tmp;
            v->size = newSize;
            temp.size = 0;
        }

        begin = LineSearch(begin, (unsigned short*)&messageTextAlignmentTagStrings);
        if (begin != 0 && *begin == 10) ++begin;
    }
    // Phase 3: select the first segment and rebuild its glyphs.
    AsCMsgDraw(self)->lineIndex = 0;
    CMsgDraw_ParseFormattedText(self, **(unsigned short***)(self + kMsgLinePointersOffset));
    return 1;
}
#pragma pop

// Report whether advancing the current segment would pass the stored line count.
int CMsgDraw_HasNoNextLine(char* self) {
    unsigned int lineCount = *(unsigned int*)((char*)self + kMsgLineCountOffset);
    unsigned int nextLineIndex = *(unsigned int*)((char*)self + kMsgLineIndexOffset) + 1;
    return nextLineIndex >= lineCount;
}

// Skip spaces and copy one bounded, space-delimited UTF-16 token.
unsigned short* WTextParser_ReadToken(void* arg0, unsigned short* arg1,
                            unsigned short* arg2, unsigned short* arg3) {
    unsigned short* start;
    int index = 0;
    unsigned short* scan = arg1;
    while (*scan != 0 || (start = index + arg1, false)) {
        if (*scan != 0x20) {
            start = index + arg1;
            break;
        }
        scan = scan + 1;
        index = (index + 1);
    }

    unsigned short* input = start;
    unsigned short* compare = start;
    unsigned short* output = arg3;
    int written = 0;
    while (*input != 0) {
        unsigned short value = *input;
        if (value == 0x20 || arg2 == compare) {
            break;
        }
        *output = value;
        ++input;
        compare++;
        output++;
        written++;
    }
    arg3[written] = 0;
    return start + written;
}

// Assign a narrow string directly when inline, otherwise replace the destination range.
void String_AssignWithInlineFastPath(unsigned int* arg0, const unsigned int* arg1) {
    unsigned int first = arg0[0];

    if ((first >> 31) == 0) {
        unsigned int second = arg1[0];
        if ((second >> 31) == 0) {
            arg0[0] = second;
            arg0[1] = arg1[1];
            arg0[2] = arg1[2];
            return;
        }
    }
    int dstLength;
    if ((first >> 31) != 0) {
        dstLength = arg0[1];
    } else {
        dstLength = *(unsigned char*)arg0 & kTaggedStringInlineMask;
    }
    unsigned char* source;

    int sourceLength;
    if (((arg1[0]) >> 31) == 0) {
        source = (unsigned char*)arg1 + 1;
        sourceLength = *(unsigned char*)arg1 & kTaggedStringInlineMask;
    } else {
        source = *(unsigned char**)((char*)arg1 + 8);
        sourceLength = arg1[1];
    }
    unsigned char buffer[8]; buffer[4] = buffer[0];
    String_ReplaceRange(arg0, 0, dstLength, source, (sourceLength + source),
                &buffer[4]);
}

#pragma opt_lifetimes off
// Advance the reveal cursor, apply sound tags, and consume wait tags at crossed glyph indices.
int CMsgDraw_UpdateReveal(char* self) {
    // Phase 1: respect wait frames and inspect the current glyph-indexed tags.
    int count = AsCMsgDraw(self)->revealDelayFrames;
    if (count > 0) {
        AsCMsgDraw(self)->revealDelayFrames = count - 1;
    } else {
        int first = (int)AsCMsgDraw(self)->revealPosition;
        int last = (int)(AsCMsgDraw(self)->revealPosition + AsCMsgDraw(self)->revealStep);
        int index = first;
        while (index <= last) {
            char* node = *(char**)(self + kMsgSoundTagTreeRootOffset);
            char* candidate = self + kMsgSoundTagTreeRootOffset;
            while (node != 0) {
                if (!(*(int*)(node + 0xc) < index)) {
                    candidate = node;
                    node = *(char**)node;
                } else {
                    node = *(char**)(node + 4);
                }
            }
            if (candidate == ((char*)&AsCMsgDraw(self)->soundTagTreeRoot) || index < *(int*)(candidate + 0xc))
                candidate = self + kMsgSoundTagTreeRootOffset;
            if (candidate != ((char*)&AsCMsgDraw(self)->soundTagTreeRoot)) {
                int isInline = !((*(unsigned int*)(candidate + 0x10) >> 31) != 0);
                char* value = isInline ? candidate + 0x11 : *(char**)(candidate + 0x18);
                if (strcmp(value, scriptAliasExceptionStrings + kNoRevealSoundTokenOffset) == 0) {
                    unsigned int state = *(unsigned int*)(self + kMsgRevealSoundStorageOffset);
                    if ((state >> 31) == 0) {
                        struct ShortHead { CW_BITS2( unsigned char isLong : 1, unsigned char size : 7) };
                        *(unsigned char*)(self + 0x9d) = 0;
                        ((ShortHead*)(((char*)&AsCMsgDraw(self)->revealSoundStorage)))->size = 0;
                    } else {
                        **(unsigned char**)(self + kMsgRevealSoundHeapDataOffset) = 0;
                        AsCMsgDraw(self)->revealSoundHeapLength = 0;
                    }
                } else {
                    char* text = isInline ? candidate + 0x11 : *(char**)(candidate + 0x18);
                    int capacity;
                    unsigned int state = *(unsigned int*)(self + kMsgRevealSoundStorageOffset);
                    if ((state >> 31) == 0)
                        capacity = *(unsigned char*)(self + kMsgRevealSoundStorageOffset) & kTaggedStringInlineMask;
                    else
                        capacity = AsCMsgDraw(self)->revealSoundHeapLength;
                    unsigned char scratch[8];
                    scratch[0] = scratch[4];
                    String_ReplaceRange(self + kMsgRevealSoundStorageOffset, 0, capacity,
                        (unsigned char*)text, (unsigned char*)text + strlen(text),
                        scratch);
                }
            }

            char* node2 = *(char**)(self + kMsgRevealDelayTreeRootOffset);
            char* found = self + kMsgRevealDelayTreeRootOffset;
            while (node2 != 0) {
                if (!(*(int*)(node2 + 0xc) < index)) {
                    found = node2;
                    node2 = *(char**)node2;
                } else {
                    node2 = *(char**)(node2 + 4);
                }
            }
            if (found == self + kMsgRevealDelayTreeRootOffset || index < *(int*)(found + 0xc))
                found = self + kMsgRevealDelayTreeRootOffset;
            if (found != self + kMsgRevealDelayTreeRootOffset) {
                char* victim = found;
                AsCMsgDraw(self)->revealDelayFrames = *(int*)(found + 0x10);
                if (found == ((char*)AsCMsgDraw(self)->firstRevealDelayNode)) {
                    char* child = *(char**)(found + 4);
                    if (child != 0) {
                        char* left;
                        while ((left = *(char**)child) != 0) child = left;
                        found = child;
                    } else {
                        char* up;
                        while (found != *(char**)(up = (char*)(*(unsigned int*)(found + 8) & ~1u)))
                            found = up;
                        found = up;
                    }
                    (*(char**)&AsCMsgDraw(self)->firstRevealDelayNode) = found;
                }
                ((void (*)(void*, void*))RBTree_RebalanceAfterErase)(victim, *(void**)(self + kMsgRevealDelayTreeRootOffset));
                found = victim;
                appFreeUni__FPv(found);
                AsCMsgDraw(self)->revealDelayTagCount -= 1;
                AsCMsgDraw(self)->revealPosition = (float)index;
                return 1;
            }
            ++index;
        }
        AsCMsgDraw(self)->revealPosition += AsCMsgDraw(self)->revealStep;
        if (AsCMsgDraw(self)->revealPosition > kFloat_4096_Message)
            AsCMsgDraw(self)->revealPosition = kFloat_4096_Message;
    }
    return 1;
}
#pragma opt_lifetimes reset

// Return the wait-frame value slot for a glyph index, inserting its default when absent.
// Return a borrowed integer-value slot; flags select the child side and cached-first update separately.
void* TextFormatIntMap_GetOrInsertValue(void* tree, int* glyphIndex) {
    void* node;
    unsigned char insertAsLeftChild;
    unsigned char updateFirstNode;
    void* insertionSlot = IntegerKeyTree_FindInsertionPosition(tree, glyphIndex, &node, &insertAsLeftChild, &updateFirstNode);
    int* keyValuePair;
    if (node == 0 || *(int*)((char*)node + kFormatNodeKeyOffset) < *glyphIndex) {
        struct {
            int key;
            int value;
        } item;
        item.key = *glyphIndex;
        item.value = *(int*)(&s_defaultRevealWaitFrames);
        void* inserted = BpEntryTree_InsertNode(tree, insertionSlot, insertAsLeftChild, updateFirstNode, &item);
        keyValuePair = (int*)((char*)inserted + kFormatNodeKeyOffset);
    } else {
        keyValuePair = (int*)((char*)node + kFormatNodeKeyOffset);
    }
    return keyValuePair + 1;
}

// Test whether the integer reveal position has reached the cached glyph count.
int CMsgDraw_IsRevealComplete(const char* self) {
    float revealPosition = AsCMsgDraw(self)->revealPosition;
    int glyphCount = AsCMsgDraw(self)->glyphCount;
    return (int)revealPosition >= glyphCount;
}

// Set both default and current packed text colors.
bool CMsgDraw_SetColor(char* self, int rgba) {
    AsCMsgDraw(self)->defaultColor = rgba;
    AsCMsgDraw(self)->currentColor = rgba;
    return true;
}

// Set the message bounds used for alignment and line layout.
bool CMsgDraw_SetLayoutSize(char* self, float* size) {
    float height;
    float width;
    width = size[0];
    height = size[1];
    AsCMsgDraw(self)->layoutWidth = width;
    AsCMsgDraw(self)->layoutHeight = height;
    return true;
}

// Set the horizontal and vertical scale used when appending glyphs.
bool CMsgDraw_SetGlyphScale(char* self, float* scale) {
    float scaleY;
    float scaleX;
    scaleX = scale[0];
    scaleY = scale[1];
    AsCMsgDraw(self)->glyphScaleX = scaleX;
    AsCMsgDraw(self)->glyphScaleY = scaleY;
    return true;
}

#define FIND_IN_TAG(hay, needle, bound, out) \
    { \
        unsigned short* n_ = (unsigned short*)(needle); \
        unsigned short* h_ = (hay); \
        unsigned short* b_ = (bound); \
        out = 0; \
        for (;;) { \
            unsigned short* s_; \
            unsigned short* t_; \
            unsigned short* r_; \
            unsigned short first_; \
            int i_; \
            int bad_; \
            if (h_ == 0) break; \
            first_ = n_[0]; \
            s_ = h_; \
            t_ = h_; \
            i_ = 0; \
            while (*t_ != 0) { \
                if (b_ == s_) { h_ = 0; break; } \
                if (first_ == *t_) { h_ = h_ + i_; break; } \
                s_ = s_ + 1; \
                t_ = t_ + 1; \
                i_ = i_ + 1; \
            } \
            if (*t_ == 0) h_ = 0; \
            if (h_ == 0) break; \
            r_ = h_ + 1; \
            t_ = n_ + 1; \
            bad_ = 0; \
            while (*t_ != 0) { \
                if (*r_ != *t_) { bad_ = 1; break; } \
                r_ = r_ + 1; \
                t_ = t_ + 1; \
            } \
            if (bad_ == 0) { out = r_; break; } \
            h_ = h_ + 1; \
        } \
    }

// Measure one formatted text line, accounting for inline scale tags and font metrics.
// Contract: text is borrowed UTF-16; rect receives {advanceWidth, maxLineHeight}
// in layout units (two floats, not a four-edge rectangle). Null inputs return 0;
// newline or the segment-break tag ends measurement successfully with return 1.
// Syntax: backslash+digit is skipped; backslash+'<...>' consumes through '>'.
// Among the six known tags only scale parses two signed decimal values here;
// reveal/sound/wait/color/alignment tags contribute no glyph width. Unknown tags
// are skipped unless they contain the segment-break token. Tags must be closed.
// Scale-Y may raise the height; glyph advances continue to use the entry X scale
// (the parsed scale-X is not applied by this implementation). No renderer state changes.
int CMsgDraw_MeasureText(char* self, unsigned short* text, float* rect) {
    // Phase 1: snapshot entry scales and seed the two-component extent.
    unsigned int entryScaleWords[2];
    float tagScale[2];
    unsigned short glyphMetrics[8];
    int fontHeight;
    unsigned short ch;

    if (text == 0 || rect == 0)
        return 0;

    entryScaleWords[0] = *(unsigned int*)&AsCMsgDraw(self)->glyphScaleX;
    entryScaleWords[1] = *(unsigned int*)&AsCMsgDraw(self)->glyphScaleY;

    fontHeight = FontAtlas_GetLineHeight(AsCMsgDraw(self)->fontAtlas);
    rect[0] = kFloat_0_MessageDraw;
    rect[1] = *(float*)&entryScaleWords[1] * (float)fontHeight;

    // Phase 2: scan one segment, skipping escapes and interpreting scale-height tags.
    for (; text != 0 && *text != 0; ++text) {
        ch = *text;
        if (ch == kFormatNewline)
            return 1;

        if (ch == kFormatEscape) {
            unsigned short next = text[1];
            if (next >= kFirstDecimalCode128 && next <= kLastDecimalCode128) {
                ++text;
            } else if (next == kFormatTagOpen) {
                unsigned short* tag = text + 2;

                text = (unsigned short*)WString_FindChar(0, kFormatTagClose, tag, 0);

                {
                    unsigned short* numericCursor;
                    int tagId;
                    int i;
                    MessageFormattingToken* entry = messageFormattingTokens;
                    tagId = -1;
                    for (i = 0; i < kFormattingTagCount; ++i) {
                        FIND_IN_TAG(tag, entry->text, text, numericCursor);
                        if (numericCursor != 0) {
                            tagId = messageFormattingTokens[i].id;
                            break;
                        }
                        ++entry;
                    }
                    switch (tagId) {
                    case kTagScale: {
                            int leadingSpaces;
                            int numericToken;
                            int negative;
                            int hasDecimalPoint;
                            float digitsValue;
                            float decimalDivisor;
                            float parsedScale;
                            unsigned short* spaceCursor;
                            unsigned short c;

                            leadingSpaces = 0;
                            spaceCursor = numericCursor;
                            while (*spaceCursor != 0) {
                                if (*spaceCursor != kSpaceCode128)
                                    break;
                                ++spaceCursor;
                                ++leadingSpaces;
                            }
                            numericCursor = numericCursor + leadingSpaces;

                            digitsValue = kFloat_0_MessageDraw;
                            decimalDivisor = kFloat_0_1_Message;
                            negative = 0;
                            hasDecimalPoint = 0;
                            while (numericCursor != 0) {
                                c = *numericCursor;
                                switch (c) {
                                case '-': numericToken = 0; break;
                                case '.': numericToken = 1; break;
                                case '0': numericToken = 2; break;
                                case '1': numericToken = 3; break;
                                case '2': numericToken = 4; break;
                                case '3': numericToken = 5; break;
                                case '4': numericToken = 6; break;
                                case '5': numericToken = 7; break;
                                case '6': numericToken = 8; break;
                                case '7': numericToken = 9; break;
                                case '8': numericToken = 10; break;
                                case '9': numericToken = 11; break;
                                default: numericToken = kEndNumericToken; break;
                                }
                                if (numericToken == kEndNumericToken)
                                    break;
                                switch (numericToken) {
                                case kDecimalToken:
                                    hasDecimalPoint = 1;
                                    break;
                                case kMinusToken:
                                    negative = 1;
                                    break;
                                default:
                                    digitsValue = digitsValue * kFloat_10_MsgText;
                                    digitsValue = digitsValue + ((float)numericToken - kFloat_2_MessageDraw);
                                    break;
                                }
                                if (hasDecimalPoint != 0)
                                    decimalDivisor = decimalDivisor * kFloat_10_MsgText;
                                ++numericCursor;
                            }
                            if (decimalDivisor < kFloat_1_Message)
                                decimalDivisor = kFloat_1_Message;
                            parsedScale = digitsValue / decimalDivisor;
                            if (negative != 0)
                                parsedScale = -parsedScale;
                            tagScale[0] = parsedScale;

                            leadingSpaces = 0;
                            spaceCursor = numericCursor;
                            while (*spaceCursor != 0) {
                                if (*spaceCursor != kSpaceCode128)
                                    break;
                                ++spaceCursor;
                                ++leadingSpaces;
                            }
                            numericCursor = numericCursor + leadingSpaces;

                            digitsValue = kFloat_0_MessageDraw;
                            decimalDivisor = kFloat_0_1_Message;
                            negative = 0;
                            hasDecimalPoint = 0;
                            while (numericCursor != 0) {
                                c = *numericCursor;
                                switch (c) {
                                case '-': numericToken = 0; break;
                                case '.': numericToken = 1; break;
                                case '0': numericToken = 2; break;
                                case '1': numericToken = 3; break;
                                case '2': numericToken = 4; break;
                                case '3': numericToken = 5; break;
                                case '4': numericToken = 6; break;
                                case '5': numericToken = 7; break;
                                case '6': numericToken = 8; break;
                                case '7': numericToken = 9; break;
                                case '8': numericToken = 10; break;
                                case '9': numericToken = 11; break;
                                default: numericToken = kEndNumericToken; break;
                                }
                                if (numericToken == kEndNumericToken)
                                    break;
                                switch (numericToken) {
                                case kDecimalToken:
                                    hasDecimalPoint = 1;
                                    break;
                                case kMinusToken:
                                    negative = 1;
                                    break;
                                default:
                                    digitsValue = digitsValue * kFloat_10_MsgText;
                                    digitsValue = digitsValue + ((float)numericToken - kFloat_2_MessageDraw);
                                    break;
                                }
                                if (hasDecimalPoint != 0)
                                    decimalDivisor = decimalDivisor * kFloat_10_MsgText;
                                ++numericCursor;
                            }
                            if (decimalDivisor < kFloat_1_Message)
                                decimalDivisor = kFloat_1_Message;
                            parsedScale = digitsValue / decimalDivisor;
                            if (negative != 0)
                                parsedScale = -parsedScale;
                            tagScale[1] = parsedScale;

                            if (tagScale[1] * (float)fontHeight > rect[1])
                                rect[1] = tagScale[1] * (float)fontHeight;
                            break;
                        }
                    case kTagRevealStep:
                    case kTagSound:
                    case kTagWaitFrames:
                    case kTagColor:
                    case kTagAlignment:
                        break;
                    default:
                        FIND_IN_TAG(tag, (unsigned short*)(&messageTextAlignmentTagStrings + kSegmentBreakTokenOffset),
                            text, numericCursor);
                        if (numericCursor != 0)
                            return 1;
                        break;
                    }
                }
            }
            continue;
        }

        {
            // Phase 3 per glyph: add atlas advance using the entry X scale and fixed spacing.
            void* font = FontAtlas_GetFontData(AsCMsgDraw(self)->fontAtlas);
            float advance;

            gtl_FontData_FindGlyph(font, ch, glyphMetrics);
            if (*text > kFullWidthGlyphThreshold)
                advance = (float)(short)glyphMetrics[4] * *(float*)&entryScaleWords[0] + kFloat_2_MessageDraw;
            else
                advance = (float)((short)glyphMetrics[4] + (short)glyphMetrics[5]) * *(float*)&entryScaleWords[0] +
                kFloat_2_MessageDraw;
            rect[0] = rect[0] + advance;
        }
    }
    return 1;
}
#undef FIND_IN_TAG

// Return the cached width and height occupied by appended glyphs.
bool CMsgDraw_GetTextExtent(char* self, float* extent) {
    float height;
    float width;
    width = AsCMsgDraw(self)->textWidth;
    height = AsCMsgDraw(self)->textHeight;
    extent[0] = width;
    extent[1] = height;
    return true;
}

// Append an atlas quad, multiply its color, and advance the text cursor and extents.
bool CMsgDraw_AppendGlyph(char* self, unsigned int code, float lineHeight) {
    unsigned char c2[4];
    unsigned char c1[4];
    unsigned char out[4];
    unsigned short glyph[6];
    unsigned int rect[9];
    char* v = ((char*)AsCMsgDraw(self)->glyphs) + AsCMsgDraw(self)->glyphCount * kGlyphStride;
    void* font = FontAtlas_GetFontData(AsCMsgDraw(self)->fontAtlas);
    gtl_FontData_FindGlyph(font, code, glyph);
    ((MessageGlyph*)v)->atlasPage = glyph[2];

    FontAtlas_GetPageDescriptor(AsCMsgDraw(self)->fontAtlas, rect, glyph[2]);
    float atlasW = (float)rect[7];
    float atlasH = (float)rect[8];
    int fontH = FontAtlas_GetLineHeight(AsCMsgDraw(self)->fontAtlas);
    float fontHf = (float)fontH;

    ((MessageGlyph*)v)->u0 = (float)glyph[0] / atlasW;
    ((MessageGlyph*)v)->v0 = (float)glyph[1] / atlasH;
    ((MessageGlyph*)v)->u1 = ((MessageGlyph*)v)->u0 + (float)(short)glyph[4] / atlasW;
    ((MessageGlyph*)v)->v1 = ((MessageGlyph*)v)->v0 + fontHf / atlasH;

    int r6;
    int d;
    int h;
    int ry;
    int topY;
    int leftX;
    leftX = (int)AsCMsgDraw(self)->cursorX;
    topY = (int)AsCMsgDraw(self)->cursorY;
    ry = (int)((float)(short)glyph[4] * AsCMsgDraw(self)->glyphScaleX + (float)leftX);
    h = (int)(fontHf * AsCMsgDraw(self)->glyphScaleY + (float)topY);
    r6 = (int)lineHeight - (h - topY);
    topY += r6;
    h += r6;
    ((MessageGlyph*)v)->left = (float)leftX;
    ((MessageGlyph*)v)->top = (float)topY;
    ((MessageGlyph*)v)->right = (float)ry;
    ((MessageGlyph*)v)->bottom = (float)h;

    *(unsigned int*)c1 = *(unsigned int*)&AsCMsgDraw(self)->colorMultiplier;
    *(unsigned int*)c2 = *(unsigned int*)&AsCMsgDraw(self)->currentColor;
    for (int i = 0; i < 4; ++i)
        out[i] = (unsigned char)(((unsigned int)c1[i] * (unsigned int)c2[i]) / 255);
    ((MessageGlyph*)v)->color = (*(unsigned int*)out);

    float x = (float)ry - AsCMsgDraw(self)->originX;
    float y = (float)h - AsCMsgDraw(self)->originY;
    if (x > AsCMsgDraw(self)->textWidth) AsCMsgDraw(self)->textWidth = x;
    if (y > AsCMsgDraw(self)->textHeight) AsCMsgDraw(self)->textHeight = y;

    if (h > AsCMsgDraw(self)->nextLineY)
        AsCMsgDraw(self)->nextLineY = h + 2;

    float advance;
    if (code > kFullWidthGlyphThreshold) {
        advance = (float)(short)glyph[4] * AsCMsgDraw(self)->glyphScaleX + 2.0f;
    } else {
        advance = AsCMsgDraw(self)->glyphScaleX * (float)((short)glyph[4] + (short)glyph[5]) + 2.0f;
    }
    AsCMsgDraw(self)->cursorX += advance;
    ++AsCMsgDraw(self)->glyphCount;
    return true;
}

// Initialize the text-parser wrapper's shared dispatch/data pointer.
void WTextParser_Init(void** parser) {
    *parser = &s_textParserDispatchTable;
}

// Enable or disable drawing without discarding cached glyphs.
bool CMsgDraw_SetDrawEnabled(char* self, int enabled) {
    AsCMsgDraw(self)->drawEnabled = enabled;
    return true;
}

// Compare the node pointers held by two formatting-map iterators.
int CMsgDrawFormattedTextMap_IteratorsNotEqual(int* left, int* right) {
    int rightNode;
    int leftNode;
    leftNode = *left;
    rightNode = *right;
    int rightDifference = rightNode - leftNode;
    int leftDifference = leftNode - rightNode;
    int anyDifference = rightDifference | leftDifference;
    return (unsigned int)anyDifference >> 31;
}

// Return the tree's root-link address as its end sentinel.
void CMsgDrawFormattedTextMap_GetEndIterator(void** result, char* tree) {
    *result = tree + 4;
}

// Copy a formatting-map iterator and return the destination iterator address.
void* CMsgDrawFormattedTextMap_CopyIterator(void** destination, void** source) {
    *destination = *source;
    return (void*)destination;
}

// Append one narrow character through the string range-replacement operation.
void MWString_PushBackChar(void* arg0, signed char arg1) {
    struct Frame {
        int gap;
        unsigned char key;
        unsigned char pad[3];
        unsigned char buf[2];
    } frame;
    int value;
    frame.buf[0] = arg1;
    unsigned int word = *(unsigned int*)arg0;
    if ((word >> 31) == 0) {
        value = *(unsigned char*)arg0 & kTaggedStringInlineMask;
    } else {
        value = *(unsigned int*)((char*)arg0 + 4);
    }
    frame.key = *(unsigned char*)((char*)&frame.key - 4);
    String_ReplaceRange(arg0, value, 0, frame.buf, frame.buf + 1, &frame.key);
}

#pragma opt_lifetimes off
// Read an optional sign, integer part, and decimal fraction from UTF-16 text.
unsigned short* WTextParser_ReadFloat(void* arg0, unsigned short* text, float* output) {
    unsigned int index = 0;
    unsigned short* scan = text;
    while (*scan != 0 || (text = text + index, false)) {
        if (*scan != 0x20) {
            text = text + index;
            break;
        }
        ++scan;
        ++index;
    }

    float ten;
    float value = kFloat_0_MessageDraw;
    int decimal = 0;
    float scale = kFloat_0_1_Message;
    int negative = 0;
    ten = kFloat_10_MsgText;
    while (text != 0) {
        unsigned short ch = *text;
        int digit;
        switch (ch) {
        case '-': digit = 0; break;
        case '.': digit = 1; break;
        case '0': digit = 2; break;
        case '1': digit = 3; break;
        case '2': digit = 4; break;
        case '3': digit = 5; break;
        case '4': digit = 6; break;
        case '5': digit = 7; break;
        case '6': digit = 8; break;
        case '7': digit = 9; break;
        case '8': digit = 10; break;
        case '9': digit = 11; break;
        default: digit = kEndNumericToken; break;
        }
        if (digit == kEndNumericToken) {
            break;
        }
        switch (digit) {
        case kMinusToken: negative = 1; break;
        case kDecimalToken: decimal = 1; break;
        case 2: case 3: case 4: case 5: case 6:
        case 7: case 8: case 9: case 10: case 11:
            value = value * ten;
            value = value + ((float)digit - kFloat_2_MessageDraw);
            break;
        }
        if (decimal != 0) {
            scale = scale * ten;
        }
        ++text;
    }
    if (output != 0) {
        float one = kFloat_1_Message;
        if (scale < one) {
            scale = one;
        }
        float result = value / scale;
        if (negative != 0) {
            result = -result;
        }
        *output = result;
    }
    return text;
}

#pragma opt_lifetimes reset
#pragma opt_lifetimes off
// Parse a signed decimal argument and return the first unconsumed character.
int* CMsgDraw_ParseNumericArgument(void* arg0, unsigned short* text, int* output) {
    unsigned int index = 0;
    unsigned short* scan = text;
    while (*scan != 0 || (text = text + index, false)) {
        if (*scan != 0x20) {
            text = text + index;
            break;
        }
        ++scan;
        ++index;
    }

    float ten;
    float scale;
    int negative;
    float value = kFloat_0_MessageDraw;
    int decimal = 0;
    scale = kFloat_0_1_Message;
    negative = 0;
    ten = kFloat_10_MsgText;
    while (text != 0) {
        unsigned short ch = *text;
        int digit;
        switch (ch) {
        case '-': digit = 0; break;
        case '.': digit = 1; break;
        case '0': digit = 2; break;
        case '1': digit = 3; break;
        case '2': digit = 4; break;
        case '3': digit = 5; break;
        case '4': digit = 6; break;
        case '5': digit = 7; break;
        case '6': digit = 8; break;
        case '7': digit = 9; break;
        case '8': digit = 10; break;
        case '9': digit = 11; break;
        default: digit = kEndNumericToken; break;
        }
        if (digit == kEndNumericToken) {
            break;
        }
        switch (digit) {
        case kMinusToken: negative = 1; break;
        case kDecimalToken: decimal = 1; break;
        case 2: case 3: case 4: case 5: case 6:
        case 7: case 8: case 9: case 10: case 11:
            value = value * ten;
            value = value + ((float)digit - kFloat_2_MessageDraw);
            break;
        }
        if (decimal != 0) {
            scale = scale * ten;
        }
        ++text;
    }
    float one = kFloat_1_Message;
    if (scale < one) {
        scale = one;
    }
    float result = value / scale;
    if (negative != 0) {
        result = -result;
    }
    *output = (int)result;
    return (int*)text;
}
#pragma opt_lifetimes reset

// Perform a lower-bound tree search and return either the matching node or the end sentinel.
void CMsgDrawFormattedTextMap_Find(void** result, void* tree, int* key) {
    char* base = (char*)tree;
    void* node = *(void**)(base + 4);
    void* parent = base + 4;
    while (node != 0) {
        int nodeKey = ((FormatTreeNode*)node)->glyphIndex;
        int keyValue = *key;
        if (nodeKey >= keyValue) {
            parent = node;
            node = *(void**)node;
        } else {
            node = *(void**)((char*)node + 4);
        }
    }
    if (parent != base + 4 && *key < ((FormatTreeNode*)parent)->glyphIndex) {
        parent = base + 4;
    }
    *result = parent;
}

// Find an insertion parent and the left/right attachment flags for a glyph-index tree.
void* CMsgDrawFormattedTextMap_FindInsertionParent(void* tree, int* key, void** parentOut,
                  unsigned char* leftOut, unsigned char* rightOut) {
#pragma opt_lifetimes off
    unsigned char no = 0;
    unsigned char yes = 1;
    void* slot = (char*)tree + 4;
    void* node = (void*)*(volatile void**)slot;
    *parentOut = 0;
    *leftOut = yes;
    *rightOut = yes;
    while (node != 0) {
        int currentKey = *key;
        slot = node;
        int nodeKey = ((FormatTreeNode*)node)->glyphIndex;
        if (currentKey < nodeKey) {
            node = *(void**)node;
            *leftOut = yes;
        } else {
            *parentOut = node;
            node = *(void**)((char*)node + 4);
            *leftOut = no;
            *rightOut = no;
        }
    }
    return slot;
}
#pragma opt_lifetimes reset

} // extern "C"
