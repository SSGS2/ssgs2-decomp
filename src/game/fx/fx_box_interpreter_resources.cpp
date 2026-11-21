#include <decomp/static_init.h>
#include <decomp/ct_member.h>
#include "MSL/error_message.h"
// Fx resource plumbing: effect-box updates, resource filename parsing, tree storage
// and interpreter shutdown. No golf-shot rules are defined here.
// Mod/API lookup (keep related settings together):
// - Effect speed/pause/stall cap: CFxBox_UpdateEffects; CFxBox.h +4/+8/+110/+118.
//   CFx_UpdateEffects (cfx_shot_effects.cpp) converts accumulated seconds to milliseconds;
//   CFxBox_Ctor (fx_box_effect_trees.cpp) supplies the nominal 1000/30 ms step and unit scale.
// - Resource syntax: EffectCommand_ParseResourceFilename; token prefix
//   fxConditionCommandTokens and extension table kEffectFilenameExtensions.
//   FxAttachments_CreateFromResourceTags (fx_sequence_scripts_and_attachments.cpp) consumes the result;
//   sequence command execution is in fx_sequence_script_parser.cpp. Search the parser symbol there
//   if changing token modes, extension handling or the 64-byte name limit.
// - Crossing tolerance: CFxBox_TestHeightCrossing / box_near; shared constants
//   kFxBoxPositiveBoundaryTolerance and kFxBoxNegativeBoundaryTolerance.
// - Storage/ownership: *InsertNode and *FreeSubtree below manage tree nodes;
//   CFxBox_UpdateEffects and FxInterpreter_dtor handle live effect removal.
// Layout companions: classes/CFxBox.h, CFxSpray.h, FxInterpreter.h; constants
// remain under their ABI names (definitions searchable in src/game/*_data.cpp).
// Preserve the game exception identities while keeping construction adapters local.
namespace Port2ExceptionRuntime { extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn, nothrow));
void __dt__Q23std12length_errorFv();
void __dt__Q23std12out_of_rangeFv();
} }
#include "classes/FxInterpreter.h"
#include "classes/CFxSpray.h"
#include "classes/CFxBox.h"
// Translation unit 0x8010B244-0x8010C720 (14 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"

