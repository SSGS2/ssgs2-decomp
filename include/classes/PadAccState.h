// PadAccState: per-pad motion history holder. Owns two sample buffers and
// remembers the pad's pointer-sampling parameters while it overrides them.
// Its vtable is the one named CAccState in the symbol table; the class
// declares no virtuals itself.
#ifndef CLASSES_PADACCSTATE_H
#define CLASSES_PADACCSTATE_H

// Each buffer owns one allocation; size counts records, not bytes.
struct SampleBuffer {
    void* data;
    int size;
};

struct SampleBufferOwner {
    int pad[3];
    SampleBuffer samples;
};

class PadAccState {
public:
    ~PadAccState();
    void* Init(int playerIndex);

    int* vtable;                        // 0x00
    SampleBufferOwner primaryHistory;   // 0x04
    char pad_018[4];                    // 0x18
    SampleBufferOwner secondaryHistory; // 0x1C
    char pad_030[0xC];                  // 0x30
    int playerIndex;                    // 0x3C
    int savedSamplingParameters;        // 0x40
    int savedSamplingParameter2;        // 0x44
    int samplingOverridden;             // 0x48
};

#define PADACCSTATE_OFF(f, o) typedef char PadAccState_off_##f[((int)&((PadAccState*)0)->f == (o)) ? 1 : -1]
PADACCSTATE_OFF(primaryHistory, 0x04);
PADACCSTATE_OFF(secondaryHistory, 0x1C);
PADACCSTATE_OFF(playerIndex, 0x3C);
PADACCSTATE_OFF(samplingOverridden, 0x48);
#undef PADACCSTATE_OFF

#endif // CLASSES_PADACCSTATE_H
