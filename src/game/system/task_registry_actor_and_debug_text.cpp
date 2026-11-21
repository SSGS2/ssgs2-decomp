#include <decomp/static_init.h>
#include <decomp/local_class.h>
#include "classes/WViewProjectedPoint.h"
#include "classes/IActor.h"
#include "classes/DebugTextView.h"
#define AsDebugText800CA990(p) ((DebugTextView*)(p))
#include "classes/WView.h"
#define AsWView(p) ((WView*)(p))
// Translation unit 0x800CA914-0x800CBF34 (28 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

// File-relative SPD directory; entries remain packed in the resource buffer.
struct SpdDirectory {
    char fileTag[6];
    unsigned short extraSectionCount;
    char reserved08[4];
    unsigned int spriteEntriesOffset;
    unsigned int squareEntriesOffset;
    unsigned int extraSectionOffset;
    unsigned short spriteCount;
    unsigned short squareCount;
};
enum { kSpdSpriteEntryBytes = 0x2C, kSpdSquareEntryBytes = 0x24 };
struct ActorErrorStringPool { char vectorLengthError[0x14]; char lengthErrorType[0x37]; };
static const int kActorLengthErrorTypeOffset = (int)&((ActorErrorStringPool*)0)->lengthErrorType;
struct DebugSpriteColor { char position[0x10]; unsigned int packedColor; };
// One 0x38-byte reusable glyph vertex, in screen pixels with normalized atlas UVs.
struct DebugGlyphVertex {
    float x, y, z, rhw;
    unsigned int packedColor, reserved14;
    float u, v;
    char reserved20[0x18];
};
// A biased cursor keeps the original +0x30 load displacement without treating
// each iteration as another DebugText owner; only vertex is ever accessed.
struct DebugGlyphCursor { char addressBias[0x30]; DebugGlyphVertex vertex; };
typedef char DebugGlyphVertex_size[(sizeof(DebugGlyphVertex) == 0x38) ? 1 : -1];
struct ActorTimedMessage { char payload[0x1C]; unsigned int expiresAtMs; };
static const int kDepthClipMask = 3;
static const int kDebugTextFormatBytes = 0x400;
static const int kDebugSpriteStride = 0x38;
static const int kSpriteVertexFormat = 0x20;
static const int kTextureIndexMask = 0x7FF;
static const int kDebugVertexListOffset = (int)&((DebugTextView*)0)->vertex0;
static const int kDebugTextRenderFlags = 0x02080000;


extern void WView_ProjectWorldPoint(void*, void*, void*);
extern int String_VFormatBoundedCopy(char*, int, void*, void*);
extern int strchr(void*, int);
extern WView* g_view;
extern float kFloat_0_5_DebugText;

extern void* __as__7WVectorFRC7WVector(void*, const void*);
extern void* Vec3_Zero(void*);
extern void* TaskActorTable_FindByName(void*, void*);
extern void* ResourceManager_LoadBitmap(void*, int, int, int, int);
extern int ResourceManager_GetBitmapWidth(void*, void*);
extern int ResourceManager_GetBitmapHeight(void*, void*);
extern void* CDummy_Create();
extern void ActorFactoryInterface_InitVtable(void*); // construct the base object before IActor fields are installed
extern void dtor_8029EA18(void*, int);
extern void __dl__FPv(void*);
extern void* __nw__FUl(unsigned int);
extern void ResourceManager_ReleaseId(void*, void*);
extern void* TaskRegistry_Dtor(void*, int);
extern void __register_global_object(void*, void*, void*);
extern char __vt__6IActor[16];
extern char __vt__6CDummy[];
extern void* g_resourceManager;
extern signed char s_taskRegistryInitialized;
extern unsigned int s_taskRegistry[7];
extern unsigned int s_taskRegistryDtorRegistration[4];
extern void* g_defaultActorSkipMask;
extern char s_iObjectTypeInfo;
extern float kFloat_6_2831855;
extern float s_debugTextTwoPi; // debug-text unit copy of two pi
extern float s_initializedTwoPi_800CA984;
extern unsigned int g_currentTimeMilliseconds;
extern unsigned int* g_actorSkipMask;
extern int strcmp(const char*, const char*);
extern int memcmp(const char*, const char*, unsigned int);
extern void* ActorFactoryRegistry_GetInstance();
extern void String_ReserveCapacity(void*, unsigned int);
struct CB4C0Alloc { char c; };
extern void String_ReplaceRange(void*, int, int, const char*, const char*, CB4C0Alloc);
extern void appFreeUni__FPv(void*);
extern void* appMallocDummy__FUl(unsigned int);
extern void* __nwa__FUl(unsigned int);
extern void* __dla__FPv(void*);
extern unsigned int strlen(const char*);
extern char* strcpy(char*, const char*);
extern void* memcpy(void*, const void*, unsigned int);
extern void* memset(void*, int, unsigned int);
extern void StringPairMap_DestroySubtree(void*, void*);
extern void StringMap_DestroyStringPair(void*, void*);
extern void* g_lastTaskRegistryFactoryResult;
extern char s_actorTypeInfo;

