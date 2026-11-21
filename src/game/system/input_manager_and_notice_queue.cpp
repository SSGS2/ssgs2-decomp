#include <decomp/static_init.h>
#include "classes/WView.h"
#include "classes/CMsgDraw.h"
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "classes/InputActionState.h"
#include "classes/InputPointerState.h"
#include "classes/CInputManager.h"
#include "classes/NoticeEntry.h"
#include "classes/NoticeDisplay.h"
#include "classes/NoticeQueue.h"
#include "classes/CGolfDoc.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
#include "game/wvector.h"
#include "sdk_protos.h"
// Translation unit 0x80185E48-0x8018747C (16 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

// Notice priorities are array positions: the last active entry wins.
enum NoticeConstants {
    kNoticeDefinitionCount = 9, kStickyNoticeSlot = 0, kItemNoticeSlot = 1, kTransientNoticeSlot = 2,
    kNoticePauseStateMask = 0x30, kNoticeClearStateMask = 0xC0,
    kItemDescriptionInsertPrefix = 0x4B, // "S_"
    kItemDescriptionKeyPrefix = 0x4E // "MSG_NOR_ITEMDESC_"
};
enum NoticeStorageConstants {
    kNoticeEntryBytes = 0x18, kNoticeHeapFlagShift = 31,
    kNoticeInlineLengthMask = 0x7f, kNoticeWideCharacterBytes = 2
};
// A notice entry owns a 12-byte packed wide string followed by timer/active/
// persistent words. The display owns a separate packed string at +0x14.
// Heap: flags high bit, character count at +4, pointer at +8. Inline: count in
// the first byte's low seven bits, UTF-16 bytes at +2 (the tag occupies a full
// two-byte character slot). Lengths count 16-bit characters, never bytes.
// Pointer/int casts of active flags are 32-bit word stores, not pointer values.


// Device channels feed the pointer independently of the named action bindings.
// Button history: 0 idle, 1 just pressed, 2 just released, 3 held.
enum PointerInputConstants {
    kPointerButtonCount = 3,
    kDeviceButtons = 1,
    kDeviceAxes = 3,
    kAxisX = 1, kAxisY = 2, kAxisZ = 3,
    kCurrentButtonBit = 1,
    kButtonJustPressed = 1, kButtonJustReleased = 2,
    kInputSuppressionMask = 6,
    kAnalogBindingKind = 3,
    kActionHashLengthShift = 6,
    kActionHashFirstMask = 0x2A, kActionHashLastMask = 0x15
};
// Byte address retains the original string-copy scheduling in NoticeQueue_UpdateAndDraw.
static const int kNoticeDisplayStringOffset = 0x14; // NoticeDisplay::stringFlags


