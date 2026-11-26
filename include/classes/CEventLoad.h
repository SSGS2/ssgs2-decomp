// CEventLoad: methods live in story_message_event_load.cpp.
// Non-virtual layout view; original function addresses are cited per field.
#ifndef CLASSES_CEVENTLOAD_H
#define CLASSES_CEVENTLOAD_H

class CEventLoad {
public:
    unsigned int state;      // 0x0: CEventLoad_Poll: switch and transitions 1/2/3
    char asyncFile[16];      // 0x4: CEventLoad_Poll: CFileAsync_Open/80068498 receiver
    unsigned int nameFlags;  // 0x14: CEventLoad_Poll: high bit selects inline or heap name
    unsigned int nameLength; // 0x18: string storage
    void* nameHeap;          // 0x1C: CEventLoad_Poll: heap name pointer
    void* loadFlags;         // 0x20: CEventLoad_ctor: saved argument
    void* vtable;            // 0x24: CEventLoad_ctor: __vt__10CEventLoad
    unsigned int bufferSize; // 0x28: CEventLoad_Poll: aligned allocation size
    void* buffer;            // 0x2C: CEventLoad_Poll: appMallocUni result

    int Poll();

    CEventLoad(const char* text, void* arg, void* unused);
    ~CEventLoad();
};

#define CEVENTLOAD_OFF(f, o) typedef char CEventLoad_off_##f[((int)&((CEventLoad*)0)->f == (o)) ? 1 : -1]
CEVENTLOAD_OFF(state, 0x0);
CEVENTLOAD_OFF(asyncFile, 0x4);
CEVENTLOAD_OFF(nameFlags, 0x14);
CEVENTLOAD_OFF(nameLength, 0x18);
CEVENTLOAD_OFF(nameHeap, 0x1C);
CEVENTLOAD_OFF(loadFlags, 0x20);
CEVENTLOAD_OFF(vtable, 0x24);
CEVENTLOAD_OFF(bufferSize, 0x28);
CEVENTLOAD_OFF(buffer, 0x2C);
#undef CEVENTLOAD_OFF

#endif // CLASSES_CEVENTLOAD_H