// Return a reference to the smaller unsigned length.
inline const unsigned int& RegistryMinimum(const unsigned int& a, const unsigned int& b) { return a < b ? a : b; }
// Compare a bounded left prefix with the complete right range, breaking ties by length.
inline int CompareRegistrySubstring(const char* leftData, unsigned int leftLength, unsigned int prefixLimit, const char* rightData, unsigned int rightLength) {
	unsigned int leftPrefixLength = RegistryMinimum(prefixLimit, leftLength);
	unsigned int rightCompareLength = rightLength;
	int ordering = memcmp(leftData, rightData, RegistryMinimum(rightCompareLength, leftPrefixLength));
	if (ordering == 0) {
		if (leftPrefixLength < rightLength) ordering = -1;
		else if (leftPrefixLength > rightLength) ordering = 1;
	}
	return ordering;
}
struct RegistryString;
inline int CompareRegistryQuery(const RegistryString& query, bool queryHeapBacked, unsigned int queryInlineLength, const char* storedData, unsigned int storedLength);
struct RegistryString {
	union { unsigned int word; unsigned char byte; };
	unsigned int sz; char* p;
	// Test the high-bit heap-ownership marker.
	bool isLong() const { return word >> 31; }
	// Return the active small-string length.
	unsigned int size() const { return !isLong() ? (byte & 0x7f) : sz; }
	// Borrow the active small-string characters.
	const char* data() const { return !isLong() ? (const char*)this + 1 : p; }
	// Compare at most prefixLimit stored characters with the external text range.
	int compare(unsigned int prefixLimit, const char* otherData, unsigned int otherLength) const {
		const char* selfData;
		unsigned int selfLength;
		if (!isLong()) { selfData = (const char*)this + 1; selfLength = byte & 0x7f; }
		else { selfData = p; selfLength = sz; }
		return CompareRegistrySubstring(selfData, selfLength, prefixLimit, otherData, otherLength);
	}
	// Compare complete registry names lexicographically.
	int compare(const RegistryString& other) const { return compare(size(), other.data(), other.size()); }
};
// Compare a registry query using its already-decoded representation.
inline int CompareRegistryQuery(const RegistryString& query, bool queryHeapBacked, unsigned int queryInlineLength, const char* storedData, unsigned int storedLength) {
	unsigned int queryPrefixLimit = !queryHeapBacked ? queryInlineLength : query.sz;
	const char* queryData;
	unsigned int queryLength;
	if (!queryHeapBacked) { queryData = (const char*)&query + 1; queryLength = queryInlineLength; }
	else { queryData = query.p; queryLength = query.sz; }
	return CompareRegistrySubstring(queryData, queryLength, queryPrefixLimit, storedData, storedLength);
}
struct FactoryMapNode { FactoryMapNode* left; FactoryMapNode* right; int color; /* +8 is the tree link-position word. */ RegistryString key; void* (*value)(); };
struct FactoryMapView { char pad[0x10]; FactoryMapNode header; };
struct ActorTypeInfo { void* name; ActorTypeInfo* base; };
struct ActorTypeDispatch { 
virtual ActorTypeInfo* type();
};

extern char __vt__Q23std11logic_error[], s_sharedCharDeleterVtable[], __vt__Q23std12length_error[], actorAndBallErrorStrings[];
extern void __dt__Q23std12length_errorFv(void*, int);
extern "C" void __dt__Q23std9exceptionFv(void*, int);
extern void __throw(const char*, void*, void*) __attribute__((noreturn));

struct CB800BufferGuard {
	char* data; char* allocator;
	// Free allocated exception text unless ownership was transferred.
	~CB800BufferGuard() { if (data) __dla__FPv(data); }
};
struct CB800Ref {
	void* table; unsigned int strong, weak; char* data;
	// Initialize the shared exception-text control block.
	CB800Ref(char* p) { strong = 1; weak = 1; table = s_sharedCharDeleterVtable; data = p; }
};
struct CB800ExceptionBase {
	void* table;
	// Initialize the standard logic-error base identity.
	CB800ExceptionBase() { table = __vt__Q23std11logic_error; }
	// Destroy the standard exception base without freeing this object.
	~CB800ExceptionBase() { __dt__Q23std9exceptionFv(this, 0); }
};
struct CB800ErrorBase : CB800ExceptionBase {
	char* data; CB800Ref* ref;
	// Copy exception text and transfer it into shared ownership.
	CB800ErrorBase(const char* text) {
		union { char byte; unsigned int alignment; } allocator;
		allocator.byte = 0;
		unsigned int n = strlen(text) + 1;
		char* allocated = (char*)__nwa__FUl((n << 1) - n);
		data = allocated;
		{
			CB800BufferGuard guard = { allocated, &allocator.byte };
			ref = new CB800Ref(allocated);
			guard.data = 0;
		}
		strcpy(data, text);
	}
};
struct CB800Error : CB800ErrorBase {
	// Construct the length-error identity around copied text.
	CB800Error(const char* const& text) : CB800ErrorBase(text) { table = __vt__Q23std12length_error; }
};
struct CB800Grow {
	void** data; unsigned int size, capacity; unsigned int* allocator; unsigned int offset;
	// Initialize an empty pointer-buffer growth guard.
	CB800Grow(unsigned int* a) : data(0), size(0), capacity(0), allocator(a), offset(0) {}
	// Clear logical pointer count and free only the owned buffer.
	~CB800Grow() { if (data) { size = 0; appFreeUni__FPv(data); } }
};
// Return a reference to the larger unsigned capacity requirement.
inline const unsigned int& CB800Max(const unsigned int& a, const unsigned int& b) { return a < b ? b : a; }

struct TaskRegistryNameVectorView { char tree[0x10]; void* names; unsigned int size, capacity; };

