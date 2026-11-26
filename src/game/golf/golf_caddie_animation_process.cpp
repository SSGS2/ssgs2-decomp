// Golf caddie animation: reading the run animation's metadata and prerequisites from its key text,
// the tree of animation names, and the per-frame update of the caddie's pet body.
//
// The caddie actor is CGolfCaddie; its pet body is CPetBody.

#include "classes/CGolfDoc.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
#include "game/types.h"
#include "game/wvector.h"
#include "sdk_protos.h"
#include "classes/CGolfCaddie.h"
#include "classes/WPuppet.h"
#include "classes/GolfCaddieEntryView.h"

#include "game/golfball.h"
#include "classes/CPetBody.h"
#include "classes/CFx.h"
#include "classes/CClubRingEffect.h"

// Called through its original pointer-shaped prototype to keep the argument order.
extern "C" void PlayAnimationByName__8CPetBodyFfPvii(void*, void*, float, int, int);
#define AsCGolfCaddie(p) ((CGolfCaddie*)(p))

// Byte positions in the external packed string pool; preserve its original storage and encoding.
enum {
    kGolfCaddie_runAnimation = 0x84,         // "駆け足"
    kGolfCaddie_ringToken = 0x8B,            // "*ring"
    kGolfCaddie_stringFormat = 0x91,         // "%s"
    kGolfCaddie_stopDistanceToken = 0x94,    // "*stop_dist"
    kGolfCaddie_floatFormat = 0x9F,          // "%f"
    kGolfCaddie_startDelayToken = 0xA2,      // "*start_delay"
    kGolfCaddie_petSpeedToken = 0xAF,        // "*pet_speed"
    kGolfCaddie_hideCaddieToken = 0xBA,      // "*no_caddie_view"
    kGolfCaddie_s_mot = 0x7b,                // "%s.mot"
    kGolfCaddie_keyDelimiter = 0x82,         // ":"
    kGolfCaddie_tutorialRunAnimation = 0xcb, // "チュートリアル駆け足"
    kGolfCaddie_waitAnimation = 0xe0,        // "待ち"
    kGolfCaddie_idleAnimation = 0xe5,        // "基本"
    kGolfCaddie_hurryAnimation = 0xea,       // "催促する"
};

enum CaddieProcessConstants {
    kGolfCaddie_emptyString = 0xCA, // NUL terminator of the preceding metadata token.
    kCaddieProcessSuppressed = 0x00800000,
    kCaddieProcessConte = 0x01000000,
    kCaddieMovementClockRunning = 0x80,
    kCaddieIdle = 0,
    kCaddieWaiting = 1,
    kCaddieAnimationState2 = 2,
    kCaddieHurrying = 3,
    kCaddieRunning = 4,
    kTenSecondsMs = 10000,
    kTwentySecondsMs = 20000,
    kFortySecondsMs = 40000,
    kSixtySecondsMs = 60000
};
// State 2 plays wait during the 60-second clock's final 20 seconds; state 1
// plays wait earlier. State 3 plays hurry; state 4 updates movement.

