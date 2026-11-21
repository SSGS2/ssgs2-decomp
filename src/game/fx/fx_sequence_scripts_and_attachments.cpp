#include <decomp/static_init.h>
// Effect sequence scripts and model-bone attachments (.seq/.spr).
// Mod lookup: FxAttachments_CreateFromResourceTags binds named-bone tag filenames;
// CFxSequence_LoadByName loads/caches command templates; FxCommandList_Assign
// makes independent command nodes; FxInterpreter_RemoveEntriesByKey/ClearEntries
// release tracked effects. Layouts: CFxSequence.h, FxInterpreter.h, WPuppet.h;
// token/extension data: phase1_program_strings.cpp, parser: fx_box_interpreter_resources.cpp.
// These APIs mutate owner/interpreter containers without locking: serialize access
// with playback and teardown. No frame-rate or distance conversion occurs here.
#pragma ipa file
#include "game/string.h"
#pragma auto_inline off
#include "sdk_protos.h"
#include "classes/CFxSequence.h"
#include "classes/FxInterpreter.h"
#include "classes/WPuppet.h"
#include "classes/CFx.h"
// FxInterpreter_ClearEntries (0x8010CA88): existing local list/link fields replace +4/+8.
// FxSequenceCache_FindByName (0x8010D4B4): existing string size/data replace +4/+8.