// Append a registered actor/factory name, growing the owned pointer vector when capacity is exhausted.
void TaskRegistry_AppendFactory(void* self, void* value)
{
	unsigned int* v = (unsigned int*)&((TaskRegistryNameVectorView*)self)->names;
	unsigned int initialCapacity;
	unsigned int initialSize;
	initialSize = *(volatile unsigned int*)(v + 1);
	initialCapacity = *(volatile unsigned int*)(v + 2);
	if (initialSize < initialCapacity) {
		unsigned int next = initialSize + 1;
		v[1] = next;
		((void**)v[0])[next - 1] = value;
		return;
	}
	unsigned int max = 0x3fffffff;
	if (max - initialCapacity < 1) {
		CB800Error error(actorAndBallErrorStrings);
		__throw(actorAndBallErrorStrings + kActorLengthErrorTypeOffset, &error, (void*)__dt__Q23std12length_errorFv);
	}
	unsigned int growthThreeFifths;
	unsigned int growthHalf;
	unsigned int minimum = v[1] + 1 - v[2];
	CB800Grow fresh(v + 2);
	unsigned int capacity = v[2];
	if (minimum > max - capacity) {
		CB800Error error(actorAndBallErrorStrings);
		__throw(actorAndBallErrorStrings + kActorLengthErrorTypeOffset, &error, (void*)__dt__Q23std12length_errorFv);
	}
	unsigned int newCapacity;
	if (capacity < 0x15555555) {
		growthThreeFifths = ((capacity + 1) * 3) / 5;
		newCapacity = capacity + CB800Max(growthThreeFifths, minimum);
	} else if (capacity < 0x2aaaaaaa) {
		growthHalf = (capacity + 1) / 2;
		newCapacity = capacity + CB800Max(growthHalf, minimum);
	} else newCapacity = max;
	if (newCapacity > max) {
		CB800Error error(actorAndBallErrorStrings);
		__throw(actorAndBallErrorStrings + kActorLengthErrorTypeOffset, &error, (void*)__dt__Q23std12length_errorFv);
	}
	fresh.data = (void**)appMallocDummy__FUl(newCapacity * 4);
	fresh.offset = v[1];
	(fresh.data + fresh.offset)[fresh.size] = value;
	fresh.capacity = newCapacity;
	++fresh.size;
	void** first = (void**)v[0];
	int count = ((char*)(first + v[1]) - (char*)first) / 4;
	fresh.offset -= count;
	memcpy(fresh.data + fresh.offset, first, count * 4);
	memset(first, 0, count * 4);
	fresh.size += count;
	v[1] = 0;
	void** old = (void**)v[0]; void** replacement = fresh.data; fresh.data = old;
	unsigned int oldCapacity = v[2]; v[2] = fresh.capacity; fresh.capacity = oldCapacity;
	v[0] = (unsigned int)replacement;
	v[1] = fresh.size; fresh.size = 0;
}

// Original 0x800CB4C0: the inline type walk returns at the first matching base.
static inline bool HasActorBaseType(ActorTypeInfo* type) {
    for (; type; type = type->base)
    if (type == (ActorTypeInfo*)&s_actorTypeInfo) return true;
    return false;
}

// Find a registered actor name, invoke its factory, and return the created actor only if its RTTI derives from IActor.
void* TaskRegistry_CreateActorByName(void* self, const char* key)
{
    // Phase 1: restrict factory lookup to names registered for this task table.
    TaskRegistryNameVectorView* registry = (TaskRegistryNameVectorView*)self;
    void** registeredName = (void**)registry->names;
    void** namesEnd = registeredName + registry->size;
    for (; registeredName != namesEnd; ++registeredName) {
        const char* name = (const char*)*registeredName;
        if (strcmp(name, key) != 0)
            continue;
        // Phase 2: build the owned comparison key and find its lower bound.
        FactoryMapView* tree = (FactoryMapView*)ActorFactoryRegistry_GetInstance();
        RegistryString parsed;
        CB4C0Alloc alloc;
        parsed.word = 0; parsed.sz = 0; parsed.p = 0;
        unsigned int nameLength = strlen(name);
        String_ReserveCapacity(&parsed, nameLength);
        String_ReplaceRange(&parsed, 0, 0, name, name + nameLength, alloc);
        FactoryMapNode* result = &tree->header;
        FactoryMapNode* node = tree->header.left;
        unsigned int longFlag = parsed.word >> 31;
        bool queryIsInline = longFlag == 0;
        bool queryIsHeapBacked = queryIsInline == 0;
        unsigned int inlineLength = parsed.byte & 0x7f;
        char* heapData = parsed.p;
        unsigned int heapLength = parsed.sz;
        while (node) {
            unsigned int queryLength;
            const char* queryData;
            if (!queryIsHeapBacked) { queryLength = inlineLength; queryData = (const char*)&parsed + 1; }
            else { queryData = heapData; queryLength = heapLength; }
            if (node->key.compare(node->key.size(), queryData, queryLength) >= 0) { result = node; node = node->left; }
            else node = node->right;
        }
        // Reject a non-equal lower bound before invoking any factory.
        if (result == &tree->header
            || CompareRegistryQuery(parsed, queryIsHeapBacked, inlineLength, result->key.data(), result->key.size()) < 0)
            result = &tree->header;
        // Phase 3: free the key before invoking the factory, then require IActor ancestry.
        ActorTypeDispatch* object;
        if (longFlag) appFreeUni__FPv(parsed.p);
        object = (ActorTypeDispatch*)(result != &tree->header ? result->value() : 0);
        g_lastTaskRegistryFactoryResult = object;
        ActorTypeDispatch* actorResult;
        if (object) {
            bool found;
            ActorTypeInfo* objectType = object->type();
            found = HasActorBaseType(objectType);
            if (found) actorResult = object;
            else actorResult = 0;
        } else actorResult = 0;
        return actorResult;
    }
    return 0;
}

