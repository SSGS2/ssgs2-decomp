// Translation unit 0x80404D8C-0x80404DCC (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

void gtl_Audio_Free(void*);

// Destroy the stream-data load task, optionally releasing its object storage.
// self may be null; only a positive deleteFlag calls the audio allocator's free hook.
// No members are cleaned up here. Return the original address even after freeing it;
// that return value does not grant ownership or make freed storage usable.
void* CSoundStreamSource_StrmDataLoadTask_dtor(void* self, int deleteFlag) {
    if (self != 0 && deleteFlag > 0) {
        gtl_Audio_Free(self);
    }
    return self;
}

} // extern "C"