extern void appFreeUni__FPv(void*);
extern void CMsgDraw_ResetTextState(void*);
extern void Text_SetContent(void*, void*);
extern void CMsgDraw_GetTextExtent(void*, void*);
extern void Text_SetPosition(void*);
extern void CMsgDraw_ClearFormattingAndRevealAll(void*);
extern void CMsgDraw_UpdateReveal(void*);
extern void Text_DrawGlyphs(void*);
extern void WString_ReplaceRange(void*, int, unsigned int, char*, char*, char*, char*);
extern void WOverlay_DrawTransformedRect(void*, void*, void*, float, void*, int, int, int);
extern void Vec3_Zero(void*);
extern void __as__7WVectorFRC7WVector(void*, void*);
extern void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
extern unsigned int OS_GetTimeMilliseconds();
extern void BaseObject_dtor(void*, int);
extern int strcmp(const char*, const char*);
extern void __dl__FPv(void*);
extern void* g_view;
extern void* g_bindingDeviceAdapter; // binding-device adapter, virtual testBinding/readAnalogBinding interface
extern float kFloat_2_PointerScreenEdgeInset;
extern void* g_sharedDoc;
extern int kNoticeDefinitions[]; // nine NoticeDefinition records: ID, slot, template, initial timer
extern char kNoticeBlackTextPrefix[]; // UTF-16 black-color tag prepended before expanding notice text.
extern char noticeTemplateStrings[]; // notice template string pool (description prefix and inserted S_ tag)
extern char __vt__13CInputManager[];
extern void* g_inputActionManager;
extern const float kFloat_0_Notice; // 0: initial notice timer / background draw parameter
extern const float kNoticeTextInset; // 12: notice text/background inset
extern const float kFloat_2_NoticeBackground; // 2: notice background height scale
extern int SharedDoc_IsPlayerFlag0Set(int);
extern int TextResource_FindNamedEntry(void*, void*);
extern void Text_ExpandTaggedText(void*, void*, void*, int);
extern void WString_ReplaceRange(void*, int, unsigned int, char*, char*, char*, char*);
extern char* ItemInventory_FindDescriptionMessageKeyText(void*, const char*);
extern void String_ReserveCapacity(void*, unsigned int);
extern void String_ReplaceRange(void*, int, int, char*, char*, char*, char*);
}
namespace N86214 {
extern "C" void Text_SetPosition(void*, void*);
extern "C" void String_ReplaceRange(void*, int, int, char*, char*, char*);
extern "C" void WString_ReplaceRange(void*, int, unsigned int, char*, char*, char*);
}
extern "C" {


struct InstanceLock {
    // Acquire the mutex overlaid by this guard.
	void lock() { OSLockMutex((OSMutex*)this); }
    // Release the overlaid mutex at scope exit.
	~InstanceLock() { OSUnlockMutex((OSMutex*)this); }
};
struct InstanceLock_u80185E48 {
	void* lock;
    // Borrow and acquire the supplied mutex.
	InstanceLock_u80185E48(void* l) { lock = l; OSLockMutex((OSMutex*)l); }
    // Release the borrowed mutex after the guarded operation.
	~InstanceLock_u80185E48() { OSUnlockMutex((OSMutex*)lock); }
};

struct LinkedNode { LinkedNode* prev; LinkedNode* next; };

struct InputNode {
	void* value;
	char pad_04[8];
	InputNode* prev;
	InputNode* next;
	void* unlink(OSMutex* lock);
	void* insert(void* h, OSMutex* lock);
};
// Detach a circular-list node under its mutex and return the next node, or null for a singleton.
inline void* InputNode::unlink(OSMutex* lock) {
	InputNode* node = this;
	OSLockMutex(lock);
	node->next->prev = node->prev;
	node->prev->next = node->next;
	InputNode* next = node->next;
	if (next == node) next = 0;
	OSUnlockMutex(lock);
	return next;
}
// Insert this node before a circular-list head, creating a singleton when the list is empty.
inline void* InputNode::insert(void* h, OSMutex* lock) {
	InputNode* head = (InputNode*)h;
	InputNode* node = this;
	OSLockMutex(lock);
	if (!head) {
		node->prev = node;
		node->next = node;
		OSUnlockMutex(lock);
		return node;
	}
	node->next = head;
	node->prev = head->prev;
	head->prev = node;
	node->prev->next = node;
	OSUnlockMutex(lock);
	return head;
}
// Return the next registered action while advancing the mutex-protected cursor.
inline void* inputNext(CInputManager* self) {
	void* result;
	OSMutex* lock = (OSMutex*)(((CInputManager*)self)->mutex);
	OSLockMutex(lock);
	void* current = ((CInputManager*)self)->currentNode;
	if (current) {
		void* next = ((InputNode*)current)->next;
		result = ((InputNode*)current)->value;
		if (next == self->activeHead) next = 0;
		((CInputManager*)self)->currentNode = next;
		OSUnlockMutex(lock);
		return result;
	}
	OSUnlockMutex(lock);
	return 0;
}
// Start a protected traversal of the registered input actions.
inline void* inputFirst(CInputManager* self) {
	InstanceLock_u80185E48 guard(((CInputManager*)self)->mutex);
	((CInputManager*)self)->currentNode = self->activeHead;
	return inputNext(self);
}
// Free pending input nodes and the manager's pointer-slot storage under the mutex.
inline void inputDestroyList(CInputManager* self) {
	void* node;
	InstanceLock_u80185E48 guard(((CInputManager*)self)->mutex);
	void* pendingHead = self->pendingHead;
	if (pendingHead) {
		char* listMutex = ((CInputManager*)self)->mutex;
		while ((node = self->pendingHead) != 0) {
			void* next = ((InputNode*)node)->unlink((OSMutex*)listMutex);
			self->pendingHead = next;
			appFreeUni__FPv(node);
		}
	}
	void* slotArray = self->slots;
	if (slotArray) {
		appFreeUni__FPv(slotArray);
		self->slots = 0;
	}
}
// Delete one action and its owned binding-array storage; wrapper guards preserve emitted branches.
static inline void DestroyInputAction(void* current) {
				if (current) {
					if (&((InputActionState*)current)->bindings) {
					if (&((InputActionState*)current)->bindings) {
					if (&((InputActionState*)current)->bindings) {
					if (&((InputActionState*)current)->bindings) {
					if (&((InputActionState*)current)->bindings) {
					if (((InputActionState*)current)->bindings) {
						char* begin = (char*)((InputActionState*)current)->bindings;
						char* end = begin + ((InputActionState*)current)->bindingCount;
						((InputActionState*)current)->bindingCount -= end - begin;
						appFreeUni__FPv(((InputActionState*)current)->bindings);
					}
					} } } } }
					__dl__FPv(current);
				}
}
#pragma push
#pragma use_lmw_stmw on
#pragma opt_common_subs off
// Destroy action bindings, recycle active nodes, then release list storage and the singleton.
void* CInputManager_dtor(CInputManager* self, int doDelete) {
	if (self) {
		self->vtbl = __vt__13CInputManager;
		void* current = inputFirst(self);
		if (current) {
			// Keep the bottom-tested traversal: a while loop selects a different mutex register.
			goto input_test;
			input_body:
				DestroyInputAction(current);
				current = inputNext(self);
			input_test:
			if (current) goto input_body;
		}
		{
		InstanceLock_u80185E48 guard(((CInputManager*)self)->mutex);
		void* activeHead = self->activeHead;
		if (activeHead && self->freeHead) {
			((InputNode*)activeHead)->prev->next = ((InputNode*)self->freeHead)->next;
			((InputNode*)self->freeHead)->next->prev = ((InputNode*)self->activeHead)->prev;
			((InputNode*)self->freeHead)->next = (InputNode*)self->activeHead;
			((InputNode*)self->activeHead)->prev = (InputNode*)self->freeHead;
			self->activeHead = 0;
		} else if (activeHead) {
			char* listMutex = ((CInputManager*)self)->mutex;
			while ((current = self->activeHead) != 0) {
				void* nextActiveNode = ((InputNode*)current)->unlink((OSMutex*)listMutex);
				void* current = self->activeHead;
				void* freeHead = self->freeHead;
				current = ((InputNode*)current)->insert(freeHead, (OSMutex*)listMutex);
				self->freeHead = current;
				self->activeHead = nextActiveNode;
			}
		}
		{
			int i = 0;
			int offset = 0;
			void* emptySlot = 0;
			for (; i < self->slotCount; offset += 4) {
				void* slotArray = self->slots;
				i++;
				*(void**)((char*)slotArray + offset) = emptySlot;
			}
		}
		}
		if (&self->activeHead) {
			inputDestroyList(self);
		}
		if (self) {
			g_inputActionManager = 0;
			BaseObject_dtor(self, 0);
		}
		if (doDelete > 0) __dl__FPv(self);
	}
	return self;
}
#pragma pop
struct String86214 { unsigned int flags; unsigned int size; char* data; };
struct Al86214 { char c; };
union U86214 { char c; int i; };

struct NoticeDefinition { int key; int index; void* resource; float value; }; // ID, queue slot, template name, initial timer
// Find one of the nine notice definitions by its public message ID.
inline NoticeDefinition* findNoticeDefinition(NoticeDefinition* p, int key) {
	for (int i = 0; i < kNoticeDefinitionCount; ++i, ++p) {
		if (p->key == key) return p;
	}
	return 0;
}
// Activate the requested notice template for the current player, unless that player suppresses notices.
// noticeId must exist in the nine-entry definition table; the lookup result is
// dereferenced without a missing-ID check. The queue owns the expanded text.
void NoticeQueue_ShowById(NoticeQueue* self, int noticeId) {
	volatile U86214 a;
	volatile U86214 b;
	void* sharedDoc = g_sharedDoc;
	if (!sharedDoc) return;
	if (SharedDoc_IsPlayerFlag0Set(AsCGolfDoc((void*)AsCSharedDoc(sharedDoc)->golfDoc)->currentPlayer)) return;
	NoticeDefinition* definition = findNoticeDefinition((NoticeDefinition*)kNoticeDefinitions, noticeId);
	void* textTemplate = (void*)TextResource_FindNamedEntry(self->textTemplates, definition->resource);
	if (textTemplate) {
	NoticeEntry* notice = (NoticeEntry*)self->entries + definition->index;
	((NoticeEntry*)notice)->remainingDisplayTime = definition->value;
	*(int*)&((NoticeEntry*)notice)->active = 1;
	switch (definition->index) {
	case 0: ((NoticeEntry*)notice)->persistent = 1; break;
	case 1: break;
	case 2: ((NoticeEntry*)notice)->persistent = 0; break;
	}
	{
	char* formatPrefix = kNoticeBlackTextPrefix;
	unsigned int oldLength = !(*(unsigned int*)notice >> 31) ? (*(unsigned char*)notice & 0x7f) : ((NoticeEntry*)notice)->stringLength;
	b.c = a.c;
	unsigned int len = wcslen((const wchar_t*)formatPrefix);
	N86214::WString_ReplaceRange(notice, 0, oldLength, formatPrefix, formatPrefix + len * 2, (char*)&b.c);
	}
	Text_ExpandTaggedText(self->textTemplates, notice, textTemplate, 0);
	}
}

// Resolve an inventory item's description template and activate the item notice slot.
void NoticeQueue_ShowItemNotice(NoticeQueue* self, const char* itemId) {
	// Allocator-tag byte copies are ABI-visible; these tokens carry no application state.
	volatile U86214 templateAllocator;
	volatile U86214 templateAllocatorCopy;
	volatile U86214 insertAllocatorCopy;
	volatile U86214 insertAllocator;
	volatile U86214 prefixAllocator;
	volatile U86214 prefixAllocatorCopy;
	// Phase 1: resolve the inventory description key and clear the previous item notice.
	if (g_sharedDoc == 0) return;
	if (SharedDoc_IsPlayerFlag0Set(AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->currentPlayer)) return;
	NoticeEntry* base = (NoticeEntry*)self->entries;
	*(int*)&((NoticeEntry*)base)[1].active = 0;
	NoticeEntry* notice = base + kItemNoticeSlot;
	if (!itemId) return;
	itemId = ItemInventory_FindDescriptionMessageKeyText(((CSharedDoc*)g_sharedDoc)->itemInventory, (char*)itemId);
	if (!itemId) return;
	String86214 templateName = {};
	
	unsigned int len; len = strlen(itemId);
	// Preserve the small-string ABI's separate reserve/replace length copies and allocator-byte copies.
	unsigned int reserveLength = len;
	unsigned int sourceLength = len; String_ReserveCapacity(&templateName, reserveLength);
	templateAllocatorCopy.c = templateAllocator.c; N86214::String_ReplaceRange(&templateName, 0, 0, (char*)itemId, (char*)itemId + sourceLength, (char*)&templateAllocatorCopy.c);
	struct StringCleanup {
	    void* object;
        // Release the temporary narrow template name if it grew onto the heap.
	    ~StringCleanup() {
	        unsigned int* words = (unsigned int*)object;
	        if (words[0] >> 31) appFreeUni__FPv(((String86214*)object)->data);
	    }
	} cleanup = {&templateName};
	// Phase 2: insert S_ after MSG_NOR_ITEMDESC_ to select the notice template.
	char* prefix;
	prefix = noticeTemplateStrings + kItemDescriptionInsertPrefix;
	char* descriptionPrefix = noticeTemplateStrings + kItemDescriptionKeyPrefix;
	unsigned int descriptionPrefixLength = strlen(descriptionPrefix);

 insertAllocatorCopy.c = insertAllocator.c;
 N86214::String_ReplaceRange(&templateName, descriptionPrefixLength, 0, prefix, prefix + (strlen(prefix)), (char*)&insertAllocatorCopy.c);
	void* render = self->textTemplates;
	char* text = !(templateName.flags >> 31) ? (char*)&templateName + 1 : templateName.data;
	void* textTemplate = (void*)TextResource_FindNamedEntry(render, text);
	if (!textTemplate) {
		return;
	}
	// Phase 3: copy the color prefix and expand the borrowed template into queue-owned text.
	{
		((NoticeEntry*)notice)->remainingDisplayTime = kFloat_0_Notice;
		*(int*)&((NoticeEntry*)notice)->active = 1;
		char* formatPrefix = kNoticeBlackTextPrefix; ((NoticeEntry*)notice)->persistent = 1;
		unsigned int oldLength = !(((NoticeEntry*)notice)->stringFlags >> 31) ? (*(unsigned char*)notice & 0x7f) : ((NoticeEntry*)notice)->stringLength;

		prefixAllocatorCopy.c = prefixAllocator.c;
  int formatPrefixLength = wcslen((const wchar_t*)formatPrefix);
  N86214::WString_ReplaceRange(notice, 0, oldLength, formatPrefix, formatPrefix + formatPrefixLength * 2, (char*)&prefixAllocatorCopy.c);
		Text_ExpandTaggedText(self->textTemplates, notice, textTemplate, 0);
	}
}

// Alternate view of NoticeDisplay's same string at +14; not a second allocation.
struct Text85 {
	char pad[0x14];
	union { unsigned int flags; unsigned char tag; };
	unsigned int size;
	char* data;
};

// Choose the last active notice, update its text/three-part background, and draw it when visible.
// Priority is highest active array index. Pause bits return without changing
// display state; clear bits disable the queue and entries but continue this call
// to empty/update the display. This routine does not decrement notice timers.
// Entry text remains entry-owned; inline strings copy by value, heap strings use
// ReplaceRange to keep display ownership independent. Positions/extents use the
// display's 2D drawing coordinates; the middle background stretches vertically.
void NoticeQueue_UpdateAndDraw(NoticeQueue* self) {
	volatile U86214 allocatorCopy;
	volatile U86214 allocator;
	float position[2];
	float textExtent[2];
	void* display;
	if (g_sharedDoc && !SharedDoc_IsPlayerFlag0Set(AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->currentPlayer) && *(int*)&self->active && self->display) {
	unsigned int state = *(unsigned int*)&AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->gameStateFlags;
	if (state & kNoticePauseStateMask) return;
	if (state & kNoticeClearStateMask) {
        // Deactivate all entries and clear the display without releasing capacity.
		char* noticeCursor = (char*)self->entries;
		*(void**)&self->active = 0;
		while (noticeCursor != (char*)self->entries + *(int*)&self->entryCount * kNoticeEntryBytes) { *(void**)&((NoticeEntry*)noticeCursor)->active = 0; noticeCursor += kNoticeEntryBytes; }
		display = self->display;
		CMsgDraw_ResetTextState(((NoticeDisplay*)display)->textRenderer);
		if ((((NoticeDisplay*)display)->stringFlags >> 31) == 0) {
			*(unsigned short*)((char*)((unsigned short*)&((NoticeDisplay*)display)->stringFlags + 1)) = 0;
			signed char __tmp = *(signed char*)&((NoticeDisplay*)display)->stringFlags;
			__tmp = (signed char)((int)__tmp & ~0x7F);
			*(unsigned char*)&((NoticeDisplay*)display)->stringFlags = (unsigned char)__tmp;
		} else {
			*(unsigned short*)((NoticeDisplay*)display)->stringData = 0;
			((NoticeDisplay*)display)->stringLength = 0;
		}
	}
    // Scan low to high, overwriting the candidate so the highest slot wins.
	int entryCount = *(int*)&self->entryCount;
	char* selectedNotice = 0;
	char* entries = (char*)self->entries;
	char* entriesEnd = entries + entryCount * kNoticeEntryBytes;
	for (char* noticeCursor = entries; noticeCursor != entriesEnd; noticeCursor += kNoticeEntryBytes) {
		if (*(int*)&((NoticeEntry*)noticeCursor)->active) selectedNotice = noticeCursor;
	}
	if (selectedNotice) {
        // Both inline: copy all 12 representation bytes, which contain the text.
        // Otherwise ReplaceRange copies characters without aliasing entry ownership.
		display = self->display;
		unsigned int flags = ((Text85*)display)->flags;
		if (!(flags >> 31) && !(((NoticeEntry*)selectedNotice)->stringFlags >> 31)) {
			unsigned int second = ((NoticeEntry*)selectedNotice)->stringLength;
			((Text85*)display)->flags = ((NoticeEntry*)selectedNotice)->stringFlags;
			((NoticeDisplay*)display)->stringLength = second;
			*(unsigned int*)&((NoticeDisplay*)display)->stringData = *(unsigned int*)&((NoticeEntry*)selectedNotice)->stringData;
		} else {
			unsigned int oldLength = (flags >> 31) ? ((Text85*)display)->size : (((Text85*)display)->tag & 0x7f);
			unsigned int len;
			char* textBegin;
			if (!(((NoticeEntry*)selectedNotice)->stringFlags >> kNoticeHeapFlagShift)) { textBegin = selectedNotice + kNoticeWideCharacterBytes; len = *(unsigned char*)selectedNotice & kNoticeInlineLengthMask; }
			else { textBegin = *(char**)&((NoticeEntry*)selectedNotice)->stringData; len = ((NoticeEntry*)selectedNotice)->stringLength; }
			char* textEnd = textBegin + len * kNoticeWideCharacterBytes;
			allocatorCopy.c = allocator.c;
			N86214::WString_ReplaceRange((char*)display + kNoticeDisplayStringOffset, 0, oldLength, textBegin, textEnd, (char*)&allocatorCopy.c);
		}
		((NoticeDisplay*)display)->dirty = 1;
	} else {
        // No winner: retain storage but reset reveal and terminate the empty string.
		display = self->display;
		void* arg = ((NoticeDisplay*)display)->textRenderer;
		CMsgDraw_ResetTextState(arg);
		if ((((NoticeDisplay*)display)->stringFlags >> 31) == 0) {
			*(unsigned short*)((char*)((unsigned short*)&((NoticeDisplay*)display)->stringFlags + 1)) = 0;
			signed char __tmp = *(signed char*)&((NoticeDisplay*)display)->stringFlags;
			__tmp = (signed char)((int)__tmp & ~0x7F);
			*(unsigned char*)&((NoticeDisplay*)display)->stringFlags = (unsigned char)__tmp;
		} else {
			*(unsigned short*)((NoticeDisplay*)display)->stringData = 0;
			((NoticeDisplay*)display)->stringLength = 0;
		}
	}
	display = self->display;
	if (((NoticeDisplay*)display)->dirty) {
        // Rebuild text layout only when dirty; extent[1] is the measured height.
		unsigned int oldLength = !(((NoticeDisplay*)display)->stringFlags >> 31) ? (*(unsigned char*)&((NoticeDisplay*)display)->stringFlags & 0x7f) : ((NoticeDisplay*)display)->stringLength;
		if (oldLength) {
			CMsgDraw_ResetTextState(((NoticeDisplay*)display)->textRenderer);
			Text_SetContent(((NoticeDisplay*)display)->textRenderer, !(((NoticeDisplay*)display)->stringFlags >> 31) ? (char*)((unsigned short*)&((NoticeDisplay*)display)->stringFlags + 1) : ((NoticeDisplay*)display)->stringData);
			CMsgDraw_GetTextExtent(((NoticeDisplay*)display)->textRenderer, &textExtent[0]);
			float middleHeight = kFloat_2_NoticeBackground * (kNoticeTextInset - ((NoticeDisplay*)display)->firstSectionHeight) + textExtent[1];
			((NoticeDisplay*)display)->secondSectionHeight = middleHeight;
			float middleTop = ((NoticeDisplay*)display)->y + ((NoticeDisplay*)display)->firstSectionHeight;
			((NoticeDisplay*)display)->secondSectionY = middleTop;
			((NoticeDisplay*)display)->thirdSectionY = middleTop + middleHeight;
			position[0] = kNoticeTextInset + ((NoticeDisplay*)display)->x;
			position[1] = kNoticeTextInset + ((NoticeDisplay*)display)->y;
			N86214::Text_SetPosition(((NoticeDisplay*)display)->textRenderer, position);
			CMsgDraw_ResetTextState(((NoticeDisplay*)display)->textRenderer);
			Text_SetContent(((NoticeDisplay*)display)->textRenderer, !(((NoticeDisplay*)display)->stringFlags >> 31) ? (char*)((unsigned short*)&((NoticeDisplay*)display)->stringFlags + 1) : ((NoticeDisplay*)display)->stringData);
			CMsgDraw_ClearFormattingAndRevealAll(((NoticeDisplay*)display)->textRenderer);
		} else {
			CMsgDraw_ResetTextState(((NoticeDisplay*)display)->textRenderer);
		}
		((NoticeDisplay*)display)->dirty = 0;
	}
    // Reveal progresses even when hidden; visibility/nonempty text gate drawing only.
	CMsgDraw_UpdateReveal(((NoticeDisplay*)display)->textRenderer);
	{
	void* display = self->display;
	if (!((NoticeDisplay*)display)->visible) return;
	unsigned int count = !(((NoticeDisplay*)display)->stringFlags >> 31) ? (*(unsigned char*)&((NoticeDisplay*)display)->stringFlags & 0x7f) : ((NoticeDisplay*)display)->stringLength;
	if (!count) return;
	WOverlay_DrawTransformedRect(((NoticeDisplay*)display)->drawResource, g_view, (char*)&((NoticeDisplay*)display)->firstDrawTransform, kFloat_0_Notice, (char*)&((NoticeDisplay*)display)->x, 0, -1, 0);
	WOverlay_DrawTransformedRect(((NoticeDisplay*)display)->drawResource, g_view, (char*)&((NoticeDisplay*)display)->secondDrawTransform, kFloat_0_Notice, (char*)&((NoticeDisplay*)display)->secondSectionX, 0, -1, 0);
	WOverlay_DrawTransformedRect(((NoticeDisplay*)display)->drawResource, g_view, (char*)&((NoticeDisplay*)display)->thirdDrawTransform, kFloat_0_Notice, (char*)&((NoticeDisplay*)display)->thirdSectionX, 0, -1, 0);
	(*(unsigned int*)&((CMsgDraw*)((NoticeDisplay*)display)->textRenderer)->drawSuppressed) = 0;
	Text_DrawGlyphs(((NoticeDisplay*)display)->textRenderer);
	}
	}
	return;
}
struct OwnedStringStorage {
	unsigned int flags;
	char pad_04[4];
	void* ptr;
};




struct NoticeStringStorage {
	unsigned int flags;
	char pad_04[4];
	void* ptr;
};

struct NoticeStringVector {
	void* data;
	int size;
	char pad_08[8];
	int originIndex;
};


struct PointerActionValue {
	PointerActionValue* next;
	char pad_04[0x10];
	unsigned int filterFlags;
	void* value18;
};

struct PointerActionRegistry {
	char pad_00[0x24];
	unsigned int mask24;
	char pad_28[4];
	PointerActionValue** buckets;
	char pad_30[0xAC];
	unsigned int mode;
	char pad_E0[4];
	unsigned char enabled;
};


extern const float kPointerInitialXFraction; // 0.625: initial pointer X fraction of viewport width
extern float kPointerInitialYFraction; // 0.5: initial pointer Y fraction of viewport height
extern const float kFloat_0_Pointer; // 0: pointer reset/delta value
extern float kFloat_2_PointerScreenEdgeInset;
extern void* g_pointerDeviceAdapter; // pointer-device adapter, virtual sampleChannel interface

struct PointerButtonMapping { char pad_000[0xE5]; unsigned char swapPrimaryButtons; };

// Return a button's two-bit history, optionally exchanging the primary and secondary buttons.
void* Pointer_GetButtonState(InputPointerState* self, int buttonIndex) {
	if (!self->enabled)
		return 0;
	if (((PointerButtonMapping*)self)->swapPrimaryButtons) {
		if (buttonIndex == 0)
			buttonIndex = 1;
		else if (buttonIndex == 1)
			buttonIndex = 0;
	}
	return *(void**)&((InputPointerState*)self)->buttons[buttonIndex].state;
}

// Release heap-backed string storage and optionally delete the string object.
void* OwnedStringStorage_dtor(OwnedStringStorage* self, int doDelete) {
	if (self != 0) {
		// Original 0x801866FC: preserve the shared deletion tail.
		do {
			if (self == 0) break;
			if ((self->flags >> 31) != 0)
				appFreeUni__FPv(self->ptr);
		} while (0);
		if (doDelete > 0)
			__dl__FPv(self);
	}
	return self;
}

// Deactivate every notice and empty the display string.
void NoticeQueue_Clear(NoticeQueue* self) {
	NoticeEntry* entry = (NoticeEntry*)self->entries;
	while (entry != (NoticeEntry*)self->entries + self->entryCount) {
		((NoticeEntry*)entry)->active = 0;
		++entry;
	}
	void* item = self->display;
	CMsgDraw_ResetTextState(((NoticeDisplay*)item)->textRenderer);
	if (((((NoticeDisplay*)item)->stringFlags) >> 31) == 0) {
		*(unsigned short*)((char*)((unsigned short*)&((NoticeDisplay*)item)->stringFlags + 1)) = 0;
		signed char __tmp = *(signed char*)&((NoticeDisplay*)item)->stringFlags;
		__tmp = (signed char)((int)__tmp & ~0x7F);
		*(unsigned char*)&((NoticeDisplay*)item)->stringFlags = (unsigned char)__tmp;
	} else {
		*(unsigned short*)((NoticeDisplay*)item)->stringData = 0;
		((NoticeDisplay*)item)->stringLength = 0;
	}
}

// Clear all pending/displayed notices and enable the queue.
void NoticeQueue_ResetAndActivate(NoticeQueue* self) {
	NoticeEntry* entry = (NoticeEntry*)self->entries;
	while (entry != (NoticeEntry*)self->entries + self->entryCount) {
		((NoticeEntry*)entry)->active = 0;
		++entry;
	}
	void* item = self->display;
	CMsgDraw_ResetTextState(((NoticeDisplay*)item)->textRenderer);
	if (((((NoticeDisplay*)item)->stringFlags) >> 31) == 0) {
		*(unsigned short*)((char*)((unsigned short*)&((NoticeDisplay*)item)->stringFlags + 1)) = 0;
		signed char __tmp = *(signed char*)&((NoticeDisplay*)item)->stringFlags;
		__tmp = (signed char)((int)__tmp & ~0x7F);
		*(unsigned char*)&((NoticeDisplay*)item)->stringFlags = (unsigned char)__tmp;
	} else {
		*(unsigned short*)((NoticeDisplay*)item)->stringData = 0;
		((NoticeDisplay*)item)->stringLength = 0;
	}
	self->active = 1;
}

// Destroy notice strings from the vector's active range, then release its backing storage.
// This specialization uses capacity as a logical origin index: the live range
// starts at data + capacity*24 and contains size entries. It is not the usual
// vector allocation limit. Each entry owns only its heap string; nodes are inline.
void* NoticeStringVector_dtor(NoticeStringVector* self, int doDelete) {
	if (self != 0) {
		NoticeStringStorage* end;
		NoticeStringStorage* begin = (NoticeStringStorage*)((char*)self->data + self->originIndex * 0x18);
		end = (NoticeStringStorage*)((char*)begin + self->size * 0x18);
		while (end > begin) {
			end = (NoticeStringStorage*)((char*)end - 0x18);
			if (end != 0) {
			do {
				if (end == 0) break;
				if ((end->flags >> 31) != 0)
					appFreeUni__FPv(((NoticeEntry*)end)->stringData);
			} while (0);
			}

		}
		self->size = 0;
		if (self != 0 && self->data != 0) {
			int i = self->size;
			NoticeStringStorage* entry = (NoticeStringStorage*)((char*)self->data + i * 0x18);
			self->size = 0;
			while (i != 0) {
				entry = (NoticeStringStorage*)((char*)entry - 0x18);
				if (entry != 0) {
				do {
					if (entry == 0) break;
					if (entry->flags >> 31)
						appFreeUni__FPv(((NoticeEntry*)entry)->stringData);
				} while (0);
				}

				--i;
			}
			appFreeUni__FPv(self->data);
		}
		if (doDelete > 0)
			__dl__FPv(self);
	}
	return self;
}

// Clamp the pointer to the viewport with a two-unit border.
void Pointer_SetPositionClamped(InputPointerState* self, float x, float y) {
	WView* global = (WView*)g_view;
	float maxX = global->viewportWidth - kFloat_2_PointerScreenEdgeInset;
	float resultX;
	if (x < kFloat_2_PointerScreenEdgeInset)
		resultX = kFloat_2_PointerScreenEdgeInset;
	else {
		float t = x > maxX ? maxX : x;
		resultX = t;
	}
	self->position[0] = resultX;
	float minY = kFloat_2_PointerScreenEdgeInset;
	bool belowY = y < minY;
	float maxY = ((WView*)g_view)->viewportHeight - minY;
	float resultY;
	if (belowY)
		resultY = minY;
	else {
		float t = y > maxY ? maxY : y;
		resultY = t;
	}
	self->position[1] = resultY;
}

#pragma push
#pragma opt_common_subs off
extern "C++" {
// Hash a terminated action name into the registry's masked bucket index under its mutex.
inline unsigned int hashActionName(PointerActionRegistry* self, const char* name) {
	OSMutex* lock = (OSMutex*)((InputPointerState*)self)->mutex;
	OSLockMutex(lock);
	unsigned int len = strlen(name);
	unsigned int key = len << kActionHashLengthShift;
	unsigned int first = (unsigned char)name[0] & kActionHashFirstMask;
	unsigned int last = (int)len > 0 ? len - 1 : 0;
	key |= first | ((unsigned char)name[last] & kActionHashLastMask);
	unsigned int index = key & self->mask24;
	OSUnlockMutex(lock);
	return index;
}
// Look up a named action in the mutex-protected hash buckets.
inline PointerActionValue* findActionByName(const char* name, PointerActionRegistry* self) {
	struct Bucket {
		PointerActionValue* value;
		const char* name;
		char pad_08[0xc];
		Bucket* next;
	};
	PointerActionValue* result = 0;
	InstanceLock_u80185E48 guard(((InputPointerState*)self)->mutex);
	if (name == 0 || self->buckets == 0)
		return result;
	unsigned int key = hashActionName(self, name);
	Bucket* node = (Bucket*)self->buckets[key];
	while (node != 0) {
		if (strcmp(node->name, name) == 0) {
			result = node->value;
			return result;
		}
		node = node->next;
	}
	return result;
}
}
// Read a named action's low state bit, applying the optional mode-dependent filter.
int Pointer_IsActionActive(PointerActionRegistry* self, const char* name, int applyModeFilter) {
	if (!self->enabled)
		return 0;
	PointerActionValue* node = findActionByName(name, self);
	int result = node ? (int)(long)node->value18 : 0;
	if (applyModeFilter && (int)self->mode == 1 && result && !(node->filterFlags & 1))
		result = 0;
	return result & 1;
}
#pragma pop

// The pointer embeds the same list prefix, but is not a CInputManager object.
struct PointerActionCursorView {
    char prefix[0x10]; void* activeHead; void* currentNode;
};
typedef char PointerCursorHeadOffset[((int)&((PointerActionCursorView*)0)->activeHead == 0x10) ? 1 : -1];
typedef char PointerCursorNextOffset[((int)&((PointerActionCursorView*)0)->currentNode == 0x14) ? 1 : -1];

// Advance the pointer's protected action-list cursor, ending after one circular traversal.
inline PointerActionValue* nextPointerAction(InputPointerState* self) {
	PointerActionValue* result;
	OSMutex* lock = (OSMutex*)self->mutex;
	OSLockMutex(lock);
	void* current = ((PointerActionCursorView*)self)->currentNode;
	if (current) {
		void* next = ((InputNode*)current)->next;
		result = (PointerActionValue*)((InputNode*)current)->value;
		if (next == ((PointerActionCursorView*)self)->activeHead) next = 0;
		((PointerActionCursorView*)self)->currentNode = next;
		OSUnlockMutex(lock);
		return result;
	}
	OSUnlockMutex(lock);
	return 0;
}

// Clear action/button histories, reset timing, place the pointer, and enable input.
void Pointer_Reset(InputPointerState* self) {
	PointerActionValue* node;
	{
	InstanceLock_u80185E48 guard(((InputPointerState*)self)->mutex);
	((PointerActionCursorView*)self)->currentNode = ((PointerActionCursorView*)self)->activeHead;
	node = nextPointerAction(self);
	}
	{
		// Original 0x80186BE0: reset each action from the guarded iterator.
		while (node) {
			node->value18 = 0;
			*(void**)&((InputActionState*)node)->previousActive = 0;
			((InputActionState*)node)->forceActive = 0;
			node = nextPointerAction(self);
		}
	}
	Vec3_Zero((char*)((InputPointerState*)self)->motionDelta);
	float scaleY = kPointerInitialYFraction;
	float width = ((WView*)g_view)->viewportWidth;
	float x = kPointerInitialXFraction * width;
	float zeroValue = kFloat_0_Pointer;
	self->position[0] = x;
	float y = scaleY * ((WView*)g_view)->viewportHeight;
	self->position[2] = zeroValue;
	self->position[1] = y;
	char* position = (char*)((InputPointerState*)self)->buttons[0].pressPosition;
	char* item = (char*)self;
	for (int i = 0; i < kPointerButtonCount; ++i) {
		((InputPointerState*)item)->buttons[0].state = 0;
		float zero[3] = {zeroValue, zeroValue, zeroValue};
		*(WVector*)position = *(WVector*)zero;
		((InputPointerState*)item)->buttons[0].pressIntervalMs = 0;
		((InputPointerState*)item)->buttons[0].lastPressMs = OS_GetTimeMilliseconds();
		((InputPointerState*)item)->buttons[0].forcePressed = 0;
		((InputPointerState*)item)->buttons[0].suppressPressed = 0;
		item += sizeof(InputButtonState);
		position += sizeof(InputButtonState);
	}
	self->stateFlags = 0;
	self->mode = 0;
	((InputPointerState*)self)->enabled = 1;
}

struct PointerActionBinding {
	char pad_00[4];
	void** bindings;
	unsigned int bindingCount;
	char pad_C[4];
	int bindingKind;
	unsigned int filterFlags;
	int active;
	int previousActive;
	int analogValue;
	unsigned char forceActive;
};
// Device adapters resolve a configured binding or read its analog value; concrete backends live elsewhere.
struct BindingDevice {
	virtual void unused_0();
	virtual void unused_1();
	virtual void unused_2();
	virtual void unused_3();
	virtual void unused_4();
	virtual void unused_5();
	virtual void unused_6();
	virtual void unused_7();
	virtual int testBinding(int bindingKind, void* binding);
	virtual int readAnalogBinding(void* binding);
};
// Device boundary for replacing Remote input: sample button channels and the three pointer axes here.
struct PointerDevice {
	virtual void unused_0();
	virtual void unused_1();
	virtual void unused_2();
	virtual void unused_3();
	virtual void unused_4();
	virtual void unused_5();
	virtual void unused_6();
	virtual void unused_7();
	virtual int sampleChannel(int channelKind, int channelIndex);
};
union Cvt86E08 { double d; struct { unsigned int hi, lo; } w; };
struct PositionBits { unsigned int x, y, z; };

// Cursor ABI: advancing the byte base retains the +0x60 button-array displacement.
// This view is not a complete pointer object and must only access this one button.
struct ShiftedPointerButtonView { char beforeButton[0x60]; InputButtonState button; };
typedef char ShiftedPointerButtonOffset[((int)&((ShiftedPointerButtonView*)0)->button == 0x60) ? 1 : -1];

// Poll named bindings and device axes/buttons, then record motion and press/release transitions.
void Pointer_Update(InputPointerState* self) {
	if (!((InputPointerState*)self)->enabled) return;
	// Phase 1: snapshot action history, apply binding tests and one-shot overrides.
	PointerActionBinding* entry;
	{
		InstanceLock_u80185E48 guard(((InputPointerState*)self)->mutex);
		((PointerActionCursorView*)self)->currentNode = ((PointerActionCursorView*)self)->activeHead;
		entry = (PointerActionBinding*)nextPointerAction(self);
	}
	{
		// Evaluate all configured bindings for each action; forceActive overrides suppression for one update.
		while (entry) {
			entry->previousActive = entry->active & 1;
			entry->active = 0;
			void** array = entry->bindings;
			while (array != entry->bindings + entry->bindingCount) {
				if (((BindingDevice*)g_bindingDeviceAdapter)->testBinding(entry->bindingKind, *array)) {
					entry->active = 1;
					break;
				}
				++array;
			}
			if (entry->forceActive) {
				entry->forceActive = 0;
				entry->active = 1;
			} else {
				if (entry->filterFlags & kInputSuppressionMask) entry->active = 0;
				if (entry->bindingKind == kAnalogBindingKind && entry->active)
					entry->analogValue = ((BindingDevice*)g_bindingDeviceAdapter)->readAnalogBinding(entry->bindings[0]);
			}
			entry = (PointerActionBinding*)nextPointerAction(self);
		}
	}
	// Phase 2: suppressed motion clears XY only; otherwise clamp XY to viewport pixels
	// with a two-pixel inset and compute XYZ displacement from the previous sample.
	if (self->stateFlags & kInputSuppressionMask) {
		float* motionDelta = (float*)((char*)((InputPointerState*)self)->motionDelta);
		float z = kFloat_0_Pointer;
		motionDelta[0] = z;
		motionDelta[1] = z;
	} else {
		PositionBits old = *(PositionBits*)&self->position[0];
		PositionBits out;
		float maxX = ((WView*)g_view)->viewportWidth - kFloat_2_PointerScreenEdgeInset;
		float sampledX = (float)((PointerDevice*)g_pointerDeviceAdapter)->sampleChannel(kDeviceAxes, kAxisX);
		float clampedX;
		if (sampledX < kFloat_2_PointerScreenEdgeInset) clampedX = kFloat_2_PointerScreenEdgeInset;
		else {
			float t = sampledX > maxX ? maxX : sampledX;
			clampedX = t;
		}
		self->position[0] = clampedX;
		float maxY = ((WView*)g_view)->viewportHeight - kFloat_2_PointerScreenEdgeInset;
		float sampledY = (float)((PointerDevice*)g_pointerDeviceAdapter)->sampleChannel(kDeviceAxes, kAxisY);
		float clampedY;
		if (sampledY < kFloat_2_PointerScreenEdgeInset) clampedY = kFloat_2_PointerScreenEdgeInset;
		else {
			float t = sampledY > maxY ? maxY : sampledY;
			clampedY = t;
		}
		self->position[1] = clampedY;
		self->position[2] = (float)((PointerDevice*)g_pointerDeviceAdapter)->sampleChannel(kDeviceAxes, kAxisZ);
		__mi__FRC7WVectorRC7WVector(&out, &self->position[0], &old);
		*(WVector*)&((InputPointerState*)self)->motionDelta = *(WVector*)&out;
	}
	// Phase 3: encode old/new pressed bits into idle/press/release/held states.
	int i = 0;
	unsigned char* item = (unsigned char*)self;
	// The cursor advances one button record; accesses retain the object's button-array base offset.
	for (; i < kPointerButtonCount; ++i) {
		// Button polling is separate from named-action polling above.
		int sampledPressed = ((PointerDevice*)g_pointerDeviceAdapter)->sampleChannel(kDeviceButtons, i);
		if (((ShiftedPointerButtonView*)item)->button.forcePressed) {
			((ShiftedPointerButtonView*)item)->button.forcePressed = 0;
			((ShiftedPointerButtonView*)item)->button.state = ((((ShiftedPointerButtonView*)item)->button.state & kCurrentButtonBit) ? 2 : 0) + 1;
		} else {
			((ShiftedPointerButtonView*)item)->button.state = ((((ShiftedPointerButtonView*)item)->button.state & kCurrentButtonBit) ? 2 : 0) + (((ShiftedPointerButtonView*)item)->button.suppressPressed ? 0 : (sampledPressed != 0));
		}
		item += sizeof(InputButtonState);
	}
	// Phase 4: record edge positions and unsigned millisecond intervals.
	// State 1 snapshots a press; state 2 snapshots a release. Held buttons preserve their timestamps.
	unsigned int nowMs = OS_GetTimeMilliseconds();
	unsigned int* unusedWords = (unsigned int*)self;
	if ((int)((InputPointerState*)self)->buttons[0].state == kButtonJustPressed) {
		*(WVector*)(&((InputPointerState*)self)->buttons[0].pressPosition) = *(WVector*)(&self->position[0]);
		unsigned int intervalMs = nowMs - ((InputPointerState*)self)->buttons[0].lastPressMs;
		((InputPointerState*)self)->buttons[0].lastPressMs = nowMs;
		((InputPointerState*)self)->buttons[0].pressIntervalMs = intervalMs;
	} else if ((int)((InputPointerState*)self)->buttons[0].state == kButtonJustReleased) *(WVector*)(&((InputPointerState*)self)->buttons[0].releasePosition) = *(WVector*)(&self->position[0]);
	if ((int)((InputPointerState*)self)->buttons[1].state == kButtonJustPressed) {
		*(WVector*)(&((InputPointerState*)self)->buttons[1].pressPosition) = *(WVector*)(&self->position[0]);
		unsigned int intervalMs = nowMs - ((InputPointerState*)self)->buttons[1].lastPressMs;
		((InputPointerState*)self)->buttons[1].lastPressMs = nowMs;
		((InputPointerState*)self)->buttons[1].pressIntervalMs = intervalMs;
	} else if ((int)((InputPointerState*)self)->buttons[1].state == kButtonJustReleased) *(WVector*)(&((InputPointerState*)self)->buttons[1].releasePosition) = *(WVector*)(&self->position[0]);
	if ((int)((InputPointerState*)self)->buttons[2].state == kButtonJustPressed) {
		*(WVector*)(&((InputPointerState*)self)->buttons[2].pressPosition) = *(WVector*)(&self->position[0]);
		unsigned int intervalMs = nowMs - ((InputPointerState*)self)->buttons[2].lastPressMs;
		((InputPointerState*)self)->buttons[2].lastPressMs = nowMs;
		((InputPointerState*)self)->buttons[2].pressIntervalMs = intervalMs;
	} else if ((int)((InputPointerState*)self)->buttons[2].state == kButtonJustReleased) *(WVector*)(&((InputPointerState*)self)->buttons[2].releasePosition) = *(WVector*)(&self->position[0]);
}

float s_inputAndNoticeFullTurnAngle;
extern const float kFloat_6_2831855; // 2*pi: shared full-turn constant

// Default direct-input hook returns zero.
int WInputDev_GetState() {
	return 0;
}

// Default direct-input hook returns zero.
int WInputDev_GetEventTime() {
	return 0;
}

// Initialize this unit's cached full-turn angle from the shared constant.
void InputAndNotices_InitFullTurnAngle() {
	s_inputAndNoticeFullTurnAngle = kFloat_6_2831855;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU8046214C", _ctors_u80185E48_0, InputAndNotices_InitFullTurnAngle);
#pragma pop