// Registry owns each node and both small strings. Child pointers are tree links;
// strings[0..1] are the two owned payloads, not links. Free their heap buffers before
// freeing the node. A small string's high bit selects heap storage; inline bytes
// belong to the node and must not be freed separately.
struct RegistryStringPairNode {
    RegistryStringPairNode* child[2];
    unsigned int parentAndColor;
    RegistryString strings[2];
};
struct TaskRegistryStorage {
    unsigned int size;
    RegistryStringPairNode* root;
    void* sentinel;
    void* firstNode;
    void* actorNames;
    unsigned int actorNameCount;
    unsigned int actorNameCapacity;
};

// Destroy both child subtrees, then both string payloads and this node's storage.
static inline void DestroyRegistryPairBranch(void* registry, RegistryStringPairNode* node) {
    if (node) {
        if (node->child[0]) StringPairMap_DestroySubtree(registry, node->child[0]);
        if (node->child[1]) StringPairMap_DestroySubtree(registry, node->child[1]);
        StringMap_DestroyStringPair(registry, &node->strings[0]);
        appFreeUni__FPv(node);
    }
}

// Free the task registry name vector and both strings in each tree node; optionally free the registry.
void* TaskRegistry_Dtor(void* value, int deallocate)
{
    // Phases: free registered-name pointer storage, destroy the string-pair
    // tree in postorder, then optionally delete the registry object.
    // The tree part below is the compiler-matched unrolling of one plain-C recursion:
    //   void destroy(node) { if (!node) return; destroy(node->child[0]); destroy(node->child[1]);
    //                        free both heap strings of node; free(node); }
    // It is written out for the root plus three levels (a0/a1, b0/b1, c0/c1); the
    // deepest level calls DestroyRegistryPairBranch, which recurses out of line.
    // Skip to "if (value && deallocate > 0)" to read the rest.
	if (value) {
		void* p = ((TaskRegistryStorage*)value)->actorNames;
		if (p) {
			unsigned int tmp = ((TaskRegistryStorage*)value)->actorNameCount;
			((TaskRegistryStorage*)value)->actorNameCount = tmp - tmp;
			appFreeUni__FPv(p);
		}
		RegistryStringPairNode* root = ((TaskRegistryStorage*)value)->root;
		if (root) {
			RegistryStringPairNode* a0 = root->child[0];
			if (a0) {
				RegistryStringPairNode* b0 = a0->child[0];
				if (b0) {
					RegistryStringPairNode* c0 = b0->child[0];
					DestroyRegistryPairBranch(value, c0);
					RegistryStringPairNode* c1 = b0->child[1];
					DestroyRegistryPairBranch(value, c1);
					StringMap_DestroyStringPair(value, &b0->strings[0]);
					appFreeUni__FPv(b0);
				}
				RegistryStringPairNode* b1 = a0->child[1];
				if (b1) {
					RegistryStringPairNode* c0 = b1->child[0];
					DestroyRegistryPairBranch(value, c0);
					RegistryStringPairNode* c1 = b1->child[1];
					DestroyRegistryPairBranch(value, c1);
					StringMap_DestroyStringPair(value, &b1->strings[0]);
					appFreeUni__FPv(b1);
				}
				StringMap_DestroyStringPair(value, &a0->strings[0]);
				appFreeUni__FPv(a0);
			}
			RegistryStringPairNode* a1 = root->child[1];
			if (a1) {
				RegistryStringPairNode* b0 = a1->child[0];
				if (b0) {
					RegistryStringPairNode* c0 = b0->child[0];
					DestroyRegistryPairBranch(value, c0);
					RegistryStringPairNode* c1 = b0->child[1];
					DestroyRegistryPairBranch(value, c1);
					StringMap_DestroyStringPair(value, &b0->strings[0]);
					appFreeUni__FPv(b0);
				}
				RegistryStringPairNode* b1 = a1->child[1];
				if (b1) {
					RegistryStringPairNode* c0 = b1->child[0];
					DestroyRegistryPairBranch(value, c0);
					RegistryStringPairNode* c1 = b1->child[1];
					DestroyRegistryPairBranch(value, c1);
					StringMap_DestroyStringPair(value, &b1->strings[0]);
					appFreeUni__FPv(b1);
				}
				StringMap_DestroyStringPair(value, &a1->strings[0]);
				appFreeUni__FPv(a1);
			}
			if (root->strings[1].word >> 31) appFreeUni__FPv(root->strings[1].p);
			if (root->strings[0].word >> 31) appFreeUni__FPv(root->strings[0].p);
			appFreeUni__FPv(root);
		}
	}
	if (value && deallocate > 0) __dl__FPv(value);
	return value;
}

// Return a square record by index, or null when the SPD directory has no such record.
void* SpdResource_GetSquareEntry(void* self, unsigned int index)
{
	if (index >= ((SpdDirectory*)self)->squareCount)
		return 0;
	return (char*)self + ((SpdDirectory*)self)->squareEntriesOffset + index * kSpdSquareEntryBytes;
}

// Return a sprite record by index, or null when the SPD directory has no such record.
void* SpdResource_GetSpriteEntry(void* self, unsigned int index)
{
	if (index >= ((SpdDirectory*)self)->spriteCount)
		return 0;
	return (char*)self + ((SpdDirectory*)self)->spriteEntriesOffset + index * kSpdSpriteEntryBytes;
}

// Return the SPD optional section, or null when its entry count is zero.
void* SpdResource_GetOptionalSection(void* self)
{
	if (((SpdDirectory*)self)->extraSectionCount == 0)
		return 0;
	return (char*)self + ((SpdDirectory*)self)->extraSectionOffset;
}

