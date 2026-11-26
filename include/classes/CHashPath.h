// CHashPath: the path lookup table built from the game's path-hash file.
//
// One table is loaded at a time and pointed to by a global. It holds the hash
// slots, each slot's bucket index and the buckets of padded path strings;
// FindPath matches a requested path's tail against the strings in the bucket
// its basename hash selects. The vtable pointer is part of the original object,
// so it is a plain member.
#ifndef CLASSES_CHASHPATH_H
#define CLASSES_CHASHPATH_H

extern "C" {
extern char __vt__9CHashPath[];
}

class CHashPath {
public:
    CHashPath();
    ~CHashPath();

    // The stored path whose tail matches path, ignoring case; null if absent.
    static void* FindPath(const char* path);

    // Same lookup as FindPath, called on the object that holds the table. The
    // object itself is not used.
    void* FindPathForwarder(const char* path);
    // Whether the path is registered.
    int Exists(const char* path);

    char* strings;        // 0x00 the loaded file image
    unsigned short count; // 0x04 number of hash slots
    char pad_06[2];
    unsigned short* hashes; // 0x08 bucket index of each hash slot
    unsigned int* offsets;  // 0x0C halfword offset of each bucket
    void* vtbl;             // 0x10
};

inline CHashPath::CHashPath() {
    vtbl = __vt__9CHashPath;
    strings = 0;
    count = 0;
    hashes = 0;
    offsets = 0;
}

typedef char CHashPath_offset_strings[((int)&((CHashPath*)0)->strings == 0x00) ? 1 : -1];
typedef char CHashPath_offset_count[((int)&((CHashPath*)0)->count == 0x04) ? 1 : -1];
typedef char CHashPath_offset_hashes[((int)&((CHashPath*)0)->hashes == 0x08) ? 1 : -1];
typedef char CHashPath_offset_offsets[((int)&((CHashPath*)0)->offsets == 0x0C) ? 1 : -1];
typedef char CHashPath_offset_vtbl[((int)&((CHashPath*)0)->vtbl == 0x10) ? 1 : -1];
typedef char CHashPath_size[(sizeof(CHashPath) == 0x14) ? 1 : -1];

#endif // CLASSES_CHASHPATH_H
