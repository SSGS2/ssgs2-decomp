#include <decomp/local_class.h>
#include "classes/TiXmlDeclarationLayout.h"
#include "classes/TiXmlNodeLayout.h"
#include "classes/TiXmlAttributeLayout.h"
// Translation unit 0x802A45E4-0x802A5CC4 (24 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

void TiXmlString_CopyCtor(void*, void*);
void TiXmlString_EncodeEntities(void*, void*);
int TiXmlString_Equals(void*, void*);
void TiXmlString_Append(void*, const char*);
int fputs(const char*, const char*);
void fprintf(void*, const char*, ...);
void __dla__FPv(void*);
void __dl__FPv(void*);
void __dt__11TiXmlStringFv(void*, int);
void TiXmlNode_dtor(void*, int);
void* __nw__FUl(unsigned int);
void TiXmlString_AssignCStr(void*, const char*);
void TiXmlString_AssignString(void*, const void*);
int TiXmlString_Length(void*);
extern "C" int __cntlzw(unsigned int);
// CodeWarrior's leading-zero idiom yields one for an empty string and zero otherwise.
inline unsigned int XmlStringIsEmpty(void* value) {
    return (unsigned int)__cntlzw((unsigned int)TiXmlString_Length(value)) >> 5;
}
void __ct__11TiXmlStringFPCc(void*, const char*);
unsigned int TiXmlString_FindChar(void*, int, int);
extern char __vt__13TiXmlDocument[];
extern char __vt__14TiXmlAttribute[];
extern const char* tinyXmlErrorMessages[];
extern char __vt__12TiXmlElement[];
extern char __vt__12TiXmlComment[];
extern char __vt__12TiXmlUnknown[];
extern char __vt__9TiXmlNode[];
extern char __vt__9TiXmlText[];
extern char __vt__16TiXmlDeclaration[];
// Packed pool: array extents retain the original string addresses, including offset 9's empty string.
struct TinyXmlWriterStrings {
    char hexEntityFormat[9];
    char emptyString[1];
    char integerFormat[3];
    char indentSpace[2];
    char elementOpenFormat[4];
    char emptyElementClose[4];
    char tagClose[2];
    char elementCloseFormat[6];
    char newline[2];
    char tagOpen[2];
    char endTagOpen[3];
    char doubleQuotedAttributeFormat[8];
    char singleQuotedAttributeFormat[8];
    char singleQuote[2];
    char equals[2];
    char doubleQuote[2];
    char doubleFormat[4];
    char commentFormat[10];
    char commentOpen[5];
    char commentClose[4];
    char textFormat[3];
    char declarationOpen[7];
    char versionFormat[14];
    char encodingFormat[15];
    char standaloneFormat[17];
    char declarationClose[3];
    char versionOpen[10];
    char attributeClose[3];
    char encodingOpen[11];
    char standaloneOpen[13];
    char reserveLengthError[35];
    char lengthErrorTypes[55];
    char outOfRangeError[27];
    char outOfRangeTypes[55];
    char stringLengthError[27];
    char alignmentPadding[2];
};
extern TinyXmlWriterStrings tinyXmlWriterStrings;
typedef char TinyXmlWriterStrings_size[(sizeof(TinyXmlWriterStrings) == 0x17c) ? 1 : -1];
typedef char TinyXmlWriterStrings_declarationOffset[((int)&((TinyXmlWriterStrings*)0)->declarationOpen == 0x56) ? 1 : -1];
enum XmlNodeType { kXmlDocument, kXmlElement, kXmlComment, kXmlUnknown, kXmlText, kXmlDeclaration };
enum XmlError { kXmlOutOfMemory = 3 };
extern char s_tinyXmlEmptyString;

struct XmlValueView { char pad[0x14]; const char* value; int valueLength; };

struct DocumentCloneStorage {
    void* vtable;
    int parent, nodeType, firstChild, lastChild, valueData, valueLength;
    int previousSibling, nextSibling;
    int userData;
    unsigned char error;
    char pad29[7];
    const char* errorText;
    int errorTextLength;
};

struct CloneableXmlNode {
    virtual void* v0();
    virtual void* v1();
    virtual void* v2();
    virtual void* v3();
    virtual void* v4();
    virtual void* clone();
    virtual void* v6();
    virtual void* v7();
    char pad04[0x1c];
    CloneableXmlNode* next;
};

