// Apply a gtl environment resource to the current renderer (no gameplay knobs).
// API lookup: gtl_GenericEnvironment_Apply in gtl_generic_environment.cpp scans archive chunks and
// sets lights, fog, ambient color, view and projection. Its third argument is
// the number of light slots to visit, including slots to disable when unused.
// s_genericEnvironmentLightSlotCount caches Renderer_InitializeDefaults' +0xA0 value during
// gtl_GenericResource_Initialize (gtl_generic_resource_loading.cpp), rather than an environment ID.

#include "classes/GtlIGenericEnvironmentResource.h"

extern "C" {

extern unsigned int s_genericEnvironmentLightSlotCount;
void gtl_GenericEnvironment_Apply(void* obj, void* overrides, unsigned int* maxLights);

} // extern "C"

// The resource's retained archive (+8) is borrowed by this forwarding call. overrides may be null; otherwise
// it is a borrowed five-pointer record: +0 lights, +4 view matrix, +8 projection matrix, +C fog, +10 ambient.
// Archive/override storage must survive this synchronous call. No ownership is transferred and no status is
// returned. Missing chunks leave their settings untouched, except unused light slots are disabled.
void gtl::IGenericEnvironmentResource::Apply(void* overrides) {
    unsigned int lightSlotCount = s_genericEnvironmentLightSlotCount;
    gtl_GenericEnvironment_Apply(resourceBuffer, overrides, &lightSlotCount);
}
