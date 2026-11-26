// File-layer storage release: frees an allocation through FileIO_Free. The wrapper runs no member
// cleanup and adds no null check.

extern "C" {

// Release an allocation through the file-I/O allocator.
void FileIO_Free(void* allocatedStorage);

// Forward storage disposal to FileIO_Free; no member cleanup is performed here.
// allocatedStorage must satisfy FileIO_Free's allocator contract; its ownership
// is consumed. This wrapper adds no null check or deleting-destructor flag.
void dtor_804130C4(void* allocatedStorage) {
    FileIO_Free(allocatedStorage);
}

} // extern "C"