// Write the same packed color bits to each of the four reusable glyph vertices.
// Integer register payload, not an address; spot_marker_draw_and_messages.cpp passes a packed unsigned color.
void DebugText_SetColor(void* self, unsigned int packedColor)
{
	((DebugSpriteColor*)AsDebugText800CA990(self)->vertex0)->packedColor = packedColor;
	((DebugSpriteColor*)AsDebugText800CA990(self)->vertex1)->packedColor = packedColor;
	((DebugSpriteColor*)AsDebugText800CA990(self)->vertex2)->packedColor = packedColor;
	((DebugSpriteColor*)AsDebugText800CA990(self)->vertex3)->packedColor = packedColor;
}

// Forward a timed message to virtual slot 10 while its expiration time is still in the future.
void IActor_HandleTimedMsg(void* self, void* arg)
{
	if (((ActorTimedMessage*)arg)->expiresAtMs > g_currentTimeMilliseconds) {
	struct Dispatch {
		virtual void pad0() DECOMP_LOCAL_VIRTUAL; virtual void pad1() DECOMP_LOCAL_VIRTUAL; virtual void pad2() DECOMP_LOCAL_VIRTUAL; virtual void pad3() DECOMP_LOCAL_VIRTUAL;
		virtual void pad4() DECOMP_LOCAL_VIRTUAL;
		virtual void pad5() DECOMP_LOCAL_VIRTUAL; virtual void pad6() DECOMP_LOCAL_VIRTUAL; virtual void pad7() DECOMP_LOCAL_VIRTUAL; virtual void pad8() DECOMP_LOCAL_VIRTUAL;
		virtual void pad9() DECOMP_LOCAL_VIRTUAL;
		virtual void call(void*) DECOMP_LOCAL_VIRTUAL;
	};
	((Dispatch*)self)->call(arg);
	}
}

// Initialize the debug-text unit copy of two pi during static startup.
void DebugText_InitStaticConstants()
{
	s_debugTextTwoPi = kFloat_6_2831855;
}

#pragma push
#pragma inline_depth(16)
#pragma inline_max_size(10000)
#pragma inline_max_total_size(10000)
// Clip one reusable glyph vertex in viewport pixels and adjust its normalized atlas UVs.
// Keep pixel/UV operations in their original order; vx/vy are caller-owned scratch values.
static inline void ClipDebugGlyphToViewport(char*& self, DebugGlyphCursor*& glyphCursor, float& half, float& vx, float& vy) {
    vx = glyphCursor->vertex.x;
    if (vx < half) {
        glyphCursor->vertex.u += AsDebugText800CA990(self)->glyphUSize * (half - vx) / (float)AsDebugText800CA990(self)->glyphWidth;
        glyphCursor->vertex.x = half;
    } else if (vx > AsWView(g_view)->viewportWidth - half) {
        glyphCursor->vertex.u -= AsDebugText800CA990(self)->glyphUSize * (half + (vx - AsWView(g_view)->viewportWidth)) / (float)AsDebugText800CA990(self)->glyphWidth;
        glyphCursor->vertex.x = AsWView(g_view)->viewportWidth - half;
    }
    vy = glyphCursor->vertex.y;
    if (vy < half) {
        glyphCursor->vertex.v += AsDebugText800CA990(self)->glyphVSize * (half - vy) / (float)AsDebugText800CA990(self)->glyphHeight;
        glyphCursor->vertex.y = half;
    } else if (vy > AsWView(g_view)->viewportHeight - half) {
        glyphCursor->vertex.v -= AsDebugText800CA990(self)->glyphVSize * (half + (vy - AsWView(g_view)->viewportHeight)) / (float)AsDebugText800CA990(self)->glyphHeight;
        glyphCursor->vertex.y = AsWView(g_view)->viewportHeight - half;
    }
}

