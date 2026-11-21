// PADMEM file-I/O device's virtual slot 20 implementation.
// The slot's operation and the caller's interpretation of zero are unresolved.
extern "C" {

// Return the slot's fixed zero result without reading or modifying device state.
// No ownership transfer; do not infer "success" or "unsupported" from zero alone.
int gtl_CFileIoDevicePADMEM_vfunc_20(void) {
    return 0;
}

} // extern "C"
