// Translation unit 0x8040465C-0x80404674 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "classes/SoundStreamSourceState.h"
extern "C" {

// Return the stored source pointer when requested; a null output still succeeds.
// self must be readable through +0x9C only when sourceOutput is nonnull; then
// sourceOutput must address one writable pointer. The stored pointer may be null.
// This is a raw pointer copy: no reference is acquired or released. Its pointee
// type and lifetime remain unresolved; 1 is returned on both paths.
int gtl_CSoundStreamSource_GetStreamCallback(void* self, void** sourceOutput) {
	if (sourceOutput != 0) {
		*sourceOutput = AsSoundStreamSourceState(self)->sourcePointer;
	}
	return 1;
}

} // extern "C"
