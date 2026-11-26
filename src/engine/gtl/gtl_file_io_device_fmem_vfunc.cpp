// FMEM file I/O device: the ChangeDirectory method, which sets the NAND current directory.

#include "classes/GtlFileIoDeviceFMEM.h"

extern "C" int NANDChangeDir(void* directoryPath);

// Set the NAND current directory through FMEM virtual slot 32.
// directoryPath is a borrowed NUL-terminated path, forwarded without copying or
// validation; return the NAND status unchanged, with no File_SetLastError translation.
// The process-wide directory change is not restored.
int gtl::CFileIoDeviceFMEM::ChangeDirectory(void* directoryPath) {
    return NANDChangeDir(directoryPath);
}
