#include <decomp/static_init.h>
#include "classes/SoundManagerState.h"
#include "classes/FloatTween.h"
#include "classes/TiXmlNodeLayout.h"
#include "classes/CFxSequence.h"
#include "classes/WPuppet.h"

// This unit bundles several unrelated actor/audio helpers: sound-manager volume tweens, attached-effect
// slot setup, bone-transform lookups for animation commands, the animation-command token parser
// (Parse_AsteriskToken and friends) and the hash-path lookup shutdown.
struct SoundManagerTweens { FloatTween soundVolume; FloatTween musicVolume; };
// Position plus primary/secondary transform pointers of an effect attachment (also the tail of
// SprayAttachmentFields below, which prefixes it with 8 reserved bytes and the owning attachment).
struct EffectAttachmentSlots {
    float positionX, positionY;
    void* transforms[4];
    void* secondaryTransforms[4];
};
struct SprayAttachmentFields {
    char reserved[8];
    void* attachment;
    float positionX, positionY;
    void* transforms[4];
    void* secondaryTransforms[4];
};
struct BoneTransformView {
    char reserved[0x78];
    float worldTransform[12];
};
struct BoneSecondaryTransformView {
    char reserved[0x18];
    float transform[12];
};
struct NamedBonePair {
    BoneTransformView* bone;
    BoneSecondaryTransformView* secondary;
};
struct AttachmentNameFilter { char reserved[0x18]; const char* name; };
struct ConditionTokenTable { char reserved[0x70]; char* tokens[1]; };
struct AnimationTimeRange { float first; float last; };
struct AnimationCommandKey { int frame; int reserved; void* commands; };
struct ActorInstanceSelection { char reserved[0xA]; unsigned char index; };
struct ActorInstanceTable { char reserved[4]; ActorInstanceSelection* selected; };
struct CtypeByteTable { char reserved[0x10]; unsigned char* lowercase; };
struct CLocaleFields { char reserved[0x38]; CtypeByteTable* ctype; };
// Command order comes from s_animationEventHandlerNames (804B6180); these indices
// are a protocol shared with that table, not C++ vtable byte offsets.
// The dispatcher borrows the parser/context and passes a mutable char** cursor.
// Concrete parser subclasses implement the effects; token names alone do not
// establish argument syntax or ownership for shot/tail/swing/fly/give.
enum AnimationCommandIndex {
    kAnimationCommandSound = 0, // *snd(
    kAnimationCommandShot = 1, // *shot
    kAnimationCommandTail = 2, // *tail
    kAnimationCommandShowClub = 3, // *showclub
    kAnimationCommandHideClub = 4, // *hideclub
    kAnimationCommandSwing = 5, // *swing
    kAnimationCommandLockClub = 6, // *lockclub
    kAnimationCommandEffect = 7, // *fx(
    kAnimationCommandPtex = 8, // *ptex( (expansion of this abbreviation unresolved)
    kAnimationCommandShowBone = 9, // *showbone(
    kAnimationCommandHideBone = 10, // *hidebone(
    kAnimationCommandPlayIf = 11, // *playif(
    kAnimationCommandOpacity = 12, // *opa(
    kAnimationCommandFace = 13, // *face(
    kAnimationCommandFly = 14, // *fly
    kAnimationCommandHide = 15, // *hide
    kAnimationCommandShow = 16, // *show
    kAnimationCommandLookUp = 17, // *lookup
    kAnimationCommandDontLookUp = 18, // *dontlookup
    kAnimationCommandGive = 19, // *give
};
// Header entries occupy words 0..1. Handler slots are absolute word indices:
// command 0..19 -> words 4..23; unmatched command -> word 24.
enum AnimationHandlerSlot {
    kAnimationHandlerSound = 4,
    kAnimationHandlerShot = 5,
    kAnimationHandlerTail = 6,
    kAnimationHandlerShowClub = 7,
    kAnimationHandlerHideClub = 8,
    kAnimationHandlerSwing = 9,
    kAnimationHandlerLockClub = 10,
    kAnimationHandlerEffect = 11,
    kAnimationHandlerPtex = 12,
    kAnimationHandlerShowBone = 13,
    kAnimationHandlerHideBone = 14,
    kAnimationHandlerPlayIf = 15,
    kAnimationHandlerOpacity = 16,
    kAnimationHandlerFace = 17,
    kAnimationHandlerFly = 18,
    kAnimationHandlerHide = 19,
    kAnimationHandlerShow = 20,
    kAnimationHandlerLookUp = 21,
    kAnimationHandlerDontLookUp = 22,
    kAnimationHandlerGive = 23,
    kAnimationHandlerUnknown = 24
};
static const int kAnimationHandlerCount = 20;
static const int kAttachmentNameBytes = 0x40;
static const int kAttachmentBoneLimit = 4;
static const int kHashPathAlignment = 4;
static const int kFileBufferAlignment = 32;
static const int kFileOpenRead = 0x10000;
static const int kAnimationKeyCommandsOffset = (int)&((AnimationCommandKey*)0)->commands;
#define AsSoundManagerState(p) ((SoundManagerState*)(p))