// Project the anchor, format centered atlas text, and clip each glyph to the viewport.
// spacing and glyph sizes are pixels; UVs are normalized atlas coordinates.
// The character map must contain every formatted character: no missing-glyph fallback exists.
void DebugText_DrawFormattedAtWorldPosition(void* self_, void* screenObject, int spacing, const char* format, ...)
{
	char* self = (char*)self_;
	if (AsDebugText800CA990(self)->atlasColumns == 0)
		return;
	char measured[sizeof(WViewProjectedPoint)];
	WView_ProjectWorldPoint(screenObject, measured, ((char*)AsDebugText800CA990(self)->worldPosition));
	if ((((WViewProjectedPoint*)measured)->clipFlags & kDepthClipMask) != 0)
		return;
	AsDebugText800CA990(self)->vertex0Z = ((WViewProjectedPoint*)measured)->screenZ;
	AsDebugText800CA990(self)->vertex0Rhw = ((WViewProjectedPoint*)measured)->rhw;
	AsDebugText800CA990(self)->vertex1Z = ((WViewProjectedPoint*)measured)->screenZ;
	AsDebugText800CA990(self)->vertex1Rhw = ((WViewProjectedPoint*)measured)->rhw;
	AsDebugText800CA990(self)->vertex2Z = ((WViewProjectedPoint*)measured)->screenZ;
	AsDebugText800CA990(self)->vertex2Rhw = ((WViewProjectedPoint*)measured)->rhw;
	AsDebugText800CA990(self)->vertex3Z = ((WViewProjectedPoint*)measured)->screenZ;
	AsDebugText800CA990(self)->vertex3Rhw = ((WViewProjectedPoint*)measured)->rhw;
	// Phase 1: project once for shared Z/RHW, then format the bounded text buffer.
	char text[kDebugTextFormatBytes];
	struct { unsigned int a; void* b; void* c; } ap;
	__va_setup(&ap);
	if (String_VFormatBoundedCopy(text, (kDebugTextFormatBytes - 1), (void*)format, &ap) == -1)
		text[(kDebugTextFormatBytes - 1)] = 0;
	int textLength = strlen(text);
	int interGlyphSpacing;
	int i;
	const char* characterCursor;
	int glyphMapAddress;
	glyphMapAddress = AsDebugText800CA990(self)->glyphMapAddress;
	characterCursor = text;
	if (glyphMapAddress == 0)
		return;
	float half = kFloat_0_5_DebugText;
	interGlyphSpacing = spacing * (textLength - 1);
	// Phase 2: find each atlas cell and center the whole string around the projected anchor.
	// glyphAddress/glyphMapAddress are 32-bit addresses; subtraction yields the cell index.
	for (i = 0; i < textLength; ++i) {
		int glyphAddress = strchr((void*)glyphMapAddress, (signed char)*characterCursor++);
		int glyphIndex = glyphAddress - glyphMapAddress;
		float uvTop = AsDebugText800CA990(self)->glyphVSize * (float)(glyphIndex / AsDebugText800CA990(self)->atlasColumns);
		float uvBottom = uvTop + AsDebugText800CA990(self)->glyphVSize;
		AsDebugText800CA990(self)->vertex1V = uvTop;
		AsDebugText800CA990(self)->vertex2V = uvTop;
		float uvLeft = AsDebugText800CA990(self)->glyphUSize * (float)(glyphIndex % AsDebugText800CA990(self)->atlasColumns);
		float uvRight = uvLeft + AsDebugText800CA990(self)->glyphUSize;
		AsDebugText800CA990(self)->vertex0U = uvLeft;
		int textWidthPixels = AsDebugText800CA990(self)->glyphWidth * (int)textLength + interGlyphSpacing;
		int glyphOffsetPixels = i * (AsDebugText800CA990(self)->glyphWidth + spacing);
		float glyphTop = AsDebugText800CA990(self)->offsetY - half * AsDebugText800CA990(self)->glyphHeight;
		float centeredTextLeft = AsDebugText800CA990(self)->offsetX - half * (float)textWidthPixels;
		float glyphLeft = (float)glyphOffsetPixels + centeredTextLeft;
		float glyphRight = glyphLeft + AsDebugText800CA990(self)->glyphWidth;
		float glyphBottom = glyphTop + AsDebugText800CA990(self)->glyphHeight;
		AsDebugText800CA990(self)->vertex0V = uvBottom;
		AsDebugText800CA990(self)->vertex1U = uvLeft;
		AsDebugText800CA990(self)->vertex2U = uvRight;
		AsDebugText800CA990(self)->vertex3V = uvBottom;
		AsDebugText800CA990(self)->vertex3U = uvRight;
		if (!(AsWView(g_view)->viewportWidth - half < ((WViewProjectedPoint*)measured)->screenX + glyphLeft)
			&& !(((WViewProjectedPoint*)measured)->screenX + glyphRight < half)
			&& !(AsWView(g_view)->viewportHeight - half < ((WViewProjectedPoint*)measured)->screenY + glyphTop)
			&& !(((WViewProjectedPoint*)measured)->screenY + glyphBottom < half)) {
			AsDebugText800CA990(self)->vertex0X = ((WViewProjectedPoint*)measured)->screenX + glyphLeft;
			AsDebugText800CA990(self)->vertex0Y = ((WViewProjectedPoint*)measured)->screenY + glyphBottom;
			AsDebugText800CA990(self)->vertex1X = ((WViewProjectedPoint*)measured)->screenX + glyphLeft;
			AsDebugText800CA990(self)->vertex1Y = ((WViewProjectedPoint*)measured)->screenY + glyphTop;
			AsDebugText800CA990(self)->vertex2X = ((WViewProjectedPoint*)measured)->screenX + glyphRight;
			AsDebugText800CA990(self)->vertex2Y = ((WViewProjectedPoint*)measured)->screenY + glyphTop;
			AsDebugText800CA990(self)->vertex3X = ((WViewProjectedPoint*)measured)->screenX + glyphRight;
			AsDebugText800CA990(self)->vertex3Y = ((WViewProjectedPoint*)measured)->screenY + glyphBottom;
			// Phase 3: clip all four corners and move UVs proportionally to the clipped pixels.
			// Clip each typed vertex directly; the draw pointer list remains in the owner.
			DebugGlyphCursor* glyphCursor = (DebugGlyphCursor*)self;
			for (int vertexIndex = 0; vertexIndex < 4; ++vertexIndex, glyphCursor = (DebugGlyphCursor*)((char*)glyphCursor + kDebugSpriteStride)) {
				float vx, vy;
				ClipDebugGlyphToViewport(self, glyphCursor, half, vx, vy);
			}
			struct Dispatch {
				virtual void pad0() DECOMP_LOCAL_VIRTUAL; virtual void pad1() DECOMP_LOCAL_VIRTUAL; virtual void pad2() DECOMP_LOCAL_VIRTUAL; virtual void pad3() DECOMP_LOCAL_VIRTUAL;
				virtual void call(void*, int, unsigned int, int) DECOMP_LOCAL_VIRTUAL;
			};
			((Dispatch*)screenObject)->call(self + kDebugVertexListOffset, kSpriteVertexFormat, (*(unsigned int*)self & kTextureIndexMask) | kDebugTextRenderFlags, 0);
		}
	}
}

#pragma pop
// Add actor-specific bits to the global skip-mask test.
void IActor_AddSkipFlags(void* self, unsigned int flags)
{
	((IActor*)self)->skipMask |= flags;
}

// Remove actor-specific bits from the skip-mask test.
void IActor_ClearSkipFlags(void* self, unsigned int flags)
{
	((IActor*)self)->skipMask &= ~flags;
}

// Default actor restore hook; subclasses may override it.
void IActor_OnRestorePreserved() {}

