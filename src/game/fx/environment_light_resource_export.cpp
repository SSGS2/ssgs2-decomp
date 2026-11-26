// Environment light resource export. Mod lookup: CopyLights copies raw light
// records; gtl_environment_light_count.cpp queries the matching count; gtl_generic_resource_loading.cpp converts each
// exported record to a fixed-function light description. This API copies data,
// not resource handles, and supplies no lighting intensity tuning of its own.

extern "C" {

void* Object_GetFieldC(void*);
void* Object_GetField8(void*);
int gtl_ResourceFile_GetChunkElementCount(void*, void*, int);
void* gtl_ResourceFile_GetChunkElement(void*, void*, int, unsigned int);
void* memcpy(void*, const void*, unsigned int);

struct GenericEnvironmentResource {
    char unk0[8];
    void* resourceBuffer; // +8: retained parent wrapper; getters borrow its parsed file and chunk root.
};

// Wii native-endian (big-endian), four-byte-aligned resource record, stride 0x88.
// The leading 0x68 bytes have the fixed-function description schema used by
// course_object_and_selection.cpp and Engine_SetLight: type; diffuse/specular/ambient float4;
// position/direction float3; range/falloff/attenuation/cone scalars.
// Extra resource fields are interpreted by GenericLight_GetDescription, which
// scales diffuse by intensity, substitutes range/linear attenuation, and for
// type 2 normalizes target-position and forms outerCone=innerCone+coneWidth.
// Export performs no unit/space/angle conversion: colors/intensity and spatial
// scalars keep the resource's representation. Changing light behavior belongs
// in its producer/converter; changing this copy stride would corrupt the schema.
struct EnvironmentLightDescription {
    unsigned int type;
    float diffuse[4], specular[4], ambient[4];
    float position[3], direction[3];
    float range, falloff, constantAttenuation, linearAttenuation, quadraticAttenuation;
    float innerCone, outerCone;
};
struct EnvironmentLightRecord {
    EnvironmentLightDescription description; // +0x00
    float intensity;                         // +0x68
    unsigned int reserved6C;                 // +0x6C: not read by the conversion
    float range;                             // +0x70
    float innerCone, coneWidth;              // +0x74,+0x78
    float target[3];                         // +0x7C
};
typedef char EnvironmentLightRecord_size[(sizeof(EnvironmentLightRecord) == 0x88) ? 1 : -1];
enum EnvironmentLightChunks {
    kLightCountTag = 0x67696C,
    kLightDataTag = 0x646C
};

#pragma use_lmw_stmw on
// Query 'lig' count metadata, just as GetLightCount does in gtl_environment_light_count.cpp.
// It bounds the separately tagged 'ld' payload traversal below: valid resources
// must supply at least this many 0x88-byte ld elements. The tags need not match.
static inline unsigned int EnvironmentLightCount(const GenericEnvironmentResource* self) {
    void* chunks = Object_GetFieldC(self->resourceBuffer);
    return gtl_ResourceFile_GetChunkElementCount(Object_GetField8(self->resourceBuffer), chunks, kLightCountTag);
}
// Export all lights to caller-owned contiguous EnvironmentLightRecord storage.
// Require live self/parent/parsed file/chunks and dest capacity >= count*0x88;
// there is NO capacity argument (extra arguments in some callers are ignored).
// Destination must not overlap the resource elements (memcpy, not memmove).
// Elements are borrowed only during each copy; no reference count changes.
// The copied records contain scalar data, so the resource may be released once
// copying finishes. Caller frees its destination with its own allocator.
// Returns 1 even for zero lights, leaving dest untouched in that case. Missing
// elements/null payloads and inconsistent counts are not recoverable errors here;
// malformed resources can crash after partial output. There is no rollback.
int gtl_IGenericEnvironmentResource_CopyLights(const GenericEnvironmentResource* self, void* dest) {
    unsigned int count;
    void* element;
    unsigned int i;
    count = EnvironmentLightCount(self);
    for (i = 0; i < count; i++) {
        element = gtl_ResourceFile_GetChunkElement(Object_GetField8(self->resourceBuffer),
                                                   Object_GetFieldC(self->resourceBuffer), kLightDataTag, i);
        memcpy(dest, element, sizeof(EnvironmentLightRecord));
        dest = (char*)dest + sizeof(EnvironmentLightRecord);
    }
    return 1;
}
#pragma use_lmw_stmw reset

} // extern "C"
