// Translation unit 0x803F9218-0x803F925C (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

void gtl_Free(void*, int);

// Optionally free the show-counter allocation through gtl_Free; return its old address.
// deletingFlag > 0 releases storage and is forwarded to the allocator unchanged.
// No members are released here; a returned freed address must not be dereferenced.
void* gtl_CRevolutionShowCounter_dtor(void* self, int deletingFlag) {
	if (self != 0) {
		if (deletingFlag > 0 && self != 0) {
			gtl_Free(self, deletingFlag);
		}
	}
	return self;
}

} // extern "C"
