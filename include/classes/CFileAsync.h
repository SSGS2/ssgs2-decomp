#ifndef CLASSES_CFILEASYNC_H
#define CLASSES_CFILEASYNC_H

#include "game/types.h"
#include <decomp/offset_of.h>

// An asynchronous DVD file reader. It holds the open file handle, whether reads
// complete synchronously, and the size requested by the last read.
// No virtual functions are declared; the vtable pointer is set by the constructors.
class CFileAsync {
public:
    // A closed file with no pending read.
    CFileAsync();
    // Open the DVD path; the handle stays invalid if the open fails.
    CFileAsync(const char* path, int synchronous);
    ~CFileAsync();

    int Open(const char* path, int synchronous);
    int GetSize() const;
    void Close();
    // The destination must stay valid until PollRead reports completion.
    int StartRead(void* destination, int byteCount);
    // The callers pass one more word, which this function never reads.
    int PollRead(int* out, int* value, int unusedWord);

    unsigned int handle;            // 0x00
    int synchronous;                // 0x04
    unsigned int requestedReadSize; // 0x08
    void* vtable;                   // 0x0C
};

typedef char CFileAsync_size[(sizeof(CFileAsync) == 0x10) ? 1 : -1];
typedef char CFileAsync_handle_offset[((int)DECOMP_OFFSETOF(CFileAsync, handle) == 0x00) ? 1 : -1];
typedef char CFileAsync_vtable_offset[((int)DECOMP_OFFSETOF(CFileAsync, vtable) == 0x0C) ? 1 : -1];

#endif // CLASSES_CFILEASYNC_H
