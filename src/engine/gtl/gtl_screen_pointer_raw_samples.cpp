// gtl screen-pointer input: expose the device-owned raw sample buffer.
// API/consumer: the gtl::CScreenPointer constructor zeroes m_samples (game/gtl_input.h,
// gtl_input_ex_devices_part1.cpp); PollAndNormalizeHistory obtains this buffer through virtual slot
// 1, converts the first sample's 16 button bytes, and normalizes each reported
// sample into m_history. This accessor does not poll hardware or normalize data.
// Mod lookup: raw input layout in game/gtl_input.h; normalization and device parameter ranges
// are in PollAndNormalizeHistory. These are device values, not screen pixels or world positions.

#include "types.h"
#include "game/gtl_input.h"

namespace gtl {

// Return a mutable borrowed pointer to all 0xB00 bytes (16 records of 0xB0).
// The result is this object's own storage, not its +4 vtable slot.
// No failure/status value, allocation, reference increment, or mutation here.
// Caller must not free the result; it dies with the object and contents may be replaced
// by the next input read. Serialize access with input polling. It is not a snapshot.
void* CScreenPointer::GetRawSamples() {
    return m_samples;
}

} // namespace gtl