// Deep-clone the document, its error state and all child nodes.
void* TiXmlDocument_Clone(const DocumentCloneStorage* self) {
    DocumentCloneStorage* obj = (DocumentCloneStorage*)__nw__FUl(0x38);
    if (obj) {
        obj->valueLength = 0; obj->valueData = 0; obj->parent = 0; obj->nodeType = kXmlDocument;
        obj->firstChild = 0; obj->lastChild = 0;
        obj->previousSibling = 0; obj->nextSibling = 0; obj->userData = 0;
        obj->vtable = (void*)__vt__13TiXmlDocument;
        obj->errorTextLength = 0; obj->errorText = 0;
        obj->error = 0;
    }
    if (!obj) return 0;
    TiXmlString_AssignCStr((char*)&((TiXmlNodeLayout*)obj)->value,
                self->valueLength ? (const char*)self->valueData : &s_tinyXmlEmptyString);
    obj->userData = self->userData;
    obj->error = self->error;
    TiXmlString_AssignCStr((char*)&obj->errorText,
                self->errorTextLength ? (const char*)self->errorText : &s_tinyXmlEmptyString);
    CloneableXmlNode* node = (CloneableXmlNode*)((TiXmlNodeLayout*)self)->firstChild;
    while (node) {
        void* child = node->clone();
        ((TiXmlNodeLayout*)child)->parent = obj;
        ((TiXmlNodeLayout*)child)->previousSibling = ((TiXmlNodeLayout*)obj)->lastChild;
        ((TiXmlNodeLayout*)child)->nextSibling = 0;
        void* tail = ((TiXmlNodeLayout*)obj)->lastChild;
        if (tail) ((TiXmlNodeLayout*)tail)->nextSibling = child;
        else ((TiXmlNodeLayout*)obj)->firstChild = child;
        ((TiXmlNodeLayout*)obj)->lastChild = child;
        node = node->next;
    }
    return obj;
}

// Keep byte-base addresses for declaration strings: typed addresses change CodeWarrior scheduling.
// Print the XML declaration and each nonempty version/encoding/standalone attribute.
void TiXmlDeclaration_Print(void* self, void* out) {
    char* s = (char*)self;
    fprintf(out, tinyXmlWriterStrings.declarationOpen);
    if (!XmlStringIsEmpty(s + (int)&((TiXmlDeclarationLayout*)0)->version))
        fprintf(out, tinyXmlWriterStrings.versionFormat,
                    AsTiXmlDeclaration(s)->version.length ? AsTiXmlDeclaration(s)->version.data : &s_tinyXmlEmptyString);
    if (!XmlStringIsEmpty(s + (int)&((TiXmlDeclarationLayout*)0)->encoding))
        fprintf(out, tinyXmlWriterStrings.encodingFormat,
                    AsTiXmlDeclaration(s)->encoding.length ? AsTiXmlDeclaration(s)->encoding.data : &s_tinyXmlEmptyString);
    if (!XmlStringIsEmpty(s + (int)&((TiXmlDeclarationLayout*)0)->standalone))
        fprintf(out, tinyXmlWriterStrings.standaloneFormat,
                    AsTiXmlDeclaration(s)->standalone.length ? AsTiXmlDeclaration(s)->standalone.data : &s_tinyXmlEmptyString);
    fprintf(out, tinyXmlWriterStrings.declarationClose);
}

// Append an unknown XML node enclosed in angle brackets.
void TiXmlUnknown_AppendAngleBracketedValueToString(XmlValueView* self, void* output) {
    TiXmlString_Append(output, tinyXmlWriterStrings.tagOpen);
    TiXmlString_Append(output, self->valueLength ? self->value : &s_tinyXmlEmptyString);
    TiXmlString_Append(output, tinyXmlWriterStrings.tagClose);
}

#pragma push
#pragma use_lmw_stmw on
// Print an indented XML comment without changing its stored text.
void TiXmlComment_Print(XmlValueView* self, const char* output, int count) {
    for (int i = 0; i < count; ++i) fputs(tinyXmlWriterStrings.indentSpace, output);
    fprintf((void*)output, tinyXmlWriterStrings.commentFormat,
                self->valueLength ? self->value : &s_tinyXmlEmptyString);
}
#pragma pop

struct PrintableXmlNode {
    virtual void* v0();
    virtual void print(void*, void*);
    char pad[0x1c]; PrintableXmlNode* next;
};
struct PrintableDocumentView { char pad[0x0c]; PrintableXmlNode* first; };

// Print every top-level child, adding a newline after each node.
void TiXmlDocument_Print(const PrintableDocumentView* self, void* output, void* indentation) {
    for (PrintableXmlNode* node = self->first; node; node = node->next) {
        node->print(output, indentation);
        fprintf(output, tinyXmlWriterStrings.newline);
    }
}

// Copy-construct a string from the node's value storage.
void TiXmlNode_CopyValue(void* destination, void* source) {
    TiXmlString_CopyCtor(destination, (char*)&((TiXmlNodeLayout*)source)->value);
}

struct XmlNodeStorage {
    void* vtable;
    int parent, nodeType, firstChild, lastChild, valueData, valueLength;
    int previousSibling, nextSibling, userData;
};