extern "C" {

// Default effect-box lifecycle hook; subclasses may override it.
void CFxBox_vfunc_1() {}

// Links occupy +0/+4/+8, then key at +12 and effect at +16. Parent bit zero
// is tree color: mask it before traversing. End is the embedded root/header
// address, not null. Different trees below own nodes, strings or effects separately.
struct RBNode { RBNode* left; RBNode* right; RBNode* parent; unsigned int key; void* effect; };
// Pair trees put first at +8; sprite-cache trees have an extra header word.
struct ListAnchor {
    unsigned int count;
    unsigned int root;
    void* firstPair;
    void* firstSprite;
};
enum {
    kMaximumTreeEntries = 0xFFFFFFFFu,
    kFilenameCapacity = 64,
    kParseSpaceDelimited = 0,
    kParseQuoted = 1,
    kResourceSequence = 1,
    kResourceSpray = 2,
    kResourceUnknown = 0,
    kInterpreterLinksOffset = (int)&((FxInterpreter*)0)->sentinel
};
struct StringLike { void* vtable; int refs; int capacity; char* data; };
struct FixedString { char* vtable; char storage[1]; };
struct Cursor { void* first; void* second; };
extern void* __vt__Q23std11logic_error[4];
extern void* s_sharedCharDeleterVtable[5];
extern void* __vt__Q23std12length_error[4];
extern char hudBitmapVariantStrings[0xF0];
extern void* __nwa__FUl(unsigned int);
extern void* appMallocDummy__FUl(unsigned int);
extern void* __nw__FUl(unsigned int);
extern void __dla__FPv(void*);
extern void RBTree_RebalanceAfterInsert(void*, void*);
extern void __dt__Q23std12length_errorFv(void*);
extern void String_ReserveCapacity(void*, void*);
struct __CopyTag_8010B8EC {};
extern void String_ReplaceRange(void*, int, int, const char*, const char*, __CopyTag_8010B8EC);
extern void appFreeUni__FPv(void*);
extern void __dt__Q33std6detail16char_array_guardFv(void*, int);

#pragma use_lmw_stmw on
#pragma scheduling on
#pragma opt_common_subs off
extern "C" void __dt__Q23std9exceptionFv(void*, int);
namespace std { struct exception { void* vtable; 
    // Initialize the standard logic-error exception identity.
    exception() : vtable((void*)__vt__Q23std11logic_error) {} ~exception(); }; }
// Shared RTTI: std::tr1::detail::shared_ptr_deleter<char, std::detail::default_delete<char[]> >.
// Use the common 16-byte ABI view in MSL/error_message.h, including its member names.
typedef ::std::detail::__message_control __ErrorCount_8010B8EC;
struct __Holder_8010B8EC { void* p; void* flag; 
    // Guard the allocated text until it is owned by its reference block.
    __Holder_8010B8EC(void* b, void* f) : p(b), flag(f) {} 
    // Delete text if ownership has not transferred from the guard.
    ~__Holder_8010B8EC() { if (p) __dla__FPv(p); } };
struct __AllocatorByte_8010B8EC { volatile char byte; char padding[3]; 
    // Initialize the allocator tag used by string and error construction.
    __AllocatorByte_8010B8EC() : byte(0) {} };
namespace std { struct logic_error_u8010B244 : exception { char* buf; __ErrorCount_8010B8EC* ref; 
    // Allocate and share a NUL-terminated copy of diagnostic text.
    logic_error_u8010B244(const char* msg) {
	__AllocatorByte_8010B8EC allocator;
	volatile char done = 0;
	const char* m = msg;
	unsigned n = strlen(m) + 1;
	char* b = (char*)__nwa__FUl(n * 2 - n);
	buf = b;
	{ __Holder_8010B8EC h(b, (void*)&done); __ErrorCount_8010B8EC* r = (__ErrorCount_8010B8EC*)__nw__FUl(16); if (r) { r->strong = 1; r->weak = 1; r->table = (void*)s_sharedCharDeleterVtable; r->pointer = b; } ref = r; h.p = 0; }
	strcpy(buf, m);
} 
    // Release the copied diagnostic through the runtime helper.
    ~logic_error_u8010B244() { __dt__Q33std6detail16char_array_guardFv(this, -1); } }; }
struct HudDiagnosticStrings { char precedingStrings[0x23]; char lengthErrorType[55]; };
struct __LengthError_8010B8EC : std::logic_error_u8010B244 { 
    // Construct and throw the sprite-cache length-error identity.
    __LengthError_8010B8EC(const char* msg) : std::logic_error_u8010B244(msg) { this->vtable = (void*)__vt__Q23std12length_error; __throw(((HudDiagnosticStrings*)hudBitmapVariantStrings)->lengthErrorType, this, (void*)__dt__Q23std12length_errorFv); } };
namespace std { struct length_error : logic_error_u8010B244 {
	// Construct the standard length-error identity from copied text.
	length_error(const char* msg) : logic_error_u8010B244(msg) { vtable = __vt__Q23std12length_error; }
	~length_error();
    DECOMP_CT_MEMBER(length_error)
};
}
#pragma use_lmw_stmw on
#pragma scheduling on
// Construct in caller-supplied storage without allocating.
inline void* operator new(size_t, void* p) { return p; }
struct __NodeGuard_8010B8EC { void* c; void* node; 
    // Free an uncommitted sprite-cache node during unwinding.
    ~__NodeGuard_8010B8EC() { if (node) appFreeUni__FPv(node); } };
struct __ParentValue_8010B8EC {
	RBNode* value;
	// Copy the tree parent link while preserving the construction boundary.
	__ParentValue_8010B8EC(RBNode* v) { try { value = v; } catch (...) { throw; } }
};
struct __ValueCopy_8010B8EC {
	unsigned flags; unsigned length; unsigned data; unsigned a; unsigned b;
	// Copy a cache string and its two associated words with heap text duplication.
	__ValueCopy_8010B8EC(const __ValueCopy_8010B8EC& src) { try {
		if (!(src.flags >> 31)) {
			flags = src.flags;
			unsigned ptr;
			unsigned len;
			len = src.length;
			ptr = src.data;
			length = len; data = ptr;
		} else {
			flags = 0; length = 0; data = 0;
			String_ReserveCapacity(this, (void*)src.length);
			__CopyTag_8010B8EC tag;
			String_ReplaceRange(this, 0, 0, (const char*)src.data, (const char*)src.data + src.length, tag);
		}
		unsigned second;
		unsigned first;
		first = src.a;
		second = src.b;
		a = first; b = second;
	} catch (...) { throw; } }
};
// Copy a sprite-cache value into a new red-black node and update the first iterator.
void* AnimatedSpriteCache_InsertNode(ListAnchor* owner, RBNode* position, int insertBefore, int updateFirst,
                  const Cursor* value) {
	if (owner->count == kMaximumTreeEntries) {
		do { union { double align; unsigned char bytes[sizeof(std::length_error)]; } port2Storage; std::length_error* port2Error = (std::length_error*)port2Storage.bytes; port2Error->__ct((char*)hudBitmapVariantStrings + 9); Port2ExceptionRuntime::__throw("!std::exception!!std::logic_error!!std::length_error!!", port2Error, (void*)Port2ExceptionRuntime::__dt__Q23std12length_errorFv); } while (0);
	}
	void* volatile constructed;
	void* volatile allocated;
	__AllocatorByte_8010B8EC nodeAllocator;
	void* allocation = appMallocDummy__FUl(32);
	allocated = allocation;
	constructed = allocation;
	__NodeGuard_8010B8EC guard;
	guard.c = &owner->root;
	guard.node = allocation;
	new ((char*)allocation + 12) __ValueCopy_8010B8EC(*(const __ValueCopy_8010B8EC*)value);
	RBNode* result = (RBNode*)guard.node;
	result->right = 0;
	guard.node = 0;
	result->left = 0;
	new (&result->parent) __ParentValue_8010B8EC(position);
	if (insertBefore) position->left = result;
	else position->right = result;
	++owner->count;
	RBTree_RebalanceAfterInsert(result, (void*)owner->root);
	if (updateFirst) owner->firstSprite = result;
	return result;
}
#pragma use_lmw_stmw reset
#pragma scheduling reset

#pragma use_lmw_stmw on
#pragma scheduling on
struct __NodeGuard_8010BEF8 { void* c; void* node; 
    // Free an uncommitted pair-tree node during unwinding.
    ~__NodeGuard_8010BEF8() { if (node) appFreeUni__FPv(node); } };
struct __PairCopy_8010BEF8 {
	void* a; void* b;
	// Copy the two pointer words of a non-owning tree payload.
	__PairCopy_8010BEF8(const __PairCopy_8010BEF8& src) { try { a = src.a; b = src.b; } catch (...) { throw; } }
};
// Insert an ID/object pair and rebalance the effect tree.
void* FxBoxTree_InsertNode(ListAnchor* owner, RBNode* position, int insertBefore, int updateFirst,
                  const Cursor* value) {
	if (owner->count == kMaximumTreeEntries) {
		do { union { double align; unsigned char bytes[sizeof(std::length_error)]; } port2Storage; std::length_error* port2Error = (std::length_error*)port2Storage.bytes; port2Error->__ct((char*)hudBitmapVariantStrings + 9); Port2ExceptionRuntime::__throw("!std::exception!!std::logic_error!!std::length_error!!", port2Error, (void*)Port2ExceptionRuntime::__dt__Q23std12length_errorFv); } while (0);
	}
	void* volatile constructed;
	void* volatile allocated;
	__AllocatorByte_8010B8EC nodeAllocator;
	void* allocation = appMallocDummy__FUl(20);
	allocated = allocation;
	constructed = allocation;
	__NodeGuard_8010BEF8 guard;
	guard.c = &owner->root;
	guard.node = allocation;
    new ((char*)allocation + 12) __PairCopy_8010BEF8(*(const __PairCopy_8010BEF8*)value);
	RBNode* result = (RBNode*)allocation;
	result->right = 0;
	guard.node = 0;
	result->left = 0;
	new (&result->parent) __ParentValue_8010B8EC(position);
	if (insertBefore) position->left = result;
	else position->right = result;
	++owner->count;
	RBTree_RebalanceAfterInsert(result, (void*)owner->root);
	if (updateFirst) owner->firstPair = result;
	return result;
}
#pragma use_lmw_stmw reset
#pragma scheduling reset

#pragma opt_common_subs reset
extern float kFloat_6_2831855;
extern float s_twoPi_FxBox;

// Initialize this effect-box unit's mutable two-pi constant.
void FxBox_InitializeUnitConstants() {
	s_twoPi_FxBox = kFloat_6_2831855;
}

void appFreeUni__FPv(void*);
void __dl__FPv(void*);

struct OwnedEffectBuffer { void* vtable; void* allocation; };

// Free the owned buffer and optionally delete its wrapper.
void* dtor_8010C34C(void* self, int deleting) {
	if (self != 0) {
		void* value = ((OwnedEffectBuffer*)self)->allocation;
		if (value != 0)
			appFreeUni__FPv(value);
		if (deleting > 0)
			__dl__FPv(self);
	}
	return self;
}

struct InitObj { unsigned int vtable; unsigned int entryCount; unsigned int next; unsigned int prev; unsigned int state; };
extern InitObj s_fxInterpreter;
extern unsigned char s_fxInterpreterInitialized;
extern const char __vt__13FxInterpreter[];
void* FxInterpreter_dtor(void*, int);
extern char s_fxInterpreterDtorRegistration[];

// Lazily initialize the interpreter list and register its shutdown destructor.
InitObj* FxInterpreter_GetInstance() {
	if ((signed char)s_fxInterpreterInitialized == 0) {
		s_fxInterpreter.vtable = (unsigned int)__vt__13FxInterpreter;
		s_fxInterpreter.entryCount = 0;
		s_fxInterpreter.prev = (unsigned int)((char*)&s_fxInterpreter + kInterpreterLinksOffset);
		s_fxInterpreter.next = (unsigned int)((char*)&s_fxInterpreter + kInterpreterLinksOffset);
		s_fxInterpreter.state = 0;
		__register_global_object(&s_fxInterpreter, (void*)FxInterpreter_dtor, s_fxInterpreterDtorRegistration);
		s_fxInterpreterInitialized = 1;
	}
	return &s_fxInterpreter;
}

void CFxSpray_DrawParticlesAndBounds(void*, void*);
// Reset draw-pass counters and draw particles/bounds for every registered spray.
void CFxBox_DrawSprays(void* self, void* configuration) {
	RBNode* sprayNode = (RBNode*)((CFxBox*)self)->sprayTreeFirst;
	RBNode* endNode = (RBNode*)&((CFxBox*)self)->sprayTreeRoot;
	// Reset both draw-pass counters before visiting sprays; their individual units are unresolved.
	(*(unsigned int*)&((CFxBox*)self)->unk_11C) = 0;
	(*(unsigned int*)&((CFxBox*)self)->unk_120) = 0;
	// Draw in key order with borrowed configuration. Successor is leftmost in
	// the right subtree, otherwise the first ancestor reached from its left.
	while (sprayNode != endNode) {
		CFxSpray_DrawParticlesAndBounds(sprayNode->effect, configuration);
		void* child = sprayNode->right;
		if (child != 0) {
			RBNode* leaf = (RBNode*)child;
			while (leaf->left != 0)
				leaf = leaf->left;
			sprayNode = leaf;
		} else {
			while (sprayNode != ((RBNode*)((unsigned int)sprayNode->parent & ~1u))->left) {
				RBNode* parent = (RBNode*)((unsigned int)sprayNode->parent & ~1u);
				sprayNode = parent;
			}
			sprayNode = (RBNode*)((unsigned int)sprayNode->parent & ~1u);
		}
	}
}

struct CachedResourceName {
	int flags;
	unsigned int length;
	void* value;
};

struct FxBoxTreeNode { FxBoxTreeNode* left; FxBoxTreeNode* right; FxBoxTreeNode* parent; char keyStorage[4]; void* owner; void* payload; };
struct FxBoxTreeHeader { unsigned int count; struct { FxBoxTreeNode* left; FxBoxTreeNode* right; } head; };
// Local view of CFxBox used by the update loop: +0x00 vtable (slot 7 = notify(effect)), then the two
// time scales, a paused flag, a frame-limit flag, and the spray and sequence red-black trees.
struct FxBoxTickView {
	virtual void vt08();
	virtual void vt0C();
	virtual void vt10();
	virtual void vt14();
	virtual void vt18();
	virtual void vt1C();
	virtual void notify(void* owner);
	float nominalStepMilliseconds;
	float timeScale;
	char boxPrefixStorage[0x104];
	unsigned char paused;
	char pausePadding[7];
	unsigned char clampSimulationStep;
	char boxStateStorage[0x3B];
	FxBoxTreeHeader sprays;
	FxBoxTreeHeader sequences;
};
struct FxBoxEffectOwner { 
virtual void dispose(int);
};
extern float kFloat_1_5_FxBox;
int CFxSpray_Update(void*, float);
int CFxSequence_UpdateTimeline(void*, float, unsigned char*);
void CFxBox_EraseSequenceAtIterator(void*, void*, int);
void RBTree_RebalanceAfterErase(void*, void*);
void CFxSequence_UnregisterSpray(void*, void*);

// Advance in key order, masking the red-black color bit from parent links.
static void rbtree_advance(FxBoxTreeNode*& node) {
	if (node->right) {
		FxBoxTreeNode* next = node->right;
		while (next->left) next = next->left;
		node = next;
		return;
	}
	FxBoxTreeNode* parent;
	while (node != (parent = (FxBoxTreeNode*)((unsigned int)node->parent & ~1u))->left) {
		node = parent;
	}
	node = parent;
}

#pragma inline_depth(3)
// Advance sprays and sequences, release finished entries, and report sequence termination.
// self is required and exclusively updated by the effect owner. delta is elapsed
// milliseconds, timeScale is dimensionless (1 = normal speed). No validation of
// negative/NaN deltas or scales is added here. Pause returns 1 before any work.
// With clampSimulationStep set, cap delta BEFORE scaling at
// 1.5*nominalStepMilliseconds (normally ~50 ms); discard the excess, never catch up.
// This controls simulation, not display/retrace frequency. Increasing the nominal
// step raises the allowed stall step; increasing timeScale speeds both consumers.
// Keep the two scale multiplications at their call sites and preserve float order.
// Sprays finish when Update returns 0: unlink node, notify owner, detach sequence,
// delete spray. Sequences are erased on a 0 update result; their separate expired
// byte returns 0 from this pass immediately, leaving later entries for next call.
// Otherwise return 1, including an empty or paused box. Updates can remove entries;
// advance before erasing a spray and let EraseSequenceAtIterator advance sequences.
int CFxBox_UpdateEffects(FxBoxTickView* self, float delta) {
	if (self->paused) return 1;
	if (self->clampSimulationStep && delta > kFloat_1_5_FxBox * self->nominalStepMilliseconds) delta = kFloat_1_5_FxBox * self->nominalStepMilliseconds;

	FxBoxTreeNode* node = self->sprays.head.right;
	while ((FxBoxTreeNode*)&self->sprays.head != node) {
		if (!node->owner) {
			FxBoxTreeNode* erased = node;
			rbtree_advance(node);
			if (erased == self->sprays.head.right) {
				self->sprays.head.right = node;
			}
			RBTree_RebalanceAfterErase(erased, self->sprays.head.left);
			appFreeUni__FPv(erased);
			--self->sprays.count;
		} else if (!CFxSpray_Update(node->owner, delta * self->timeScale)) {
			// Save the finished spray before advancing/unlinking its tree node.
			void* owner = node->owner;
			FxBoxTreeNode* erased = node;
			rbtree_advance(node);
			if (erased == self->sprays.head.right) {
				self->sprays.head.right = node;
			}
			RBTree_RebalanceAfterErase(erased, self->sprays.head.left);
			appFreeUni__FPv(erased);
			self->sprays.count -= 1;
			self->notify(owner);
			// CFx_RemoveSpray: original parent sequence pointer +0x88.
			if (*(void**)&((CFxSpray*)owner)->parentSequence) CFxSequence_UnregisterSpray(*(void**)&((CFxSpray*)owner)->parentSequence, owner);
			if (owner) ((FxBoxEffectOwner*)owner)->dispose(1);
		} else {
			rbtree_advance(node);
		}
	}
	FxBoxTreeNode* node2 = self->sequences.head.right;
	while ((FxBoxTreeNode*)&self->sequences.head != node2) {
		if (!node2->owner) {
			FxBoxTreeNode* erased = node2;
			rbtree_advance(node2);
			if (erased == self->sequences.head.right) {
				self->sequences.head.right = node2;
			}
			RBTree_RebalanceAfterErase(erased, self->sequences.head.left);
			appFreeUni__FPv(erased);
			self->sequences.count -= 1;
		} else {
			unsigned char expired;
			if (!CFxSequence_UpdateTimeline(node2->owner, delta * self->timeScale, &expired)) {
				CFxBox_EraseSequenceAtIterator(self, (void**)&node2, 0);
			} else {
				rbtree_advance(node2);
			}
			if (expired) return 0;
		}
	}
	return 1;
}
#pragma inline_depth reset

// Free a resource-name buffer only when its narrow string uses heap storage.
void FxBoxResourceMap_DestroyStringStorage(void*, CachedResourceName* arg) {
	// The null test appears twice only because two inlined copies were emitted by the original compiler.
	if (arg == 0)
		return;
	if (arg == 0)
		return;
	if (((unsigned int)arg->flags >> 31) != 0)
		appFreeUni__FPv(arg->value);
}

struct InterpreterListNode { InterpreterListNode* prev; InterpreterListNode* next; };
extern unsigned int g_fx;
void CFx_RemoveSpray(void*, void*);
void CFx_RemoveSequence(void*, void*, int);
#pragma opt_common_subs off
#pragma scheduling on
// Unregister interpreter-owned effects from the live manager and free all list nodes.
void* FxInterpreter_dtor(void* self, int deleting) {
	if (self) {
	unsigned int* list = (unsigned int*)self;
	(*(unsigned int*)&((FxInterpreter*)list)->vtbl) = (unsigned int)__vt__13FxInterpreter;
	// Phase 1: unregister payloads through g_fx while preserving links. Manager
	// removal handles effects; this loop does not delete their pointers directly.
	// If g_fx is absent, only the list nodes are subsequently freed.
	if (((FxInterpreter*)list)->entryCount != 0) {
		InterpreterListNode* sentinel = (InterpreterListNode*)&((FxInterpreter*)list)->sentinel;
		InterpreterListNode* node = sentinel->next;
		while (node != sentinel) {
			if (g_fx != 0) {
				int type = ((FxInterpreterEntry*)node)->kind;
				if (type == kResourceSpray) CFx_RemoveSpray((void*)g_fx, ((FxInterpreterEntry*)node)->value);
				else if (type == kResourceSequence) CFx_RemoveSequence((void*)g_fx, ((FxInterpreterEntry*)node)->value, 1);
			}
			node = node->next;
		}
		// Phase 2: detach the range, then free nodes and decrement entryCount.
		InterpreterListNode* begin;
		InterpreterListNode* current;
		current = ((InterpreterListNode*)&((FxInterpreter*)list)->sentinel)->next;
		begin = (InterpreterListNode*)&((FxInterpreter*)list)->sentinel;
		if (current != begin) {
			InterpreterListNode* last = begin->prev;
			current->prev->next = last->next;
			last->next->prev = current->prev;
			while (current != begin) {
				InterpreterListNode* nxt = current->next;
				InterpreterListNode* dead = current;
				current = nxt;
				appFreeUni__FPv(dead);
				--((FxInterpreter*)list)->entryCount;
			}
		}
	}
	// The four identical address tests below are an always-true guard (the member address is never null),
	// duplicated by the original inlining; ignore them.
	// Phase 3: container destruction repeats node-only cleanup if any remain.
	// The embedded sentinel is never freed separately.
	if (&((FxInterpreter*)list)->entryCount && &((FxInterpreter*)list)->entryCount && &((FxInterpreter*)list)->entryCount && &((FxInterpreter*)list)->entryCount) {
		InterpreterListNode* sentinel = (InterpreterListNode*)&((FxInterpreter*)list)->sentinel;
		InterpreterListNode* begin = sentinel;
		InterpreterListNode* current = sentinel->next;
		if (current != begin) {
			InterpreterListNode* last = begin->prev;
			current->prev->next = last->next;
			last->next->prev = current->prev;
			while (current != begin) {
				InterpreterListNode* nxt = current->next;
				InterpreterListNode* dead = current;
				current = nxt;
				appFreeUni__FPv(dead);
				--list[1];
			}
		}
	}
	if (deleting > 0) __dl__FPv(self);
	}
	return self;
}
#pragma scheduling reset
#pragma opt_common_subs reset

#pragma use_lmw_stmw on
#pragma scheduling on
// Recursively free sequence-tree nodes without destroying their referenced effects.
// Postorder cleanup has its first levels expanded inline: recurse left/right,
// free node, retain referenced effect. No payload destructor is called.
void FxSequenceTree_FreeSubtree(void* self, void** arg) {
    // In plain C this is just: if (n->left) free(n->left); if (n->right) free(n->right); free(n);
    // applied recursively (node[0] = left, node[1] = right). The four nested levels below are the
    // compiler-unrolled form of that recursion; skip to the end of the function.
    // Phases: free left subtree, free right subtree, release this node.
    // The expanded first levels preserve that postorder without deleting payloads.
	void** first = (void**)arg[0];
	if (first != 0) {
		void** node = (void**)first[0];
		if (node != 0) {
			void** child = (void**)node[0];
			if (child != 0) {
				if (child[0] != 0) FxSequenceTree_FreeSubtree(self, (void**)child[0]);
				if (child[1] != 0) FxSequenceTree_FreeSubtree(self, (void**)child[1]);
				appFreeUni__FPv(child);
			}
			child = (void**)node[1];
			if (child != 0) {
				if (child[0] != 0) FxSequenceTree_FreeSubtree(self, (void**)child[0]);
				if (child[1] != 0) FxSequenceTree_FreeSubtree(self, (void**)child[1]);
				appFreeUni__FPv(child);
			}
			appFreeUni__FPv(node);
		}
		node = (void**)first[1];
		if (node != 0) {
			void** child = (void**)node[0];
			if (child != 0) {
				if (child[0] != 0) FxSequenceTree_FreeSubtree(self, (void**)child[0]);
				if (child[1] != 0) FxSequenceTree_FreeSubtree(self, (void**)child[1]);
				appFreeUni__FPv(child);
			}
			child = (void**)node[1];
			if (child != 0) {
				if (child[0] != 0) FxSequenceTree_FreeSubtree(self, (void**)child[0]);
				if (child[1] != 0) FxSequenceTree_FreeSubtree(self, (void**)child[1]);
				appFreeUni__FPv(child);
			}
			appFreeUni__FPv(node);
		}
		appFreeUni__FPv(first);
	}
	void** second = (void**)arg[1];
	if (second != 0) {
		void** node = (void**)second[0];
		if (node != 0) {
			void** child = (void**)node[0];
			if (child != 0) {
				if (child[0] != 0) FxSequenceTree_FreeSubtree(self, (void**)child[0]);
				if (child[1] != 0) FxSequenceTree_FreeSubtree(self, (void**)child[1]);
				appFreeUni__FPv(child);
			}
			child = (void**)node[1];
			if (child != 0) {
				if (child[0] != 0) FxSequenceTree_FreeSubtree(self, (void**)child[0]);
				if (child[1] != 0) FxSequenceTree_FreeSubtree(self, (void**)child[1]);
				appFreeUni__FPv(child);
			}
			appFreeUni__FPv(node);
		}
		node = (void**)second[1];
		if (node != 0) {
			void** child = (void**)node[0];
			if (child != 0) {
				if (child[0] != 0) FxSequenceTree_FreeSubtree(self, (void**)child[0]);
				if (child[1] != 0) FxSequenceTree_FreeSubtree(self, (void**)child[1]);
				appFreeUni__FPv(child);
			}
			child = (void**)node[1];
			if (child != 0) {
				if (child[0] != 0) FxSequenceTree_FreeSubtree(self, (void**)child[0]);
				if (child[1] != 0) FxSequenceTree_FreeSubtree(self, (void**)child[1]);
				appFreeUni__FPv(child);
			}
			appFreeUni__FPv(node);
		}
		appFreeUni__FPv(second);
	}
	appFreeUni__FPv(arg);
}
#pragma use_lmw_stmw reset
#pragma scheduling reset

struct BBNode { BBNode* left; BBNode* right; char parentStorage[4]; int flags; char nameLength[4]; void* value; };
#pragma inline_depth(3)
#pragma use_lmw_stmw on
#pragma scheduling on
// Free each resource-cache name before releasing its tree node.
void FxBoxResourceMap_DestroySubtree(void* self, BBNode* arg) {
	if (arg->left) FxBoxResourceMap_DestroySubtree(self, arg->left);
	if (arg->right) FxBoxResourceMap_DestroySubtree(self, arg->right);
	FxBoxResourceMap_DestroyStringStorage(self, (CachedResourceName*)&arg->flags);
	appFreeUni__FPv(arg);
}
#pragma inline_depth reset
#pragma use_lmw_stmw reset
#pragma scheduling reset

extern const char fxConditionCommandTokens[];
struct EffectFilenameExtensions { char sequence[4]; char spray[4]; };
extern const char kEffectFilenameExtensions[];
void* String_ParseSpaceDelimitedToken(void*, const char*);
void* String_ParseQuotedToken(void*, const char*);
char* strrchr(const char*, int);
int String_CompareCString(const char*, const char*);
int EffectCommand_ParseResourceFilename(void*, void*, const char*, int, int*);

// Parse a filename token and accept only supported sequence/spray extensions.
// parseCursor points to a mutable script cursor. outputFilename is WRITABLE
// caller storage despite the legacy const signature: both token parsers write it.
// Writes are unbounded; reserve room for the complete token plus NUL. The 64-byte
// check occurs after parsing. Cursor advances even when extension checks fail.
// resourceKind is assigned only after extension checks; return 1 supported, 0 rejected.
// Ownership stays with caller.
int EffectCommand_ParseResourceFilename(void* unused, void* parseCursor, const char* outputFilename, int tokenMode, int* resourceKind) {
	(void)unused;
	switch (tokenMode) {
	case kParseSpaceDelimited: {
		char** cursorSlot = (char**)parseCursor;
		*cursorSlot = (char*)String_ParseSpaceDelimitedToken(*cursorSlot + strlen(fxConditionCommandTokens), outputFilename);
		break;
	}
	case kParseQuoted: {
		char** cursorSlot = (char**)parseCursor;
		*cursorSlot = (char*)String_ParseQuotedToken(*cursorSlot + strlen(fxConditionCommandTokens), outputFilename);
		break;
	}
	default: return 0;
	}
	int filenameLength = (int)strlen(outputFilename);
	if (filenameLength == 0 || filenameLength >= kFilenameCapacity) return 0;
	char* extension = strrchr(outputFilename, '.');
	if (extension == 0 || strlen(extension) < 4) return 0;
	if (String_CompareCString(extension + 1, ((const EffectFilenameExtensions*)kEffectFilenameExtensions)->sequence) == 0) {
		*resourceKind = kResourceSequence;
	} else if (String_CompareCString(extension + 1, ((const EffectFilenameExtensions*)kEffectFilenameExtensions)->spray) == 0) {
		*resourceKind = kResourceSpray;
	} else {
		*resourceKind = kResourceUnknown;
		return 0;
	}
	return 1;
}

extern float kFxBoxPositiveBoundaryTolerance;
extern float kFxBoxNegativeBoundaryTolerance;
// Test strict positive/negative floating-point tolerances used by box boundary crossing.
static inline int box_near(float lhs, float rhs) {
	float difference = lhs - rhs;
	int withinTolerance = 0;
	if (difference < kFxBoxPositiveBoundaryTolerance && difference > kFxBoxNegativeBoundaryTolerance) withinTolerance = 1;
	return withinTolerance;
}
// Accept a height crossing or a step close enough to the boundary. Required
// previous/current/target borrow Vec2 storage; only previous.y, current.y and
// target.x are read. All heights and maximumStep use the caller's same distance
// unit; this helper does no coordinate conversion. unused is an ignored ABI word.
// Return 1 if current is within strict tolerance, or previous/current are on
// opposite sides. On the same side (or previous near the boundary), test current
// +/- maximumStep toward target: return 1 if near or beyond it, otherwise 0.
// No outputs are written. Keep the asymmetric Y-versus-X member access and the
// strict tolerance endpoints; neither same-side samples nor exact equality are
// prerequisites for success. The safe negative/NaN step domain is not established.
int CFxBox_TestHeightCrossing(float maximumStep, int unused, Vec2* previous, Vec2* current, Vec2* target) {
	(void)unused;
	if (!box_near(current->y, target->x)) {
		if (current->y < target->x) {
			if (box_near(previous->y, target->x) || previous->y < target->x) {
				int result = 1;
				if (!box_near(current->y + maximumStep, target->x)) {
					int crossed = current->y + maximumStep > target->x;
					if (!crossed) result = 0;
				}
				return result;
			}
		} else {
			if (box_near(previous->y, target->x) || previous->y > target->x) {
				int result = 1;
				if (!box_near(current->y - maximumStep, target->x)) {
					int crossed = current->y - maximumStep < target->x;
					if (!crossed) result = 0;
				}
				return result;
			}
		}
	}
	return 1;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462108", _ctors_u8010B244_0, FxBox_InitializeUnitConstants);
#pragma pop
