// Numeric HUD/menu text (CFontNumDraw): format values and draw pane-owned text.
// Mod lookup:
// - Precision, zero padding, positive overflow trimming: SetInteger/SetFloat,
//   TextDescriptorFormat below and descriptor +8 (loader in font_num_draw_entries.cpp).
// - Alignment/font/color: Draw, descriptor bits 8/0x20, variant bit 10,
//   pane Q7 opacity and packed tint. Position/offsets are layout pixels.
// - Scheduling: DrawList (font_registry_and_round_setup_mixed.cpp) clears drawProcessed AFTER each table;
//   the frame UI phase in game_main_loop_and_startup.cpp calls that registry operation.
// - Lifetime: ClearEntries releases entries/keys; Draw removes null tree nodes.
// Shared layout: UITextTable.h; constructors/registry: font_registry_and_round_setup_mixed.cpp. No gameplay
// tuning or presentation-rate constant. Receivers require live pane/font data
// and serialized UI access. Lock 3 covers mutation/draw; stream/locale/string
// guards unwind on exceptions. Setters require an existing key, not a new ID.
// Preserve stream construction, float/truncation order and cleanup ABI.

#include "sdk_protos.h"
#include "classes/UITextTable.h"

// Text descriptor word (+8): low bits enable formatting/alignment; variant uses bit 10.
// Width, precision and digit-limit fields each occupy five bits.
enum TextDescriptorFormat {
    kTrimPositiveDecimalOverflow = 1,
    kSetDecimalPrecision = 2,
    kZeroPadToWidth = 4,
    kAlignTextRight = 8,
    kAlignTextCenter = 0x20,
    kFontVariantShift = 10,
    kWidthShift = 16,
    kPrecisionShift = 21,
    kDigitLimitShift = 26,
    kFormatFieldMask = 31
};
// MSL stream constants below are representation values, not descriptor bits.
// openmode 0x10 enables the put area; 0x08 enables the get area.
// Format masks are applied to the high halfword of the 32-bit big-endian flags.
// Thus halfword masks 4/0x800 OR full-word bits 0x00040000/0x08000000.
// The high halfword is the actual MSL flag set: 4=fixed decimal notation,
// 0x800=show positive sign. Evidence: mw_string_stream_and_text_tags.cpp StreamFormatFlags and
// MWStream_FormatDouble select fixed formatting and emit '+' from these bits;
// MWNumPut signed formatting uses the same sign policy. These are not alignment.
// Integer's fixed bit has no integer effect; with zero padding it leaves '+'
// disabled, otherwise enables it. Float always enables '+', even with padding.
// Both OR existing flags. Precision is decimal places in fixed mode; width is
// minimum field width, not truncation. Pane alignment is independent of padding.
// Preserve this MSL halfword representation on big-endian Wii.
enum TextStreamSettings {
    kOpenForOutput = 0x10,
    kOpenForInput = 8,
    kIosFixedDecimal = 4,
    kIosShowPositiveSign = 0x800,
    kSpaceCharacter = 0x20,
    kZeroCharacter = 0x30,
    kDecimalRadix = 10,
    kTextAlignLeft = 0,
    kTextAlignCenter = 1,
    kTextAlignRight = 2
};
// Offsets relative to basic_ios, reached through a virtual-base pointer (not stream.bytes).
enum TextIosOffset {
    kIosPrecision = 0x28,
    kIosWidth = 0x2C,
    kIosFormatFlags = 0x30,
    kIosState = 0x34,
    kIosFillCharacter = 0x38,
    kIosVirtualBaseAdjustment = 0x3C
};
// Offsets relative to Stream5262C::bytes. Buffer get/put triples borrow its string data.
enum TextStreamOffset {
    kStreamStringBuffer = 0x14,
    kStreamGetBegin = 0x18,
    kStreamGetCurrent = 0x1C,
    kStreamGetEnd = 0x20,
    kStreamPutBegin = 0x24,
    kStreamPutCurrent = 0x28,
    kStreamPutEnd = 0x2C,
    kStreamOpenMode = 0x38,
    kStreamData = 0x3C,
    kStreamStringTag = 0x40,
    kStreamStringLength = 0x44,
    kStreamHeapString = 0x48,
    kStreamVirtualIos = 0x4C
};