extern "C" {

void* WPuppet_FindAnimationRangeByName(void*, const char*);
void WPuppet_FindKeyRange(void*, int, int, int*, int*);
int String_ScanTokensAfterSubstring(char*, const char*, const char*, ...);
void __pl__FRC7WVectorRC7WVector(void*, void*, float*);
void String_ReserveCapacity(void*, unsigned int);
void String_ReplaceRange(void*, unsigned int, unsigned int, const void*, const void*, const void*);
void appFreeUni__FPv(void*);
void CaddieDialogueStringMap_DestroySubtree(void*, void*);
void CaddieDialogueStringPair_Destroy(void*, void*);
void* CaddieDialogueTextMap_FindInsertionPosition(void*, void*, void**, unsigned char*, unsigned char*);
void* StringWStringMap_InsertNode(void*, void*, unsigned char, unsigned char, void*);
void WString_ReserveCapacity(void*, unsigned int);
void WString_ReplaceRange(void*, unsigned int, unsigned int, const void*, const void*, const void*);
void* Resource_OpenResolvedPath(void*, const char*, int, int, int, int);
void* __nwa__FUl(unsigned int);
void Resource_Release(void*);
unsigned char String_ParseDecimalInt(const char*);
void __dla__FPv(void*) throw();
signed char* String_ParseKeyBeforeColon(void*, signed char*, signed char*);
void* CQuadTree_GetInstance(void*);
void GetGroundPoint__9CQuadTreeF7WVectorbPib(void*, void*, void*, int, int, int);
void __as__7WVectorFRC7WVector(void*, void*);
void* GolfDoc_GetCurrent();
int GolfDoc_GetPlayerCount();
void CaddieObject_ApplyEnabledState(void*);
int Float_IsAbsBelow(float*, float);
char* Vec3_Set(void*, float, float, float);
float Vector_SignedAngleXZ(void*, void*);
void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
void* GolfBall__Fv();
int SharedDoc_GetTurnTimeLimitMs();
int strcmp(const char*, const char*);
void RingEffect_UpdateRotation(void*, float);
extern const float kFloat_0_00001_Caddie, kFloat_neg0_00001_GolfCaddie, kFloat_1_GolfCaddie, kFloat_20_GolfCaddie;
extern const float kPetYawDegreesToRadians;
extern const char golfCaddieStrings[];
extern void* g_resourceManager;
extern const float kFloat_30_GolfCaddie, kFloat_0_5_GolfCaddie, kFloat_0_GolfCaddie, kFloat_0_3_GolfCaddie,
    kFloat_3_2_GolfCaddie;

// Per-caddie model metadata parsed from the run animation's key text.
struct CaddieAnimationMetadata {
    void* pet;
    char paddingToDistances[0x34];
    float stopDistance, startDelay, petSpeed;
    unsigned char ringEnabled;
    bool caddieViewEnabled;
};
struct PuppetKeyText {
    unsigned int frame, key;
    char* text;
};
struct RingVisibility {
    char padding[8];
    unsigned char visible;
};
struct CaddieOperationType {
    char padding[0xC];
    unsigned char type;
};
struct CaddieOperationOwner {
    char padding[0x14];
    CaddieOperationType* operation;
};

typedef Vec3f Vec3;

// Read ring visibility, stopping distance, start delay and speed from the run animation key text.
int CGolfCaddie::ParseAnimationMetadata(void* caddieEntry) {
    void* puppet = ((CPetBody*)(*(void**)caddieEntry))->GetPuppet();
    // 0x8012A730: no animation metadata is a normal failure.
    if (puppet) {
        {
            void* runRange = WPuppet_FindAnimationRangeByName(puppet, golfCaddieStrings + kGolfCaddie_runAnimation);
            if (runRange) {
                float scaledFrame = kFloat_30_GolfCaddie * *(float*)runRange + kFloat_0_5_GolfCaddie;
                int keyIndex = -1, lastKeyIndex;
                int keyFrame = (int)scaledFrame;
                WPuppet_FindKeyRange(puppet, keyFrame, keyFrame, &keyIndex, &lastKeyIndex);
                if (keyIndex > -1) {
                    char ringResourceName[16];
                    char* text = ((PuppetKeyText*)((WPuppet*)puppet)->keyRecords)[keyIndex].text; // original +0xC0
                    ringResourceName[0] = 0;
                    unsigned char found =
                        String_ScanTokensAfterSubstring((char*)&text, golfCaddieStrings + kGolfCaddie_ringToken,
                                                        golfCaddieStrings + kGolfCaddie_stringFormat, ringResourceName);
                    CaddieAnimationMetadata* out = (CaddieAnimationMetadata*)caddieEntry;
                    out->ringEnabled = (unsigned char)found;
                    if (found) {
                        void** object = &texture3D; // original +0x84
                        if (!*object)
                            *object = new CClubRingEffect(ringResourceName);
                        Vec3 v;
                        float p[3];
                        p[0] = kFloat_0_GolfCaddie;
                        p[1] = kFloat_0_3_GolfCaddie;
                        p[2] = kFloat_0_GolfCaddie;
                        __pl__FRC7WVectorRC7WVector(&v, position, p); // original +0x38
                        ((CClubRingEffect*)(*object))->SetTranslation(&v);
                        ((RingVisibility*)*object)->visible = 1;
                    }
                    String_ScanTokensAfterSubstring((char*)&text, golfCaddieStrings + kGolfCaddie_stopDistanceToken,
                                                    golfCaddieStrings + kGolfCaddie_floatFormat, &out->stopDistance);
                    out->stopDistance = kFloat_3_2_GolfCaddie * out->stopDistance;
                    String_ScanTokensAfterSubstring((char*)&text, golfCaddieStrings + kGolfCaddie_startDelayToken,
                                                    golfCaddieStrings + kGolfCaddie_floatFormat, &out->startDelay);
                    String_ScanTokensAfterSubstring((char*)&text, golfCaddieStrings + kGolfCaddie_petSpeedToken,
                                                    golfCaddieStrings + kGolfCaddie_floatFormat, &out->petSpeed);
                    int hasHideCaddieToken =
                        String_ScanTokensAfterSubstring((char*)&text, golfCaddieStrings + kGolfCaddie_hideCaddieToken,
                                                        golfCaddieStrings + kGolfCaddie_emptyString);
                    out->caddieViewEnabled = hasHideCaddieToken == 0;
                    out->petSpeed = kFloat_3_2_GolfCaddie * out->petSpeed;
                    return 1;
                }
            }
        }
    }
    return 0;
}

// Set the caddie ring effect's visibility byte.
void CaddieRing_SetVisible(void* self, unsigned char value) {
    ((RingVisibility*)self)->visible = value;
}

// Return the golf-document state word used to gate caddie updates.
char* CGolfDoc::GetGameStateFlagsPtr() {
    char* self = (char*)this;
    return (char*)&AsCGolfDoc(self)->gameStateFlags; // original +0x358: flag word read by CGolfCaddie::OnProcess
}

struct SmallString {
    unsigned char tag;
    char inlineData[3];
    unsigned int heapLength;
    char* data;
};
struct PendingPetAnimationEntry {
    unsigned char queueInsteadOfPlay;
    char pad[3];
    unsigned int value;
    SmallString string;
};

struct LoadAllocator {
    volatile char byte;
    char pad[3];
    // Retain the allocator byte ABI; default construction intentionally leaves it unset.
    LoadAllocator() {}
    // Copy the allocator byte without acquiring storage ownership.
    LoadAllocator(const LoadAllocator& s) : byte(s.byte) {}
};
// MW strings: bit 31 of flags selects heap storage. Inline bytes start at +1 and
// their length occupies the low seven tag bits; heap length/pointer are at +4/+8.
// Heap copies allocate independently. Dialogue-map nodes own both key and wide value.
struct LoadString {
    unsigned int flags, length;
    char* data;
    // Initialize an empty narrow string or independently copy inline/heap source text.
    LoadString() : flags(0), length(0), data(0) {}
    // Release only heap-backed narrow-string storage.
    ~LoadString() {
        if (flags >> 31)
            appFreeUni__FPv(data);
    }
    // Test whether characters reside in the string object rather than a heap buffer.
    bool isShort() const {
        return (flags >> 31) == 0;
    }
    // Test whether the string owns a heap buffer.
    bool isLong() const {
        return !isShort();
    }
    // Read the inline character count from the low seven tag bits.
    unsigned int shortLen() const {
        return *(const unsigned char*)this & 0x7f;
    }
    // Initialize an empty narrow string or independently copy inline/heap source text.
    LoadString(const LoadString& s) {
        unsigned int f = s.flags;
        if (!(f >> 31)) {
            flags = f;
            length = s.length;
            data = s.data;
        } else {
            flags = 0;
            length = 0;
            data = 0;
            String_ReserveCapacity(this, s.length);
            LoadAllocator copy, alloc;
            copy.byte = alloc.byte;
            String_ReplaceRange(this, 0, 0, s.data, s.data + s.length, &copy);
        }
    }
};
struct LoadWide {
    unsigned int flags, length;
    unsigned short* data;
    // Initialize an empty wide string or independently copy inline/heap source text.
    LoadWide() : flags(0), length(0), data(0) {}
    // Release only heap-backed wide-string storage.
    ~LoadWide() {
        if (flags >> 31)
            appFreeUni__FPv(data);
    }
    // Initialize an empty wide string or independently copy inline/heap source text.
    LoadWide(const LoadWide& s) {
        unsigned int f = s.flags;
        if (!(f >> 31)) {
            flags = f;
            length = s.length;
            data = s.data;
        } else {
            unsigned int n = s.length;
            flags = 0;
            length = 0;
            data = 0;
            WString_ReserveCapacity(this, n);
            LoadAllocator a, c = a;
            WString_ReplaceRange(this, 0, 0, s.data, s.data + s.length, &c);
        }
    }
};
struct LoadValue {
    unsigned char type;
    LoadWide text;
    // Construct the wide-text member; the prerequisite tag remains uninitialized.
    LoadValue() {}
    // Construct the wide-text member and copy the prerequisite tag when supplied.
    LoadValue(const LoadValue& s) : type(s.type), text(s.text) {}
    // Let the owned wide-text member release its storage.
    ~LoadValue() {}
};
struct LoadPair {
    LoadString key;
    LoadValue value;
    // Copy the animation-name key and its prerequisite/text value into an owned pair.
    LoadPair(const LoadString& k, const LoadValue& v) : key(k), value(v) {}
    // Destroy the owned key and value through their member destructors.
    ~LoadPair() {}
};
struct LoadFile {
    virtual void slot0();
    virtual void slot1();
    virtual void read(void*, unsigned int);
    unsigned int length;
};
struct LoadFileGuard_u80129C6C {
    LoadFile** ptr;
    // Borrow the live file-handle slot so early release can clear the guard target.
    LoadFileGuard_u80129C6C(LoadFile** p) : ptr(p) {}
    // Release a remaining file handle and clear its owner slot on exit.
    ~LoadFileGuard_u80129C6C() {
        if (*ptr) {
            if (*ptr) {
                Resource_Release(*ptr);
                *ptr = 0;
            }
        }
    }
};
struct DialogueTreeNode {
    void* left;
    void* right;
    unsigned int parentAndColor;
    LoadPair pair;
};

struct LoadOwnerC {
    char pad00[0x28];
    unsigned int count;
    void* root;
    char pad30[4];
    void** end;
};

// Return the shorter byte-count reference for lexicographic comparison.
static inline const unsigned int& load_min(const unsigned int& a, const unsigned int& b) {
    return a < b ? a : b;
}
// Compare a bounded string prefix with external bytes, breaking equal prefixes by length.
static inline int load_compare_part(const LoadString& a, unsigned int n, const char* q, unsigned int len) {
    unsigned int other;
    const char* p;
    if (!(a.flags >> 31)) {
        p = (const char*)&a + 1;
        other = *(const unsigned char*)&a & 0x7f;
    } else {
        p = a.data;
        other = a.length;
    }
    unsigned int count = load_min(len, other);
    int cmp = memcmp(p, q, load_min(n, count));
    if (!cmp) {
        if (count < n)
            return -1;
        if (count == n)
            return 0;
        return 1;
    }
    return cmp;
}
// Compare bounded string bytes with reference-shaped length arguments preserved for the ABI.
static inline int load_compare_ref(const LoadString& a, const unsigned int& n, const char* q, const unsigned int& len) {
    unsigned int other;
    const char* p;
    if (!(a.flags >> 31)) {
        p = (const char*)&a + 1;
        other = *(const unsigned char*)&a & 0x7f;
    } else {
        p = a.data;
        other = a.length;
    }
    unsigned int count = load_min(len, other);
    int cmp = memcmp(p, q, load_min(n, count));
    if (!cmp) {
        if (count < n)
            return -1;
        if (count == n)
            return 0;
        return 1;
    }
    return cmp;
}
// Compare two short/heap narrow strings lexicographically.
static inline int load_compare(const LoadString& a, const LoadString& b) {
    unsigned long n;
    const char* q;
    if (!(b.flags >> 31)) {
        q = (const char*)&b + 1;
        n = *(const unsigned char*)&b & 0x7f;
    } else {
        q = b.data;
        n = b.length;
    }
    unsigned int len;
    if (!(a.flags >> 31))
        len = *(const unsigned char*)&a & 0x7f;
    else
        len = a.length;
    return load_compare_part(a, n, q, len);
}
// Find an insertion position in the caddie entry's owned animation-name tree.
static inline void* load_lookup(void* arg, LoadString* key, void** out, unsigned char* changed, unsigned char* last) {
    return CaddieDialogueTextMap_FindInsertionPosition((char*)&((LoadOwnerC*)arg)->count, key, out, changed, last);
}
// Copy a key/value pair into a newly inserted animation-name tree node.
static inline void* load_insert(void* arg, void* position, unsigned char changed, unsigned char last, LoadPair* pair) {
    return StringWStringMap_InsertNode((char*)&((LoadOwnerC*)arg)->count, position, changed, last, pair);
}

struct LoadNode {
    LoadNode* left;
    LoadNode* right;
    char parentAndColor[4];
    LoadString key;
    unsigned char type;
};
struct QueryRaw {
    unsigned int flags, length;
    char* data;
};

// Return this entry's end sentinel; its address is &root, never a payload node.
static inline LoadNode* PrerequisiteEnd(void* entry) {
    return (LoadNode*)&((LoadOwnerC*)entry)->root;
}

// Break equal byte-prefix comparisons by the compared string lengths.
static inline int query_finish(int cmp, const unsigned int& count, unsigned int n) {
    if (!cmp) {
        if (count < n)
            return -1;
        if (count == n)
            return 0;
        return 1;
    }
    return cmp;
}

#pragma use_lmw_stmw on
#pragma opt_common_subs off
#pragma opt_propagation off
#pragma opt_strength_reduction off
#pragma opt_lifetimes off
// Look up an animation prerequisite and reject it when the current operation type is too low.
// Missing names are permitted. A mapped type 0 needs no active operation; higher types require
// the ACTIVE caddie operation type >= the map value, even when caddieEntry is another entry.
// Both entry pointers must be valid when testing a mapped name; the name is borrowed NUL-terminated text.
int CGolfCaddie::CanPlayAnimation(void* caddieEntry, const char* animationName) {
    struct Frame {
        LoadAllocator copy, alloc;
        unsigned int count, other, n, len;
        unsigned int len2, n2, other2, count2;
        QueryRaw query;
    } frame;
#define query frame.query
    query.flags = 0;
    query.length = 0;
    query.data = 0;
    unsigned int animationNameLength = strlen(animationName);
    String_ReserveCapacity(&query, animationNameLength);
    frame.copy.byte = frame.alloc.byte;
    String_ReplaceRange(&query, 0, 0, animationName, animationName + animationNameLength, &frame.copy);
    unsigned int n;
    LoadNode* searchNode;
    char* queryHeapData;
    unsigned int queryHeapLength;
    unsigned int queryOwnsHeap;
    unsigned int queryInlineLength;
    LoadNode* lowerBoundCandidate;
    bool isLong;
    bool isShort;
    lowerBoundCandidate = PrerequisiteEnd(caddieEntry);
    queryInlineLength = *(unsigned char*)&query & 0x7f;
    queryOwnsHeap = query.flags >> 31;
    searchNode = PrerequisiteEnd(caddieEntry)->left;
    queryHeapData = query.data;
    queryHeapLength = query.length;
    isShort = queryOwnsHeap == 0;
    isLong = isShort == 0;
    // Query representation is cached once: inline bytes live at +1, heap bytes
    // at data. Comparison is memcmp(min(nodeLength,queryLength)), then length.
    // The frame's paired count slots preserve the inlined string API's references.
    // Find the lower bound for animationName using short/heap string comparison.
    while (searchNode) {
        const char* q;
        if (!isLong) {
            n = queryInlineLength;
            q = (const char*)&query + 1;
        } else {
            q = queryHeapData;
            n = queryHeapLength;
        }
        frame.n = n;
        unsigned int len;
        len =
            (!(searchNode->key.flags >> 31)) ? *(const unsigned char*)&searchNode->key & 0x7f : searchNode->key.length;
        frame.len = len;
        const char* p;
        unsigned int other;
        if (!(searchNode->key.flags >> 31)) {
            p = (const char*)&searchNode->key + 1;
            other = *(const unsigned char*)&searchNode->key & 0x7f;
        } else {
            p = searchNode->key.data;
            other = searchNode->key.length;
        }
        frame.other = other;
        const unsigned int* small = &frame.other;
        if (len < other)
            small = &frame.len;
        frame.count = *small;
        small = &frame.count;
        if (n < frame.count)
            small = &frame.n;
        int cmp = memcmp(p, q, *small);
        if (query_finish(cmp, frame.count, n) >= 0) {
            lowerBoundCandidate = searchNode;
            searchNode = searchNode->left;
        } else {
            searchNode = searchNode->right;
        }
    }
    // lower_bound can point to a greater name; compare in the reverse direction
    // to accept only equality. The end sentinel is never dereferenced for its key.
    do {
        if (lowerBoundCandidate != PrerequisiteEnd(caddieEntry)) {
            const char* q;
            unsigned int n;
            if (!(lowerBoundCandidate->key.flags >> 31)) {
                q = (const char*)&lowerBoundCandidate->key + 1;
                n = *(const unsigned char*)&lowerBoundCandidate->key & 0x7f;
            } else {
                q = lowerBoundCandidate->key.data;
                n = lowerBoundCandidate->key.length;
            }
            frame.n2 = n;
            unsigned int len;
            len = (!isLong) ? queryInlineLength : query.length;
            frame.len2 = len;
            const char* p;
            if (!isLong) {
                p = (const char*)&query + 1;
            } else {
                p = query.data;
                queryInlineLength = query.length;
            }
            frame.other2 = queryInlineLength;
            const unsigned int* small = &frame.other2;
            if (len < queryInlineLength)
                small = &frame.len2;
            frame.count2 = *small;
            small = &frame.count2;
            if (n < frame.count2)
                small = &frame.n2;
            int cmp = memcmp(p, q, *small);
            if (query_finish(cmp, frame.count2, n) >= 0)
                break;
        }
        lowerBoundCandidate = PrerequisiteEnd(caddieEntry);
    } while (0);
    // The query is no longer needed; only an exact map entry imposes a prerequisite.
    if (queryOwnsHeap)
        appFreeUni__FPv(query.data);
    if (lowerBoundCandidate == PrerequisiteEnd(caddieEntry))
        return 1;
    void* owner = activeCaddie; // original +0x1C
    void* current = ((CaddieOperationOwner*)owner)->operation;
    if (current) {
        if (((CaddieOperationType*)current)->type < lowerBoundCandidate->type)
            return 0;
    } else {
        if (lowerBoundCandidate->type != 0)
            return 0;
    }
    return 1;
#undef query
}
#pragma opt_common_subs reset
#pragma opt_propagation reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset

// Destroy both descendants, the owned string pair, then one non-null map node.
// The caller retains the tree header and resets count/root/begin after all nodes go.
static inline void DestroyPrerequisiteNode(void* caddieEntry, void* node) {
    if (*(void**)node)
        CaddieDialogueStringMap_DestroySubtree((char*)&((LoadOwnerC*)caddieEntry)->count, *(void**)node);
    if (((DialogueTreeNode*)node)->right)
        CaddieDialogueStringMap_DestroySubtree((char*)&((LoadOwnerC*)caddieEntry)->count,
                                               ((DialogueTreeNode*)node)->right);
    CaddieDialogueStringPair_Destroy((char*)&((LoadOwnerC*)caddieEntry)->count, &((DialogueTreeNode*)node)->pair);
    appFreeUni__FPv(node);
}

// Inert compatibility self-alias; preprocessing retains the existing C link symbol.
#define CGolfCaddie_LoadAnimationPrerequisites CGolfCaddie_LoadAnimationPrerequisites

// Replace the caddie model's .mot text map with parsed colon-delimited keys and prerequisite types.
// Legacy symbol says dialogue text; this loader supplies CanPlayAnimation's prerequisite map.
// .mot records are "decimal prerequisite : animation name". The map owns copies of names;
// existing nodes are destroyed before opening the file, so open failure leaves an empty map.
// Parsing uses fixed 256-byte temporary buffers without bounds checks; preserve that contract.
void CGolfCaddie_LoadAnimationPrerequisites(void* self, void* caddieEntry) {
    void* oldRoot;
    void* subtree;
    void* leftSideBranch;
    void* leftDescendant;
    void* rightSideBranch;
    void* leftSiblingDescendant;
    void* rightDescendant;
    void* rightSiblingDescendant;
    LoadOwnerC* owner = (LoadOwnerC*)caddieEntry;
    oldRoot = owner->root;
    // Destroy descendants in postorder, then the root's inline strings; retain the unrolled order.
    if (oldRoot) {
        subtree = *(void**)oldRoot;
        if (subtree) {
            leftSideBranch = *(void**)subtree;
            if (leftSideBranch) {
                leftDescendant = *(void**)leftSideBranch;
                if (leftDescendant) {
                    DestroyPrerequisiteNode(caddieEntry, leftDescendant);
                }
                leftDescendant = ((DialogueTreeNode*)leftSideBranch)->right;
                if (leftDescendant) {
                    DestroyPrerequisiteNode(caddieEntry, leftDescendant);
                }
                CaddieDialogueStringPair_Destroy((char*)&((LoadOwnerC*)caddieEntry)->count,
                                                 &((DialogueTreeNode*)leftSideBranch)->pair);
                appFreeUni__FPv(leftSideBranch);
            }
            leftSideBranch = ((DialogueTreeNode*)subtree)->right;
            if (leftSideBranch) {
                leftSiblingDescendant = *(void**)leftSideBranch;
                if (leftSiblingDescendant) {
                    DestroyPrerequisiteNode(caddieEntry, leftSiblingDescendant);
                }
                leftSiblingDescendant = ((DialogueTreeNode*)leftSideBranch)->right;
                if (leftSiblingDescendant) {
                    DestroyPrerequisiteNode(caddieEntry, leftSiblingDescendant);
                }
                CaddieDialogueStringPair_Destroy((char*)&((LoadOwnerC*)caddieEntry)->count,
                                                 &((DialogueTreeNode*)leftSideBranch)->pair);
                appFreeUni__FPv(leftSideBranch);
            }
            CaddieDialogueStringPair_Destroy((char*)&((LoadOwnerC*)caddieEntry)->count,
                                             &((DialogueTreeNode*)subtree)->pair);
            appFreeUni__FPv(subtree);
        }
        subtree = ((DialogueTreeNode*)oldRoot)->right;
        if (subtree) {
            rightSideBranch = *(void**)subtree;
            if (rightSideBranch) {
                rightDescendant = *(void**)rightSideBranch;
                if (rightDescendant) {
                    DestroyPrerequisiteNode(caddieEntry, rightDescendant);
                }
                rightDescendant = ((DialogueTreeNode*)rightSideBranch)->right;
                if (rightDescendant) {
                    DestroyPrerequisiteNode(caddieEntry, rightDescendant);
                }
                CaddieDialogueStringPair_Destroy((char*)&((LoadOwnerC*)caddieEntry)->count,
                                                 &((DialogueTreeNode*)rightSideBranch)->pair);
                appFreeUni__FPv(rightSideBranch);
            }
            rightSideBranch = ((DialogueTreeNode*)subtree)->right;
            if (rightSideBranch) {
                rightSiblingDescendant = *(void**)rightSideBranch;
                if (rightSiblingDescendant) {
                    DestroyPrerequisiteNode(caddieEntry, rightSiblingDescendant);
                }
                rightSiblingDescendant = ((DialogueTreeNode*)rightSideBranch)->right;
                if (rightSiblingDescendant) {
                    DestroyPrerequisiteNode(caddieEntry, rightSiblingDescendant);
                }
                CaddieDialogueStringPair_Destroy((char*)&((LoadOwnerC*)caddieEntry)->count,
                                                 &((DialogueTreeNode*)rightSideBranch)->pair);
                appFreeUni__FPv(rightSideBranch);
            }
            CaddieDialogueStringPair_Destroy((char*)&((LoadOwnerC*)caddieEntry)->count,
                                             &((DialogueTreeNode*)subtree)->pair);
            appFreeUni__FPv(subtree);
        }
        {
            char* p = (char*)oldRoot;
            if (p + 0xc) {
                if (p + 0x18) {
                    if (p + 0x1c) {
                        if ((*(unsigned int*)(p + 0x1c)) >> 31)
                            appFreeUni__FPv(*(void**)(p + 0x24));
                    }
                }
                if (p + 0xc) {
                    if ((*(unsigned int*)(p + 0xc)) >> 31)
                        appFreeUni__FPv(*(void**)(p + 0x14));
                }
            }
        }
        appFreeUni__FPv(oldRoot);
        owner->count = 0;
        owner->root = 0;
        owner->end = &owner->root;
    }

    // Old map is empty: open the model-specific file and retain it through the unwind guard.
    char filename[32];
    filename[0] = 0;
    sprintf(filename, golfCaddieStrings + kGolfCaddie_s_mot,
            AsGolfCaddieEntry80128808(caddieEntry)->caddieRecord->model);
    LoadFile* file = (LoadFile*)Resource_OpenResolvedPath(g_resourceManager, filename, -1, 0, 0, 0);
    LoadFileGuard_u80129C6C guard(&file);
    if (file) {
        unsigned int length = file->length;
        char* buffer = (char*)__nwa__FUl(length + 1);
        file->read(buffer, length);
        buffer[length] = 0;
        if (file) {
            Resource_Release(file);
            file = 0;
        }
        // The file is now closed; parse the private NUL-terminated copy into owned map entries.
        LoadAllocator allocator;
        unsigned char allocatorByte = *(volatile unsigned char*)&allocator;
        char* cursor = buffer;
        while (cursor && *cursor && strstr(cursor, golfCaddieStrings + kGolfCaddie_keyDelimiter)) {
            char prerequisiteText[256] = {};
            char animationNameText[256] = {};
            signed char* s = String_ParseKeyBeforeColon(self, (signed char*)prerequisiteText, (signed char*)cursor);
            if (!s || !*s || !prerequisiteText[0])
                break;
            char* dst = animationNameText;
            while (*s == ' ')
                ++s;
            for (;;) {
                *dst = *s;
                ++dst;
                char ch = *++s;
                if (ch == '\r' || ch == 0)
                    break;
            }
            *dst = 0;
            if (*s)
                ++s;
            cursor = (char*)s;

            // Find or create the animation-name key; duplicate names overwrite the prerequisite tag.
            LoadPair* destination;
            {
                LoadString key;
                unsigned int len = strlen(animationNameText);
                String_ReserveCapacity(&key, len);
                LoadAllocator copy;
                copy.byte = allocatorByte;
                String_ReplaceRange(&key, 0, 0, animationNameText, animationNameText + len, &copy);

                void* out;
                unsigned char becomesFirst, insertLeft;
                void* position = load_lookup(caddieEntry, &key, &out, &insertLeft, &becomesFirst);
                do {
                    if (out) {
                        destination = &((DialogueTreeNode*)out)->pair;
                        if (load_compare(destination->key, key) >= 0)
                            break;
                    }
                    {
                        LoadValue empty;
                        LoadPair pair(key, empty);
                        destination =
                            &((DialogueTreeNode*)load_insert(caddieEntry, position, insertLeft, becomesFirst, &pair))
                                 ->pair;
                    }
                } while (0);
                ;
            }
            destination->value.type = (unsigned char)String_ParseDecimalInt(prerequisiteText);
        }
        __dla__FPv(buffer);
    }
}

#undef CGolfCaddie_LoadAnimationPrerequisites

struct VectorWords {
    unsigned int a, b, c;
};
// Borrowed command state: position is a world-space vector stored as three float bit patterns.
// Position/yaw flags are consumed once; yawDegrees is converted to radians at the pet boundary.
// Animation entries are owned by state: each update consumes at most one entry, even when
// queueInsteadOfPlay cannot enqueue because a next animation is already present.
struct PendingPetState {
    char pad0[0xC];
    int snapToGround;
    int positionPending;
    int yawPending;
    int animationPending;
    VectorWords position;
    char pad28[4];
    float yawDegrees;
    char pad30[4];
    PendingPetAnimationEntry* entries;
    int count;
};
struct PetUpdateInterface;
struct PetUpdateVtable {
    char pad[0x74];
    void (*fn)(PetUpdateInterface*, float);
};
struct PetUpdateInterface {
    PetUpdateVtable* vt;
};
struct PendingPetCommand {
    void* p0;
    PendingPetState* state;
    void** pet;
};

// Shift animation entries by assignment, copying owned strings rather than their heap pointers.
static inline PendingPetAnimationEntry* CopyPendingAnimationEntries(PendingPetAnimationEntry* src,
                                                                    PendingPetAnimationEntry* last,
                                                                    PendingPetAnimationEntry* dst,
                                                                    LoadAllocator& allocator) {
    for (; src < last; ++src, ++dst) {
        dst->queueInsteadOfPlay = src->queueInsteadOfPlay;
        dst->value = src->value;
        unsigned int dh = (*(unsigned int*)&dst->string) >> 31;
        unsigned int sw;
        if (!dh && !((sw = *(unsigned int*)&src->string) >> 31)) {
            unsigned int* d = (unsigned int*)&dst->string;
            unsigned int* s = (unsigned int*)&src->string;
            unsigned int b = s[1];
            d[0] = sw;
            d[1] = b;
            d[2] = s[2];
        } else {
            unsigned int dstSize = dh ? dst->string.heapLength : (dst->string.tag & 0x7f);
            const char* srcData;
            unsigned int srcSize;
            if (!((*(unsigned int*)&src->string) >> 31)) {
                srcData = src->string.inlineData;
                srcSize = src->string.tag & 0x7f;
            } else {
                srcData = src->string.data;
                srcSize = src->string.heapLength;
            }
            LoadAllocator terminator(allocator);
            String_ReplaceRange(&dst->string, 0, dstSize, srcData, srcData + srcSize, &terminator);
        }
    }
    return dst;
}

// Apply queued position, yaw and animation requests, then update the pet for this frame.
void CGolfCaddie::ApplyPendingPetState(float delta) {
    PendingPetCommand* self = (PendingPetCommand*)this;
    if (!self->state || !self->pet)
        return;

    if (self->state->positionPending) {
        self->state->positionPending = 0;
        VectorWords positionWords = self->state->position;
        if (self->state->snapToGround) {
            VectorWords groundPositionWords;
            VectorWords requestedPositionWords = self->state->position;
            GetGroundPoint__9CQuadTreeF7WVectorbPib(&groundPositionWords,
                                                    CQuadTree_GetInstance((void*)requestedPositionWords.a),
                                                    &requestedPositionWords, 1, 0, 0);
            *(WVector*)&positionWords = *(WVector*)&groundPositionWords;
        }
        ((CPetBody*)(*self->pet))->SetPosition(&positionWords);
    }

    if (self->state->yawPending) {
        self->state->yawPending = 0;
        ((CPetBody*)(*self->pet))->SetYaw(kPetYawDegreesToRadians * self->state->yawDegrees);
    }

    if (self->state->animationPending) {
        if (self->state->count) {
            PendingPetAnimationEntry* entries = self->state->entries;
            if (!entries->queueInsteadOfPlay) {
                char* value =
                    !((*(unsigned int*)&entries->string) >> 31) ? entries->string.inlineData : entries->string.data;
                ((void (*)(void*, void*, float, int, int))PlayAnimationByName__8CPetBodyFfPvii)(
                    *self->pet, value, kFloat_0_GolfCaddie, 0, 1);
            } else if (!strlen(((CPetBody*)(*self->pet))->GetNextAnimationName())) {
                PendingPetAnimationEntry* e = self->state->entries;
                char* value = !((*(unsigned int*)&e->string) >> 31) ? e->string.inlineData : e->string.data;
                ((CPetBody*)(*self->pet))->SetNextAnimation(value, 0);
            }

            // Erase the consumed front entry by shifting strings, then destroy the former tail.
            PendingPetState* state = self->state;
            LoadAllocator junk;
            PendingPetAnimationEntry* pos = state->entries;
            PendingPetAnimationEntry* last = pos + state->count;
            if (last - pos - 1 != 0)
                CopyPendingAnimationEntries(pos + 1, last, pos, junk);
            int n = state->count - 1;
            PendingPetAnimationEntry* base = state->entries;
            state->count = n;
            PendingPetAnimationEntry* tail = base + n;
            if (tail) {
                tail->queueInsteadOfPlay = 0;
                tail->value = 0;
                if (!((*(unsigned int*)&tail->string) >> 31)) {
                    tail->string.inlineData[0] = 0;
                    *(signed char*)&tail->string.tag &= ~0x7f;
                } else {
                    tail->string.data[0] = 0;
                    tail->string.heapLength = 0;
                }
                if (&tail->string && ((*(unsigned int*)&tail->string) >> 31))
                    appFreeUni__FPv(tail->string.data);
            }
        } else if (((CPetBody*)(*self->pet))->GetAnimationFinished()) {
            self->state->animationPending = 0;
        }
    }

    PetUpdateInterface* object = (PetUpdateInterface*)*self->pet;
    void (*fn)(PetUpdateInterface*, float) = object->vt->fn;
    fn(object, delta);
}

#pragma dont_inline on
#pragma use_lmw_stmw on
#pragma opt_common_subs off
#pragma opt_propagation off
#pragma opt_strength_reduction off
#pragma opt_lifetimes off
// Update scripted caddies or the active caddie's movement, idle animations, commentary and ring effect.
// delta is elapsed seconds; turnTimeLimitMs/remainingTimeMs are milliseconds.
// Position/target/displacement are world-space, Y-up; yaw is radians around Y.
// Early returns skip the final virtual pet tick as well as ring/speech updates.
void CGolfCaddie::OnProcess(float delta) {
    int flags = *(unsigned int*)((CGolfDoc*)((char*)GolfDoc_GetCurrent()))->GetGameStateFlagsPtr();
    char* caddie = (char*)this;

    // Choose scripted multi-caddie processing or the active-caddie path; both can return early.
    if (flags & kCaddieProcessSuppressed)
        return;
    // original 8012A9F4 bne 8012B030: returns at the epilogue, as every early exit below; only the main path reaches the tail (the *activeCaddie virtual update)
    {
        flags = *(unsigned int*)((CGolfDoc*)((char*)GolfDoc_GetCurrent()))->GetGameStateFlagsPtr();
        if (flags & kCaddieProcessConte) {
            int offset;
            int i;
            for (offset = 0, i = 0; i < (unsigned char)GolfDoc_GetPlayerCount(); offset += 12, i++) {
                ((CGolfCaddie*)((PendingPetCommand*)((char*)AsCGolfCaddie(caddie)->conteCaddies + offset)))
                    ->ApplyPendingPetState(delta);
                void* node = *(void**)(((char*)AsCGolfCaddie(caddie)->conteCaddies + offset) + 8);
                if (node)
                    ((CGolfCaddie*)this)->UpdateBoneRelativeRenderParams(((CPetBody*)(*(void**)node))->GetPuppet());
                CaddieObject_ApplyEnabledState((char*)AsCGolfCaddie(caddie)->conteCaddies + offset);
            }
            return; // original 8012AA74 effects 8012B030 (epilogue)
        }
        if (((CGolfCaddie*)this)->UpdatePendingOperation())
            return; // original 8012AA84 bne 8012B030
        void* activeEntry = AsCGolfCaddie(caddie)->activeCaddie;
        if (0 == activeEntry)
            return; // original 8012AA90 beq 8012B030
        if (!*(void**)activeEntry)
            return; // original 8012AA9C..AAA0 bne / effects 8012B030
        ((CGolfCaddie*)this)
            ->UpdateBoneRelativeRenderParams(
                ((CPetBody*)(*(void**)activeEntry))
                    ->GetPuppet()); // original 0x8012AA94 lwz r3,0x0(r3), tested, then the bl: one dereference
        flags = *(unsigned int*)((CGolfDoc*)((char*)GolfDoc_GetCurrent()))->GetGameStateFlagsPtr();
        // Count down movement delay; crossing zero triggers yaw, run animation and dust setup.
        if (flags & kCaddieMovementClockRunning) {
            float v = AsCGolfCaddie(caddie)->movementDelay;
            if (v > kFloat_0_GolfCaddie) {
                AsCGolfCaddie(caddie)->movementDelay = v - delta;
                if (Float_IsAbsBelow(&AsCGolfCaddie(caddie)->movementDelay, kFloat_0_00001_Caddie))
                    AsCGolfCaddie(caddie)->movementDelay = kFloat_neg0_00001_GolfCaddie;
                // original +0x44
            }
        }
        if (AsCGolfCaddie(caddie)->movementDelay < kFloat_0_GolfCaddie) {
            int targetPositionBits[3];
            char worldForward[12];

            char* s = Vec3_Set(worldForward, kFloat_0_GolfCaddie, kFloat_0_GolfCaddie, kFloat_1_GolfCaddie);
            WVector petForwardForYaw =
                ((CPetBody*)(*(void**)AsCGolfCaddie(caddie)->activeCaddie))->GetNegatedBaseForward();
            AsCGolfCaddie(caddie)->yaw = Vector_SignedAngleXZ(&petForwardForYaw, s);
            WVector petForwardForTurn =
                ((CPetBody*)(*(void**)AsCGolfCaddie(caddie)->activeCaddie))->GetNegatedBaseForward();
            unsigned char targetDisplacement[12];
            __mi__FRC7WVectorRC7WVector(targetDisplacement, AsCGolfCaddie(caddie)->movementTarget,
                                        AsCGolfCaddie(caddie)->position); // original +0x4C, +0x38
            AsCGolfCaddie(caddie)->yaw += Vector_SignedAngleXZ(targetDisplacement, &petForwardForTurn);
            ((CPetBody*)(*(void**)AsCGolfCaddie(caddie)->activeCaddie))->SetYaw(AsCGolfCaddie(caddie)->yaw);
            AsCGolfCaddie(caddie)->movementDelay = kFloat_0_GolfCaddie;
            if (AsCGolfCaddie(caddie)->animationState != kCaddieRunning) {
                AsCGolfCaddie(caddie)->animationState = kCaddieRunning;
                if (AsCGolfDoc(GolfDoc_GetCurrent())->tutorialStage == 6)
                    ((CPetBody*)(*(void**)AsCGolfCaddie(caddie)->activeCaddie))
                        ->PlayAnimationByName(kFloat_0_GolfCaddie,
                                              (void*)(golfCaddieStrings + kGolfCaddie_runAnimation), 1, 0);
                else
                    ((CPetBody*)(*(void**)AsCGolfCaddie(caddie)->activeCaddie))
                        ->PlayAnimationByName(kFloat_0_GolfCaddie,
                                              (void*)(golfCaddieStrings + kGolfCaddie_tutorialRunAnimation), 1, 0);
                float* movementDirection =
                    ((CPetBody*)(*(void**)AsCGolfCaddie(caddie)->activeCaddie))->GetMovementDirectionPtr();
                void* effects = CFx::GetInstance();
                AsCGolfCaddie(caddie)->runDustEffect =
                    ((CFx*)effects)->MakeRunDust(AsCGolfCaddie(caddie)->position, movementDirection); // original +0x38
                if (AsCGolfCaddie(caddie)->texture3D &&
                    ((CaddieAnimationMetadata*)AsCGolfCaddie(caddie)->activeCaddie)->ringEnabled)
                    CaddieRing_SetVisible(AsCGolfCaddie(caddie)->texture3D, 0);
            }
            targetPositionBits[0] = *(int*)&AsCGolfCaddie(caddie)->movementTarget[0]; // original +0x4C: copy float bits
            targetPositionBits[1] = *(int*)&AsCGolfCaddie(caddie)->movementTarget[1]; // original +0x50
            targetPositionBits[2] = *(int*)&AsCGolfCaddie(caddie)->movementTarget[2]; // original +0x54
            ((CPetBody*)(*(void**)AsCGolfCaddie(caddie)->activeCaddie))->StartMovement((float*)targetPositionBits);
        }
    }
    // Movement setup is complete: update running motion or timed idle/hurry commentary.
    if (AsCGolfCaddie(caddie)->animationState == kCaddieRunning)
        ((CGolfCaddie*)this)->UpdateMovement();
    else {
        void* ball = GolfBall__Fv();
        if (((CGolfBall*)ball)->m_eventPreDataIndex[0] < 0) {
            unsigned int turnTimeLimitMs = SharedDoc_GetTurnTimeLimitMs();
            if (0 == turnTimeLimitMs) {
                AsCGolfCaddie(caddie)->idleAnimationTimer -= delta;
                if (AsCGolfCaddie(caddie)->idleAnimationTimer < kFloat_0_GolfCaddie) {
                    AsCGolfCaddie(caddie)->idleAnimationTimer = kFloat_20_GolfCaddie;
                    if (CanPlayAnimation(AsCGolfCaddie(caddie)->activeCaddie,
                                         golfCaddieStrings + kGolfCaddie_waitAnimation)) {
                        AsCGolfCaddie(caddie)->animationState = kCaddieWaiting;
                        ((CPetBody*)(*(void**)AsCGolfCaddie(caddie)->activeCaddie))
                            ->PlayAnimationByName(kFloat_0_GolfCaddie,
                                                  (void*)(golfCaddieStrings + kGolfCaddie_waitAnimation), 0, 0);
                    }
                }
                if (AsCGolfCaddie(caddie)->animationState == kCaddieWaiting &&
                    !strcmp(((CPetBody*)(*(void**)AsCGolfCaddie(caddie)->activeCaddie))->GetCurrentAnimationName(),
                            golfCaddieStrings + kGolfCaddie_idleAnimation))
                    AsCGolfCaddie(caddie)->animationState = kCaddieIdle;
            } else if (turnTimeLimitMs == kFortySecondsMs) {

                if (AsCGolfDoc(GolfDoc_GetCurrent())->remainingTimeMs < kTenSecondsMs &&
                    AsCGolfCaddie(caddie)->animationState == kCaddieWaiting) {
                    if (CanPlayAnimation(AsCGolfCaddie(caddie)->activeCaddie,
                                         golfCaddieStrings + kGolfCaddie_hurryAnimation)) {
                        AsCGolfCaddie(caddie)->animationState = kCaddieHurrying;
                        ((CPetBody*)(*(void**)AsCGolfCaddie(caddie)->activeCaddie))
                            ->PlayAnimationByName(kFloat_0_GolfCaddie,
                                                  (void*)(golfCaddieStrings + kGolfCaddie_hurryAnimation), 0, 0);
                    }
                } else if (AsCGolfDoc(GolfDoc_GetCurrent())->remainingTimeMs < kTwentySecondsMs &&
                           AsCGolfCaddie(caddie)->animationState == kCaddieIdle) {
                    if (CanPlayAnimation(AsCGolfCaddie(caddie)->activeCaddie,
                                         golfCaddieStrings + kGolfCaddie_waitAnimation)) {
                        AsCGolfCaddie(caddie)->animationState = kCaddieWaiting;
                        ((CPetBody*)(*(void**)AsCGolfCaddie(caddie)->activeCaddie))
                            ->PlayAnimationByName(kFloat_0_GolfCaddie,
                                                  (void*)(golfCaddieStrings + kGolfCaddie_waitAnimation), 0, 0);
                    }
                } else if (AsCGolfDoc(GolfDoc_GetCurrent())->remainingTimeMs > kTwentySecondsMs) {
                    AsCGolfCaddie(caddie)->animationState = kCaddieIdle;
                }
            } else if (turnTimeLimitMs == kSixtySecondsMs) {

                if (AsCGolfDoc(GolfDoc_GetCurrent())->remainingTimeMs < kTenSecondsMs &&
                    AsCGolfCaddie(caddie)->animationState == kCaddieAnimationState2) {
                    if (CanPlayAnimation(AsCGolfCaddie(caddie)->activeCaddie,
                                         golfCaddieStrings + kGolfCaddie_hurryAnimation)) {
                        AsCGolfCaddie(caddie)->animationState = kCaddieHurrying;
                        ((CPetBody*)(*(void**)AsCGolfCaddie(caddie)->activeCaddie))
                            ->PlayAnimationByName(kFloat_0_GolfCaddie,
                                                  (void*)(golfCaddieStrings + kGolfCaddie_hurryAnimation), 0, 0);
                    }
                } else if (AsCGolfDoc(GolfDoc_GetCurrent())->remainingTimeMs < kTwentySecondsMs &&
                           AsCGolfCaddie(caddie)->animationState == kCaddieIdle) {
                    if (CanPlayAnimation(AsCGolfCaddie(caddie)->activeCaddie,
                                         golfCaddieStrings + kGolfCaddie_waitAnimation)) {
                        AsCGolfCaddie(caddie)->animationState = kCaddieAnimationState2;
                        ((CPetBody*)(*(void**)AsCGolfCaddie(caddie)->activeCaddie))
                            ->PlayAnimationByName(kFloat_0_GolfCaddie,
                                                  (void*)(golfCaddieStrings + kGolfCaddie_waitAnimation), 0, 0);
                    }
                } else if (AsCGolfDoc(GolfDoc_GetCurrent())->remainingTimeMs < kFortySecondsMs &&
                           AsCGolfCaddie(caddie)->animationState == kCaddieIdle) {
                    if (CanPlayAnimation(AsCGolfCaddie(caddie)->activeCaddie,
                                         golfCaddieStrings + kGolfCaddie_waitAnimation)) {
                        AsCGolfCaddie(caddie)->animationState = kCaddieWaiting;
                        ((CPetBody*)(*(void**)AsCGolfCaddie(caddie)->activeCaddie))
                            ->PlayAnimationByName(kFloat_0_GolfCaddie,
                                                  (void*)(golfCaddieStrings + kGolfCaddie_waitAnimation), 0, 0);
                    }
                } else if (AsCGolfDoc(GolfDoc_GetCurrent())->remainingTimeMs > kFortySecondsMs) {
                    AsCGolfCaddie(caddie)->animationState = kCaddieIdle;
                }
            }
        }
    }
    // Advance the active pet once, then rotate the optional ring and consider situational speech.
    void* target = *(void**)AsCGolfCaddie(caddie)->activeCaddie;
    ((PetUpdateInterface*)target)->vt->fn((PetUpdateInterface*)target, delta);
    if (AsCGolfCaddie(caddie)->texture3D &&
        ((CaddieAnimationMetadata*)AsCGolfCaddie(caddie)->activeCaddie)->ringEnabled)
        RingEffect_UpdateRotation(AsCGolfCaddie(caddie)->texture3D, delta);
    if (AsCGolfCaddie(caddie)->activeCaddie &&
        (*(unsigned int*)((CGolfDoc*)((char*)GolfDoc_GetCurrent()))->GetGameStateFlagsPtr() & ~3u) &&
        AsCGolfDoc(GolfDoc_GetCurrent())->tutorialStage >= 2 && !AsCGolfDoc(GolfDoc_GetCurrent())->shotPhase &&
        ((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[0] < 0)
        UpdateSituationalCommentary(delta);
}

#pragma opt_common_subs reset
#pragma opt_propagation reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma dont_inline reset
// Copy a key without ASCII spaces and return the cursor after its colon, or null at NUL.
// Leading spaces/LF are skipped; high-bit bytes copy a second byte verbatim (Shift-JIS path).
// keyOut must have room for the entire key and NUL; malformed input may leave it unterminated.
signed char* String_ParseKeyBeforeColon(void*, signed char* keyOut, signed char* cursor) {
    signed char c;
    signed char sc;
    while ((sc = *cursor) == ' ' || sc == '\n') {
        ++cursor;
    }

    c = sc;
    while (c != 0) {
        if (c == ' ') {
            ++cursor;
        } else {
            if (((unsigned char)c & 0x80) != 0) {
                *keyOut++ = *cursor++;
            }
            *keyOut++ = *cursor++;
            c = *cursor;
            if (c == ':') {
                *keyOut = 0;
                return cursor + 1;
            }
        }
        c = *cursor;
    }
    return 0;
}

} // extern "C"