#define AsCFxSequence(p) ((CFxSequence*)(p))
// Translation unit 0x8010C720-0x8010DCF0 (17 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "MSL/list.h"
// Scalar list ABI used only for node cleanup; actual command payloads are
// described by Fn8010MergeNode::Value below, not by this void* placeholder.
typedef std::list<void*, app::app_allocator<void*> > FxCommandList;
extern "C" {

extern const float kFloat_6_2831855;
extern float s_fxSequenceRotationPeriod;
extern const float kFloat_0_SoundEmitter;
extern const float kFloat_neg1_FxBox;
extern CFx* g_fx;

void appFreeUni__FPv(void*);
void CFxBox_ClearEffects(void*, int);
void FxSequenceTree_FreeSubtree(void*, void*);
void CFx_RemoveSpray(void*, void*);
void CFx_RemoveSequence(void*, void*, int);
void Vec3_Zero(void*);
void* __as__7WVectorFRC7WVector(void*, void*);
void Matrix_SetIdentityAffine(void*);
void* memset(void*, int, unsigned int);
void __dl__FPv(void*);
void OSReport(const char*, ...);
void MWString_ConstructCString(void*, const char*);

void ResourceHandle_dtor(void*, int);
void* FxCommandList_Assign(void*, void*);
void CFxSequence_ParseEntryLine(void*, char*, unsigned int);
void* FxSequenceNameTree_GetOrInsertValue(void*, void*);
void* FxSequenceCache_GetIteratorValue(void*);
void CFxSequence_RandomizeAndSortCommandTimes(void*);
int EffectCommand_ParseResourceFilename(void*, void**, void*, unsigned int, int*);
void FxAttachment_Init(void*);
void* CFx_CreateAttachedEffect(void*, void*, void*, int, int, int);
void* appMallocDummy__FUl(unsigned int);
void* Resource_OpenResolvedPath(void*, const char*, int, int, unsigned char, int);
int Stream_ScanFormat(void*, const char*, ...);
void Resource_Release(void*);
void* __nw__FUl(unsigned int);
void* CFxSequence_ctor(void*, void*, unsigned char);
// Packed view of the existing string storage; member lengths include terminators.
struct FxParserStringPool {
    char debugSequenceName[11];
    char emptyMessage[1];
    char scanLineFormat[3];
};
#define FxParserStringPoolView (*(FxParserStringPool*)fxSequenceParserStrings)
extern const char fxSequenceParserStrings[];
extern char fxConditionCommandTokens[];

// Byte-address expressions preserve the compiler aliasing/lifetime behavior of embedded objects.
enum SequenceStorageOffset {
    kLocalPositionOffset = 8, kSprayTreeOffset = 0x20, kSprayRootOffset = 0x24,
    kAttachmentSlotsOffset = 0x2c, kAttachmentFlagsOffset = 0x3c,
    kSequenceMatrixOffset = 0x4c, kWorldPositionOffset = 0x84, kPositionOffsetOffset = 0x90,
    kCommandListOffset = 0x1c0, kCommandSentinelOffset = 0x1c4, kNextCommandOffset = 0x1cc,
    kOwnerCacheOffset = 0x134, kOwnerResourcesOffset = 0x140, kTreeRightOffset = 4, kEmptyDebugMessageOffset = 0xb, kScanLineFormatOffset = 0xc
};

enum EffectEntryType { kSequenceEntry = 1, kSprayEntry = 2 };
enum { kBoneSuppressAttachedEffects = 0x40000, kSequenceResourceOpenFlags = 0x2000 };

// The command/attachment lists embed their sentinel; it is never freed as a node.
// Fn8010Link, Fn8010MergeNode and Fn8010EntryNode name +0 previousLink and +4 nextLink
// (forward traversal runs through +4). Fn8010Item names those same offsets prev/next.
// CFxSequence::firstCommandNode aliases sentinel+4: the FIRST command node.
// Command nodes own fixed-size value copies. Attachment nodes track effect pointers:
// removal asks g_fx to destroy the effect, then frees the list node separately.
// The owner name tree owns cached CFxSequence templates; list assignment copies
// command values rather than sharing the cached list's nodes.
struct Fn8010Link {
    Fn8010Link* previousLink;
    Fn8010Link* nextLink;
};

struct Fn8010List {
    unsigned int reserved;
    unsigned int count;
    Fn8010Link sentinel;
    Fn8010Link* end;
};


// List header used by the standalone list destructors.
struct FxCommandListView { unsigned int count; Fn8010Link sentinel; };

struct Fn8010Item { Fn8010Item* prev; Fn8010Item* next; int key; int type; void* value; };

#pragma push
#pragma use_lmw_stmw on
// Remove the ordered entries for one attachment key and release their live effects.
void FxInterpreter_RemoveEntriesByKey(Fn8010List* self, int key) {
    if (key < 0) return;
    Fn8010Link* sentinel = &self->sentinel;
    Fn8010Item* node = (Fn8010Item*)sentinel->nextLink;
    while ((Fn8010Link*)node != sentinel) {
            if (key == node->key) {
                Fn8010Item* cur = node;
                node = (Fn8010Item*)node->next;
                if (g_fx) {
                    if (cur->type == kSprayEntry) CFx_RemoveSpray((void*)g_fx, cur->value);
                    else if (cur->type == kSequenceEntry) CFx_RemoveSequence((void*)g_fx, cur->value, 1);
                }
                cur->prev->next = cur->next;
                cur->next->prev = cur->prev;
                appFreeUni__FPv(cur);
                --self->count;
            } else {
                if (node->key > key) break;
                node = (Fn8010Item*)node->next;
            }
        }
}
#pragma pop

// Release each tracked effect, unlink the entire entry list, and free its nodes.
void FxInterpreter_ClearEntries(Fn8010List* self) {
    Fn8010Link* sentinel = &self->sentinel;
    Fn8010Item* node = (Fn8010Item*)sentinel->nextLink;
    while ((Fn8010Link*)node != sentinel) {
        if (g_fx) {
            if (node->type == kSprayEntry) CFx_RemoveSpray((void*)g_fx, node->value);
            else if (node->type == kSequenceEntry) CFx_RemoveSequence((void*)g_fx, node->value, 1);
        }
        node = node->next;
    }
    Fn8010Link* end;
    Fn8010Link* first;
    first = self->sentinel.nextLink;
    end = &self->sentinel;
    if (first != end) {
        Fn8010Link* target = *(Fn8010Link**)first;
        Fn8010Link* entry = *(Fn8010Link**)end;
        target->nextLink = entry->nextLink;
        *(Fn8010Link**)((char*)entry->nextLink) = *(Fn8010Link**)first;
        while (first != end) {
            Fn8010Link* cur = first;
            first = first->nextLink;
            appFreeUni__FPv(cur);
            --self->count;
        }
    }
}

// Initialize transforms, spray ownership, playback timing, and the command-list sentinel.
void* CFxSequence_ctor(void* mem, void* sequenceOwner, unsigned char isCachedTemplate) {
    Fn8010Link* node;
    unsigned char cachedTemplate = isCachedTemplate;
    char* self = (char*)mem;
    Fn8010Link* sentinel = (Fn8010Link*)(self + kCommandSentinelOffset);
    extern char __vt__11CFxSequence[];
    AsCFxSequence(self)->vtbl = __vt__11CFxSequence;
    AsCFxSequence(self)->sprayCount = 0;
    AsCFxSequence(self)->sprayTreeRoot = 0;
    (*(Fn8010Link**)&AsCFxSequence(self)->firstSprayNode) = (Fn8010Link*)(self + kSprayRootOffset);
    AsCFxSequence(self)->sequenceOwner = sequenceOwner;
    AsCFxSequence(self)->commandCount = 0;
    sentinel->nextLink = sentinel;
    sentinel->previousLink = sentinel;
    (*(unsigned int*)&AsCFxSequence(self)->effectBox) = 0;
    AsCFxSequence(self)->unk_4 = 1;
    Vec3_Zero(self + kLocalPositionOffset);
    AsCFxSequence(self)->playbackEnabled = 1;
    float zero[3];
    float init = kFloat_0_SoundEmitter;
    zero[0] = init;
    zero[1] = init;
    zero[2] = init;
    __as__7WVectorFRC7WVector(self + kWorldPositionOffset, __as__7WVectorFRC7WVector(self + kPositionOffsetOffset, zero));
    float neg;
    float z = kFloat_0_SoundEmitter;
    neg = kFloat_neg1_FxBox;
    AsCFxSequence(self)->deleteWhenRemovedFromBox = cachedTemplate;
    AsCFxSequence(self)->elapsedTime = z;
    AsCFxSequence(self)->resumeDelay = z;
    AsCFxSequence(self)->attachmentPositionX = neg;
    AsCFxSequence(self)->attachmentPositionY = neg;
    Matrix_SetIdentityAffine(self + kSequenceMatrixOffset);
    memset(self + kAttachmentSlotsOffset, 0, 0x10);
    memset(self + kAttachmentFlagsOffset, 0, 0x10);
    node = (*(Fn8010Link**)&AsCFxSequence(self)->firstCommandNode);
    sentinel = (Fn8010Link*)(self + kCommandSentinelOffset);
    AsCFxSequence(self)->bypassResourceCache = 0;
    AsCFxSequence(self)->attachment = 0;
    if (node != sentinel) {
        Fn8010Link* entry = sentinel->previousLink;
        node->previousLink->nextLink = entry->nextLink;
        entry->nextLink->previousLink = node->previousLink;
        while (node != sentinel) {
            Fn8010Link* cur = node;
            node = node->nextLink;
            appFreeUni__FPv(cur);
            --AsCFxSequence(self)->commandCount;
        }
    }
    (*(Fn8010Link**)&AsCFxSequence(self)->nextCommand) = (Fn8010Link*)(self + kCommandSentinelOffset);
    AsCFxSequence(self)->failedCommandText = 0;
    AsCFxSequence(self)->currentSpray = 0;
    return self;
}

// Discard parsed commands and reset attachment and spray playback cursors.
void CFxSequence_ClearCommands(char* self) {
    AsCFxSequence(self)->attachment = 0;
    Fn8010Link* sentinel = (Fn8010Link*)(self + kCommandSentinelOffset);
    Fn8010Link* node = (*(Fn8010Link**)&AsCFxSequence(self)->firstCommandNode);
    if (node != sentinel) {
        Fn8010Link* sn = sentinel->previousLink;
        node->previousLink->nextLink = sn->nextLink;
        sn->nextLink->previousLink = node->previousLink;
        while (node != sentinel) {
            Fn8010Link* cur = node;
            node = node->nextLink;
            appFreeUni__FPv(cur);
            --AsCFxSequence(self)->commandCount;
        }
    }
        (*(Fn8010Link**)&AsCFxSequence(self)->nextCommand) = (Fn8010Link*)(self + kCommandSentinelOffset);
    AsCFxSequence(self)->failedCommandText = 0;
    AsCFxSequence(self)->currentSpray = 0;
}

// Unlink and free all command-list nodes, then optionally delete the list.
void* FxCommandList_dtor(void* self, int shouldDelete) {
    if (self) {
        Fn8010Link* sentinel = &((FxCommandListView*)self)->sentinel;
        Fn8010Link* node = sentinel->nextLink;
        if (node != sentinel) {
            Fn8010Link* tail = sentinel->previousLink;
            node->previousLink->nextLink = tail->nextLink;
            tail->nextLink->previousLink = node->previousLink;
            while (node != sentinel) {
                Fn8010Link* cur = node;
                node = node->nextLink;
                appFreeUni__FPv(cur);
                --*(unsigned int*)self;
            }
        }
        if (0 < shouldDelete)
            __dl__FPv(self);
    }
    return self;
}

// Destroy the nested command-list storage and optionally delete its owner.
void* dtor_8010CD94(void* self, int shouldDelete) {
    return ::std::__list_deleting_dtor((FxCommandList*)self, shouldDelete);
}

#pragma push
#pragma use_lmw_stmw on
struct Fn8010CE50Object {
    virtual void destroy(int);
};
// Release the effect box, command list, and spray tree before optional deletion.
void* CFxSequence_dtor(char* self, int deleteFlag) {
    if (self) {
    void* top;
    void* old;
    extern char __vt__11CFxSequence[];
    *(void**)self = __vt__11CFxSequence;
    old = AsCFxSequence(self)->effectBox;
    if (old) {
        CFxBox_ClearEffects(old, 0);
        old = AsCFxSequence(self)->effectBox;
        if (old) {
            if (old) {
            ((Fn8010CE50Object*)old)->destroy(1);
            }
            AsCFxSequence(self)->effectBox = 0;
        }
    }
    if (self + kCommandListOffset) {
    if (self + kCommandListOffset) {
    if (self + kCommandListOffset) {
    if (self + kCommandListOffset) {
    Fn8010Link* sentinel;
    Fn8010Link* node;
    node = (*(Fn8010Link**)&AsCFxSequence(self)->firstCommandNode);
    sentinel = (Fn8010Link*)(self + kCommandSentinelOffset);
    if (node != sentinel) {
        Fn8010Link* first = sentinel->previousLink;
        node->previousLink->nextLink = first->nextLink;
        first->nextLink->previousLink = node->previousLink;
        while (node != sentinel) {
            Fn8010Link* cur = node;
            node = node->nextLink;
            appFreeUni__FPv(cur);
            --*(unsigned int*)(self + kCommandListOffset);
        }
    }
    } } } }
    if (self + kSprayTreeOffset) {
    if (self + kSprayTreeOffset) {
    if (self + kSprayTreeOffset) {
    if (self + kSprayTreeOffset) {
    if (self + kSprayTreeOffset) {
    if (self + kSprayTreeOffset) {
        top = (*(void**)&AsCFxSequence(self)->sprayTreeRoot);
        if (top) {
            { void* a = *(void**)top;
            if (a) {
                { void* b = *(void**)a;
                if (b) {
                    { void* c = *(void**)b;
                    if (c) {
                        if (*(void**)c) FxSequenceTree_FreeSubtree(self + kSprayTreeOffset, *(void**)c);
                        if (*(void**)((char*)c + kTreeRightOffset)) FxSequenceTree_FreeSubtree(self + kSprayTreeOffset, *(void**)((char*)c + kTreeRightOffset));
                        appFreeUni__FPv(c);
                    }
                    } { void* c = *(void**)((char*)b + kTreeRightOffset);
                    if (c) {
                        if (*(void**)c) FxSequenceTree_FreeSubtree(self + kSprayTreeOffset, *(void**)c);
                        if (*(void**)((char*)c + kTreeRightOffset)) FxSequenceTree_FreeSubtree(self + kSprayTreeOffset, *(void**)((char*)c + kTreeRightOffset));
                        appFreeUni__FPv(c);
                    }
                    } appFreeUni__FPv(b);
                }
                } { void* b = *(void**)((char*)a + kTreeRightOffset);
                if (b) {
                    { void* c = *(void**)b;
                    if (c) {
                        if (*(void**)c) FxSequenceTree_FreeSubtree(self + kSprayTreeOffset, *(void**)c);
                        if (*(void**)((char*)c + kTreeRightOffset)) FxSequenceTree_FreeSubtree(self + kSprayTreeOffset, *(void**)((char*)c + kTreeRightOffset));
                        appFreeUni__FPv(c);
                    }
                    } { void* c = *(void**)((char*)b + kTreeRightOffset);
                    if (c) {
                        if (*(void**)c) FxSequenceTree_FreeSubtree(self + kSprayTreeOffset, *(void**)c);
                        if (*(void**)((char*)c + kTreeRightOffset)) FxSequenceTree_FreeSubtree(self + kSprayTreeOffset, *(void**)((char*)c + kTreeRightOffset));
                        appFreeUni__FPv(c);
                    }
                    } appFreeUni__FPv(b);
                }
                } appFreeUni__FPv(a);
            }
            } { void* root = *(void**)((char*)top + kTreeRightOffset);
            if (root) {
                { void* a = *(void**)root;
                if (a) {
                    { void* b = *(void**)a;
                    if (b) {
                        if (*(void**)b) FxSequenceTree_FreeSubtree(self + kSprayTreeOffset, *(void**)b);
                        if (*(void**)((char*)b + kTreeRightOffset)) FxSequenceTree_FreeSubtree(self + kSprayTreeOffset, *(void**)((char*)b + kTreeRightOffset));
                        appFreeUni__FPv(b);
                    }
                    } { void* b = *(void**)((char*)a + kTreeRightOffset);
                    if (b) {
                        if (*(void**)b) FxSequenceTree_FreeSubtree(self + kSprayTreeOffset, *(void**)b);
                        if (*(void**)((char*)b + kTreeRightOffset)) FxSequenceTree_FreeSubtree(self + kSprayTreeOffset, *(void**)((char*)b + kTreeRightOffset));
                        appFreeUni__FPv(b);
                    }
                    } appFreeUni__FPv(a);
                }
                } { void* a = *(void**)((char*)root + kTreeRightOffset);
                if (a) {
                    { void* b = *(void**)a;
                    if (b) {
                        if (*(void**)b) FxSequenceTree_FreeSubtree(self + kSprayTreeOffset, *(void**)b);
                        if (*(void**)((char*)b + kTreeRightOffset)) FxSequenceTree_FreeSubtree(self + kSprayTreeOffset, *(void**)((char*)b + kTreeRightOffset));
                        appFreeUni__FPv(b);
                    }
                    } { void* b = *(void**)((char*)a + kTreeRightOffset);
                    if (b) {
                        if (*(void**)b) FxSequenceTree_FreeSubtree(self + kSprayTreeOffset, *(void**)b);
                        if (*(void**)((char*)b + kTreeRightOffset)) FxSequenceTree_FreeSubtree(self + kSprayTreeOffset, *(void**)((char*)b + kTreeRightOffset));
                        appFreeUni__FPv(b);
                    }
                    } appFreeUni__FPv(a);
                }
                } appFreeUni__FPv(root);
            }
            } appFreeUni__FPv(top);
        }
    }
    } } } } }
    if (deleteFlag > 0) __dl__FPv(self);
    }
    return self;
}
#pragma pop

// Store a resource-holder pointer in the caller-provided word.
void ResourceHolder_SetResourceAddress(unsigned int* holder, unsigned int resourceAddress) {
    *holder = resourceAddress;
}

// Initialize the effect-sequence rotation-period value.
void FxSequence_InitRotationPeriod() {
    s_fxSequenceRotationPeriod = kFloat_6_2831855;
}

struct Fn8010CD88Arg {
    char reserved[8];
    unsigned int value;
};

// Copy the list head cursor from the embedded list header.
void FxCommandList_GetFirstCursor(unsigned int* outCursor, Fn8010CD88Arg* listHeader) {
    *outCursor = listHeader->value;
}

// Copy one iterator word.
void FxWordIterator_Copy(unsigned int* destination, unsigned int* source) {
    *destination = *source;
}

// Return the sentinel address immediately after the list count.
void FxCommandList_GetEndCursor(unsigned int** outSentinel, unsigned int* listHeader) {
    *outSentinel = listHeader + 1;
}

// Compare two iterator words.
int FxWordIterator_Equals(unsigned int* left, unsigned int* right) {
    return *left == *right;
}

struct Fn8010MergeList {
    unsigned int count;
    Fn8010Link sentinel;
    // Return the number of command-list nodes.
    unsigned int size() const { return count; }
};

} // extern "C"

