// FmemFileStat: layout view; gtl_file_io_device_fmem_stat.cpp: vfunc_8 writes vfunc_7 file-size result at +0x24 and sets +0x28 to one
// No inheritance, virtual declarations, constructors, or emitted runtime objects.
#ifndef CLASSES_FMEMFILESTAT_H
#define CLASSES_FMEMFILESTAT_H

struct FmemFileStat {
    char pad_000[0x24];
    int size;   // 0x24
    int unk_28; // 0x28: vfunc_8 writes 1; meaning unknown.
};

#define OFF(f, o) typedef char FmemFileStat_off_##f[((int)&((FmemFileStat*)0)->f == (o)) ? 1 : -1]
OFF(size, 0x24);
OFF(unk_28, 0x28);
#undef OFF
#define AsFmemFileStat(p) ((FmemFileStat*)(p))

#endif
