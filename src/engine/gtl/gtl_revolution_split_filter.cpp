// Translation unit 0x803F9D60-0x803F9DC0 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

void* gtl_CSplitFilter_dtor(void*, int);
void gtl_Free(void*);

// Destroy the base split-filter resources, then optionally free the derived storage.
// Null is accepted. Base destruction receives 0 so that it leaves self allocated;
// deleting > 0 frees that storage with gtl_Free. Returns self, possibly freed.
void* gtl_CRevolutionSplitFilter_dtor(void* self, int deleting) {
	if (self != 0) {
		gtl_CSplitFilter_dtor(self, 0);
		if (deleting > 0) {
			if (self != 0)
				gtl_Free(self);
		}
	}
	return self;
}

} // extern "C"
