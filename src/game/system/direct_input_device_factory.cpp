// DirectInput device factory: creates keyboard and mouse input providers by name, and the
// string-map insertion and teardown helpers that its device registry uses.

#include <decomp/static_init.h>
#include <decomp/ct_member.h>
// Legacy DirectInput provider/device factory plus string-map node storage.
// Mod/API index: device name keys -> directInputStrings and CreateDeviceByName;
// keyboard ABI initialization -> DirectInputKeyboard_Ctor; map ownership and
// insertion -> StringMap_InsertNode, StringMapEntryCopy, StringMap_DestroySubtree.
// These are engine compatibility APIs, not evidence that Wii keyboard/mouse
// control is used by the stock game. Millisecond polling time is separate from
// the InputEx button-repeat call counts in gtl_input_ex_button_part1.cpp. Shared layouts live in
// classes/WDirectInput.h and classes/DirectInputKeyboard.h; data companions
// supply directInputStrings/stringMapRuntimeStrings and type-name pools.
#include "classes/WDirectInput.h"
// Borrow a provider layout without changing pointer identity or ownership.
static inline WDirectInput* AsWDirectInput(void* object) {
    return (WDirectInput*)object;
}
#include "classes/DirectInputKeyboard.h"
#define AsDirectInputKeyboard(object) ((DirectInputKeyboard*)(object))
// Kept as a cast macro because the constructor is compiled with dont_inline.

namespace exception_runtime {
extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12length_errorFv(void*, int);
extern const char stringMapRuntimeStrings[];
}
}

#include "sdk_protos.h"
#include "game/exception.h"

extern "C" {

// Byte pool shared by legacy DirectInput type names and factory device keys.
struct DirectInputStringPool {
    char typeName[12];
    char displayNames[17]; // Keyboard and Mouse, followed by the original padding.
    char keyboardKey[9];
    char mouseKey[6];
};
typedef char DirectInputStringPool_size[(sizeof(DirectInputStringPool) == 44) ? 1 : -1];
extern DirectInputStringPool directInputStrings;
enum {
    kObjectTypeNameOffset = 0xE1,
    kLengthErrorTypeOffset = 0x1A,
    kKeyboardProcVtableOffset = 0x48,
    kHeapStringFlagShift = 31
};
extern char stringMapRuntimeStrings[];
extern int s_iObjectTypeInfo;
extern float kFloat_6_2831855;
extern float s_inputObjectTypeFullTurnRadians;
extern float s_directInputModuleFullTurnRadians;
extern char __vt__12WDirectInput[];
extern void* IntPair_Set(void*, const char*, void*);
extern unsigned int OS_GetTimeMilliseconds();
extern void __dl__FPv(void*);
extern void appFreeUni__FPv(void*);
extern void String_FreeHeapBuffer(void*, void*);
extern void* memset(void*, int, unsigned int);
extern char __vt__19DirectInputKeyboard[];
extern void* __nw__FUl(unsigned int);
extern int CString_Compare(const char*, const char*);
extern void* __nwa__FUl(unsigned int);
extern void* appMallocDummy__FUl(unsigned int);
extern void String_ReserveCapacity(void*, unsigned int);
extern void String_ReplaceRange(void*, int, int, const char*, const char*, void*);
extern void RBTree_RebalanceAfterInsert(void*, void*, unsigned int);
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];
extern void DirectInputMouse_Ctor(void*);
// Return the legacy input-provider type name (DirectInput).
char* WDirectInput_GetDeviceName() {
    return directInputStrings.typeName;
}

// Return the supported device display-name list (Keyboard, Mouse).
char* WDirectInput_EnumModeName() {
    return directInputStrings.displayNames;
}

// Initialize the input module floating-point sentinel.
void WDirectInput_InitializeModuleAngle() {
    s_directInputModuleFullTurnRadians = kFloat_6_2831855;
}

// Optional input-provider hook; the Wii implementation has no work here.
void WInputDev_SetAlphaNumericMode() {}

// Optional input-provider hook; the Wii implementation has no work here.
void WInputDev_Reset() {}

// Return the most recent input poll timestamp in milliseconds.
int WInputDev_GetLastInputTime(void* self) {
    return AsWDirectInput(self)->dwLastInputTime;
}

// Read the second base status byte; its precise meaning is still unresolved.
int WInputDev_IsUpdated(void* self) {
    return AsWDirectInput(self)->bUpdated;
}

// Report that this input provider is available.
int WInputDev_IsAlphaNumericMode() {
    return 1;
}

// Store the input-provider enable byte without normalizing its value.
void WInputDev_SetActive(void* self, int enabled) {
    AsWDirectInput(self)->active = (unsigned char)enabled;
}

