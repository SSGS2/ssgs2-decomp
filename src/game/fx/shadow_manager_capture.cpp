// Translation unit 0x801F05C4-0x801F1A5C (22 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"
#include "classes/GfxObjList.h"
#include "classes/CShadowManager.h"
#include "classes/WPuppet.h"
#include "classes/ShadowSelection801F.h"
#include "classes/GeneTexBufferLayout.h"
// Views of the shadow capture queue and named groups; byte-derived addresses retain MWCC scheduling.
struct ShadowCaptureVertex { float position[3]; unsigned char r, g, b, a; };
struct ShadowCaptureQueue {
    void* savedRenderTarget;
    unsigned int textureCount;
    void* targetSentinelPrevious;
    void* firstTarget;
    void* currentTarget;
    unsigned int captureViewport[6], savedViewport[6], captureScissor[4], savedScissor[4];
    ShadowCaptureVertex quad[4];
};
// Group and target nodes share this previous/next/value prefix. A capture pass
// requires one target per group in the same list order, including disabled groups.
// It terminates at the group sentinel and does not independently bounds-check targets.
struct ShadowListNode { void* previous; void* next; void* value; };
// Cached caster owns two pointer-vector allocations; saved resources carry acquired references.
struct ShadowCachedCasterRecord {
    void* puppet;
    void** casterModels; unsigned int casterCount, casterCapacity;
    void** savedResources; unsigned int savedResourceCount, savedResourceCapacity;
};
typedef char ShadowCachedCasterRecord_saved_offset[(int)&((ShadowCachedCasterRecord*)0)->savedResources == 0x10 ? 1 : -1];
// Unlink a node under the caller's list ownership; payload destruction and count updates are separate.
static inline void UnlinkShadowListNode(void* node) {
    ((ShadowListNode*)((ShadowListNode*)node)->previous)->next = ((ShadowListNode*)node)->next;
    ((ShadowListNode*)((ShadowListNode*)node)->next)->previous = ((ShadowListNode*)node)->previous;
}
struct ShadowNamedGroup {
    unsigned int count;
    void* previous; void* next;
    char name[0x40];
    char reserved4C[0x40];
    unsigned int anchor[3];
    char reserved98[0x34];
    unsigned int lightDirection[3];
    char reservedD8[0x1C];
    float projectionScale;
    unsigned int viewMatrix[16], projectionMatrix[16];
    unsigned int captureTexture, flags;
};
struct ShadowExceptionStrings { char reserved[0xD], vectorLengthError[20], lengthErrorType[55]; };
struct ShadowCameraRenderState {
    char reserved[0x20];
    unsigned int viewMatrix[16], projectionMatrix[16];
};
#define SHADOW_OFFSET(type, field) ((int)&((type*)0)->field)
typedef char ShadowCaptureQueue_quad_offset[(SHADOW_OFFSET(ShadowCaptureQueue, quad) == 0x64) ? 1 : -1];
typedef char ShadowNamedGroup_flags_offset[(SHADOW_OFFSET(ShadowNamedGroup, flags) == 0x17C) ? 1 : -1];
enum { kShadowProjectionDisabled = 0x20, kShadowResourceBufferKind = 3,
       kShadowLights = 8, kShadowCaptureTextureSize = 256,
       kShadowQuadPrimitive = 0x98, kShadowQuadVertexFormat = 0xF005,
       kShadowResourceVectorMaxElements = 0x3FFFFFFF };
