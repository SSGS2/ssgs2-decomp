#include <decomp/static_init.h>
#define XML_MEMBER_OFFSET(type, member) ((int)&((type*)0)->member)
#include "classes/TiXmlNodeLayout.h"
// Translation unit 0x802A3004-0x802A45E4 (26 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#pragma use_lmw_stmw on

struct TiXmlString {
    char* str;
    int capacity;
    TiXmlString(const char*);
    ~TiXmlString();
};
extern "C" {

extern const float kFloat_6_2831855;
extern float s_xmlNodeTwoPi;
extern float s_xmlElementTwoPi;

extern char __vt__9TiXmlNode[];
extern char __vt__12TiXmlElement[];
extern char __vt__14TiXmlAttribute[];
extern char xmlEntityTableStorage[];
extern char tinyXmlErrorMessages[];
static char s_tinyXmlEmptyString[2] = "";
extern char tinyXmlWriterStrings[];
struct XmlWriterStrings {
    char encodedByteFormat[9];
    char empty[1];
    char padding_A[3];
    char space[2];
    char openTagFormat[4];
    char emptyTagClose[4];
    char tagClose[2];
    char endTagFormat[6];
    char newline[2];
    char tagOpen[2];
    char endTagOpen[3];
};
#define XmlWriterStringsView (*( XmlWriterStrings*)tinyXmlWriterStrings)

// Five 12-byte entity records: spelling, byte length, character, padding.
struct XmlEntity { const char* spelling; unsigned length; char character; char padding[3]; };
#define XmlEntities ((XmlEntity*)xmlEntityTableStorage)
enum XmlEntityIndex { kEntityAmpersand, kEntityLessThan, kEntityGreaterThan,
    kEntityDoubleQuote, kEntityApostrophe };
enum XmlVirtualSlot { kXmlDeletingDestructor = 2, kXmlAppendToStream = 4 };
enum XmlNodeKind { kXmlDocument = 0, kXmlElement = 1, kXmlText = 4 };
enum { kXmlOutOfMemory = 3, kFirstPrintableAscii = 0x20, kLastPrintableAscii = 0x7e };
struct ExpandedAttributeString { void* flags; int length; int data; };


int strcmp(const char*, const char*);
void* __nwa__FUl(unsigned int);
char* strcpy(char*, const char*);

struct XmlNodeTypeView {
    char prefixPadding[0x8];
    int nodeType;
};

// Return the document/element/text kind stored in the shared node header.
int TiXmlNode_GetNodeType(XmlNodeTypeView* p) {
    return ((TiXmlNodeLayout*)p)->nodeType;
}

// Initialize this XML node unit's shared scalar at startup.
void InitializeXmlNodeScalars() {
    s_xmlNodeTwoPi = kFloat_6_2831855;
}

// Initialize this XML element unit's shared scalar at startup.
void InitializeXmlElementScalars() {
    s_xmlElementTwoPi = kFloat_6_2831855;
}

struct XmlStringView {
    char* str; // 0x0
    int capacity; // 0x4
};

int strlen(const char* s);
void __dla__FPv(void*);
char* strncat(char*, const char*, unsigned int);

// Measure an allocated XML string; zero capacity means empty.
int TiXmlString_Length(XmlStringView* p) {
    if (p->capacity != 0) {
        return strlen(p->str);
    }
    return 0;
}

#pragma dont_inline on
// Compare two allocated XML strings; absent buffers compare false.
int TiXmlString_Equals(XmlStringView* a, XmlStringView* b) {
    if (a->capacity != 0 && b->capacity != 0) {
        return strcmp(a->str, b->str) == 0;
    }
    return 0;
}
#pragma dont_inline reset

// Find a byte at or after start; return -1 when absent.
int TiXmlString_FindChar(const XmlStringView* self, signed char ch, unsigned int start) {
    int flag = self->capacity;
    unsigned int length;
    if (flag != 0) length = strlen(self->str);
    else length = 0;
    if (start >= length) return -1;
    char* p = self->str + start;
    while (*p != 0) {
        if ((signed char)*p == ch) return (int)(p - self->str);
        ++p;
    }
    return -1;
}

struct XmlOwnedString {
    char* str;
    int capacity;
};

// Replace the owned buffer with a deep copy, releasing the previous buffer.
void TiXmlString_AssignString(XmlOwnedString* self, const XmlOwnedString* other) {
    int otherCapacity = other->capacity;
    int len = otherCapacity != 0 ? strlen(other->str) : 0;
    if (len == 0) {
        if (self->str != 0) __dla__FPv(self->str);
        self->str = 0;
        self->capacity = 0;
        return;
    }
    char* copy;
    unsigned int length = (other->capacity != 0 ? strlen(other->str) : 0) + 1;
    copy = (char*)__nwa__FUl(length);
    strcpy(copy, other->capacity != 0 ? other->str : s_tinyXmlEmptyString);
    if (self->str != 0) __dla__FPv(self->str);
    self->capacity = length;
    self->str = copy;
}

// Append at most count bytes, doubling the required allocation when it grows.
void TiXmlString_AppendCount(XmlOwnedString* self, const char* sourceText, unsigned int count) {
    unsigned int oldLength = self->capacity != 0 ? strlen(self->str) : 0;
    unsigned int needed = count + oldLength + 1;
    if (needed > (unsigned int)self->capacity) {
        unsigned int capacity = needed << 1;
        char* copy = (char*)__nwa__FUl(capacity);
        copy[0] = 0;
        if (self->capacity != 0 && self->str != 0) strcpy(copy, self->str);
        strncat(copy, sourceText, count);
        if (self->capacity != 0 && self->str != 0) __dla__FPv(self->str);
        self->str = copy;
        self->capacity = capacity;
    } else {
        strncat(self->str, sourceText, count);
    }
}

#pragma dont_inline on
} // extern "C"
// Own a copy of a terminated string; a null argument leaves the buffer unallocated.
TiXmlString::TiXmlString(const char* str) {
    XmlOwnedString* self = (XmlOwnedString*)this;
    if (str == 0) {
        self->capacity = 0;
        self->str = 0;
    } else {
        unsigned int len = strlen(str) + 1;
        char* copy = (char*)__nwa__FUl(len);
        strcpy(copy, str);
        self->capacity = len;
        self->str = copy;
        return;
    }
}
extern "C" {
#pragma dont_inline reset

struct XmlAncestorView {
    char prefixPadding[4];
    XmlAncestorView* parent;
    int nodeType;
};

// Follow parent links until a document node is found.
XmlAncestorView* TiXmlNode_GetDocumentAncestor(XmlAncestorView* p) {
    while (p != 0) {
        int valid = 0;
        if (p != 0 && p->nodeType == kXmlDocument) valid = 1;
        XmlAncestorView* r0 = valid ? p : 0;
        if (r0 != 0) {
            int validAgain = 0;
            if (p != 0 && p->nodeType == kXmlDocument) validAgain = 1;
            if (validAgain) return p;
            return 0;
        }
        p = p->parent;
    }
    return 0;
}

struct XmlNodeInitView {
    char* vtable; // 0x0
    int parent; // 0x4
    int nodeType; // 0x8
    int firstChild; // 0xc
    int lastChild; // 0x10
    int valueData; // 0x14
    int valueCapacity; // 0x18
    int previousSibling; // 0x1c
    int nextSibling; // 0x20
    int userData; // 0x24
};

// Initialize an empty node with its supplied node kind and no links or value.
void TiXmlNode_Ctor(XmlNodeInitView* self, int nodeKind) {
    self->valueCapacity = 0;
    self->vtable = __vt__9TiXmlNode;
    self->valueData = 0;
    self->parent = 0;
    self->nodeType = nodeKind;
    self->firstChild = 0;
    self->lastChild = 0;
    self->previousSibling = 0;
    self->nextSibling = 0;
    self->userData = 0;
}

struct XmlChildLinks {
    char prefixPadding[0x4]; // 0x0
    XmlChildLinks* parent; // 0x4
    char nodeType[0x4]; // 0x8
    XmlChildLinks* firstChild; // 0xc
    XmlChildLinks* lastChild; // 0x10
    char valueData[0x8]; // 0x14
    XmlChildLinks* previousSibling; // 0x1c
    XmlChildLinks* nextSibling; // 0x20
};

// str/valueCapacity at +0x14/+0x18 are the node value buffer and allocation capacity.
// valueCapacity is allocated storage, not a cached character count.
struct XmlNodeLifetimeView {
    char* vtable;
    char parent[8];
    XmlNodeLifetimeView* first;
    char lastChild[4];
    void* str;
    int valueCapacity;
    char previousSibling[4];
    XmlNodeLifetimeView* next;
};

extern char __vt__9TiXmlNode[];
void __dl__FPv(void*);

class XmlNodeDeletingInterface_u802A3004 { public: 
virtual void destroy(int);
};
// Destroy all children and the owned node value, then optionally free the node.
XmlNodeLifetimeView* TiXmlNode_dtor(XmlNodeLifetimeView* self, int deleting) {
    if (self != 0) {
        XmlNodeLifetimeView* item = self->first;
        self->vtable = __vt__9TiXmlNode;
        while (item != 0) {
            XmlNodeLifetimeView* current = item;
            item = item->next;
            if (current != 0) {
                ((XmlNodeDeletingInterface_u802A3004*)current)->destroy(1);
            }
        }
        if ((void*)((char*)self + XML_MEMBER_OFFSET(XmlNodeLifetimeView, str)) != 0) {
            if (self->str != 0) __dla__FPv(self->str);
            self->str = 0;
            self->valueCapacity = 0;
        }
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

struct XmlStringDestructorView;
void __dt__11TiXmlStringFv(XmlStringDestructorView*);
// Destructor ABI adapter for {char* buffer, int allocationCapacity}.
struct XmlStringDestructorView {
    char* str;
    int capacity;
    // Forward cleanup of the owned XML byte buffer through the string destructor ABI.
    ~XmlStringDestructorView() { __dt__11TiXmlStringFv(this); }
};



TiXmlString TiXmlNode_CopyValue(XmlNodeLifetimeView*);
XmlOwnedString* __ct__11TiXmlStringFPCc(XmlOwnedString*, const char*);

} // extern "C"
// Release the owned byte array and clear its pointer and capacity.
TiXmlString::~TiXmlString() {
        if (str) delete[] str;
        str = 0;
        capacity = 0;
}
extern "C" {
#pragma inline_depth(1)
// Find the first child whose copied value equals name.
XmlNodeLifetimeView* TiXmlNode_FindChildByValue(const XmlNodeLifetimeView* self, const char* name) {
    XmlNodeLifetimeView* item = self->first;
    while (item != 0) {
        int equal;
        {
        TiXmlString itemName = TiXmlNode_CopyValue(item);
        TiXmlString searchName(name);
        equal = TiXmlString_Equals((XmlStringView*)&itemName, (XmlStringView*)&searchName);
        }
        if (equal != 0) return item;
        item = item->next;
    }
    return 0;
}
#pragma inline_depth()

#pragma inline_depth(1)
// Find the next sibling whose copied value equals name.
XmlNodeLifetimeView* TiXmlNode_FindNextSiblingByValue(const XmlNodeLifetimeView* self, const char* name) {
    XmlNodeLifetimeView* item = self->next;
    while (item != 0) {
        int equal;
        {
        TiXmlString itemName = TiXmlNode_CopyValue(item);
        TiXmlString searchName(name);
        equal = TiXmlString_Equals((XmlStringView*)&itemName, (XmlStringView*)&searchName);
        }
        if (equal != 0) return item;
        item = item->next;
    }
    return 0;
}
#pragma inline_depth()

// Append a child and update its parent and both sibling links.
XmlChildLinks* TiXmlNode_LinkEndChild(XmlChildLinks* self, XmlChildLinks* item) {
    item->parent = self;
    item->previousSibling = self->lastChild;
    item->nextSibling = 0;
    if (self->lastChild != 0) {
        self->lastChild->nextSibling = item;
    } else {
        self->firstChild = item;
    }
    self->lastChild = item;
    return item;
}

// Allocate a deep copy; an unallocated source produces an empty string.
XmlOwnedString* TiXmlString_CopyCtor(XmlOwnedString* self, XmlOwnedString* other) {
    if (other == self) {
        return self;
    }
    if (other->capacity == 0) {
        self->capacity = 0;
        self->str = 0;
        return self;
    }
    unsigned int len = strlen(other->str) + 1;
    char* copy = (char*)__nwa__FUl(len);
    strcpy(copy, other->str);
    self->capacity = len;
    self->str = copy;
    return self;
}

// Replace the owned string, treating a null source as empty.
void TiXmlString_AssignCStr(XmlOwnedString* self, const char* str) {
    if (str == 0) {
        if (self->str != 0) {
            __dla__FPv(self->str);
        }
        self->str = 0;
        self->capacity = 0;
    } else {
        unsigned int len = strlen(str) + 1;
        char* copy = (char*)__nwa__FUl(len);
        strcpy(copy, str);
        if (self->str != 0) {
            __dla__FPv(self->str);
        }
        self->capacity = len;
        self->str = copy;
    }
}

extern int Compatibility_ReportSuccess(unsigned int);
extern int sprintf(char*, const char*, ...);

#pragma inline_depth(0)
// Escape XML delimiters and control bytes using the existing entity/pair-copy rules.
// Compatibility_ReportSuccess is verified in name_entry_and_misc_utilities.cpp: it ignores arguments and
// returns 1. Thus ANY remaining byte with a successor copies two bytes here; it
// is not a verified multibyte lead-byte classifier. Preserve that compatibility
// behavior when changing encoding rules. Input/output must be distinct buffers.
void TiXmlString_EncodeEntities(XmlStringView* input, XmlOwnedString* output) {
    unsigned int index = 0;
    int length = TiXmlString_Length((XmlStringView*)input);
    while ((int)index < length) {
        signed char ch = (signed char)input->str[index];
        if (ch == '&' && (int)index < length - 2 && input->str[index + 1] == '#' && input->str[index + 2] == 'x') {
            while ((int)index < length) {
                const char* source = input->capacity != 0 ? input->str : s_tinyXmlEmptyString;
                TiXmlString_AppendCount(output, source + index, 1);
                ++index;
                if ((signed char)input->str[index] == ';') break;
            }
        } else if (ch == '&') {
            TiXmlString_AppendCount(output, *(const char**)xmlEntityTableStorage, *(unsigned int*)(xmlEntityTableStorage + kEntityAmpersand * sizeof(XmlEntity) + XML_MEMBER_OFFSET(XmlEntity, length)));
            ++index;
        } else if (ch == '<') {
            TiXmlString_AppendCount(output, *(const char**)(xmlEntityTableStorage + kEntityLessThan * sizeof(XmlEntity) + XML_MEMBER_OFFSET(XmlEntity, spelling)), *(unsigned int*)(xmlEntityTableStorage + kEntityLessThan * sizeof(XmlEntity) + XML_MEMBER_OFFSET(XmlEntity, length)));
            ++index;
        } else if (ch == '>') {
            TiXmlString_AppendCount(output, *(const char**)(xmlEntityTableStorage + kEntityGreaterThan * sizeof(XmlEntity) + XML_MEMBER_OFFSET(XmlEntity, spelling)), *(unsigned int*)(xmlEntityTableStorage + kEntityGreaterThan * sizeof(XmlEntity) + XML_MEMBER_OFFSET(XmlEntity, length)));
            ++index;
        } else if (ch == '"') {
            TiXmlString_AppendCount(output, *(const char**)(xmlEntityTableStorage + kEntityDoubleQuote * sizeof(XmlEntity) + XML_MEMBER_OFFSET(XmlEntity, spelling)), *(unsigned int*)(xmlEntityTableStorage + kEntityDoubleQuote * sizeof(XmlEntity) + XML_MEMBER_OFFSET(XmlEntity, length)));
            ++index;
        } else if (ch == '\'') {
            TiXmlString_AppendCount(output, *(const char**)(xmlEntityTableStorage + kEntityApostrophe * sizeof(XmlEntity) + XML_MEMBER_OFFSET(XmlEntity, spelling)), *(unsigned int*)(xmlEntityTableStorage + kEntityApostrophe * sizeof(XmlEntity) + XML_MEMBER_OFFSET(XmlEntity, length)));
            ++index;
        } else if (Compatibility_ReportSuccess((unsigned char)ch) && (int)index < length - 1) {
            TiXmlString_AppendCount(output, input->str + index, 2);
            index += 2;
        } else if (ch < kFirstPrintableAscii || ch > kLastPrintableAscii) {
            char buffer[32];
            sprintf(buffer, tinyXmlWriterStrings, (unsigned char)ch);
            int n = strlen(buffer);
            TiXmlString_AppendCount(output, buffer, n);
            ++index;
        } else {
            char one = ch;
            TiXmlString_AppendCount(output, &one, 1);
            ++index;
        }
    }
}

struct XmlAttribute {
    char* vtable;
    char documentStorage[4];
    XmlOwnedString key;
    XmlOwnedString value;
    XmlAttribute* next;
    XmlAttribute* prev;
};
struct XmlAttributeList {
    char prefixPadding[0x28];
    XmlAttribute sentinel;
};
struct XmlDocumentErrorView {
    char prefixPadding[4];
    XmlDocumentErrorView* parent;
    int nodeType;
    char firstChild[0x1C];
    char error;
    int errorId;
    XmlOwnedString errorMessage;
};

#pragma opt_common_subs off
// Original 0x802A3F8C: search backward through the attribute list.
// Replace a matching attribute or allocate one; record document error 3 on allocation failure.
void TiXmlElement_SetAttribute(XmlAttributeList* self, const char* key, const char* value) {
    XmlAttribute* node = self->sentinel.prev;
    while (node != (XmlAttribute*)((char*)self + XML_MEMBER_OFFSET(XmlAttributeList, sentinel)) || (node = 0, false)) {
        XmlOwnedString keyCopy;
        __ct__11TiXmlStringFPCc(&keyCopy, key);
        int same = TiXmlString_Equals((XmlStringView*)&node->key, (XmlStringView*)&keyCopy);
        if (keyCopy.str != 0) __dla__FPv(keyCopy.str);
        keyCopy.str = 0;
        keyCopy.capacity = 0;
        if (same != 0) break;
        node = node->prev;
    }
    if (node != 0) {
        TiXmlString_AssignCStr(&node->value, value);
        return;
    }
    XmlAttribute* added = new XmlAttribute;
    if (added != 0) {
        added->vtable = __vt__14TiXmlAttribute;
        __ct__11TiXmlStringFPCc(&added->key, key);
        __ct__11TiXmlStringFPCc(&added->value, value);
        added->next = 0;
        added->prev = 0;
    }
    if (added != 0) {
        added->prev = (XmlAttribute*)((char*)self + XML_MEMBER_OFFSET(XmlAttributeList, sentinel));
        added->next = self->sentinel.next;
        self->sentinel.next->prev = added;
        self->sentinel.next = added;
        return;
    }
    XmlDocumentErrorView* walk = (XmlDocumentErrorView*)self;
    // The final assignment belongs only to the exhausted-search path.
    while (walk != 0 || (walk = 0, false)) {
        int ok = 0;
        if (walk != 0 && walk->nodeType == kXmlDocument) ok = 1;
        XmlDocumentErrorView* r0 = ok ? walk : 0;
        if (r0 != 0) {
            int ok2 = 0;
            if (walk != 0 && walk->nodeType == kXmlDocument) ok2 = 1;
            if (!ok2) walk = 0;
            break;
        }
        walk = walk->parent;
    }
    if (walk != 0) {
        walk->error = 1;
        walk->errorId = kXmlOutOfMemory;
        TiXmlString_AssignCStr(&walk->errorMessage, *(const char**)(tinyXmlErrorMessages + 0xc));
    }
}
#pragma opt_common_subs reset

// Volatile scalar stores preserve the original element-construction order.
// The value buffer/capacity is the same pair as in the node views;
// integer link/data words here are Wii pointer bits, not scalar lengths or IDs.
struct XmlElementStoreView {
    volatile char* vtable; // 0x0
    volatile int parent; // 0x4
    volatile int nodeType; // 0x8
    volatile int firstChild; // 0xc
    volatile int lastChild; // 0x10
    volatile char* str; // 0x14
    volatile int valueCapacity; // 0x18
    volatile int previousSibling; // 0x1c
    volatile int nextSibling; // 0x20
    volatile int userData; // 0x24
    volatile char* attributeSentinelVtable; // 0x28
    volatile int attributeDocument; // 0x2c
    volatile int attributeNameData; // 0x30
    volatile int attributeNameCapacity; // 0x34
    volatile int attributeValueData; // 0x38
    volatile int attributeValueCapacity; // 0x3c
    volatile void* lastAttribute; // 0x40
    volatile void* firstAttribute; // 0x44
};

#pragma inline_depth(0)
// Initialize an element, its circular attribute sentinel, and its tag name.
XmlElementStoreView* TiXmlElement_Ctor(XmlElementStoreView* self, const char* arg) {
    self->firstChild = 0;
    self->lastChild = 0;
    self->valueCapacity = 0;
    self->str = 0;
    self->parent = 0;
    self->nodeType = kXmlElement;
    self->previousSibling = 0;
    self->nextSibling = 0;
    self->userData = 0;
    self->vtable = __vt__12TiXmlElement;
    self->attributeSentinelVtable = __vt__14TiXmlAttribute;
    self->attributeNameCapacity = 0;
    self->attributeNameData = 0;
    self->attributeValueCapacity = 0;
    self->attributeValueData = 0;
    self->firstAttribute = (void*)((char*)self + XML_MEMBER_OFFSET(XmlElementStoreView, attributeSentinelVtable));
    self->lastAttribute = (void*)((char*)self + XML_MEMBER_OFFSET(XmlElementStoreView, attributeSentinelVtable));
    self->lastChild = 0;
    self->firstChild = 0;
    TiXmlString_AssignCStr((XmlOwnedString*)((char*)self + XML_MEMBER_OFFSET(XmlElementStoreView, str)), arg);
    return self;
}
#pragma inline_depth()

extern char tinyXmlWriterStrings[];
extern void* String_ReserveCapacity(void*, unsigned int);
extern void String_ReplaceRange(void*, int, int, void*, void*, void*);
extern char* strcat(char*, const char*);
extern int String_ParseDecimalInt(const char*);

struct XmlAllocatorByte {
    char c;
    char pad[3];
};

struct XmlAttributeReadView {
    char prefixPadding[8];
    char* nameStr; // 0x8
    int nameFlag; // 0xc
    void* valueData; // 0x10
    int valueCapacity; // 0x14
    XmlAttributeReadView* prev; // 0x18
    XmlAttributeReadView* next; // 0x1c
};

// Append a terminated string, growing the owned allocation when required.
void TiXmlString_Append(XmlStringView* self, const char* arg) {
    unsigned int oldLen;
    if (self->capacity != 0) oldLen = strlen(self->str);
    else oldLen = 0;
    unsigned int argLen = strlen(arg);
    unsigned int needed = oldLen + argLen + 1;
    if (needed > (unsigned int)self->capacity) {
        unsigned int capacity = needed << 1;
        char* copy = (char*)__nwa__FUl(capacity);
        copy[0] = 0;
        if (self->capacity != 0 && self->str != 0) strcpy(copy, self->str);
        strcat(copy, arg);
        if (self->capacity != 0 && self->str != 0) __dla__FPv(self->str);
        self->str = copy;
        self->capacity = capacity;
    } else {
        strcat(self->str, arg);
    }
}

// Original 0x802A3E98: search forward through the attribute list.
// Return an attribute value and optionally parse its decimal integer value.
void* TiXmlElement_GetAttribute(void* self, const char* search, int* out) {
    XmlAttributeReadView* sentinel = (XmlAttributeReadView*)&((TiXmlNodeLayout*)self)->documentError;
    XmlAttributeReadView* node = (XmlAttributeReadView*)((TiXmlNodeLayout*)self)->firstAttribute; // original attribute list +0x44.
    while (node != sentinel || (node = 0, false)) {
        XmlOwnedString local;
        __ct__11TiXmlStringFPCc(&local, search);
        int equal = TiXmlString_Equals((XmlStringView*)&node->nameStr, (XmlStringView*)&local);
        if (local.str != 0) __dla__FPv(local.str);
        local.str = 0;
        local.capacity = 0;
        if (equal != 0) break;
        node = node->next;
    }
    if (node == 0) {
        if (out != 0) *out = 0;
        return 0;
    }
    char* result;
    if (node->valueCapacity != 0) result = (char*)node->valueData;
    else result = s_tinyXmlEmptyString;
    if (out != 0) {
        if (result != 0) *out = String_ParseDecimalInt(result);
        else *out = 0;
    }
    return result;
}

#pragma opt_common_subs off
#pragma inline_depth(1)
// Find an attribute by name, stopping at the circular list sentinel.
inline XmlAttributeReadView* FindD40(void* container, void* search) {
    XmlAttributeReadView* node;
    int equal;
    XmlAttributeReadView* sentinel = (XmlAttributeReadView*)&((TiXmlNodeLayout*)container)->documentError;
    node = (XmlAttributeReadView*)((TiXmlNodeLayout*)container)->firstAttribute;
    while (node != sentinel) {
        XmlOwnedString local;
        __ct__11TiXmlStringFPCc(&local, (const char*)search);
        equal = TiXmlString_Equals((XmlStringView*)&node->nameStr, (XmlStringView*)&local);
        if (local.str != 0) __dla__FPv(local.str);
        local.str = 0;
        local.capacity = 0;
        if (equal) return node;
        node = node->next;
    }
    return 0;
}
// Copy the named attribute into the engine string format; use the pool's empty string if absent.
void TiXmlElement_CopyAttributeToEngineString(void* outObj, void* container, void* search) {
    XmlAttributeReadView* node = FindD40(container, search);
    if (node != 0) {
        char* str;
        if (node->valueCapacity != 0) str = (char*)node->valueData;
        else str = s_tinyXmlEmptyString;
        ((ExpandedAttributeString*)outObj)->flags = 0;
        ((ExpandedAttributeString*)outObj)->length = 0;
        ((ExpandedAttributeString*)outObj)->data = 0;
        unsigned int len = strlen(str);
        String_ReserveCapacity(outObj, len);
        volatile XmlAllocatorByte extraSrc;
        XmlAllocatorByte extra;
        extra.c = extraSrc.c;
        String_ReplaceRange(outObj, 0, 0, str, (void*)((char*)str + len), &extra);
    } else {
        char* str = (tinyXmlWriterStrings + XML_MEMBER_OFFSET(XmlWriterStrings, empty));
        ((ExpandedAttributeString*)outObj)->flags = 0;
        ((ExpandedAttributeString*)outObj)->length = 0;
        ((ExpandedAttributeString*)outObj)->data = 0;
        unsigned int len = strlen(str);
        String_ReserveCapacity(outObj, len);
        volatile XmlAllocatorByte extraSrc;
        XmlAllocatorByte extra;
        extra.c = extraSrc.c;
        String_ReplaceRange(outObj, 0, 0, str, (void*)((char*)str + len), &extra);
    }
}
#pragma inline_depth(0)

extern int fprintf(void*, const char*, ...);

struct XmlPrintable {
    virtual void reserved();
    virtual void print(void* stream, int count);
};

// Print an indented element, keeping a sole text child on the same line.
void TiXmlElement_Print(XmlElementStoreView* self, void* stream, int count) {
    int i;
    for (i = 0; i < count; i++) {
        fprintf(stream, (tinyXmlWriterStrings + XML_MEMBER_OFFSET(XmlWriterStrings, space)));
    }
    int length = self->valueCapacity;
    fprintf(stream, (tinyXmlWriterStrings + XML_MEMBER_OFFSET(XmlWriterStrings, openTagFormat)), length != 0 ? (const char*)self->str : s_tinyXmlEmptyString);
    void* node = (void*)self->firstAttribute;
    if (node == (void*)((char*)self + XML_MEMBER_OFFSET(XmlElementStoreView, attributeSentinelVtable))) node = 0;
    while (node != 0) {
        fprintf(stream, (tinyXmlWriterStrings + XML_MEMBER_OFFSET(XmlWriterStrings, space)));
        ((XmlPrintable*)node)->print(stream, count);
        XmlStringView* p1 = (XmlStringView*)&((XmlAttributeReadView*)node)->next->valueData;
        int empty1 = TiXmlString_Length(p1) == 0;
        if (empty1) {
            XmlStringView* p2 = (XmlStringView*)&((XmlAttributeReadView*)node)->next->nameStr;
            int empty2 = TiXmlString_Length(p2) == 0;
            if (empty2) {
                node = 0;
                continue;
            }
        }
        node = ((XmlAttributeReadView*)node)->next;
    }
    void* r4 = (void*)self->firstChild;
    if (r4 == 0) {
        fprintf(stream, (tinyXmlWriterStrings + XML_MEMBER_OFFSET(XmlWriterStrings, emptyTagClose)));
    } else {
        if (r4 == (void*)self->lastChild) {
            int isType4 = 0;
            if (r4 != 0) {
                if (((TiXmlNodeLayout*)r4)->nodeType == kXmlText) isType4 = 1;
            }
            r4 = isType4 ? r4 : 0;
            if (r4 != 0) {
                fprintf(stream, (tinyXmlWriterStrings + XML_MEMBER_OFFSET(XmlWriterStrings, tagClose)));
                void* sub = (void*)self->firstChild;
                ((XmlPrintable*)sub)->print(stream, count + 1);
                int length = self->valueCapacity;
                fprintf(stream, (tinyXmlWriterStrings + XML_MEMBER_OFFSET(XmlWriterStrings, endTagFormat)), length != 0 ? (const char*)self->str : s_tinyXmlEmptyString);
                return;
            }
    }
        fprintf(stream, (tinyXmlWriterStrings + XML_MEMBER_OFFSET(XmlWriterStrings, tagClose)));
        void* cur = (void*)self->firstChild;
        while (cur != 0) {
            int valid = 0;
            if (cur != 0 && ((TiXmlNodeLayout*)cur)->nodeType == kXmlText) valid = 1;
            void* r0 = valid ? cur : 0;
            if (r0 == 0) {
                fprintf(stream, (tinyXmlWriterStrings + XML_MEMBER_OFFSET(XmlWriterStrings, newline)));
            }
            ((XmlPrintable*)cur)->print(stream, count + 1);
            cur = ((TiXmlNodeLayout*)cur)->nextSibling;
        }
        fprintf(stream, (tinyXmlWriterStrings + XML_MEMBER_OFFSET(XmlWriterStrings, newline)));
        for (i = 0; i < count; i++) {
            fprintf(stream, (tinyXmlWriterStrings + XML_MEMBER_OFFSET(XmlWriterStrings, space)));
        }
        int length = self->valueCapacity;
        fprintf(stream, (tinyXmlWriterStrings + XML_MEMBER_OFFSET(XmlWriterStrings, endTagFormat)), length != 0 ? (const char*)self->str : s_tinyXmlEmptyString);
    }
}

// Append compact element markup and recursively serialize children.
void TiXmlElement_AppendElementMarkupToString(XmlElementStoreView* self, XmlStringView* stream) {
    TiXmlString_Append(stream, (tinyXmlWriterStrings + XML_MEMBER_OFFSET(XmlWriterStrings, tagOpen)));
    if (self->valueCapacity != 0) TiXmlString_Append(stream, (const char*)self->str);
    else TiXmlString_Append(stream, s_tinyXmlEmptyString);
    XmlAttributeReadView* node = (XmlAttributeReadView*)self->firstAttribute;
    XmlAttributeReadView* sentinel = (XmlAttributeReadView*)&((TiXmlNodeLayout*)self)->documentError;
    if (node == sentinel) node = 0;
    char* labelD = (tinyXmlWriterStrings + XML_MEMBER_OFFSET(XmlWriterStrings, space));
    while (node != 0) {
        TiXmlString_Append(stream, labelD);
        ((void (*)(XmlAttributeReadView*, XmlStringView*))(*(void***)node)[kXmlAppendToStream])(node, stream);
        XmlAttributeReadView* next = node->next;
        if (TiXmlString_Length((XmlStringView*)&next->valueData)) {
            node = next;
        } else {
            if (TiXmlString_Length((XmlStringView*)&next->nameStr)) node = next;
            else node = 0;
        }
    }
    if (self->firstChild != 0) {
        TiXmlString_Append(stream, (tinyXmlWriterStrings + XML_MEMBER_OFFSET(XmlWriterStrings, tagClose)));
        XmlNodeLifetimeView* n = (XmlNodeLifetimeView*)self->firstChild;
        while (n != 0) {
            /* Original 0x802A4540-0x802A4558: the child's +0x20 link is read after its vfunc_4 call returns. */
            ((void (*)(XmlNodeLifetimeView*, XmlStringView*))(*(void***)n)[kXmlAppendToStream])(n, stream);
            n = n->next;
        }
        TiXmlString_Append(stream, (tinyXmlWriterStrings + XML_MEMBER_OFFSET(XmlWriterStrings, endTagOpen)));
        if (self->valueCapacity != 0) TiXmlString_Append(stream, (const char*)self->str);
        else TiXmlString_Append(stream, s_tinyXmlEmptyString);
        TiXmlString_Append(stream, (tinyXmlWriterStrings + XML_MEMBER_OFFSET(XmlWriterStrings, tagClose)));
    } else {
        TiXmlString_Append(stream, (tinyXmlWriterStrings + XML_MEMBER_OFFSET(XmlWriterStrings, emptyTagClose)));
    }
}

// Unlink and destroy attributes, destroy children, and optionally free the element.
XmlElementStoreView* TiXmlElement_dtor(XmlElementStoreView* self, int deleting) {
    if (self != 0) {
        self->vtable = __vt__12TiXmlElement;
        // Phase 1: unlink each owned attribute before invoking its deleting destructor.
        XmlAttributeReadView* sentinel = (XmlAttributeReadView*)&((TiXmlNodeLayout*)self)->documentError;
        XmlAttributeReadView* attributeCursor = (XmlAttributeReadView*)self->firstAttribute;
        while (attributeCursor != 0 && attributeCursor != sentinel) {
            XmlAttributeReadView* attributeToDelete = (attributeCursor != sentinel) ? attributeCursor : 0;
            attributeCursor = (XmlAttributeReadView*)self->firstAttribute;
            while (attributeCursor != sentinel) {
                if (attributeToDelete == attributeCursor) {
                    XmlAttributeReadView* prev = attributeCursor->prev;
                    XmlAttributeReadView* next = attributeCursor->next;
                    prev->next = next;
                    next->prev = prev;
                    attributeCursor->next = 0;
                    attributeCursor->prev = 0;
                    break;
                }
                attributeCursor = attributeCursor->next;
            }
            if (attributeToDelete != 0) {
                ((void (*)(XmlAttributeReadView*, int))(*(void***)attributeToDelete)[kXmlDeletingDestructor])(attributeToDelete, 1);
            }
            attributeCursor = (XmlAttributeReadView*)self->firstAttribute;
        }
        // Phase 2: release the embedded sentinel's two owned string buffers.
        if ((void*)((char*)self + XML_MEMBER_OFFSET(XmlElementStoreView, attributeSentinelVtable)) != 0) {
            if ((void*)((char*)self + XML_MEMBER_OFFSET(XmlElementStoreView, attributeValueData)) != 0) {
                if (self->attributeValueData != 0) __dla__FPv((void*)self->attributeValueData);
                self->attributeValueData = 0;
                self->attributeValueCapacity = 0;
            }
            if ((void*)((char*)self + XML_MEMBER_OFFSET(XmlElementStoreView, attributeNameData)) != 0) {
                /* Original 0x802A3C80-0x802A3CA0: +0x30/+0x34 are zeroed even when +0x30 was already null. */
                if (self->attributeNameData != 0) __dla__FPv((void*)self->attributeNameData);
                self->attributeNameData = 0;
                self->attributeNameCapacity = 0;
            }
        }
        // Phase 3: destroy child nodes, then the element value buffer and optional owner.
        if (self != 0) {
            self->vtable = __vt__9TiXmlNode;
            XmlNodeLifetimeView* node = (XmlNodeLifetimeView*)self->firstChild;
            while (node != 0) {
                XmlNodeLifetimeView* next = node->next;
                if (node != 0) {
                    ((void (*)(XmlNodeLifetimeView*, int))(*(void***)node)[kXmlDeletingDestructor])(node, 1);
                }
                node = next;
            }
            if ((void*)((char*)self + XML_MEMBER_OFFSET(XmlElementStoreView, str)) != 0) {
                if (self->str != 0) __dla__FPv((void*)self->str);
                self->str = 0;
                self->valueCapacity = 0;
            }
        }
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804624B4", _ctors_u802A3004_0, InitializeXmlNodeScalars);
STATIC_INIT_ENTRY(".CRT$XCU804624B4", _ctors_u802A3004_1, InitializeXmlElementScalars);
#pragma pop