// Initialize the legacy input object type descriptor.
void WDirectInput_RegisterObjectBaseType() {
    s_inputObjectTypeFullTurnRadians = kFloat_6_2831855;
    IntPair_Set(&s_iObjectTypeInfo, stringMapRuntimeStrings + kObjectTypeNameOffset, 0);
}

// Initialize the input-provider flags and device-construction context.
void WDirectInput_Ctor(void* self) {
    AsWDirectInput(self)->active = 1;
    AsWDirectInput(self)->bUpdated = 0;
    AsWDirectInput(self)->vtbl = (void*)__vt__12WDirectInput;
    AsWDirectInput(self)->deviceFactoryContext = 0;
}

// Stamp the provider with the current millisecond clock.
void WInputDev_ResetInputTime(void* self) {
    AsWDirectInput(self)->dwLastInputTime = OS_GetTimeMilliseconds(); // 0x802A02AC: stw at +0x8
}

// Release the provider storage when called as a deleting destructor.
void* WDirectInput_dtor(void* self, int deleteStorage) {
    if (self != 0 && deleteStorage > 0) {
        __dl__FPv(self);
    }
    return self;
}

struct OwnedInputBuffer {
    void* vtable;
    void* allocation;
};
// Free the owned input buffer, then optionally its wrapper.
void* OwnedInputBuffer_dtor(void* self, int deleteStorage) {
    if (self != 0) {
        void* allocation = ((OwnedInputBuffer*)self)->allocation;
        if (allocation != 0) {
            appFreeUni__FPv(allocation);
        }
        if (deleteStorage > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

#pragma dont_inline on
// Construct the legacy keyboard/WProc interfaces and clear the three tracked keys.
// Required writable 24-byte allocation. factoryContext/deviceArgument are unused
// ABI arguments; this constructor neither borrows nor retains their contents.
// Initializes +4/+5 enable/status, +10 registry, +0/+C interface vptrs and three
// key bytes beginning at key32. Other bytes remain untouched; returns self.
void* DirectInputKeyboard_Ctor(void* self, int factoryContext, void* deviceArgument) {
    *(unsigned char*)(AsDirectInputKeyboard(self)->inputBaseStateBytes) = 1;
    AsDirectInputKeyboard(self)->inputBaseStateBytes[1] = 0; // 0x5: WDirectInput base status byte
    AsDirectInputKeyboard(self)->proc.registry = 0;          // 0x10
    AsDirectInputKeyboard(self)->vtbl = (void*)__vt__19DirectInputKeyboard;
    AsDirectInputKeyboard(self)->proc.vtbl = (void*)(__vt__19DirectInputKeyboard + kKeyboardProcVtableOffset); // 0xc
    memset(((char*)&AsDirectInputKeyboard(self)->key32), 0, 3);
    return self;
}
#pragma dont_inline off

// The factory returns the mouse interface at +8, after the original two-word base.
struct LegacyMouseBaseStorage {
    int storage[2];
};
struct LegacyMouseDeviceStorage {
    int storage[16];
};
struct LegacyMouseAllocation : LegacyMouseBaseStorage, LegacyMouseDeviceStorage {
    // Initialize the complete allocation before returning its secondary interface.
    LegacyMouseAllocation() {
        DirectInputMouse_Ctor(this);
    }
};

struct LegacyKeyboardAllocation {
    int a[6];
    // Forward factory ABI context to the keyboard constructor (currently ignored).
    LegacyKeyboardAllocation(int context, void* argument) {
        DirectInputKeyboard_Ctor(this, context, argument);
    }
};

// Create a legacy keyboard or mouse by name; the mouse returns its secondary interface.
// self/name required; name is a borrowed NUL-terminated key compared by CString.
// Unknown name returns null with no allocation. Recognized names allocate a new
// owning device, with normal new/constructor exception behavior and no local
// catch. Ownership passes to caller: release through the returned interface's
// virtual deleting destructor (positive delete flag), never appFreeUni on the
// returned mouse+8 pointer. Its interface thunk must recover the complete object.
// deviceArgument/context are forwarded only for keyboard and currently ignored;
// no retained argument or provider ownership is created by this function.
void* WDirectInput_MakeClone(void* self, const char* name, void* deviceArgument) {
    DirectInputStringPool* deviceNames = &directInputStrings;
    if (CString_Compare(name, deviceNames->keyboardKey) == 0) {
        return new LegacyKeyboardAllocation(AsWDirectInput(self)->deviceFactoryContext, deviceArgument);
    }
    if (CString_Compare(name, deviceNames->mouseKey) == 0) {
        return (LegacyMouseDeviceStorage*)new LegacyMouseAllocation;
    }
    return 0;
}

struct StringMapEntryStorage {
    unsigned int flags;
    unsigned int length;
    char* data;
    unsigned int local;
};

struct SharedStringBuffer {
    void* vtbl;
    unsigned int refs;
    unsigned int refs2;
    char* data;
};

// Three tree links, then the key ownership word, heap length and data pointer.
struct StringTreeNode {
    StringTreeNode* first;
    StringTreeNode* second;
    unsigned int parentLink;
    unsigned int owned;
    unsigned int heapLength;
    void* ownedPtr;
};

// Destroy a string-map subtree and free each heap-backed key before its node.
void StringMap_DestroySubtree(void* owner, StringTreeNode* node) {
    StringTreeNode* group = node->first;
    StringTreeNode* sub;
    StringTreeNode* leaf;
    if (group != 0) {
        sub = group->first;
        if (sub != 0) {
            leaf = sub->first;
            if (leaf != 0) {
                if (leaf->first != 0)
                    StringMap_DestroySubtree(owner, leaf->first);
                if (leaf->second != 0)
                    StringMap_DestroySubtree(owner, leaf->second);
                String_FreeHeapBuffer(owner, &leaf->owned);
                appFreeUni__FPv(leaf);
            }
            leaf = sub->second;
            if (leaf != 0) {
                if (leaf->first != 0)
                    StringMap_DestroySubtree(owner, leaf->first);
                if (leaf->second != 0)
                    StringMap_DestroySubtree(owner, leaf->second);
                String_FreeHeapBuffer(owner, &leaf->owned);
                appFreeUni__FPv(leaf);
            }
            if (&sub->owned != 0) {
                if (&sub->owned != 0) {
                    if ((sub->owned >> kHeapStringFlagShift) != 0)
                        appFreeUni__FPv(sub->ownedPtr);
                }
            }
            appFreeUni__FPv(sub);
        }
        sub = group->second;
        if (sub != 0) {
            leaf = sub->first;
            if (leaf != 0) {
                if (leaf->first != 0)
                    StringMap_DestroySubtree(owner, leaf->first);
                if (leaf->second != 0)
                    StringMap_DestroySubtree(owner, leaf->second);
                String_FreeHeapBuffer(owner, &leaf->owned);
                appFreeUni__FPv(leaf);
            }
            leaf = sub->second;
            if (leaf != 0) {
                if (leaf->first != 0)
                    StringMap_DestroySubtree(owner, leaf->first);
                if (leaf->second != 0)
                    StringMap_DestroySubtree(owner, leaf->second);
                String_FreeHeapBuffer(owner, &leaf->owned);
                appFreeUni__FPv(leaf);
            }
            if (&sub->owned != 0) {
                if (&sub->owned != 0) {
                    if ((sub->owned >> kHeapStringFlagShift) != 0)
                        appFreeUni__FPv(sub->ownedPtr);
                }
            }
            appFreeUni__FPv(sub);
        }
        if (&group->owned != 0) {
            if (&group->owned != 0) {
                if ((group->owned >> kHeapStringFlagShift) != 0)
                    appFreeUni__FPv(group->ownedPtr);
            }
        }
        appFreeUni__FPv(group);
    }
    group = node->second;
    if (group != 0) {
        sub = group->first;
        if (sub != 0) {
            leaf = sub->first;
            if (leaf != 0) {
                if (leaf->first != 0)
                    StringMap_DestroySubtree(owner, leaf->first);
                if (leaf->second != 0)
                    StringMap_DestroySubtree(owner, leaf->second);
                String_FreeHeapBuffer(owner, &leaf->owned);
                appFreeUni__FPv(leaf);
            }
            leaf = sub->second;
            if (leaf != 0) {
                if (leaf->first != 0)
                    StringMap_DestroySubtree(owner, leaf->first);
                if (leaf->second != 0)
                    StringMap_DestroySubtree(owner, leaf->second);
                String_FreeHeapBuffer(owner, &leaf->owned);
                appFreeUni__FPv(leaf);
            }
            if (&sub->owned != 0) {
                if (&sub->owned != 0) {
                    if ((sub->owned >> kHeapStringFlagShift) != 0)
                        appFreeUni__FPv(sub->ownedPtr);
                }
            }
            appFreeUni__FPv(sub);
        }
        sub = group->second;
        if (sub != 0) {
            leaf = sub->first;
            if (leaf != 0) {
                if (leaf->first != 0)
                    StringMap_DestroySubtree(owner, leaf->first);
                if (leaf->second != 0)
                    StringMap_DestroySubtree(owner, leaf->second);
                String_FreeHeapBuffer(owner, &leaf->owned);
                appFreeUni__FPv(leaf);
            }
            leaf = sub->second;
            if (leaf != 0) {
                if (leaf->first != 0)
                    StringMap_DestroySubtree(owner, leaf->first);
                if (leaf->second != 0)
                    StringMap_DestroySubtree(owner, leaf->second);
                String_FreeHeapBuffer(owner, &leaf->owned);
                appFreeUni__FPv(leaf);
            }
            if (&sub->owned != 0) {
                if (&sub->owned != 0) {
                    if ((sub->owned >> kHeapStringFlagShift) != 0)
                        appFreeUni__FPv(sub->ownedPtr);
                }
            }
            appFreeUni__FPv(sub);
        }
        if (&group->owned != 0) {
            if (&group->owned != 0) {
                if ((group->owned >> kHeapStringFlagShift) != 0)
                    appFreeUni__FPv(group->ownedPtr);
            }
        }
        appFreeUni__FPv(group);
    }
    if (&node->owned != 0) {
        if (&node->owned != 0) {
            if ((node->owned >> kHeapStringFlagShift) != 0)
                appFreeUni__FPv(node->ownedPtr);
        }
    }
    appFreeUni__FPv(node);
}

extern void __dla__FPv(void*);

struct ZeroAlloc {
    char c;
    // Initialize the empty allocator tag used when constructing exception text.
    ZeroAlloc() : c(0) {}
};
struct RawAlloc {
    char c;
};

struct StringMapEntryCopy {
    unsigned int flags;
    unsigned int length;
    char* data;
    unsigned int local;
    // Copy a 16-byte entry, not a whole node. At entry+0 bit31 selects storage:
    // clear -> copy inline representation words verbatim; set -> reserve/copy
    // s[1] bytes from s[2] into independently owned heap string storage.
    // entry+0C copies the mapped word unchanged, with no pointee retain/release.
    // src is borrowed, valid for construction, and must not alias new storage.
    StringMapEntryCopy(const unsigned int* s) {
        unsigned int f = s[0];
        try {
            if ((f >> kHeapStringFlagShift) == 0) {
                flags = f;
                length = s[1];
                data = (char*)s[2];
            } else {
                flags = 0;
                length = 0;
                data = 0;
                String_ReserveCapacity(this, s[1]);
                RawAlloc src8;
                RawAlloc flag = src8;
                String_ReplaceRange(this, 0, 0, (const char*)s[2], (const char*)(s[2] + s[1]), &flag);
            }
            local = s[3];
        } catch (...) {
            throw;
        }
    }
};

struct CStringMapNodeStorage {
    char state;
    void* begin;
    void* end;
    // Obtain 0x1C bytes for three links plus the 16-byte key/value entry.
    CStringMapNodeStorage() : state(0), begin(appMallocDummy__FUl(0x1c)), end(begin) {}
    // Raw allocator tag owns nothing: StringNodeGuard controls the allocation.
    ~CStringMapNodeStorage() {}
};

typedef CStringMapNodeStorage StringNodeAllocation;

struct StringGuard {
    char* data;
    char* allocator;
    // Release exception-message bytes unless ownership moved to the shared holder.
    ~StringGuard() {
        if (data)
            __dla__FPv(data);
    }
};
struct ErrorBase_u8029FAC4 : std::exception {
    // Install the reconstructed exception dispatch in its ABI vptr word.
    void SetTable(const void* vt) {
        *(const void**)this = vt;
    }

    char* data;
    void* holder;
    // Build a shared owned copy of the borrowed length-error diagnostic.
    ErrorBase_u8029FAC4(const char* message) {
        SetTable(__vt__Q23std11logic_error);
        volatile char allocator2 = 0;
        ZeroAlloc allocator;
        unsigned int length = strlen(message) + 1;
        data = (char*)__nwa__FUl(length * 2 - length);
        char* copy = data;
        {
            StringGuard guard = {copy, (char*)&allocator};
            struct Holder {
                void* vtable;
                int a, b;
                char* data;
            };
            Holder* h = (Holder*)__nw__FUl(16);
            if (h) {
                h->a = 1;
                h->b = 1;
                h->vtable = s_sharedCharDeleterVtable;
                h->data = copy;
            }
            holder = h;
            guard.data = 0;
        }
        strcpy(data, message);
    }
};
} // extern "C"
namespace std {
struct length_error : ErrorBase_u8029FAC4 {
    // Preserve the message-owning base and specialize its dispatch to length_error.
    length_error(const char* message) : ErrorBase_u8029FAC4(message) {
        SetTable(__vt__Q23std12length_error);
    }
    ~length_error();
    DECOMP_CT_MEMBER(length_error)
};
}
extern "C" {

struct Fn8029FAC4NodeGuard {
    void* allocator;
    void* node;
    // Temporarily own raw node bytes; tree ownership begins when node is cleared.
    Fn8029FAC4NodeGuard(void* t, void* n) : allocator((char*)t + 4), node(n) {}
    // Free raw bytes if entry construction unwinds before publication.
    ~Fn8029FAC4NodeGuard() {
        if (node)
            appFreeUni__FPv(node);
    }
};

typedef Fn8029FAC4NodeGuard StringNodeGuard;

struct StringParentLink {
    void* ptr;
    // Construct the parent word in place, retaining the original EH shape.
    StringParentLink(void* p) {
        try {
            ptr = p;
        } catch (...) {
            throw;
        }
    }
};

// Placement construction borrows already allocated node storage; no allocation.
inline void* operator new(size_t, void* p) {
    return p;
}

#pragma push
#pragma use_lmw_stmw on
#pragma opt_common_subs off
// Tree nodes have three links, followed by the 16-byte key/value representation.
struct StringMapNode {
    void* left;
    void* right;
    void* parent;
    StringMapEntryCopy entry;
};
struct StringMapHeader {
    unsigned int count;
    void* root;
    void* first;
};
// Copy a key/value into a new tree node, attach it and rebalance the string map.
// self is a live header {u32 count, root pointer, first pointer}; slot is the
// parent/sentinel with a writable left (+0) or right (+4) link chosen by insertLeft.
// Caller has already searched the tree and selected an empty insertion link;
// there is no key lookup, duplicate check or replacement policy here.
// becomesFirst publishes the new minimum node at header+8. src points to the
// 16-byte ENTRY (not node), borrowed until copy completes; mapped word is shallow.
// The peculiar count test is unsigned count==0xFFFFFFFF (addition wraps), not
// a 65535-entry limit. It throws length_error before allocation/publication.
// After allocation, the raw-node guard frees on entry-construction exceptions;
// normal insertion transfers the constructed key/node to the tree, increments
// count and rebalances. No status result: returns the new TREE-OWNED node.
// Allocation failure is delegated to the allocator; a null allocation is not
// handled. There is no rollback after guard.node=0/link publication; rebalance
// must not throw. Serialize tree mutation; preserve source lifetimes and EH.
void* StringMap_InsertNode(void* self, void* slot, int insertLeft, int becomesFirst, const void* src) {
    // Phase 1: capacity guard and standard exception construction.
    if (((StringMapHeader*)self)->count + 0x10000 == 0xffff) {
        const char* msg = stringMapRuntimeStrings;
        {
            union {
                double alignment;
                unsigned char bytes[sizeof(std::length_error)];
            } storage;
            std::length_error* error = (std::length_error*)storage.bytes;
            error->__ct(msg);
            exception_runtime::__throw(exception_runtime::stringMapRuntimeStrings + kLengthErrorTypeOffset, error,
                                       (void*)exception_runtime::__dt__Q23std12length_errorFv);
        }
    }
    // Phase 2: allocate and construct an independent key with shallow mapped word.
    StringNodeAllocation alloc;
    StringNodeGuard guard(self, alloc.begin);
    StringMapEntryCopy* element = &((StringMapNode*)guard.node)->entry;
    new (element) StringMapEntryCopy((const unsigned int*)src);
    // Phase 3: initialize links, transfer ownership, attach/count/rebalance, then
    // publish the first-node cache. Keep this order and the parent wrapper.
    StringMapNode* insertedNode = (StringMapNode*)guard.node;
    insertedNode->right = 0;
    guard.node = 0;
    insertedNode->left = 0;
    new (&insertedNode->parent) StringParentLink(slot);
    if (insertLeft)
        ((StringMapNode*)slot)->left = insertedNode;
    else
        ((StringMapNode*)slot)->right = insertedNode;
    unsigned int oldCount = *(unsigned int*)self;
    *(unsigned int*)self = oldCount + 1;
    RBTree_RebalanceAfterInsert(insertedNode, ((StringMapHeader*)self)->root, oldCount);
    if (becomesFirst)
        ((StringMapHeader*)self)->first = insertedNode;
    return insertedNode;
}
#pragma pop

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804624A4", _ctors_u8029FAC4_0, WDirectInput_RegisterObjectBaseType);
STATIC_INIT_ENTRY(".CRT$XCU804624A4", _ctors_u8029FAC4_1, WDirectInput_InitializeModuleAngle);
#pragma pop
