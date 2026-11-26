// File path registry for the file-IO layer: a red-black tree that maps device path prefixes to
// their file devices, plus the default device path lookup and path string helpers.

#include <decomp/static_init.h>
#include "classes/FileRegistryNode.h"
#define AsFileRegistryNode(p) ((FileRegistryNode*)(p))
#include "classes/FileRegistryString.h"
#define AsFileRegistryString(p) ((FileRegistryString*)(p))
#include "classes/FileRegistryTree.h"
#define AsFileRegistryTree(p) ((FileRegistryTree*)(p))

// The opt_* pragmas in this file switch individual optimizer passes on/off around single functions, because the
// original compiler needed those settings to produce the same code; they change no behaviour, only the generated
// instruction order. The #define/#undef pair below gives the header's char_array_guard a unit-specific name.
#pragma opt_common_subs on
#pragma opt_propagation on
#pragma opt_lifetimes off
#define char_array_guard CharArrayGuard_u804170C0
#include "game/stdexcept_local.h"
#undef char_array_guard
#pragma opt_common_subs reset
#pragma opt_propagation reset
#pragma opt_lifetimes reset
#include "sdk_protos.h"
struct FileIoPathStrings {
    char dvdDevice[5];
    char hostDevice[6];
    char fileMemoryDevice[6];
    char physicalMemoryDevices[4][7];
    char separator[2];
    char parentDirectory[3];
    char currentDirectory[2];
    char treeInsertError[26];
    char lengthErrorType[55];
    char stringReserveError[35];
    char stringRangeError[27];
    char rangeErrorType[55];
    char stringLengthError[27];
    char reserved115[3];
};
typedef char FileIoPathStrings_size[(sizeof(FileIoPathStrings) == 0x118) ? 1 : -1];
// Registry ownership: the tree owns node storage and each key string; file pointers
// are borrowed device objects. parentAndColor = aligned parent address | red bit.
// The sentinel is the address of tree.root, not a separately allocated node.
struct RegistryAllocationGuardState {
    void* owner;
    void* allocation;
};
enum {
    kFilePathBufferBytes = 0x104,
    kMissingFileDevice = 0x14,
    kInvalidDeviceName = 0x15,
    kRegistryNodeColorBit = 1,
    kInlineStringLengthMask = 0x7f,
    kRegistryRootOffset = (int)&((FileRegistryTree*)0)->root
};

extern "C" {
void __dl__FPv(void*);
void (*g_fileIoFreeCallback)(void*);
#pragma pool_data on
// Reserved mutex storage, not a declared OSMutex-sized object; platform APIs
// receive a borrowed OSMutex pointer. No consumer below accesses neighboring data
// through this storage's type or assumes a combined mutex/registry/path object.
char s_fileIoMutex[0x24];
// Registry header (0xC), default path (0x104), then original trailing padding.
// Keep the split Wii definitions for identical symbol/relocation layout.
char s_fileRegistryTree[0xc];
char s_defaultDevicePath[kFilePathBufferBytes];
// Default path used when the caller supplies an empty path.
#pragma pool_data reset
char s_fileRegistryDestructorRegistration[0xC];
void FileRegistry_Dtor();
extern FileIoPathStrings fileIoPathStrings;
extern char s_fileIoAutoLockMutexVTable[];

void FileIOString_FitCapacityForLength(void*, unsigned int);
void FileIOString_ReplaceRangeWithAllocator(void*, unsigned int, unsigned int, const char*, const char*, void*);
void FileRegistry_RebalanceAfterInsert(void*, unsigned int);
void* memset(void*, int, size_t);
int gtl_FileIO_CheckSaveArea(void*, void*, int, int, int, int);
void* (*g_fileIoAllocate)(unsigned int);
char s_sharedCharDeleterVtable[];

} // extern "C"

// Release a registry key only when its small-string flags select heap storage.
// The allocator argument is unused; callers pass it as before.
void FileRegistryString::ReleaseStorage(int unusedAllocator, FileRegistryString* string) {
    if (string) {
        if (string) {
            unsigned int flags = string->flags;
            if (flags >> 31) {
                void* heapData = string->data;
                g_fileIoFreeCallback(heapData);
            }
        }
    }
}

