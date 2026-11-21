// gtl screen-pointer input: expose the device-owned raw sample buffer.
// API/consumer: CScreenPointer::CScreenPointer zeroes m_samples (game/gtl_input.h,
// gtl_input_ex_devices_part1.cpp); gtl_CScreenPointer_PollAndNormalizeHistory obtains this buffer through virtual slot
// 1, converts the first sample's 16 button bytes, and normalizes each reported
// sample into m_history. This accessor does not poll hardware or normalize data.
// Mod lookup: raw input layout below; normalization and device parameter ranges
// are in vfunc_6. These are device values, not screen pixels or world positions.
// Translation unit 0x803E9BB8-0x803E9BC0 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

// pointers into it. Unused bytes remain opaque: axis meanings are not established.
struct ScreenPointerRawSample {
    int parameter00, parameter04;       // / GetItemParameter(1,0/4)[1]
    char reserved08[4];
    int parameter0C, parameter10;       // / GetItemParameter(1,0xC/0x10)[1]
    char reserved14[0x1C];
    signed char buttons[16];           // +30: any nonzero byte means pressed
    char reserved40[0x10];
    int parameter50, parameter54, parameter58, parameter5C; // normalized likewise
    int nonnegativeResult;             // +60: consumer maps negatives to zero
    char reserved64[0x4C];
};
struct ScreenPointerBufferView {
    char deviceBase[0x14];
    ScreenPointerRawSample samples[16]; // +14..+B13, followed by 16 history entries
};
typedef char ScreenPointerSampleSize[(sizeof(ScreenPointerRawSample) == 0xB0) ? 1 : -1];
enum { kScreenPointerSamplesOffset = (int)&((ScreenPointerBufferView*)0)->samples };

// Return a mutable borrowed pointer to all 0xB00 bytes (16 records of 0xB0).
// Required self is the complete CScreenPointer receiver, not its +4 vtable slot.
// No failure/status value, allocation, reference increment, or mutation here.
// Caller must not free the result; it dies with self and contents may be replaced
// by the next input read. Serialize access with input polling. It is not a snapshot.
void* gtl_CScreenPointer_GetRawSamples(void* self) {
    return (char*)self + kScreenPointerSamplesOffset;
}

} // extern "C"
