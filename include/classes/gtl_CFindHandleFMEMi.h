#ifndef CLASSES_GTL_CFINDHANDLEFMEMI_H
#define CLASSES_GTL_CFINDHANDLEFMEMI_H

namespace gtl {

// Directory-search state of a file listing: the wildcard pattern and the packed
// NUL-separated entry names, with a byte cursor for the next entry.
class CFindHandleFMEMi {
public:
    // Return 1 when the bare entry name matches the stored pattern, 0 otherwise.
    int MatchEntryName(const char* candidateEntryName);

    void* vtable;                    // 0x0
    int patternFlags;                // 0x4: top bit selects heap pattern storage
    unsigned int patternLength;      // 0x8
    char* patternData;               // 0xC: heap pattern pointer
    char* packedNames;               // 0x10: NUL-separated directory entries
    unsigned int packedNameBytes;    // 0x14: vector bounds checked before each lookup
    unsigned int packedNameCapacity; // 0x18
    unsigned int nextNameOffset;     // 0x1C: byte cursor in packedNames
    unsigned int nameCount;          // 0x20
    unsigned int nextEntryIndex;     // 0x24
};

} // namespace gtl

#endif // CLASSES_GTL_CFINDHANDLEFMEMI_H
