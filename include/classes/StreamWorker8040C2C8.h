// Streaming worker singleton returned by gtl_AudioWorker_GetInstance, owned by gtl_audio_worker_strm_player.cpp.
// Address-only storage views. InitStreamSingleton supplies these addresses to
// OSInitThreadQueue, OSInitMessageQueue (four messages), and OSInitMutex (mutex).
#ifndef CLASSES_STREAMWORKER8040C2C8_H
#define CLASSES_STREAMWORKER8040C2C8_H

class StreamWorker8040C2C8 {
public:
    char pad_000[0x318];
    char threadQueue[0x8];    // 0x318: original 0x8040C3C0 passes to OSInitThreadQueue
    char messageQueue[0x20];  // 0x320: original 0x8040C394 passes to OSInitMessageQueue; send/receive commands
    char messageBuffer[0x10]; // 0x340: original 0x8040C398 supplies four OSMessage entries to OSInitMessageQueue
    char pad_350[4];
    char mutex[0x18]; // 0x354: original 0x8040C3C8 passes to OSInitMutex; GlobalLock/StartLock lock it
};

#define STREAMWORKER8040C2C8_OFF(f, o) \
    typedef char StreamWorker8040C2C8_off_##f[((int)&((StreamWorker8040C2C8*)0)->f == (o)) ? 1 : -1]
STREAMWORKER8040C2C8_OFF(threadQueue, 0x318);
STREAMWORKER8040C2C8_OFF(messageQueue, 0x320);
STREAMWORKER8040C2C8_OFF(messageBuffer, 0x340);
STREAMWORKER8040C2C8_OFF(mutex, 0x354);
#undef STREAMWORKER8040C2C8_OFF

#endif
