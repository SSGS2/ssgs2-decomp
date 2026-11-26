// Destructor thunk that releases a block through the file-I/O free hook (FileIO_Free).
// It adds no checks of its own: null and invalid pointers are handled by FileIO_Free.

extern "C" {
extern void FileIO_Free(void* p);

// Release an allocation through the file-I/O allocator's free hook.
// Ownership of allocation is surrendered; no member destructors run here.
// Valid-pointer/null handling follows FileIO_Free; the thunk adds no checks.
void dtor_8041D344(void* allocation) {
    FileIO_Free(allocation);
}

} // extern "C"
