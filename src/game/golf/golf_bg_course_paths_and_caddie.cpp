// The course background actor: building the weather and rank archive paths,
// tearing it down, and constructing the caddie actor that shares its load
// helpers.
#include <decomp/static_init.h>
#include "MSL/vector.h"
// Vector destroyed by dtor_80126FC8; element type not known yet (Elem_80126FC8 is a tag);
// its size reset loads the size twice (form 2).
struct Elem_80126FC8;
// Pointer elements are borrowed; destroying an element releases no pointed-to object.
namespace std {
template <> struct __vec_destroys_elements<Elem_80126FC8*> {
    enum {
        value = 2
    };
    static void destroy(Elem_80126FC8**) {}
};
}
typedef std::vector<Elem_80126FC8*, app::app_allocator<Elem_80126FC8*> > Elem_80126FC8Vector;
#include "iff/IffStruct.h"
#include "classes/CGolfDoc.h"
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))

#include "sdk_protos.h"
#include "classes/CGolfCaddie.h"
#include "classes/CGolfBg.h"
#include "classes/CHashPath.h"

extern "C" {

// Views of course selection bytes currently covered by padding in the shared headers.
struct CourseResourceSelection {
    char reserved0[0x9C6];
    unsigned char weatherVariant;
    char reserved9C7[0x1D];
    char courseResourceName[0x20];
};
struct CourseHoleSelection {
    char reserved0[0x3B7];
    unsigned char holeNumber;
};

enum CoursePathConstants {
    kCourseWeatherVariantCount = 3,
    kSpecialCoursePartyMode = 2,
    kSharedPathGameMode = 6,
    kCaddieMatrixCount = 4,
    kAffineMatrixBytes = 0x30
};

struct CaddieLoadNode {
    CaddieLoadNode* left;
    CaddieLoadNode* right;
    char links[8];
    unsigned int nameFlags;
    char nameInline[4];
    void* heapName;
};
struct CaddieLoadTree {
    unsigned int count;
    CaddieLoadNode* root;
};
// Pending-load nodes own only heap-backed names (nameFlags bit 31). Child links may
// be null; the empty tree's iterator addresses its root slot rather than a heap sentinel.

extern unsigned char s_golfBgTypeDescriptor;
extern unsigned char s_actorTypeInfo;
extern const char golfBgActorFactoryName;
extern const char golfBgTaskClassName;
extern const float kFloat_6_2831855;
extern float s_twoPi_80125CAC_805E29D8;
extern const char golfBackgroundStrings[];
struct GolfBackgroundStringPool {
    char reserved0[0x25];
    char courseHoleVariantFormat[0xA]; // +0x25: "%s_%02d%s"
    char specialCourseSuffix[0x4];     // +0x2F: "_dc"
    char weatherSeparator[0x2];        // +0x33: "_"
    char listArchiveSuffix[0xA];       // +0x35: ".list.zip"
    char courseHoleFormat[0x8];        // +0x3F: "%s_%02d"
    char reserved47[0x8D];
    char stringFormat[0x3]; // +0xD4: "%s"
    char reservedD7[0x52];
    char courseRankFormat[0x8]; // +0x129: "%s_rank"
    char reserved131[0x90];
    char actorClassName[0x8]; // +0x1C1
};

// Return the golf background actor type descriptor.
void* CGolfBg::GetRTTI() {
    return &s_golfBgTypeDescriptor;
}

void* IntPair_Set(void*, const char*, void*);
void* ActorFactoryRegistry_GetInstance();
void StringMap_SetByName(void*, void*, const char*);
void CGolfBg_Create();
void* TaskRegistry_GetInstance();
void TaskRegistry_AppendFactory(void*, const char*);
void appFreeUni__FPv(void*);
void __dl__FPv(void*);
void __dt__18NativeBase80201D18Fv(void*, int);
char* String_FormatTemporary(const char*, ...);
void String_ReplaceRange(void*, int, int, const char*, const char*, void*);
void String_ReserveCapacity(void*, unsigned int);
void String_ReplaceRange(void*, int, int, const char*, const char*, void*);

void* SharedStringState_GetSecondString(int);
// Shared game-mode fields come from CSharedDoc; retain the signed view for party mode.
extern void* g_sharedDoc;
extern const char* s_courseWeatherResourceSuffixes[3];

struct PtrArray {
    void* data;
    int count;
};

// String ownership is the sign bit; inline strings use the remaining bytes of the record.
struct Item {
    unsigned int flags;
    unsigned int length;
    void* ptr;
};
struct ItemArray {
    Item* data;
    int count;
};

struct SmallString {
    unsigned int flags;
    union {
        unsigned char small[4];
        const char* large;
    } value;
};

struct StringLocal {
    unsigned int flags;
    unsigned char small[3];
    unsigned char pad7;
    const char* large;
    // Empty cleanup hook; this storage view performs no automatic buffer release.
    ~StringLocal() {}
};

struct OneByte {
    char c;
};

// Read character count; this branch must remain expanded at the call site for identical code.
#define STR_LEN(p) ((((p)->flags >> 31) == 0) ? (((unsigned char*)(p))[0] & 0x7f) : (p)->length)
// Borrow the narrow character data; do not retain it across replacement. An inline version
// preserves .text but changes unwind relocations, so retain the expression expansion.
#define STR_PTR(p) ((((p)->flags >> 31) == 0) ? ((char*)(p) + 1) : (p)->data)

#pragma opt_propagation off
// Build the course/hole/weather archive path, falling back when a special path is unavailable.
// path is an initialized destination string. Return 1 after appending the weather/archive
// suffix, 0 for an unsupported weather byte; even failure can replace path with a base path.
// Base "<course>_<hole:02>[_dc]" + "_" + weather suffix + ".list.zip".
// The _dc variant is accepted only when its completed archive resolves through CHashPath.
int CGolfBg::BuildWeatherResourcePath(PathString* path) {
    // Each allocator source/tag pair belongs to one expanded MSL string operation.
    // The allocator is stateless; retain separate bytes for the original ABI/unwind layout.
    char buf[0x100] = {};
    volatile OneByte candidateBaseAllocatorSource;
    OneByte candidateBaseAllocatorTag;
    OneByte candidateArchiveAllocatorTag;
    volatile OneByte candidateArchiveAllocatorSource;
    OneByte candidateWeatherAllocatorTag;
    volatile OneByte candidateWeatherAllocatorSource;
    OneByte candidateSeparatorAllocatorTag;
    volatile OneByte candidateSeparatorAllocatorSource;
    volatile OneByte outputBaseAllocatorSource;
    OneByte outputBaseAllocatorTag;
    OneByte outputArchiveAllocatorTag;
    volatile OneByte outputArchiveAllocatorSource;
    OneByte outputWeatherAllocatorTag;
    volatile OneByte outputWeatherAllocatorSource;
    OneByte outputSeparatorAllocatorTag;
    volatile OneByte outputSeparatorAllocatorSource;
    int n;
#define GS AsCSharedDoc(g_sharedDoc)
    // Phase 1: choose a special party base, an external mode-6 base, or the ordinary base.
    if ((*(int*)&AsCSharedDoc(GS)->partyGameMode) == kSpecialCoursePartyMode) {
        void* player = ((void*)AsCSharedDoc(GS)->golfDoc);
        sprintf(buf, ((GolfBackgroundStringPool*)golfBackgroundStrings)->courseHoleVariantFormat,
                ((CourseResourceSelection*)GS)->courseResourceName, ((CourseHoleSelection*)player)->holeNumber,
                ((GolfBackgroundStringPool*)golfBackgroundStrings)->specialCourseSuffix);
        PathString s;
        s.flags = 0;
        s.length = 0;
        s.data = 0;
        unsigned int len = strlen(buf);
        String_ReserveCapacity(&s, len);
        candidateBaseAllocatorTag.c = candidateBaseAllocatorSource.c;
        String_ReplaceRange(&s, 0, 0, buf, buf + len, &candidateBaseAllocatorTag);
        struct StringCleanup {
            void* object;
            // Release the temporary candidate path if it used heap storage.
            ~StringCleanup() {
                unsigned int* words = (unsigned int*)object;
                if (words[0] >> 31)
                    appFreeUni__FPv(*(char**)(words + 2));
            }
        } cleanup = {&s};
        int mode = ((CourseResourceSelection*)GS)->weatherVariant;
        int ok;
        if (mode < 0 || mode >= kCourseWeatherVariantCount) {
            ok = 0;
        } else {
            const char* part = ((GolfBackgroundStringPool*)golfBackgroundStrings)->weatherSeparator;
            n = STR_LEN(&s);
            candidateSeparatorAllocatorTag.c = candidateSeparatorAllocatorSource.c;
            unsigned int m = strlen(part);
            String_ReplaceRange(&s, n, 0, part, part + m, &candidateSeparatorAllocatorTag);
            part = s_courseWeatherResourceSuffixes[mode];
            n = STR_LEN(&s);
            candidateWeatherAllocatorTag.c = candidateWeatherAllocatorSource.c;
            m = strlen(part);
            String_ReplaceRange(&s, n, 0, part, part + m, &candidateWeatherAllocatorTag);
            part = ((GolfBackgroundStringPool*)golfBackgroundStrings)->listArchiveSuffix;
            n = STR_LEN(&s);
            candidateArchiveAllocatorTag.c = candidateArchiveAllocatorSource.c;
            m = strlen(part);
            String_ReplaceRange(&s, n, 0, part, part + m, &candidateArchiveAllocatorTag);
            ok = 1;
        }
        if (ok == 0 || CHashPath::FindPath((const char*)STR_PTR(&s)) == 0)
            buf[0] = 0;
    } else if (AsCSharedDoc(GS)->gameMode == kSharedPathGameMode) {
        void* result = SharedStringState_GetSecondString(0);
        if (result != 0) {
            result = SharedStringState_GetSecondString(0);
            sprintf(buf, ((GolfBackgroundStringPool*)golfBackgroundStrings)->stringFormat, result);
        }
    }
    if (strlen(buf) == 0) {
        sprintf(buf, ((GolfBackgroundStringPool*)golfBackgroundStrings)->courseHoleFormat,
                ((CourseResourceSelection*)GS)->courseResourceName,
                ((CourseHoleSelection*)AsCSharedDoc(GS)->golfDoc)->holeNumber);
    }
    // Phase 2: replace the output base, then append separator, weather and archive suffix.
    n = STR_LEN(path);
    outputBaseAllocatorTag.c = outputBaseAllocatorSource.c;
    unsigned int m = strlen(buf);
    String_ReplaceRange(path, 0, n, buf, buf + m, &outputBaseAllocatorTag);
    int mode = ((CourseResourceSelection*)GS)->weatherVariant;
    if (mode < 0 || mode >= kCourseWeatherVariantCount)
        return 0;
    const char* part = ((GolfBackgroundStringPool*)golfBackgroundStrings)->weatherSeparator;
    n = STR_LEN(path);
    outputSeparatorAllocatorTag.c = outputSeparatorAllocatorSource.c;
    m = strlen(part);
    String_ReplaceRange(path, n, 0, part, part + m, &outputSeparatorAllocatorTag);
    part = s_courseWeatherResourceSuffixes[mode];
    n = STR_LEN(path);
    outputWeatherAllocatorTag.c = outputWeatherAllocatorSource.c;
    m = strlen(part);
    String_ReplaceRange(path, n, 0, part, part + m, &outputWeatherAllocatorTag);
    part = ((GolfBackgroundStringPool*)golfBackgroundStrings)->listArchiveSuffix;
    n = STR_LEN(path);
    outputArchiveAllocatorTag.c = outputArchiveAllocatorSource.c;
    m = strlen(part);
    String_ReplaceRange(path, n, 0, part, part + m, &outputArchiveAllocatorTag);
    return 1;
}

// Clear and free a pointer-vector backing allocation and optionally its owner.
void* dtor_80126FC8(void* self, int deleting) {
    return ::std::__vector_deleting_dtor((Elem_80126FC8Vector*)self, deleting);
}

// Free the heap-backed strings in the list, then destroy the actor base.
CGolfBg::~CGolfBg() {
    // The four repeated address tests are how the inlined list teardown compiles here.
    if (((&strings.data != 0) && (&strings.data != 0)) && ((&strings.data != 0) && (&strings.data != 0))) {
        if (strings.data != 0) {
            Item* item;
            int count = strings.count;
            item = (Item*)strings.data + count;
            strings.count -= count;
            while (count != 0) {
                --item;
                if (item != 0 && (item->flags >> 31) != 0)
                    appFreeUni__FPv(item->ptr);
                --count;
            }
            appFreeUni__FPv(strings.data);
        }
    }

    __dt__18NativeBase80201D18Fv(this, 0);
}

#pragma opt_lifetimes off
// Build the selected course rank archive path for the current weather variant.
// path is an initialized destination. Return 0 for a missing formatted base or unsupported
// weather, 1 on completion. Unsupported weather leaves the newly written base in path.
int CGolfBg::BuildCourseRankListPath(PathString* path) {
    const char* s;
    int mode;
    unsigned int appendlen;
    char* first;
    volatile OneByte rankBaseAllocatorSource;
    OneByte rankBaseAllocatorTag;
    OneByte rankArchiveAllocatorTag;
    volatile OneByte rankArchiveAllocatorSource;
    OneByte rankWeatherAllocatorTag;
    volatile OneByte rankWeatherAllocatorSource;
    OneByte rankSeparatorAllocatorTag;
    volatile OneByte rankSeparatorAllocatorSource;
    first = String_FormatTemporary(((GolfBackgroundStringPool*)golfBackgroundStrings)->courseRankFormat,
                                   AsCSharedDoc(g_sharedDoc)->selectedCourseData != 0
                                       ? ((IFF_STRUCT::sCourse*)AsCSharedDoc(g_sharedDoc)->selectedCourseData)->mapName
                                       : 0);
    if (first == 0)
        return 0;
    unsigned int len = STR_LEN(path);
    rankBaseAllocatorTag.c = rankBaseAllocatorSource.c;
    unsigned int m = strlen(first);
    String_ReplaceRange(path, 0, len, first, first + m, &rankBaseAllocatorTag);
    mode = ((CourseResourceSelection*)g_sharedDoc)->weatherVariant;
    if (mode < 0 || mode >= kCourseWeatherVariantCount)
        return 0;
    s = ((GolfBackgroundStringPool*)golfBackgroundStrings)->weatherSeparator;
    appendlen = STR_LEN(path);
    rankSeparatorAllocatorTag.c = rankSeparatorAllocatorSource.c;
    m = strlen(s);
    String_ReplaceRange(path, appendlen, 0, s, s + m, &rankSeparatorAllocatorTag);
    s = s_courseWeatherResourceSuffixes[mode];
    appendlen = STR_LEN(path);
    rankWeatherAllocatorTag.c = rankWeatherAllocatorSource.c;
    m = strlen(s);
    String_ReplaceRange(path, appendlen, 0, s, s + m, &rankWeatherAllocatorTag);
    s = ((GolfBackgroundStringPool*)golfBackgroundStrings)->listArchiveSuffix;
    appendlen = STR_LEN(path);
    rankArchiveAllocatorTag.c = rankArchiveAllocatorSource.c;
    m = strlen(s);
    String_ReplaceRange(path, appendlen, 0, s, s + m, &rankArchiveAllocatorTag);
    return 1;
}

#pragma opt_lifetimes reset
// Clear and free pointer-vector storage and optionally its owner.
PtrArray* dtor_801263D8(PtrArray* self, int deleting) {
    if (self != 0) {
        if (self->data != 0) {
            self->count = *(volatile int*)&((PtrArray*)self)->count - *(volatile int*)&((PtrArray*)self)->count;
            appFreeUni__FPv(self->data);
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

// Clear string-array elements backwards, freeing only heap-backed strings.
// The backing array remains allocated; this function clears element ownership and size only.
void StringArray_ClearElements(ItemArray* self) {
    Item* end = self->data + self->count;
    int count = self->count;
    self->count -= count;
    while (count != 0) {
        --end;
        if (end != 0 && (end->flags >> 31) != 0)
            appFreeUni__FPv(end->ptr);
        --count;
    }
}

// Reset and free pointer-array storage and optionally its owner.
PtrArray* dtor_80126448(PtrArray* self, int deleting) {
    if (self != 0) {
        self->count = 0;
        if (self != 0) {
            if (self->data != 0) {
                self->count = 0;
                appFreeUni__FPv(self->data);
            }
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

// Register golf background actor/task factories and initialize the full-turn constant.
void CGolfBg_RegisterType() {
    s_twoPi_80125CAC_805E29D8 = kFloat_6_2831855;
    IntPair_Set(&s_golfBgTypeDescriptor, ((GolfBackgroundStringPool*)golfBackgroundStrings)->actorClassName,
                &s_actorTypeInfo);
    void* actorFactoryRegistry = ActorFactoryRegistry_GetInstance();
    StringMap_SetByName(actorFactoryRegistry, (void*)CGolfBg_Create, &golfBgActorFactoryName);
    TaskRegistry_AppendFactory(TaskRegistry_GetInstance(), &golfBgTaskClassName);
}

extern const unsigned char __vt__11CGolfCaddie[];
void __ct__18NativeBase80201D18Fv(void*);
void Matrix_SetIdentityAffine(void*);
void IntStringTree_DestroySubtree(void*, void*);

struct CleanupNode {
    void* left;
    void* right;
    char pad8[8];
    unsigned int flags;
    char pad14[4];
    void* owned;
};

// Free one node after recursively destroying each of its child subtrees.
static inline void FreeCaddieLoadNode(void* tree, CleanupNode* node) {
    if (node->left != 0)
        IntStringTree_DestroySubtree(tree, node->left);
    if (node->right != 0)
        IntStringTree_DestroySubtree(tree, node->right);
    if ((char*)node + (int)&((CaddieLoadNode*)0)->links[4] && (char*)node + (int)&((CaddieLoadNode*)0)->nameFlags &&
        (node->flags >> 31) != 0)
        appFreeUni__FPv(node->owned);
    appFreeUni__FPv(node);
}

// Destroy the caddie-load tree in postorder, including heap-backed node names.
void* IntStringTree_dtor(void* self, int deleting) {
    // Postorder cleanup is unrolled for three levels; deeper branches use the shared helper.
    // Preserve all six expanded object guards; only node nameFlags determines heap ownership.
    if (((self != 0) && (self != 0)) && (((self != 0) && (self != 0)) && ((self != 0) && (self != 0)))) {
        CleanupNode* root = (CleanupNode*)((CaddieLoadTree*)self)->root;
        if (root != 0) {
            CleanupNode* n0 = (CleanupNode*)root->left;
            if (n0 != 0) {
                CleanupNode* n1 = (CleanupNode*)n0->left;
                if (n1 != 0) {
                    CleanupNode* n2 = (CleanupNode*)n1->left;
                    if (n2 != 0)
                        FreeCaddieLoadNode(self, n2);
                    n2 = (CleanupNode*)n1->right;
                    if (n2 != 0)
                        FreeCaddieLoadNode(self, n2);
                    if ((char*)n1 + (int)&((CaddieLoadNode*)0)->links[4] &&
                        (char*)n1 + (int)&((CaddieLoadNode*)0)->nameFlags && ((n1->flags >> 31) != 0))
                        appFreeUni__FPv(n1->owned);
                    appFreeUni__FPv(n1);
                }
                n1 = (CleanupNode*)n0->right;
                if (n1 != 0) {
                    CleanupNode* n2 = (CleanupNode*)n1->left;
                    if (n2 != 0)
                        FreeCaddieLoadNode(self, n2);
                    n2 = (CleanupNode*)n1->right;
                    if (n2 != 0)
                        FreeCaddieLoadNode(self, n2);
                    if ((char*)n1 + (int)&((CaddieLoadNode*)0)->links[4] &&
                        (char*)n1 + (int)&((CaddieLoadNode*)0)->nameFlags && ((n1->flags >> 31) != 0))
                        appFreeUni__FPv(n1->owned);
                    appFreeUni__FPv(n1);
                }
                if ((char*)n0 + (int)&((CaddieLoadNode*)0)->links[4] &&
                    (char*)n0 + (int)&((CaddieLoadNode*)0)->nameFlags && ((n0->flags >> 31) != 0))
                    appFreeUni__FPv(n0->owned);
                appFreeUni__FPv(n0);
            }
            n0 = (CleanupNode*)root->right;
            if (n0 != 0) {
                CleanupNode* n1 = (CleanupNode*)n0->left;
                if (n1 != 0) {
                    CleanupNode* n2 = (CleanupNode*)n1->left;
                    if (n2 != 0)
                        FreeCaddieLoadNode(self, n2);
                    n2 = (CleanupNode*)n1->right;
                    if (n2 != 0)
                        FreeCaddieLoadNode(self, n2);
                    if ((char*)n1 + (int)&((CaddieLoadNode*)0)->links[4] &&
                        (char*)n1 + (int)&((CaddieLoadNode*)0)->nameFlags && ((n1->flags >> 31) != 0))
                        appFreeUni__FPv(n1->owned);
                    appFreeUni__FPv(n1);
                }
                n1 = (CleanupNode*)n0->right;
                if (n1 != 0) {
                    CleanupNode* n2 = (CleanupNode*)n1->left;
                    if (n2 != 0)
                        FreeCaddieLoadNode(self, n2);
                    n2 = (CleanupNode*)n1->right;
                    if (n2 != 0)
                        FreeCaddieLoadNode(self, n2);
                    if ((char*)n1 + (int)&((CaddieLoadNode*)0)->links[4] &&
                        (char*)n1 + (int)&((CaddieLoadNode*)0)->nameFlags && ((n1->flags >> 31) != 0))
                        appFreeUni__FPv(n1->owned);
                    appFreeUni__FPv(n1);
                }
                if ((char*)n0 + (int)&((CaddieLoadNode*)0)->links[4] &&
                    (char*)n0 + (int)&((CaddieLoadNode*)0)->nameFlags && ((n0->flags >> 31) != 0))
                    appFreeUni__FPv(n0->owned);
                appFreeUni__FPv(n0);
            }
            if ((char*)root + (int)&((CaddieLoadNode*)0)->links[4] &&
                (char*)root + (int)&((CaddieLoadNode*)0)->nameFlags && ((root->flags >> 31) != 0))
                appFreeUni__FPv(root->owned);
            appFreeUni__FPv(root);
        }
        if (deleting > 0)
            __dl__FPv(self);
    }

    return self;
}

void* IntStringTree_dtor(void*, int);
// Initialize the caddie's actor bases, clear its pending model loads and set four identity matrices.
CGolfCaddie::CGolfCaddie() {
    unsigned int emptyNameCount = 0;
    // Phase 1: initialize the actor bases and empty owning containers before cleanup paths.
    __ct__18NativeBase80201D18Fv(this);
    secondaryVtable = (void*)(__vt__11CGolfCaddie + 0x3c);
    caddieEntries = 0;
    vtbl = (void*)__vt__11CGolfCaddie;
    caddieEntryCount = 0;
    caddieEntryCapacity = 0;
    caddieVisible = 1;
    texture3D = 0;
    soundBankNames = 0;
    soundBankNameCapacity = 0;
    conteCaddies = 0;
    storyConteCaddieOverride = 0; // Story-conte caddie override, filled later by its actor reply.
    pendingModelLoadCount = 0;
    pendingModelLoadTreeRoot = 0;
    pendingModelLoadIterator = ((char*)&pendingModelLoadTreeRoot);
    petPreload = 0;
    petPreloadPlayerIndex = -1;
    petPreloadActive = 0;
    soundBankNameCount = 0;

    // Phase 2: retain the original empty-range/string and pending-tree cleanup expansions.
    // Both ranges were just reset; the repeated code reflects inlined container operations.
    char* nameCursor = (char*)soundBankNames + emptyNameCount * 0xc;
    int remainingNames = emptyNameCount;
    while (remainingNames != 0) {
        nameCursor -= 0xc;
        if (0 != nameCursor) {

            if ((*(unsigned int*)nameCursor) >> 31)
                appFreeUni__FPv(*(void**)(nameCursor + 8));
        }
        remainingNames = remainingNames - 1;
    }
    void* root = pendingModelLoadTreeRoot;
    if (0 != root) {
        void* member = ((char*)&pendingModelLoadCount);
        void* n0 = ((CaddieLoadNode*)root)->left;
        if (0 != n0) {
            void* n1 = ((CaddieLoadNode*)n0)->left;
            if (n1) {
                void* n2 = ((CaddieLoadNode*)n1)->left;
                if (n2) {
                    if (((CaddieLoadNode*)n2)->left != 0)
                        IntStringTree_DestroySubtree(member, ((CaddieLoadNode*)n2)->left);
                    if (((CaddieLoadNode*)n2)->right != 0)
                        IntStringTree_DestroySubtree(member, ((CaddieLoadNode*)n2)->right);
                    if ((char*)n2 + (int)&((CaddieLoadNode*)0)->links[4] &&
                        (char*)n2 + (int)&((CaddieLoadNode*)0)->nameFlags &&
                        ((*(unsigned int*)((char*)n2 + (int)&((CaddieLoadNode*)0)->nameFlags) >> 31) != 0))
                        appFreeUni__FPv(((CaddieLoadNode*)n2)->heapName);
                    appFreeUni__FPv(n2);
                }
                n2 = ((CaddieLoadNode*)n1)->right;
                if (n2 != 0) {
                    if (((CaddieLoadNode*)n2)->left != 0)
                        IntStringTree_DestroySubtree(member, ((CaddieLoadNode*)n2)->left);
                    if (((CaddieLoadNode*)n2)->right != 0)
                        IntStringTree_DestroySubtree(member, ((CaddieLoadNode*)n2)->right);
                    if ((char*)n2 + (int)&((CaddieLoadNode*)0)->links[4] &&
                        (char*)n2 + (int)&((CaddieLoadNode*)0)->nameFlags &&
                        ((*(unsigned int*)((char*)n2 + (int)&((CaddieLoadNode*)0)->nameFlags) >> 31) != 0))
                        appFreeUni__FPv(((CaddieLoadNode*)n2)->heapName);
                    appFreeUni__FPv(n2);
                }
                if ((char*)n1 + (int)&((CaddieLoadNode*)0)->links[4] &&
                    (char*)n1 + (int)&((CaddieLoadNode*)0)->nameFlags &&
                    ((*(unsigned int*)((char*)n1 + (int)&((CaddieLoadNode*)0)->nameFlags) >> 31) != 0))
                    appFreeUni__FPv(((CaddieLoadNode*)n1)->heapName);
                appFreeUni__FPv(n1);
            }
            n1 = ((CaddieLoadNode*)n0)->right;
            if (n1 != 0) {
                void* n2 = ((CaddieLoadNode*)n1)->left;
                if (n2 != 0) {
                    if (((CaddieLoadNode*)n2)->left != 0)
                        IntStringTree_DestroySubtree(member, ((CaddieLoadNode*)n2)->left);
                    if (((CaddieLoadNode*)n2)->right != 0)
                        IntStringTree_DestroySubtree(member, ((CaddieLoadNode*)n2)->right);
                    if ((char*)n2 + (int)&((CaddieLoadNode*)0)->links[4] &&
                        (char*)n2 + (int)&((CaddieLoadNode*)0)->nameFlags &&
                        ((*(unsigned int*)((char*)n2 + (int)&((CaddieLoadNode*)0)->nameFlags) >> 31) != 0))
                        appFreeUni__FPv(((CaddieLoadNode*)n2)->heapName);
                    appFreeUni__FPv(n2);
                }
                n2 = ((CaddieLoadNode*)n1)->right;
                if (0 != n2) {
                    if (((CaddieLoadNode*)n2)->left != 0)
                        IntStringTree_DestroySubtree(member, ((CaddieLoadNode*)n2)->left);
                    if (((CaddieLoadNode*)n2)->right != 0)
                        IntStringTree_DestroySubtree(member, ((CaddieLoadNode*)n2)->right);
                    if ((char*)n2 + (int)&((CaddieLoadNode*)0)->links[4] &&
                        (char*)n2 + (int)&((CaddieLoadNode*)0)->nameFlags &&
                        ((*(unsigned int*)((char*)n2 + (int)&((CaddieLoadNode*)0)->nameFlags) >> 31) != 0))
                        appFreeUni__FPv(((CaddieLoadNode*)n2)->heapName);
                    appFreeUni__FPv(n2);
                }
                if ((char*)n1 + (int)&((CaddieLoadNode*)0)->links[4] &&
                    (char*)n1 + (int)&((CaddieLoadNode*)0)->nameFlags &&
                    ((*(unsigned int*)((char*)n1 + (int)&((CaddieLoadNode*)0)->nameFlags) >> 31) != 0))
                    appFreeUni__FPv(((CaddieLoadNode*)n1)->heapName);
                appFreeUni__FPv(n1);
            }
            if ((char*)n0 + (int)&((CaddieLoadNode*)0)->links[4] && (char*)n0 + (int)&((CaddieLoadNode*)0)->nameFlags &&
                ((*(unsigned int*)((char*)n0 + (int)&((CaddieLoadNode*)0)->nameFlags) >> 31) != 0))
                appFreeUni__FPv(((CaddieLoadNode*)n0)->heapName);
            appFreeUni__FPv(n0);
        }
        void* n0b = ((CaddieLoadNode*)root)->right;
        if (n0b != 0) {
            void* n1 = ((CaddieLoadNode*)n0b)->left;
            if (n1) {
                void* n2 = ((CaddieLoadNode*)n1)->left;
                if (0 != n2) {
                    if (((CaddieLoadNode*)n2)->left != 0)
                        IntStringTree_DestroySubtree(member, ((CaddieLoadNode*)n2)->left);
                    if (((CaddieLoadNode*)n2)->right != 0)
                        IntStringTree_DestroySubtree(member, ((CaddieLoadNode*)n2)->right);
                    if ((char*)n2 + (int)&((CaddieLoadNode*)0)->links[4] &&
                        (char*)n2 + (int)&((CaddieLoadNode*)0)->nameFlags &&
                        ((*(unsigned int*)((char*)n2 + (int)&((CaddieLoadNode*)0)->nameFlags) >> 31) != 0))
                        appFreeUni__FPv(((CaddieLoadNode*)n2)->heapName);
                    appFreeUni__FPv(n2);
                }
                n2 = ((CaddieLoadNode*)n1)->right;
                if (n2 != 0) {
                    if (((CaddieLoadNode*)n2)->left != 0)
                        IntStringTree_DestroySubtree(member, ((CaddieLoadNode*)n2)->left);
                    if (((CaddieLoadNode*)n2)->right != 0)
                        IntStringTree_DestroySubtree(member, ((CaddieLoadNode*)n2)->right);
                    if ((char*)n2 + (int)&((CaddieLoadNode*)0)->links[4] &&
                        (char*)n2 + (int)&((CaddieLoadNode*)0)->nameFlags &&
                        ((*(unsigned int*)((char*)n2 + (int)&((CaddieLoadNode*)0)->nameFlags) >> 31) != 0))
                        appFreeUni__FPv(((CaddieLoadNode*)n2)->heapName);
                    appFreeUni__FPv(n2);
                }
                if ((char*)n1 + (int)&((CaddieLoadNode*)0)->links[4] &&
                    (char*)n1 + (int)&((CaddieLoadNode*)0)->nameFlags &&
                    ((*(unsigned int*)((char*)n1 + (int)&((CaddieLoadNode*)0)->nameFlags) >> 31) != 0))
                    appFreeUni__FPv(((CaddieLoadNode*)n1)->heapName);
                appFreeUni__FPv(n1);
            }
            n1 = ((CaddieLoadNode*)n0b)->right;
            if (n1 != 0) {
                void* n2 = ((CaddieLoadNode*)n1)->left;
                if (n2 != 0) {
                    if (((CaddieLoadNode*)n2)->left != 0)
                        IntStringTree_DestroySubtree(member, ((CaddieLoadNode*)n2)->left);
                    if (((CaddieLoadNode*)n2)->right != 0)
                        IntStringTree_DestroySubtree(member, ((CaddieLoadNode*)n2)->right);
                    if ((char*)n2 + (int)&((CaddieLoadNode*)0)->links[4] &&
                        (char*)n2 + (int)&((CaddieLoadNode*)0)->nameFlags &&
                        ((*(unsigned int*)((char*)n2 + (int)&((CaddieLoadNode*)0)->nameFlags) >> 31) != 0))
                        appFreeUni__FPv(((CaddieLoadNode*)n2)->heapName);
                    appFreeUni__FPv(n2);
                }
                n2 = ((CaddieLoadNode*)n1)->right;
                if (0 != n2) {
                    if (((CaddieLoadNode*)n2)->left != 0)
                        IntStringTree_DestroySubtree(member, ((CaddieLoadNode*)n2)->left);
                    if (((CaddieLoadNode*)n2)->right != 0)
                        IntStringTree_DestroySubtree(member, ((CaddieLoadNode*)n2)->right);
                    if ((char*)n2 + (int)&((CaddieLoadNode*)0)->links[4] &&
                        (char*)n2 + (int)&((CaddieLoadNode*)0)->nameFlags &&
                        ((*(unsigned int*)((char*)n2 + (int)&((CaddieLoadNode*)0)->nameFlags) >> 31) != 0))
                        appFreeUni__FPv(((CaddieLoadNode*)n2)->heapName);
                    appFreeUni__FPv(n2);
                }
                if ((char*)n1 + (int)&((CaddieLoadNode*)0)->links[4] &&
                    (char*)n1 + (int)&((CaddieLoadNode*)0)->nameFlags &&
                    ((*(unsigned int*)((char*)n1 + (int)&((CaddieLoadNode*)0)->nameFlags) >> 31) != 0))
                    appFreeUni__FPv(((CaddieLoadNode*)n1)->heapName);
                appFreeUni__FPv(n1);
            }
            if ((char*)n0b + (int)&((CaddieLoadNode*)0)->links[4] &&
                (char*)n0b + (int)&((CaddieLoadNode*)0)->nameFlags &&
                ((*(unsigned int*)((char*)n0b + (int)&((CaddieLoadNode*)0)->nameFlags) >> 31) != 0))
                appFreeUni__FPv(((CaddieLoadNode*)n0b)->heapName);
            appFreeUni__FPv(n0b);
        }
        if ((char*)root + (int)&((CaddieLoadNode*)0)->links[4] && (char*)root + (int)&((CaddieLoadNode*)0)->nameFlags &&
            ((*(unsigned int*)((char*)root + (int)&((CaddieLoadNode*)0)->nameFlags) >> 31) != 0))
            appFreeUni__FPv(((CaddieLoadNode*)root)->heapName);
        appFreeUni__FPv(root);
        pendingModelLoadTreeRoot = 0;
        pendingModelLoadIterator = ((char*)&pendingModelLoadTreeRoot);
        pendingModelLoadCount = 0;
    }
    // Phase 3: publish four identity transforms and mark the effect entry unregistered.
    // Four consecutive affine matrices fill +0x88..+0x148.
    for (int i = 0; kCaddieMatrixCount > i; ++i)
        Matrix_SetIdentityAffine((char*)affineMatrices + i * kAffineMatrixBytes);
    effectEntryKey = -1;
}

// Allocate and construct the caddie actor.
CGolfCaddie* CGolfCaddie_Create() {
    return new CGolfCaddie;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462120", _ctors_u80125CAC_0, CGolfBg_RegisterType);
#pragma pop
