// Translation unit 0x8041D344-0x8041D348 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {
extern void FileIO_Free(void *p);

// Release an allocation through the file-I/O allocator's free hook.
// Ownership of allocation is surrendered; no member destructors run here.
// Valid-pointer/null handling follows FileIO_Free; the thunk adds no checks.
void dtor_8041D344(void *allocation) { FileIO_Free(allocation); }

} // extern "C"
