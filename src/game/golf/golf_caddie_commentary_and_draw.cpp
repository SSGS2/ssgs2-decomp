#include <decomp/offset_of.h>
#include "game/singletons.h"
#include "game/wvector.h"
#include "game/types.h"
// Translation unit 0x8012C52C-0x8012D504 (3 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "classes/CSharedDoc.h"
#include "classes/CGolfDoc.h"
#include "classes/FadeState.h"
#include "classes/CGolfCaddie.h"
extern "C" {

typedef Vec3f Vec3;
struct ObjAtAC { Vec3 v; };
struct ObjAt88 { char pad[0x88]; };
struct State {
    char pad0[0x10]; unsigned char active;
};
struct Root {
    State* state;
    char beforeEffectReady[0x40];
    unsigned char effectReady;
};
struct RenderObject {
    virtual void v00(); virtual void v01(); virtual void v02(); virtual void v03();
    virtual void v04(); virtual void v05(); virtual void v06(); virtual void v07();
    virtual void v08(); virtual void v09(); virtual void v10(); virtual void v11();
    virtual void v12(); virtual void v13(); virtual void v14(); virtual void v15();
    virtual void v16(); virtual void v17(); virtual void v18(); virtual void v19();
    virtual void v20(); virtual void v21(); virtual void v22(); virtual void v23();
    virtual void v24(); virtual void v25(); virtual void draw(int, int);
};
struct CaddieDrawState {
    char pad0[0x1c]; Root* root;
};
// The draw entry uses this root-only view, CGolfCaddie's shared effect handle,
// and the typed player transform array below. These are overlapping views of
// the same borrowed owner; the transform cache owns matrix values only.

struct PlayerAttachmentTransform { float basis[9]; Vec3 translation; };
struct CaddieAttachmentTransforms {
    char beforeTransforms[0x88];
    PlayerAttachmentTransform players[4];
};
enum CaddieTransformOffsets {
    kPlayerTransformsOffset = (int)DECOMP_OFFSETOF(CaddieAttachmentTransforms, players),
    kFirstTranslationOffset = (int)DECOMP_OFFSETOF(CaddieAttachmentTransforms, players[0].translation)
};
struct BoneTransform { char beforeMatrix[0x78]; float matrix[12]; };
struct BoneEntry { BoneTransform* bone; };

// Only the used entries are named; the prefix preserves their original pool offsets.
struct CaddieStringPool {
    char otherStrings[0x10E];
    char headNub[14];
    char bubbleMessage[7];
    char breatheBone[8];
};
typedef char PlayerAttachmentTransform_stride_check[(sizeof(PlayerAttachmentTransform) == 0x30) ? 1 : -1];
typedef char CaddieStringPool_breathe_offset_check[((int)DECOMP_OFFSETOF(CaddieStringPool, breatheBone) == 0x123) ? 1 : -1];

enum CaddieAndStreamConstants {
    kFadeTransitionState = 2, kYAxis = 1, kRandomCommentaryCount = 5,
    kStreamBadBit = 1, kStreamFailBits = 5, kStreamUnitBuffer = 0x2000,
    kStreamAdjustMask = 0xB0, kStreamAdjustLeft = 0x20,
    kStreamInput = 8, kStreamOutput = 0x10, kStreamInputOutput = 0x18,
    kSpaceCharacter = 32, kCtypeWidenSlot = 7, kStreamOverflowSlot = 0x38 / 4
};

extern signed char s_fadeOverlayInitialized;
extern void* g_sharedDoc;
extern float kFloat_neg1_5707964_CaddieRotation;
extern float kFloat_0_GolfCaddie;
extern char s_fadeOverlay[];
extern char s_fadeOverlayDestructorRecord[];
extern char golfCaddieStrings[];
extern void FadeOverlay_Init(void*);
extern void ResourceState_Dtor();
extern void __register_global_object(void*, void (*)(), void*);
extern int FadeState_GetState(void*);
extern int SharedDoc_IsCourse8Or13();
extern void* CPetBody_GetPuppet(void*, const char*);
extern void* WPuppet_FindNamedBoneEntry(void*, const char*);
extern void Matrix_SetAxisRotation(Vec3*, float, int);
extern void Matrix_CopyAffine(void*, void*);
extern void __as__7WVectorFRC7WVector(void*, Vec3*);
extern void Matrix_ConcatAffineAssign(void*, Vec3*);
extern void RingEffect_Draw(void*, Root*);
extern void OStreamSentry_Ctor(void*, void*);
extern void OStream_Flush(void*);
extern void MWStream_ThrowFailure(void*, const char*);
extern void __throw(void*, void*, void*);
extern unsigned int strlen(const char*);
extern char iosClearFailureMessage[];
extern void* g_randomHouse;
extern int s_ctypeFacetId;
extern int s_localeFacetIdCounter;
extern char iosBaseVtable[];
extern char iStreamVtableGroup[];
extern char s_basicOstreamVtable[];
extern char outputStringStreamVtables[];
extern char s_appStringstreamVTables[];
extern char stringStreamBufferVtable[];
extern char s_appStringbufVTable[];
struct ActionName { const char* text; unsigned int reserved; };
extern const ActionName caddieActionDescriptors[];
extern unsigned int MT19937_NextU32(void*);
extern void MWIOS_InitializeStreamBuffer(void*, void*);
extern void MWCtypeFacet_Construct(void*, int, int, int);
extern void MWLocale_InstallFacet(void*, void*, int);
extern void MWSharedOwner_ReleaseReference(void*);
extern void* MWLocale_GetDefault();
extern int memcmp(const char*, const char*, unsigned int);
extern void MWStream_CopyLocale(void*, void*);
// Dispatches num_put's signed 32-bit formatter at vtable +0x10.
extern void MWOStream_InsertSigned32(void*, int);
extern void MWStringStream_CopyBufferString(void*, void*);
extern void String_ReserveCapacity(void*, unsigned int);
extern void* WPuppet_FindOrCacheBone(void*, const char*);
extern void std_ios_base_dtor(void*, short);
extern void appFreeUni__FPv(void*);
extern void* __nw__FUl(unsigned int);
extern void __dl__FPv(void*);

struct TreeNode { TreeNode* left; TreeNode* right; TreeNode* parent; unsigned int stringFlags; unsigned int size; char* stringData; unsigned char eligibilityThreshold; char pad19[3]; unsigned int messageFlags; unsigned int messageLength; char* messageData; };
// Tree is a partial external-object view. +0x1C selects the root link and its
// address doubles as the end sentinel; +0x14 is a pointer used as a byte
// source for eligibility. Its low-address stringFlags byte is compared directly
// to entry+0x18, NOT decoded as a string length. What that byte represents in the
// resource format is unresolved. Do not replace this predicate with a rank test.
struct Tree { char pad[0x14]; TreeNode* eligibilityData; char pad18[4]; TreeNode* root; };
struct Handle { char pad[0x34]; unsigned int refs; };
struct Item { char pad[0x38]; unsigned char ch; };
struct CaddieCommentaryState { char pad[0x1c]; Tree* tree; char pad2[0x128]; unsigned char state[2]; };
// state is a prefix view, not a two-element bounds contract: state[0] at +0x148
// latches the last action, and state[actionIndex+1] is that action's busy flag.
// The full action count must agree with caddieActionDescriptors and the owner
// allocation; neither count is available here. The caller supplies a valid index.
struct RootString { char pad[0xC]; unsigned char length; };
struct TmpStream { char bytes[0x4c]; };
struct TmpObject { char bytes[0x4c]; };
extern void MWStream_CopyLocale(void*, void*);

struct InnerBuffer {
    void** vtable;
    char pad04[0x10];
    unsigned char* current;
    unsigned char* end;
};
struct StreamBuffer {
    char pad0[0x24];
    InnerBuffer* buffer;
    char pad28[4];
    int width;
    unsigned short flags;
    unsigned char state;
    unsigned char exceptionMask;
    char pad34[4];
    signed char fill;
};
struct Stream {
    StreamBuffer* buffer;
};
void* OStream_WriteCString(Stream*, const char*);
// Write one character through the put area, falling back to the streambuf overflow virtual.
static int PutStreamCharacter(InnerBuffer* b, signed char c) {
    if (!b) return -1;
    if (b->current < b->end) {
        unsigned char* written = b->current++;
        *written = (unsigned char)c;
        return *written;
    }
    typedef int (*Fn)(InnerBuffer*, unsigned int);
    return ((Fn)b->vtable[kStreamOverflowSlot])(b, (unsigned char)c);
}
// Record an insertion failure and throw only when its state intersects the exception mask.
static void RecordStreamInsertionFailure(Stream* s, int bad) {
    if (!bad) return;
    StreamBuffer* state = s->buffer;
    state->state |= kStreamFailBits;
    if (!state->buffer) state->state |= kStreamBadBit;
    if (state->state & state->exceptionMask) MWStream_ThrowFailure(state, iosClearFailureMessage);
}

struct Ios {
    void* vptr;
    char pad04[0x30];
    int state;
    signed char fill;
    char pad39[3];
    int virtualBaseOffset;
    char pad40[0xc];
};
struct Locale {
    struct Facets { void** data; unsigned int size; }* facets;
    void* ref;
    // Release this temporary's locale reference, without owning the facet array.
    ~Locale() throw() { if (ref) MWSharedOwner_ReleaseReference(ref); }
};
struct Str {
    unsigned int stringFlags;
    unsigned int size;
    char* stringData;
    // Free heap string data; inline storage belongs to the containing object.
    ~Str() { if (stringFlags >> 31) appFreeUni__FPv(stringData); }
};
struct OStr {
    Ios* inputIos;
    int w4;
    void* inputVtable;
    Ios* outputIos;
    void* outputVtable;
    void* stringBufferVtable;
    void* getBegin;
    void* getCurrent;
    void* getEnd;
    void* putBegin;
    void* putCurrent;
    void* putEnd;
    Locale::Facets* locFacets;
    void* locRef;
    unsigned char mode;
    char pad39[3];
    char* bufferBase;
    unsigned int bufferStringFlags;
    unsigned int bufferStringLength;
    char* bufferStringData;
    Ios ios;
    // Tear down stream/locale/string subobjects in CodeWarrior virtual-base order.
    ~OStr() throw() {
        inputIos->virtualBaseOffset = (char*)&ios - (char*)inputIos;
        if (bufferStringFlags >> 31) appFreeUni__FPv(bufferStringData);
        if (locRef) MWSharedOwner_ReleaseReference(locRef);
        inputIos->virtualBaseOffset = (char*)&stringBufferVtable - (char*)inputIos;
        outputIos->virtualBaseOffset = (char*)&stringBufferVtable - (char*)outputIos;
        inputIos->virtualBaseOffset = (char*)&outputIos - (char*)inputIos;
        std_ios_base_dtor(&ios, 0);
    }
};
// Construct the ctype facet used to widen the stream's default fill character.
struct Facet { char b[0x18]; Facet() { MWCtypeFacet_Construct(this, 0, 0, 0); } };
struct LocaleReference { void* vtable; int references; };
// Copy a locale's facet/reference pair and retain the shared reference.
static inline void copyLocale(void** dst, Locale* src) throw() {
    dst[0] = src->facets;
    void* r = src->ref;
    dst[1] = r;
    if (r) ++((LocaleReference*)r)->references;
}
// Return the smaller unsigned bound by reference for the byte comparison helpers.
static inline const unsigned int& mn(const unsigned int& x, const unsigned int& y) { return x < y ? x : y; }
// Decode a narrow string's heap length or seven-bit inline length.
template <class S> static inline unsigned int slen(S* s) { return (s->stringFlags >> 31) ? s->size : *(unsigned char*)&s->stringFlags & 0x7f; }
// Borrow the narrow string bytes from heap or in-object storage.
template <class S> static inline const char* sptrOf(S* s) { return (s->stringFlags >> 31) ? s->stringData : (char*)&s->stringFlags + 1; }
// Compare a node's action key against a temporary key, preserving prefix ordering.
static int strCompare(TreeNode* n, Str* k) {
    const char* kp = sptrOf(k);
    unsigned int t = mn(slen(n), slen(n));
    int cmp = memcmp(sptrOf(n), kp, mn(slen(k), t));
    if (!cmp) cmp = t < slen(k) ? -1 : t == slen(k) ? 0 : 1;
    return cmp;
}
// Compare a temporary action key against a node (the reverse lookup direction).
static int strCompare2(Str* k, TreeNode* n) {
    const char* np = sptrOf(n);
    unsigned int t = mn(slen(k), slen(k));
    int cmp = memcmp(sptrOf(k), np, mn(slen(n), t));
    if (!cmp) cmp = t < slen(n) ? -1 : t == slen(n) ? 0 : 1;
    return cmp;
}
struct Words3 { void* a; void* b; void* c; };
struct HeadBonePosition { char beforePosition[0x9C]; Words3 position; };
extern void IActor_DispatchMessage(void*, void*, const char*, int, const char*, void*, int, int);
extern void String_ReplaceRange(void*, int, int, const char*, const char*, void*);

// Install stream virtual bases and an empty owned buffer after locale fill setup.
// Locale references retained here are released by OStr's existing destructor.
static inline void InitializeCommentaryStreamBuffer(OStr& os) {
    os.outputVtable = s_basicOstreamVtable;
    *(void**)os.outputIos = s_basicOstreamVtable + 0xc;
    os.outputIos->virtualBaseOffset = (char*)&os.stringBufferVtable - (char*)os.outputIos;
    os.inputVtable = outputStringStreamVtables;
    *(void**)os.inputIos = outputStringStreamVtables + 0x18;
    os.outputVtable = outputStringStreamVtables + 0xc;
    os.inputIos->virtualBaseOffset = (char*)&os.stringBufferVtable - (char*)os.inputIos;
    os.inputVtable = s_appStringstreamVTables;
    *(void**)os.inputIos = s_appStringstreamVTables + 0x18;
    os.outputVtable = s_appStringstreamVTables + 0xc;
    os.inputIos->virtualBaseOffset = (char*)&os.ios - (char*)os.inputIos;
    os.stringBufferVtable = stringStreamBufferVtable;
    os.getBegin = 0;
    os.getCurrent = 0;
    os.getEnd = 0;
    os.putBegin = 0;
    os.putCurrent = 0;
    os.putEnd = 0;
    copyLocale((void**)&os.locFacets, (Locale*)MWLocale_GetDefault());
    os.stringBufferVtable = s_appStringbufVTable;
    os.bufferStringFlags = 0;
    os.mode = kStreamInputOutput;
    os.bufferStringLength = 0;
    os.bufferStringData = 0;
    char* pb = (os.bufferStringFlags >> 31) ? 0 : (char*)&os.bufferStringFlags + 1;
    os.bufferBase = pb;
    if (os.mode & kStreamOutput) { os.putCurrent = pb; os.putBegin = pb; os.putEnd = pb; }
    if (os.mode & kStreamInput) { os.getBegin = pb; os.getCurrent = pb; os.getEnd = pb; }



}

// Read the unresolved eligibility resource byte without treating it as string metadata.
// Keep the low-address byte exactly: this is not a decoded length or a proven rank.
static inline unsigned char CommentaryEligibilityByte(const TreeNode* resource) {
    return *(const unsigned char*)&resource->stringFlags;
}

#pragma opt_common_subs off
// Look up an action plus variant number, check its caddie eligibility, and emit a head-anchored bubble.
// actionIndex must index both descriptors and owner busy storage. variantNumber
// zero chooses 1..5 randomly; nonzero is used verbatim. force bypasses the global
// action gate only, not the per-action busy bit or resource eligibility predicate.
// Returns nothing; even a missing/ineligible key leaves its busy bit SET. This
// routine does not roll it back. Puppet/bone/tree data is borrowed; the dispatched
// position is a stack copy, requiring synchronous consumption/copy by the receiver.
void CGolfCaddie_TriggerCommentary(CaddieCommentaryState* self, unsigned char actionIndex, int force, unsigned char variantNumber) {
    // Phase 1: reject busy actions, latch this one, and choose its variant suffix.
    if (!self->tree) return;
    unsigned char* actionBusy = self->state + actionIndex + 1;
    if (*actionBusy) return;
    if (!force && self->state[0]) return; // 0x8012C52C: Owner::state starts at +0x148.
    *actionBusy = 1;
    variantNumber = (!variantNumber) ? (unsigned char)(MT19937_NextU32(g_randomHouse) % kRandomCommentaryCount + 1) : variantNumber;

    // Phase 2: construct a CodeWarrior stringstream, including virtual bases and
    // a locale ctype facet to widen the fill space. This is formatting machinery,
    // not commentary selection; the resulting key is descriptor.text + decimal N.
    OStr os;
    // These members are not constructed yet when ios initialization can throw.
    os.bufferStringFlags = 0;
    os.locRef = 0;
    os.ios.vptr = iosBaseVtable;
    os.inputIos = &os.ios;
    os.ios.vptr = iStreamVtableGroup + 0xc;
    os.outputIos = &os.ios;
    os.inputVtable = iStreamVtableGroup;
    os.inputIos->virtualBaseOffset = (char*)&os.outputIos - (char*)os.inputIos;
    Ios* b = os.inputIos;
    os.w4 = 0;
    MWIOS_InitializeStreamBuffer(b, &os.stringBufferVtable); // original r4 = r31+0x10c = &os+0x14 at 0x8012C5E4 (basic_ios init with the stream buffer)
    b->state = 0;
    int ch;
    {
        Locale loc;
        MWStream_CopyLocale(&loc, b);
        if (!s_ctypeFacetId) s_ctypeFacetId = ++s_localeFacetIdCounter;
        unsigned int index = s_ctypeFacetId;
        void* item = 0;
        if (index < loc.facets->size)
            item = loc.facets->data[index];
        if (!item) {
            Facet* p = new Facet;
            if (!s_ctypeFacetId) s_ctypeFacetId = ++s_localeFacetIdCounter;
            MWLocale_InstallFacet(loc.facets, p, s_ctypeFacetId);
            item = loc.facets->data[index];
        }
        typedef int (*GetChar)(void*, signed char);
        ch = ((GetChar)(*(void***)item)[kCtypeWidenSlot])(item, kSpaceCharacter);
    }
    b->fill = ch;

// Finish virtual-base setup and initialize the empty stream buffer.
    InitializeCommentaryStreamBuffer(os);

    // Phase 3: format the key and copy it out of the stream's owned buffer.
    MWOStream_InsertSigned32(OStream_WriteCString((Stream*)&os.outputIos, caddieActionDescriptors[actionIndex].text), variantNumber);
    Str actionName;
    MWStringStream_CopyBufferString(&actionName, &os);
    char* text = (actionName.stringFlags >> 31) ? actionName.stringData : (char*)&actionName.stringFlags + 1;

    // Phase 4: lower_bound on action keys, then the reverse comparison verifies
    // equality. Narrow keys use inline bytes at +1; message text uses +2.
    Tree* tree = self->tree;
    TreeNode* matchingEntry = (TreeNode*)&tree->root; // 0x8012C52C: sentinel at the root-link field.
    {
    Str key;
    key.stringFlags = 0;
    key.size = 0;
    key.stringData = 0;
    unsigned int len = strlen(text);
    String_ReserveCapacity(&key, len);
    char allocSrc;
    char alloc = allocSrc;
    String_ReplaceRange(&key, 0, 0, text, text + len, &alloc);

    TreeNode* node = tree->root;
    matchingEntry = (TreeNode*)&tree->root;
    unsigned int keyHeapFlag = key.stringFlags >> 31;
    unsigned char keyInlineLength = *(unsigned char*)&key & 0x7f;
    char* keyHeapData = key.stringData;
    unsigned int keyHeapLength = key.size;
    bool keyUsesHeap = !!keyHeapFlag;
    for (; node; ) {
        const char* keyText = !keyUsesHeap ? (char*)&key.stringFlags + 1 : keyHeapData;
        unsigned int keyLength = !keyUsesHeap ? keyInlineLength : keyHeapLength;
        unsigned int t = mn(slen(node), slen(node));
        int cmp = memcmp(sptrOf(node), keyText, mn(keyLength, t));
        if (!cmp) { if (t < keyLength) cmp = -1; else if (t == keyLength) cmp = 0; else cmp = 1; }
        if (cmp >= 0) { matchingEntry = node; node = node->left; }
        else node = node->right;
    }
    if (matchingEntry != (TreeNode*)&tree->root) {
        const char* nodeText = sptrOf(matchingEntry);
        unsigned int nodeLength = slen(matchingEntry);
        const char* keyText = !keyUsesHeap ? (char*)&key.stringFlags + 1 : keyHeapData;
        unsigned int keyLength = !keyUsesHeap ? keyInlineLength : keyHeapLength;
        unsigned int t = mn(keyLength, keyLength);
        int cmp = memcmp(keyText, nodeText, mn(nodeLength, t));
        if (!cmp) { if (t < nodeLength) cmp = -1; else if (t == nodeLength) cmp = 0; else cmp = 1; }
        matchingEntry = (cmp >= 0) ? matchingEntry : (TreeNode*)&tree->root;
    } else {
        matchingEntry = (TreeNode*)&tree->root;
    }
    }
    // Phase 5: apply the raw resource-byte threshold, then dispatch the message
    // at the head-nub bone position. Coordinate space is inherited from the bone
    // cache; this routine applies no local/world transform to that position.
    if (matchingEntry == (TreeNode*)&self->tree->root) return;
    if (self->tree->eligibilityData) {
        if (CommentaryEligibilityByte(self->tree->eligibilityData) < matchingEntry->eligibilityThreshold) return;
    } else if (matchingEntry->eligibilityThreshold) {
        return;
    }
    self->state[0] = actionIndex;
    void* resource = CPetBody_GetPuppet(*(void**)self->tree, 0);
    CaddieStringPool* strings = (CaddieStringPool*)golfCaddieStrings;
    Words3* data = &((HeadBonePosition*)WPuppet_FindOrCacheBone(resource, strings->headNub))->position;
    Words3 saved = *data;
    IActor_DispatchMessage(self, self, strings->bubbleMessage, 0, (matchingEntry->messageFlags >> 31) ? matchingEntry->messageData : (char*)&matchingEntry->messageFlags + 2, &saved, 0, 0);
}

#pragma opt_common_subs reset
// Insert text with field-width padding and update stream error state, preserving sentry cleanup.
void* OStream_WriteCString(Stream* stream, const char* text) {
    struct Sentry {
        unsigned char ready, unwinding;
        char paddingCount[2];
        Stream* stream;
        // Flush unit-buffered output on normal sentry exit if the stream is good.
        ~Sentry() {
            StreamBuffer* state = stream->buffer;
            if (!(state->state & kStreamFailBits) && (state->flags & kStreamUnitBuffer) && !unwinding)
                OStream_Flush(stream);
        }
    } sentry;
    OStreamSentry_Ctor(&sentry, stream);
    if (sentry.ready) {
        int failed = 1;
        try {
        unsigned int textLength = strlen(text);
        StreamBuffer* output = stream->buffer;
        int paddingCount = output->width > (int)textLength ? output->width - textLength : 0;
        unsigned short mode = output->flags & kStreamAdjustMask;
        InnerBuffer* beforeText = output->buffer;
        signed char fill = output->fill;
        if (mode != kStreamAdjustLeft) {
            for (int i = 0; i < paddingCount; ++i)
                if (PutStreamCharacter(beforeText, fill) == -1) beforeText = 0;
        }
        InnerBuffer* afterText = beforeText;
        for (int i = 0; i < (int)textLength; ++i)
            if (PutStreamCharacter(afterText, (signed char)text[i]) == -1) afterText = 0;
        InnerBuffer* afterPadding = afterText;
        if (mode == kStreamAdjustLeft) {
            for (int i = 0; i < paddingCount; ++i)
                afterPadding = (PutStreamCharacter(afterPadding, fill) != -1) ? afterPadding : 0;
        }
        output->width = 0;
        failed = !afterPadding;
        } catch (...) {
            stream->buffer->state |= kStreamBadBit;
            if (stream->buffer->exceptionMask & kStreamBadBit) {
                sentry.unwinding = 1;
                throw;
            }
        }
        try { RecordStreamInsertionFailure(stream, failed); }
        catch (...) { sentry.unwinding = 1; throw; }
    }
    return stream;
}

#pragma opt_common_subs off
// Draw the visible caddie and ring effect, caching per-player breathing-bone transforms on two courses.
void CGolfCaddie_Draw(CaddieDrawState* self) {
    // Phase 1: visibility/root and global transition gates precede all rendering.
    if (!((CGolfCaddie*)self)->caddieVisible || !self->root)
        return;

    if (!Sing_805DC608()) {
        if (!s_fadeOverlayInitialized) {
            FadeOverlay_Init(s_fadeOverlay);
            __register_global_object(s_fadeOverlay, ResourceState_Dtor, s_fadeOverlayDestructorRecord);
            s_fadeOverlayInitialized = 1;
        }
        Sing_805DC608() = s_fadeOverlay;
    }
    if (FadeState_GetState(Sing_805DC608()) == kFadeTransitionState) {
        if (!Sing_805DC608()) {
            if (!s_fadeOverlayInitialized) {
                FadeOverlay_Init(s_fadeOverlay);
                __register_global_object(s_fadeOverlay, ResourceState_Dtor, s_fadeOverlayDestructorRecord);
                s_fadeOverlayInitialized = 1;
            }
            Sing_805DC608() = s_fadeOverlay;
        }
        if (!((FadeState*)Sing_805DC608())->captureRequested) // 0x8012D2DC.
            return;
    }

    // Phase 2: on courses 8/13 cache the current player's breathing-bone affine
    // transform. currentPlayer must be 0..3. Copy the cached bone basis, compose
    // a -pi/2-radian Y rotation with translation zeroed, then restore translation.
    // This changes orientation only. Matrix_ConcatAffineAssign defines the
    // multiplication convention; model/world direction is unresolved in this unit.
    // The inline cache owns values, not the borrowed puppet/bone objects.
    if (SharedDoc_IsCourse8Or13()) {
        void* caddieBody = self->root->state;
        if (caddieBody) {
            const char* name = ((CaddieStringPool*)golfCaddieStrings)->breatheBone;
            void* puppet = CPetBody_GetPuppet(caddieBody, name);
            void* boneEntry = WPuppet_FindNamedBoneEntry(puppet, name);
            if (boneEntry) {
                void* golfDoc = (void*)((CSharedDoc*)g_sharedDoc)->golfDoc;
                unsigned char playerIndex = ((CGolfDoc*)golfDoc)->currentPlayer;
                char rotation[0x30];
                Matrix_SetAxisRotation((Vec3*)rotation, kFloat_neg1_5707964_CaddieRotation, kYAxis);
                int playerTransformOffset = playerIndex * sizeof(PlayerAttachmentTransform);
                // Byte-based field-derived addresses preserve CodeWarrior's original register lifetimes.
                void* transform = (char*)self + playerTransformOffset + kPlayerTransformsOffset;
                Matrix_CopyAffine(transform, ((BoneEntry*)boneEntry)->bone->matrix);
                float zero = kFloat_0_GolfCaddie;
                struct Position { void* a; void* b; void* c; } *positionWords = (Position*)((char*)self + playerTransformOffset + kFirstTranslationOffset);
                Position saved = *positionWords;
                Vec3* position = (Vec3*)positionWords;
                Vec3 zeroVec = { zero, zero, zero };
                *(WVector*)position = *(WVector*)&zeroVec;
                Matrix_ConcatAffineAssign(transform, (Vec3*)rotation);
                *(WVector*)position = *(WVector*)((Vec3*)&saved);
            }
        }
    }

    // Phase 3: draw the body and, once ready, the attached ring effect.
    void* caddieBody = self->root->state;
    if (caddieBody) {
        ((RenderObject*)caddieBody)->draw(1, 1);
    }
    if (((CGolfCaddie*)self)->texture3D && self->root->effectReady)
        RingEffect_Draw(((CGolfCaddie*)self)->texture3D, self->root);
}

#pragma opt_common_subs reset
} // extern "C"
