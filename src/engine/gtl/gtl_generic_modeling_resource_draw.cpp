// Translation unit 0x803D9498-0x803D94A4 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

void gtl_Model_DrawWithFrustumCulling(void* a, void* b, void* c, void* d);

void gtl_IGenericModelingResource_QueueDraw(void* self, void* b, void* c) {
    gtl_Model_DrawWithFrustumCulling((char*)self + 0xc, b, c, self);
}

} // extern "C"
