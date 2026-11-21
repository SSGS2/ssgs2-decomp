// Translation unit 0x8041D728-0x8041D730 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {
// Change the process-wide NAND directory and return the NAND API status.
int NANDChangeDir(void* directoryPath);

// Set the NAND current directory through FMEM virtual slot 32.
// self is unused. directoryPath is a borrowed NUL-terminated path, forwarded
// without copying or validation; return the NAND status unchanged, with no
// File_SetLastError translation. The process-wide directory change is not restored.
int gtl_CFileIoDeviceFMEM_vfunc_32(void* self, void* directoryPath) {
    return NANDChangeDir(directoryPath);
}
} // extern "C"
