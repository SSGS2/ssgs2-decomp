// Translation unit 0x804130C4-0x804130C8 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

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
