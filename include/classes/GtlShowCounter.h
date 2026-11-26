// Show counter: a count of displayed frames that advances only while counting is active.
// The state word selects counting (0), paused (1) or reset-on-next-start (2).
// The media filters and the revolution display counter embed or point at this layout.
#ifndef CLASSES_GTLSHOWCOUNTER_H
#define CLASSES_GTLSHOWCOUNTER_H

namespace gtl {

class IShowCounter {
public:
    // Initialize a zeroed counter that clears its history when first started.
    void Init();
    // Free the storage when flag is positive; returns the object.
    void* Destroy(int flag);

    // Counting control.
    // Start counting, clearing both counts only when a reset was armed.
    int StartCounting();
    int Pause();
    int ArmResetOnStart();
    // Run the counter's update hook, then arm a reset; always returns 1.
    int ResetAndStartCounting();
    // Advance the count while counting (state 0), keeping the old count as the previous one.
    int AdvanceCount();

    // Counts and state.
    int GetCount(int* out);
    int GetPreviousCount(int* out);
    int SetCount(unsigned int value);
    int SetPreviousCount(unsigned int value);
    int ResetCounts();
    int GetState(unsigned int* value);
    int SetState(unsigned int value);

    unsigned int unk_0; // 0x0: set to 1 by Init
    void* vtbl;         // 0x4
    int state;          // 0x8
    int count;          // 0xc
    int previousCount;  // 0x10
};

} // namespace gtl

#define GTL_ISHOWCOUNTER_OFF(f, o) \
    typedef char gtl_IShowCounter_off_##f[((int)&((gtl::IShowCounter*)0)->f == (o)) ? 1 : -1]
GTL_ISHOWCOUNTER_OFF(unk_0, 0x0);
GTL_ISHOWCOUNTER_OFF(vtbl, 0x4);
GTL_ISHOWCOUNTER_OFF(state, 0x8);
GTL_ISHOWCOUNTER_OFF(count, 0xc);
GTL_ISHOWCOUNTER_OFF(previousCount, 0x10);
#undef GTL_ISHOWCOUNTER_OFF

#endif
