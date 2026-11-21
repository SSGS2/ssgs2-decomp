// Translation unit 0x80418364-0x80418C3C (5 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "types.h"
#include "sdk_protos.h"

#pragma use_lmw_stmw on

struct gtl_CFileIoDeviceHIO {
    void *vt;       /* 0x00 */
    u32 pathStringFlags;       /* 0x04: packed path-string flags/inline bytes, not device flags */
    u32 pathStringLength;       /* 0x08: heap path length when flag bit 31 is set */
    char *pathStringData;      /* 0x0C: owned heap path when flag bit 31 is set */
};

struct FileString {
    u32 flags;
    u32 size;
    char *data;
};

struct FileNode {
    FileNode *left;
    FileNode *right;
    u32 unknown; // Tree bookkeeping word; not part of the key string.
    FileString text;
};

struct FileTree {
    void *vt;
    FileNode *root;
};

void operator delete(void *) throw();
extern "C" void (*g_fileIoFreeCallback)(void *);
extern "C" void FileRegistry_DestroySubtree(FileTree *, FileNode *);

enum FileStringLimits { kInlineLengthMask = 0x7F, kPathScanBytes = 1024, kPathPartMax = 255 };

// Release heap-backed key bytes; inline strings do not own a separate allocation.
static inline void releaseString(FileString *text) {
    // The repeated guards preserve emitted template-destructor branches.
    if (text) {
        if (text) {
            if (text->flags >> 31) g_fileIoFreeCallback(text->data);
        }
    }
}

template <int Depth> struct ReleaseTree {
    // Release child subtrees before their owning node, unrolling Depth levels.
    static inline void run(FileTree *tree, FileNode *node) {
        if (node) {
            ReleaseTree<Depth - 1>::run(tree, node->left);
            ReleaseTree<Depth - 1>::run(tree, node->right);
            releaseString(&node->text);
            g_fileIoFreeCallback(node);
        }
    }
};

template <> struct ReleaseTree<0> {
    // Continue traversal through the original recursive helper at the unroll boundary.
    static inline void run(FileTree *tree, FileNode *node) {
        if (node) FileRegistry_DestroySubtree(tree, node);
    }
};

// Release the registry tree through its nested owner wrappers; guards are ABI-visible.
static inline void releaseRegistryTree(FileTree *self) {
    // Preserve five wrapper checks plus the destructor's outer check.
    if (self && self && self && self && self)
        ReleaseTree<4>::run(self, self->root);
}

extern "C" {

extern char __vt__Q23gtl16CFileIoDeviceHIO[];
extern char gtlFileIoDeviceVtable[];
extern char pathSeparatorString;

void FileIO_Free(void *p);
void FileIOString_ReplaceRangeWithAllocator(void *a, u32 b, u32 c, const char *d, const char *e, void *f);

extern const char kFilePathDriveScanFormat[]; // sscanf pattern for the drive prefix before ':'.

// Release the registry's owned tree nodes, then optionally delete the registry.
FileTree *FileRegistry_Dtor(FileTree *self, int flags) {
    if (self) {
        releaseRegistryTree(self);
        if (flags > 0) operator delete(self);
    }
    return self;
}

// Split optional drive, directory, filename and extension outputs; bound the last three to 255 bytes.
void FileIO_SplitPath(const char *path, char *drive, char *directory, char *name, char *extension) {
    const char *dot = 0;
    // Phase 1: consume the optional drive prefix, including its colon.
    char buffer[kPathScanBytes] = {0};
    const char *colon = strchr(path, ':');
    int scanned = sscanf(path, kFilePathDriveScanFormat, buffer);
    if (colon && scanned > 0) {
        size_t length = strlen(buffer);
        if (drive) {
            strncpy(drive, path, length + 1);
            drive[length + 1] = 0;
        }
        path = (const char *)((u32)length + (u32)path + 1);
    } else if (drive) {
        *drive = 0;
    }
    // Phase 2: remember the final separator and dot in the remaining path.
    const char *cursor = path;
    const char *slash = 0;
    while (*cursor) {
        if (*cursor == '/' || *cursor == '\\') {
            slash = cursor + 1;
        } else if (*cursor == '.') {
            dot = cursor;
        }
        ++cursor;
    }
    // Phase 3: copy optional outputs with room for a trailing null byte.
    if (slash) {
        if (directory) {
            u32 length = (u32)(slash - path);
            u32 count = length < kPathPartMax ? length : kPathPartMax;
            strncpy(directory, path, count);
            directory[count] = 0;
        }
        path = slash;
    } else if (directory) {
        *directory = 0;
    }
    if (dot && dot >= path) {
        if (name) {
            u32 length = (u32)(dot - path);
            u32 count = length < kPathPartMax ? length : kPathPartMax;
            strncpy(name, path, count);
            name[count] = 0;
        }
        if (extension) {
            u32 length = (u32)(cursor - dot);
            u32 count = length < kPathPartMax ? length : kPathPartMax;
            strncpy(extension, dot, count);
            extension[count] = 0;
        }
    } else {
        if (name) {
            u32 length = (u32)(cursor - path);
            u32 count = length < kPathPartMax ? length : kPathPartMax;
            strncpy(name, path, count);
            name[count] = 0;
        }
        if (extension) *extension = 0;
    }
}

#pragma opt_propagation off
// Initialize the HIO device's packed path string with the default separator.
gtl_CFileIoDeviceHIO *gtl_CFileIoDeviceHIO_Ctor(volatile gtl_CFileIoDeviceHIO *self) {
    volatile gtl_CFileIoDeviceHIO *that;
    int packedStringByte;
    u32 inlineLength;
    // Allocator tags carry no value here, but their volatile byte copy is ABI-visible.
    volatile char allocatorTags[8];
    that = self;
    packedStringByte = 0;
    that->pathStringFlags = packedStringByte;
    that->pathStringLength = packedStringByte;
    that->pathStringData = (char *)packedStringByte;
    packedStringByte = ((volatile unsigned char *)that)[4];
    that->vt = (void *)gtlFileIoDeviceVtable;
    inlineLength = packedStringByte & kInlineLengthMask;
    allocatorTags[4] = allocatorTags[0];
    FileIOString_ReplaceRangeWithAllocator((void *)&that->pathStringFlags, 0, inlineLength,
        (const char *)&pathSeparatorString,
        (const char *)((char *)&pathSeparatorString + strlen((const char *)&pathSeparatorString)),
        (void *)&allocatorTags[4]);
    that->vt = (void *)__vt__Q23gtl16CFileIoDeviceHIO;
    return (gtl_CFileIoDeviceHIO *)that;
}
#pragma opt_propagation reset

// Free an owned heap path, then optionally release the HIO device through its allocator.
gtl_CFileIoDeviceHIO *gtl_CFileIoDeviceHIO_dtor(gtl_CFileIoDeviceHIO *self, s32 flags) {
    if (self) {
        if (self) {
            if (&self->pathStringFlags) {
                if (self->pathStringFlags >> 31) {
                    FileIO_Free(self->pathStringData);
                }
            }
        }
        if (flags > 0) {
            FileIO_Free(self);
        }
    }
    return self;
}

// Report that the HIO device is available.
s32 gtl_CFileIoDeviceHIO_IsAvailable(void) {
    return 1;
}

} // extern "C"
