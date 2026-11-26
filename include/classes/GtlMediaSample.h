// gtl::CMediaSample: one buffer of media data with its format descriptor and byte count. Its storage is owned by
// the caller (the sample pool or the filter that allocated it). The class name is inferred from the symbols.
#ifndef CLASSES_GTLMEDIASAMPLE_H
#define CLASSES_GTLMEDIASAMPLE_H

namespace gtl {

class CMediaSample {
public:
    void Init(int data, int byteCount, int format);
    void* Destroy(int deleteFlag);

    int CopyFormat(void* out);
    int GetData(int* out);
    int GetSize(int* out);
    int SetBuffer(int dataAddress, int byteCount);
    int SetFormat(void* newFormat);

    int references; // 0x0
    void* vtbl;     // 0x4
    int format;     // 0x8
    int data;       // 0xC
    int size;       // 0x10
};

#define OFF(f, o) typedef char CMediaSample_off_##f[((int)&((CMediaSample*)0)->f == (o)) ? 1 : -1]
OFF(references, 0x0);
OFF(vtbl, 0x4);
OFF(format, 0x8);
OFF(data, 0xC);
OFF(size, 0x10);
#undef OFF

} // namespace gtl

#endif
