// TinyXML parser: reads text, comments, declarations, attributes and elements, and checks
// whitespace against the locale table.
// Also the error message strings and the destructors for parser-owned pairs and text nodes.

#include "classes/TiXmlNodeLayout.h"
#include "classes/TiXmlAttributeLayout.h"
#define XML_OFFSET(type, field) ((int)&((type*)0)->field)
#define AsTiXmlNode(p) ((TiXmlNodeLayout*)(p))
#define AsTiXmlAttribute(p) ((TiXmlAttributeLayout*)(p))
// 0x802A0FF0 unit: node strings at +0x14, attribute strings at +8/+0x10.
// Parser contract: input/cursors borrow a NUL-terminated byte string; node/attribute values
// copy text into owned strings. Virtual slot 3 is parsing, not drawing/updating.
// Success normally returns the first unconsumed input byte. A null return indicates failure;
// document error fields are written where shown (some callers also require a document ancestor).
// Preserve legacy malformed-input behavior: these routines are not a validating/bounds-safe XML API.

extern "C" {

void __dl__FPv(void*);
void TiXmlNode_dtor(void*, int);
void __dla__FPv(void*);

// Release an otherwise trivial node wrapper when called as a deleting destructor.
void* TiXmlAttributeBase_dtor(void* self, int flag) {
    if (self) {
        if (flag > 0)
            __dl__FPv(self);
    }
    return self;
}

// Destroy the common node state and optionally release the text node.
void* TiXmlText_dtor(void* self, int flag) {
    if (self) {
        TiXmlNode_dtor(self, 0);
        if (flag > 0)
            __dl__FPv(self);
    }
    return self;
}

struct Container {
    void* data;
    unsigned int count;
};

#pragma opt_common_subs off
#pragma opt_propagation off
#pragma use_lmw_stmw on
// Byte layout of the shared parser string pool (including its leading empty string).
struct XmlParserStringPool {
    char empty[1];
    char hexEntityPrefix[4];
    char declarationOpen[6];
    char commentOpen[5];
    char closingTagOpen[3];
    char tagClose[2];
    char cdataOpen[10];
    char cdataClose[4];
    char commentClose[4];
    char singleQuote[2];
    char doubleQuote[2];
    char tagOpen[2];
    char version[8];
    char encoding[9];
    char standalone[11];
    char trailingPadding[7];
};
enum XmlError {
    kXmlErrorOutOfMemory = 3,
    kXmlErrorElement = 4,
    kXmlErrorElementName = 5,
    kXmlErrorElementValue = 6,
    kXmlErrorAttributes = 7,
    kXmlErrorEmptyTag = 8,
    kXmlErrorEndTag = 9,
    kXmlErrorUnknown = 10,
    kXmlErrorComment = 11,
    kXmlErrorDeclaration = 12,
    kXmlErrorEmptyDocument = 13
};
enum XmlNodeType {
    kXmlElement = 1,
    kXmlComment = 2,
    kXmlUnknown = 3,
    kXmlText = 4,
    kXmlDeclaration = 5
};
enum {
    kCharacterCount = 0x100,
    kLocaleSpaceFlag = 0x100,
    kLocaleAlphaFlag = 1
};

extern XmlParserStringPool tinyXmlParserStrings;
void TiXmlString_AssignCStr(void*, const char*);
char* TiXmlBase_ReadText(const char*, void*, int, const char*, int);
struct Parser {
    char pad[0x14];
};
// Read normalized text up to the next opening bracket and leave the bracket for the caller.
char* TiXmlText_ReadText(Parser* self, const char* text) {
    TiXmlString_AssignCStr(&AsTiXmlNode(self)->value, tinyXmlParserStrings.empty);
    const char* base = tinyXmlParserStrings.empty;
    const char* suffix = (base + XML_OFFSET(XmlParserStringPool, tagOpen));
    char* result = TiXmlBase_ReadText(text, &AsTiXmlNode(self)->value, 1, suffix, 0);
    return result ? result - 1 : 0;
}

struct PairObject {
    char pad[8];
    Container first;
    Container second;
};
// Destroy the two owned strings, then optionally release the attribute storage.
PairObject* PairObject_dtor(PairObject* self, int flag) {
    if (self) {
        if (&self->second) {
            if (self->second.data)
                __dla__FPv(self->second.data);
            self->second.data = 0;
            self->second.count = 0;
        }
        if (&self->first) {
            if (self->first.data)
                __dla__FPv(self->first.data);
            self->first.data = 0;
            self->first.count = 0;
        }
        if (flag > 0)
            __dl__FPv(self);
    }
    return self;
}

// Document error fields start after the common node prefix.
struct ErrorState {
    char pad[0x28];
    unsigned char error;
    char pad29[3];
    int code;
    char message[8];
};
struct AttributeOwnerView {
    void* vtable;
    ErrorState* document;
};
ErrorState* TiXmlNode_GetDocumentAncestor(void*);
int TiXml_MatchesPrefix(const char*, const char*, int);
char* TiXmlBase_ReadName(const char*, void*);
unsigned long strlen(const char*);
struct LocaleTable {
    char pad[8];
    unsigned short* flags;
};
struct LocaleGlobal {
    char pad[0x38];
    LocaleTable* table;
};
extern LocaleGlobal s_cLocaleData;
struct ErrorStrings {
    const char* entries[14];
};
extern ErrorStrings tinyXmlErrorMessages;
#pragma opt_strength_reduction on
#pragma opt_propagation on
#pragma peephole on
#pragma opt_common_subs on
// Whitespace helper map (all recognize locale space plus explicit LF/CR):
// spaceFlag/skipSpace: reload the global locale through spaceFlag; input byte converted unsigned.
// skipJobSpace: same lookup, explicit signed-char NUL tests retained from comment parsing.
// spaceFlagCached/skipSpaceCached: capture the locale table once for the scan.
// targetSpaceFlag/skipTargetSpace: same captured table with reversed helper argument order.
// Initial null/empty input becomes null; an all-space nonempty string stops at its NUL pointer.
// Keep variants separate: their signedness, locale reads and pragma regions affect generated code.
// Test one byte against the current locale's space class, returning zero outside [0,255].
inline int spaceFlag(int c) {
    int invalid = 0;
    if (c < 0 || c >= kCharacterCount)
        invalid = 1;
    return invalid ? 0 : s_cLocaleData.table->flags[c] & kLocaleSpaceFlag;
}
// Skip leading spaces using the current locale and leave the first non-space byte unconsumed.
inline const char* skipSpace(const char* text) {
    if (!text || !*text)
        text = 0;
    else {
        while (text && *text) {
            int c = (unsigned char)*text;
            int space = spaceFlag(c);
            if (!space && (signed char)c != 10 && (signed char)c != 13)
                break;
            ++text;
        }
    }
    return text;
}
#pragma opt_common_subs off
#pragma peephole off
#pragma opt_propagation on

#pragma opt_common_subs on
#pragma peephole on
// Skip comment-parser spaces while retaining its signed-byte termination tests.
inline const char* skipJobSpace(const char* text) {
    if (!text || !(signed char)*text)
        text = 0;
    else {
        int c;
        while (text && (c = (unsigned char)*text, (signed char)c != 0)) {
            int space = spaceFlag(c);
            if (!space && (signed char)c != 10 && (signed char)c != 13)
                break;
            ++text;
        }
    }
    return text;
}
#pragma opt_common_subs off
#pragma opt_propagation on
// Validate a comment opener and read its contents through the closing comment delimiter.
char* TiXmlComment_Parse(Parser* self, const char* text) {
    register ErrorState* state = TiXmlNode_GetDocumentAncestor(self);
    TiXmlString_AssignCStr(&AsTiXmlNode(self)->value, tinyXmlParserStrings.empty);
    text = skipJobSpace(text);
    const char* prefix = tinyXmlParserStrings.commentOpen;
    const char* suffix = tinyXmlParserStrings.commentClose;
    if (!TiXml_MatchesPrefix(text, prefix, 0)) {
        ErrorStrings* strings = &tinyXmlErrorMessages;
        state->error = 1;
        void* message = state->message;
        state->code = kXmlErrorComment;
        TiXmlString_AssignCStr(message, strings->entries[kXmlErrorComment]);
        return 0;
    }
    return TiXmlBase_ReadText(text + strlen(prefix), &AsTiXmlNode(self)->value, 0, suffix, 0);
}

#pragma opt_propagation off
#pragma peephole on
} // extern "C"
struct TiXmlString {
    char* str;
    int length;
    TiXmlString(const char*);
    ~TiXmlString();
};
// Release the owned character buffer and reset the empty-string state.
TiXmlString::~TiXmlString() {
    if (str)
        delete[] str;
    str = 0;
    length = 0;
}
extern "C" {

#pragma opt_common_subs on
#pragma opt_propagation on
#pragma peephole on
struct ParseNode;
struct ParseNode {
    virtual void slot0(int);
    virtual void slot1();
    virtual void slot2();
    virtual const char* parse(const char*);
};
ParseNode* TiXmlNode_Identify(void*, const char*);
void TiXmlNode_LinkEndChild(void*, ParseNode*);
int String_ComparePrefixIgnoreCase(const char*, const char*, unsigned long);
void TiXmlString_Append(void*, const char*);
// Borrow the static error message for a valid XmlError index; no range validation is performed.
inline const char* errorString(int index) {
    return tinyXmlErrorMessages.entries[index];
}
// Parse and link top-level nodes, recording an error for empty input.
const char* TiXmlDocument_ParseTopLevelNodes(ErrorState* self, const char* text) {
    if (!text || !*text) {
        self->error = 1;
        self->code = kXmlErrorEmptyDocument;
        TiXmlString_AssignCStr(self->message, errorString(kXmlErrorEmptyDocument));
        return 0;
    }
    text = skipSpace(text);
    if (!text) {
        self->error = 1;
        self->code = kXmlErrorEmptyDocument;
        TiXmlString_AssignCStr(self->message, errorString(kXmlErrorEmptyDocument));
        return 0;
    }
    while (text && *text) {
        ParseNode* node = TiXmlNode_Identify(self, text);
        if (!node)
            break;
        text = node->parse(text);
        TiXmlNode_LinkEndChild(self, node);
        text = skipSpace(text);
    }
    return text;
}
#pragma opt_common_subs off
#pragma opt_propagation on
#pragma opt_loop_invariants off
#pragma opt_strength_reduction off
// Test a byte against a caller-supplied locale snapshot, rejecting out-of-range values.
inline int spaceFlagCached(int c, LocaleTable* table) {
    int invalid = 0;
    if (c < 0 || c >= kCharacterCount)
        invalid = 1;
    return invalid ? 0 : table->flags[c] & kLocaleSpaceFlag;
}
// Skip leading spaces with one locale-table snapshot for this complete scan.
inline const char* skipSpaceCached(const char* text) {
    if (!text || !*text)
        text = 0;
    else {
        int c;
        LocaleTable* table = s_cLocaleData.table;
        while (text && (c = (unsigned char)*text, (signed char)c != 0)) {
            int space = spaceFlagCached(c, table);
            if (!space && (signed char)c != 10 && (signed char)c != 13)
                break;
            ++text;
        }
    }
    return text;
}
// Read CDATA or an unrecognized tag into the node value.
const char* TiXmlUnknown_Read(Parser* inputSelf, const char* inputText) {
    register int tag = 0;
    const char* text = inputText;
    Parser* self = inputSelf;
    ErrorState* state = TiXmlNode_GetDocumentAncestor(self);
    text = skipSpaceCached(text);
    if (!text || !*text) {
        if (state) {
            state->error = 1;
            state->code = kXmlErrorUnknown;
            TiXmlString_AssignCStr(state->message, errorString(kXmlErrorUnknown));
        }
        return 0;
    }
    TiXmlString_AssignCStr(&AsTiXmlNode(self)->value, tinyXmlParserStrings.empty);
    if (String_ComparePrefixIgnoreCase(text, tinyXmlParserStrings.cdataOpen, 9) == 0) {
        text += 9;
        tag = 0;
    } else if ((unsigned char)*text == '<') {
        tag = 1;
        ++text;
    }
    if (!tag) {
        while (text && *text && String_ComparePrefixIgnoreCase(text, tinyXmlParserStrings.cdataClose, 3) != 0) {
            char buf[2];
            buf[0] = *text;
            buf[1] = 0;
            TiXmlString_Append(&AsTiXmlNode(self)->value, buf);
            ++text;
        }
        text += 3;
    } else {
        unsigned char c;
        while (text && (c = (unsigned char)*text, (signed char)c != 0) && (signed char)c != '>') {
            char buf[2];
            buf[0] = c;
            buf[1] = 0;
            TiXmlString_Append(&AsTiXmlNode(self)->value, buf);
            ++text;
        }
    }
    if (!text && state) {
        state->error = 1;
        state->code = kXmlErrorUnknown;
        TiXmlString_AssignCStr(state->message, errorString(kXmlErrorUnknown));
    }
    if ((unsigned char)*text == '>')
        return text + 1;
    return text;
}
extern unsigned char s_tinyXmlCondenseWhitespace;
const char* TiXmlBase_ReadEntity(const char*, char*);
#pragma opt_common_subs on
// Decode an entity or copy one byte to out, returning the first byte after that character/entity.
inline const char* decodeChar(int c, const char* text, char* out) {
    if (c == '&')
        text = TiXmlBase_ReadEntity(text, out);
    else {
        *out = c;
        text = text + 1;
    }
    return text;
}
// Test a byte against the text-reader's locale snapshot, rejecting out-of-range values.
inline int targetSpaceFlag(LocaleTable* table, int c) {
    int invalid = 0;
    if (c < 0 || c >= kCharacterCount)
        invalid = 1;
    return invalid ? 0 : table->flags[c] & kLocaleSpaceFlag;
}
// Skip text-reader leading spaces using one captured locale table.
inline const char* skipTargetSpace(const char* text) {
    if (!text || !*text)
        text = 0;
    else {
        LocaleTable* table = s_cLocaleData.table;
        int c;
        while (text && (c = (unsigned char)*text, (signed char)c != 0)) {
            int space = targetSpaceFlag(table, c);
            if (!space && (signed char)c != 10 && (signed char)c != 13)
                break;
            ++text;
        }
    }
    return text;
}
#pragma opt_propagation off
#pragma peephole on
#pragma opt_strength_reduction on
// Decode entities into a string, optionally collapsing whitespace, until the end delimiter.
// out owns copied bytes; end is a borrowed delimiter, mode passes through to prefix matching.
// Return is the stopping cursor + strlen(end), even if scanning stopped at NUL; callers must
// supply the expected delimiter. TiXmlText_ReadText subtracts one to leave '<' unconsumed.
char* TiXmlBase_ReadText(const char* text, void* out, int normalize, const char* end, int mode) {
    TiXmlString_AssignCStr(out, tinyXmlParserStrings.empty);
    if (!normalize || !s_tinyXmlCondenseWhitespace) {
        while (text && *text && !TiXml_MatchesPrefix(text, end, mode)) {
            char c;
            text = decodeChar((unsigned char)*text, text, &c);
            char buf[2];
            buf[0] = c;
            buf[1] = 0;
            TiXmlString_Append(out, buf);
        }
    } else {
        int pending = 0;
        text = skipTargetSpace(text);
        while (text && *text && !TiXml_MatchesPrefix(text, end, mode)) {
            int c = (unsigned char)*text;
            if ((signed char)c == 13 || (signed char)c == 10) {
                pending = 1;
                ++text;
            } else if (spaceFlag(c)) {
                pending = 1;
                ++text;
            } else {
                if (pending) {
                    char buf[2];
                    buf[0] = ' ';
                    buf[1] = 0;
                    TiXmlString_Append(out, buf);
                    pending = 0;
                }
                char decoded;
                text = decodeChar((unsigned char)*text, text, &decoded);
                char buf[2];
                buf[0] = decoded;
                buf[1] = 0;
                TiXmlString_Append(out, buf);
            }
        }
    }
    return (char*)text + strlen(end);
}
void TiXmlNode_Ctor(void*, int);
void TiXmlElement_Ctor(void*, const char*);
extern char __vt__16TiXmlDeclaration[];
extern char __vt__12TiXmlComment[];
extern char __vt__12TiXmlUnknown[];
struct Node40 {
    void* vtable;
    void* owner;
    char pad[0x20];
    Container a, b, c;
    // Construct a declaration node with three empty owned declaration strings.
    Node40() {
        TiXmlNode_Ctor(this, kXmlDeclaration);
        vtable = __vt__16TiXmlDeclaration;
        a.count = 0;
        a.data = 0;
        b.count = 0;
        b.data = 0;
        c.count = 0;
        c.data = 0;
    }
};
struct Node48 {
    char pad[0x48];
    // Construct an element node using a copied initial name.
    Node48(const char* s) {
        TiXmlElement_Ctor(this, s);
    }
};
struct Node28A {
    void* vtable;
    char pad[0x24];
    // Construct the common node state and install the comment-node vtable.
    Node28A() {
        TiXmlNode_Ctor(this, kXmlComment);
        vtable = __vt__12TiXmlComment;
    }
};
struct Node28B {
    void* vtable;
    char pad[0x24];
    // Construct the common node state and install the unknown-node vtable.
    Node28B() {
        TiXmlNode_Ctor(this, kXmlUnknown);
        vtable = __vt__12TiXmlUnknown;
    }
};
#pragma opt_common_subs on
#pragma opt_propagation on
#pragma opt_strength_reduction on
#pragma opt_loop_invariants on
// Test a byte for locale alphabetic class, returning zero outside the byte range.
inline int alphaFlag(int c) {
    int invalid = 0;
    if (c < 0 || c >= kCharacterCount)
        invalid = 1;
    return invalid ? 0 : s_cLocaleData.table->flags[c] & kLocaleAlphaFlag;
}
#pragma opt_common_subs on
#pragma opt_propagation off
// Choose and allocate a declaration, element, comment, or unknown node from its opener.
// Returned node is newly allocated and parent-linked but not in the child list until LinkEndChild.
ParseNode* TiXmlNode_Identify(void* self, const char* input) {
    register ParseNode* node;
    const char* text;
    if (!input || !*input)
        text = 0;
    else {
        while (input && *input) {
            int c = (unsigned char)*input;
            int space = spaceFlag(c);
            if (!space && (signed char)c != 10 && (signed char)c != 13)
                break;
            ++input;
        }
        text = input;
    }
    if (!text || !*text || *text != '<')
        return 0;
    ErrorState* state = TiXmlNode_GetDocumentAncestor(self);
    text = skipSpace(text);
    if (!text || !*text)
        return 0;
    const char* base = tinyXmlParserStrings.empty;
    const char* prefix = (base + XML_OFFSET(XmlParserStringPool, declarationOpen));
    const char* suffix = (base + XML_OFFSET(XmlParserStringPool, commentOpen));
    if (TiXml_MatchesPrefix(text, prefix, 1)) {
        node = (ParseNode*)new Node40;
    } else {
        unsigned char c = text[1];
        int invalid = 0;
        int ch = (signed char)c;
        if (ch < 0 || ch >= kCharacterCount)
            invalid = 1;
        int alpha = invalid ? 0 : s_cLocaleData.table->flags[ch] & kLocaleAlphaFlag;
        if (alpha || (signed char)c == '_') {
            node = (ParseNode*)new Node48(tinyXmlParserStrings.empty);
        } else if (TiXml_MatchesPrefix(text, suffix, 0)) {
            node = (ParseNode*)new Node28A;
        } else {
            node = (ParseNode*)new Node28B;
        }
    }
    if (node)
        AsTiXmlNode(node)->parent = self;
    else if (state) {
        state->error = 1;
        state->code = kXmlErrorOutOfMemory;
        TiXmlString_AssignCStr(state->message, errorString(kXmlErrorOutOfMemory));
    }
    return node;
}
extern char __vt__9TiXmlText[];
unsigned int TiXmlString_Length(void*);
struct TextBase {
    void* vtable;
    char pad[0x10];
    // Initialize the shared node prefix as an XML text node.
    TextBase() {
        TiXmlNode_Ctor(this, kXmlText);
    }
    // Destroy shared node state without freeing this enclosing text-node allocation.
    ~TextBase() {
        TiXmlNode_dtor(this, 0);
    }
};
struct TextNode : TextBase {
    char string[0x14];
    // Install text parsing behavior and copy the initial owned text string.
    TextNode(const char* base) {
        vtable = __vt__9TiXmlText;
        TiXmlString_AssignCStr(string, base);
    }
};
#pragma opt_propagation on
#pragma opt_common_subs off
// Report whether every owned text byte is locale whitespace (including the empty string).
inline int textAllSpace(TextNode* node) {
    for (unsigned int i = 0; i < TiXmlString_Length(node->string); ++i) {
        int c = ((unsigned char*)*(char**)node->string)[i];
        int invalid = 0;
        if (c < 0 || c >= kCharacterCount)
            invalid = 1;
        int space = invalid ? 0 : s_cLocaleData.table->flags[c] & kLocaleSpaceFlag;
        if (!space)
            return 0;
    }
    return 1;
}
#pragma opt_common_subs on
#pragma opt_propagation off
#pragma opt_loop_invariants on
#pragma opt_propagation on
#pragma peephole on
#pragma opt_common_subs on
#pragma opt_strength_reduction on
// Parse element text and children until a closing-tag opener is reached.
// Returns the borrowed pointer AT '</', leaving the closing tag for Element_vfunc_3.
// Linked children become parent-owned; whitespace-only text nodes are deleted locally.
const char* TiXmlElement_ReadValue(void* self, const char* text) {
    ErrorState* state = TiXmlNode_GetDocumentAncestor(self);
    text = skipSpace(text);
    register const char* base = tinyXmlParserStrings.empty;
    while (text && *text) {
        if (*text != '<') {
            TextNode* node = new TextNode(base);
            if (!node) {
                if (state) {
                    state->error = 1;
                    state->code = kXmlErrorOutOfMemory;
                    TiXmlString_AssignCStr(state->message, errorString(kXmlErrorOutOfMemory));
                }
                return 0;
            }
            text = ((ParseNode*)node)->parse(text);
            if (!textAllSpace(node))
                TiXmlNode_LinkEndChild(self, (ParseNode*)node);
            else if (node)
                ((ParseNode*)node)->slot0(1);
        } else {
            if (TiXml_MatchesPrefix(text, (base + XML_OFFSET(XmlParserStringPool, closingTagOpen)), 0))
                return text;
            ParseNode* node = TiXmlNode_Identify(self, text);
            if (node) {
                text = node->parse(text);
                TiXmlNode_LinkEndChild(self, node);
            } else
                return 0;
        }
        text = skipSpace(text);
    }
    if (!text && state) {
        state->error = 1;
        state->code = kXmlErrorElementValue;
        TiXmlString_AssignCStr(state->message, errorString(kXmlErrorElementValue));
    }
    return text;
}
#pragma opt_propagation off
// Parse an attribute name, equals sign, and quoted or unquoted value.
// Layout-derived byte access below preserves aliasing and instruction scheduling.
// Grammar: space* name space* '=' space* (single-quoted | double-quoted | unquoted).
// Quoted values decode entities and return AFTER the quote; unquoted values copy bytes literally
// and return AT whitespace, '/', '>' or NUL. This legacy parser accepts unquoted attributes.
// On name/equals/missing-value failure return null and report kXmlErrorAttributes if a document exists.
char* TiXmlAttribute_Parse(void* selfIn, const char* inputText) {
    char* self = (char*)selfIn;
    const char* text = inputText;
    if (!text || !*text)
        text = 0;
    else {
        while (text && *text) {
            int ch = (unsigned char)*text;
            int space = spaceFlag(ch);
            if (!space && (signed char)ch != 10 && (signed char)ch != 13)
                break;
            ++text;
        }
    }
    if (text == 0 || !*text)
        return 0;

    // Phase 1: read the attribute name after the leading whitespace already skipped above.
    text = TiXmlBase_ReadName(text, &AsTiXmlAttribute(self)->name);
    if (!text || !*text) {
        char* state = *(char**)(self + XML_OFFSET(AttributeOwnerView, document));
        if (state) {
            state[XML_OFFSET(ErrorState, error)] = 1;
            *(int*)(state + XML_OFFSET(ErrorState, code)) = kXmlErrorAttributes;
            TiXmlString_AssignCStr((state + XML_OFFSET(ErrorState, message)),
                                   tinyXmlErrorMessages.entries[kXmlErrorAttributes]);
        }
        return 0;
    }

    // Phase 2: skip inter-token space and require exactly the equals delimiter.
    if (!text || !*text)
        text = 0;
    else
        while (text && *text) {
            int ch = (unsigned char)*text;
            int invalid = 0;
            if (ch < 0 || ch >= kCharacterCount)
                invalid = 1;
            int space = invalid ? 0 : s_cLocaleData.table->flags[ch] & kLocaleSpaceFlag;
            if (!space && (signed char)ch != 10 && (signed char)ch != 13)
                break;
            ++text;
        }
    if (text == 0 || !*text || *text != '=') {
        char* state = *(char**)(self + XML_OFFSET(AttributeOwnerView, document));
        if (state) {
            state[XML_OFFSET(ErrorState, error)] = 1;
            *(int*)(state + XML_OFFSET(ErrorState, code)) = kXmlErrorAttributes;
            TiXmlString_AssignCStr((state + XML_OFFSET(ErrorState, message)),
                                   tinyXmlErrorMessages.entries[kXmlErrorAttributes]);
        }
        return 0;
    }

    // Phase 3: find the value; each failure writes the same three document-error fields.
    char* value = (char*)(text + 1);
    if (value == 0 || !text[1])
        value = 0;
    else {
        while (value && *value) {
            int ch = (unsigned char)*value;
            int space = spaceFlag(ch);
            if (!space && (signed char)ch != 10 && (signed char)ch != 13)
                break;
            ++value;
        }
    }
    if (value == 0 || !*value) {
        char* state = *(char**)(self + XML_OFFSET(AttributeOwnerView, document));
        if (state) {
            state[XML_OFFSET(ErrorState, error)] = 1;
            *(int*)(state + XML_OFFSET(ErrorState, code)) = kXmlErrorAttributes;
            TiXmlString_AssignCStr((state + XML_OFFSET(ErrorState, message)),
                                   tinyXmlErrorMessages.entries[kXmlErrorAttributes]);
        }
        return 0;
    }

    // Phase 4: parse the chosen value form and leave the caller at the next grammar token.
    int ch = (signed char)*value;
    if (ch == '\'') {
        value = (char*)TiXmlBase_ReadText(value + 1, (self + XML_OFFSET(TiXmlAttributeLayout, value)), 0,
                                          tinyXmlParserStrings.singleQuote, 0);
    } else if (ch == '"') {
        value = (char*)TiXmlBase_ReadText(value + 1, (self + XML_OFFSET(TiXmlAttributeLayout, value)), 0,
                                          tinyXmlParserStrings.doubleQuote, 0);
    } else {
        TiXmlString_AssignCStr((self + XML_OFFSET(TiXmlAttributeLayout, value)), tinyXmlParserStrings.empty);
        LocaleGlobal* globals = &s_cLocaleData;
        int zero = 0;
        int byte;
        while (value && (byte = (unsigned char)*value, (signed char)byte != 0) &&
               !(byte < 0 || byte >= kCharacterCount ? 0 : globals->table->flags[byte] & kLocaleSpaceFlag) &&
               (signed char)byte != 10 && (signed char)byte != 13 && (signed char)byte != '/' &&
               (signed char)byte != '>') {
            char buf[2];
            buf[0] = byte;
            buf[1] = (char)zero;
            TiXmlString_Append((self + XML_OFFSET(TiXmlAttributeLayout, value)), buf);
            ++value;
        }
    }
    return (char*)value;
}
#pragma opt_propagation on

extern char __vt__14TiXmlAttribute[];
struct LocalPair : PairObject {
    // Initialize the legacy temporary attribute wrapper in its existing layout.
    LocalPair() {
        first.data = __vt__14TiXmlAttribute;
        first.count = 0;
        second.data = 0;
        second.count = 0;
    }
    // Destroy the temporary's two owned strings without deleting its stack storage.
    ~LocalPair() {
        PairObject_dtor(this, 0);
    }
};

struct AttrLocal2 : PairObject {
    int extra0;
    int extra1;
};

extern char s_tinyXmlEmptyString;

struct DeclLocal {
    void* vt;
    ErrorState* doc;
    Container first;
    Container second;
    void* prev;
    void* next;
    // Initialize a temporary declaration-attribute owner and its empty list links.
    DeclLocal() {
        vt = __vt__14TiXmlAttribute;
        first.count = 0;
        first.data = 0;
        second.count = 0;
        second.data = 0;
        prev = 0;
        next = 0;
    }
    // Release value/name string arrays after a declaration field has copied the desired bytes.
    ~DeclLocal() {
        if (second.data)
            __dla__FPv(second.data);
        second.data = 0;
        second.count = 0;
        if (first.data)
            __dla__FPv(first.data);
        first.data = 0;
        first.count = 0;
    }
};

struct DeclString {
    void* data;
    unsigned int count;
    // Replace the owned declaration string with a copy of borrowed NUL-terminated bytes.
    DeclString& operator=(const char* s) {
        TiXmlString_AssignCStr(this, s);
        return *this;
    }
};

struct DeclNode {
    char pad[0x28];
    DeclString version;
    DeclString encoding;
    DeclString standalone;
};

// Read version, encoding, and standalone attributes from an XML declaration.
// Known values are copied out of temporary attributes; unknown tokens are skipped.
// Returns after '>' on success, null on invalid opener/exhaustion.
char* TiXmlDeclaration_Parse(void* selfIn, const char* inputText) {
    const char* cur;
    DeclNode* self = (DeclNode*)selfIn;
    const char* text;
    if (!inputText || !*inputText)
        text = 0;
    else {
        const char* p = inputText;
        while (p && *p) {
            int ch = (unsigned char)*p;
            if (!spaceFlag(ch) && (signed char)ch != 10 && (signed char)ch != 13)
                break;
            ++p;
        }
        text = p;
    }

    ErrorState* state = TiXmlNode_GetDocumentAncestor(self);
    if (!text || !*text || !TiXml_MatchesPrefix(text, tinyXmlParserStrings.declarationOpen, 1)) {
        if (state) {
            state->error = 1;
            state->code = kXmlErrorDeclaration;
            TiXmlString_AssignCStr(state->message, tinyXmlErrorMessages.entries[kXmlErrorDeclaration]);
        }
        return 0;
    }

    // Phase 1: opener accepted; clear all three declaration outputs.
    cur = text + 5;
    self->version = tinyXmlParserStrings.empty;
    self->encoding = tinyXmlParserStrings.empty;
    self->standalone = tinyXmlParserStrings.empty;

    // Phase 2: consume recognized attributes using temporary owners and copy their values.
    while (cur && *cur) {
        if ((signed char)*cur == '>')
            return (char*)cur + 1;

        cur = skipSpace(cur);
        if (TiXml_MatchesPrefix(cur, tinyXmlParserStrings.version, 1)) {
            DeclLocal local;
            cur = TiXmlAttribute_Parse(&local, cur);
            self->version = local.second.count ? (const char*)local.second.data : (const char*)&s_tinyXmlEmptyString;
        } else if (TiXml_MatchesPrefix(cur, tinyXmlParserStrings.encoding, 1)) {
            DeclLocal local;
            cur = TiXmlAttribute_Parse(&local, cur);
            self->encoding = local.second.count ? (const char*)local.second.data : (const char*)&s_tinyXmlEmptyString;
        } else if (TiXml_MatchesPrefix(cur, tinyXmlParserStrings.standalone, 1)) {
            DeclLocal local;
            cur = TiXmlAttribute_Parse(&local, cur);
            self->standalone = local.second.count ? (const char*)local.second.data : (const char*)&s_tinyXmlEmptyString;
        } else {
            while (cur && *cur && (signed char)*cur != '>' && !spaceFlag((unsigned char)*cur))
                ++cur;
        }
    }
    return 0;
}

extern char s_tinyXmlEmptyString;
void __ct__11TiXmlStringFPCc(void*, const char*);
void TiXmlElement_SetAttribute(void*, const char*, const char*);
const char* TiXmlElement_ReadValue(void*, const char*);
struct TagData {
    char* data;
    unsigned int count;
};
struct TagString : TagData {
    // Free the generated closing-tag buffer on every exit from element parsing.
    ~TagString() {
        if (data)
            __dla__FPv(data);
        data = 0;
        count = 0;
    }
};
struct AttrLocal {
    char* vt;
    ErrorState* state;
    TagData first;
    TagData second;
    int extra0;
    int extra1;
    // Initialize temporary name/value owners and borrow the destination document's error state.
    AttrLocal(ErrorState* s) : vt(__vt__14TiXmlAttribute) {
        first.count = 0;
        first.data = 0;
        second.count = 0;
        second.data = 0;
        extra0 = 0;
        extra1 = 0;
        state = s;
    }
    // Free temporary attribute value/name arrays after SetAttribute has copied their contents.
    ~AttrLocal() {
        if (second.data)
            __dla__FPv(second.data);
        second.data = 0;
        second.count = 0;
        if (first.data)
            __dla__FPv(first.data);
        first.data = 0;
        first.count = 0;
    }
};

// Read an element name and attributes, then its body and matching closing tag.
// Returns after '/>' for an empty element, or after the complete matching '</name>' otherwise.
// Body parsing returns AT its closing opener; this function verifies/consumes the generated closer.
// closer and attr own temporary strings. SetAttribute copies their bytes; linked child nodes
// belong to self. A null return can leave a partially filled element; there is no rollback.
char* TiXmlElement_Parse(void* selfIn, const char* inputText) {
    char* cur;
    char* self = (char*)selfIn;
    const char* text;
    if (!inputText || !*inputText)
        text = 0;
    else {
        const char* p = inputText;
        while (p && *p) {
            int ch = (unsigned char)*p;
            if (!spaceFlag(ch) && (signed char)ch != 10 && (signed char)ch != 13)
                break;
            ++p;
        }
        text = p;
    }
    ErrorState* state = TiXmlNode_GetDocumentAncestor(self);
    if (!text || !*text || *text != '<') {
        if (state) {
            state->error = 1;
            state->code = kXmlErrorElement;
            TiXmlString_AssignCStr(state->message, errorString(kXmlErrorElement));
        }
        return 0;
    }
    // Phase 1: require '<', read the element name, then construct the expected closing tag.
    char* name = (char*)skipSpace(text + 1);
    cur = TiXmlBase_ReadName(name, &AsTiXmlNode(self)->value);
    if (!cur || !*cur) {
        if (state) {
            state->error = 1;
            state->code = kXmlErrorElementName;
            TiXmlString_AssignCStr(state->message, errorString(kXmlErrorElementName));
        }
        return 0;
    }
    TagString closer;
    __ct__11TiXmlStringFPCc(&closer, tinyXmlParserStrings.closingTagOpen);
    TiXmlString_Append(&closer, AsTiXmlNode(self)->valueCapacity ? AsTiXmlNode(self)->value : &s_tinyXmlEmptyString);
    TiXmlString_Append(&closer, tinyXmlParserStrings.tagClose);
    // Phase 2: scan opening-tag attributes until '/>' or '>'; cur always names the next token.
    int lead;
    while (cur && (lead = (unsigned char)*cur, (signed char)lead != 0)) {
        if (!cur || !(signed char)lead)
            cur = 0;
        else {
            while (cur && *cur) {
                int ch = (unsigned char)*cur;
                if (!spaceFlag(ch) && (signed char)ch != 10 && (signed char)ch != 13)
                    break;
                ++cur;
            }
        }
        if (!cur || !*cur) {
            if (state) {
                state->error = 1;
                state->code = kXmlErrorAttributes;
                TiXmlString_AssignCStr(state->message, errorString(kXmlErrorAttributes));
            }
            return 0;
        }
        signed char c = *cur;
        if (c == '/') {
            if (cur[1] != '>') {
                if (state) {
                    state->error = 1;
                    state->code = kXmlErrorEmptyTag;
                    TiXmlString_AssignCStr(state->message, errorString(kXmlErrorEmptyTag));
                }
                return 0;
            }
            return cur + 2;
        }
        // Phase 3: parse nested content, then match and consume this element's exact closer.
        if (c == '>') {
            char* body = (char*)TiXmlElement_ReadValue(self, cur + 1);
            if (!body || !*body)
                return 0;
            if (TiXml_MatchesPrefix(body, closer.count ? closer.data : &s_tinyXmlEmptyString, 0))
                return body + TiXmlString_Length(&closer);
            if (state) {
                state->error = 1;
                state->code = kXmlErrorEndTag;
                TiXmlString_AssignCStr(state->message, errorString(kXmlErrorEndTag));
            }
            return 0;
        }
        // Attribute branch: copy from a temporary owner before its destructor frees the strings.
        AttrLocal attr(state);
        cur = TiXmlAttribute_Parse(&attr, cur);
        if (!cur || !*cur) {
            if (state) {
                state->error = 1;
                state->code = kXmlErrorElement;
                TiXmlString_AssignCStr(state->message, errorString(kXmlErrorElement));
            }
            return 0;
        }
        const char* firstStr = attr.first.count ? attr.first.data : &s_tinyXmlEmptyString;
        const char* secondStr = attr.second.count ? attr.second.data : &s_tinyXmlEmptyString;
        TiXmlElement_SetAttribute(self, firstStr, secondStr);
    }
    return cur;
}

} // extern "C"