extern "C" {

// Initialize the empty registry sentinel and register its global destructor.
void FileRegistry_InitGlobalTree() {
    AsFileRegistryTree(s_fileRegistryTree)->count = 0;
    AsFileRegistryTree(s_fileRegistryTree)->root = 0;
    // Keep the sentinel byte address expression to preserve CodeWarrior's register assignment.
    (*(int*)&AsFileRegistryTree(s_fileRegistryTree)->first) = (int)(s_fileRegistryTree + kRegistryRootOffset);
    __register_global_object(s_fileRegistryTree, (void*)FileRegistry_Dtor, s_fileRegistryDestructorRegistration);
}

// Free an allocation guard's owned buffer and optionally delete the guard.
void* dtor_804182A0(void* self, int shouldDelete) {
    if (self) {
        void* allocation = ((RegistryAllocationGuardState*)self)->allocation;
        if (allocation) {
            g_fileIoFreeCallback(allocation);
        }
        if (shouldDelete > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

#pragma opt_propagation off
// Build a registry string from a NUL-terminated path using its configured allocator.
void* FileIOString_CtorFromCString(void* self, const char* text) {
    AsFileRegistryString(self)->flags = 0;
    AsFileRegistryString(self)->length = 0;
    AsFileRegistryString(self)->data = 0;
    size_t length = strlen(text);
    FileIOString_FitCapacityForLength(self, (unsigned int)length);
    unsigned int allocator;
    volatile unsigned char sourceAllocator;
    *(unsigned char*)&allocator = sourceAllocator;
    FileIOString_ReplaceRangeWithAllocator(self, 0, 0, text, text + length, &allocator);
    return self;
}
#pragma opt_propagation reset

#pragma use_lmw_stmw on
#pragma opt_common_subs off
#pragma opt_propagation on
#pragma opt_lifetimes off
#pragma opt_branch_folding on
// Return the key byte count from the heap length or inline length tag.
inline unsigned int SaveAreaNodeStringLength(void* node) {
    return (AsFileRegistryNode(node)->stringFlags >> 31)
               ? AsFileRegistryNode(node)->stringLength
               : (*(unsigned char*)&AsFileRegistryNode(node)->stringFlags) & kInlineStringLengthMask;
}
// Borrow key bytes from heap storage or from the node itself; do not free them.
inline const char* SaveAreaNodeStringData(void* node) {
    return (AsFileRegistryNode(node)->stringFlags >> 31)
               ? AsFileRegistryNode(node)->stringData
               : (char*)&AsFileRegistryNode(node)->stringFlags + 1; // SaveAreaNodeStringData: inline string +0xD
}
// Walk in-order from first to the embedded sentinel, borrowing the matching device.
// Always unlock mutex, including on a miss. deviceNameOrFile changes role only
// after the comparison succeeds; the other reference outputs are traversal scratch.
// In/out: node advances; deviceNameOrFile goes from borrowed key text to borrowed
// device (or null). sentinel and mutex retain their values. right/successor/child/
// parent are disposable traversal outputs, never owned resources. The references
// retain caller lifetimes; a four-reference version changed register allocation.
inline void FindSaveAreaDeviceAndUnlock(void*& node, void*& sentinel, void*& deviceNameOrFile, void*& mutex,
                                        void*& right, void*& successor, void*& child, void*& parent) {
    while (node != sentinel) {
        if (strncmp((const char*)deviceNameOrFile, SaveAreaNodeStringData(node), SaveAreaNodeStringLength(node)) == 0) {
            deviceNameOrFile = AsFileRegistryNode(node)->file;
            OSUnlockMutex((OSMutex*)mutex);
            return;
        }
        right = AsFileRegistryNode(node)->right;
        if (right) {
            successor = right;
            while ((child = *(void**)successor) != 0) {
                successor = child;
            }
            node = successor;
        } else {
            while (node !=
                   *(void**)(parent = (void*)(AsFileRegistryNode(node)->parentAndColor & ~kRegistryNodeColorBit)))
                node = parent;
            node = parent;
        }
    }
    OSUnlockMutex((OSMutex*)mutex);
    deviceNameOrFile = 0;
}

// Look up the fmem device under the registry mutex, then delegate its save-area check.
// Despite the shared symbol name, this always selects fmem:, not the current directory.
// The self-strcmp and its 0x15 error path are retained from the original expansion.
// saveArea/queryResult/statusOut are forwarded unchanged; callback and context
// are opaque here. Return 0x14 on missing device, otherwise the delegate status.
int gtl_FileIO_CheckFmemSaveArea(void* saveArea, void* queryResult, void* statusOut, int completionCallback,
                                 int callbackContext) {
    void* mutex;
    void* sentinel;
    void* node;
    void* deviceNameOrFile;
    node = &fileIoPathStrings;
    char* deviceName = ((FileIoPathStrings*)node)->fileMemoryDevice;
    if (strcmp((const char*)deviceName, (const char*)deviceName)) {
        return kInvalidDeviceName;
    }
    deviceNameOrFile = ((FileIoPathStrings*)node)->fileMemoryDevice;
    mutex = (void*)s_fileIoMutex;
    OSLockMutex((OSMutex*)mutex);
    node = AsFileRegistryTree(s_fileRegistryTree)->first;
    sentinel = (char*)&AsFileRegistryTree(s_fileRegistryTree)->root;
    void* right;
    void* successor;
    void* child;
    void* parent;
    FindSaveAreaDeviceAndUnlock(node, sentinel, deviceNameOrFile, mutex, right, successor, child, parent);
    if (deviceNameOrFile == 0) {
        return kMissingFileDevice;
    }
    return gtl_FileIO_CheckSaveArea(deviceNameOrFile, saveArea, (int)queryResult, (int)statusOut, completionCallback,
                                    callbackContext);
}
#pragma opt_branch_folding reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma use_lmw_stmw reset

// Construct payloads directly in caller-provided registry node storage.
extern "C++" {
inline void* operator new(size_t, void* p) {
    return p;
}
}
#pragma opt_common_subs off
#pragma opt_propagation off
#pragma opt_dead_code on
#pragma opt_lifetimes off
#pragma opt_pointer_analysis off
struct NodeStorageGuard {
    void* owner;
    void* ptr;
    // Guard raw node storage until payload construction and child initialization commit.
    NodeStorageGuard(void* a, void* p) : owner(a), ptr(p) {}
    // Free only uncommitted node storage; payload construction handles its own unwind.
    ~NodeStorageGuard() {
        if (ptr)
            g_fileIoFreeCallback(ptr);
    }
};
#pragma opt_propagation on
#pragma opt_common_subs off
// Key/file payload copy: inline strings copy all three words, heap strings allocate their own storage.
struct FileRegistryValue {
    unsigned int stringFlags, stringLength, stringDataBits, fileBits;
    // Deep-copy the key representation and borrow the same file/device pointer.
    FileRegistryValue(const FileRegistryValue& v) {
        try {
            if (!(v.stringFlags >> 31)) {
                stringFlags = v.stringFlags;
                int length = v.stringLength;
                unsigned int dataBits = v.stringDataBits;
                stringLength = length;
                stringDataBits = dataBits;
            } else {
                stringFlags = 0;
                unsigned int capacity = v.stringLength;
                stringLength = 0;
                stringDataBits = 0;
                FileIOString_FitCapacityForLength(this, capacity);
                unsigned char copy;
                volatile unsigned char pad[3];
                volatile unsigned char empty;
                copy = empty;
                const char* first = (const char*)v.stringDataBits;
                const char* last = first + v.stringLength;
                FileIOString_ReplaceRangeWithAllocator(this, 0, 0, first, last, &copy);
            }
            fileBits = v.fileBits;
        } catch (...) {
            __throw(0, 0, 0);
        }
    }
};
struct ParentLink {
    void* ptr;
    // Install the aligned parent pointer before red/black tagging during repair.
    ParentLink(void* p) try : ptr(p) {
    } catch (...) {
    }
};
struct Allocation804177CC {
    volatile unsigned char tag;
    void* ptr;
    // Leave temporary allocation state untouched until the allocator fills it.
    Allocation804177CC() {}
    // Copy the allocation pointer; the opaque tag has no ownership role.
    Allocation804177CC(const Allocation804177CC& v) : ptr(v.ptr) {}
};
// Allocate one unconstructed registry node through the configured callback.
inline Allocation804177CC AllocateNode() {
    Allocation804177CC result;
    result.tag = 0;
    result.ptr = g_fileIoAllocate(sizeof(FileRegistryNode));
    return result;
}
// Construct the registry length error using the pooled type/message ABI.
inline void RaiseRegistryLengthError(const char* text) {
    std::length_error error(text, fileIoPathStrings.lengthErrorType);
}
extern char __vt__Q23std12length_error[];
struct TreeInsertLengthError : std::exception {
    char* message;
    std::detail::message_control* control;
    // Build and throw the maximum-node-count error with shared message ownership.
    TreeInsertLengthError() {
        const char* text = fileIoPathStrings.treeInsertError;
        void* const error = this;
        *(void**)this = __vt__Q23std11logic_error;
        volatile std::detail::default_delete_chars deleter;
        std::detail::default_delete_chars copy;
        unsigned int length = strlen(text) + 1;
        message = static_cast<char*>(operator new[]((length << 1) - length));
        {
            char* buffer = message;
            std::detail::CharArrayGuard_u804170C0 guard(buffer, copy);
            control = new std::detail::message_control(buffer);
            guard.release();
        }
        strcpy(message, text);
        *(void**)error = __vt__Q23std12length_error;
        __throw(fileIoPathStrings.lengthErrorType, error, (void*)std::__dt__Q23std12length_errorFv);
    }
};
#pragma use_lmw_stmw on
#pragma use_lmw_stmw on
#pragma opt_common_subs off
#pragma opt_propagation on
#pragma opt_dead_code on
#pragma opt_pointer_analysis off
#pragma opt_lifetimes off
#pragma opt_branch_folding on
// Copy a key/file value into a new node, link it, rebalance, and update the first-node cache.
// Parameters (all untyped for ABI reasons; the real types are the registry classes in classes/FileRegistry*.h):
//   treeArgument         = registry tree (count, root/sentinel, first-node cache)
//   parentArgument       = existing node the new node hangs under (the sentinel when the tree is empty)
//   insertLeftArgument   = nonzero -> new node becomes parent's left child, zero -> right child (bool in a pointer slot)
//   updateFirstArgument  = nonzero -> the new node becomes the cached first (leftmost) node
//   valueArgument        = key/file value to deep-copy into the node
// Returns the new node. Aborts via TreeInsertLengthError if the tree already holds 0xFFFFFFFF nodes.
void* FileRegistry_InsertNode(void* treeArgument, void* parentArgument, void* insertLeftArgument,
                              void* updateFirstArgument, void* valueArgument) {
    void* savedTree = treeArgument;
    void* tree = treeArgument;
    void* parent = parentArgument;
    void* insertLeft = insertLeftArgument;
    void* updateFirst = updateFirstArgument;
    void* value = valueArgument;
    void* savedInsertLeft = insertLeftArgument;
    void* savedUpdateFirst = updateFirstArgument;
    void* unusedRegister;
    (void)unusedRegister;
    if (AsFileRegistryTree(tree)->count == 0xFFFFFFFFu) {
        TreeInsertLengthError error;
    }

    // Phase 1: guard the raw allocation while deep-copying the key/file payload.
    Allocation804177CC allocation = AllocateNode();
    void* base = allocation.ptr;
    NodeStorageGuard guard((char*)&AsFileRegistryTree(tree)->root, base);
    void* node = (char*)&AsFileRegistryNode(base)->stringFlags;
    new (node) FileRegistryValue(*(FileRegistryValue*)value);
    base = guard.ptr;
    (*(unsigned int*)&AsFileRegistryNode(base)->right) = 0;
    guard.ptr = 0;
    (*(unsigned int*)&AsFileRegistryNode(base)->left) = 0;
    void* childSlot = (char*)&AsFileRegistryNode(base)->parentAndColor;
    new (childSlot) ParentLink(parent);
    // Phase 2: attach to the selected parent link, then repair and publish first-node cache.
    if (insertLeft != 0) {
        AsFileRegistryNode(parent)->left = base;
    } else {
        AsFileRegistryNode(parent)->right = base;
    }
    unsigned int nodeCount = AsFileRegistryTree(tree)->count;
    AsFileRegistryTree(tree)->count = nodeCount + 1;
    FileRegistry_RebalanceAfterInsert(base, AsFileRegistryTree(tree)->root);
    if (updateFirst != 0) {
        AsFileRegistryTree(tree)->first = base;
    }
    return base;
}
#pragma use_lmw_stmw reset
#pragma use_lmw_stmw reset
#pragma opt_common_subs reset
#pragma opt_propagation reset
#pragma opt_dead_code reset
#pragma opt_lifetimes reset
#pragma opt_pointer_analysis reset
#pragma opt_branch_folding reset

#pragma opt_common_subs off
#pragma opt_propagation on
#pragma opt_dead_code off
#pragma opt_pointer_analysis off
class FileIoAutoLock {
    OSMutex* mutex;
    void* table;

public:
    // Lock the recursive IO mutex and install the auto-lock dispatch table.
    FileIoAutoLock(void* p, int) {
        table = s_fileIoAutoLockMutexVTable;
        mutex = (OSMutex*)p;
        OSLockMutex(mutex);
    }
    // Lock the recursive IO mutex using the same non-deleting guard form.
    FileIoAutoLock(void* p) : mutex((OSMutex*)p), table(s_fileIoAutoLockMutexVTable) {
        OSLockMutex(mutex);
    }
    // Release one recursion level of the borrowed IO mutex.
    ~FileIoAutoLock() {
        table = s_fileIoAutoLockMutexVTable;
        OSUnlockMutex(mutex);
    }
};

int FileIO_SplitDeviceAndNormalizePath(const char*, char*, unsigned int, char*, unsigned int);
// This view describes only mutex storage, not a combined global object.
// Registry and default path have their own storage/symbols in both build forms.
struct FileIoMutexStorageView {
    char mutex[0x24];
};

#pragma opt_propagation off
#pragma opt_common_subs on
// Return prefix+suffix byte count including NUL; null dst queries size only.
// Non-null dst must have enough storage: this helper does not bound the copies.
inline int JoinPathUnderLock(FileIoMutexStorageView& state, char* dst, const char* prefix, const char* suffix) {
    OSMutex* const mutex = (OSMutex*)state.mutex;
    OSLockMutex((OSMutex*)mutex);
    size_t n = strlen(suffix);
    n = strlen(prefix) + n;
    if ((unsigned int)dst == 0) {
        OSUnlockMutex((OSMutex*)mutex);
        return n + 1;
    }
    strcpy(dst, prefix);
    strcat(dst, suffix);
    OSUnlockMutex((OSMutex*)mutex);
    return n + 1;
}

#pragma opt_common_subs off
// A registered device object. Only slot 33 (getPath) is called; slot00..slot32 are placeholders that put it at the
// right vtable index.
// The file path table is an 8-byte header + 4*33 bytes, one entry per slot.
class FileIoDeviceNode {
public:
    virtual void slot00();
    virtual void slot01();
    virtual void slot02();
    virtual void slot03();
    virtual void slot04();
    virtual void slot05();
    virtual void slot06();
    virtual void slot07();
    virtual void slot08();
    virtual void slot09();
    virtual void slot10();
    virtual void slot11();
    virtual void slot12();
    virtual void slot13();
    virtual void slot14();
    virtual void slot15();
    virtual void slot16();
    virtual void slot17();
    virtual void slot18();
    virtual void slot19();
    virtual void slot20();
    virtual void slot21();
    virtual void slot22();
    virtual void slot23();
    virtual void slot24();
    virtual void slot25();
    virtual void slot26();
    virtual void slot27();
    virtual void slot28();
    virtual void slot29();
    virtual void slot30();
    virtual void slot31();
    virtual void slot32();
    virtual void getPath(char*, unsigned int);
};
#pragma opt_branch_folding off
// Read the key byte length from its inline tag or heap-string length.
inline unsigned int PathNodeStringLength(void* node) {
    if (AsFileRegistryNode(node)->stringFlags >> 31)
        return AsFileRegistryNode(node)->stringLength;
    return (*(unsigned char*)&AsFileRegistryNode(node)->stringFlags) & kInlineStringLengthMask;
}
// Borrow the key byte address without transferring inline or heap storage.
inline char* PathNodeStringData(void* node) {
    if (AsFileRegistryNode(node)->stringFlags >> 31)
        return AsFileRegistryNode(node)->stringData;
    return (char*)&AsFileRegistryNode(node)->stringFlags + 1; // PathNodeStringData: inline string +0xD
}
// GetDevicePath: a found file and a missing file share the caller continuation.
// Walks the registry in key order starting at the first node: a node matches when its key is a prefix of pathbuf
// (e.g. a device name); on a match node = its file/device pointer, on no match node = 0, and the mutex is released.
// The inner loops are the usual tree successor step: if there is a right child, go to its
// leftmost descendant (*(void**)r is the left child); otherwise climb while the node is the right child of its parent
// (parentAndColor with the red bit cleared).
// Consume one lookup-lock recursion level on every exit; the outer RAII lock
// remains held while GetDevicePath calls the borrowed device and joins the path.
// node is the only output. Traversal scratch is local to the lookup.
inline void FindPathDeviceAndUnlock(void*& node, void*& sentinel, void* const& mutex2, const char* pathbuf) {
    unsigned int len;
    void* right;
    void* r;
    void* child;
    void* parent;
    while (node != sentinel) {
        len = PathNodeStringLength(node);
        if (strncmp(pathbuf, PathNodeStringData(node), len) == 0) {
            node = AsFileRegistryNode(node)->file;
            OSUnlockMutex((OSMutex*)mutex2);
            return;
        }
        right = AsFileRegistryNode(node)->right;
        if (right) {
            r = right;
            while ((child = *(void**)r) != 0)
                r = child;
            node = r;
        } else {
            while (node !=
                   *(void**)(parent = (void*)(AsFileRegistryNode(node)->parentAndColor & ~kRegistryNodeColorBit)))
                node = parent;
            node = parent;
        }
    }
    OSUnlockMutex((OSMutex*)mutex2);
    node = 0;
}

// Resolve a registered device prefix and join its virtual path; return 0 on miss.
// Nested locks are intentional: the platform mutex supports recursive locking.
inline int GetDevicePath(const char* pathbuf, char* outpath) {
    FileIoAutoLock inner(s_fileIoMutex);
    void* const mutex2 = s_fileIoMutex;
    OSLockMutex((OSMutex*)mutex2);

    char* tree = s_fileRegistryTree;
    void* sentinel = tree + 4;
    void* node = AsFileRegistryTree(tree)->first;
    FindPathDeviceAndUnlock(node, sentinel, mutex2, pathbuf);
    if (node == 0)
        return 0;
    char buf[kFilePathBufferBytes];
    memset(buf, 0, kFilePathBufferBytes);
    ((FileIoDeviceNode*)node)->getPath(buf, kFilePathBufferBytes);

    return JoinPathUnderLock(*(FileIoMutexStorageView*)s_fileIoMutex, outpath, pathbuf, buf);
}

#pragma use_lmw_stmw on
#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_dead_code off
#pragma opt_pointer_analysis off
#pragma opt_branch_folding off
#pragma opt_lifetimes off
#pragma opt_propagation on
#pragma opt_branch_folding on
// GetDefaultPath: a found file and a missing file share the caller continuation.
inline void FindDefaultDeviceAndUnlock(void*& node, void* const& sentinel, void* const& mutex, const char* path) {
    while (node != sentinel) {
        unsigned int len;
        if (!(AsFileRegistryNode(node)->stringFlags >> 31))
            len = (*(unsigned char*)&AsFileRegistryNode(node)->stringFlags) & kInlineStringLengthMask;
        else
            len = AsFileRegistryNode(node)->stringLength;
        const char* text;
        if (!(AsFileRegistryNode(node)->stringFlags >> 31))
            text = (char*)&AsFileRegistryNode(node)->stringFlags + 1; // FileIO_ResolveFullPath: inline string +0xD
        else
            text = AsFileRegistryNode(node)->stringData;
        if (strncmp(path, text, len) == 0) {
            node = AsFileRegistryNode(node)->file;
            OSUnlockMutex((OSMutex*)mutex);
            return;
        }
        void* r = AsFileRegistryNode(node)->right;
        if (r) {
            void* child;
            while ((child = *(void**)r) != 0)
                r = child;
            node = r;
        } else {
            void* parent;
            while (node !=
                   *(void**)(parent = (void*)(AsFileRegistryNode(node)->parentAndColor & ~kRegistryNodeColorBit)))
                node = parent;
            node = parent;
        }
    }
    OSUnlockMutex((OSMutex*)mutex);
    node = 0;
}

// Resolve the supplied default prefix and return its joined path size or 0 on miss.
inline int GetDefaultPath(const char* path, char* dst) {
    FileIoAutoLock inner(s_fileIoMutex, 0);
    void* const mutex = s_fileIoMutex;
    OSLockMutex((OSMutex*)mutex);
    char* tree = s_fileRegistryTree;
    void* const sentinel = tree + 4;
    void* node = AsFileRegistryTree(tree)->first;
    FindDefaultDeviceAndUnlock(node, sentinel, mutex, path);
    if (node == 0)
        return 0;
    char buf[kFilePathBufferBytes];
    memset(buf, 0, sizeof(buf));
    ((FileIoDeviceNode*)node)->getPath(buf, sizeof(buf));
    return JoinPathUnderLock(*(FileIoMutexStorageView*)s_fileIoMutex, dst, path, buf);
}
#pragma opt_common_subs off
#pragma opt_branch_folding off
#pragma opt_propagation off
// Resolve the registered default device prefix to its full path under the recursive mutex.
int FileIO_GetDefaultDevicePath(int, void* dst) {
    int result;
    {
        FileIoAutoLock outer(s_fileIoMutex, 0);
        result = GetDefaultPath(s_defaultDevicePath, (char*)dst);
    }
    return result;
}
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma opt_common_subs off
#pragma opt_dead_code reset
#pragma opt_pointer_analysis reset
#pragma opt_branch_folding reset
// Resolve a device prefix, normalize its relative path, and return the required output size.
// Flow: the text before ':' (if any) selects the device prefix; FileIO_SplitDeviceAndNormalizePath (external,
// not verified here) fills pathbuf with the device part and deviceRelativePath with the normalized rest
// (path cleanup); JoinPathUnderLock then builds outBuf from the two. outBuf receives the joined
// full path. A path with no ':' uses the default device prefix (s_defaultDevicePath). A trailing '/' on the input is
// kept (re-appended after normalizing) unless the result is just "/". Returns 0 on failure, otherwise the size
// needed for the full path; with outBuf == 0 only the size is returned. If filenameStart is non-null it receives
// the ADDRESS (not an offset) of the character after the last '/' inside outBuf.
int FileIO_ResolveFullPath(const char* path, int /*unused*/, void* outBuf, int* filenameStart) {
    FileIoAutoLock outer(s_fileIoMutex);
    char pathbuf[kFilePathBufferBytes];
    char outpath[kFilePathBufferBytes];
    char* colon = strchr(path, ':');
    if (colon) {
        strcpy(pathbuf, (char*)path);
        colon = strchr(pathbuf, ':');
        colon[1] = 0;
    } else {
        strcpy(pathbuf, s_defaultDevicePath);
    }
    GetDevicePath(pathbuf, outpath);
    colon = strchr(outpath, ':');
    char* const deviceRelativePath = colon ? colon + 1 : outpath;

    if (!FileIO_SplitDeviceAndNormalizePath(path, pathbuf, kFilePathBufferBytes, deviceRelativePath,
                                            kFilePathBufferBytes))
        return 0;

    char* inputPath = (char*)path;
    size_t inputLength = strlen(inputPath);
    size_t relativeLength = strlen(deviceRelativePath);
    if (inputLength != 0 && inputPath[inputLength - 1] == '/' && relativeLength > 1) {
        if (relativeLength + 1 >= kFilePathBufferBytes)
            return 0;
        strcat(deviceRelativePath, fileIoPathStrings.separator);
    }

    // Normalize first; only then join, report size, and optionally return filename position.
    int requiredBytes =
        JoinPathUnderLock(*(FileIoMutexStorageView*)s_fileIoMutex, (char*)outBuf, pathbuf, deviceRelativePath);
    if (outBuf == 0)
        return requiredBytes;
    if (filenameStart) {
        char* slash = strrchr((char*)outBuf, '/');
        *filenameStart = (int)(slash + 1);
    }
    return requiredBytes;
}
#pragma use_lmw_stmw reset
#pragma opt_common_subs reset
#pragma opt_propagation reset
#pragma opt_dead_code reset
#pragma opt_pointer_analysis reset
#pragma opt_branch_folding reset

#pragma opt_common_subs off
// Set only the tagged color bit; preserve the node's parent address.
static inline void PaintRegistryNodeRed(void*& node, unsigned int& links) {
    links = AsFileRegistryNode(node)->parentAndColor;
    links |= kRegistryNodeColorBit;
    AsFileRegistryNode(node)->parentAndColor = links;
}
// Clear only the tagged color bit; preserve the node's parent address.
static inline void PaintRegistryNodeBlack(void*& node, unsigned int& links) {
    links = AsFileRegistryNode(node)->parentAndColor;
    links &= ~kRegistryNodeColorBit;
    AsFileRegistryNode(node)->parentAndColor = links;
}

// Restore red-black invariants using mirrored rotations and a low-bit parent color tag.
// Node field `parentAndColor` = parent pointer | color bit (kRegistryNodeColorBit set = red). Names below:
//   node = current (red) node, parentOrLink = its parent, uncleOrChild = grandparent / uncle / moved child
//   (reused for several roles), rotationPivot = grandparent or the node rotated up, root = tree root.
// Structure (standard insertion fix-up, written out twice as mirror images):
//   while node is not root and its parent is red:
//     parent is LEFT child of grandparent:  (1) red uncle -> recolor, node = grandparent, loop
//                                           (2) black uncle, node is right child -> rotate parent left
//                                           (3) black uncle, node is left child -> recolor, rotate grandparent right
//     parent is RIGHT child of grandparent: mirror of (1)-(3) with left/right swapped
//   finally the root is painted black.
// Loop invariant: node is red; only node and its parent can both be red.
// Repair the red-parent violation with mirrored recoloring and rotations, then blacken root.
void FileRegistry_RebalanceAfterInsert(void* insertedNode, unsigned int rootAddress) {
    void* node;
    void* root;
    void* parentOrLink;
    void* uncleOrChild;
    void* rotationPivot;
    unsigned int parentLinks;
    // Phase 1: insert red; only a red parent can violate the red-black invariant.
    node = insertedNode;
    root = (void*)rootAddress;
    PaintRegistryNodeRed(node, parentLinks);
    // 0x80417E90: red-parent cases are mirrored around the grandparent.
    while (node != root && (parentLinks = AsFileRegistryNode(node)->parentAndColor,
                            parentOrLink = (void*)(parentLinks & ~kRegistryNodeColorBit),
                            parentLinks = AsFileRegistryNode(parentOrLink)->parentAndColor,
                            parentLinks &= kRegistryNodeColorBit, parentLinks == kRegistryNodeColorBit)) {
        parentLinks = AsFileRegistryNode(parentOrLink)->parentAndColor;
        uncleOrChild = (void*)(parentLinks & ~kRegistryNodeColorBit);
        rotationPivot = AsFileRegistryNode(uncleOrChild)->left;
        // Parent is the grandparent's left child; uncle is on the right.
        if (parentOrLink == rotationPivot) {
            uncleOrChild = AsFileRegistryNode(uncleOrChild)->right;
            // Case 1 (red uncle): recolor parent/uncle black and grandparent red, then climb.
            if (uncleOrChild != 0 && (parentLinks = AsFileRegistryNode(uncleOrChild)->parentAndColor,
                                      parentLinks &= kRegistryNodeColorBit, parentLinks == kRegistryNodeColorBit)) {
                PaintRegistryNodeBlack(parentOrLink, parentLinks);
                PaintRegistryNodeBlack(uncleOrChild, parentLinks);
                parentLinks = AsFileRegistryNode(node)->parentAndColor;
                node = (void*)(parentLinks & ~kRegistryNodeColorBit);
                parentLinks = AsFileRegistryNode(node)->parentAndColor;
                node = (void*)(parentLinks & ~kRegistryNodeColorBit);
                PaintRegistryNodeRed(node, parentLinks);
                continue;
            }

            // Case 2 (black/missing uncle): inner (left-right) case rotates parent left first.
            rotationPivot = AsFileRegistryNode(parentOrLink)->right;
            if (node == rotationPivot) {
                node = parentOrLink;
                if (root == parentOrLink)
                    root = rotationPivot; // 0x80417F24: rotating the root
                parentLinks = (*(unsigned int*)&AsFileRegistryNode(rotationPivot)->left);
                (*(unsigned int*)&AsFileRegistryNode(parentOrLink)->right) = parentLinks;
                uncleOrChild = AsFileRegistryNode(rotationPivot)->left;
                if (uncleOrChild != 0) {
                    parentLinks = AsFileRegistryNode(uncleOrChild)->parentAndColor;
                    parentLinks &= kRegistryNodeColorBit;
                    parentLinks = (unsigned int)parentOrLink | parentLinks;
                    AsFileRegistryNode(uncleOrChild)->parentAndColor = parentLinks;
                }
                parentLinks = AsFileRegistryNode(rotationPivot)->parentAndColor;
                uncleOrChild = (void*)AsFileRegistryNode(parentOrLink)->parentAndColor;
                parentLinks &= kRegistryNodeColorBit;
                parentLinks |= ((unsigned int)uncleOrChild & ~kRegistryNodeColorBit);
                AsFileRegistryNode(rotationPivot)->parentAndColor = parentLinks;
                parentLinks = AsFileRegistryNode(parentOrLink)->parentAndColor;
                uncleOrChild = (void*)(parentLinks & ~kRegistryNodeColorBit);
                parentLinks = (*(unsigned int*)&AsFileRegistryNode(uncleOrChild)->left);
                if (parentOrLink == (void*)parentLinks) {
                    (*(unsigned int*)&AsFileRegistryNode(uncleOrChild)->left) = (unsigned int)rotationPivot;
                } else {
                    (*(unsigned int*)&AsFileRegistryNode(uncleOrChild)->right) = (unsigned int)rotationPivot;
                }
                (*(unsigned int*)&AsFileRegistryNode(rotationPivot)->left) = (unsigned int)parentOrLink;
                parentLinks = AsFileRegistryNode(parentOrLink)->parentAndColor;
                parentLinks &= kRegistryNodeColorBit;
                parentLinks = (unsigned int)rotationPivot | parentLinks;
                AsFileRegistryNode(parentOrLink)->parentAndColor = parentLinks;
            }
            // Case 3 (outer left-left): parent black, grandparent red; rotate grandparent right.
            parentLinks = AsFileRegistryNode(node)->parentAndColor;
            parentOrLink = (void*)(parentLinks & ~kRegistryNodeColorBit);
            PaintRegistryNodeBlack(parentOrLink, parentLinks);
            parentLinks = AsFileRegistryNode(node)->parentAndColor;
            parentOrLink = (void*)(parentLinks & ~kRegistryNodeColorBit);
            parentLinks = AsFileRegistryNode(parentOrLink)->parentAndColor;
            parentOrLink = (void*)(parentLinks & ~kRegistryNodeColorBit);
            PaintRegistryNodeRed(parentOrLink, parentLinks);
            parentLinks = AsFileRegistryNode(node)->parentAndColor;
            parentOrLink = (void*)(parentLinks & ~kRegistryNodeColorBit);
            parentLinks = AsFileRegistryNode(parentOrLink)->parentAndColor;
            rotationPivot = (void*)(parentLinks & ~kRegistryNodeColorBit);
            uncleOrChild = AsFileRegistryNode(rotationPivot)->left;
            if (root == rotationPivot)
                root = uncleOrChild; // 0x80417FE0
            parentLinks = (*(unsigned int*)&AsFileRegistryNode(uncleOrChild)->right);
            (*(unsigned int*)&AsFileRegistryNode(rotationPivot)->left) = parentLinks;
            parentOrLink = AsFileRegistryNode(uncleOrChild)->right;
            if (parentOrLink != 0) {
                parentLinks = AsFileRegistryNode(parentOrLink)->parentAndColor;
                parentLinks &= kRegistryNodeColorBit;
                parentLinks = (unsigned int)rotationPivot | parentLinks;
                AsFileRegistryNode(parentOrLink)->parentAndColor = parentLinks;
            }
            parentLinks = AsFileRegistryNode(uncleOrChild)->parentAndColor;
            parentOrLink = (void*)AsFileRegistryNode(rotationPivot)->parentAndColor;
            parentLinks &= kRegistryNodeColorBit;
            parentLinks |= ((unsigned int)parentOrLink & ~kRegistryNodeColorBit);
            AsFileRegistryNode(uncleOrChild)->parentAndColor = parentLinks;
            parentLinks = AsFileRegistryNode(rotationPivot)->parentAndColor;
            parentOrLink = (void*)(parentLinks & ~kRegistryNodeColorBit);
            parentLinks = (*(unsigned int*)&AsFileRegistryNode(parentOrLink)->left);
            if (rotationPivot == (void*)parentLinks) {
                (*(unsigned int*)&AsFileRegistryNode(parentOrLink)->left) = (unsigned int)uncleOrChild;
            } else {
                (*(unsigned int*)&AsFileRegistryNode(parentOrLink)->right) = (unsigned int)uncleOrChild;
            }
            (*(unsigned int*)&AsFileRegistryNode(uncleOrChild)->right) = (unsigned int)rotationPivot;
            parentLinks = AsFileRegistryNode(rotationPivot)->parentAndColor;
            parentLinks &= kRegistryNodeColorBit;
            parentLinks = (unsigned int)uncleOrChild | parentLinks;
            AsFileRegistryNode(rotationPivot)->parentAndColor = parentLinks;
            continue;
        } else {
            // Mirrored right-parent case: red left uncle only needs recoloring.
            if (rotationPivot != 0 && (parentLinks = AsFileRegistryNode(rotationPivot)->parentAndColor,
                                       parentLinks &= kRegistryNodeColorBit, parentLinks == kRegistryNodeColorBit)) {
                PaintRegistryNodeBlack(parentOrLink, parentLinks);
                parentLinks = AsFileRegistryNode(rotationPivot)->parentAndColor;
                parentLinks &= ~kRegistryNodeColorBit;
                AsFileRegistryNode(rotationPivot)->parentAndColor = parentLinks;
                parentLinks = AsFileRegistryNode(node)->parentAndColor;
                node = (void*)(parentLinks & ~kRegistryNodeColorBit);
                parentLinks = AsFileRegistryNode(node)->parentAndColor;
                node = (void*)(parentLinks & ~kRegistryNodeColorBit);
                PaintRegistryNodeRed(node, parentLinks);
                continue;
            }

            // Inner (right-left) case: rotate parent right to make an outer case.
            rotationPivot = AsFileRegistryNode(parentOrLink)->left;
            if (node == rotationPivot) {
                node = parentOrLink;
                if (root == parentOrLink)
                    root = rotationPivot; // 0x804180BC
                parentLinks = (*(unsigned int*)&AsFileRegistryNode(rotationPivot)->right);
                (*(unsigned int*)&AsFileRegistryNode(parentOrLink)->left) = parentLinks;
                uncleOrChild = AsFileRegistryNode(rotationPivot)->right;
                if (uncleOrChild != 0) {
                    parentLinks = AsFileRegistryNode(uncleOrChild)->parentAndColor;
                    parentLinks &= kRegistryNodeColorBit;
                    parentLinks = (unsigned int)parentOrLink | parentLinks;
                    AsFileRegistryNode(uncleOrChild)->parentAndColor = parentLinks;
                }
                parentLinks = AsFileRegistryNode(rotationPivot)->parentAndColor;
                uncleOrChild = (void*)AsFileRegistryNode(parentOrLink)->parentAndColor;
                parentLinks &= kRegistryNodeColorBit;
                parentLinks |= ((unsigned int)uncleOrChild & ~kRegistryNodeColorBit);
                AsFileRegistryNode(rotationPivot)->parentAndColor = parentLinks;
                parentLinks = AsFileRegistryNode(parentOrLink)->parentAndColor;
                uncleOrChild = (void*)(parentLinks & ~kRegistryNodeColorBit);
                parentLinks = (*(unsigned int*)&AsFileRegistryNode(uncleOrChild)->left);
                if (parentOrLink == (void*)parentLinks) {
                    (*(unsigned int*)&AsFileRegistryNode(uncleOrChild)->left) = (unsigned int)rotationPivot;
                } else {
                    (*(unsigned int*)&AsFileRegistryNode(uncleOrChild)->right) = (unsigned int)rotationPivot;
                }
                (*(unsigned int*)&AsFileRegistryNode(rotationPivot)->right) = (unsigned int)parentOrLink;
                parentLinks = AsFileRegistryNode(parentOrLink)->parentAndColor;
                parentLinks &= kRegistryNodeColorBit;
                parentLinks = (unsigned int)rotationPivot | parentLinks;
                AsFileRegistryNode(parentOrLink)->parentAndColor = parentLinks;
            }
            // Outer right-right case: recolor, then rotate grandparent left.
            parentLinks = AsFileRegistryNode(node)->parentAndColor;
            parentOrLink = (void*)(parentLinks & ~kRegistryNodeColorBit);
            PaintRegistryNodeBlack(parentOrLink, parentLinks);
            parentLinks = AsFileRegistryNode(node)->parentAndColor;
            parentOrLink = (void*)(parentLinks & ~kRegistryNodeColorBit);
            parentLinks = AsFileRegistryNode(parentOrLink)->parentAndColor;
            parentOrLink = (void*)(parentLinks & ~kRegistryNodeColorBit);
            PaintRegistryNodeRed(parentOrLink, parentLinks);
            parentLinks = AsFileRegistryNode(node)->parentAndColor;
            parentOrLink = (void*)(parentLinks & ~kRegistryNodeColorBit);
            parentLinks = AsFileRegistryNode(parentOrLink)->parentAndColor;
            rotationPivot = (void*)(parentLinks & ~kRegistryNodeColorBit);
            uncleOrChild = AsFileRegistryNode(rotationPivot)->right;
            if (root == rotationPivot)
                root = uncleOrChild; // 0x80418178
            parentLinks = (*(unsigned int*)&AsFileRegistryNode(uncleOrChild)->left);
            (*(unsigned int*)&AsFileRegistryNode(rotationPivot)->right) = parentLinks;
            parentOrLink = AsFileRegistryNode(uncleOrChild)->left;
            if (parentOrLink != 0) {
                parentLinks = AsFileRegistryNode(parentOrLink)->parentAndColor;
                parentLinks &= kRegistryNodeColorBit;
                parentLinks = (unsigned int)rotationPivot | parentLinks;
                AsFileRegistryNode(parentOrLink)->parentAndColor = parentLinks;
            }
            parentLinks = AsFileRegistryNode(uncleOrChild)->parentAndColor;
            parentOrLink = (void*)AsFileRegistryNode(rotationPivot)->parentAndColor;
            parentLinks &= kRegistryNodeColorBit;
            parentLinks |= ((unsigned int)parentOrLink & ~kRegistryNodeColorBit);
            AsFileRegistryNode(uncleOrChild)->parentAndColor = parentLinks;
            parentLinks = AsFileRegistryNode(rotationPivot)->parentAndColor;
            parentOrLink = (void*)(parentLinks & ~kRegistryNodeColorBit);
            parentLinks = (*(unsigned int*)&AsFileRegistryNode(parentOrLink)->left);
            if (rotationPivot == (void*)parentLinks) {
                (*(unsigned int*)&AsFileRegistryNode(parentOrLink)->left) = (unsigned int)uncleOrChild;
            } else {
                (*(unsigned int*)&AsFileRegistryNode(parentOrLink)->right) = (unsigned int)uncleOrChild;
            }
            (*(unsigned int*)&AsFileRegistryNode(uncleOrChild)->left) = (unsigned int)rotationPivot;
            parentLinks = AsFileRegistryNode(rotationPivot)->parentAndColor;
            parentLinks &= kRegistryNodeColorBit;
            parentLinks = (unsigned int)uncleOrChild | parentLinks;
            AsFileRegistryNode(rotationPivot)->parentAndColor = parentLinks;
            continue;
        }
    }
    // Phase 3: the root must finish black, even after recoloring up to the top.
    PaintRegistryNodeBlack(root, parentLinks);
}
#pragma opt_common_subs reset

#pragma use_lmw_stmw on
#pragma opt_common_subs off
#pragma inline_depth(0)
#pragma opt_lifetimes off
#pragma opt_propagation off
// Destroy registry keys and nodes in postorder; the first levels are explicitly unrolled.
// Precondition: subtree is a real, non-null node, never the embedded sentinel.
// Visit left, right, then key and node; three explicit levels precede recursion.
// Repeated member-address checks are destructor expansion, not extra ownership.
// inline_depth(0) on this body preserves calls at the recursive leaves.
// Heap keys use g_fileIoFreeCallback; the borrowed node->file is never destroyed.
void FileRegistry_DestroySubtree(void* allocator, void* subtree) {
    void* const savedAllocator = allocator;
    void* const root = subtree;
    void* branchA = AsFileRegistryNode(root)->left;
    void* branchB;
    void* branchC;
    // Phase 1: retire the root's left half, descending left before right.
    if (branchA) {
        branchB = AsFileRegistryNode(branchA)->left;
        if (branchB) {
            branchC = AsFileRegistryNode(branchB)->left;
            if (branchC != 0) {
                void* child = AsFileRegistryNode(branchC)->left;
                if (child)
                    FileRegistry_DestroySubtree(savedAllocator, child);
                child = AsFileRegistryNode(branchC)->right;
                if (child)
                    FileRegistry_DestroySubtree(savedAllocator, child);
                FileRegistryString::ReleaseStorage((int)savedAllocator,
                                                   (FileRegistryString*)&AsFileRegistryNode(branchC)->stringFlags);
                g_fileIoFreeCallback(branchC);
            }
            branchC = AsFileRegistryNode(branchB)->right;
            if (branchC != 0) {
                void* child = AsFileRegistryNode(branchC)->left;
                if (child)
                    FileRegistry_DestroySubtree(savedAllocator, child);
                child = AsFileRegistryNode(branchC)->right;
                if (child)
                    FileRegistry_DestroySubtree(savedAllocator, child);
                FileRegistryString::ReleaseStorage((int)savedAllocator,
                                                   (FileRegistryString*)&AsFileRegistryNode(branchC)->stringFlags);
                g_fileIoFreeCallback(branchC);
            }
            if ((char*)&AsFileRegistryNode(branchB)->stringFlags) {
                if ((char*)&AsFileRegistryNode(branchB)->stringFlags) {
                    if (AsFileRegistryNode(branchB)->stringFlags >> 31)
                        g_fileIoFreeCallback((*(void**)&AsFileRegistryNode(branchB)->stringData));
                }
            }
            g_fileIoFreeCallback(branchB);
        }
        branchC = AsFileRegistryNode(branchA)->right;
        if (branchC) {
            branchB = AsFileRegistryNode(branchC)->left;
            if (branchB != 0) {
                void* child = AsFileRegistryNode(branchB)->left;
                if (child)
                    FileRegistry_DestroySubtree(savedAllocator, child);
                child = AsFileRegistryNode(branchB)->right;
                if (child)
                    FileRegistry_DestroySubtree(savedAllocator, child);
                FileRegistryString::ReleaseStorage((int)savedAllocator,
                                                   (FileRegistryString*)&AsFileRegistryNode(branchB)->stringFlags);
                g_fileIoFreeCallback(branchB);
            }
            branchB = AsFileRegistryNode(branchC)->right;
            if (branchB != 0) {
                void* child = AsFileRegistryNode(branchB)->left;
                if (child)
                    FileRegistry_DestroySubtree(savedAllocator, child);
                child = AsFileRegistryNode(branchB)->right;
                if (child)
                    FileRegistry_DestroySubtree(savedAllocator, child);
                FileRegistryString::ReleaseStorage((int)savedAllocator,
                                                   (FileRegistryString*)&AsFileRegistryNode(branchB)->stringFlags);
                g_fileIoFreeCallback(branchB);
            }
            if ((char*)&AsFileRegistryNode(branchC)->stringFlags) {
                if ((char*)&AsFileRegistryNode(branchC)->stringFlags) {
                    if (AsFileRegistryNode(branchC)->stringFlags >> 31)
                        g_fileIoFreeCallback((*(void**)&AsFileRegistryNode(branchC)->stringData));
                }
            }
            g_fileIoFreeCallback(branchC);
        }
        if ((char*)&AsFileRegistryNode(branchA)->stringFlags) {
            if ((char*)&AsFileRegistryNode(branchA)->stringFlags) {
                if (AsFileRegistryNode(branchA)->stringFlags >> 31)
                    g_fileIoFreeCallback((*(void**)&AsFileRegistryNode(branchA)->stringData));
            }
        }
        g_fileIoFreeCallback(branchA);
    }
    // Left subtree is gone; apply the same postorder ownership rule on the right.
    // Phase 2: retire the right half. The compiler-expanded temporaries now
    // reuse branchC for depth 1 and branchA for depth 3 (opposite the left half).
    branchC = AsFileRegistryNode(root)->right;
    if (branchC) {
        branchB = AsFileRegistryNode(branchC)->left;
        if (branchB) {
            branchA = AsFileRegistryNode(branchB)->left;
            if (branchA != 0) {
                void* child = AsFileRegistryNode(branchA)->left;
                if (child)
                    FileRegistry_DestroySubtree(savedAllocator, child);
                child = AsFileRegistryNode(branchA)->right;
                if (child)
                    FileRegistry_DestroySubtree(savedAllocator, child);
                FileRegistryString::ReleaseStorage((int)savedAllocator,
                                                   (FileRegistryString*)&AsFileRegistryNode(branchA)->stringFlags);
                g_fileIoFreeCallback(branchA);
            }
            branchA = AsFileRegistryNode(branchB)->right;
            if (branchA != 0) {
                void* child = AsFileRegistryNode(branchA)->left;
                if (child)
                    FileRegistry_DestroySubtree(savedAllocator, child);
                child = AsFileRegistryNode(branchA)->right;
                if (child)
                    FileRegistry_DestroySubtree(savedAllocator, child);
                FileRegistryString::ReleaseStorage((int)savedAllocator,
                                                   (FileRegistryString*)&AsFileRegistryNode(branchA)->stringFlags);
                g_fileIoFreeCallback(branchA);
            }
            if ((char*)&AsFileRegistryNode(branchB)->stringFlags) {
                if ((char*)&AsFileRegistryNode(branchB)->stringFlags) {
                    if (AsFileRegistryNode(branchB)->stringFlags >> 31)
                        g_fileIoFreeCallback((*(void**)&AsFileRegistryNode(branchB)->stringData));
                }
            }
            g_fileIoFreeCallback(branchB);
        }
        branchB = AsFileRegistryNode(branchC)->right;
        if (branchB) {
            branchA = AsFileRegistryNode(branchB)->left;
            if (branchA != 0) {
                void* child = AsFileRegistryNode(branchA)->left;
                if (child)
                    FileRegistry_DestroySubtree(savedAllocator, child);
                child = AsFileRegistryNode(branchA)->right;
                if (child)
                    FileRegistry_DestroySubtree(savedAllocator, child);
                FileRegistryString::ReleaseStorage((int)savedAllocator,
                                                   (FileRegistryString*)&AsFileRegistryNode(branchA)->stringFlags);
                g_fileIoFreeCallback(branchA);
            }
            branchA = AsFileRegistryNode(branchB)->right;
            if (branchA != 0) {
                void* child = AsFileRegistryNode(branchA)->left;
                if (child)
                    FileRegistry_DestroySubtree(savedAllocator, child);
                child = AsFileRegistryNode(branchA)->right;
                if (child)
                    FileRegistry_DestroySubtree(savedAllocator, child);
                FileRegistryString::ReleaseStorage((int)savedAllocator,
                                                   (FileRegistryString*)&AsFileRegistryNode(branchA)->stringFlags);
                g_fileIoFreeCallback(branchA);
            }
            if ((char*)&AsFileRegistryNode(branchB)->stringFlags) {
                if ((char*)&AsFileRegistryNode(branchB)->stringFlags) {
                    if (AsFileRegistryNode(branchB)->stringFlags >> 31)
                        g_fileIoFreeCallback((*(void**)&AsFileRegistryNode(branchB)->stringData));
                }
            }
            g_fileIoFreeCallback(branchB);
        }
        if ((char*)&AsFileRegistryNode(branchC)->stringFlags) {
            if ((char*)&AsFileRegistryNode(branchC)->stringFlags) {
                if (AsFileRegistryNode(branchC)->stringFlags >> 31)
                    g_fileIoFreeCallback((*(void**)&AsFileRegistryNode(branchC)->stringData));
            }
        }
        g_fileIoFreeCallback(branchC);
    }
    // Phase 3: release the root's heap key, then the root; device payload is borrowed.
    if ((char*)&AsFileRegistryNode(root)->stringFlags) {
        if ((char*)&AsFileRegistryNode(root)->stringFlags) {
            if (AsFileRegistryNode(root)->stringFlags >> 31)
                g_fileIoFreeCallback((*(void**)&AsFileRegistryNode(root)->stringData));
        }
    }
    g_fileIoFreeCallback(root);
}
#pragma opt_propagation reset
#pragma inline_depth reset
#pragma opt_lifetimes reset
#pragma opt_common_subs reset

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462718", _ctors_u804170C0_0, FileRegistry_InitGlobalTree);
#pragma pop