struct Fn8010MergeNode {
    Fn8010MergeNode* previousLink;
    Fn8010MergeNode* nextLink;
    struct Value {
        unsigned int a, b, c, d;
        unsigned char bytes[0x104];
        // Use the caller-provided placement address without allocating.
        static void* operator new(size_t, void* p) { return p; }
        // Copy the complete fixed-size command payload without interpreting its bytes.
        Value(const Value& other) {
            struct Raw { unsigned int words[0x45]; };
            try {
                *(Raw*)this = *(const Raw*)&other;
            } catch (...) {
                throw;
            }
        }
    } value;
};

#pragma push
#pragma use_lmw_stmw on
struct Fn8010AssignmentFlag { unsigned char value; };

struct Fn8010AssignmentBase : Fn8010MergeList {
    // Initialize an empty temporary command list with self-linked sentinel.
    Fn8010AssignmentBase() {
        Fn8010Link* entry = &sentinel;
        *(Fn8010Link* volatile*)&sentinel.previousLink = entry;
        *(Fn8010Link* volatile*)&sentinel.nextLink = entry;
        *(volatile unsigned int*)&count = 0;
    }
    // Unlink and free all nodes owned by this temporary list.
    ~Fn8010AssignmentBase() {
        Fn8010Link* end = &sentinel;
        Fn8010Link* node = sentinel.nextLink;
        if (node != end) {
            Fn8010Link* tail = end->previousLink;
            node->previousLink->nextLink = tail->nextLink;
            tail->nextLink->previousLink = node->previousLink;
            while (node != end) {
                Fn8010Link* cur = node;
                node = node->nextLink;
                appFreeUni__FPv(cur);
                --count;
            }
        }
    }
};