// Allocate a node of the requested type and copy its value and user data.
void* make_802A51BC(void* self, char* vtable, int state) {
    XmlNodeStorage* obj = (XmlNodeStorage*)__nw__FUl(0x28);
    if (obj) {
        obj->valueLength = 0;
        obj->valueData = 0;
        obj->parent = 0;
        obj->nodeType = state;
        obj->firstChild = 0;
        obj->lastChild = 0;
        obj->previousSibling = 0;
        obj->nextSibling = 0;
        obj->userData = 0;
        obj->vtable = vtable;
    }
    if (!obj) return 0;
    TiXmlString_AssignCStr((char*)&((TiXmlNodeLayout*)obj)->value,
                ((TiXmlNodeLayout*)self)->valueCapacity ? ((TiXmlNodeLayout*)self)->value : &s_tinyXmlEmptyString);
    obj->userData = ((TiXmlNodeLayout*)self)->userData;
    return obj;
}

// Clone a comment's text and user data.
void* TiXmlComment_Clone(void* self) {
    XmlNodeStorage* obj = (XmlNodeStorage*)__nw__FUl(0x28);
    if (obj) {
        obj->valueLength = 0; obj->valueData = 0; obj->parent = 0; obj->nodeType = kXmlComment;
        obj->firstChild = 0; obj->lastChild = 0; obj->previousSibling = 0; obj->nextSibling = 0;
        obj->userData = 0; obj->vtable = __vt__12TiXmlComment;
    }
    if (!obj) return 0;
    TiXmlString_AssignCStr((char*)&((TiXmlNodeLayout*)obj)->value,
                ((TiXmlNodeLayout*)self)->valueCapacity ? ((TiXmlNodeLayout*)self)->value : &s_tinyXmlEmptyString);
    obj->userData = ((TiXmlNodeLayout*)self)->userData;
    return obj;
}
// Clone an unknown node's text and user data.
void* TiXmlUnknown_Clone(void* self) {
    XmlNodeStorage* obj = (XmlNodeStorage*)__nw__FUl(0x28);
    if (obj) {
        obj->valueLength = 0; obj->valueData = 0; obj->parent = 0; obj->nodeType = kXmlUnknown;
        obj->firstChild = 0; obj->lastChild = 0; obj->previousSibling = 0; obj->nextSibling = 0;
        obj->userData = 0; obj->vtable = __vt__12TiXmlUnknown;
    }
    if (!obj) return 0;
    TiXmlString_AssignCStr((char*)&((TiXmlNodeLayout*)obj)->value,
                ((TiXmlNodeLayout*)self)->valueCapacity ? ((TiXmlNodeLayout*)self)->value : &s_tinyXmlEmptyString);
    obj->userData = ((TiXmlNodeLayout*)self)->userData;
    return obj;
}

// Clone a text node's value and user data.
void* TiXmlText_Clone(void* self) {
    XmlNodeStorage* obj = (XmlNodeStorage*)__nw__FUl(0x28);
    if (obj) {
        obj->valueLength = 0; obj->valueData = 0; obj->parent = 0; obj->nodeType = kXmlText;
        obj->firstChild = 0; obj->lastChild = 0; obj->previousSibling = 0; obj->nextSibling = 0;
        obj->userData = 0; obj->vtable = __vt__9TiXmlText;
        TiXmlString_AssignCStr((char*)&((TiXmlNodeLayout*)obj)->value, tinyXmlWriterStrings.emptyString);
    }
    if (!obj) return 0;
    TiXmlString_AssignCStr((char*)&((TiXmlNodeLayout*)obj)->value,
                ((TiXmlNodeLayout*)self)->valueCapacity ? ((TiXmlNodeLayout*)self)->value : &s_tinyXmlEmptyString);
    obj->userData = ((TiXmlNodeLayout*)self)->userData;
    return obj;
}

struct DeclarationInitStorage { void* vtable; int parent, nodeType, firstChild, lastChild, valueData, valueLength; int previousSibling, nextSibling, userData, versionData, versionLength, encodingData, encodingLength, standaloneData, standaloneLength; };
// Clone the node value and all three XML declaration attributes.
void* TiXmlDeclaration_Clone(void* self) {
    DeclarationInitStorage* obj = (DeclarationInitStorage*)__nw__FUl(0x40);
    if (obj) {
        obj->valueLength = 0; obj->valueData = 0; obj->parent = 0; obj->nodeType = kXmlDeclaration;
        obj->firstChild = 0; obj->lastChild = 0; obj->previousSibling = 0; obj->nextSibling = 0;
        obj->userData = 0; obj->vtable = __vt__16TiXmlDeclaration;
        obj->versionLength = 0; obj->versionData = 0; obj->encodingLength = 0; obj->encodingData = 0;
        obj->standaloneLength = 0; obj->standaloneData = 0;
    }
    if (!obj) return 0;
    TiXmlString_AssignCStr((char*)&((TiXmlNodeLayout*)obj)->value,
                ((TiXmlNodeLayout*)self)->valueCapacity ? ((TiXmlNodeLayout*)self)->value : &s_tinyXmlEmptyString);
    obj->userData = ((TiXmlNodeLayout*)self)->userData;
    TiXmlString_AssignString((char*)&AsTiXmlDeclaration(obj)->version, (char*)&AsTiXmlDeclaration(self)->version);
    TiXmlString_AssignString((char*)&AsTiXmlDeclaration(obj)->encoding, (char*)&AsTiXmlDeclaration(self)->encoding);
    TiXmlString_AssignString((char*)&AsTiXmlDeclaration(obj)->standalone, (char*)&AsTiXmlDeclaration(self)->standalone);
    return obj;
}

