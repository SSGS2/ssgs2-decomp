// Translation unit 0x803F9A3C-0x803F9A9C (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

void gtl_CSamplingFilter_dtor(void*, int);
void gtl_Free(void*);

// Destroy the sampling-filter base and optionally release this object's storage.
// Null is accepted; deleteFlag > 0 frees self, otherwise caller retains storage.
// The returned address is unchanged and may already have been freed.
void* gtl_CRevolutionSamplingFilter_dtor(void* self, int deleteFlag) {
	if (self != 0) {
		gtl_CSamplingFilter_dtor(self, 0);
		if (deleteFlag > 0 && self != 0) {
			gtl_Free(self);
		}
	}
	return self;
}

} // extern "C"