extern "C" {

struct ShadowDrawArgument { char reserved[0x2C]; int drawArgument; };

// Set the integer draw argument forwarded to projected-shadow rendering.
void ShadowManager_SetProjectionDrawArgument(ShadowDrawArgument* self, int value) {
	self->drawArgument = value;
}

extern unsigned char s_shadowResourceLookupEnabled;
extern unsigned char kShadowCaptureAlphaByPresetAndState[];

// Toggle the shared shadow-model resource lookup.
void ShadowManager_SetResourceLookupEnabled(void* unused, unsigned char value) {
	s_shadowResourceLookupEnabled = value;
}

// Select an alpha from the three-state table and apply it to all capture-quad vertices.
void ShadowManager_SetCaptureQuadAlphaPreset(void* self, int alphaRow, int alphaState) {
	int offset = alphaRow * 4 - alphaRow;
	unsigned char alpha = 0xFF;
	unsigned char* entry = kShadowCaptureAlphaByPresetAndState + offset;
	switch (alphaState) {
	case 0: alpha = entry[0]; break;
	case 1: alpha = entry[1]; break;
	case 2: alpha = entry[2]; break;
	}
	ShadowCaptureQueue* captureQueue = (ShadowCaptureQueue*)((CShadowManager*)self)->captureQueue;
	if (captureQueue != 0) {
		captureQueue->quad[0].a = alpha;
		captureQueue->quad[1].a = alpha;
		captureQueue->quad[2].a = alpha;
		captureQueue->quad[3].a = alpha;
	}
}





extern void appFreeUni__FPv(void*);
extern void __dl__FPv(void*);
extern void Engine_SetRenderStateList(void*, int);
extern void Camera_SetClip(void*, float, float, int);
extern void Camera_Apply(void*, float, float);
extern void* Camera_GetRenderState(void*);
extern void SetTransform(int, void*);
extern void gtl_GenericRenderer_SetProjectionMatrix(void*);
extern void gtl_GenericRenderer_SetViewMatrix(void*);
extern void ShadowModel_DrawWithProjectedTextureStages(void*, void*, void*, void*);
extern unsigned int kProjectedShadowCameraRenderStates[];
extern unsigned int kProjectedShadowProjectionRenderStates[];
extern const float kFloat_3_Shadow;
extern const float kFloat_100000_Shadow;
extern const float kFloat_0_Shadow;
extern const float kFloat_1_TextureRender;
extern int WPuppet_GetModelBufferCount(void*, int);
extern void WPuppet_GetModelResource(void*, int, int, void*);
extern void WPuppet_SetModelResource(void*, int, int, void*);
extern void gtl_GenericRenderer_SetCullingMode(int);
extern void CubeResourceStorage_dtor(void*, int);
extern void* WPuppet_GetChildList(void*);
extern void ShadowModelPointerVector_ClearCountVolatile(void*);
extern int Object_GetFirstWord(void*);
extern void ShadowModelPointerVector_Reserve(void*, unsigned int);
extern void* g_shadowResourceList;
extern void* ShadowModel_FindOrLoadCachedResource(void*, void*, int);
extern void ShadowModelPointerVector_Append(void*, void*);
extern void WPuppetChildIterator_EmptyConstruct(void*);
extern unsigned int WPuppetChildList_GetBeginNode(const unsigned int*);
extern void WPuppetChildIterator_Assign(unsigned int*, const unsigned int*);
extern void* WPuppetChildIterator_GetValuePointer(void**);
extern void WPuppetChildIterator_Advance(unsigned int**);
extern void* WPuppetChildList_GetEndNode(unsigned char*);
extern int WPuppetChildIterator_NotEqual(const unsigned int*, const unsigned int*);
extern void ShadowModelPointerVector_Resize(void*, int);

// Find a named render group in the sentinel list.
void* GfxObjList_FindByName(void* self, void* name) {
    ShadowListNode* node = (ShadowListNode*)AsGfxObjList(self)->firstGroup;
    ShadowListNode* sentinel = (ShadowListNode*)&AsGfxObjList(self)->groupSentinel;
    while (node != sentinel) {
        if (strcmp(((ShadowNamedGroup*)node->value)->name, (char*)name) == 0)
            return node->value;
        node = (ShadowListNode*)node->next;
    }
    return 0;
}

#include "MSL/vector.h"
// Vector destroyed by __dt__11GolfCupListFv; element type not known yet (Elem_dt__11GolfCupListFv is a tag);
// its size reset loads the size twice (form 2).
struct Elem_dt__11GolfCupListFv;
// Pointer elements have no payload destructor; vector storage cleanup owns the allocation.
namespace std { template <> struct __vec_destroys_elements<Elem_dt__11GolfCupListFv*> { enum { value = 2 }; static void destroy(Elem_dt__11GolfCupListFv**) {} }; }
typedef std::vector<Elem_dt__11GolfCupListFv*, app::app_allocator<Elem_dt__11GolfCupListFv*> > Elem_dt__11GolfCupListFvVector;
#include "MSL/list.h"
#include <decomp/cw_bitfields.h>
typedef std::vector<void*, app::app_allocator<void*> > ShadowPtrVector;
typedef std::list<void*, app::app_allocator<void*> > ShadowNodeList;
struct RemoveRef { CW_BITS2( unsigned int refs : 16, unsigned int rest : 16) };
// Drop a model-resource reference and destroy it when the final reference is gone.
inline void RemoveRelease(RemoveRef* p) {
	p->refs--;
	if (!p->refs) CubeResourceStorage_dtor(p, 1);
}
// Clear a pointer vector's count and free backing storage; payloads are handled by caller.
inline void RemoveArray(unsigned int* a) {
	((ShadowPtrVector*)a)->~ShadowPtrVector();
}
// Unlink and free list nodes, decrementing count; payload resources are handled by caller.
inline void RemoveList(unsigned int* object) {
	((ShadowNodeList*)object)->clear();
}
#pragma push
#pragma use_lmw_stmw on
// Delete a named group and its model resources, then remove the FIRST capture target.
// The target removal does not search for the matched group; pairing policy is unresolved.
void GfxObjList_RemoveGroupByName(void* self, void* name) {
	void* namedGroup;
	unsigned int* casterWords;
	unsigned int* groupNode = AsGfxObjList(self)->firstGroup;
	unsigned char* sentinel = (unsigned char*)&AsGfxObjList(self)->groupSentinel;
	// Phase 1: find the group and release its caster payloads before freeing their nodes.
	while ((unsigned char*)groupNode != sentinel) {
		if (strcmp((char*)((ShadowListNode*)groupNode)->value + SHADOW_OFFSET(ShadowNamedGroup, name), (char*)name) == 0) {
			namedGroup = ((ShadowListNode*)groupNode)->value;
			if (namedGroup) {
			if (namedGroup) {
				Memory_Clear((unsigned char*)namedGroup + SHADOW_OFFSET(ShadowNamedGroup, name), 0x40);
				unsigned int* casterSentinel = (unsigned int*)((unsigned char*)namedGroup + 4);
				sentinel = 0;
				for (unsigned int* casterNode = *(unsigned int**)((unsigned char*)namedGroup + 8); casterNode != casterSentinel; casterNode = (unsigned int*)((ShadowListNode*)casterNode)->next) {
					casterWords = (unsigned int*)((ShadowListNode*)casterNode)->value;
					if (casterWords) {
					if (casterWords) {
						*(unsigned int*)casterWords = (unsigned int)sentinel;
					if (((ShadowCachedCasterRecord*)casterWords)->savedResourceCount) {
						for (RemoveRef** resourceSlot = (RemoveRef**)((ShadowCachedCasterRecord*)casterWords)->savedResources; resourceSlot != (RemoveRef**)((ShadowCachedCasterRecord*)casterWords)->savedResources + ((ShadowCachedCasterRecord*)casterWords)->savedResourceCount; ++resourceSlot)
							if (*resourceSlot) { RemoveRelease(*resourceSlot); *resourceSlot = (RemoveRef*)sentinel; }
					}
					RemoveArray(casterWords + 4);
					RemoveArray(casterWords + 1);
					__dl__FPv(casterWords);
					}
					((ShadowListNode*)casterNode)->value = sentinel;
					}
				}
				((ShadowNodeList*)namedGroup)->clear();
				((ShadowNodeList*)namedGroup)->~ShadowNodeList();
				__dl__FPv(namedGroup);
			}
			((ShadowListNode*)groupNode)->value = 0;
			}
			// Phase 2: remove the matched group node, then remove the first target independently.
			UnlinkShadowListNode(groupNode);
			appFreeUni__FPv(groupNode);
			--AsGfxObjList(self)->groupCount; // GfxObjList_RemoveGroupByName: +0x1C
			unsigned int* container = (unsigned int*)AsGfxObjList(self)->renderResources;
			if (((ShadowCaptureQueue*)container)->textureCount != 0) {
				unsigned int* firstTargetNode = (unsigned int*)((ShadowCaptureQueue*)container)->firstTarget;
				RemoveRelease((RemoveRef*)((ShadowListNode*)firstTargetNode)->value);
				UnlinkShadowListNode(firstTargetNode);
				appFreeUni__FPv(firstTargetNode);
				--((ShadowCaptureQueue*)container)->textureCount;
			}
			return;
		}
		groupNode = (unsigned int*)((ShadowListNode*)groupNode)->next;
	}
}

#pragma pop
// Find a shadow group and set or clear the flag that suppresses projected capture.
void* ShadowModel_SetNamedProjectionEnabled(void* self, void* name, int clearFlag) {
    unsigned int* node = AsGfxObjList(self)->firstGroup;
    unsigned char* sentinel = (unsigned char*)&AsGfxObjList(self)->groupSentinel;
    void* object;
    while ((unsigned char*)node != sentinel || (object = 0, false)) {
        if (strcmp((char*)*(void**)((unsigned char*)node + 8) + SHADOW_OFFSET(ShadowNamedGroup, name), (char*)name) == 0) {
            object = *(void**)((unsigned char*)node + 8);
            break;
        }
        node = *(unsigned int**)((unsigned char*)node + 4);
    }
    if (object) {
        if (clearFlag)
            *(unsigned int*)((unsigned char*)object + SHADOW_OFFSET(ShadowNamedGroup, flags)) &= ~kShadowProjectionDisabled;
        else
            *(unsigned int*)((unsigned char*)object + SHADOW_OFFSET(ShadowNamedGroup, flags)) |= kShadowProjectionDisabled;
    }
    return object;
}

// Free the cup pointer-vector allocation and optionally its container.
void* __dt__11GolfCupListFv(void* self, int shouldDelete) {
    return ::std::__vector_deleting_dtor((Elem_dt__11GolfCupListFvVector*)self, shouldDelete);
}

#pragma opt_unroll_loops off
#pragma opt_common_subs off
// Apply camera matrices and render states, then draw the selected projected-shadow model.
void ShadowManager_DrawProjectedModel(void* self, void* camera) {
	unsigned int cameraRenderStates[32];
	unsigned int projectionRenderStates[28];
	int cameraStatePairs = 15;
	unsigned int* stateSource = kProjectedShadowCameraRenderStates - 1;
	unsigned int* stateDestination = cameraRenderStates - 1;
	do {
		*++stateDestination = *++stateSource;
		*++stateDestination = *++stateSource;
	} while (--cameraStatePairs);
	Engine_SetRenderStateList(cameraRenderStates, 15);
	void* selection;
	int drawArgument;
	drawArgument = *(int*)((unsigned char*)self + SHADOW_OFFSET(CShadowManager, projectionDrawArgument));
	selection = *(void**)((unsigned char*)self + SHADOW_OFFSET(CShadowManager, selection));
	Camera_SetClip(camera, kFloat_3_Shadow, kFloat_100000_Shadow, 0);
	Camera_Apply(camera, kFloat_0_Shadow, kFloat_0_Shadow);
	void* view = Camera_GetRenderState(camera);
	SetTransform(1, (unsigned char*)view + SHADOW_OFFSET(ShadowCameraRenderState, projectionMatrix));
	gtl_GenericRenderer_SetProjectionMatrix((unsigned char*)view + SHADOW_OFFSET(ShadowCameraRenderState, projectionMatrix));
	SetTransform(0, (unsigned char*)view + SHADOW_OFFSET(ShadowCameraRenderState, viewMatrix));
	gtl_GenericRenderer_SetViewMatrix((unsigned char*)view + SHADOW_OFFSET(ShadowCameraRenderState, viewMatrix));
	unsigned int* projectionStateSource = kProjectedShadowProjectionRenderStates - 1;
	unsigned int* projectionStateDestination = projectionRenderStates - 1;
	int projectionStatePairs = 14;
	do {
		*++projectionStateDestination = *++projectionStateSource;
		*++projectionStateDestination = *++projectionStateSource;
	} while (--projectionStatePairs);
	Engine_SetRenderStateList(projectionRenderStates, 14);
	void* object = *(void**)selection;
	void* selectedEntries = *(int*)((unsigned char*)selection + SHADOW_OFFSET(ShadowSelection801F, flags)) ? (unsigned char*)self + SHADOW_OFFSET(CShadowManager, entryCount) : 0;
	ShadowModel_DrawWithProjectedTextureStages(object, camera, selectedEntries, (void*)drawArgument);
}
#pragma opt_common_subs reset
#pragma opt_unroll_loops reset

struct ShadowResourceRef;
struct ShadowTemporaryResources { char reserved[0x10]; ShadowResourceRef** items; };
struct ShadowPuppetDrawInterface {
	virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
	virtual void v4(); virtual void v5(); virtual void v6(); virtual void v7();
	virtual void Draw(void*, int, int, int, int, float, int, int);
};
struct ShadowResourceRef { CW_BITS2( unsigned int refs : 16, unsigned int rest : 16) };
// Release a borrowed-and-retained temporary model resource reference.
inline void ReleaseShadowResource(ShadowResourceRef* p) {
	p->refs--;
	if (p->refs == 0) CubeResourceStorage_dtor(p, 1);
}
// Acquire a model resource into slot and detach it from the device for this pass.
inline void SaveAndUnbindShadowResource(void* device, int i, ShadowResourceRef** slot) {
	WPuppet_GetModelResource(device, kShadowResourceBufferKind, i, slot);
	WPuppet_SetModelResource(device, kShadowResourceBufferKind, i, 0);
}
// Release and clear one saved model-resource slot after restoration.
inline void ClearSavedShadowResource(ShadowTemporaryResources* self, int i) {
	if (self->items[i]) {
		ReleaseShadowResource(self->items[i]);
		self->items[i] = 0;
	}
}
#pragma opt_strength_reduction off
// Temporarily unbind puppet model resources for the shadow draw, then restore and release them.
void ShadowModel_DrawWithTemporaryResources(ShadowTemporaryResources* self, void* device, void* arg, int skinMatrices) {
	int i;
	int n;
	n = WPuppet_GetModelBufferCount(device, 3);
	for (i = 0; i < n; ++i) {
		SaveAndUnbindShadowResource(device, i, &self->items[i]);
	}
	gtl_GenericRenderer_SetCullingMode(0);
	((ShadowPuppetDrawInterface*)device)->Draw(arg, 1, 1, 1, 0, kFloat_1_TextureRender, skinMatrices, 0);
	gtl_GenericRenderer_SetCullingMode(1);
	for (i = 0; i < n; ++i) {
		WPuppet_SetModelResource(device, kShadowResourceBufferKind, i, self->items[i]);
		ClearSavedShadowResource(self, i);
	}
}
#pragma opt_strength_reduction reset

extern int WPuppet_GetSkinMatrices(void*);
extern void WPuppet_GetModelCount(void*);
extern void Model_SetMatrixRecursive(void*, int, unsigned int*);
struct ShadowPuppetCasters { void* resource; void** list; void* items; };
struct ShadowMatrixRowWords { unsigned int w[3]; };
struct ShadowAffineMatrixWords { unsigned int w[12]; };
// Copy one 3x4 world matrix using the original redundant word-copy schedule.
// Both pointers borrow 12 words; preserving this schedule keeps MWCC code unchanged.
static inline void CopyShadowWorldMatrixWords(unsigned int* copy, unsigned int* src) {
	*(ShadowMatrixRowWords*)(copy+0) = *(ShadowMatrixRowWords*)(src+0);
	*(ShadowMatrixRowWords*)(copy+3) = *(ShadowMatrixRowWords*)(src+3);
	*(ShadowMatrixRowWords*)(copy+6) = *(ShadowMatrixRowWords*)(src+6);
	*(ShadowMatrixRowWords*)(copy+9) = *(ShadowMatrixRowWords*)(src+9);
	*(ShadowMatrixRowWords*)(copy+0) = *(ShadowMatrixRowWords*)(src+0);
	*(ShadowMatrixRowWords*)(copy+3) = *(ShadowMatrixRowWords*)(src+3);
	*(ShadowMatrixRowWords*)(copy+6) = *(ShadowMatrixRowWords*)(src+6);
	*(ShadowMatrixRowWords*)(copy+9) = *(ShadowMatrixRowWords*)(src+9);
	*(ShadowAffineMatrixWords*)copy = *(ShadowAffineMatrixWords*)src;
}

#pragma push
#pragma use_lmw_stmw on
// Apply the puppet matrix and draw the cached root/child shadow casters.
void ShadowModel_DrawPuppetAndChildCasters(ShadowPuppetCasters* self, void* arg) {
	if (!self->resource) return;
	// Phase 1: reject incomplete caster bindings and snapshot the world transform.
	int skinMatrices = WPuppet_GetSkinMatrices(self->resource);
	if (!skinMatrices) return;
	if (!self->items) return;
	WPuppet_GetModelCount(self->resource);
	// Snapshot the puppet's 3x4 world matrix (12 words) into `worldMatrixWords`. The repeated row/whole-matrix copies below
	// are all the same snapshot; they stay because MWCC emits a different (shorter) sequence for a single worldMatrixWords.
	unsigned int worldMatrixWords[12];
	unsigned int* src = (unsigned int*)((char*)self->resource + SHADOW_OFFSET(WPuppet, worldMatrix));
	CopyShadowWorldMatrixWords(worldMatrixWords, src);
	CopyShadowWorldMatrixWords(worldMatrixWords, src);
	void* node;
	// Phase 2: draw the root; child nodes and cached casters then advance in parallel.
	void** casterModels = self->list;
	if (casterModels[0]) {
		Model_SetMatrixRecursive(casterModels[0], 0, (unsigned int*)worldMatrixWords);
		ShadowModel_DrawWithTemporaryResources((ShadowTemporaryResources*)self, casterModels[0], arg, skinMatrices);
	}
	// Child list = intrusive circular list: header +4 is the sentinel node, +8 the first node; each node is
	// {prev +0, next +4, puppet +8}. list[0] is the root caster, list[1..] pair with the children in order.
	void* obj = WPuppet_GetChildList(self->resource);
	node = *(void**)((char*)obj + 8);
	void* sentinel = (char*)obj + 4;
	casterModels++;
	while (node != sentinel) {
		if (*casterModels) {
			WPuppet_GetModelCount(((ShadowListNode*)node)->value);
			Model_SetMatrixRecursive(*casterModels, 0, (unsigned int*)worldMatrixWords);
			ShadowModel_DrawWithTemporaryResources((ShadowTemporaryResources*)self, *casterModels, arg, skinMatrices);
		}
		node = ((ShadowListNode*)node)->next;
		++casterModels;
	}
}

#pragma pop
struct ShadowPuppetBinding { void* first; unsigned int set[3]; int temporaryResourceVector; };
// Cache root and child shadow models and size temporary resource storage to the largest puppet.
void ShadowModel_BindPuppetAndChildren(ShadowPuppetBinding* self, void* puppet, int selector) {
    self->first = puppet;
    int maxResourceCount = WPuppet_GetModelBufferCount(puppet, kShadowResourceBufferKind);
    void* children = WPuppet_GetChildList(self->first);
    ShadowModelPointerVector_ClearCountVolatile(&self->set[0]);
    int childCount = Object_GetFirstWord(children);
    ShadowModelPointerVector_Reserve(&self->set[0], childCount + 1);
    void* shadowModel = ShadowModel_FindOrLoadCachedResource(g_shadowResourceList, self->first, selector);
    ShadowModelPointerVector_Append(&self->set[0], &shadowModel);
    unsigned int iter;
    WPuppetChildIterator_EmptyConstruct(&iter);
    unsigned int end = WPuppetChildList_GetBeginNode((const unsigned int*)children);
    WPuppetChildIterator_Assign(&iter, &end);
    unsigned int last;
    // 0x801F173C: reload the end iterator after each callback.
    while ((last = (unsigned int)WPuppetChildList_GetEndNode((unsigned char*)children)),
        WPuppetChildIterator_NotEqual(&iter, &last)) {
        void* node = WPuppetChildIterator_GetValuePointer((void**)&iter);
        shadowModel = ShadowModel_FindOrLoadCachedResource(g_shadowResourceList, *(void**)node, selector);
        ShadowModelPointerVector_Append(&self->set[0], &shadowModel);
        node = WPuppetChildIterator_GetValuePointer((void**)&iter);
        int childResourceCount = WPuppet_GetModelBufferCount(*(void**)node, kShadowResourceBufferKind);
        if (maxResourceCount < childResourceCount) maxResourceCount = WPuppet_GetModelBufferCount(*(void**)WPuppetChildIterator_GetValuePointer((void**)&iter), kShadowResourceBufferKind);
        WPuppetChildIterator_Advance((unsigned int**)&iter);
    }
    ShadowModelPointerVector_Resize(&self->temporaryResourceVector, maxResourceCount);
}



// Clear the shadow-model pointer-vector count through its original volatile word access.
void ShadowModelPointerVector_ClearCountVolatile(void* self) {
	volatile int* p = (volatile int*)((char*)self + 4);
	int old = *p;
	union { int a; unsigned int b; } bits;
	bits.a = old;
	*p = old - bits.b;
}

extern void Engine_AcquireCurrentRenderTarget(int, void*);
extern void Engine_GetViewport(void*);
extern void Engine_SetViewport(void*);
extern int g_textureCaptureRenderTarget;
extern void Engine_SetRenderTarget(int, int);
extern void Engine_GetScissorRect(void*);
extern void Engine_SetScissorRect(void*);
extern void Engine_GetLightEnabled(int, void*);
extern void Engine_SetLightEnabled(int, int);
extern void Engine_GetRenderState(int, void*);
struct ShadowRenderStatePairs8 { unsigned int w[16]; };
struct ShadowRenderStatePairs7 { unsigned int w[14]; };
struct ShadowRenderStatePairs9 { unsigned int w[18]; };
struct ShadowVectorWords { unsigned int x, y, z; };
struct ShadowCaptureRenderStateStorage { ShadowRenderStatePairs9 captureQuadDraw; unsigned char pad[0x70]; ShadowRenderStatePairs8 captureSetup; ShadowRenderStatePairs7 casterDraw; };
// State-table roles (shared field spellings retained): a = 8-pair capture setup,
// b = 7-pair caster draw, c = 9-pair capture-quad draw. These are preset copies,
// not snapshots of the caller's render state; individual pair meanings unresolved.
enum ShadowCaptureSlots { kShadowViewTransform = 0, kShadowProjectionTransform = 1,
    kShadowWorldTransform = 11, kShadowPreservedStateId = 1 };
extern ShadowCaptureRenderStateStorage textureCaptureRenderStates;
extern void Vec3_Scale(void*, void*, float);
extern void Vec3_Subtract(void*, void*, void*);
extern void MTXLookAt(void*, void*, void*, void*);
extern void Mtx44_SetOrthoCentered(void*, float, float, float, float);
extern void Engine_SetTextureStageResource(int, int);
extern void Engine_SetFlexibleVertexFormat(int);
extern void Engine_SetType7Resource(int);
extern void DrawPrimitiveUP(int, int, void*, int);
extern void Engine_CopyFramebufferToTexture(int, int, void*, int, int, int, int, int, int, int, int);
extern unsigned char kUnitYAxis[];
extern unsigned char kIdentityMatrix4x4[];
extern void Engine_SetCachedRenderState(int, int);
extern void WView_InvalidateViewAndProjection(void*);
extern void Engine_ClearFramebuffer(int, int, int, float, int, int);
extern void PuppetList_GetRepeatedFirstAnchorPosition(void*, void*, void*);
extern void gtl_GenericRenderer_SetProjectionMatrix(void*);
extern void SetTransform(int, void*);
extern void Engine_SetRenderStateList(void*, int);
extern const float kFloat_256_TextureRender;
extern const float kFloat_8_Shadow;
extern const float kFloat_0_01_Shadow;

// Capture each enabled group's projected shadow while preserving viewport, scissor, lights, and target.
// State contract: save/restore render target 0 (including its acquired reference),
// viewport, scissor, all eight light-enable bits, and cached render-state ID 1.
// View/projection caches are invalidated on exit. Texture stage 0, vertex format,
// world transform and the preset state lists are changed without individual restore;
// framebuffer contents are cleared again after returning to the saved target.
// captureQueue must exist; a non-null saved target means setup is already active.
void ShadowManager_RenderShadowTextures(void* self, void* view) {
    // Phase 1: acquire the caller's target/rectangle state once for this capture.
	ShadowCaptureRenderStateStorage* renderStates = &textureCaptureRenderStates;
	unsigned char* managerBytes = (unsigned char*)self;
	void* captureQueue = *(void**)(managerBytes + SHADOW_OFFSET(CShadowManager, captureQueue));
	if (*(int*)captureQueue == 0) {
		Engine_AcquireCurrentRenderTarget(0, captureQueue);
		Engine_GetViewport(&((ShadowCaptureQueue*)captureQueue)->savedViewport);
		Engine_SetViewport(&((ShadowCaptureQueue*)captureQueue)->captureViewport);
		Engine_SetRenderTarget(0, g_textureCaptureRenderTarget);
		((ShadowCaptureQueue*)captureQueue)->currentTarget = ((ShadowCaptureQueue*)captureQueue)->firstTarget;
		Engine_GetScissorRect(&((ShadowCaptureQueue*)captureQueue)->savedScissor);
		Engine_SetScissorRect(&((ShadowCaptureQueue*)captureQueue)->captureScissor);
	}
	unsigned int savedLights[8];
	for (int i = 0; i < kShadowLights; ++i) {
		Engine_GetLightEnabled(i, &savedLights[i]);
		Engine_SetLightEnabled(i, 0);
	}
	unsigned int savedRenderState;
	Engine_GetRenderState(kShadowPreservedStateId, &savedRenderState);
	ShadowRenderStatePairs8* captureSetupPreset = (ShadowRenderStatePairs8*)((char*)(&renderStates->captureSetup));
	ShadowRenderStatePairs8 captureSetupStates = *captureSetupPreset;
	Engine_SetRenderStateList(&captureSetupStates, 8);
	ShadowRenderStatePairs7* casterDrawPreset = (ShadowRenderStatePairs7*)((char*)(&renderStates->casterDraw));
	ShadowRenderStatePairs7 casterDrawStates = *casterDrawPreset;
	if (managerBytes[SHADOW_OFFSET(CShadowManager, selectionFlags)]) {
        // Phase 2: advance paired group/target nodes together. Disabled groups
        // still clear and copy a texture, preserving list cardinality and order.
		void* groupNode = *(void**)(managerBytes + SHADOW_OFFSET(CShadowManager, listSentinelPrevious));
		void* targetNode = *(void**)(*(unsigned char**)(managerBytes + SHADOW_OFFSET(CShadowManager, captureQueue)) + 0xC);
		float captureExtent = kFloat_256_TextureRender;
		for (; (void*)(managerBytes + SHADOW_OFFSET(CShadowManager, listSentinelNext)) != groupNode; groupNode = ((ShadowListNode*)groupNode)->next, targetNode = ((ShadowListNode*)targetNode)->next) {
			((ShadowNamedGroup*)((ShadowListNode*)groupNode)->value)->captureTexture = (unsigned int)((ShadowListNode*)targetNode)->value;
			Engine_ClearFramebuffer(0, 0, 3, kFloat_1_TextureRender, -1, 0);
			if (managerBytes[SHADOW_OFFSET(CShadowManager, selectionFlags)]) {
				unsigned char* groupBytes = *(unsigned char**)((unsigned char*)groupNode + 8);
				if (!(((ShadowNamedGroup*)groupBytes)->flags & kShadowProjectionDisabled)) {
					ShadowVectorWords averagePosition;
					ShadowVectorWords eyePosition;
					ShadowVectorWords lookAtPosition;
					PuppetList_GetRepeatedFirstAnchorPosition(&averagePosition, groupBytes, groupBytes + SHADOW_OFFSET(ShadowNamedGroup, anchor));
					lookAtPosition = averagePosition;
                    // Light-space view: eye = average caster anchor - 8*lightDirection,
                    // look at that anchor using world +Y as up. Projection width/height
                    // are 256*group scale with near/far 0.01/100000 in scene units.
					Vec3_Scale(&eyePosition, groupBytes + SHADOW_OFFSET(ShadowNamedGroup, lightDirection), kFloat_8_Shadow);
					Vec3_Subtract(&eyePosition, &lookAtPosition, &eyePosition);
					MTXLookAt(groupBytes + SHADOW_OFFSET(ShadowNamedGroup, viewMatrix), &eyePosition, &lookAtPosition, kUnitYAxis);
					float projectionExtent = captureExtent * ((ShadowNamedGroup*)groupBytes)->projectionScale;
					Mtx44_SetOrthoCentered(groupBytes + SHADOW_OFFSET(ShadowNamedGroup, projectionMatrix), projectionExtent, projectionExtent, kFloat_0_01_Shadow, kFloat_100000_Shadow);
					Engine_SetRenderStateList(&casterDrawStates, 7);
					groupBytes = *(unsigned char**)((unsigned char*)groupNode + 8);
					SetTransform(kShadowViewTransform, groupBytes + SHADOW_OFFSET(ShadowNamedGroup, viewMatrix));
					gtl_GenericRenderer_SetViewMatrix(groupBytes + SHADOW_OFFSET(ShadowNamedGroup, viewMatrix));
					SetTransform(kShadowProjectionTransform, groupBytes + SHADOW_OFFSET(ShadowNamedGroup, projectionMatrix));
					gtl_GenericRenderer_SetProjectionMatrix(groupBytes + SHADOW_OFFSET(ShadowNamedGroup, projectionMatrix));
					unsigned char* sentinel = groupBytes + 4;
					for (unsigned char* n = *(unsigned char**)(groupBytes + 8); n != sentinel; n = *(unsigned char**)(n + 4)) ShadowModel_DrawPuppetAndChildCasters((ShadowPuppetCasters*)*(void**)(n + 8), view);
				}
			}
			unsigned char* captureBytes = *(unsigned char**)(managerBytes + SHADOW_OFFSET(CShadowManager, captureQueue));
            // Phase 3: draw the capture quad, copy the framebuffer into this
            // group's target, then advance the queue cursor exactly once.
			Engine_SetTextureStageResource(0, 0);
			Engine_SetFlexibleVertexFormat(kShadowQuadVertexFormat);
			Engine_SetType7Resource(0);
			SetTransform(kShadowWorldTransform, kIdentityMatrix4x4);
			ShadowRenderStatePairs9 captureQuadStates = renderStates->captureQuadDraw;
			Engine_SetRenderStateList(&captureQuadStates, 9);
			DrawPrimitiveUP(kShadowQuadPrimitive, 2, captureBytes + SHADOW_OFFSET(ShadowCaptureQueue, quad), sizeof(ShadowCaptureVertex));
// The callee reads eleven integer arguments; the original call also loads an ignored f1.
			((void (*)(int, int, float, void*, int, int, int, int, int, int, int, int))Engine_CopyFramebufferToTexture)(kShadowCaptureTextureSize, 0, kFloat_0_Shadow, *(void**)(*(unsigned char**)(captureBytes + SHADOW_OFFSET(ShadowCaptureQueue, currentTarget)) + 8), 0, 0, 0, 0, 0, 0, 0, 0);
			*(void**)(captureBytes + SHADOW_OFFSET(ShadowCaptureQueue, currentTarget)) = *(void**)(*(unsigned char**)(captureBytes + SHADOW_OFFSET(ShadowCaptureQueue, currentTarget)) + 4);
		}
	}
    // Phase 4: restore acquired state and release the saved target's high-half refcount.
	Engine_SetViewport(&((ShadowCaptureQueue*)captureQueue)->savedViewport);
	Engine_SetRenderTarget(0, *(int*)captureQueue);
	if (*(void**)captureQueue) {
		unsigned int* targetRef = (unsigned int*)*(void**)captureQueue;
		unsigned int packedReferenceWord = *targetRef;
		unsigned int remainingRefs = (packedReferenceWord >> 16) - 1;
		*targetRef = (remainingRefs << 16) | (packedReferenceWord & 0xFFFF);
		if (remainingRefs == 0) {
			CubeResourceStorage_dtor(targetRef, 1);
		}
		*(void**)captureQueue = 0;
	}
	Engine_SetScissorRect(&((ShadowCaptureQueue*)captureQueue)->savedScissor);
	Engine_ClearFramebuffer(0, 0, 7, 0.0f, 0, 0);
	for (int i = 0; i < kShadowLights; ++i) Engine_SetLightEnabled(i, savedLights[i]);
	Engine_SetCachedRenderState(kShadowPreservedStateId, savedRenderState);
	WView_InvalidateViewAndProjection(view);
}









#pragma push
#pragma use_lmw_stmw on
// Free a word-vector allocation after clearing its logical count; preserve nested guards.
inline void FreeShadowWordArray(unsigned int* array) {
    ((ShadowPtrVector*)array)->~ShadowPtrVector();
}
// Remove a matching puppet from one named group and release its cached resources.
void GfxObjList_RemoveEntryFromNamedGroup(void* self, void* lookupCursor, void* value) {
    // Input: group-name string. After lookup, this ABI argument becomes the matching group's caster-node cursor.
    // Keep its lifetime across both phases to preserve the Wii register allocation.
    unsigned char* sentinel = (unsigned char*)&AsGfxObjList(self)->groupSentinel;
    unsigned int* groupNode = AsGfxObjList(self)->firstGroup;
    void* namedGroup;
    unsigned int* casterWords;
    // Phase 1: locate the named group without changing its capture-target queue.
    while ((unsigned char*)groupNode != sentinel || (namedGroup = 0, false)) {
        if (strcmp((char*)((ShadowListNode*)groupNode)->value + SHADOW_OFFSET(ShadowNamedGroup, name), (char*)lookupCursor) == 0) {
            namedGroup = ((ShadowListNode*)groupNode)->value;
            break;
        }
        groupNode = (unsigned int*)((ShadowListNode*)groupNode)->next;
    }
    if (namedGroup == 0) return;
    // Phase 2: find the puppet entry, release its resources, then unlink only that entry.
    lookupCursor = *(void**)((unsigned char*)namedGroup + 8);
    unsigned int* casterSentinel = (unsigned int*)((unsigned char*)namedGroup + 4);
    while ((unsigned char*)lookupCursor != (unsigned char*)casterSentinel) {
        casterWords = *(unsigned int**)((unsigned char*)lookupCursor + 8);
        if (*(void**)casterWords == value) {
            if (casterWords) {
                if (casterWords) {
                    casterWords[0] = 0;
                    if (casterWords[5]) {
                        ShadowResourceRef** resourceSlot = (ShadowResourceRef**)casterWords[4];
                        for (; resourceSlot != (ShadowResourceRef**)casterWords[4] + casterWords[5]; ++resourceSlot) {
                            if (*resourceSlot) { ReleaseShadowResource(*resourceSlot); *resourceSlot = 0; }
                        }
                    }
                    FreeShadowWordArray(casterWords + 4);
                    FreeShadowWordArray(casterWords + 1);
                    __dl__FPv(casterWords);
                }
                *(void**)((unsigned char*)lookupCursor + 8) = 0;
            }
            UnlinkShadowListNode(lookupCursor);
            appFreeUni__FPv(lookupCursor);
            --*(unsigned int*)namedGroup;
            return;
        }
        lookupCursor = *(void**)((unsigned char*)lookupCursor + 4);
    }
}
#pragma pop

extern void* __nw__FUl(unsigned int);
extern void* __dt__Q23std12length_errorFv(void*, short);
extern void* appMallocDummy__FUl(unsigned int);
extern void* memcpy(void*, const void*, unsigned int);
extern void* memset(void*, int, unsigned int);
extern void* __nwa__FUl(unsigned int);
extern const unsigned char puppetBoneExceptionStrings[];
extern unsigned char __vt__Q23std11logic_error[];
extern unsigned char s_sharedCharDeleterVtable[];
extern unsigned char __vt__Q23std12length_error[];
extern void __dla__FPv(void*);
#pragma push
#pragma use_lmw_stmw on
// ---- std::vector<void*>::reserve scaffolding (inlined library code) ----
// ShadowModelPointerVector_Reserve below is vector.reserve(size): when size exceeds capacity it allocates a
// new array, moves the old pointers over, swaps the arrays and frees the old one. Over the maximum element
// count it throws std::length_error("vector length error"), built inline (message copy + shared control block).
// Plain C: if (size > capacity) { grow to exactly `size` elements, keeping the contents }.
// Holder = temporary-message-buffer guard of that throw (named after the function it was first seen in, 0x801F1898).
struct fn_801F1898_Holder {
    char* p; char* flag;
    // Guard the temporary character allocation and remember its allocator flag.
    fn_801F1898_Holder(char* b, char* f) { p = b; flag = f; }
    // Delete the temporary array unless ownership was transferred away.
    ~fn_801F1898_Holder() { if (p) __dla__FPv(p); }
};
// Temporary replacement array {data, count, capacity}; its destructor frees whichever block it ends up owning.
struct ReserveTemp {
	unsigned int* data; unsigned int count, capacity;
	// Construct an empty allocation guard for pointer-vector growth.
	ReserveTemp() : data(0), count(0), capacity(0) {}
	// Clear the temporary count and free whichever vector allocation the guard owns.
	~ReserveTemp() { if (data) { count -= *(int*)&count; appFreeUni__FPv(data); } }
};
// Exchange vector backing storage while transferring the requested logical count.
inline void ReserveSwap(unsigned int* a, unsigned int* b, int count) {
	void* old = *(void**)a;
	void* p = *(void**)b; *(void**)b = old;
	b[1] += count;
	a[1] = 0;
	unsigned int cap = a[2]; a[2] = b[2]; b[2] = cap;
	*(void**)a = p;
	unsigned int n = a[1]; a[1] = b[1]; b[1] = n;
}
// Grow pointer storage with length checking and transfer ownership of existing resource pointers.
void ShadowModelPointerVector_Reserve(void* self, unsigned int size) {
	ReserveTemp* v = (ReserveTemp*)self;
	if (size <= v->capacity) return;
	ReserveTemp temp;
	if (size > kShadowResourceVectorMaxElements) {
		char done __attribute__((aligned(4))) = 0;
		struct { void* vt; char* buf; void* ref; } err;
		void* error = &err;
		const char* message = (char*)puppetBoneExceptionStrings + SHADOW_OFFSET(ShadowExceptionStrings, vectorLengthError);
		err.vt = __vt__Q23std11logic_error;
		unsigned n = strlen(message) + 1;
		char* b = (char*)__nwa__FUl(n * 2 - n);
		err.buf = b;
		{
			fn_801F1898_Holder h(b, &done);
			void* r = __nw__FUl(16);
			if (r) { int table = (int)s_sharedCharDeleterVtable; ((int*)r)[1] = 1; ((int*)r)[2] = 1; *(int*)r = table; ((char**)r)[3] = b; }
			err.ref = r;
			h.p = 0;
		}
		strcpy(err.buf, message);
		err.vt = __vt__Q23std12length_error;
		__throw((char*)puppetBoneExceptionStrings + SHADOW_OFFSET(ShadowExceptionStrings, lengthErrorType), error, (void*)__dt__Q23std12length_errorFv);
	}
	temp.data = (unsigned int*)appMallocDummy__FUl(size * 4);
	temp.capacity = size;
	int count;
	unsigned int* data = v->data;
	count = (data + v->count) - data;
	unsigned int bytes = count * 4;
	memcpy(temp.data + temp.count, data, bytes);
	memset(data, 0, bytes);
	ReserveSwap((unsigned int*)v, (unsigned int*)&temp, count);
}
#pragma pop

// Empty iterator construction hook; leaves the iterator storage untouched.
void WPuppetChildIterator_EmptyConstruct(void*) {}

// Copy a child-list iterator's node pointer.
void WPuppetChildIterator_Assign(unsigned int* destination, const unsigned int* source) {
	*destination = *source;
}

// Move to the next child-list node.
void WPuppetChildIterator_Advance(unsigned int** value) {
	*value = *(unsigned int**)((unsigned char*)*value + 4);
}

// Return the address of the child pointer held by the current node.
void* WPuppetChildIterator_GetValuePointer(void** value) {
	return (unsigned char*)*value + 8;
}

// Return the child-list begin node.
unsigned int WPuppetChildList_GetBeginNode(const unsigned int* value) {
	return value[2];
}

// Return the address of the embedded child-list sentinel.
void* WPuppetChildList_GetEndNode(unsigned char* value) {
	return (void*)(value + 4);
}

// Compare the node pointers held by two child-list iterators.
int WPuppetChildIterator_NotEqual(const unsigned int* a, const unsigned int* b) {
	return *a != *b;
}

} // extern "C"