struct XmlNodeDestroyStorage { void** vtable; char pad04[8]; void* first; void* last; void* data; void* count; };
struct DestroyableXmlNode {
    virtual void destroy(int);
    char pad04[0x1c];
    DestroyableXmlNode* next;
};
// Destroy child nodes and the value string, then optionally delete this node.
void* TiXmlUnknown_dtor(XmlNodeDestroyStorage* self, int deleting) {
    if (self) {
        if (self) {
            DestroyableXmlNode* node = (DestroyableXmlNode*)self->first;
            self->vtable = (void**)__vt__9TiXmlNode;
            while (node) {
                DestroyableXmlNode* cur = node;
                node = node->next;
                if (cur) cur->destroy(1);
            }
            if (&self->data) {
                if (self->data) __dla__FPv(self->data);
                self->data = 0;
                self->count = 0;
            }
        }
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

struct DeclarationDestroyStorage { void* vtable; char pad04[8]; void* firstChild; char pad10[4]; void* valueData; void* valueLength; char pad1c[0x0c]; void* versionData; void* versionLength; void* encodingData; void* encodingLength; void* standaloneData; void* standaloneLength; };
// Release declaration strings, children and node text in reverse construction order.
void* TiXmlDeclaration_dtor(DeclarationDestroyStorage* self, int deleting) {
    if (self) {
        if ((char*)&AsTiXmlDeclaration(self)->standalone) {
            if (self->standaloneData) __dla__FPv(self->standaloneData);
            self->standaloneData = 0; self->standaloneLength = 0;
        }
        if ((char*)&AsTiXmlDeclaration(self)->encoding) {
            if (self->encodingData) __dla__FPv(self->encodingData);
            self->encodingData = 0; self->encodingLength = 0;
        }
        if ((char*)&AsTiXmlDeclaration(self)->version) {
            if (self->versionData) __dla__FPv(self->versionData);
            self->versionData = 0; self->versionLength = 0;
        }
        if (self) {
            self->vtable = __vt__9TiXmlNode;
            DestroyableXmlNode* node = (DestroyableXmlNode*)self->firstChild;
            while (node) {
                DestroyableXmlNode* cur = node;
                node = node->next;
                if (cur) cur->destroy(1);
            }
            if ((char*)&((TiXmlNodeLayout*)self)->value) {
                if (self->valueData) __dla__FPv(self->valueData);
                self->valueData = 0; self->valueLength = 0;
            }
        }
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

struct DocumentInitStorage {
    void* vtable;
    int parent, nodeType, firstChild, lastChild, valueData, valueLength;
    int previousSibling, nextSibling, userData;
    unsigned char error;
    char pad29[7];
    int errorText, errorTextLength;
};

// Construct an empty document with no children, user data or error.
void TiXmlDocument_Ctor(DocumentInitStorage* self) {
    self->valueLength = 0;
    self->valueData = 0;
    self->parent = 0;
    self->nodeType = kXmlDocument;
    self->firstChild = 0;
    self->lastChild = 0;
    self->previousSibling = 0;
    self->nextSibling = 0;
    self->userData = 0;
    self->vtable = (void*)__vt__13TiXmlDocument;
    self->errorTextLength = 0;
    self->errorText = 0;
    self->error = 0;
}

struct ParsableXmlDocument {
    virtual void* v0(); virtual void* v1(); virtual void* v2();
    virtual void* parse(void*);
    char pad04[8];
    void* first;
    void* last;
    char pad14[0x14];
    unsigned char error;
};

// Clear the document's children, parse the supplied input and report whether parsing succeeded.
int TiXmlDocument_ClearAndParse(ParsableXmlDocument* self, void* input) {
    struct Node {
        virtual void* destroy(int) DECOMP_LOCAL_VIRTUAL;
        char pad[0x1c]; Node* next;
    };
    Node* node = (Node*)self->first;
    while (node) {
        Node* cur = node;
        node = node->next;
        if (cur) cur->destroy(1);
    }
    self->first = 0;
    self->last = 0;
    self->parse(input);
    return self->error == 0;
}

struct AttributeDestroyStorage { char pad[8]; void* first; void* last; void* second; void* secondLast; };
// Destroy an XML attribute's value and name strings, then optionally delete the attribute.
void* TiXmlAttribute_dtor(AttributeDestroyStorage* self, int deleting) {
    if (self) {
        if (self) {
            if ((char*)&((TiXmlAttributeLayout*)self)->value) {
                if (self->second) __dla__FPv(self->second);
                self->second = 0; self->secondLast = 0;
            }
            if (&((TiXmlAttributeLayout*)self)->name) {
                if (self->first) __dla__FPv(self->first);
                self->first = 0; self->last = 0;
            }
        }
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

// Append an XML comment containing the entity-encoded node value.
void TiXmlComment_AppendCommentToString(void* self, void* output) {
    TiXmlString_Append(output, tinyXmlWriterStrings.commentOpen);
    struct { void* data; unsigned int count; } local;
    local.count = 0;
    local.data = 0;
    TiXmlString_EncodeEntities((char*)&((TiXmlNodeLayout*)self)->value, &local);
    TiXmlString_Append(output, local.count ? (const char*)local.data : &s_tinyXmlEmptyString);
    if (local.data) __dla__FPv(local.data);
    local.data = 0;
    local.count = 0;
    TiXmlString_Append(output, tinyXmlWriterStrings.commentClose);
}

// Append entity-encoded text to the output string.
void TiXmlText_AppendToString(void* self, void* output) {
    struct Container { void* data; unsigned int count; };
    Container local;
    local.count = 0;
    local.data = 0;
    TiXmlString_EncodeEntities((char*)&((TiXmlNodeLayout*)self)->value, &local);
    TiXmlString_Append(output, local.count != 0 ? (const char*)local.data : &s_tinyXmlEmptyString);
    if (local.data) __dla__FPv(local.data);
    local.data = 0;
    local.count = 0;
}

struct XmlTextValueView {
    char pad[0x14];
    const char* value;
    int valueLength;
};

struct EncodedXmlText {
    void* data;
    unsigned int count;
};

// Print the entity-encoded text value to the output stream.
void TiXmlText_PrintEncoded(XmlTextValueView* self, void* output) {
    EncodedXmlText local;
    local.count = 0;
    local.data = 0;
    TiXmlString_EncodeEntities((char*)&((TiXmlNodeLayout*)self)->value, &local);
    fprintf(output, tinyXmlWriterStrings.textFormat,
                local.count ? local.data : &s_tinyXmlEmptyString);
    if (local.data) __dla__FPv(local.data);
    local.data = 0;
    local.count = 0;
}

#pragma push
#pragma use_lmw_stmw on
// Print indentation followed by the unknown node's stored value.
void TiXmlUnknown_Print(void* self, void* output, int count) {
    for (int i = 0; i < count; ++i)
        fprintf(output, tinyXmlWriterStrings.indentSpace);
    struct UnknownXmlValueView { char pad[0x14]; const char* value; int valueLength; } *s = (UnknownXmlValueView*)self;
    fprintf(output, tinyXmlWriterStrings.textFormat,
                s->valueLength ? s->value : &s_tinyXmlEmptyString);
}
#pragma pop

struct StreamableXmlNode {
    virtual void v0(); virtual void v1(); virtual void append(void*);
    char pad04[4];
    int nodeType;
    char pad0c[0x14];
    StreamableXmlNode* next;
};
struct StreamableDocumentView { char pad[0x0c]; StreamableXmlNode* first; };
// Append top-level nodes through the first element, including any preceding declaration.
void TiXmlDocument_AppendNodesThroughFirstElement(const StreamableDocumentView* self, void* output) {
    StreamableXmlNode* node = self->first;
    while (node) {
        node->append(output);
        StreamableXmlNode* tmp = (node && node->nodeType == kXmlElement) ? node : 0;
        if (tmp) break;
        node = node->next;
    }
}

// Print an escaped name/value pair, choosing quotes that avoid literal double quotes.
void TiXmlAttribute_PrintToStream(const void* self, void* output) {
    struct Local { void* data; unsigned int count; } encodedName, encodedValue, tmpName, tmpValue;
    encodedName.count = 0; encodedName.data = 0;
    encodedValue.count = 0; encodedValue.data = 0;
    __ct__11TiXmlStringFPCc(&tmpName, ((TiXmlAttributeLayout*)self)->nameLength ? ((TiXmlAttributeLayout*)self)->name : &s_tinyXmlEmptyString);
    TiXmlString_EncodeEntities(&tmpName, &encodedName);
    if (tmpName.data) __dla__FPv(tmpName.data);
    tmpName.data = 0; tmpName.count = 0;
    __ct__11TiXmlStringFPCc(&tmpValue, ((TiXmlAttributeLayout*)self)->valueLength ? ((TiXmlAttributeLayout*)self)->value : &s_tinyXmlEmptyString);
    TiXmlString_EncodeEntities(&tmpValue, &encodedValue);
    if (tmpValue.data) __dla__FPv(tmpValue.data);
    tmpValue.data = 0; tmpValue.count = 0;
    unsigned int doubleQuoteIndex = TiXmlString_FindChar((char*)&((TiXmlAttributeLayout*)self)->value, '"', 0);
    if (doubleQuoteIndex == (unsigned int)-1) {
        fprintf(output, tinyXmlWriterStrings.doubleQuotedAttributeFormat,
                    encodedName.count ? encodedName.data : &s_tinyXmlEmptyString,
                    encodedValue.count ? encodedValue.data : &s_tinyXmlEmptyString);
    } else {
        fprintf(output, tinyXmlWriterStrings.singleQuotedAttributeFormat,
                    encodedName.count ? encodedName.data : &s_tinyXmlEmptyString,
                    encodedValue.count ? encodedValue.data : &s_tinyXmlEmptyString);
    }
    if (encodedValue.data) __dla__FPv(encodedValue.data);
    encodedValue.data = 0; encodedValue.count = 0;
    if (encodedName.data) __dla__FPv(encodedName.data);
    encodedName.data = 0; encodedName.count = 0;
}

#pragma push
#pragma use_lmw_stmw on
#pragma opt_common_subs off
#pragma opt_lifetimes off
// Append an escaped name/value pair using single or double quotation marks.
void TiXmlAttribute_AppendNameAndQuotedValue(void* self, void* output) {
    struct Local { void* data; unsigned int count; };
    Local singleQuotedName, singleQuotedValue, doubleQuotedName, doubleQuotedValue;
    void* attribute = self;
    void* destination = output;
    int zero = 0;
    unsigned int doubleQuoteIndex = TiXmlString_FindChar((char*)&((TiXmlAttributeLayout*)attribute)->value, '"', 0);
    if (doubleQuoteIndex != (unsigned int)-1) {
        singleQuotedName.count = zero; singleQuotedName.data = (void*)zero;
        TiXmlString_EncodeEntities((char*)&((TiXmlAttributeLayout*)attribute)->name, &singleQuotedName);
        TiXmlString_Append(destination, singleQuotedName.count ? (const char*)singleQuotedName.data : &s_tinyXmlEmptyString);
        if (singleQuotedName.data) __dla__FPv(singleQuotedName.data);
        singleQuotedName.data = (void*)zero; singleQuotedName.count = zero;
        const char* quote = tinyXmlWriterStrings.singleQuote;
        TiXmlString_Append(destination, tinyXmlWriterStrings.equals);
        TiXmlString_Append(destination, quote);
        singleQuotedValue.count = zero; singleQuotedValue.data = (void*)zero;
        TiXmlString_EncodeEntities((char*)&((TiXmlAttributeLayout*)attribute)->value, &singleQuotedValue);
        TiXmlString_Append(destination, singleQuotedValue.count ? (const char*)singleQuotedValue.data : &s_tinyXmlEmptyString);
        if (singleQuotedValue.data) __dla__FPv(singleQuotedValue.data);
        singleQuotedValue.data = (void*)zero; singleQuotedValue.count = zero;
        TiXmlString_Append(destination, tinyXmlWriterStrings.singleQuote);
    } else {
        doubleQuotedName.count = zero; doubleQuotedName.data = (void*)zero;
        TiXmlString_EncodeEntities((char*)&((TiXmlAttributeLayout*)attribute)->name, &doubleQuotedName);
        TiXmlString_Append(destination, doubleQuotedName.count ? (const char*)doubleQuotedName.data : &s_tinyXmlEmptyString);
        if (doubleQuotedName.data) __dla__FPv(doubleQuotedName.data);
        doubleQuotedName.data = (void*)zero; doubleQuotedName.count = zero;
        const char* quote = tinyXmlWriterStrings.doubleQuote;
        TiXmlString_Append(destination, tinyXmlWriterStrings.equals);
        TiXmlString_Append(destination, quote);
        doubleQuotedValue.count = zero; doubleQuotedValue.data = (void*)zero;
        TiXmlString_EncodeEntities((char*)&((TiXmlAttributeLayout*)attribute)->value, &doubleQuotedValue);
        TiXmlString_Append(destination, doubleQuotedValue.count ? (const char*)doubleQuotedValue.data : &s_tinyXmlEmptyString);
        if (doubleQuotedValue.data) __dla__FPv(doubleQuotedValue.data);
        doubleQuotedValue.data = (void*)zero; doubleQuotedValue.count = zero;
        TiXmlString_Append(destination, tinyXmlWriterStrings.doubleQuote);
    }
}
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma pop

// Append the declaration and its nonempty, entity-encoded attributes.
void TiXmlDeclaration_AppendTagAndEncodedAttributes(void* self, void* output) {
    char* s = (char*)self;
    TiXmlString_Append(output, tinyXmlWriterStrings.declarationOpen);
    if (!XmlStringIsEmpty(s + (int)&((TiXmlDeclarationLayout*)0)->version)) {
        TiXmlString_Append(output, tinyXmlWriterStrings.versionOpen);
        struct Local { void* data; unsigned int count; } local;
        local.count = 0; local.data = 0;
        TiXmlString_EncodeEntities(s + (int)&((TiXmlDeclarationLayout*)0)->version, &local);
        TiXmlString_Append(output, local.count ? (const char*)local.data : &s_tinyXmlEmptyString);
        if (local.data) __dla__FPv(local.data);
        local.data = 0; local.count = 0;
        TiXmlString_Append(output, tinyXmlWriterStrings.attributeClose);
    }
    if (!XmlStringIsEmpty(s + (int)&((TiXmlDeclarationLayout*)0)->encoding)) {
        TiXmlString_Append(output, tinyXmlWriterStrings.encodingOpen);
        struct Local { void* data; unsigned int count; } local;
        local.count = 0; local.data = 0;
        TiXmlString_EncodeEntities(s + (int)&((TiXmlDeclarationLayout*)0)->encoding, &local);
        TiXmlString_Append(output, local.count ? (const char*)local.data : &s_tinyXmlEmptyString);
        if (local.data) __dla__FPv(local.data);
        local.data = 0; local.count = 0;
        TiXmlString_Append(output, tinyXmlWriterStrings.attributeClose);
    }
    if (!XmlStringIsEmpty(s + (int)&((TiXmlDeclarationLayout*)0)->standalone)) {
        TiXmlString_Append(output, tinyXmlWriterStrings.standaloneOpen);
        struct Local { void* data; unsigned int count; } local;
        local.count = 0; local.data = 0;
        TiXmlString_EncodeEntities(s + (int)&((TiXmlDeclarationLayout*)0)->standalone, &local);
        TiXmlString_Append(output, local.count ? (const char*)local.data : &s_tinyXmlEmptyString);
        if (local.data) __dla__FPv(local.data);
        local.data = 0; local.count = 0;
        TiXmlString_Append(output, tinyXmlWriterStrings.attributeClose);
    }
    TiXmlString_Append(output, tinyXmlWriterStrings.declarationClose);
}

struct TiXmlString {
    void* data;
    unsigned int count;
    TiXmlString() { count = 0; data = 0; }
    TiXmlString(const char*);
    int operator==(const TiXmlString& other) const {
        return TiXmlString_Equals((void*)this, (void*)&other);
    }
    ~TiXmlString() {
        if (data) __dla__FPv(data);
        data = 0;
        count = 0;
    }
};
struct CloneElementBase {
    void* vtable;
    void* parent;
    int type;
    void* first;
    void* last;
    struct Value {
        void* data;
        unsigned int count;
        Value() { count = 0; data = 0; }
    } value;
    void* prev;
    void* next;
    void* user;
    CloneElementBase() {
        parent = 0; type = 1; first = 0; last = 0;
        prev = 0; next = 0; user = 0;
        vtable = __vt__12TiXmlElement;
    }
    ~CloneElementBase() { TiXmlNode_dtor(this, -1); }
};
void TiXmlAttributeBase_dtor(void*, int);
struct CloneAttributeBase {
    virtual ~CloneAttributeBase() { TiXmlAttributeBase_dtor(this, -1); }
    char bytes[4];
};
struct TiXmlAttribute : CloneAttributeBase {
    TiXmlString name;
    TiXmlString value;
    void* prev;
    void* next;
    TiXmlAttribute() { next = this; prev = this; }
    TiXmlAttribute(const char* n, const char* v) : name(n), value(v) {
        prev = 0;
        next = 0;
    }
    virtual ~TiXmlAttribute() {}
};
typedef TiXmlAttribute CloneAttribute;
struct CloneAttributeSet {
    CloneAttribute sentinel;
    CloneAttribute* FindCloneAttribute(const char* name);
};
struct CloneElement : CloneElementBase {
    CloneAttributeSet attributes;
    CloneElement(const char* str) {
        last = 0; first = 0;
        TiXmlString_AssignCStr(&value, str);
    }
    void SetCloneAttribute(const char*, const char*);
};

inline CloneAttribute* CloneAttributeSet::FindCloneAttribute(const char* name) {
    CloneAttribute* node = (CloneAttribute*)sentinel.next;
    while (node != &sentinel) {
        if (node->name == name) return node;
        node = (CloneAttribute*)node->next;
    }
    return 0;
}

// Find the owning document by walking parent links.
inline void* FindCloneDocument(void* ancestor) {
    while (ancestor) {
        int isDocument = (ancestor && ((TiXmlNodeLayout*)ancestor)->nodeType == 0);
        void* document = isDocument ? ancestor : 0;
        if (document) {
            int ancestorIsDocument = (ancestor && ((TiXmlNodeLayout*)ancestor)->nodeType == 0);
            if (ancestorIsDocument) return ancestor;
            return 0;
        }
        ancestor = ((TiXmlNodeLayout*)ancestor)->parent;
    }
    return 0;
}
// Update an existing attribute or append a new one; record allocation errors on the document.
inline void CloneElement::SetCloneAttribute(const char* name, const char* valueText) {
        CloneAttribute* attribute;
        CloneElement* element = this;
        CloneAttribute* existing = attributes.FindCloneAttribute(name);

        if (existing) {
            TiXmlString_AssignCStr((char*)&existing->value, valueText);
            return;
        }
        {
            attribute = new CloneAttribute(name, valueText);
            if (attribute) {
                attribute->next = &element->attributes.sentinel;
                attribute->prev = element->attributes.sentinel.prev;
                ((CloneAttribute*)element->attributes.sentinel.prev)->next = attribute;
                element->attributes.sentinel.prev = attribute;
            } else {
                void* document = FindCloneDocument(element);
                if (document) {
                    ((TiXmlNodeLayout*)document)->documentError = 1;
                    ((TiXmlNodeLayout*)document)->documentErrorId = kXmlOutOfMemory;
                    const char* errorMessage = tinyXmlErrorMessages[kXmlOutOfMemory];
                    TiXmlString_AssignCStr((char*)&((DocumentCloneStorage*)document)->errorText, errorMessage);
                }
            }
        }
}

#pragma push
#pragma use_lmw_stmw on
// Deep-clone an element, copying its attributes, user data and child nodes.
void* TiXmlElement_CloneElement(const void* self) {
    CloneElement* clone = new CloneElement(((TiXmlNodeLayout*)self)->valueCapacity ? ((TiXmlNodeLayout*)self)->value : &s_tinyXmlEmptyString);
    if (!clone) return 0;
    const char* elementName = ((TiXmlNodeLayout*)self)->valueCapacity ? ((TiXmlNodeLayout*)self)->value : &s_tinyXmlEmptyString;
    TiXmlString_AssignCStr((char*)&clone->value, elementName);
    ((TiXmlNodeLayout*)clone)->userData = ((TiXmlNodeLayout*)self)->userData;

    void* attribute = ((TiXmlNodeLayout*)self)->firstAttribute;
    if (attribute == (void*)&((CloneElement*)self)->attributes.sentinel) attribute = 0;

    while (attribute) {
        clone->SetCloneAttribute(
            ((TiXmlAttributeLayout*)attribute)->nameLength ? ((TiXmlAttributeLayout*)attribute)->name : &s_tinyXmlEmptyString,
            ((TiXmlAttributeLayout*)attribute)->valueLength ? ((TiXmlAttributeLayout*)attribute)->value : &s_tinyXmlEmptyString);
        int nextValueLength = TiXmlString_Length((char*)&((TiXmlAttributeLayout*)((TiXmlAttributeLayout*)attribute)->next)->value);
        if ((unsigned int)__cntlzw((unsigned int)nextValueLength) >> 5) {
            int nextNameLength = TiXmlString_Length((char*)&((TiXmlAttributeLayout*)((TiXmlAttributeLayout*)attribute)->next)->name);
            if ((unsigned int)__cntlzw((unsigned int)nextNameLength) >> 5) {
                attribute = 0;
                continue;
            }
        }
        attribute = ((TiXmlAttributeLayout*)attribute)->next;
    }

    CloneableXmlNode* node = (*(CloneableXmlNode**)&((TiXmlNodeLayout*)self)->firstChild);
    void* zero = 0;
    (void)zero;
    while (node) {
        void* child = node->clone();
        ((TiXmlNodeLayout*)child)->parent = clone;
        ((TiXmlNodeLayout*)child)->previousSibling = ((TiXmlNodeLayout*)clone)->lastChild;
        ((TiXmlNodeLayout*)child)->nextSibling = 0;
        void* tail = ((TiXmlNodeLayout*)clone)->lastChild;
        if (tail) ((TiXmlNodeLayout*)tail)->nextSibling = child;
        else ((TiXmlNodeLayout*)clone)->firstChild = child;
        ((TiXmlNodeLayout*)clone)->lastChild = child;
        node = node->next;
    }

    return clone;
}
#pragma pop

} // extern "C"
