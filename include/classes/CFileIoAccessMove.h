#ifndef CLASSES_CFILEIOACCESSMOVE_H
#define CLASSES_CFILEIOACCESSMOVE_H

#include "game/types.h"
#include "classes/CFileIoAccess.h"

class CFileIoAccessMove : public CFileIoAccess {
public:
    int state;                    // 0xc
    unsigned int secondPathFlags; // 0x10
    int secondPathLength;         // 0x14
    int secondPathData;           // 0x18
    unsigned int firstPathFlags;  // 0x1c
    int firstPathLength;          // 0x20
    int firstPathData;            // 0x24

    int SetPathsAndStart(const char* sourcePath, const char* destinationPath);
    int vfunc_2();
    int IsFinished();
    int Update();

    CFileIoAccessMove();
    ~CFileIoAccessMove();
};

#define CFILEIOACCESSMOVE_OFF(f, o) \
    typedef char CFileIoAccessMove_off_##f[((int)&((CFileIoAccessMove*)0)->f == (o)) ? 1 : -1]
CFILEIOACCESSMOVE_OFF(state, 0xc);
CFILEIOACCESSMOVE_OFF(secondPathFlags, 0x10);
CFILEIOACCESSMOVE_OFF(secondPathLength, 0x14);
CFILEIOACCESSMOVE_OFF(secondPathData, 0x18);
CFILEIOACCESSMOVE_OFF(firstPathFlags, 0x1c);
CFILEIOACCESSMOVE_OFF(firstPathLength, 0x20);
CFILEIOACCESSMOVE_OFF(firstPathData, 0x24);
#undef CFILEIOACCESSMOVE_OFF

#endif // CLASSES_CFILEIOACCESSMOVE_H
