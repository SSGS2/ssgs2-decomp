// The stream callback getter of a streamed sound source (gtl::CSoundStreamSource::GetStreamCallback).
// It copies the stored callback pointer to the caller when an output is given.

#include "classes/SoundStreamSource.h"

// Return the stored callback pointer when requested; a null output still succeeds.
// This is a raw pointer copy: no reference is acquired or released. The stored
// pointer may be null. Returns 1 on both paths.
int gtl::CSoundStreamSource::GetStreamCallback(void** sourceOutput) {
    if (sourceOutput != 0) {
        *sourceOutput = callback;
    }
    return 1;
}