// Return whether any globally selected skip bit intersects this actor.
int IActor_Skipped(void* self)
{
	if (g_actorSkipMask != 0 && (*g_actorSkipMask & ((IActor*)self)->skipMask) != 0)
		return 1;
	return 0;
}

// Borrow the actor dispatch table without changing ownership.
void* IActor_GetDispatchObject(void* self)
{
	return ((IActor*)self)->dispatchObject;
}

// Find a borrowed named target in the actor dispatch table.
void* IActor_FindDispatchTargetByName(void* self, void* key)
{
	return TaskActorTable_FindByName(((IActor*)self)->dispatchObject, key);
}

// Initialize the actor unit copy of two pi during static startup.
void IActor_InitStaticConstants()
{
	s_initializedTwoPi_800CA984 = kFloat_6_2831855;
}

// Release a resource-manager ID and optionally free its owning handle.
void* ResourceHandle_Dtor(void* self, int deleting)
{
	if (self != 0) {
		if (*(void**)self != 0) {
			ResourceManager_ReleaseId(g_resourceManager, *(void**)self);
			*(void**)self = 0;
		}
		if (deleting > 0)
			__dl__FPv(self);
	}
	return self;
}

// Allocate a default actor with display priority two and the global skip mask.
void* IActor_Create()
{
	struct Object {
		void* vtable;
		// Persistent IActor layout:
        // a +04: cleared, role unknown; displayPriority +08 starts at 2;
        // skipMask +0C and dispatchObjectAddress +10 match classes/IActor.h;
        // e +14: cleared, role unknown. See classes/IActor.h consumers.
        unsigned int a, displayPriority, skipMask, dispatchObjectAddress, e;
		// Initialize the actor base fields and install this factory's final vtable.
		Object() {
			ActorFactoryInterface_InitVtable(this);
			vtable = __vt__6IActor;
			a = 0;
			displayPriority = 2;
			skipMask = 0;
			dispatchObjectAddress = 0;
			e = 0;
			skipMask = (unsigned int)g_defaultActorSkipMask;
		}
	};
	return new Object();
}

// Allocate a dummy actor with the same initial state as IActor.
void* CDummy_Create()
{
	struct Object {
        void* vtable;
        // Persistent IActor layout:
        // a +04: cleared, role unknown; displayPriority +08 starts at 2;
        // skipMask +0C and dispatchObjectAddress +10 match classes/IActor.h;
        // e +14: cleared, role unknown. See classes/IActor.h consumers.
        unsigned int a, displayPriority, skipMask, dispatchObjectAddress, e;
		// Initialize the actor base fields and install this factory's final vtable.
		Object() {
			ActorFactoryInterface_InitVtable(this);
			vtable = __vt__6IActor;
			a = 0;
			displayPriority = 2;
			skipMask = 0;
			dispatchObjectAddress = 0;
			e = 0;
			skipMask = (unsigned int)g_defaultActorSkipMask;
			vtable = __vt__6CDummy;
		}
	};
	return new Object();
}

// Message ABI words: [0]=messageSelector, [1..4]=payloadWord1..4, [7]=payloadWord7.
// Payload types depend on the selector; this dispatcher does not interpret them.
// Words 5/6 are uninitialized and must not be read.
// key is a borrowed registry name; no key means broadcast. Receivers consume the
// stack packet synchronously (target slot 9 by reference, table slot 15 by value).
struct Args800CBDB4 { void* w[8]; };

// Pack message arguments and dispatch to a named target or broadcast through the actor table.
// Always returns zero, including a missing named target; this is not a delivery count.
int IActor_DispatchMessage(void* self, void* messageSelector, void* key, void* payloadWord1, void* payloadWord2, void* payloadWord3, void* payloadWord4, void* payloadWord7)
{
	Args800CBDB4 packet;
	// Slots 5 and 6 retain the original ABI treatment; only these six slots are populated.
	packet.w[1] = payloadWord1; packet.w[0] = messageSelector; packet.w[2] = payloadWord2; packet.w[3] = payloadWord3; packet.w[4] = payloadWord4; packet.w[7] = payloadWord7;
	if (key != 0) {
		void* target = TaskActorTable_FindByName(((IActor*)self)->dispatchObject, key);
		if (target != 0) {
			struct Dispatch { 
			virtual void pad0() DECOMP_LOCAL_VIRTUAL; virtual void pad1() DECOMP_LOCAL_VIRTUAL; virtual void pad2() DECOMP_LOCAL_VIRTUAL; virtual void pad3() DECOMP_LOCAL_VIRTUAL; virtual void pad4() DECOMP_LOCAL_VIRTUAL; virtual void pad5() DECOMP_LOCAL_VIRTUAL; virtual void pad6() DECOMP_LOCAL_VIRTUAL; virtual void pad7() DECOMP_LOCAL_VIRTUAL; virtual void pad8() DECOMP_LOCAL_VIRTUAL; virtual void pad9(Args800CBDB4&) DECOMP_LOCAL_VIRTUAL;
			};
			((Dispatch*)target)->pad9(packet);
		}
	} else {
		struct Dispatch { 
		virtual void pad0() DECOMP_LOCAL_VIRTUAL; virtual void pad1() DECOMP_LOCAL_VIRTUAL; virtual void pad2() DECOMP_LOCAL_VIRTUAL; virtual void pad3() DECOMP_LOCAL_VIRTUAL; virtual void pad4() DECOMP_LOCAL_VIRTUAL; virtual void pad5() DECOMP_LOCAL_VIRTUAL; virtual void pad6() DECOMP_LOCAL_VIRTUAL; virtual void pad7() DECOMP_LOCAL_VIRTUAL; virtual void pad8() DECOMP_LOCAL_VIRTUAL; virtual void pad9() DECOMP_LOCAL_VIRTUAL; virtual void pad10() DECOMP_LOCAL_VIRTUAL; virtual void pad11() DECOMP_LOCAL_VIRTUAL; virtual void pad12() DECOMP_LOCAL_VIRTUAL; virtual void pad13() DECOMP_LOCAL_VIRTUAL; virtual void pad14() DECOMP_LOCAL_VIRTUAL; virtual void pad15(Args800CBDB4) DECOMP_LOCAL_VIRTUAL;
		};
		((Dispatch*)((IActor*)self)->dispatchObject)->pad15(packet);
	}
	return 0;
}

