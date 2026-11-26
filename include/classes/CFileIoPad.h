// CFileIoPad: file I/O through a controller-memory pad channel.
//
// It adds the channel number to CFileIo and supplies the pad-qualified path and
// the callbacks that finish a transfer. The vtable comes from the original data,
// so it is not declared virtual here.
#ifndef CLASSES_CFILEIOPAD_H
#define CLASSES_CFILEIOPAD_H

#include "classes/CFileIo.h"

class CFileIoPad : public CFileIo {
public:
    ~CFileIoPad();

    // Device capability and lifecycle hooks (vtable slots 3 to 14).
    int vfunc_3();
    int vfunc_4();
    int vfunc_5();
    int vfunc_6();
    int vfunc_10();
    int vfunc_11();
    int vfunc_8();
    int ReadAlignedBuffer();
    int Open(void* openArgument, void* unused);
    int StartPaddedWrite(void* data, void* unused);
    int HasFileError();
    int vfunc_9();
    static int vfunc_14();

    // Return the shared controller file object, or construct and open one.
    static void* CreateInstance(void* controller, int openArgument1, int openArgument2);

    // Prefix the requested path with the pad channel's directory.
    int SetPathAndAllocateBuffer(const char* name, int requestedBufferSize);

    static void OnOpenComplete(int status);
    static void OnTransferComplete(int status, int bytesTransferred);

    int padIndex; // 0x58  controller channel
};

#define CFILEIOPAD_OFF(f, o) typedef char CFileIoPad_off_##f[((int)&((CFileIoPad*)0)->f == (o)) ? 1 : -1]
CFILEIOPAD_OFF(padIndex, 0x58);
#undef CFILEIOPAD_OFF
typedef char CFileIoPad_size[(sizeof(CFileIoPad) == 0x5C) ? 1 : -1];

#endif // CLASSES_CFILEIOPAD_H