extern "C" {
void __unexpected(...) __attribute__((noreturn));

void MWStringBuf_GetString(void*, void*);
void MWStringStream_CopyBufferString(void*, void*);
void* appMallocDummy__FUl(unsigned int);
void appFreeUni__FPv(void*);
void MWSharedOwner_ReleaseReference(void*);
void std_ios_base_dtor(void*, short);
void __dl__FPv(void*);
void* memcpy(void*, const void*, unsigned int);
void* memset(void*, int, unsigned int);
void TableLock_ctor(void*, int, int);
void MWStream_CopyLocale(void*, void*);
void MWIOS_InitializeStreamBuffer(void*, void*);
void* MWLocale_GetDefault(void*);
void MWCtypeFacet_Construct(void*, int, int, int);
void MWLocale_InstallFacet(void*, void*, int);
void UIGroup_SetEnabled(void*, int);
void MWOStream_InsertSigned32(void*, int);
void String_ReplaceRange(void*, int, int, int, int, void*);
void UITextTable_ApplyNumericTextFormatFlags(void*, void*, int);
void RBTree_FreeTrivialSubtree(void*, void*);
void __dt__9TableLockFv(void*, int);
extern int s_localeFacetIdCounter;
extern int s_ctypeFacetId;
extern float kFloat_0_FontRenderer;

struct TableLockGuard {
    char bytes[8];
    // Release the table lock on normal return and exception unwinding.
    ~TableLockGuard() {
        __dt__9TableLockFv(this, -1);
    }
};
struct TextTreeNodeView {
    char* left;
    char* right;
    unsigned int parentColor;
    int key;
    char* entry;
};
struct Obj80251FDC {
    unsigned char pad[0x10];
    int a;
    int b;
    int c;
};
struct Meta80251FDC {
    int x;
    int y;
    unsigned int flags;
};
// Numeric setters borrow this entry; only textTag/textLength/textData are replaced.
// With bit 31 clear the 12 bytes at textTag hold an inline MSL string (length in
// the first byte, bytes at +1); with bit 31 set textData owns heap storage.
struct NumericTextEntryView {
    Meta80251FDC* descriptor;
    void* group;
    void* vtable;
    int hasLayout;
    unsigned int textTag;
    int textLength;
    char* textData;
};
typedef char NumericTextEntryView_textOffset[(int)&((NumericTextEntryView*)0)->textTag == 0x10 ? 1 : -1];
// Preserve byte-address aliasing in the setters; these offsets come from the
// typed schema above, rather than duplicating numeric entry-layout assumptions.
enum NumericTextEntryOffsets {
    kNumericDescriptorOffset = (int)&((NumericTextEntryView*)0)->descriptor,
    kNumericGroupOffset = (int)&((NumericTextEntryView*)0)->group,
    kNumericDescriptorFlagsOffset = (int)&((Meta80251FDC*)0)->flags,
    kNumericTextOffset = (int)&((NumericTextEntryView*)0)->textTag,
    kNumericTextLengthOffset = (int)&((NumericTextEntryView*)0)->textLength,
    kNumericTextDataOffset = (int)&((NumericTextEntryView*)0)->textData
};
void OutputStringStream_CopyBufferString(void*, char*);
void MWLocale_InstallFacet(void*, void*, int);
void* __nw__FUl(unsigned int);
void OStream_InsertFloat(void*, float);

extern char iosBaseVtable[], s_basicOstreamVtable[], s_integerTextOutputStringStreamVtables[];
extern char stringStreamBufferVtable[], s_appStringbufVTable[];

struct Facet51FDC {
    char bytes[0x18];
    // Construct a locale ctype facet using the runtime's existing three-argument ABI.
    Facet51FDC(int a, int b, int c) {
        MWCtypeFacet_Construct(this, a, b, c);
    }
};
struct CType51FDC {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual signed char widen(signed char);
};
struct Locale51FDC {
    char* table;
    void* ref;
    // Release the retained locale implementation, if present.
    ~Locale51FDC() throw() {
        if (ref)
            MWSharedOwner_ReleaseReference(ref);
    }
};
struct String51FDC {
    unsigned int tag;
    int length;
    char* data;
    // Release heap storage only; strings with bit 31 clear store their bytes inline.
    ~String51FDC() {
        if (tag >> 31)
            appFreeUni__FPv(data);
    }
};
struct Lock51FDC {
    char bytes[8];
    // Unlock using the runtime's short deleting-destructor argument.
    ~Lock51FDC() {
        ((void (*)(void*, short))__dt__9TableLockFv)(this, (short)-1);
    }
    // Encode the entry-format sign selector: positive=0, negative=1, zero=2.
    static int sign(int count) {
        if (count == 0)
            return 2;
        int result = 1;
        if (count > 0)
            result = 0;
        return result;
    }
};
struct S51Byte4 {
    unsigned char value;
    char pad[3];
};
struct S51Stream {
    char* volatile base; // 0x00
    void* vtable;        // 0x04
    void* bufVtable;     // 0x08
    void* getBegin;      // 0x0c: borrowed readable range
    void* getCurrent;    // 0x10: read cursor
    void* getEnd;        // 0x14: one past readable range
    void* putBegin;      // 0x18: borrowed writable range
    void* putCurrent;    // 0x1c: write cursor
    void* putEnd;        // 0x20: one past writable range
    void* locale;        // 0x24
    void* ref;           // 0x28
    unsigned char mode;  // 0x2c
    char pad2d[3];
    char* data;          // 0x30
    unsigned int flags;  // 0x34
    unsigned int length; // 0x38
    char* string;        // 0x3c
    char ios[0x40];      // 0x40
};
// Set the virtual-base displacement used by this stage of stream construction/destruction.
inline void S51Relocate(char* base, char* target) {
    *(int*)(base + 0x3c) = target - base;
}
// Release a locale handle's retained reference, if present.
inline void S51Release(void* ptr) throw() {
    if (ptr)
        MWSharedOwner_ReleaseReference(ptr);
}
// Copy the default locale handle and retain its reference-counted implementation.
inline void S51CopyLocale(void** dst) throw() {
    void** src = (void**)MWLocale_GetDefault(stringStreamBufferVtable);
    dst[0] = src[0];
    void* r = src[1];
    dst[1] = r;
    if (r)
        ++((int*)r)[1];
}

// Resolve or install the integer stream's ctype facet without changing locale ownership.
static inline void EnsureIntegerCtypeFacet(Locale51FDC& handle, int index, void*& obj) {
    if ((unsigned)index >= *(unsigned int*)(handle.table + 4) || !(obj = (*(void***)handle.table)[index])) {
        Facet51FDC* created = new Facet51FDC(0, 0, 0);
        char* table = handle.table;
        if (!s_ctypeFacetId)
            s_ctypeFacetId = ++s_localeFacetIdCounter;
        MWLocale_InstallFacet(table, created, s_ctypeFacetId);
        obj = (*(void***)handle.table)[index];
    }
}

// Establish integer ostream/virtual-ios bases in caller-owned storage.
static inline void InitializeIntegerStreamBase(S51Stream* sp, char*& ios) {
    sp->base = ios;
    *(void* volatile*)ios = iosBaseVtable;
    sp->vtable = s_basicOstreamVtable;
    *(void* volatile*)ios = s_basicOstreamVtable + 0xc;
    S51Relocate(sp->base, (char*)&sp->bufVtable);
    ios = sp->base;
    MWIOS_InitializeStreamBuffer(ios, &sp->bufVtable);
    *(int*)(ios + kIosState) = 0;
}

// Establish the integer stringbuf vtable and empty get/put ranges.
static inline void InitializeIntegerStringBuffer(S51Stream* sp) {
    sp->vtable = s_integerTextOutputStringStreamVtables;
    *(void**)sp->base = s_integerTextOutputStringStreamVtables + 0xc;
    S51Relocate(sp->base, sp->ios);
    sp->bufVtable = stringStreamBufferVtable;
    sp->getBegin = 0;
    sp->getCurrent = 0;
    sp->getEnd = 0;
    sp->putBegin = 0;
    sp->putCurrent = 0;
    sp->putEnd = 0;
}

// Attach the integer stringbuf to its empty inline string and default format.
static inline void AttachIntegerInlineString(S51Stream* sp) {
    sp->flags = 0;
    sp->bufVtable = s_appStringbufVTable;
    sp->mode = kOpenForOutput;
    sp->length = 0;
    sp->string = 0;
    char* data = sp->string;
    if (!(sp->flags >> 31))
        data = (char*)&sp->flags + 1;
    sp->data = data;
    if (sp->mode & kOpenForOutput) {
        sp->putCurrent = data;
        sp->putBegin = data;
        sp->putEnd = data;
    }
    if (sp->mode & kOpenForInput) {
        sp->getBegin = data;
        sp->getCurrent = data;
        sp->getEnd = data;
    }
    *(unsigned short*)(sp->base + kIosFormatFlags) |= kIosFixedDecimal;
    *(int*)(sp->base + kIosPrecision) = 0;
}

#pragma use_lmw_stmw on
// Find the keyed text entry, apply descriptor formatting to the integer, and store the resulting string.
// Bit 0 trims only when count / 10^digits > 0: negatives remain unchanged, even
// beyond the requested digit count; digits==0 turns positive values into zero.
// Width and precision are five-bit descriptor values; decimal scale must fit int.
// Requires an existing key with a valid descriptor/group; this lower-bound search does not insert.
// Formatting replaces the entry's owned text; the stream and temporary string remain local owners.
int UITextTable_SetInteger(char* self, int key, int count) {
    Lock51FDC lock;
    TableLock_ctor(&lock, 3, 1);
    // Phase 1: lock and find the existing descriptor by key.
    char* node = *(char**)(self + 0x1c);
    char* found = self + 0x1c;
    while (node) {
        int value = ((TextTreeNodeView*)node)->key;
        if (value >= key) {
            found = node;
            node = ((TextTreeNodeView*)node)->left;
        } else
            node = ((TextTreeNodeView*)node)->right;
    }
    if (found == self + 0x1c || key < ((TextTreeNodeView*)found)->key)
        found = self + 0x1c;
    char* entry = ((TextTreeNodeView*)found)->entry;
    char* descriptor = *(char**)(entry + kNumericDescriptorOffset);
    int formattedInteger = count;
    // Phase 2: construct stream/locale/fill state, then attach the small-string buffer.
    S51Stream stream;
    S51Stream* outputStream = &stream;
    char* ios = outputStream->ios;
    InitializeIntegerStreamBase(outputStream, ios);
    signed char result;
    {
        Locale51FDC handle;
        MWStream_CopyLocale(&handle, ios);
        volatile S51Byte4 unused;
        unused.value = 0;
        if (!s_ctypeFacetId)
            s_ctypeFacetId = ++s_localeFacetIdCounter;
        int index = s_ctypeFacetId;
        void* obj;
        EnsureIntegerCtypeFacet(handle, index, obj);
        signed char arg = kSpaceCharacter;
        result = ((CType51FDC*)obj)->widen(arg);
    }
    ios[kIosFillCharacter] = result;
    InitializeIntegerStringBuffer(outputStream);
    S51CopyLocale((void**)&outputStream->locale);
    AttachIntegerInlineString(outputStream);
    UIGroup_SetEnabled(*(void**)(entry + kNumericGroupOffset), 1);
    // Preserve the positive-quotient gate; this is not an absolute-value digit limit.
    if (*(unsigned*)(descriptor + kNumericDescriptorFlagsOffset) & kTrimPositiveDecimalOverflow) {
        int digits = (*(unsigned*)(descriptor + kNumericDescriptorFlagsOffset) >> kDigitLimitShift) & kFormatFieldMask;
        int scale = 1;
        for (int i = 0; i < digits; ++i)
            scale *= kDecimalRadix;
        int quotient = count / scale;
        if (quotient > 0)
            formattedInteger = count - quotient * scale;
    }
    if (*(unsigned*)(descriptor + kNumericDescriptorFlagsOffset) & kZeroPadToWidth) {
        *(int*)(outputStream->base + kIosWidth) =
            (*(unsigned*)(descriptor + kNumericDescriptorFlagsOffset) >> kWidthShift) & kFormatFieldMask;
        outputStream->base[kIosFillCharacter] = kZeroCharacter;
    }
    if (*(unsigned*)(descriptor + kNumericDescriptorFlagsOffset) & kSetDecimalPrecision)
        *(int*)(outputStream->base + kIosPrecision) =
            (*(unsigned*)(descriptor + kNumericDescriptorFlagsOffset) >> kPrecisionShift) & kFormatFieldMask;
    if (!(*(unsigned*)(descriptor + kNumericDescriptorFlagsOffset) & kZeroPadToWidth))
        *(unsigned short*)(outputStream->base + kIosFormatFlags) |= kIosShowPositiveSign;
    // Phase 3: format and copy the result into the entry (inline or owned-heap string).
    MWOStream_InsertSigned32(&stream, formattedInteger);
    {
        String51FDC formattedText;
        OutputStringStream_CopyBufferString(&formattedText, (char*)&stream);
        if (!(*(unsigned*)(entry + kNumericTextOffset) >> 31) && !(formattedText.tag >> 31)) {
            *(unsigned*)(entry + kNumericTextOffset) = formattedText.tag;
            *(int*)(entry + kNumericTextLengthOffset) = formattedText.length;
            *(char**)(entry + kNumericTextDataOffset) = formattedText.data;
        } else {
            int oldTextLength;
            if (*(unsigned*)(entry + kNumericTextOffset) >> 31)
                oldTextLength = *(int*)(entry + kNumericTextLengthOffset);
            else
                oldTextLength = *(unsigned char*)(entry + kNumericTextOffset) & 0x7f;
            int newTextLength;
            char* textBegin;
            if (!(formattedText.tag >> 31)) {
                newTextLength = *(unsigned char*)&formattedText & 0x7f;
                textBegin = (char*)&formattedText + 1;
            } else {
                textBegin = formattedText.data;
                newTextLength = formattedText.length;
            }
            volatile S51Byte4 allocator;
            S51Byte4 copy;
            copy.value = allocator.value;
            String_ReplaceRange(entry + kNumericTextOffset, 0, oldTextLength, (int)textBegin,
                                (int)(textBegin + newTextLength), &copy);
        }
    }
    // Phase 4: apply sign-dependent entry formatting, then release stream/string/locale owners.
    int status = Lock51FDC::sign(count);
    UITextTable_ApplyNumericTextFormatFlags(self, entry, status);
    S51Relocate(outputStream->base, outputStream->ios);
    if (outputStream->flags >> 31)
        appFreeUni__FPv(outputStream->string);
    S51Release(outputStream->ref);
    S51Relocate(outputStream->base, (char*)&outputStream->bufVtable);
    std_ios_base_dtor(outputStream->ios, (short)0);
    return 1;
}
#pragma use_lmw_stmw reset

extern char iosBaseVtable[], iStreamVtableGroup[], s_basicOstreamVtable[];
extern char outputStringStreamVtables[], s_appStringstreamVTables[], stringStreamBufferVtable[], s_appStringbufVTable[];

struct Facet5262C {
    char bytes[0x18];
    // Construct the ctype facet through the float-stream specialization's runtime prototype.
    Facet5262C(const unsigned short* a, bool b, unsigned long c) {
        ((void (*)(void*, const unsigned short*, bool, unsigned long))MWCtypeFacet_Construct)(this, a, b, c);
    }
};

struct CType5262C {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual signed char widen(signed char);
};

struct Lock5262C {
    char bytes[8];
    // Unlock on scope exit using the existing short destructor flag.
    ~Lock5262C() {
        ((void (*)(void*, short))__dt__9TableLockFv)(this, (short)-1);
    }
    // Encode float sign for entry styling: positive=0, negative=1, zero=2.
    static int sign(float value) {
        if (kFloat_0_FontRenderer == value)
            return 2;
        int result = 1;
        if (value > kFloat_0_FontRenderer)
            result = 0;
        return result;
    }
};

struct Locale5262C {
    char* table;
    void* ref;
    // Release the retained locale implementation, if present.
    ~Locale5262C() throw() {
        if (ref)
            MWSharedOwner_ReleaseReference(ref);
    }
};

struct String5262C {
    unsigned int tag;
    int length;
    char* data;
    // Free heap text only when the ownership tag's high bit is set.
    ~String5262C() {
        if (tag >> 31)
            appFreeUni__FPv(data);
    }
};

struct Byte5262C {
    unsigned char value;
};

struct Stream5262C {
    // Iostream prefix, embedded stringbuf at 0x14, and virtual basic_ios at 0x4c.
    char bytes[0x8c];
    Stream5262C();
    // Construct the input/virtual-ios base and attach the embedded buffer; return basic_ios.
    char* init(char* vb) {
        char* small = bytes;
        *(void* volatile*)vb = iosBaseVtable;
        *(char* volatile*)small = vb;
        *(void* volatile*)vb = iStreamVtableGroup + 0xc;
        *(char**)(small + 0xc) = vb;
        *(void**)(small + 8) = iStreamVtableGroup;
        char* first = *(char* volatile*)small;
        *(int*)(first + 0x3c) = (small + 0xc) - first;
        char* base = *(char* volatile*)small;
        *(int*)(small + 4) = 0;
        MWIOS_InitializeStreamBuffer(base, small + kStreamStringBuffer);
        *(int*)(base + 0x34) = 0;
        return base;
    }
    // Release the stringbuf locale implementation's reference.
    void release() throw() {
        if (*(void**)(bytes + 0x34))
            MWSharedOwner_ReleaseReference(*(void**)(bytes + 0x34));
    }
    // Copy/retain the default locale into the stringbuf locale subobject.
    void copyLocale() throw() {
        char* loc = (char*)MWLocale_GetDefault(stringStreamBufferVtable);
        char* dest = bytes + 0x30;
        *(int*)dest = *(int*)loc;
        char* ref = *(char**)(loc + 4);
        *(char**)(dest + 4) = ref;
        if (ref)
            ++*(int*)(ref + 4);
    }
    // Free owned string/locale state, restore base adjustments and destroy the virtual ios base.
    ~Stream5262C() {
        char* p = bytes;
        char* base = *(char**)p;
        *(int*)(base + 0x3c) = (p + 0x4c) - base;
        if (*(unsigned int*)(p + 0x40) >> 31)
            appFreeUni__FPv(*(void**)(p + 0x48));
        release();
        base = *(char**)p;
        *(int*)(base + 0x3c) = (p + 0x14) - base;
        base = *(char**)(p + 0xc);
        *(int*)(base + 0x3c) = (p + 0x14) - base;
        base = *(char**)p;
        *(int*)(base + 0x3c) = (p + 0xc) - base;
        std_ios_base_dtor(p + 0x4c, (short)0);
    }
};

// Initialize the virtual ios base and widen the default space fill character through its locale.
inline Stream5262C::Stream5262C() {
    Facet5262C* created;
    int index;
    signed char ret;
    char* base;
    base = init(bytes + 0x4c);
    {
        Locale5262C handle;
        MWStream_CopyLocale(&handle, base);
        volatile Byte5262C unused;
        unused.value = 0;
        if (!s_ctypeFacetId)
            s_ctypeFacetId = ++s_localeFacetIdCounter;
        index = s_ctypeFacetId;
        void* obj;
        if ((unsigned)index >= *(unsigned int*)(handle.table + 4) || !(obj = (*(void***)handle.table)[index])) {
            created = new Facet5262C(0, 0, 0);
            char* table = handle.table;
            if (!s_ctypeFacetId)
                s_ctypeFacetId = ++s_localeFacetIdCounter;
            MWLocale_InstallFacet(table, created, s_ctypeFacetId);
            obj = (*(void***)handle.table)[index];
        }
        signed char arg = 0x20;
        ret = ((CType5262C*)obj)->widen(arg);
    }
    base[0x38] = (char)ret;
}

// Finish the float ostream/iostream bases without changing buffer ownership.
static inline void InitializeFloatStreamBases(char* small) {
    // Prefix: input ios/state/vtable at +00/+04/+08; output ios/vtable at +0C/+10.
    // Each virtual-ios pointer is reloaded after writes: the bases may alias.
    // Phase 1: standalone output base and its buffer displacement.
    *(void**)(small + 0x10) = s_basicOstreamVtable;
    **(void***)(small + 0xc) = s_basicOstreamVtable + 0xc;
    *(int*)(*(char**)(small + 0xc) + 0x3c) = (small + kStreamStringBuffer) - *(char**)(small + 0xc);
    // Phase 2: join input/output bases under the standard stringstream tables.
    *(void**)(small + 8) = outputStringStreamVtables;
    **(void***)small = outputStringStreamVtables + 0x18;
    *(void**)(small + 0x10) = outputStringStreamVtables + 0xc;
    *(int*)(*(char**)small + kIosVirtualBaseAdjustment) = (small + kStreamStringBuffer) - *(char**)small;
    // Phase 3: application tables and final virtual-ios displacement.
    *(void**)(small + 8) = s_appStringstreamVTables;
    **(void***)small = s_appStringstreamVTables + 0x18;
    *(void**)(small + 0x10) = s_appStringstreamVTables + 0xc;
    *(int*)(*(char**)small + kIosVirtualBaseAdjustment) = (small + kStreamVirtualIos) - *(char**)small;
}

// Attach the float stringbuf to its empty inline string and default format.
static inline void AttachFloatInlineString(char* small) {
    char* data = 0;
    *(int*)(small + kStreamStringTag) = 0;
    *(void**)(small + kStreamStringBuffer) = s_appStringbufVTable;
    *(unsigned char*)(small + kStreamOpenMode) = kOpenForInput | kOpenForOutput;
    *(int*)(small + kStreamStringLength) = 0;
    *(int*)(small + kStreamHeapString) = 0;
    data = *(char**)(small + kStreamHeapString);
    if (!(*(unsigned int*)(small + kStreamStringTag) >> 31))
        data = small + 0x41;
    *(char**)(small + kStreamData) = data;
    if (small[kStreamOpenMode] & kOpenForOutput) {
        *(char**)(small + kStreamPutCurrent) = data;
        *(char**)(small + kStreamPutBegin) = data;
        *(char**)(small + kStreamPutEnd) = data;
    }
    if (small[kStreamOpenMode] & kOpenForInput) {
        *(char**)(small + kStreamGetBegin) = data;
        *(char**)(small + kStreamGetCurrent) = data;
        *(char**)(small + kStreamGetEnd) = data;
    }
    *(unsigned short*)(*(char**)small + kIosFormatFlags) |= kIosFixedDecimal;
    *(int*)(*(char**)small + kIosPrecision) = 0;
}

// Format a finite numeric value for the keyed entry, replacing non-finite values with zero.
// Bit 0 trims only when count / 10^digits > 0: negatives remain unchanged, even
// beyond the requested digit count; digits==0 turns positive values into zero.
// Width and precision are five-bit descriptor values; decimal scale must fit int.
// Requires an existing key, as SetInteger does; only a positive quotient strips leading digits.
int UITextTable_SetFloat(char* self, int key, float value) {
    volatile Byte5262C allocator;
    Byte5262C copy;
    Lock5262C lock;
    TableLock_ctor(&lock, 3, 1);
    int floatClass = __fpclassifyf(value);
    if (floatClass == 1 || __fpclassifyf(value) == 2)
        value = kFloat_0_FontRenderer;
    char* node = *(char**)(self + 0x1c);
    char* found = self + 0x1c;
    while (node) {
        if (((TextTreeNodeView*)node)->key >= key) {
            found = node;
            node = ((TextTreeNodeView*)node)->left;
        } else
            node = ((TextTreeNodeView*)node)->right;
    }
    if (found == self + 0x1c || key < ((TextTreeNodeView*)found)->key)
        found = self + 0x1c;
    char* entry = ((TextTreeNodeView*)found)->entry;
    char* descriptor = *(char**)(entry + kNumericDescriptorOffset);
    float formattedValue = value;
    // Phase 1: the key is resolved; build both stream bases and their virtual-base adjustments.
    Stream5262C stream;
    char* streamBytes = stream.bytes;
    InitializeFloatStreamBases(streamBytes);
    // Phase 2: initialize the buffer/locale and borrow its initially empty inline string storage.
    *(void**)(streamBytes + kStreamStringBuffer) = stringStreamBufferVtable;
    *(int*)(streamBytes + kStreamGetBegin) = 0;
    *(int*)(streamBytes + kStreamGetCurrent) = 0;
    *(int*)(streamBytes + kStreamGetEnd) = 0;
    *(int*)(streamBytes + kStreamPutBegin) = 0;
    *(int*)(streamBytes + kStreamPutCurrent) = 0;
    *(int*)(streamBytes + kStreamPutEnd) = 0;
    stream.copyLocale();
    AttachFloatInlineString(streamBytes);
    UIGroup_SetEnabled(*(void**)(entry + kNumericGroupOffset), 1);
    // Preserve the positive-quotient gate; this is not an absolute-value digit limit.
    if (*(unsigned*)(descriptor + kNumericDescriptorFlagsOffset) & kTrimPositiveDecimalOverflow) {
        int digits = (*(unsigned*)(descriptor + kNumericDescriptorFlagsOffset) >> kDigitLimitShift) & kFormatFieldMask;
        int scale = 1;
        for (int i = 0; i < digits; ++i)
            scale *= 10;
        int quotient = (int)value / scale;
        if (quotient > 0)
            formattedValue = value - quotient * scale;
    }
    if (*(unsigned*)(descriptor + kNumericDescriptorFlagsOffset) & kZeroPadToWidth) {
        *(int*)(*(char**)streamBytes + kIosWidth) =
            (*(unsigned*)(descriptor + kNumericDescriptorFlagsOffset) >> kWidthShift) & kFormatFieldMask;
        (*(char**)streamBytes)[kIosFillCharacter] = 0x30;
    }
    if (*(unsigned*)(descriptor + kNumericDescriptorFlagsOffset) & kSetDecimalPrecision)
        *(int*)(*(char**)streamBytes + kIosPrecision) =
            (*(unsigned*)(descriptor + kNumericDescriptorFlagsOffset) >> kPrecisionShift) & kFormatFieldMask;
    *(unsigned short*)(*(char**)streamBytes + kIosFormatFlags) |= kIosShowPositiveSign;
    // Phase 3: emit the formatted value, then copy it into the entry's own string storage.
    OStream_InsertFloat(streamBytes + 0xc, formattedValue);
    {
        String5262C formattedText;
        MWStringStream_CopyBufferString(&formattedText, streamBytes);
        unsigned int stringTag;
        unsigned int hadHeapText = *(unsigned*)(entry + kNumericTextOffset) >> 31;
        if (!hadHeapText && !((stringTag = formattedText.tag) >> 31)) {
            int newTextLength = formattedText.length;
            *(unsigned*)(entry + kNumericTextOffset) = stringTag;
            *(int*)(entry + kNumericTextLengthOffset) = newTextLength;
            *(char**)(entry + kNumericTextDataOffset) = formattedText.data;
        } else {
            int oldTextLength;
            if (hadHeapText)
                oldTextLength = *(int*)(entry + kNumericTextLengthOffset);
            else
                oldTextLength = *(unsigned char*)(entry + kNumericTextOffset) & 0x7f;
            int newTextLength;
            char* textBegin;
            if (!(formattedText.tag >> 31)) {
                newTextLength = *(unsigned char*)&formattedText & 0x7f;
                textBegin = (char*)&formattedText + 1;
            } else {
                textBegin = formattedText.data;
                newTextLength = formattedText.length;
            }
            copy.value = allocator.value;
            String_ReplaceRange(entry + kNumericTextOffset, 0, oldTextLength, (int)textBegin,
                                (int)(textBegin + newTextLength), &copy);
        }
    }
    // Phase 4: select sign formatting; automatic cleanup destroys the stream and unlocks the table.
    int status = Lock5262C::sign(value);
    UITextTable_ApplyNumericTextFormatFlags(self, entry, status);
    return 1;
}

struct Buffer80253360 {
    int* first;
    unsigned int last;
    int limit;
};

struct Temp80253360 {
    int* first;
    int last;
    int limit;
    // Release the temporary pointer-array allocation, without destroying pointees.
    ~Temp80253360() {
        appFreeUni__FPv(first);
    }
};

// Copy the output stream buffer contents into the destination string.
void OutputStringStream_CopyBufferString(void* dest, char* stream) {
    MWStringBuf_GetString(dest, stream + 8);
}

// Tree owner at table+0x18: count, root, first. Sentinel is &root (table+0x1C).
// Parent bit 0 is colour. Node+0x10 owns an entry; clearing deletes entries before node storage.
struct Node80252C78 {
    Node80252C78* left;
    Node80252C78* right;
    unsigned int parent;
    int key;
    void* payload;
};
struct Entry80252C78 {
    void* first;
    void* second;
};

// Release the two child subtrees and node storage after entry destruction.
static inline void ReleaseTextNodeStorage(char* table, Node80252C78* node) {
    Entry80252C78* links = (Entry80252C78*)node;
    if (links->first)
        RBTree_FreeTrivialSubtree(table + 0x18, links->first);
    if (links->second)
        RBTree_FreeTrivialSubtree(table + 0x18, links->second);
    appFreeUni__FPv(node);
}

// Destroy entry objects and release every node of the text-entry tree.
void UITextTable_ClearEntries(char* self) {
    TableLockGuard lock;
    TableLock_ctor(&lock, 3, 1);
    Node80252C78* node = *(Node80252C78**)(self + 0x20);
    Node80252C78* sentinel = (Node80252C78*)(self + 0x1c);
    // Phase 1: destroy payloads in key order; all links stay live for traversal.
    while (node != sentinel) {
        void* object = node->payload;
        if (object != 0) {
            void** vtable = ((void***)object)[2];
            ((void (*)(void*, int))(vtable[3]))(object, 1);
        }
        Node80252C78* child = node->right;
        if (child != 0) {
            while (child->left != 0)
                child = child->left;
            node = child;
        } else {
            do {
                child = (Node80252C78*)(node->parent & ~1u);
                if (node == child->left)
                    break;
                node = child;
            } while (1);
            node = child;
        }
    }
    // Phase 2: release node storage in postorder, then reset the owner sentinel.
    Node80252C78* root = *(Node80252C78**)(self + 0x1c);
    if (root != 0) {
        Node80252C78* branch = root->left;
        if (branch != 0) {
            Node80252C78* childBranch = branch->left;
            if (childBranch != 0) {
                Node80252C78* descendant = childBranch->left;
                if (descendant != 0) {
                    ReleaseTextNodeStorage(self, descendant);
                }
                descendant = childBranch->right;
                if (descendant != 0) {
                    ReleaseTextNodeStorage(self, descendant);
                }
                appFreeUni__FPv(childBranch);
            }
            childBranch = branch->right;
            if (childBranch != 0) {
                Node80252C78* descendant = childBranch->left;
                if (descendant != 0) {
                    ReleaseTextNodeStorage(self, descendant);
                }
                descendant = childBranch->right;
                if (descendant != 0) {
                    ReleaseTextNodeStorage(self, descendant);
                }
                appFreeUni__FPv(childBranch);
            }
            appFreeUni__FPv(branch);
        }
        branch = root->right;
        if (branch != 0) {
            Node80252C78* childBranch = branch->left;
            if (childBranch != 0) {
                Node80252C78* descendant = childBranch->left;
                if (descendant != 0) {
                    ReleaseTextNodeStorage(self, descendant);
                }
                descendant = childBranch->right;
                if (descendant != 0) {
                    ReleaseTextNodeStorage(self, descendant);
                }
                appFreeUni__FPv(childBranch);
            }
            childBranch = branch->right;
            if (childBranch != 0) {
                Node80252C78* descendant = childBranch->left;
                if (descendant != 0) {
                    ReleaseTextNodeStorage(self, descendant);
                }
                descendant = childBranch->right;
                if (descendant != 0) {
                    ReleaseTextNodeStorage(self, descendant);
                }
                appFreeUni__FPv(childBranch);
            }
            appFreeUni__FPv(branch);
        }
        appFreeUni__FPv(root);
        *(void**)&AsUITextTable(self)->entries.count = 0;
        AsUITextTable(self)->entries.root = 0;
        AsUITextTable(self)->entries.first = self + 0x1c;
    }
}

// Free the pointer-vector allocation, clear its size and optionally delete the descriptor.
void* PtrVector_dtor(void* self, int deleting) {
    if (self != 0 && self != 0 && self != 0 && self != 0 && self != 0) {
        int* p = *(int**)self;
        if (p != 0) {
            ((Buffer80253360*)self)->last = ((Buffer80253360*)self)->last - ((Buffer80253360*)self)->last;
            appFreeUni__FPv(p);
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

// Initialize the pointer vector and reserve its initial 32 elements.
void* PointerVector_InitReserved(Buffer80253360* self) {
    self->first = 0;
    self->last = 0;
    self->limit = 0;
    if (self->last < 32) {
        Buffer80253360 temp = {0, 0, 0};
        temp.first = (int*)appMallocDummy__FUl(0x80);
        temp.limit = 32;
        int* end = self->first + self->last;
        int n = end - self->first;
        memcpy(temp.first + temp.last, self->first, n * 4);
        memset(self->first, 0, n * 4);
        temp.last += n;
        Buffer80253360 old = *self;
        self->first = temp.first;
        self->last = temp.last;
        self->limit = temp.limit;
        temp.first = 0;
        temp.last = 0;
        if (old.first)
            appFreeUni__FPv(old.first);
    }
    return self;
}

// Destroy the output string stream, including its locale and optional virtual ios base.
void* ostringstream_dtor(void* self, short deleting) {
    char* p = (char*)self;
    short zero = 0;
    if (self != 0) {
        char* first = *(char**)p;
        *(int*)(first + 0x3C) = (int)(p + 0x40) - (int)first;
        if (p + 8 != 0) {
            if (p + 0x34 != 0) {
                if ((*(unsigned int*)(p + 0x34) >> 31) != 0)
                    appFreeUni__FPv(*(void**)(p + 0x3C));
            }
            if (p + 8 != 0) {
                if (p + 0x24 != 0) {
                    if (p + 0x24 != 0) {
                        try {
                            if (*(void**)(p + 0x28) != 0)
                                MWSharedOwner_ReleaseReference(*(void**)(p + 0x28));
                        } catch (...) {
                            __unexpected(p + 8);
                        }
                    }
                }
            }
        }
        if (p != 0) {
            first = *(char**)p;
            *(int*)(first + 0x3C) = (int)(p + 8) - (int)first;
        }
        if (deleting != 0 && p + 0x40 != 0)
            std_ios_base_dtor(p + 0x40, zero);
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

int UIGroup_IsEnabled(void*);
void* RBTree_RebalanceAfterErase(void*, void*);
void UIGroup_GetScale(void*, void*);
void* UIGroup_GetTintColor(void*);
void UIGroup_GetPosition(void*, void*);
void UIGroup_GetSourceSize(void*, void*);
int UIGroup_GetCombinedOpacity(void*);
void HudFont_DrawAlignedTextWithVariant(void*, void*, int, int, void*, float, float, float, float);

struct NodeFB8 {
    NodeFB8* left;
    NodeFB8* right;
    unsigned int parentColor;
    char padC[4];
    void* entry;
};
struct TextDescriptorView {
    int rangeStart, rangeEnd;
    unsigned int flags;
};
struct TextMetricsView {
    char prefix[0xAC];
    int height;
};
struct EntryFB8 {
    TextDescriptorView* descriptor;
    void* group;
    void** vtable;
    int hasLayout;
    unsigned int textFlags;
    int textLength;
    char* textData;
    void* fontMetrics;
};

// Read a borrowed entry's byte length from the MSL inline/heap string representation.
static inline int TextEntryByteLength(EntryFB8* entry, unsigned int textFlags) {
    if (textFlags & 0x80000000u)
        return entry->textLength;
    unsigned char inlineLength = *(unsigned char*)&entry->textFlags;
    return inlineLength & 0x7F;
}

// Convert the pane's packed tint and opacity to the font draw colour without owning either.
// Pane opacity is Q7: nominal range 0..128, with 128 opaque (UIGroup setters
// and UIGroup_GetCombinedOpacity in ui_group_and_bp_edit_sprite.cpp). Parent composition multiplies
// local/parent values then shifts by 7, without clamping. The arithmetic here
// maps 0->0 and nonzero n->2*n-1, then keeps only the low eight bits in alpha.
// Values 0..128 produce the monotone byte range 0..255; larger/negative inputs
// wrap rather than clamp; callers must supply nominal pane-opacity values.
// Swap the low/high RGB bytes to match the font draw color packing.
static inline unsigned int PackTextPaneColor(int opacity, unsigned int rgb) {
    int expandedOpacity = (opacity != 0) ? (2 * opacity - 1) : 0;
    unsigned int packed = ((unsigned int)expandedOpacity) << 24;
    packed |= (rgb & 0xFFu) << 16;
    packed |= (rgb & 0x0000FF00u);
    packed |= (rgb >> 16) & 0xFFu;
    return packed;
}

// Draw each enabled nonempty entry with its pane alignment, opacity and font metrics, erasing null entries.
// drawProcessed is a registry-pass latch, not a retrace/frame counter. The
// CFontNumDraw constructor seeds 0; Draw sets 1 BEFORE drawEnabled is tested
// (disabled tables consume it too). DrawList in font_registry_and_round_setup_mixed.cpp calls Draw then resets
// owner+0x24 to 0 even on early return. An earlier direct Draw suppresses that
// registry draw; a direct Draw AFTER the registry can draw again. Setters do not
// reset it. Preserve this order when moving the frame UI phase (game_main_loop_and_startup.cpp).
// Return 1 includes skipped/empty tables; it is not a submitted-glyph count.
int UITextTable_Draw(void* textTable) {
    char* self = (char*)textTable;
    char lock[8];
    TableLock_ctor(lock, 3, 1);
    struct Unlock {
        void* lock;
        // Release the external lock storage on every return, including early draw exits.
        ~Unlock() {
            __dt__9TableLockFv(lock, -1);
        }
    } unlock = {lock};
    // Phase 1: enter the table lock and consume this frame's once-only draw latch.
    if (AsUITextTable(self)->drawProcessed != 0) { // original +0x24
        return 1;
    }
    AsUITextTable(self)->drawProcessed = 1;
    if (AsUITextTable(self)->drawEnabled == 0) { // original +0x28
        return 1;
    }
    NodeFB8* node = (NodeFB8*)AsUITextTable(self)->entries.first;
    NodeFB8* sentinel = (NodeFB8*)(self + 0x1c);
    float half = 0.5f;
    while (sentinel != node) {
        EntryFB8* entry = (EntryFB8*)node->entry;
        // Phase 2 (mutation): remove null entries and continue at the erase result.
        // Maintenance path: cache the successor before erase/free; repair first if needed.
        // Continue from RebalanceAfterErase's result; never advance through the freed node.
        if (entry == 0) {
            NodeFB8* old = node;
            if (node == (NodeFB8*)AsUITextTable(self)->entries.first) {
                NodeFB8* right = node->right;
                NodeFB8* nxt;
                if (right != 0) {
                    NodeFB8* c = right;
                    while (c->left != 0)
                        c = c->left;
                    nxt = c;
                } else {
                    NodeFB8* c2 = node;
                    while (1) {
                        NodeFB8* parent = (NodeFB8*)(c2->parentColor & ~1u);
                        if (c2 == parent->left) {
                            nxt = parent;
                            break;
                        }
                        c2 = parent;
                    }
                }
                node = nxt;
                AsUITextTable(self)->entries.first = node;
            }
            NodeFB8* nextNode = (NodeFB8*)RBTree_RebalanceAfterErase(
                old, *(void**)(self + 0x1c)); // retain raw form: field access reused sentinel register
            appFreeUni__FPv(old);
            AsUITextTable(self)->entries.count = AsUITextTable(self)->entries.count - 1;
            if (sentinel == nextNode) {
                return 1;
            }
            node = nextNode;
            continue;
        }
        // Phase 3 (rendering): retained nodes only; tree links stay intact in this block.
        // Resolve pane visibility and string/metrics readiness before any font draw state is written.
        if (entry != 0) {
            void* obj = entry->group;
            if (UIGroup_IsEnabled(obj) != 0) {
                float scale[2];
                UIGroup_GetScale(obj, scale);
                void* groupTint = UIGroup_GetTintColor(obj);
                unsigned int textFlags = entry->textFlags;
                int textLength = TextEntryByteLength(entry, textFlags);
                if (textLength != 0 && entry->fontMetrics != 0) {
                    void** entryVtable = entry->vtable;
                    int (*isReady)(void*) = (int (*)(void*))entryVtable[2];
                    int entryReady = isReady(entry);
                    if (entryReady != 0 && entry->hasLayout != 0) {
                        void* metrics = entry->fontMetrics;
                        int fontHeight = ((TextMetricsView*)metrics)->height;
                        float scaleX = scale[0];
                        float scaledHeight = (float)fontHeight;
                        float position[2];
                        float size[2];
                        UIGroup_GetPosition(obj, position);
                        UIGroup_GetSourceSize(obj, size);
                        // Phase 3a: align in pane/screen units, keeping the existing X-scaled baseline.
                        float textX = position[0];
                        float sum = position[1] + size[1];
                        AsUITextTable(self)->horizontalAlignment = kTextAlignLeft; // original +0x10
                        unsigned int flags = entry->descriptor->flags;
                        // Baseline is pane bottom minus font height scaled by X (existing behavior).
                        float baselineY = sum - scaledHeight * scaleX;
                        float textY = baselineY;
                        if (flags & kAlignTextRight) {
                            float f0 = position[0] + size[0];
                            AsUITextTable(self)->horizontalAlignment = kTextAlignRight;
                            textX = f0;
                        } else if (flags & kAlignTextCenter) {
                            float f0 = half * size[0] + position[0];
                            AsUITextTable(self)->horizontalAlignment = kTextAlignCenter;
                            textX = f0;
                        }
                        int opacity = UIGroup_GetCombinedOpacity(obj);
                        unsigned int rgb = (unsigned int)(unsigned long)groupTint;
                        unsigned int packed = PackTextPaneColor(opacity, rgb);
                        AsUITextTable(self)->packedColor = packed; // original +0x0C
                        // Phase 3b: resolve the borrowed string bytes immediately before drawing.
                        unsigned int currentTextFlags = entry->textFlags;
                        int hasHeapString = (currentTextFlags >> 31) & 1;
                        unsigned int descriptorFlags = entry->descriptor->flags;
                        int fontVariant = (descriptorFlags >> kFontVariantShift) & 1;
                        void* textBytes;
                        if (hasHeapString)
                            textBytes = entry->textData;
                        else
                            textBytes = (char*)&entry->textFlags + 1;
                        HudFont_DrawAlignedTextWithVariant(
                            self, textBytes, fontVariant, 0, &entry->fontMetrics, textX + AsUITextTable(self)->offsetX,
                            textY + AsUITextTable(self)->offsetY, 0.0f, scaleX); // original +0x2C/+0x30
                    }
                }
            }
        }
        // Phase 4: advance a retained node in key order, stripping parent colour bits.
        NodeFB8* cur = node;
        NodeFB8* right = cur->right;
        if (right != 0) {
            NodeFB8* c = right;
            while (c->left != 0)
                c = c->left;
            node = c;
        } else {
            NodeFB8* c2 = cur;
            while (1) {
                NodeFB8* parent = (NodeFB8*)(c2->parentColor & ~1u);
                if (c2 == parent->left) {
                    node = parent;
                    break;
                }
                c2 = parent;
            }
        }
    }
    return 1;
}

} // extern "C"