// Load the bitmap and set glyph dimensions, UV steps, character map, and anchor offsets.
// Caller must supply nonzero glyph dimensions and a loadable bitmap with nonzero width/height;
// no failure checks precede the divisions. characterMapAddress is borrowed for subsequent draws.
// The first word is overwritten without releasing an old atlas handle: arrange its release before reload.
void DebugText_LoadGlyphAtlas(void* self, void* bitmapPath, float offsetX, float offsetY, int glyphWidth, int glyphHeight, unsigned int characterMapAddress)
{
	unsigned int* atlasBitmap = (unsigned int*)((char*)self); // first word of the object holds the loaded bitmap handle
	*atlasBitmap = (unsigned int)ResourceManager_LoadBitmap(g_resourceManager, (int)bitmapPath, 0, 0, 0);
	int bitmapWidth = (int)ResourceManager_GetBitmapWidth(g_resourceManager, (void*)*atlasBitmap);
	int bitmapHeight = (int)ResourceManager_GetBitmapHeight(g_resourceManager, (void*)*atlasBitmap);
	AsDebugText800CA990(self)->atlasColumns = (unsigned int)(bitmapWidth / glyphWidth);
	AsDebugText800CA990(self)->glyphMapAddress = characterMapAddress;
	AsDebugText800CA990(self)->offsetX = offsetX;
	AsDebugText800CA990(self)->offsetY = offsetY;
	AsDebugText800CA990(self)->glyphWidth = (unsigned int)glyphWidth;
	AsDebugText800CA990(self)->glyphHeight = (unsigned int)glyphHeight;
	AsDebugText800CA990(self)->glyphUSize = (float)glyphWidth / (float)bitmapWidth;
	AsDebugText800CA990(self)->glyphVSize = (float)glyphHeight / (float)bitmapHeight;
}

// Initialize and register destruction of the task registry on first access.
void* TaskRegistry_GetInstance()
{
	if (s_taskRegistryInitialized == 0) {
		// s_taskRegistry is laid out as TaskRegistryStorage (7 words): empty tree, empty name vector.
		unsigned int* words = s_taskRegistry;
		words[0] = 0;                          // size
		words[1] = 0;                          // root
		words[3] = (unsigned int)(words + 1);  // firstNode points back at the root slot (empty tree)
		words[4] = 0;                          // actorNames (name vector data)
		words[5] = 0;                          // actorNameCount
		words[6] = 0;                          // actorNameCapacity
		__register_global_object(words, (void*)TaskRegistry_Dtor, s_taskRegistryDtorRegistration);
		s_taskRegistryInitialized = 1;
	}
	return s_taskRegistry;
}

// Copy the world-space text anchor; return the vector assignment result.
void* DebugText_SetWorldPosition(void* self, const void* source)
{
	return __as__7WVectorFRC7WVector(((char*)AsDebugText800CA990(self)->worldPosition), source);
}

// Return the existing base object type descriptor.
void* CTaskDoc_GetRTTI()
{
	return &s_iObjectTypeInfo;
}

// Clear the atlas, zero the world anchor, and link four embedded vertices into a terminated list.
void* DebugText_Init(void* self)
{
	*(void**)self = 0;
	Vec3_Zero(((char*)AsDebugText800CA990(self)->worldPosition));
	char* base = (char*)&AsDebugText800CA990(self)->vertex0X;
	AsDebugText800CA990(self)->vertexListTerminator = 0;
	AsDebugText800CA990(self)->vertex0 = base;
	AsDebugText800CA990(self)->vertex1 = base + kDebugSpriteStride;
	AsDebugText800CA990(self)->vertex2 = base + 2 * kDebugSpriteStride;
	AsDebugText800CA990(self)->vertex3 = base + 3 * kDebugSpriteStride;
	return self;
}

// Initialize the native actor base with priority two and the default skip mask.
void* __ct__18NativeBase80201D18Fv(void* self)
{
	ActorFactoryInterface_InitVtable(self);
	*(void**)self = &__vt__6IActor;
	((IActor*)self)->unk_4 = 0;
	((IActor*)self)->displayPriority = 2;
	((IActor*)self)->skipMask = 0;
	(*(unsigned int*)&((IActor*)self)->dispatchObject) = 0;
	((IActor*)self)->unk_14 = 0; // Constructor-only actor word; its runtime meaning remains unresolved.
	((IActor*)self)->skipMask = (unsigned int)g_defaultActorSkipMask;
	return self;
}

// Destroy the native actor base and optionally release its allocation.
void* __dt__18NativeBase80201D18Fv(void* self, int deleting)
{
	if (self != 0) {
		dtor_8029EA18(self, 0);
		if (deleting > 0)
			__dl__FPv(self);
	}
	return self;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804620BC", _ctors_u800CA914_0, IActor_InitStaticConstants);
STATIC_INIT_ENTRY(".CRT$XCU804620BC", _ctors_u800CA914_1, DebugText_InitStaticConstants);
#pragma pop