// Normal destruction remains explicit. This owner handles exceptional exits.
struct EH5Cleanup {
    void* object;
    void (*destroy)(void*, int);
    bool active;
    // Arm exception cleanup for a borrowed object and its matching destructor.
    EH5Cleanup(void* p, void (*d)(void*, int)) : object(p), destroy(d), active(true) {}
    // Disarm cleanup before the explicit normal-path destruction.
    void release() { active = false; }
    // Destroy only when normal-path cleanup has not already taken responsibility.
    ~EH5Cleanup() { if (active) destroy(object, -1); }
};

// Translation unit 0x802A5CC4-0x802A6E98 (19 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".
class SgsDisposable {
public:
    virtual void destroy(int);
};

struct HashFieldsReal { void* data; unsigned short count; char pad[2]; void* begin; void* end; };
struct CHashPath : HashFieldsReal {
    virtual void destroy(int);
    // Initialize an empty path table; file storage is owned by the global buffer.
    CHashPath() {
    data = 0; count = 0; begin = 0; end = 0; }
};

struct SgsBlk {
    char pad0[0xc];
    void* head;
    char pad1[4];
    void* arr;
    int cnt;
};

extern "C" {

extern const float kFloat_6_2831855;
extern float s_actorHelpersTauA;
extern float s_actorHelpersTauB;
extern float s_actorHelpersTauC;
extern const float kFloat_0_soundPlayback;
extern void* g_fx;
extern void* g_hashPathFileBuffer;
extern void* g_hashPath;
extern char s_cLocaleData[];
extern char __vt__9TiXmlNode[];
extern char* s_animationEventHandlerNames[];
extern char __vt__9CHashPath[];
extern const float kAnimationEventFrameScale;
extern const float kFloat_0_00001_AnimationEventFrame;
extern const char kSprayScriptExtension[];
double ceil(double);
double floor(double);
unsigned int strlen(const char*);

void* CFx_CreateAttachedEffect(void*, void*, void*, int, int, int);
int CString_Compare(const char*, const char*);
void __dl__FPv(void*);
void __dla__FPv(void*);
void* __nw__FUl(unsigned long);
void appFreeUni__FPv(void*);
void FloatTween_Init(void*, float);
void SoundManager_ResetPlaybackState(void*);
void SoundPlaybackState_Ctor(void*);

void* String_ParseQuotedToken(void*, void*);
void strncpy(void*, void*, int);
char* strchr(const char*, int);
char* strrchr(const char*, int);
int String_CompareCString(const char*, const char*);
void* CFxSpray_CreateFromScript(void*, const char*, int, int);
void* CFxBox_RegisterSpray(void*, void*);
void* CFxSequence_Create(void*, const char*, int, int, int);
void CFxBox_RegisterSequence(void*, void*);
void* memcpy(void*, const void*, unsigned int);
char* strcpy(char*, const char*);
void gtl_CFile_Ctor(void*);
int gtl_CFile_Open(void*, const char*, unsigned int);
unsigned int gtl_CFile_GetFileSize(void*);
void gtl_CFile_Read(void*, void*, unsigned int);
void gtl_CFile_Close(void*);
void gtl_CFile_dtor(void*, int);
void* appMallocUni__FUl(unsigned int);

int WPuppet_FindKeyRange(void*, int, int, int*, int*);
#pragma push
#pragma use_lmw_stmw on
// Find one of twenty command names and dispatch its virtual handler, or call the fallback handler.
void AnimationEvent_DispatchNamedHandler(void* self, void* commandName, void* commandCursor, void* handlerContext) {
    int handlerIndex;
    for (handlerIndex = 0; handlerIndex < kAnimationHandlerCount; handlerIndex++) {
        if (CString_Compare((const char*)commandName, ((char**)s_animationEventHandlerNames)[handlerIndex]) == 0) break;
    }
    switch (handlerIndex) {
    case kAnimationCommandSound: ((void (**)(void*, void*, void*))*(void**)self)[kAnimationHandlerSound](self, commandCursor, handlerContext); break;
    case kAnimationCommandShot: ((void (**)(void*, void*, void*))*(void**)self)[kAnimationHandlerShot](self, commandCursor, handlerContext); break;
    case kAnimationCommandTail: ((void (**)(void*, void*, void*))*(void**)self)[kAnimationHandlerTail](self, commandCursor, handlerContext); break;
    case kAnimationCommandShowClub: ((void (**)(void*, void*, void*))*(void**)self)[kAnimationHandlerShowClub](self, commandCursor, handlerContext); break;
    case kAnimationCommandHideClub: ((void (**)(void*, void*, void*))*(void**)self)[kAnimationHandlerHideClub](self, commandCursor, handlerContext); break;
    case kAnimationCommandSwing: ((void (**)(void*, void*, void*))*(void**)self)[kAnimationHandlerSwing](self, commandCursor, handlerContext); break;
    case kAnimationCommandLockClub: ((void (**)(void*, void*, void*))*(void**)self)[kAnimationHandlerLockClub](self, commandCursor, handlerContext); break;
    case kAnimationCommandEffect: ((void (**)(void*, void*, void*))*(void**)self)[kAnimationHandlerEffect](self, commandCursor, handlerContext); break;
    case kAnimationCommandPtex: ((void (**)(void*, void*, void*))*(void**)self)[kAnimationHandlerPtex](self, commandCursor, handlerContext); break;
    case kAnimationCommandShowBone: ((void (**)(void*, void*, void*))*(void**)self)[kAnimationHandlerShowBone](self, commandCursor, handlerContext); break;
    case kAnimationCommandHideBone: ((void (**)(void*, void*, void*))*(void**)self)[kAnimationHandlerHideBone](self, commandCursor, handlerContext); break;
    case kAnimationCommandPlayIf: ((void (**)(void*, void*, void*))*(void**)self)[kAnimationHandlerPlayIf](self, commandCursor, handlerContext); break;
    case kAnimationCommandOpacity: ((void (**)(void*, void*, void*))*(void**)self)[kAnimationHandlerOpacity](self, commandCursor, handlerContext); break;
    case kAnimationCommandFace: ((void (**)(void*, void*, void*))*(void**)self)[kAnimationHandlerFace](self, commandCursor, handlerContext); break;
    case kAnimationCommandFly: ((void (**)(void*, void*, void*))*(void**)self)[kAnimationHandlerFly](self, commandCursor, handlerContext); break;
    case kAnimationCommandHide: ((void (**)(void*, void*, void*))*(void**)self)[kAnimationHandlerHide](self, commandCursor, handlerContext); break;
    case kAnimationCommandShow: ((void (**)(void*, void*, void*))*(void**)self)[kAnimationHandlerShow](self, commandCursor, handlerContext); break;
    case kAnimationCommandLookUp: ((void (**)(void*, void*, void*))*(void**)self)[kAnimationHandlerLookUp](self, commandCursor, handlerContext); break;
    case kAnimationCommandDontLookUp: ((void (**)(void*, void*, void*))*(void**)self)[kAnimationHandlerDontLookUp](self, commandCursor, handlerContext); break;
    case kAnimationCommandGive: ((void (**)(void*, void*, void*))*(void**)self)[kAnimationHandlerGive](self, commandCursor, handlerContext); break;
    default: ((void (**)(void*, void*, void*))*(void**)self)[kAnimationHandlerUnknown](self, commandCursor, handlerContext); break;
    }
}
#pragma pop
// output must accommodate the scanned token plus NUL; no capacity is supplied.
// On success cursor advances beyond the copied token (including '(' when found).
// On failure cursor is null if '*' was absent, otherwise it stays at the found '*'.
int Parse_AsteriskToken(void*, char*, void**);
void* InstanceTable_Get();
int ResolvedItemSelection_ContainsPart(void*, void*);
char* String_ParseDecimalInt(char*);
void StdString_ReserveCapacity(void*, unsigned int);
void StdString_ReplaceRange(void*, int, int, char*, char*, char*);
int CString_CompareIgnoringCase(const char*, const char*); // Compare strings without case using the active locale table.
int CString_Compare(const char*, const char*);
void* WPuppet_FindOrCacheBone(void*, const char*);
void* WPuppet_FindNamedBoneEntry(void*, const char*);
void* FxAttachment_Init(void*);

// Default event hook; performs no work.
void CDisposable_vfunc_27() {}

// Default creature capability hook; returns true.
int CCreature_vfunc_26() { return 1; }

// Initialize the first animation/parser two-pi cache.
void ActorHelpers_InitializeTauCacheA() { s_actorHelpersTauA = kFloat_6_2831855; }

// Initialize the second animation/parser two-pi cache.
void ActorHelpers_InitializeTauCacheB() { s_actorHelpersTauB = kFloat_6_2831855; }

// Initialize the third animation/parser two-pi cache.
void ActorHelpers_InitializeTauCacheC() { s_actorHelpersTauC = kFloat_6_2831855; }

// Create a globally managed attached effect from the actor's two supplied attachment arguments.
void* Actor_CreateAttachedEffect(void* /*unused*/, void* a, void* b) {
    return CFx_CreateAttachedEffect(g_fx, a, b, 1, 0, 0);
}

// Free the hash-path wrapper when called as a deleting destructor.
void* CHashPath_dtor(void* p, int n) {
    if (p != 0 && n > 0) {
        __dl__FPv(p);
    }
    return p;
}

struct Z8B { char pad[16]; };
class Z8 : public Z8B { public: 
virtual void v08(int);
};
// Destroy the path lookup object and release its loaded file buffer.
void HashPath_Shutdown() {
    if (g_hashPath != 0) {
        if (g_hashPath != 0) {
((Z8*)g_hashPath)->v08(1);
        }
        g_hashPath = 0;
    }
    if (g_hashPathFileBuffer != 0) {
        appFreeUni__FPv(g_hashPathFileBuffer);
        g_hashPathFileBuffer = 0;
    }
}

// Initialize both volume tweens, allocate playback state, and reset manager settings.
void* SoundManager_Ctor(void* self) {
    FloatTween_Init(self, kFloat_0_soundPlayback);
    FloatTween_Init(&((SoundManagerTweens*)self)->musicVolume, kFloat_0_soundPlayback);
    void* p = __nw__FUl(0x208);
    if (p != 0) {
        {
            try {
                SoundPlaybackState_Ctor(p);
            } catch (...) {
                __dl__FPv(p);
                throw;
            }
        }
    }
    AsSoundManagerState(self)->state = p;
    SoundManager_ResetPlaybackState(self);
    return self;
}

// Parse from the cursor in *out INTO s, then accept a nonempty result shorter than 64 bytes.
// Despite its const type, s is destination storage passed to the legacy void* parser.
// *out is updated even on failure. Caller must provide at least 64 writable bytes;
// the length check occurs after parsing. Repeated strlen calls preserve existing code.
int Parse_NonemptyQuotedTokenUnder64Bytes(void* /*unused*/, void** out, const char* s) {
    *out = String_ParseQuotedToken(*out, (void*)s);
    unsigned int len = strlen(s);
    // Original 0x802A6D90: the successful result follows the failure result.
    do {
        if (len != 0) {
            unsigned int len2 = strlen(s);
            if (len2 < kAttachmentNameBytes) break;
        }
        return 0;
    } while (0);
    return 1;
}

// Create a spray or sequence by extension, copy its attachment data, and register it with the effect box.
void FxBox_CreateAndAttachEffectByExtension(void* /*unused*/, void* data, const char* name, void* ctx) {
    char* dot = strrchr(name, '.');
    if (dot == 0) return;
    if (strlen(dot) < 4) return;
    if (String_CompareCString(dot + 1, kSprayScriptExtension) == 0) {
        void* p = CFxSpray_CreateFromScript(ctx, name, 0, 0);
        if (p == 0) return;
        ((SprayAttachmentFields*)p)->attachment = data;
        ((SprayAttachmentFields*)p)->positionX = ((EffectAttachmentSlots*)data)->positionX;
        ((SprayAttachmentFields*)p)->positionY = ((EffectAttachmentSlots*)data)->positionY;
        memcpy(((SprayAttachmentFields*)p)->transforms, ((EffectAttachmentSlots*)data)->transforms, sizeof(((EffectAttachmentSlots*)data)->transforms));
        memcpy(((SprayAttachmentFields*)p)->secondaryTransforms, ((EffectAttachmentSlots*)data)->secondaryTransforms, sizeof(((EffectAttachmentSlots*)data)->secondaryTransforms));
        CFxBox_RegisterSpray(ctx, p);
    } else {
        void* p = CFxSequence_Create(ctx, name, 1, 0, 0);
        if (p == 0) return;
        *(void**)&((CFxSequence*)p)->attachment = data;
        ((CFxSequence*)p)->attachmentPositionX = ((EffectAttachmentSlots*)data)->positionX;
        ((CFxSequence*)p)->attachmentPositionY = ((EffectAttachmentSlots*)data)->positionY;
        CFxBox_RegisterSequence(ctx, p);
    }
}

#pragma push
#pragma use_lmw_stmw on
// Read optional ampersand conditions and return the following token only when a condition matches.
// Grammar: zero or more quoted "&<decimal>" conditions followed by a quoted value.
// Conditions are ORed: arg's selection at +0x1BC is tried first, then the selected
// actor's table entry at (*table)+0x70+actorIndex*4. The decimal parser's char* result
// is an integer token encoded in pointer bits: compare pointer VALUES, never text.
// A selection match skips table lookup for that token. Once found, later conditions
// are consumed without further lookups. With no conditions the value is accepted.
// path advances even on rejection. output needs at least 64 bytes; 0 may leave it
// untouched OR copied with an invalid result; 1 means a nonempty <64-byte value.
// Existing checks follow unbounded copies; malformed resources are not made safe here.
int ParseConditionedQuotedToken(void* /*unused*/, char** path, char* output, char* arg, void* table) {
    char scratch[kAttachmentNameBytes];
    *path = (char*)String_ParseQuotedToken(*path, scratch);
    int len = strlen(scratch);
    if (len == 0 || len >= kAttachmentNameBytes) return 0;
    int hasConditions = 0;
    int found = 0;
    while (scratch[0] == '&') {
        hasConditions = 1;
        char* token = String_ParseDecimalInt(scratch + 1);
        if (!found) {
            if (arg != 0 && ResolvedItemSelection_ContainsPart(arg + 0x1bc, token)) {
                found = 1;
            } else if (table != 0 && *(void**)table != 0) {
                char* instanceTable = (char*)InstanceTable_Get();
                void* selectedActor = *(void**)(instanceTable + 4);
                unsigned char actorIndex = ((ActorInstanceSelection*)selectedActor)->index;
                void* actorTokenEntry = (char*)*(void**)table + (actorIndex << 2);
                if (token == ((ConditionTokenTable*)actorTokenEntry)->tokens[0]) found = 1;
            }
        }
        *path = (char*)String_ParseQuotedToken(*path, scratch);
        len = strlen(scratch);
        if (len == 0 || len >= kAttachmentNameBytes) break;
    }
    if (hasConditions && !found) return 0;
    strcpy(output, scratch);
    if (strlen(output) == 0 || strlen(output) >= kAttachmentNameBytes) return 0;
    return 1;
}
#pragma pop

// Destroy XML child nodes, release the value string, and optionally free the comment object.
void* TiXmlComment_dtor(void* self, int n) {
    if (self != 0) {
        if (self != 0) {
            *(void**)self = (void*)__vt__9TiXmlNode;
            void* r31 = ((SgsBlk*)self)->head;
            // Original TiXmlComment_dtor: destroy the child chain.
            while (r31 != 0) {
                void* cur = r31;
                r31 = ((TiXmlNodeLayout*)r31)->nextSibling;
                if (cur != 0) {
                    ((SgsDisposable*)cur)->destroy(1);
                }
            }

            SgsBlk* b = (SgsBlk*)self;
            if (&((TiXmlNodeLayout*)self)->value) {
                if (b->arr != 0) {
                    __dla__FPv(b->arr);
                }
                b->arr = 0;
                b->cnt = 0;
            }
        }
        if (n > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}


#pragma push
#pragma use_lmw_stmw on
#pragma opt_lifetimes off
// Original CHashPath_FindPath: search padded strings and preserve the shared result.
// Search the aligned strings in one hash bucket for a case-insensitive path suffix.
inline void FindHashedPath(void*& result, char*& item, unsigned int& count, unsigned int& i, const char* path) {
    while ((unsigned short)i < (unsigned short)count) {
        unsigned int itemlen = strlen(item);
        unsigned int pathlen = strlen(path);
        int diff = (int)itemlen - (int)pathlen;
        int ok;
        if (diff < 0 || (CString_CompareIgnoringCase(item + diff, path) != 0)) {
            ok = 0;
        } else {
            ok = 1;
        }
        if (ok != 0) { result = item; return; }
        item += (strlen(item) + 4) & ~3u;
        ++i;
    }
    result = 0;
}
// Lowercase and hash the basename, then search the selected bucket for the requested path suffix.
// Return a borrowed pointer into g_hashPathFileBuffer, or null; loading/shutdown
// invalidates it. path is a NUL-terminated path; basename selects the hash bucket.
void* CHashPath_FindPath(const char* path) {
    struct Table { char* strings; unsigned short count; char pad[2]; unsigned short* hashes; unsigned int* offsets; };
    unsigned int i;
    register Table* table = (Table*)g_hashPath;
    if (table == 0) return 0;
    register void* result;
    register char* part;
    // Original CHashPath_FindPath: share the empty-result return.
    do {
        if (table->strings == 0 || table->count == 0 || table->hashes == 0 || table->offsets == 0) { result = 0; break; }
        // Phase 1: isolate the basename and create a lowercase temporary string.
        char* slash = strrchr(path, '/');
        part = slash != 0 ? slash + 1 : (char*)path;
        struct Str { unsigned int flag; unsigned int size; unsigned int capacity; } __attribute__((aligned(8))) parsed;
        parsed.flag = 0;
        parsed.size = 0;
        parsed.capacity = 0;
        unsigned int len = strlen(part);
        StdString_ReserveCapacity(&parsed, len);
        char terminator = ((char*)&parsed)[-4];
        StdString_ReplaceRange(&parsed, 0, 0, part, part + len, &terminator);
        unsigned int heap = parsed.flag >> 31;
        unsigned int nbytes;
        char* begin;
        if (heap == 0) { begin = ((char*)&parsed) + 1; nbytes = ((unsigned char*)&parsed)[0] & 0x7f; }
        else { begin = (char*)parsed.capacity; nbytes = parsed.size; }
        int sso = heap == 0;
        char* end = begin + nbytes;
        char* src;
        char* out = sso ? ((char*)&parsed) + 1 : (char*)parsed.capacity;
        src = sso ? ((char*)&parsed) + 1 : (char*)parsed.capacity;
        for (; src != end;) {
            int oor = 0;
            int c = (signed char)*src;
            if (c < 0 || c >= 0x100) oor = 1;
            int v;
            if (oor) v = c;
            else v = ((CLocaleFields*)s_cLocaleData)->ctype->lowercase[c];
            *out = v;
            ++src;
            ++out;
        }
        // Phase 2: hash the lowercase basename; the original path remains the suffix query.
        unsigned int hashHeap;
        char* hp;
        char hc;
        hashHeap = parsed.flag >> 31;
        hp = hashHeap == 0 ? ((char*)&parsed) + 1 : (char*)parsed.capacity;
        unsigned int hash = 0;
        for (; (hc = *hp) != 0; ++hp) {
            hash = (hash << 4) + (signed char)hc;
            unsigned int high = hash & 0xF0000000;
            if (high != 0) {
                hash ^= high >> 24;
                hash ^= high;
            }
        }
        if (hashHeap) __dl__FPv((void*)parsed.capacity);
        // Phase 3: resolve the bucket's 16-bit count/offset view and search aligned strings.
        unsigned int rem = hash % table->count;
        unsigned int idx = table->hashes[rem & 0xFFFF];
        char* base = table->strings + table->offsets[idx] * 2;
        char* item = base + 2;
        unsigned int count = *(unsigned short*)base;
        i = 0;
        FindHashedPath(result, item, count, i, path);
    } while (0);
    return result;
}
#pragma pop

// Copy an asterisk-prefixed command (first param is unused) through its space, parenthesis or terminator boundary.
int Parse_AsteriskToken(void* /*unused*/, char* output, void** cursor) {
    char* found = (char*)strchr((const char*)*cursor, '*');
    *cursor = found;
    if (found == 0) {
        return 0;
    }
    char* next = found;    // character being tested for the delimiter
    char* current = found; // advances in lockstep with next; its NUL test (before the step) ends the scan
    int count = 0;
    // Original 0x802A653C: scan until a space, parenthesis or terminator.
    while (*current != 0) {
        next = next + 1;
        {
            signed char c = (signed char)*next;
            current = current + 1;
            count = count + 1;
            if (c == ' ') break;
            if (c == '(') {
                count = count + 1;
                break;
            }
        }
    }
    // The delimiter scan includes an opening parenthesis in the copied token.
    if (count <= 1) {
        return 0;
    }
    strncpy(output, found, count);
    output[count] = 0;
    *cursor = (char*)*cursor + count;
    return 1;
}

class RefParser {
public:
    virtual void v08(); virtual void v0c(); virtual void v10(); virtual void v14();
    virtual void v18(); virtual void v1c(); virtual void v20(); virtual void v24();
    virtual void v28(); virtual void v2c(); virtual void v30(); virtual void v34();
    virtual void v38(); virtual void v3c(); virtual void v40(); virtual void v44();
    virtual void v48(); virtual void v4c(); virtual void v50(); virtual void v54();
    virtual void v58(); virtual void v5c(); virtual void v60();
    virtual int check(void**, void*, char*);
    virtual void prepare(void*);
    virtual void finish(void*, char*);
};
#pragma push
#pragma use_lmw_stmw on
// Resolve up to four attachment bones, using optional name conditions and fallback model lookups.
void FxAttachment_ParseBoneReferences(void* self, void** path, void* arg, void* entries, void* results, void* mode, void* lookup) {
    char name[kAttachmentNameBytes];
    name[0] = 0;
    *path = String_ParseQuotedToken(*path, name);
    unsigned int len = strlen(name);
    if (len == 0 || strlen(name) >= kAttachmentNameBytes) return;

    // Phase 1: optional @name filter, then resolve the conditioned attachment suffix.
    char suffix[kAttachmentNameBytes];
    suffix[0] = 0;
    if (name[0] == '@') {
        if (results == 0 || CString_Compare(name + 1, ((AttachmentNameFilter*)results)->name) != 0) return;
        *path = String_ParseQuotedToken(*path, suffix);
        len = strlen(suffix);
        if (len == 0 || strlen(suffix) >= kAttachmentNameBytes) return;
    } else {
        strcpy(suffix, name);
    }

    char query[kAttachmentNameBytes];
    if (!((RefParser*)self)->check(path, arg, query)) return;
    char context[0x28];
    FxAttachment_Init(context);
    void* slots = context;
    // Phase 2: fill up to four borrowed transform slots; fallback lookup is per actor.
    int count = 0;
    do {
        if (mode == 0 || count != 0) {
            *path = String_ParseQuotedToken(*path, query);
            len = strlen(query);
            if (len == 0 || strlen(query) >= kAttachmentNameBytes) break;
        }
        int found = 0;
        void* item = WPuppet_FindOrCacheBone(entries, query);
        if (item != 0) {
            ((EffectAttachmentSlots*)slots)->transforms[0] = ((BoneTransformView*)item)->worldTransform;
            found = 1;
            if (count == 0) ((RefParser*)self)->prepare(item);
        } else {
            item = WPuppet_FindNamedBoneEntry(entries, query);
            if (item != 0) {
                found = 1;
                void* first = *(void**)item;
                if (first != 0) ((EffectAttachmentSlots*)slots)->transforms[0] = ((BoneTransformView*)first)->worldTransform;
                ((EffectAttachmentSlots*)slots)->secondaryTransforms[0] = ((NamedBonePair*)item)->secondary->transform;
            }
        }
        if (!found && lookup != 0) {
            void* p = InstanceTable_Get();
            void* p2 = (void*)((ActorInstanceTable*)p)->selected;
            unsigned char index = ((ActorInstanceSelection*)p2)->index;
            void* list = *(void**)((char*)lookup + index * 4 + 4);
            if (list != 0) {
                p = InstanceTable_Get();
                p2 = (void*)((ActorInstanceTable*)p)->selected;
                index = ((ActorInstanceSelection*)p2)->index;
                list = *(void**)((char*)lookup + index * 4 + 4);
                void* v = WPuppet_FindOrCacheBone(list, query);
                if (v != 0) ((EffectAttachmentSlots*)slots)->transforms[0] = ((BoneTransformView*)v)->worldTransform;
                else {
                    p = InstanceTable_Get();
                    p2 = (void*)((ActorInstanceTable*)p)->selected;
                    index = ((ActorInstanceSelection*)p2)->index;
                    list = *(void**)((char*)lookup + index * 4 + 4);
                    v = WPuppet_FindNamedBoneEntry(list, query);
                    if (v != 0) {
                        void* first = *(void**)v;
                        if (first != 0) ((EffectAttachmentSlots*)slots)->transforms[0] = ((BoneTransformView*)first)->worldTransform;
                        ((EffectAttachmentSlots*)slots)->secondaryTransforms[0] = ((NamedBonePair*)v)->secondary->transform;
                    }
                }
            }
        }
        ++count;
        slots = (char*)slots + 4;
    } while ((unsigned int)count < kAttachmentBoneLimit);
    // Phase 3: hand the temporary slot views to the virtual attachment creator.
    if (count > 0) ((RefParser*)self)->finish(context, suffix);
}
#pragma pop

// Replace the current hash-path table with a loaded, 32-byte-aligned file buffer.
void Load_HashPathTable(void* self) {
    // Phase 1: discard the old wrapper before releasing its backing strings.
    if (g_hashPath != 0) {
        if (g_hashPath != 0) {
            ((CHashPath*)g_hashPath)->destroy(1);
        }
        g_hashPath = 0;
    }
    if (g_hashPathFileBuffer != 0) {
        appFreeUni__FPv(g_hashPathFileBuffer);
        g_hashPathFileBuffer = 0;
    }
    // Phase 2: read the aligned file allocation, then install views into that allocation.
    char local[0x20];
    gtl_CFile_Ctor(local);
    EH5Cleanup eh5Cleanup22(local, (void (*)(void*, int))gtl_CFile_dtor);
    if (!gtl_CFile_Open(local, (const char*)self, kFileOpenRead)) {
        { eh5Cleanup22.release(); gtl_CFile_dtor(local, -1); }
        return;
    }
    unsigned int size = gtl_CFile_GetFileSize(local);
    size = (size + kFileBufferAlignment - 1) & ~(unsigned int)(kFileBufferAlignment - 1);
    g_hashPathFileBuffer = appMallocUni__FUl(size);
    gtl_CFile_Read(local, g_hashPathFileBuffer, size);
    gtl_CFile_Close(local);
    struct Entry { void* data; unsigned short count; char pad[2]; void* begin; void* end; void* vtable; };
    Entry* e = (Entry*)new CHashPath;
    void* data = g_hashPathFileBuffer;
    g_hashPath = e;
    e->data = data;
    unsigned short cnt = *(unsigned short*)data;
    e->count = cnt;
    char* begin = (char*)data + 2;
    e->begin = begin;
    e->end = begin + cnt * 2;
    { eh5Cleanup22.release(); gtl_CFile_dtor(local, -1); }
}

#pragma push
#pragma use_lmw_stmw on
#pragma opt_lifetimes off
// Convert a time interval to animation frames and dispatch commands from matching key records.
// start/end and bounds are animation times in seconds; keys run at 30 frames/sec.
// Normal ranges exclude the first bound frame and apply epsilon before ceil/floor;
// singleFrame instead processes only the rounded start frame. Handler context is borrowed.
// Commands mutate a local cursor; the parser's +0x0C hook finishes each key record.
void AnimationEventParser_ProcessFrameRange(void* self, void* manager, float startTime, float endTime, void* bounds, void* handlerContext, void* singleFrame) {
    void* parser = self;
    void* puppet = manager;
    void* context = handlerContext;
    void* isSingleFrame = singleFrame;
    float endFrame;
    double commandBuffer[0x80];
    float firstFrame, lastFrame, startFrame;
    int frameIndex, frameBegin, frameEnd, first, last, i, offset;
    void* commandCursor;
    endFrame = endTime;
    if (bounds == 0) return;
    if ((!(startTime <= ((AnimationTimeRange*)bounds)->last) || startTime == endFrame) && isSingleFrame == 0) return;
    // Phase 1: clamp time to the animation bounds and calculate the inclusive key range.
    firstFrame = kAnimationEventFrameScale * ((AnimationTimeRange*)bounds)->first;
    lastFrame = kAnimationEventFrameScale * ((AnimationTimeRange*)bounds)->last;
    startFrame = kAnimationEventFrameScale * (startTime - ((AnimationTimeRange*)bounds)->first) + firstFrame;
    endFrame = ((AnimationTimeRange*)bounds)->last > endTime ? endTime : ((AnimationTimeRange*)bounds)->last;
    endFrame = kAnimationEventFrameScale * (endFrame - ((AnimationTimeRange*)bounds)->first) + firstFrame;
    frameIndex = (int)(float)ceil((double)(startFrame - kFloat_0_00001_AnimationEventFrame));
    frameBegin = frameIndex;
    if (isSingleFrame == 0) {
        frameEnd = (int)(float)floor((double)(endFrame - kFloat_0_00001_AnimationEventFrame));
        if ((float)frameBegin == firstFrame) frameBegin++;
        if ((float)frameEnd > lastFrame) frameEnd = (int)lastFrame;
        frameIndex = frameEnd;
    }
    if (frameBegin > frameIndex) return;
    if (puppet == 0) return;
    if (!WPuppet_FindKeyRange(puppet, frameBegin, frameIndex, &first, &last)) return;
    // Phase 2: parse each key's command stream and dispatch through the name/slot map.
    for (i = first, offset = first * 12; i <= last; ++i, offset += 12) {
        commandCursor = *(void**)((char*)((WPuppet*)puppet)->keyRecords + offset + kAnimationKeyCommandsOffset);
        if (commandCursor != 0) {
            while (commandCursor != 0) {
                if (!Parse_AsteriskToken(parser, (char*)commandBuffer, &commandCursor)) break;
                AnimationEvent_DispatchNamedHandler(parser, commandBuffer, &commandCursor, context);
            }
            (*(void (**)(void*, void*))((char*)*(void**)parser + 0xc))(parser, &commandCursor);
        }
    }
}
#pragma pop

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804624BC", _ctors_u802A5CC4_0, ActorHelpers_InitializeTauCacheA);
STATIC_INIT_ENTRY(".CRT$XCU804624BC", _ctors_u802A5CC4_1, ActorHelpers_InitializeTauCacheB);
STATIC_INIT_ENTRY(".CRT$XCU804624BC", _ctors_u802A5CC4_2, ActorHelpers_InitializeTauCacheC);
#pragma pop