struct Fn8010AssignmentGuard {
    Fn8010Link* volatile tail;
    Fn8010Link* node;
    // Free the new node if construction did not transfer it to the list.
    ~Fn8010AssignmentGuard() {
        if (node) appFreeUni__FPv(node);
    }
};

// Copy one command into an owned node and splice it into the temporary list.
static inline void fn8010Append(Fn8010AssignmentBase* self, Fn8010Link* from,
                               Fn8010Link*& tail,
                               volatile Fn8010AssignmentFlag& flag,
                               Fn8010Link* volatile& allocated,
                               Fn8010Link* volatile& allocatedEnd) {
    flag.value = 0;
    tail = &self->sentinel;
    Fn8010MergeNode* copy = (Fn8010MergeNode*)appMallocDummy__FUl(0x11c);
    allocated = (Fn8010Link*)copy;
    allocatedEnd = (Fn8010Link*)copy;
    Fn8010AssignmentGuard guard;
    guard.tail = tail;
    guard.node = (Fn8010Link*)copy;
    new (&copy->value) Fn8010MergeNode::Value(((Fn8010MergeNode*)from)->value);
    Fn8010Link* node = guard.node;
    guard.node = 0;
    tail->previousLink->nextLink = node;
    node->previousLink = tail->previousLink;
    tail->previousLink = node;
    node->nextLink = tail;
    ++self->count;
}

struct Fn8010AssignmentTemp : Fn8010AssignmentBase {
    // Copy the half-open source range into an owned temporary command list.
    Fn8010AssignmentTemp(Fn8010Link* begin, Fn8010Link* end,
                        Fn8010Link* volatile& p, Fn8010Link* volatile& rangeEnd,
                        Fn8010Link* volatile& allocated, Fn8010Link* volatile& allocatedEnd) {
        volatile Fn8010AssignmentFlag appendFlag;
        volatile Fn8010AssignmentFlag flag;
        flag.value = 0;
        Fn8010Link* tail;
        rangeEnd = end;
        p = begin;
        if (begin != end) {
            while (p != rangeEnd) {
                Fn8010Link* from = p;
                fn8010Append(this, from, tail, appendFlag, allocated, allocatedEnd);
                p = p->nextLink;
            }
        }
    }
};

// Advance in logical order: nextLink forward, previousLink backward.
static inline void fn8010Advance(int n, Fn8010Link*& cursor) {
    if (n >= 0) {
        while (n > 0) { cursor = cursor->nextLink; --n; }
    } else {
        while (n < 0) { cursor = cursor->previousLink; ++n; }
    }
}

// Borrow the smaller unsigned operand while preserving temporary lifetime.
static inline const unsigned int& fn8010Min(const unsigned int& b, const unsigned int& a) {
    if (b < a) return b;
    return a;
}

// Locate a logical command index from the nearer end; count denotes the sentinel.
static inline Fn8010Link* fn8010NodeAt(Fn8010MergeList* self, unsigned int length) {
    volatile Fn8010AssignmentFlag backFlag;
    volatile Fn8010AssignmentFlag frontFlag;
    unsigned int count = self->size();
    Fn8010Link* cursor;
    if (length <= count / 2) {
        frontFlag.value = 0;
        cursor = self->sentinel.nextLink;
        fn8010Advance(length, cursor);
    } else {
        backFlag.value = 0;
        cursor = &self->sentinel;
        fn8010Advance(length - count, cursor);
    }
    return cursor;
}

// Copy source command payloads into existing destination nodes, advancing both cursors.
static inline void fn8010Assign(Fn8010Link* volatile& source,
                                    Fn8010Link* volatile& end,
                                    Fn8010Link*& result) {
    while (source != end) {
        Fn8010MergeNode* from = (Fn8010MergeNode*)source;
        ((Fn8010MergeNode*)result)->value = from->value;
        result = result->nextLink;
        source = source->nextLink;
    }
}

