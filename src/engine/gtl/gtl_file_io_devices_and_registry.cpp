#include "classes/FileRegistryNode.h"
#include "classes/FileHandle.h"
#include "classes/FileRegistryString.h"
#define AsFileRegistryNode(p) ((FileRegistryNode*)(p))
#include "classes/FileRegistryTree.h"
#define AsFileRegistryTree(p) ((FileRegistryTree*)(p))
extern "C" void* FileRegistryKey_dtor(void*, int);

// Normal destruction remains explicit. This owner handles exceptional exits.
// Scope guard (name from the EH5 exception-table cleanup it models): runs destroy(object, -1) on unwind unless release() was called.
struct EH5Cleanup {
    void* object;
    void (*destroy)(void*, int);
    bool active;
    // Arm destruction of a partially constructed object for exceptional exit.
    EH5Cleanup(void* p, void (*d)(void*, int)) : object(p), destroy(d), active(true) {}
    // Disarm after explicit normal-exit cleanup or ownership transfer.
    void release() { active = false; }
    // Run the registered member destructor only while this unwind owner is armed.
    ~EH5Cleanup() { if (active) destroy(object, -1); }
};

// Translation unit 0x804143A8-0x80415A88 (22 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "types.h"
#include "sdk_protos.h"

extern "C" {
extern void *(*g_fileIoAllocate)(u32);
extern void (*g_fileIoFreeCallback)(void *);
extern void *(*g_fileIoAllocatorCallback2)(void *, u32);
extern void *(*g_fileIoAllocatorCallback3)(u32);

extern char s_fileIoAutoLockMutexVTable[];
extern char gtlFileIoDeviceVtable[];
extern char __vt__Q23gtl19CFileIoDevicePADMEM[];
extern char pathSeparatorString;
extern char s_fileIoMutex[];
extern char fileIoPathStrings[];
extern char s_fileRegistryTree[];
extern void *s_fileRegistryDefaultDevice;

void __dl__FPv(void *);
// FileIOString_ReplaceRangeWithAllocator: public symbol spelling is retained for coordinator renaming.
void FileIOString_ReplaceRangeWithAllocator(void *str, u32 position, u32 count, const char *first, const char *last, void *allocatorTag);
void FileIOString_FitCapacityForLength(void *str, u32 length);
void *FileRegistry_InsertNode(void *tree, void *parentNode, unsigned char insertAsLeft, unsigned char updateFirst, void *keyAndValue);
void *FileIOString_CtorFromCString(void *str, const char *cstr);
void *gtl_CFileIoDeviceDVD_AllocateStorage(u32);
void *gtl_CFileIoDeviceHIO_AllocateStorage(u32);
void *gtl_CFileIoDeviceFMEM_AllocateStorage(u32);
void *gtl_CFileIoDevicePADMEM_AllocateStorage(u32);
void FileIOString_dtor(void *, int);
void *FileRegistry_GetOrInsertValue(void *, const void *);
void FileRegistry_DestroySubtree(void *, void *);
void *gtl_CFileIoDevicePADMEM_Ctor(void *, int);
void FileIO_ResolveFullPath(void *, u32, void *, u32);

// Ownership: registry -> nodes -> heap keys (only when flags bit 31 is set).
// Each node also references a device. FileRegistry_Clear frees keys/nodes ONLY;
// FileRegistry_ClearOwnedDevices deletes device objects first, then keys/nodes.
// Open wrappers borrow the device and own a backend handle until FileIO_CloseFile.
// Node parentAndColor packs the color in bit 0; mask it before ascending.
// The end iterator is &tree.root; an empty tree has root=0, first=&root, count=0.
//
// Device descriptors and open handles are different layouts, despite sharing offsets.
// Device80415730: opaque device descriptor type (shape not recovered); only handled through pointers here.
class Device80415730;
// Open options at 0x8..0x10 are copied verbatim to the backend; the shared handle
// header names transfer-buffer ownership and async completion fields.
struct FileIoOpenOptions { char reserved00[8]; u32 backendState, openMode, accessMode; };
struct FileIoAllocatorCallbacks {
    void* (*allocate)(u32);
    void (*free)(void*);
    void* (*allocatorCallback2)(void*, u32);
    void* (*allocatorCallback3)(u32);
};
struct PadMemoryDeviceStorage { void* vtable; FileRegistryString path; u32 channel; };
struct FileIoGlobalStorage {
    char mutex[0x18];
    char reserved18[0x0c];
    FileRegistryTree devices;
    char defaultDevicePath[0x104];
};
struct FileIoDevicePrefixPool {
    char dvd[5], host[6], fileMemory[6];
    char physicalMemory0[7], physicalMemory1[7], physicalMemory2[7], physicalMemory3[7];
};
typedef char FileIoRegistryOffset[((int)&((FileIoGlobalStorage*)0)->devices == 0x24) ? 1 : -1];
typedef char FileIoDefaultPathOffset[((int)&((FileIoGlobalStorage*)0)->defaultDevicePath == 0x30) ? 1 : -1];
typedef char FileIoHandleSize[(sizeof(FileHandle) == 0x28) ? 1 : -1];

enum FileIoConstants {
    kDeviceRegistryOffset = (int)&((FileIoGlobalStorage*)0)->devices,
    kDefaultDevicePathOffset = (int)&((FileIoGlobalStorage*)0)->defaultDevicePath,
    kFileIoPathCapacity = 0x104,
    kFileIoExtendedOpen = 0x40000000,
    kInlineStringLengthMask = 0x7f,
    kDvdPrefix = (int)&((FileIoDevicePrefixPool*)0)->dvd,
    kHioPrefix = (int)&((FileIoDevicePrefixPool*)0)->host,
    kFmemPrefix = (int)&((FileIoDevicePrefixPool*)0)->fileMemory,
    kPadMemory0Prefix = (int)&((FileIoDevicePrefixPool*)0)->physicalMemory0,
    kPadMemory1Prefix = (int)&((FileIoDevicePrefixPool*)0)->physicalMemory1,
    kPadMemory2Prefix = (int)&((FileIoDevicePrefixPool*)0)->physicalMemory2,
    kPadMemory3Prefix = (int)&((FileIoDevicePrefixPool*)0)->physicalMemory3
};

struct CAutoLockMutex {
    void *mutex;
    void *vtable;
    // Acquire one mutex level and retain the borrowed mutex for scope exit.
    CAutoLockMutex(void *m) {
        vtable = (void *)s_fileIoAutoLockMutexVTable;
        mutex = m;
        OSLockMutex((OSMutex *)m);
    }
    // Acquire one mutex level with the original volatile guard stores.
    CAutoLockMutex(void *m, int) {
        *(void *volatile *)&vtable = (void *)s_fileIoAutoLockMutexVTable;
        *(void *volatile *)&mutex = m;
        OSLockMutex((OSMutex *)m);
    }
    // Release the one mutex acquisition owned by this scope guard.
    ~CAutoLockMutex() {
        vtable = (void *)s_fileIoAutoLockMutexVTable;
        OSUnlockMutex((OSMutex *)mutex);
    }
};

// Allocate caller-owned bytes using the installed file-I/O allocator.
void *FileIO_Alloc(u32 size) {
    return g_fileIoAllocate(size);
}

// Return a file-I/O allocation to its paired free callback.
void FileIO_Free(void *p) {
    g_fileIoFreeCallback(p);
}

// Forward the pointer and size to callback 2; its further semantics are unresolved.
void *FileIO_InvokeAllocatorCallback2(void *p, u32 size) {
    return g_fileIoAllocatorCallback2(p, size);
}

// Forward the opaque argument word to callback 3.
void *FileIO_InvokeAllocatorCallback3(u32 callbackArgumentWord) {
    return g_fileIoAllocatorCallback3(callbackArgumentWord);
}

void gtl_CFileIoDeviceDVD_Ctor(void *p);
void gtl_CFileIoDeviceHIO_Ctor(void *p);
void gtl_CFileIoDeviceFMEM_Ctor(void *p);
void FileRegistry_Clear(void *p);

int FileIo_InitializeDevices(const void *callbacks) {
    void *allocate = (void*)((const FileIoAllocatorCallbacks*)callbacks)->allocate;
    void *freeCallback = (void*)((const FileIoAllocatorCallbacks*)callbacks)->free;
    char *globalStorage = s_fileIoMutex;
    void *allocatorCallback2 = (void*)((const FileIoAllocatorCallbacks*)callbacks)->allocatorCallback2;
    void *allocatorCallback3 = (void*)((const FileIoAllocatorCallbacks*)callbacks)->allocatorCallback3;
    g_fileIoAllocate = (void *(*)(u32))allocate;
    g_fileIoFreeCallback = (void (*)(void *))freeCallback;
    g_fileIoAllocatorCallback2 = (void *(*)(void *, u32))allocatorCallback2;
    g_fileIoAllocatorCallback3 = (void *(*)(u32))allocatorCallback3;
    FileRegistry_Clear(((char*)(globalStorage + kDeviceRegistryOffset)));

    char dvdName[12];
    FileIOString_CtorFromCString(dvdName, fileIoPathStrings);
    void *device = gtl_CFileIoDeviceDVD_AllocateStorage(0x10);
    if (device) gtl_CFileIoDeviceDVD_Ctor(device);
    void *node = FileRegistry_GetOrInsertValue(((char*)(globalStorage + kDeviceRegistryOffset)), dvdName);
    *(void **)node = device;
    FileIOString_dtor(dvdName, -1);

    char hostName[12];
    FileIOString_CtorFromCString(hostName, fileIoPathStrings + kHioPrefix);
    device = gtl_CFileIoDeviceHIO_AllocateStorage(0x10);
    if (device) gtl_CFileIoDeviceHIO_Ctor(device);
    node = FileRegistry_GetOrInsertValue(((char*)(globalStorage + kDeviceRegistryOffset)), hostName);
    *(void **)node = device;
    FileIOString_dtor(hostName, -1);

    char fileMemoryName[12];
    FileIOString_CtorFromCString(fileMemoryName, fileIoPathStrings + kFmemPrefix);
    device = gtl_CFileIoDeviceFMEM_AllocateStorage(0x1c);
    if (device) gtl_CFileIoDeviceFMEM_Ctor(device);
    node = FileRegistry_GetOrInsertValue(((char*)(globalStorage + kDeviceRegistryOffset)), fileMemoryName);
    *(void **)node = device;
    FileIOString_dtor(fileMemoryName, -1);

    char physicalMemory0Name[12];
    FileIOString_CtorFromCString(physicalMemory0Name, fileIoPathStrings + kPadMemory0Prefix);
    device = gtl_CFileIoDevicePADMEM_AllocateStorage(0x14);
    if (device) gtl_CFileIoDevicePADMEM_Ctor(device, 0);
    node = FileRegistry_GetOrInsertValue(((char*)(globalStorage + kDeviceRegistryOffset)), physicalMemory0Name);
    *(void **)node = device;
    FileIOString_dtor(physicalMemory0Name, -1);

    char physicalMemory1Name[12];
    FileIOString_CtorFromCString(physicalMemory1Name, fileIoPathStrings + kPadMemory1Prefix);
    device = gtl_CFileIoDevicePADMEM_AllocateStorage(0x14);
    if (device) gtl_CFileIoDevicePADMEM_Ctor(device, 1);
    node = FileRegistry_GetOrInsertValue(((char*)(globalStorage + kDeviceRegistryOffset)), physicalMemory1Name);
    *(void **)node = device;
    FileIOString_dtor(physicalMemory1Name, -1);

    char physicalMemory2Name[12];
    FileIOString_CtorFromCString(physicalMemory2Name, fileIoPathStrings + kPadMemory2Prefix);
    device = gtl_CFileIoDevicePADMEM_AllocateStorage(0x14);
    if (device) gtl_CFileIoDevicePADMEM_Ctor(device, 2);
    node = FileRegistry_GetOrInsertValue(((char*)(globalStorage + kDeviceRegistryOffset)), physicalMemory2Name);
    *(void **)node = device;
    FileIOString_dtor(physicalMemory2Name, -1);

    char physicalMemory3Name[12];
    FileIOString_CtorFromCString(physicalMemory3Name, fileIoPathStrings + kPadMemory3Prefix);
    device = gtl_CFileIoDevicePADMEM_AllocateStorage(0x14);
    if (device) gtl_CFileIoDevicePADMEM_Ctor(device, 3);
    node = FileRegistry_GetOrInsertValue(((char*)(globalStorage + kDeviceRegistryOffset)), physicalMemory3Name);
    *(void **)node = device;
    FileIOString_dtor(physicalMemory3Name, -1);

    strcpy(((char*)(globalStorage + kDefaultDevicePathOffset)), fileIoPathStrings);
    OSInitMutex((OSMutex *)((char *)globalStorage + 0));
    return 1;
}
// Allocate raw DVD device storage through the installed callback; construction is separate.
void *gtl_CFileIoDeviceDVD_AllocateStorage(u32 size) {
    return g_fileIoAllocate(size);
}

// Release device storage through the installed file-I/O free callback.
void dtor_804146C4(void *p) {
    g_fileIoFreeCallback(p);
}

// Allocate raw HIO device storage through the installed callback; construction is separate.
void *gtl_CFileIoDeviceHIO_AllocateStorage(u32 size) {
    return g_fileIoAllocate(size);
}

// Release device storage through the installed file-I/O free callback.
void dtor_804146DC(void *p) {
    g_fileIoFreeCallback(p);
}

// Allocate raw FMEM device storage through the installed callback; construction is separate.
void *gtl_CFileIoDeviceFMEM_AllocateStorage(u32 size) {
    return g_fileIoAllocate(size);
}

// Release device storage through the installed file-I/O free callback.
void dtor_804146F4(void *p) {
    g_fileIoFreeCallback(p);
}

// Allocate raw PADMEM device storage through the installed callback; construction is separate.
void *gtl_CFileIoDevicePADMEM_AllocateStorage(u32 size) {
    return g_fileIoAllocate(size);
}

// Release device storage through the installed file-I/O free callback.
void dtor_8041470C(void *p) {
    g_fileIoFreeCallback(p);
}

// Release a temporary key's heap string and optionally free its containing storage.
void *FileRegistryKey_dtor(void *self, int flags) {
    if (self) {
        if (self) {
            if (((u32)*(u32 *)self) >> 31) {
                g_fileIoFreeCallback(AsFileRegistryTree(self)->first);
            }
        }
        if (flags > 0) __dl__FPv(self);
    }
    return self;
}

#pragma opt_propagation off
#pragma opt_common_subs off
// Initialize a physical-memory device with a separator path and its channel index.
void *gtl_CFileIoDevicePADMEM_Ctor(void *self, int channel) {
    void *object;
    u32 inlineLength;
    int savedChannel;
    u32 zero;
    unsigned char inlineFlags;
    size_t separatorLength;
    struct { volatile char sp8[4]; volatile char spC; } temp;
    object = self;
    savedChannel = channel;
    zero = 0;
    ((volatile PadMemoryDeviceStorage*)object)->path.flags = zero;
    ((volatile PadMemoryDeviceStorage*)object)->path.length = zero;
    (*(volatile u32*)&((PadMemoryDeviceStorage*)object)->path.data) = zero;
    inlineFlags = ((volatile unsigned char *)object)[4];
    *(void **)object = (void *)gtlFileIoDeviceVtable;
    inlineLength = inlineFlags & 0x7F;
    temp.spC = temp.sp8[0];
    separatorLength = strlen((const char *)&pathSeparatorString);
    FileIOString_ReplaceRangeWithAllocator(&((PadMemoryDeviceStorage*)object)->path, 0, inlineLength, (const char *)&pathSeparatorString,
                  (const char *)((char *)&pathSeparatorString + separatorLength), (void *)&temp.spC);
    ((PadMemoryDeviceStorage*)object)->channel = (u32)savedChannel;
    *(void **)object = (void *)__vt__Q23gtl19CFileIoDevicePADMEM;
    return object;
}
#pragma opt_propagation reset
#pragma opt_common_subs reset

#pragma use_lmw_stmw on
// Destroy all device registry nodes and owned key strings, then restore the empty sentinel.
void FileRegistry_Clear(void *registry) {
    void *tree = registry;
    void *root = (*(void**)&AsFileRegistryTree(tree)->root);
    // In plain C this is: free every node of the registry tree (postorder; for each node free its
    // heap key if flags bit 31 is set, then the node). The long nested block is the compiler-unrolled
    // top levels of that recursion; the identical inner blocks (subtree, subtree, key, node) differ only
    // in which node they handle, and the doubled `if ((char*)&node->stringFlags)` tests are always true.
    // (An inline-helper rewrite was tried and changed the generated code, so the unrolled form stays.)
    // Postorder, expanded through three levels: left descendants, right descendants,
    // heap key, then node. Deeper branches delegate to FileRegistry_DestroySubtree.
    // Repeated member-address guards mirror nested string destructors; they are not
    // separate ownership tests. No device destructor is called in this routine.
    // Only a nonempty tree needs destruction.
    if (root != 0) {
        {
            void *branch = *(void **)root;
            if (branch != 0) {
                void *child = *(void **)branch;
                if (child != 0) {
                    void *leaf = *(void **)child;
                    if (leaf != 0) {
                        void *subtree = *(void **)leaf;
                        if (subtree != 0) FileRegistry_DestroySubtree(tree, subtree);
                        subtree = AsFileRegistryNode(leaf)->right;
                        if (subtree != 0) FileRegistry_DestroySubtree(tree, subtree);
                        if ((char*)&AsFileRegistryNode(leaf)->stringFlags) {
                            if ((char*)&AsFileRegistryNode(leaf)->stringFlags) {
                                if (AsFileRegistryNode(leaf)->stringFlags >> 31) g_fileIoFreeCallback((*(void **)&AsFileRegistryNode(leaf)->stringData));
                            }
                        }
                        g_fileIoFreeCallback(leaf);
                    }
                    leaf = AsFileRegistryNode(child)->right;
                    if (leaf != 0) {
                        void *subtree = *(void **)leaf;
                        if (subtree != 0) FileRegistry_DestroySubtree(tree, subtree);
                        subtree = AsFileRegistryNode(leaf)->right;
                        if (subtree != 0) FileRegistry_DestroySubtree(tree, subtree);
                        if ((char*)&AsFileRegistryNode(leaf)->stringFlags) {
                            if ((char*)&AsFileRegistryNode(leaf)->stringFlags) {
                                if (AsFileRegistryNode(leaf)->stringFlags >> 31) g_fileIoFreeCallback((*(void **)&AsFileRegistryNode(leaf)->stringData));
                            }
                        }
                        g_fileIoFreeCallback(leaf);
                    }
                    if ((char*)&AsFileRegistryNode(child)->stringFlags) {
                        if ((char*)&AsFileRegistryNode(child)->stringFlags) {
                            if (AsFileRegistryNode(child)->stringFlags >> 31) g_fileIoFreeCallback((*(void **)&AsFileRegistryNode(child)->stringData));
                        }
                    }
                    g_fileIoFreeCallback(child);
                }
                void *leaf = AsFileRegistryNode(branch)->right;
                if (leaf != 0) {
                    void *child = *(void **)leaf;
                    if (child != 0) {
                        void *subtree = *(void **)child;
                        if (subtree != 0) FileRegistry_DestroySubtree(tree, subtree);
                        subtree = AsFileRegistryNode(child)->right;
                        if (subtree != 0) FileRegistry_DestroySubtree(tree, subtree);
                        if ((char*)&AsFileRegistryNode(child)->stringFlags) {
                            if ((char*)&AsFileRegistryNode(child)->stringFlags) {
                                if (AsFileRegistryNode(child)->stringFlags >> 31) g_fileIoFreeCallback((*(void **)&AsFileRegistryNode(child)->stringData));
                            }
                        }
                        g_fileIoFreeCallback(child);
                    }
                    child = AsFileRegistryNode(leaf)->right;
                    if (child != 0) {
                        void *subtree = *(void **)child;
                        if (subtree != 0) FileRegistry_DestroySubtree(tree, subtree);
                        subtree = AsFileRegistryNode(child)->right;
                        if (subtree != 0) FileRegistry_DestroySubtree(tree, subtree);
                        if ((char*)&AsFileRegistryNode(child)->stringFlags) {
                            if ((char*)&AsFileRegistryNode(child)->stringFlags) {
                                if (AsFileRegistryNode(child)->stringFlags >> 31) g_fileIoFreeCallback((*(void **)&AsFileRegistryNode(child)->stringData));
                            }
                        }
                        g_fileIoFreeCallback(child);
                    }
                    if ((char*)&AsFileRegistryNode(leaf)->stringFlags) {
                        if ((char*)&AsFileRegistryNode(leaf)->stringFlags) {
                            if (AsFileRegistryNode(leaf)->stringFlags >> 31) g_fileIoFreeCallback((*(void **)&AsFileRegistryNode(leaf)->stringData));
                        }
                    }
                    g_fileIoFreeCallback(leaf);
                }
                if ((char*)&AsFileRegistryNode(branch)->stringFlags) {
                    if ((char*)&AsFileRegistryNode(branch)->stringFlags) {
                        if (AsFileRegistryNode(branch)->stringFlags >> 31) g_fileIoFreeCallback((*(void **)&AsFileRegistryNode(branch)->stringData));
                    }
                }
                g_fileIoFreeCallback(branch);
            }
        }
        {
            void *branch = AsFileRegistryNode(root)->right;
            if (branch != 0) {
                void *leaf = *(void **)branch;
                if (leaf != 0) {
                    void *child = *(void **)leaf;
                    if (child != 0) {
                        void *subtree = *(void **)child;
                        if (subtree != 0) FileRegistry_DestroySubtree(tree, subtree);
                        subtree = AsFileRegistryNode(child)->right;
                        if (subtree != 0) FileRegistry_DestroySubtree(tree, subtree);
                        if ((char*)&AsFileRegistryNode(child)->stringFlags) {
                            if ((char*)&AsFileRegistryNode(child)->stringFlags) {
                                if (AsFileRegistryNode(child)->stringFlags >> 31) g_fileIoFreeCallback((*(void **)&AsFileRegistryNode(child)->stringData));
                            }
                        }
                        g_fileIoFreeCallback(child);
                    }
                    child = AsFileRegistryNode(leaf)->right;
                    if (child != 0) {
                        void *subtree = *(void **)child;
                        if (subtree != 0) FileRegistry_DestroySubtree(tree, subtree);
                        subtree = AsFileRegistryNode(child)->right;
                        if (subtree != 0) FileRegistry_DestroySubtree(tree, subtree);
                        if ((char*)&AsFileRegistryNode(child)->stringFlags) {
                            if ((char*)&AsFileRegistryNode(child)->stringFlags) {
                                if (AsFileRegistryNode(child)->stringFlags >> 31) g_fileIoFreeCallback((*(void **)&AsFileRegistryNode(child)->stringData));
                            }
                        }
                        g_fileIoFreeCallback(child);
                    }
                    if ((char*)&AsFileRegistryNode(leaf)->stringFlags) {
                        if ((char*)&AsFileRegistryNode(leaf)->stringFlags) {
                            if (AsFileRegistryNode(leaf)->stringFlags >> 31) g_fileIoFreeCallback((*(void **)&AsFileRegistryNode(leaf)->stringData));
                        }
                    }
                    g_fileIoFreeCallback(leaf);
                }
                leaf = AsFileRegistryNode(branch)->right;
                if (leaf != 0) {
                    void *child = *(void **)leaf;
                    if (child != 0) {
                        void *subtree = *(void **)child;
                        if (subtree != 0) FileRegistry_DestroySubtree(tree, subtree);
                        subtree = AsFileRegistryNode(child)->right;
                        if (subtree != 0) FileRegistry_DestroySubtree(tree, subtree);
                        if ((char*)&AsFileRegistryNode(child)->stringFlags) {
                            if ((char*)&AsFileRegistryNode(child)->stringFlags) {
                                if (AsFileRegistryNode(child)->stringFlags >> 31) g_fileIoFreeCallback((*(void **)&AsFileRegistryNode(child)->stringData));
                            }
                        }
                        g_fileIoFreeCallback(child);
                    }
                    child = AsFileRegistryNode(leaf)->right;
                    if (child != 0) {
                        void *subtree = *(void **)child;
                        if (subtree != 0) FileRegistry_DestroySubtree(tree, subtree);
                        subtree = AsFileRegistryNode(child)->right;
                        if (subtree != 0) FileRegistry_DestroySubtree(tree, subtree);
                        if ((char*)&AsFileRegistryNode(child)->stringFlags) {
                            if ((char*)&AsFileRegistryNode(child)->stringFlags) {
                                if (AsFileRegistryNode(child)->stringFlags >> 31) g_fileIoFreeCallback((*(void **)&AsFileRegistryNode(child)->stringData));
                            }
                        }
                        g_fileIoFreeCallback(child);
                    }
                    if ((char*)&AsFileRegistryNode(leaf)->stringFlags) {
                        if ((char*)&AsFileRegistryNode(leaf)->stringFlags) {
                            if (AsFileRegistryNode(leaf)->stringFlags >> 31) g_fileIoFreeCallback((*(void **)&AsFileRegistryNode(leaf)->stringData));
                        }
                    }
                    g_fileIoFreeCallback(leaf);
                }
                if ((char*)&AsFileRegistryNode(branch)->stringFlags) {
                    if ((char*)&AsFileRegistryNode(branch)->stringFlags) {
                        if (AsFileRegistryNode(branch)->stringFlags >> 31) g_fileIoFreeCallback((*(void **)&AsFileRegistryNode(branch)->stringData));
                    }
                }
                g_fileIoFreeCallback(branch);
            }
        }
        if ((char*)&AsFileRegistryNode(root)->stringFlags) {
            if ((char*)&AsFileRegistryNode(root)->stringFlags) {
                if (AsFileRegistryNode(root)->stringFlags >> 31) g_fileIoFreeCallback((*(void **)&AsFileRegistryNode(root)->stringData));
            }
        }
        g_fileIoFreeCallback(root);
        // All nodes are gone; restore count/root/first to the empty header invariant.
        {
            void *empty = 0;
            void *sentinel = (char*)&AsFileRegistryTree(tree)->root;
            *(void **)tree = empty;
            (*(void**)&AsFileRegistryTree(tree)->root) = empty;
            AsFileRegistryTree(tree)->first = sentinel;
        }
    }
}
#pragma use_lmw_stmw reset

#pragma use_lmw_stmw on
#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_strength_reduction off
// Return the shorter of two string lengths, retaining reference identity.
inline const u32 &smallerStringLength(const u32 &a, const u32 &b) { return a < b ? a : b; }
// Decode byte length from the inline tag or the heap-string length word.
inline u32 fileIoStringLength(const void *p) {
    return !(*(const u32 *)p >> 31) ? (*(const unsigned char *)p & 127) : *((const u32 *)p + 1);
}
// Borrow character storage from the inline bytes or heap pointer.
inline const char *fileIoStringData(const void *p) {
    return !(*(const u32 *)p >> 31) ? (const char *)p + 1 : *((const char *const *)p + 2);
}
struct FileIoStringView { u32 word; u32 length; const char *data; };
// Compare inline or heap-backed file-I/O strings lexically, then by length.
inline int compareFileIoStrings(const FileIoStringView *a, const FileIoStringView *b) {
    const char *bd;
    u32 bl;
    u32 count;
    u32 al;
    u32 bsize;
    u32 asize;
    const char *ad;
    u32 aval;
    if (!(b->word >> 31)) {
        bd = (const char *)b + 1;
        bl = *(const unsigned char *)b & 127;
    } else {
        bd = b->data;
        bl = b->length;
    }
    bsize = bl;
    asize = fileIoStringLength(a);
    if (!(a->word >> 31)) {
        ad = (const char *)a + 1;
        aval = *(const unsigned char *)a & 127;
    } else {
        ad = a->data;
        aval = a->length;
    }
    al = aval;
    count = smallerStringLength(asize, al);
    int cmp = memcmp(ad, bd, smallerStringLength(bsize, count));
    if (cmp == 0) {
        if (count < bl) cmp = -1;
        else if (count == bl) cmp = 0;
        else if (count != bl) cmp = 1;
    }
    return cmp;
}
// Find a key's lower bound and return the parent and branch flags needed for insertion.
void *FileRegistry_FindInsertionPosition(void *self, const void *key, void **outNode, char *flag1, char *flag2) {
    const char *keyStorage = (const char *)key;
    void **lowerBoundOut = outNode;
    char *insertLeft = flag1;
    char *noLowerBound = flag2;
    void *parent = (char*)&AsFileRegistryTree(self)->root;
    int unusedZero = 0;
    int unusedOne = 1;
    u32 unusedWord;
    *lowerBoundOut = 0;
    void *node = (*(void**)&AsFileRegistryTree(self)->root);
    *insertLeft = 1;
    *noLowerBound = 1;
    while (node != 0) {
        parent = node;
        int cmp = compareFileIoStrings((const FileIoStringView *)keyStorage, (const FileIoStringView *)((char*)&AsFileRegistryNode(node)->stringFlags));
        if (cmp < 0) {
            node = *(void **)node;
            *insertLeft = 1;
        } else {
            *lowerBoundOut = node;
            node = AsFileRegistryNode(node)->right;
            *insertLeft = 0;
            *noLowerBound = 0;
        }
    }
    return parent;
}
#pragma use_lmw_stmw reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset

#pragma use_lmw_stmw on
#pragma opt_common_subs off
#pragma opt_propagation off
#pragma opt_lifetimes off
// Return the existing key's value slot or insert a copied key with the default value.
void *FileRegistry_GetOrInsertValue(void *self, const void *key) {
    volatile u32 allocatorByte;
    u32 allocatorCopy;
    char noLowerBound;
    char insertLeft;
    void *lowerBound;
    void *registry = self;
    const char *keyStorage = (const char *)key;
    void *parent = FileRegistry_FindInsertionPosition(self, key, &lowerBound, &insertLeft, &noLowerBound);
    void *candidate = lowerBound;
    char *nodeString;
    // Original 0x80414D34: use the lower bound or construct a new registry entry.
    do {
    if (candidate != 0) {
            nodeString = (char*)&AsFileRegistryNode(candidate)->stringFlags;
            if (compareFileIoStrings((const FileIoStringView *)nodeString, (const FileIoStringView *)keyStorage) >= 0) break;
        }
        {
            struct Temp { u32 words[3]; void *value; } temp;
            char *temporaryKey = (char *)&temp;
            u32 keyWord0 = *(u32 *)keyStorage;
            if (!(keyWord0 >> 31)) {
                struct Words { u32 a, b, c; };
                *(Words *)temporaryKey = *(const Words *)keyStorage;
            } else {
                temp.words[0] = 0;
                temp.words[1] = 0;
                temp.words[2] = 0;
                FileIOString_FitCapacityForLength(temporaryKey, *(u32 *)(keyStorage + 4));
                *(char *)&allocatorCopy = *(volatile char *)&allocatorByte;
                const char *data = *(const char **)(keyStorage + 8);
                FileIOString_ReplaceRangeWithAllocator(temporaryKey, 0, 0, data, data + *(u32 *)(keyStorage + 4), &allocatorCopy);
            }
            temp.value = s_fileRegistryDefaultDevice;
            EH5Cleanup eh5Cleanup33(&temp, (void (*)(void*, int))FileRegistryKey_dtor);
            void *ret = FileRegistry_InsertNode(self, parent, insertLeft, noLowerBound, temporaryKey);
            nodeString = (char*)&AsFileRegistryNode(ret)->stringFlags;
            u32 w = *(u32 *)temporaryKey;
            if (w >> 31) {
                { eh5Cleanup33.release(); g_fileIoFreeCallback(*(void **)(temporaryKey + 8)); }
            }
        }
    } while (0);
    return (char*)&AsFileRegistryNode(nodeString)->stringFlags;
}
#pragma use_lmw_stmw reset
#pragma opt_common_subs reset
#pragma opt_propagation reset
#pragma opt_lifetimes reset

class Device80415730 {
public:
    virtual void slot8();
    virtual void flush();
    virtual int open(void *, u32, u32, u32, u32);
    virtual void m3(); virtual void m4(); virtual void m5(); virtual void m6();
    virtual void m7(); virtual void m8(); virtual void m9(); virtual void m10();
    virtual void m11(); virtual void m12(); virtual void m13(); virtual void m14();
    virtual void m15(); virtual void m16(); virtual void m17(); virtual void m18();
    virtual void m19(); virtual void m20(); virtual void m21(); virtual void m22();
    virtual void m23(); virtual void m24();
    virtual int openEx(void *, u32, u32, u32, u32, u32, u32, u32);
};

#pragma opt_common_subs off
// Lock the device registry and invoke each registered device's flush method in key order.
int FileIO_FlushAllDevices() {
    CAutoLockMutex guard((void *)s_fileIoMutex);
    void *mutex = (void *)s_fileIoMutex;
    void *base = (void *)s_fileRegistryTree;
    void *node = AsFileRegistryTree(base)->first;
    void *sentinel = (char*)&AsFileRegistryTree(base)->root;
    while (node != sentinel) {
        ((Device80415730*)AsFileRegistryNode(node)->file)->flush();
        void *rightChild = AsFileRegistryNode(node)->right;
        if (rightChild != 0) {
            void *leftChild;
            while ((leftChild = *(void **)rightChild) != 0) rightChild = leftChild;
            node = rightChild;
        } else {
            void *parent;
            while ((parent = (void *)(AsFileRegistryNode(node)->parentAndColor & ~1u)), node != *(void **)parent) {
                node = parent;
            }
            node = parent;
        }
    }
    (void)mutex;
    return 1;
}
#pragma opt_common_subs reset

#pragma use_lmw_stmw on
#pragma opt_common_subs off
#pragma opt_lifetimes off
// Resolve a path and open through the first registry prefix match (key order, not longest prefix).
// Return an owned wrapper address as int, or -1 for no device/backend-open failure.
// Device lifetime must outlast wrappers. extra0..2 apply only to extended open.
int FileIO_OpenFile(void *pathNodeOrDevice, u32 openMode, u32 deviceArgument, u32 a4_unused, u32 accessMode, u32 openFlags, u32 a7_unused, u32 extra0, u32 extra1, u32 extra2) {
    void *mutex;
    void *handleOrEnd;
    mutex = (void *)s_fileIoMutex;

    CAutoLockMutex guard(mutex, 0);
    char buf[kFileIoPathCapacity];

    FileIO_ResolveFullPath(pathNodeOrDevice, kFileIoPathCapacity, buf, 0);
    OSLockMutex((OSMutex *)mutex);

    char *base = s_fileRegistryTree;
    pathNodeOrDevice = AsFileRegistryTree(base)->first;
    handleOrEnd = (void *)((char*)&AsFileRegistryTree(base)->root);
    // Pointer lifetime map (storage reuse preserves the compiled register lifetimes):
    // pathNodeOrDevice: input path -> registry iterator -> borrowed device (or null).
    // handleOrEnd: &tree.root end sentinel -> newly owned wrapper allocation.
    // Guard construction and the explicit lock above acquire the recursive mutex twice.
    // SEARCH-EXHAUSTION SIDE EFFECT: the false arm of this predicate unlocks once
    // and nulls the iterator. A prefix match unlocks once in the body instead.
    // The guard releases the outer acquisition on every return/exception.
    while (pathNodeOrDevice != handleOrEnd || (OSUnlockMutex((OSMutex *)mutex), pathNodeOrDevice = 0, false)) {
        u32 word0 = AsFileRegistryNode(pathNodeOrDevice)->stringFlags;
        u32 len;
        void *namePtr;
        len = !(word0 >> 31) ? (*(unsigned char *)((char*)&AsFileRegistryNode(pathNodeOrDevice)->stringFlags) & 0x7F) : AsFileRegistryNode(pathNodeOrDevice)->stringLength;
        namePtr = !(AsFileRegistryNode(pathNodeOrDevice)->stringFlags >> 31) ? (char*)&AsFileRegistryNode(pathNodeOrDevice)->stringFlags + 1 : (*(void **)&AsFileRegistryNode(pathNodeOrDevice)->stringData);
        int cmp = strncmp(buf, (const char *)namePtr, len);
        if (cmp == 0) {
            pathNodeOrDevice = AsFileRegistryNode(pathNodeOrDevice)->file;
            OSUnlockMutex((OSMutex *)mutex);
            break;
        }
        void *child = AsFileRegistryNode(pathNodeOrDevice)->right;
        if (child != 0) {
            void *r0;
            while ((r0 = *(void **)child) != 0) child = r0;
            pathNodeOrDevice = child;
        } else {
            void *r3;
            while ((r3 = (void *)(AsFileRegistryNode(pathNodeOrDevice)->parentAndColor & ~1u)), pathNodeOrDevice != *(void **)r3) {
                pathNodeOrDevice = r3;
            }
            pathNodeOrDevice = r3;
        }
    }
    if (pathNodeOrDevice == 0) {
        return -1;
    }

    // Allocate wrapper storage. The original assumes allocation succeeds: even though
    // initialization is guarded, backend dispatch below dereferences it unconditionally.
    // FileIoOpenOptions covers +8..+16; FileHandle covers the rest of the SAME 40 bytes.
    // The backend result is stored at +0 and is distinct from this wrapper address.
    handleOrEnd = g_fileIoAllocate(0x28);
    if (handleOrEnd) {
        ((FileIoOpenOptions*)handleOrEnd)->openMode = openMode;
        ((FileIoOpenOptions*)handleOrEnd)->accessMode = accessMode;
        ((FileHandle*)handleOrEnd)->flags = openFlags;
        (*(Device80415730**)&((FileHandle*)handleOrEnd)->device) = (Device80415730*)pathNodeOrDevice;
        (*(u32*)&((FileHandle*)handleOrEnd)->handle) = (u32)-1;
        ((FileIoOpenOptions*)handleOrEnd)->backendState = 0;
        (*(u32*)&((FileHandle*)handleOrEnd)->transferBuffer) = 0;
        ((FileHandle*)handleOrEnd)->allocatedBytes = 0;
        (*(u32*)&((FileHandle*)handleOrEnd)->readCopyDestination) = 0;
        ((FileHandle*)handleOrEnd)->requestedBytes = 0;
    }

    // Backend borrows buf for this call; failure (-1) releases just the wrapper.
    // Success returns the wrapper as an integer; later operations dispatch via its device.
    u32 result;
    if (openFlags & kFileIoExtendedOpen) {
        result = (*(Device80415730**)&((FileHandle*)handleOrEnd)->device)->openEx(buf, openMode, deviceArgument, accessMode, openFlags, extra0, extra1, extra2);
        (*(u32*)&((FileHandle*)handleOrEnd)->handle) = result;
    } else {
        result = (*(Device80415730**)&((FileHandle*)handleOrEnd)->device)->open(buf, openMode, deviceArgument, accessMode, openFlags);
        (*(u32*)&((FileHandle*)handleOrEnd)->handle) = result;
    }

    if ((*(u32*)&((FileHandle*)handleOrEnd)->handle) == (u32)-1) {
        if (handleOrEnd) g_fileIoFreeCallback(handleOrEnd);
        return -1;
    }
    return (int)handleOrEnd;
}
#pragma opt_common_subs reset
#pragma use_lmw_stmw reset
#pragma opt_propagation reset
#pragma opt_lifetimes reset

class Cleanup804150CC {
public:
    virtual ~Cleanup804150CC();
};

#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
// Destroy registered device objects, then free their registry nodes and key strings.
void FileRegistry_ClearOwnedDevices(void) {
    void *registry = (void *)s_fileRegistryTree;
    void *rootOrZero = 0;
    void *node;
    void *child;
    void *endOrLeaf;
    // Phase 1: in-order traversal deletes devices while nodes and keys remain valid.
    endOrLeaf = (char*)&AsFileRegistryTree(registry)->root;
    node = AsFileRegistryTree(registry)->first;
    while (node != endOrLeaf) {
        void *obj = AsFileRegistryNode(node)->file;
        if (obj != 0) {
            delete (Cleanup804150CC *)obj;
            AsFileRegistryNode(node)->file = rootOrZero;
        }
        void *rightChild = AsFileRegistryNode(node)->right;
        if (rightChild != 0) {
            void *leftChild;
            while ((leftChild = *(void **)rightChild) != 0) rightChild = leftChild;
            node = rightChild;
        } else {
            void *parent;
            while ((parent = (void *)(AsFileRegistryNode(node)->parentAndColor & ~1u)), node != *(void **)parent) {
                node = parent;
            }
            node = parent;
        }
    }
    // Phase 2: postorder destruction of keys and nodes, then empty-header reset.
    // Same unrolled tree free as in FileRegistry_Clear: in plain C, free every node's heap key (flags bit 31)
    // and the node itself, children first; the repeated blocks and doubled address tests are compiler artifacts.
    // Devices were cleared above; this phase has the same ownership rule as Clear.
    rootOrZero = (*(void**)&AsFileRegistryTree(s_fileRegistryTree)->root);
    if (rootOrZero != 0) {
        node = AsFileRegistryNode(rootOrZero)->left;
        if (node != 0) {
            child = AsFileRegistryNode(node)->left;
            if (child != 0) {
                endOrLeaf = AsFileRegistryNode(child)->left;
                if (endOrLeaf != 0) {
                    void *subtree = *(void **)endOrLeaf;
                    if (subtree != 0) FileRegistry_DestroySubtree(s_fileRegistryTree, subtree);
                    subtree = AsFileRegistryNode(endOrLeaf)->right;
                    if (subtree != 0) FileRegistry_DestroySubtree(s_fileRegistryTree, subtree);
                    if ((char*)&AsFileRegistryNode(endOrLeaf)->stringFlags) {
                        if ((char*)&AsFileRegistryNode(endOrLeaf)->stringFlags) {
                            if (AsFileRegistryNode(endOrLeaf)->stringFlags >> 31) g_fileIoFreeCallback((*(void **)&AsFileRegistryNode(endOrLeaf)->stringData));
                        }
                    }
                    g_fileIoFreeCallback(endOrLeaf);
                }
                endOrLeaf = AsFileRegistryNode(child)->right;
                if (endOrLeaf != 0) {
                    void *subtree = *(void **)endOrLeaf;
                    if (subtree != 0) FileRegistry_DestroySubtree(s_fileRegistryTree, subtree);
                    subtree = AsFileRegistryNode(endOrLeaf)->right;
                    if (subtree != 0) FileRegistry_DestroySubtree(s_fileRegistryTree, subtree);
                    if ((char*)&AsFileRegistryNode(endOrLeaf)->stringFlags) {
                        if ((char*)&AsFileRegistryNode(endOrLeaf)->stringFlags) {
                            if (AsFileRegistryNode(endOrLeaf)->stringFlags >> 31) g_fileIoFreeCallback((*(void **)&AsFileRegistryNode(endOrLeaf)->stringData));
                        }
                    }
                    g_fileIoFreeCallback(endOrLeaf);
                }
                if ((char*)&AsFileRegistryNode(child)->stringFlags) {
                    if ((char*)&AsFileRegistryNode(child)->stringFlags) {
                        if (AsFileRegistryNode(child)->stringFlags >> 31) g_fileIoFreeCallback((*(void **)&AsFileRegistryNode(child)->stringData));
                    }
                }
                g_fileIoFreeCallback(child);
            }
            endOrLeaf = AsFileRegistryNode(node)->right;
            if (endOrLeaf != 0) {
                child = AsFileRegistryNode(endOrLeaf)->left;
                if (child != 0) {
                    void *subtree = *(void **)child;
                    if (subtree != 0) FileRegistry_DestroySubtree(s_fileRegistryTree, subtree);
                    subtree = AsFileRegistryNode(child)->right;
                    if (subtree != 0) FileRegistry_DestroySubtree(s_fileRegistryTree, subtree);
                    if ((char*)&AsFileRegistryNode(child)->stringFlags) {
                        if ((char*)&AsFileRegistryNode(child)->stringFlags) {
                            if (AsFileRegistryNode(child)->stringFlags >> 31) g_fileIoFreeCallback((*(void **)&AsFileRegistryNode(child)->stringData));
                        }
                    }
                    g_fileIoFreeCallback(child);
                }
                child = AsFileRegistryNode(endOrLeaf)->right;
                if (child != 0) {
                    void *subtree = *(void **)child;
                    if (subtree != 0) FileRegistry_DestroySubtree(s_fileRegistryTree, subtree);
                    subtree = AsFileRegistryNode(child)->right;
                    if (subtree != 0) FileRegistry_DestroySubtree(s_fileRegistryTree, subtree);
                    if ((char*)&AsFileRegistryNode(child)->stringFlags) {
                        if ((char*)&AsFileRegistryNode(child)->stringFlags) {
                            if (AsFileRegistryNode(child)->stringFlags >> 31) g_fileIoFreeCallback((*(void **)&AsFileRegistryNode(child)->stringData));
                        }
                    }
                    g_fileIoFreeCallback(child);
                }
                if ((char*)&AsFileRegistryNode(endOrLeaf)->stringFlags) {
                    if ((char*)&AsFileRegistryNode(endOrLeaf)->stringFlags) {
                        if (AsFileRegistryNode(endOrLeaf)->stringFlags >> 31) g_fileIoFreeCallback((*(void **)&AsFileRegistryNode(endOrLeaf)->stringData));
                    }
                }
                g_fileIoFreeCallback(endOrLeaf);
            }
            if ((char*)&AsFileRegistryNode(node)->stringFlags) {
                if ((char*)&AsFileRegistryNode(node)->stringFlags) {
                    if (AsFileRegistryNode(node)->stringFlags >> 31) g_fileIoFreeCallback((*(void **)&AsFileRegistryNode(node)->stringData));
                }
            }
            g_fileIoFreeCallback(node);
        }
        node = AsFileRegistryNode(rootOrZero)->right;
        if (node != 0) {
            endOrLeaf = AsFileRegistryNode(node)->left;
            if (endOrLeaf != 0) {
                child = AsFileRegistryNode(endOrLeaf)->left;
                if (child != 0) {
                    void *subtree = *(void **)child;
                    if (subtree != 0) FileRegistry_DestroySubtree(s_fileRegistryTree, subtree);
                    subtree = AsFileRegistryNode(child)->right;
                    if (subtree != 0) FileRegistry_DestroySubtree(s_fileRegistryTree, subtree);
                    if ((char*)&AsFileRegistryNode(child)->stringFlags) {
                        if ((char*)&AsFileRegistryNode(child)->stringFlags) {
                            if (AsFileRegistryNode(child)->stringFlags >> 31) g_fileIoFreeCallback((*(void **)&AsFileRegistryNode(child)->stringData));
                        }
                    }
                    g_fileIoFreeCallback(child);
                }
                child = AsFileRegistryNode(endOrLeaf)->right;
                if (child != 0) {
                    void *subtree = *(void **)child;
                    if (subtree != 0) FileRegistry_DestroySubtree(s_fileRegistryTree, subtree);
                    subtree = AsFileRegistryNode(child)->right;
                    if (subtree != 0) FileRegistry_DestroySubtree(s_fileRegistryTree, subtree);
                    if ((char*)&AsFileRegistryNode(child)->stringFlags) {
                        if ((char*)&AsFileRegistryNode(child)->stringFlags) {
                            if (AsFileRegistryNode(child)->stringFlags >> 31) g_fileIoFreeCallback((*(void **)&AsFileRegistryNode(child)->stringData));
                        }
                    }
                    g_fileIoFreeCallback(child);
                }
                if ((char*)&AsFileRegistryNode(endOrLeaf)->stringFlags) {
                    if ((char*)&AsFileRegistryNode(endOrLeaf)->stringFlags) {
                        if (AsFileRegistryNode(endOrLeaf)->stringFlags >> 31) g_fileIoFreeCallback((*(void **)&AsFileRegistryNode(endOrLeaf)->stringData));
                    }
                }
                g_fileIoFreeCallback(endOrLeaf);
            }
            endOrLeaf = AsFileRegistryNode(node)->right;
            if (endOrLeaf != 0) {
                child = AsFileRegistryNode(endOrLeaf)->left;
                if (child != 0) {
                    void *subtree = *(void **)child;
                    if (subtree != 0) FileRegistry_DestroySubtree(s_fileRegistryTree, subtree);
                    subtree = AsFileRegistryNode(child)->right;
                    if (subtree != 0) FileRegistry_DestroySubtree(s_fileRegistryTree, subtree);
                    if ((char*)&AsFileRegistryNode(child)->stringFlags) {
                        if ((char*)&AsFileRegistryNode(child)->stringFlags) {
                            if (AsFileRegistryNode(child)->stringFlags >> 31) g_fileIoFreeCallback((*(void **)&AsFileRegistryNode(child)->stringData));
                        }
                    }
                    g_fileIoFreeCallback(child);
                }
                child = AsFileRegistryNode(endOrLeaf)->right;
                if (child != 0) {
                    void *subtree = *(void **)child;
                    if (subtree != 0) FileRegistry_DestroySubtree(s_fileRegistryTree, subtree);
                    subtree = AsFileRegistryNode(child)->right;
                    if (subtree != 0) FileRegistry_DestroySubtree(s_fileRegistryTree, subtree);
                    if ((char*)&AsFileRegistryNode(child)->stringFlags) {
                        if ((char*)&AsFileRegistryNode(child)->stringFlags) {
                            if (AsFileRegistryNode(child)->stringFlags >> 31) g_fileIoFreeCallback((*(void **)&AsFileRegistryNode(child)->stringData));
                        }
                    }
                    g_fileIoFreeCallback(child);
                }
                if ((char*)&AsFileRegistryNode(endOrLeaf)->stringFlags) {
                    if ((char*)&AsFileRegistryNode(endOrLeaf)->stringFlags) {
                        if (AsFileRegistryNode(endOrLeaf)->stringFlags >> 31) g_fileIoFreeCallback((*(void **)&AsFileRegistryNode(endOrLeaf)->stringData));
                    }
                }
                g_fileIoFreeCallback(endOrLeaf);
            }
            if ((char*)&AsFileRegistryNode(node)->stringFlags) {
                if ((char*)&AsFileRegistryNode(node)->stringFlags) {
                    if (AsFileRegistryNode(node)->stringFlags >> 31) g_fileIoFreeCallback((*(void **)&AsFileRegistryNode(node)->stringData));
                }
            }
            g_fileIoFreeCallback(node);
        }
        if ((char*)&AsFileRegistryNode(rootOrZero)->stringFlags) {
            if ((char*)&AsFileRegistryNode(rootOrZero)->stringFlags) {
                if (AsFileRegistryNode(rootOrZero)->stringFlags >> 31) g_fileIoFreeCallback((*(void **)&AsFileRegistryNode(rootOrZero)->stringData));
            }
        }
        g_fileIoFreeCallback(rootOrZero);
        *(u32 *)s_fileRegistryTree = 0;
        AsFileRegistryTree(s_fileRegistryTree)->root = 0;
        AsFileRegistryTree(s_fileRegistryTree)->first = (char*)&AsFileRegistryTree(s_fileRegistryTree)->root;
    }
}
#pragma opt_propagation reset

#pragma opt_common_subs reset
#pragma opt_lifetimes reset
} // extern "C"
