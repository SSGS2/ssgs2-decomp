extern "C" void* FileIOString_dtor_8041F6F4(void*, int);

// Normal destruction remains explicit. This owner handles exceptional exits.
struct EH5Cleanup {
    void* object;
    void (*destroy)(void*, int);
    bool active;
    // Adopt a local object for exceptional-exit destruction.
    EH5Cleanup(void* p, void (*d)(void*, int)) : object(p), destroy(d), active(true) {}
    // Disarm after explicit normal-path cleanup.
    void release() { active = false; }
    // Destroy only when normal-path cleanup has not already released ownership.
    ~EH5Cleanup() { if (active) destroy(object, -1); }
};

// Translation unit 0x8041E8CC-0x8041EDF8 (3 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "types.h"
#define char_array_guard CharArrayGuard_u8041E8CC
#include "game/stdexcept_local.h"
#undef char_array_guard
#include "sdk_protos.h"


// The shared declaration marks __throw nothrow. Dispatch through a volatile
// function pointer so enclosing owners retain their original unwind coverage.
// Construct a shared-message out_of_range exception and dispatch through the throw ABI.
static void EH5RaiseOutOfRange(const char* text, const char* typeName) {
    struct Error { void* table; char* message; void* control; } error;
    error.table = __vt__Q23std11logic_error;
    std::detail::default_delete_chars deleter;
    unsigned int length = strlen(text) + 1;
    error.message = new char[length];
    {
        std::detail::CharArrayGuard_u8041E8CC guard(error.message, deleter);
        error.control = new std::detail::message_control(error.message);
        guard.release();
    }
    strcpy(error.message, text);
    error.table = __vt__Q23std12out_of_range;
    typedef void (*ThrowFunction)(const char*, void*, void*);
    ThrowFunction volatile dispatch = __throw;
    dispatch(typeName, &error, (void*)std::__dt__Q23std12out_of_rangeFv);
}

