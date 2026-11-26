// Queues a model resource for frustum-culled drawing. The vector block at +0x0C is passed as the first argument.
#include "classes/GtlIGenericModelingResource.h"

extern "C" {

void gtl_Model_DrawWithFrustumCulling(void* a, void* b, void* c, void* d);
}

void gtl::IGenericModelingResource::QueueDraw(void* b, void* c) {
    gtl_Model_DrawWithFrustumCulling(&textures, b, c, this);
}
