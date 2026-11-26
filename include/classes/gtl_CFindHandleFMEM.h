#ifndef CLASSES_GTL_CFINDHANDLEFMEM_H
#define CLASSES_GTL_CFINDHANDLEFMEM_H

namespace gtl {

// Find handle of the memory-card-free (FMEM) file device. Its +4 word and +0xC
// word hold the pattern: see MatchesCandidate.
class CFindHandleFMEM {
public:
    // Return 1 when the candidate path matches the stored pattern, 0 otherwise.
    int MatchesCandidate(const char* path);

    void* field00; // 0x0
    int field04;   // 0x4: pattern length or heap tag
    char pad_08[0x4];
    void* field0C; // 0xC: heap pattern pointer
};

} // namespace gtl

#endif // CLASSES_GTL_CFINDHANDLEFMEM_H