extern "C" {

// Assign a command list, reusing existing nodes and appending or erasing the remainder.
void* FxCommandList_Assign(void* dstPtr, void* srcPtr) {
    Fn8010MergeList* dst = (Fn8010MergeList*)dstPtr;
    Fn8010MergeList* src = (Fn8010MergeList*)srcPtr;
    if (dst != src) {
        Fn8010Link* volatile savedCursor;
        unsigned int dstN;
        unsigned int srcN;
        Fn8010Link* volatile fullEnd;
        Fn8010Link* volatile first;
        Fn8010Link* volatile end;
        Fn8010Link* volatile start;
        Fn8010Link* volatile allocatedEnd;
        Fn8010Link* volatile allocated;
        Fn8010Link* volatile rangeEnd;
        Fn8010Link* volatile p;
        Fn8010Link* volatile sourceEndB;
        Fn8010Link* volatile sourceB;
        Fn8010Link* volatile eraseCursor;
        Fn8010Link* volatile sourceEndA;
        Fn8010Link* volatile sourceA;
        srcN = src->size();
        dstN = dst->size();
        const unsigned int& minimum = fn8010Min(srcN, dstN);
        Fn8010Link* cursor = fn8010NodeAt(src, minimum);
        start = cursor;
        end = &src->sentinel;
        fullEnd = &src->sentinel;
        savedCursor = cursor;
        Fn8010Link* srcFirst = src->sentinel.nextLink;
        first = srcFirst;
        if (cursor != &src->sentinel) {
            Fn8010AssignmentTemp temp(cursor, &src->sentinel, p, rangeEnd, allocated, allocatedEnd);
            Fn8010Link* sourceStart;
            Fn8010Link* sourceLimit = start;
            sourceStart = first;
            sourceEndA = sourceLimit;
            Fn8010Link* a = dst->sentinel.nextLink;
            sourceA = sourceStart;
            fn8010Assign(sourceA, sourceEndA, a);
            if (temp.count != 0) {
                Fn8010Link* tail;
                Fn8010Link* head = temp.sentinel.nextLink;
                tail = temp.sentinel.previousLink;
                temp.sentinel.nextLink = &temp.sentinel;
                temp.sentinel.previousLink = &temp.sentinel;
                a->previousLink->nextLink = head;
                head->previousLink = a->previousLink;
                a->previousLink = tail;
                tail->nextLink = a;
                dst->count += temp.count;
                temp.count = 0;
            }
        } else {
            sourceEndB = cursor;
            Fn8010Link* a = dst->sentinel.nextLink;
            sourceB = srcFirst;
            fn8010Assign(sourceB, sourceEndB, a);
            Fn8010Link* dstEnd = &dst->sentinel;
            eraseCursor = a;
            if (a != dstEnd) {
                Fn8010Link* tail = dstEnd->previousLink;
                a->previousLink->nextLink = tail->nextLink;
                tail->nextLink->previousLink = a->previousLink;
                Fn8010Link* node;
                while ((node = eraseCursor) != dstEnd) {
                    eraseCursor = node->nextLink;
                    appFreeUni__FPv(node);
                    --dst->count;
                }
            }
        }
    }
    return dst;
}
#pragma pop

struct Fn8010Payload {
    unsigned int id;
    int type;
    void* value;
    // Use the caller-provided placement address without allocating.
    static void* operator new(size_t, void* p) { return p; }
    struct Argument { unsigned int unused; volatile int type; void* volatile value; };
    // Copy the attachment key, effect kind and borrowed live-effect pointer.
    Fn8010Payload(unsigned int i, const Argument& other) {
        try {
            *(volatile unsigned int*)&id = i;
            type = other.type;
            value = other.value;
        } catch (...) {
            throw;
        }
    }
};

struct Fn8010EntryNode {
    Fn8010EntryNode* previousLink;
    Fn8010EntryNode* nextLink;
    Fn8010Payload payload;
};

struct Fn8010EntryGuardState {
    Fn8010Link* volatile tail;
    Fn8010Link* node;
};

struct Fn8010EntryGuard {
    Fn8010EntryGuardState& state;
    // Guard a newly allocated attachment node until list insertion takes ownership.
    Fn8010EntryGuard(Fn8010EntryGuardState& s, Fn8010Link* tail, Fn8010Link* node) : state(s) {
        state.tail = tail;
        state.node = node;
    }
    // Free a node only if its payload was not committed to the list.
    ~Fn8010EntryGuard() { if (state.node) appFreeUni__FPv(state.node); }
};

#pragma push
#pragma use_lmw_stmw on
#pragma opt_propagation off
// Named-bone metadata supplies both the tag text and attachment data.
struct FxAttachmentBone { char reserved00[0x74]; unsigned int flags; float matrix[12]; };
struct FxAttachmentTag { char reserved00[0x18]; char attachmentData[0x10]; char* text; };
struct FxNamedBonePair { FxAttachmentBone* bone; FxAttachmentTag* tag; };
struct FxAttachmentParameters {
    char reserved00[8]; void* boneMatrix; char reserved0C[0xc];
    void* tagData; char reserved1C[0xc];
};

// Parse bone resource tags, track sprays/sequences under one key, and return it or -1.
// model and its bone matrices/tag data are borrowed; the effect API consumes the
// stack parameter record synchronously. index selects an 8-byte command token.
// All supported tags in this call use nextAttachmentKey; increment occurs only
// when the LAST handled tag is supported. Unparsed tags leave that result alone.
// Required: initialized interpreter/list, live g_fx, and a live model whose
// namedBones has namedBoneCount {bone*, tag*} records. Every visited tag pointer
// must exist (it is dereferenced before the nullable bone test); text may be null.
// index must be 0 (space-delimited *fx) or 1 (quoted *playif); the token pool has
// two eight-byte slots. The parser always skips strlen(first token), even for 1:
// preserve this parsing rule when editing tags. Other indices are out of contract.
// Each complete filename token must fit parsed[64] INCLUDING NUL: token parsing
// writes without bounds BEFORE rejecting lengths >=64. Malformed input must still
// advance cursor on rejection, otherwise the retry loop cannot terminate.
// The 40-byte value record borrows bone matrix +0x78 and tag data +0x18; model
// storage must survive every attached effect that retains these references.
// Creation/allocation results are not validated: node allocation must succeed;
// a null effect result can be recorded and is later passed to the removal API.
// Return -1 does NOT roll back earlier attachments: an unsupported final handled
// tag clears success but leaves prior nodes/effects live under the unadvanced key.
// Release via RemoveEntriesByKey/ClearEntries while g_fx and model still live.
int FxAttachments_CreateFromResourceTags(char* self, WPuppet* model, int index) {
    unsigned int id;
    char* table;
    unsigned int offset;
    Fn8010Link* sentinel;
    char* entry;
    unsigned int i;
    unsigned char zero;
    bool lastHandledTagSupported;
    char* cursor;
    int mode;
    char parsed[0x40];
    char value[0x28];
    Fn8010Payload::Argument data1;
    Fn8010Payload::Argument data2;
    struct { Fn8010EntryGuardState mode2, mode1; } guards;
    volatile Fn8010AssignmentFlag flag1;
    volatile Fn8010AssignmentFlag flag2;
    Fn8010Link* volatile allocated1;
    Fn8010Link* volatile allocatedEnd1;
    Fn8010Link* volatile allocated2;
    Fn8010Link* volatile allocatedEnd2;
    // Phase 1: locate eligible named bones and scan their matching resource tags.
    table = fxConditionCommandTokens + index * 8;
    lastHandledTagSupported = 0;
    i = 0;
    offset = 0;
    for (; i <model->namedBoneCount; offset += 8, ++i) {
        entry = (char*)model->namedBones + offset;
        if (!entry) continue;
        char* key = ((FxNamedBonePair*)entry)->tag->text;
        if (!key) continue;
        if (!((FxNamedBonePair*)entry)->bone) continue;
        if (((FxNamedBonePair*)entry)->bone->flags & kBoneSuppressAttachedEffects) continue;
        cursor = key;
        for (;;) {
            cursor = (char*)strstr(cursor, table);
            if (!cursor) break;
            if (!EffectCommand_ParseResourceFilename(self, (void**)&cursor, parsed, index, &mode)) continue;
            FxAttachment_Init(value);
            *(char**)(value + 8) = *(char**)(entry + 0) + 0x78;
            ((FxAttachmentParameters*)value)->tagData = ((FxNamedBonePair*)entry)->tag->attachmentData;
            // Phase 2: create a spray and guard its node until payload construction
            // succeeds; clearing guard.node commits list ownership before splicing.
            if (mode == kSprayEntry) {
                zero = 0;
                data2.value = CFx_CreateAttachedEffect((void*)g_fx, value, parsed, zero, 0, zero);
                id = ((FxInterpreter*)self)->nextAttachmentKey;
                data2.type = kSprayEntry;
                sentinel = (Fn8010Link*)(self + 8);
                flag2.value = zero;
                Fn8010EntryNode* copy = (Fn8010EntryNode*)appMallocDummy__FUl(0x14);
                allocated2 = (Fn8010Link*)copy;
                allocatedEnd2 = (Fn8010Link*)copy;
                {
                    Fn8010EntryGuard guard(guards.mode2, sentinel, (Fn8010Link*)copy);
                    new (&copy->payload) Fn8010Payload(id, data2);
                    guards.mode2.node = 0;
                    sentinel->previousLink->nextLink = (Fn8010Link*)copy;
                    copy->previousLink = (Fn8010EntryNode*)sentinel->previousLink;
                    sentinel->previousLink = (Fn8010Link*)copy;
                    copy->nextLink = (Fn8010EntryNode*)sentinel;
                    ++((FxInterpreter*)self)->entryCount;
                }
                lastHandledTagSupported = 1;
                continue;
            }
            // Same commit sequence for sequence effects; type controls later removal.
            if (mode == kSequenceEntry) {
                zero = 0;
                data1.value = CFx_CreateAttachedEffect((void*)g_fx, value, parsed, zero, 0, zero);
                id = ((FxInterpreter*)self)->nextAttachmentKey;
                data1.type = kSequenceEntry;
                sentinel = (Fn8010Link*)(self + 8);
                flag1.value = zero;
                Fn8010EntryNode* copy = (Fn8010EntryNode*)appMallocDummy__FUl(0x14);
                allocated1 = (Fn8010Link*)copy;
                allocatedEnd1 = (Fn8010Link*)copy;
                {
                    Fn8010EntryGuard guard(guards.mode1, sentinel, (Fn8010Link*)copy);
                    new (&copy->payload) Fn8010Payload(id, data1);
                    guards.mode1.node = 0;
                    sentinel->previousLink->nextLink = (Fn8010Link*)copy;
                    copy->previousLink = (Fn8010EntryNode*)sentinel->previousLink;
                    sentinel->previousLink = (Fn8010Link*)copy;
                    copy->nextLink = (Fn8010EntryNode*)sentinel;
                    ++((FxInterpreter*)self)->entryCount;
                }
                lastHandledTagSupported = 1;
                continue;
            }
            // Preserve the original result rule: an unsupported later tag clears success.
            lastHandledTagSupported = 0;
        }
    }
    // Phase 3: publish one key for this call only when the final handled tag permits it.
    if (lastHandledTagSupported) {
        id = ((FxInterpreter*)self)->nextAttachmentKey;
        ((FxInterpreter*)self)->nextAttachmentKey = id + 1;
        return (int)id;
    }
    return -1;
}
#pragma pop

struct Fn8010String {
    unsigned char flag;
    char inlineData[3];
    unsigned int size;
    char* data;
};

// Name-tree nodes keep a 12-byte small-string key after their three links.
struct FxNameTreeNodeView {
    void* left; void* right; void* parent;
    union { Fn8010String key; unsigned int keyFlags; unsigned char keyHeader; };
};

// Decode the packed string length without changing ownership.
static unsigned int fn8010StringSize(Fn8010String* s) {
    return (s->flag & 0x80) ? (unsigned int)s->size : (unsigned int)(s->flag & 0x7f);
}

// Borrow characters from the packed inline or heap representation.
static const unsigned char* fn8010StringData(Fn8010String* s) {
    return (s->flag & 0x80) ? (const unsigned char*)s->data : (const unsigned char*)&s->flag + 1;
}

// Return borrowed characters and decode their length together.
static inline const unsigned char* fn8010StrGet(Fn8010String* s, unsigned int& len) {
    if ((*(unsigned int*)s >> 31) == 0) {
        len = *(unsigned char*)s;
        const unsigned char* d = (const unsigned char*)s + 1;
        len &= 0x7f;
        return d;
    }
    const unsigned char* d = (const unsigned char*)s->data;
    len = s->size;
    return d;
}

// Decode the packed inline/heap string length.
static inline unsigned int fn8010StrLen(Fn8010String* s) {
    if ((*(unsigned int*)s >> 31) == 0) return *(unsigned char*)s & 0x7f;
    return s->size;
}

// Order equal-prefix strings by unsigned length (-1, 0 or 1).
static inline int fn8010StrTie(unsigned int a, unsigned int b) {
    if (a < b) return -1;
    if (a == b) return 0;
    return 1;
}

#pragma push
#pragma use_lmw_stmw on
#pragma opt_common_subs off
#pragma opt_strength_reduction off
// Find an exact string key in the sequence cache, or return its header sentinel.
// Search invariant: candidate is the smallest visited node whose key >= searchKey,
// or &header.root. Compare unsigned bytes first, then lengths for equal prefixes.
// Walk left on >=, right on <. The final reverse comparison rejects a larger key,
// leaving either an exact match or the sentinel. Result borrows the owner's node.
// Repeated length reads preserve the recovered string accessor/lifetime boundaries.
void FxSequenceCache_FindByName(unsigned int** outIterator, const void* tree, const Fn8010String* searchKey) {
    unsigned int searchLength;
    unsigned int candidateLength;
    unsigned char* candidate;
    unsigned int** result;
    const void* treeHeader;
    const Fn8010String* key;
    unsigned int nodeCompareLength;
    unsigned int nodeLengthSecondRead;
    unsigned int searchDataLength;
    unsigned int nodeLengthFirstRead;
    unsigned int searchLengthFirstRead;
    unsigned int candidateDataLength;
    unsigned int searchLengthSecondRead;
    unsigned int searchCompareLength;
    unsigned int nodeAddress;
    result = outIterator;
    treeHeader = tree;
    key = searchKey;
    candidate = (unsigned char*)treeHeader + 4;
    nodeAddress = (unsigned int)*(unsigned char**)((char*)treeHeader + 4);
    while (nodeAddress) {
        const unsigned char* kdata;
        if ((*(unsigned int*)key >> 31) == 0) {
            kdata = (const unsigned char*)key->inlineData;
            searchLength = *(unsigned char*)key & 0x7f;
        } else {
            kdata = (const unsigned char*)key->data;
            searchLength = key->size;
        }
        searchDataLength = searchLength;
        if ((((FxNameTreeNodeView*)nodeAddress)->keyFlags >> 31) == 0) {
            nodeLengthFirstRead = ((FxNameTreeNodeView*)nodeAddress)->keyHeader & 0x7f;
        } else {
            nodeLengthFirstRead = ((FxNameTreeNodeView*)nodeAddress)->key.size;
        }
        const unsigned char* ndata;
        unsigned int nt;
        if ((((FxNameTreeNodeView*)nodeAddress)->keyFlags >> 31) == 0) {
            ndata = (const unsigned char*)((FxNameTreeNodeView*)nodeAddress)->key.inlineData;
            nt = ((FxNameTreeNodeView*)nodeAddress)->keyHeader & 0x7f;
        } else {
            ndata = (const unsigned char*)((FxNameTreeNodeView*)nodeAddress)->key.data;
            nt = ((FxNameTreeNodeView*)nodeAddress)->key.size;
        }
        nodeLengthSecondRead = nt;
        const unsigned int& nl = fn8010Min(nodeLengthFirstRead, nodeLengthSecondRead);
        nodeCompareLength = nl;
        int cmp = memcmp(ndata, kdata, fn8010Min(searchDataLength, nodeCompareLength));
        if (cmp == 0) {
            if (nodeCompareLength < searchLength) cmp = -1;
            else {
                if (nodeCompareLength == searchLength) cmp = searchLength - nodeCompareLength;
                else cmp = 1;
            }
        }
        if (cmp >= 0) {
            candidate = (unsigned char*)nodeAddress;
            nodeAddress = (unsigned int)*(unsigned char**)nodeAddress;
        } else {
            nodeAddress = (unsigned int)*(unsigned char**)(nodeAddress + 4);
        }
    }
    if (candidate != (unsigned char*)treeHeader + 4) {
        const Fn8010String* ikey = (Fn8010String*)(candidate + 0xc);
        const unsigned char* idata;
        if ((*(unsigned int*)ikey >> 31) == 0) {
            idata = (const unsigned char*)ikey + 1;
            candidateLength = *(unsigned char*)ikey & 0x7f;
        } else {
            idata = (const unsigned char*)ikey->data;
            candidateLength = ikey->size;
        }
        candidateDataLength = candidateLength;
        unsigned int at;
        if ((*(unsigned int*)key >> 31) == 0) {
            at = *(unsigned char*)key & 0x7f;
        } else {
            at = key->size;
        }
        searchLengthFirstRead = at;
        const unsigned char* kdata2;
        unsigned int bt;
        if ((*(unsigned int*)key >> 31) == 0) {
            kdata2 = (const unsigned char*)key->inlineData;
            bt = *(unsigned char*)key & 0x7f;
        } else {
            kdata2 = (const unsigned char*)key->data;
            bt = key->size;
        }
        searchLengthSecondRead = bt;
        const unsigned int* minp = &searchLengthSecondRead;
        if (searchLengthFirstRead < searchLengthSecondRead) minp = &searchLengthFirstRead;
        searchCompareLength = *minp;
        const unsigned int* cmpp = &searchCompareLength;
        if (candidateDataLength < searchCompareLength) cmpp = &candidateDataLength;
        int cmp = memcmp(kdata2, idata, *cmpp);
        if (cmp == 0) {
            if (searchCompareLength < candidateLength) cmp = -1;
            else {
                if (searchCompareLength == candidateLength) cmp = candidateLength - searchCompareLength;
                else cmp = 1;
            }
        }
        if (cmp < 0) candidate = (unsigned char*)treeHeader + 4;
    }
    *result = (unsigned int*)candidate;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
#pragma opt_common_subs off
// Replace a live sequence's commands from a named stream or cached template.
// Required self/sequenceOwner/name: initialized sequence, live owner cache/resource
// resolver, NUL-terminated name. The owner outlives the live sequence and cached
// templates; name/key text is copied, never retained from the caller.
// Clears old commands BEFORE lookup/open. Open failure returns 0 with the command
// list cleared and bypassResourceCache recorded; old playback is not restored.
// ClearCommands also clears attachment/currentSpray/failedCommandText and sets
// nextCommand to the sentinel. This is the open-failure state callers observe.
// The custom scanner's "%n" reads through spaces to tab/CR/LF (raw_file_stream_and_matrix_utils.cpp),
// unlike scanf's count-output conversion. Its zero return ends the loop without
// an error distinction. ParseEntryLine returns void: if it returns normally,
// this wrapper keeps, caches and sorts the accumulated commands and returns 1.
// It offers no parse-success/rollback guarantee. If parsing/allocation throws,
// resource-holder unwinding releases the stream but does not restore old commands.
// Scanner writes into lineBuffer[1024] without a width. Lines must fit including
// NUL; each field subsequently split by ParseEntryLine must also fit its 0x104
// byte token buffers (fx_sequence_script_parser.cpp). Template/node allocations must succeed (the
// null template path is dereferenced); allocation exceptions propagate.
// Stream ownership is temporary and released before template publication; the
// holder also releases it on unwind. Live commands are value copies, freed by
// ClearCommands/destruction, never aliases of the cached template's nodes.
// bypassCache==0: a miss publishes a new template in the owner's name tree.
// Nonzero: always parse and still allocate/copy a template, but do NOT publish or
// free that template in this routine. Preserve this existing lifetime gap; do
// not assume bypass mode transfers it to the cache or that it is leak-free.
// Success randomizes/sorts live command times (and may consume RNG in that
// callee); cache-hit path also resets nextCommand explicitly. No locking here.
int CFxSequence_LoadByName(char* self, const char* name, unsigned char bypassCache) {
    void* sequenceTemplate;
    unsigned int cacheIterator[1];
    void* resource;
    // A holder word stores &resource, not the resource itself. Releasing the
    // stream and setting resource=0 disarms the holder before its destructor.
    // Iterator arrays hold a single node pointer, never owned node storage.
    unsigned int resourceHolder[1];
    unsigned int cacheEnd[1];
    // Phase 1: clear live commands, construct a temporary 12-byte packed key,
    // and borrow a cache iterator. The key is destroyed immediately after lookup.
    CFxSequence_ClearCommands(self);
    if (strcmp(name, FxParserStringPoolView.debugSequenceName) == 0)
        OSReport(fxSequenceParserStrings + kEmptyDebugMessageOffset);
    (*(unsigned char*)&AsCFxSequence(self)->bypassResourceCache) = bypassCache;
    char lookupName[12];
    MWString_ConstructCString(lookupName, name);
    FxSequenceCache_FindByName((unsigned int**)cacheIterator, (Fn8010List*)((char*)AsCFxSequence(self)->sequenceOwner + kOwnerCacheOffset), (Fn8010String*)lookupName);
    ((NString*)(lookupName))->~NString();
    FxCommandList_GetEndCursor((unsigned int**)cacheEnd, (unsigned int*)((char*)AsCFxSequence(self)->sequenceOwner + kOwnerCacheOffset));
    if (FxWordIterator_Equals(cacheIterator, cacheEnd) != 0 || (*(unsigned char*)&AsCFxSequence(self)->bypassResourceCache) != 0) {
        resource = Resource_OpenResolvedPath(*(void**)((char*)AsCFxSequence(self)->sequenceOwner + kOwnerResourcesOffset), name, kSequenceResourceOpenFlags, 0, (*(unsigned char*)&AsCFxSequence(self)->bypassResourceCache), 0);
        ResourceHolder_SetResourceAddress((unsigned int*)resourceHolder, (unsigned int)&resource);
        if (!resource) {
            ResourceHandle_dtor((void*)resourceHolder, -1);
            return 0;
        }
        // Phase 2a: parse a stream into live commands, then copy a cache template.
        char lineBuffer[0x400];
        unsigned int lineNumber = 0;
        while (Stream_ScanFormat(resource, fxSequenceParserStrings + kScanLineFormatOffset, lineBuffer) != 0) {
            ++lineNumber;
            CFxSequence_ParseEntryLine(self, lineBuffer, lineNumber);
        }
        if (resource) { Resource_Release(resource); resource = 0; }
        sequenceTemplate = __nw__FUl(0x1d8);
        if (sequenceTemplate) CFxSequence_ctor(sequenceTemplate, AsCFxSequence(self)->sequenceOwner, 1);
        FxCommandList_Assign((char*)&AsCFxSequence(sequenceTemplate)->commandCount, (char*)&AsCFxSequence(self)->commandCount);
        if (!(*(unsigned char*)&AsCFxSequence(self)->bypassResourceCache)) {
            char cacheName[12];
            MWString_ConstructCString(cacheName, name);
            void* place = FxSequenceNameTree_GetOrInsertValue((char*)AsCFxSequence(self)->sequenceOwner + kOwnerCacheOffset, cacheName);
            *(void**)place = sequenceTemplate;
            ((NString*)(cacheName))->~NString();
        }
        ResourceHandle_dtor((void*)resourceHolder, -1);
    } else {
        // Phase 2b: copy cached command values and reset the live execution cursor.
        char* cachedSequence = (char*)*(void**)((char*)FxSequenceCache_GetIteratorValue(cacheIterator) + 0xc);
        FxCommandList_Assign((char*)&AsCFxSequence(self)->commandCount, (char*)&AsCFxSequence(cachedSequence)->commandCount);
        unsigned int value;
        FxCommandList_GetFirstCursor(&value, (Fn8010CD88Arg*)((char*)&AsCFxSequence(self)->commandCount));
        FxWordIterator_Copy((unsigned int*)(self + kNextCommandOffset), &value);
    }
    // Phase 3: both paths leave independent live nodes ready for timing/sorting.
    CFxSequence_RandomizeAndSortCommandTimes(self);
    return 1;
}
#pragma pop


// Guard the local views used by this unit against accidental layout changes.
#define POLISH_OFFSET(T, F, O) typedef char polish_##T##_##F[((int)&((T*)0)->F == (O)) ? 1 : -1]
POLISH_OFFSET(FxNameTreeNodeView, key, 0xc);
POLISH_OFFSET(FxCommandListView, sentinel, 0x4);
POLISH_OFFSET(FxAttachmentBone, matrix, 0x78);
POLISH_OFFSET(FxAttachmentTag, text, 0x28);
POLISH_OFFSET(FxAttachmentParameters, tagData, 0x18);
#undef POLISH_OFFSET
typedef char polish_size_FxAttachmentParameters[(sizeof(FxAttachmentParameters) == 0x28) ? 1 : -1];
typedef char polish_size_FxNamedBonePair[(sizeof(FxNamedBonePair) == 0x8) ? 1 : -1];

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. With "#pragma ipa file" the code is generated at
   the end of the file, so force_active must still be on there: a push/pop around the entry drops the flag, the linker
   dead-strips the entry and the initialiser never runs. */
#pragma force_active on
STATIC_INIT_ENTRY_NC(".CRT$XCU8046210C", _ctors_u8010C720_0, FxSequence_InitRotationPeriod);
