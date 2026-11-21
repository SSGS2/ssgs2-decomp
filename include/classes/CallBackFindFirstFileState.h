// Non-virtual layout view of gtl::CCallBackFindFirstFile; methods in gtl_callback_find_first_file_dtor.cpp.
// 0x8041E25C: the high bit of the string flags gates release of the data pointer.
#ifndef CLASSES_CALLBACKFINDFIRSTFILESTATE_H
#define CLASSES_CALLBACKFINDFIRSTFILESTATE_H
struct CallBackFindFirstFileState {
    char pad_000[8];
    unsigned int stringFlags; // 0x8
    char pad_00C[4];
    void* stringData; // 0x10
};
#define OFF(f, o) typedef char CallBackFindFirstFileState_off_##f[((int)&((CallBackFindFirstFileState*)0)->f == (o)) ? 1 : -1]
OFF(stringFlags, 0x8);
OFF(stringData, 0x10);
#undef OFF
#endif