extern "C" {
void FileIO_Free(void *p);
void FileIO_InvokeAllocatorCallback3(void *p);
void FileIOString_Reserve(void *buf, unsigned int len);
void FileIOString_ReplaceRange(void *buf, int a, int b, const char *c, const char *d, void *e);
char *strchr(const char *s, int c);
extern char filePathDotString[];

struct gtl_CCallBackDiskFreeSpace {
    virtual void vfunc_0(int);
};

// Release the callback object when invoked as a deleting destructor.
void *gtl_CCallBackDiskFreeSpace_dtor(gtl_CCallBackDiskFreeSpace *self, int flags) {
    if (self) {
        if (flags > 0) {
            FileIO_Free(self);
        }
    }
    return self;
}

/* 0x8041EC8C/0x8041ECA0: the pattern string is at self+0x04 (word) / +0x05 (short data) / +0x0C (long data) */
struct gtl_CFindHandleFMEMi_Layout {
    void* vtable;
    int patternFlags;
    unsigned int patternLength;
    char* patternData;
    char* packedNames;                  // 0x10: NUL-separated directory entries
    unsigned int packedNameBytes;      // 0x14: vector bounds checked before each lookup
    unsigned int packedNameCapacity;
    unsigned int nextNameOffset;       // 0x1C: byte cursor in packedNames
    unsigned int nameCount;
    unsigned int nextEntryIndex;
};
enum { kMatchAnyCharacter = '?', kMatchAnySequence = '*', kExtensionSeparator = '.',
       kFileAttributeDirectory = 0x10 };
struct FileListResult {
    unsigned int attributes;
    char reserved[0x28];
    char entryName[0x114]; // A bare packed directory entry, without a directory prefix.
};
struct FileListCallbackContext {
    void* callback;
    unsigned int owner;
    unsigned int headerBytes;
};
struct FilePathUtilityStrings {
    char wildcardPathSuffix[3];
    char pathSeparator[2];
    char fileMemoryDevice[6];
    char vectorLengthError[20];
    char lengthErrorType[55];
    char vectorRangeError[30];
    char outOfRangeType[55];
};
typedef char FileListResult_size[(sizeof(FileListResult) == 0x140) ? 1 : -1];
typedef char FindHandle_nextEntryIndex_offset[((int)&((gtl_CFindHandleFMEMi_Layout*)0)->nextEntryIndex == 0x24) ? 1 : -1];

// Test a case-sensitive entry name against the stored restricted wildcard pattern; return 1/0.
// Borrow candidateEntryName/self; the temporary candidate owns its copied string.
// The input is a bare directory-entry name. Append a dot when it contains no dot
// anywhere. This is NOT a general glob implementation:
// literals and ? consume one byte; * runs seek the FIRST occurrence of the next
// pattern byte, with no backtracking if a later match fails: a*ab.txt rejects
// aaab.txt. After *, ? is sought as a literal byte, not an arbitrary character.
// A trailing star run seeks NUL and can match an empty suffix after a literal.
// Runs reached after consuming a byte are handled before the next end check.
// Literal equality has precedence: a * in the name consumes a matching pattern
// * as one literal, which can produce different results from a general glob.
int gtl_CFindHandleFMEMi_MatchEntryName(gtl_CFindHandleFMEMi_Layout *self, const char *candidateEntryName) {
    char _pad[12];
    // CodeWarrior narrow string: 12 bytes, high tag bit selects heap storage;
    // heap form is flags/length/pointer, inline form is byte-length/text at +1.
    unsigned int candidateName[3] = {0, 0, 0};
    _pad[0] = 0;
    unsigned int candidateEntryNameLength = strlen(candidateEntryName);
    FileIOString_Reserve(candidateName, candidateEntryNameLength);
    // Preserve the reconstructed allocator-tag transport from the preceding
    // stack slot. These bytes are not candidateName metadata; their values are
    // unresolved ABI scaffolding. Do not replace them with a string length/tag.
    char copyAllocatorTag = *((unsigned char *)((char *)candidateName - 4));
    FileIOString_ReplaceRange(candidateName, 0, 0, candidateEntryName, candidateEntryName + candidateEntryNameLength, &copyAllocatorTag);
    EH5Cleanup eh5Cleanup38(candidateName, (void (*)(void*, int))FileIOString_dtor_8041F6F4);

    if (strchr(candidateEntryName, kExtensionSeparator) == 0) {
        unsigned int candidateLength;
        unsigned int first = candidateName[0];
        if (first >> 31) {
            candidateLength = candidateName[1];
        } else {
            /* 0x8041EC48: lbz 0x18(r1) - short length is byte 0 of the string, not the low byte of the word */
            candidateLength = *(unsigned char *)candidateName & 0x7fu;
        }
        unsigned int dotLength = strlen(filePathDotString);
        char dotAllocatorTag = *((unsigned char *)(&copyAllocatorTag) - 4);
        FileIOString_ReplaceRange(candidateName, candidateLength, 0, filePathDotString, filePathDotString + dotLength, &dotAllocatorTag);
    }

    char *pattern;
    if (self->patternFlags >> 31) {
        pattern = (char *)self->patternData;
    } else {
        pattern = (char *)&self->patternFlags + 1;
    }
    char *candidateText;
    unsigned int candidateUsesHeap;
    unsigned int first = candidateName[0];
    if (first >> 31) {
        candidateText = (char *)candidateName[2];
        candidateUsesHeap = 1;
    } else {
        candidateText = (char *)candidateName + 1;
        candidateUsesHeap = 0;
    }

    char *patternCursor = pattern;
    char *textCursor = candidateText;

    // Phase 2: advance cursors monotonically; no saved restart point exists.
    for (;;) {
        {
            signed char patternByte = (signed char)*patternCursor;
            signed char textByte;
            if (patternByte == 0 || (textByte = (signed char)*textCursor) == 0) {
                if (patternByte != 0) break;
                signed char remainingTextByte = (signed char)*textCursor;
                if (remainingTextByte != 0) break;
                if (candidateUsesHeap) { eh5Cleanup38.release(); FileIO_InvokeAllocatorCallback3((void *)candidateName[2]); }
                return 1;
            }
        }

        {
            signed char patternCharacter = (signed char)*patternCursor;
            signed char textCharacter = (signed char)*textCursor;
            if (patternCharacter != textCharacter && patternCharacter != kMatchAnyCharacter) {
                if (patternCharacter == kMatchAnySequence) {} else break;
            } else {
                patternCursor++;
                textCursor++;
            }
        }

        if (*patternCursor != kMatchAnySequence) continue;
        // Collapse the star run, then seek its following byte (including NUL).
        do { patternCursor++; } while (*patternCursor == kMatchAnySequence);
        {
            signed char target = (signed char)*patternCursor;
            while (1) {
                signed char textByte = (signed char)*textCursor;
                if (textByte == target) break;
                if (textByte == 0) break;
                textCursor++;
            }
        }
    }
    if (candidateUsesHeap) { eh5Cleanup38.release(); FileIO_InvokeAllocatorCallback3((void *)candidateName[2]); }
    return 0;
}

struct gtl_CCallBackFileList {
    virtual void vfunc_0(int);
    gtl_CFindHandleFMEMi_Layout* findHandle; /* 0x04 */
    char reserved[12];
    void* completionCallback; /* 0x14 */
    void* userData; /* 0x18 */
};

extern char filePathUtilityStrings[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12out_of_range[];
void __dt__Q23std12out_of_rangeFv(void*);
int NANDReadDir(void*, int, void*);
void File_SetLastError(int);
void *memset(void*, int, size_t);
char *strncpy(char*, const char*, size_t);

#pragma use_lmw_stmw on
// Consume packed names until the next match, probe directory status, and invoke completion.
// nextNameOffset advances past every tested NUL-terminated name, including a match;
// nextEntryIndex is written only on a match and is relative to this call's entryIndex=0.
// Packed names must fit result.entryName (0x114 bytes including NUL); copyLength is unchecked.
// Completion borrows &result only during the call. result is initialized ONLY on
// a match, yet completion also runs on exhaustion with operationResult unchanged.
// The enclosing search protocol must distinguish that case before reading result.
// This call consumes/frees context and self after completion; findHandle is borrowed.
void gtl_CCallBackFileList_FindNextMatchAndComplete(gtl_CCallBackFileList *self, void *operationResult, void *context) {
    int found = 0;
    unsigned int entryIndex = 0;
    unsigned int currentName[3] = {0, 0, 0};
    EH5Cleanup eh5Cleanup37(currentName, (void (*)(void*, int))FileIOString_dtor_8041F6F4);
    char allocatorTag = 0;
    FileListResult result;

    // Phase 1: copy each packed entry into an owned temporary and test it.
    // The count test precedes the body, so an empty list never reads packedNames.
    if (entryIndex < self->findHandle->nameCount) for (;;) {
        void *findState = self->findHandle;
        gtl_CFindHandleFMEMi_Layout* names = (gtl_CFindHandleFMEMi_Layout*)findState;
        unsigned int nameOffset = names->nextNameOffset;
        unsigned int namesByteCount = names->packedNameBytes;
        if (!(nameOffset < namesByteCount)) {
            // Reject an exhausted packed-name buffer.
            EH5RaiseOutOfRange(((FilePathUtilityStrings*)filePathUtilityStrings)->vectorRangeError, ((FilePathUtilityStrings*)filePathUtilityStrings)->outOfRangeType);
            return;
        }
        char *entryName = names->packedNames + nameOffset;
        unsigned int oldNameLength;
        if (currentName[0] >> 31) oldNameLength = currentName[1];
        else oldNameLength = *(unsigned char*)&currentName[0] & 0x7f; /* 0x8041E9E8: lbz 0x2c(r1) */
        allocatorTag = 0;
        size_t entryNameLength = strlen(entryName);
        FileIOString_ReplaceRange(&currentName[0], 0, oldNameLength, entryName, entryName + entryNameLength, &allocatorTag);

        // Advance past this NUL-terminated name before testing its pattern.
        void *cursorState = self->findHandle;
        gtl_CFindHandleFMEMi_Layout* cursor = (gtl_CFindHandleFMEMi_Layout*)cursorState;
        unsigned int nextOffset = cursor->nextNameOffset;
        unsigned int currentNameLength;
        if (currentName[0] >> 31) currentNameLength = currentName[1];
        else currentNameLength = *(unsigned char*)&currentName[0] & 0x7f; /* 0x8041EA38: lbz 0x2c(r1) */
        nextOffset = currentNameLength + nextOffset + 1;
        cursor->nextNameOffset = nextOffset;

        // Virtual slot 2 tests the current name against the find pattern.
        void *matchHandle = self->findHandle;
        char *matchName;
        if (currentName[0] >> 31) matchName = (char*)currentName[2];
        else matchName = (char*)&currentName[0] + 1;
        void **vtable = *(void***)matchHandle;
        typedef int (*MatchFunction)(void*, char*);
        int matches = ((MatchFunction)vtable[2])(matchHandle, matchName);
        if (matches == 0) {
            entryIndex++;
        } else {
            void *matchedState = self->findHandle;
            ((gtl_CFindHandleFMEMi_Layout*)matchedState)->nextEntryIndex = entryIndex + 1;
            found = 1;
            break;
        }

        // Stop when every directory entry has been tested.
        void *countState = self->findHandle;
        unsigned int nameCount = ((gtl_CFindHandleFMEMi_Layout*)countState)->nameCount;
        if (entryIndex >= nameCount) break;
    }

    // Phase 2: zero-fill the callback record and copy the bare matched name.
    // NANDReadDir success supplies the directory attribute; errors leave it clear.
    if (found) {
        memset(&result, 0, sizeof(result));
        unsigned int nameFlags = currentName[0];
        int inlineName = !(nameFlags >> 31);
        unsigned int copyLength;
        if (inlineName) copyLength = *(unsigned char*)&currentName[0] & 0x7f; /* 0x8041EADC: lbz 0x2c(r1) */
        else copyLength = currentName[1];
        char *copyText;
        if (inlineName) copyText = (char*)&currentName[0] + 1;
        else copyText = (char*)currentName[2];
        strncpy(result.entryName, copyText, copyLength);
        unsigned int directoryEntryCount = 0;
        int readDirResult = NANDReadDir(result.entryName, 0, &directoryEntryCount);
        if (readDirResult == 0) {
            result.attributes |= kFileAttributeDirectory;
        }
    }

    File_SetLastError(0);
    // Phase 3: synchronous completion sees borrowed local result and live findHandle.
    {
        typedef void (*CompletionFunction)(void*, void*, void*, void*);
        CompletionFunction completion = (CompletionFunction)self->completionCallback;
        completion(operationResult, &result, self->findHandle, self->userData);
    }

    if (context) {
        ((FileListCallbackContext*)context)->owner = 0; // 0x04
        ((FileListCallbackContext*)context)->headerBytes = 0; // 0x08
        FileIO_InvokeAllocatorCallback3(context);
    }

    // Phase 4: release callback-owned storage, then the temporary name.
    void **vtable = *(void***)self;
    typedef void (*DeletingDestructor)(void*, int);
    ((DeletingDestructor)vtable[2])(self, -1);
    FileIO_InvokeAllocatorCallback3(self);

    if (currentName[0] >> 31) {
        { eh5Cleanup37.release(); FileIO_InvokeAllocatorCallback3((void*)currentName[2]); }
    }
}
#pragma use_lmw_stmw reset

} // extern "C"
