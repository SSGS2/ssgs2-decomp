// gtl environment lighting: report the number of light references in a loaded
// environment resource. API lookup: GetLightCount here; CopyLights (environment_light_resource_export.cpp)
// copies that many 0x88-byte light descriptions; environment Apply (gtl_generic_environment.cpp)
// uses the reference-list tag to select lights for render slots.
// Producer: gtl_IGenericEnvironmentResource_CreateFromMemory (gtl_generic_resource_loading.cpp) copies
// input bytes into an IG3DXBuffer and retains it at wrapper +8. Destructor
// (gtl_generic_environment_resource.cpp) decrements its reference and releases at zero. Queries borrow it;
// neither output nor this function acquires a reference. Keep the wrapper live.
// Resource directory schema (gtl_image_decode_and_tiling.cpp, native big-endian Wii words): data +0x14
// is entry count; data +0x20 is an array of 16-byte {tag,count,stride,offset}.
// light-reference list (Apply uses 12-byte references); 0x646C selects actual
// 0x88-byte light records (CopyLights). CopyLights copies the first reference-count
// descriptions, so that chunk must contain at least as many records; this query
// does not validate their relationship or file version.
// Object_GetField8 is a raw +8 accessor (buffer data here); Object_GetFieldC is
// raw +0xC (buffer length here). The latter is passed as an obsolete second ABI
// argument, NOT a chunk pointer: GetChunkElementCount ignores it and scans data.
// No gameplay light-count limit is defined here; edit environment data/Apply.
// Translation unit 0x803DBB84-0x803DBBEC (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

struct gtl_IGenericEnvironmentResource {
    char reserved00[8];
    void* resourceBuffer; // Retained IG3DXBuffer; borrowed for the duration of this query.
};

void* Object_GetFieldC(void* obj);
void* Object_GetField8(void* obj);
int gtl_ResourceFile_GetChunkElementCount(void* file, void* chunk, unsigned int tag);

enum { kEnvironmentLightReferenceTag = 0x0067696C };

// Required self/res/data and writable count (one int); trusted directory must fit
// the live resource. Writes the FIRST matching chunk's element count, or zero
// if absent, then returns 1 in both cases. Return value is completion, not count
// or proof of validity. No null/bounds/version checks or recoverable failure exit;
// invalid inputs can fault. Output must not overlap resource/wrapper storage.
int gtl_IGenericEnvironmentResource_GetLightCount(gtl_IGenericEnvironmentResource* self, int* count) {
    void* ignoredLengthWord = Object_GetFieldC(self->resourceBuffer);
    *count = gtl_ResourceFile_GetChunkElementCount(Object_GetField8(self->resourceBuffer), ignoredLengthWord, kEnvironmentLightReferenceTag);
    return 1;
}

} // extern "C"
