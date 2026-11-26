// MWStringStorage: offset-preserving view; no constructors, bases, or virtual methods.
// Evidence: StdString_ReserveCapacity (0x800B11E8) tests the high bit of flags, reallocates data, and stores length; fn_string_len/fn_string_ptr select small or heap storage.
#ifndef CLASSES_MWSTRINGSTORAGE_H
#define CLASSES_MWSTRINGSTORAGE_H

struct MWStringStorage {
    unsigned int flags;  // 0x0
    unsigned int length; // 0x4
    char* data;          // 0x8
};

#define OFF(f, o) typedef char MWStringStorage_off_##f[((int)&((MWStringStorage*)0)->f == (o)) ? 1 : -1]
OFF(flags, 0x0);
OFF(length, 0x4);
OFF(data, 0x8);
#undef OFF
typedef char MWStringStorage_size[(sizeof(MWStringStorage) == 0xC) ? 1 : -1];
#define AsMWStringStorage(p) ((MWStringStorage*)(p))

#endif // CLASSES_MWSTRINGSTORAGE_H
