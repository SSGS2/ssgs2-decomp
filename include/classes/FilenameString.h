// FilenameString. FilenameString_Ctor and path parsing helpers in sound_manager.cpp.
// Explicit layout view without inheritance or virtual methods.
#ifndef CLASSES_FILENAMESTRING_H
#define CLASSES_FILENAMESTRING_H

#include "classes/MWStringStorage.h"

class FilenameString {
public:
    MWStringStorage
        input; // 0x0: FilenameString_SplitAndGetStem splits input with FileIO_SplitPath and builds derived path strings
    MWStringStorage
        drive; // 0xC: FilenameString_SplitAndGetStem splits input with FileIO_SplitPath and builds derived path strings
    MWStringStorage
        directory; // 0x18: FilenameString_SplitAndGetStem splits input with FileIO_SplitPath and builds derived path strings
    MWStringStorage
        stem; // 0x24: FilenameString_SplitAndGetStem splits input with FileIO_SplitPath and builds derived path strings
    MWStringStorage
        extension; // 0x30: FilenameString_SplitAndGetStem splits input with FileIO_SplitPath and builds derived path strings
    MWStringStorage
        driveDirectory; // 0x3C: FilenameString_SplitAndGetStem splits input with FileIO_SplitPath and builds derived path strings
    MWStringStorage
        unk_48; // 0x48: FilenameString_SplitAndGetStem splits input with FileIO_SplitPath and builds derived path strings
    MWStringStorage
        fileName; // 0x54: FilenameString_SplitAndGetStem splits input with FileIO_SplitPath and builds derived path strings
    void* vtable; // 0x60: FilenameString_Ctor stores __vt__14filenamestring
};

#define OFF(f, o) typedef char FilenameString_off_##f[((int)&((FilenameString*)0)->f == (o)) ? 1 : -1]
OFF(input, 0x0);
OFF(drive, 0xC);
OFF(directory, 0x18);
OFF(stem, 0x24);
OFF(extension, 0x30);
OFF(driveDirectory, 0x3C);
OFF(unk_48, 0x48);
OFF(fileName, 0x54);
OFF(vtable, 0x60);
#undef OFF
#define AsFilenameString(p) ((FilenameString*)(p))

#endif
