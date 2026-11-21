#include "classes/CFxSequence.h"
#include "classes/CFxSpray.h"
#include "game/types.h"
#include "game/wvector.h"
// Translation unit 0x8010DCF0-0x8010F380 (6 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"

// Construct into caller-owned storage; allocation and release remain the caller's job.
inline void* operator new(size_t, void* p) { return p; }
extern "C" {

// Circular owned list: next points toward older parsed lines, prev is execution order.
// The sentinel contains links only. Each 0x11C-byte node owns its inline command text.
// Timed range is [earliestFrame,latestFrame), except equal endpoints mean a fixed time.
// value starts at earliestFrame, then range randomization resolves the script time;
// unsigned -1 marks a label and -2 marks an immediate command.
struct Node {
    Node* next;
    Node* prev;
    unsigned int sourceLine;
    unsigned int earliestFrame;
    unsigned int latestFrame;
    unsigned int value;
    unsigned char commandText[0x104];
};

struct CommandListHead { Node* next; Node* prev; };

// Mirrors only timeline state; the sentinel owns two links, without a command payload.
struct State {
    unsigned char baseStorage[0x80];
    unsigned char playbackEnabled;
    unsigned char attachmentStorage[0x1F];
    float elapsed;
    float timer;
    unsigned int failedCommandMetadata[4];
    unsigned char failedCommandPrefix[8];
    unsigned int failedCommandWord;
    unsigned char failedCommandRemainder[0xFC];
    unsigned int count;
    CommandListHead sentinel;
};

extern float kFloat_0_SoundEmitter;
extern float kFloat_0_5_FxSequence;
// Twelve-byte engine string: heap length/data overlap inline characters.
struct SequenceNameStringRep { unsigned int flag, heapLength, heapDataAddress; };
void* StringTree_FindInsertionPosition(void*, SequenceNameStringRep*, void**, unsigned char*, unsigned char*);
void* NamedObjectMap_InsertNode(void*, void*, unsigned char, unsigned char, void*);
void String_ReserveCapacity(SequenceNameStringRep*, unsigned int);
void String_ReplaceRange(SequenceNameStringRep*, int, int, unsigned int, unsigned int, unsigned char*);
extern unsigned int s_fxSequenceNameDefaultValue;
unsigned long Runtime_ConvertDoubleToUnsigned(double);
void* appMallocDummy__FUl(unsigned int);
void appFreeUni__FPv(void*);
// Execute a script command: create a spray, append a track, attach it, play sound, jump or stop.
int CFxSequence_ExecuteScriptCommand(void*, signed char*, unsigned char*);



struct BigString { unsigned char data[0x12c]; };
// Packed source strings; array extents include terminators and original padding.
struct FxSequenceParserStrings {
    unsigned char defaultSequence[11];
    unsigned char pad_0B[1];
    unsigned char countFormat[3];
    unsigned char timeSeparator[2];
    unsigned char lineEnd[3];
    unsigned char rangeSeparator[2];
    unsigned char whitespace[5];
    unsigned char pad_1B[1];
    unsigned char assignment[2];
    unsigned char argumentSeparator[2];
    unsigned char argumentTrim[6];
    unsigned char spray[6];
    unsigned char strongBond[11];
    unsigned char cameraSide[11];
    unsigned char addSpawn[10];
    unsigned char attached[9];
    unsigned char interval[9];
    unsigned char age[4];
    unsigned char onDeath[8];
    unsigned char onGround[9];
    unsigned char addRevolution[15];
    unsigned char floatFormat[3];
    unsigned char addRotation[13];
    unsigned char addNormalRotation[14];
    unsigned char addParentRevolution[16];
    unsigned char attachedToParent[17];
    unsigned char on[3];
    unsigned char integerFormat[3];
    unsigned char sound[6];
    unsigned char waveExtension[5];
    unsigned char jumpToLabel[5];
    unsigned char stop[5];
};
extern FxSequenceParserStrings fxSequenceParserStrings;

#define FX_OFFSET(type, field) ((int)&((type*)0)->field)
enum {
    kCommandTextCapacity = 0x104,
    kSprayActionListOffset = 0x34C,
    kSprayParentOrbitListOffset = 0x2CC,
    kCommandArgumentCount = 16,
    kCommentPrefix = ';',
    // Unsigned arithmetic is retained for the original wrapped sentinel tests.
    kCommandSentinelBias = 0x10000,
    kLabelCommandTime = -1, kImmediateCommandTime = -2,
    kLabelBiasedTime = 0xFFFF,
    kImmediateBiasedTime = 0xFFFE,
    kSpawnAtInterval = 0,
    kSpawnAtAge = 1,
    kSpawnOnDeath = 2,
    kSpawnOnGround = 3,
    kParentAxisX = 1, kParentAxisY = 2, kParentAxisZ = 4
};
struct SequenceAttachmentView {
    char pad_00[8]; Vec3f wind;
    char pad_14[0x4C-0x14]; float cameraMatrix[12];
};
struct SprayAttachmentView {
    char pad_00[0xC]; float attachmentX, attachmentY;
    char pad_14[0xB8-0x14]; WVector position;
    char pad_C4[12]; WVector velocity;
};
struct CameraTransformView { Vec3f axes[3]; Vec3f position; };
struct EffectBoxTimeScale { void* vtable; float timeScale; };
struct RegistryNode { char links[12]; void* key; void* value; };
struct NameTreeNode { char links[12]; SequenceNameStringRep key; unsigned int value; };

// Return the name/value pair stored after a cache iterator node's three tree links.
void* FxSequenceCache_GetIteratorValue(void* p) {
    return &((NameTreeNode*)*(void**)p)->key;
}

struct TableRoot { unsigned char pad[0x38]; void* table; };
extern TableRoot s_cLocaleData;
void cTokenV_Init(void*);
void cTokenV_SetInputBuffer(void*, const char*, unsigned int);
int cToken_ReadUntilDelimiter(void*, char*, const unsigned char*, int);
void cTokenS_Init(void*);
void Buffer_SetData(void*, const char*, unsigned int);
void String_TrimCharacterSet(char*, const unsigned char*);
void cTokenS_dtor(void*, short);
void cToken_dtor(void*, short);
unsigned int String_ParseDecimalInt(const char*);

struct Link8 { Link8* next; Link8* prev; };
struct Entry114 { unsigned int sourceLine; unsigned int earliestFrame; unsigned int latestFrame; int value; char commandText[kCommandTextCapacity]; };
struct Entry120 { Link8 link; Entry114 val; };
struct AllocDEC8 {
    unsigned char allocator;
    Entry120* result;
    Entry120* allocated;
    unsigned int reserved;
    Link8* head;
    Entry120* cleanup;
    // Allocate an unlinked command node, owned by cleanup until insertion commits.
    AllocDEC8(Link8* h): allocator(0) {
        result = (Entry120*)appMallocDummy__FUl(0x11C);
        allocated = result;
        head = h;
        cleanup = allocated;
    }
    // Free an uncommitted node on scope exit.
    ~AllocDEC8() { if (cleanup) appFreeUni__FPv(cleanup); }
};
// Release the stack range tokenizer without deleting its enclosing storage.
struct TokenS { unsigned char data[0x18]; ~TokenS() { cTokenS_dtor(this, (short)-1); } };
// Release the stack line tokenizer without deleting its enclosing storage.
// The token object is 0x18 bytes: cTokenV_Init writes its owned buffer at +0x10 and its input buffer at +0x14;
// cToken_dtor frees +0x10.
struct TokenV { unsigned char data[0x10]; ~TokenV() { cToken_dtor(this, (short)0); } };

#pragma opt_common_subs off
#pragma opt_propagation off
#pragma opt_lifetimes off
#pragma use_lmw_stmw on
// Parse label, immediate or time-range commands and prepend them to the sequence's command list.
void CFxSequence_ParseEntryLine(void* self, const char* input, unsigned int sourceLine) {
    if ((signed char)*input == 0) return;
    if ((signed char)*input == kCommentPrefix) return;
    {
        __alloca(0);
        char firstTimeOrLabel[kCommandTextCapacity];
        char lastTime[kCommandTextCapacity];
        char commandText[kCommandTextCapacity];
        Entry120 node;
        TokenV lineTokens;
        TokenS rangeTokens;
        // Phase 1: split time/label, optional range end, and the command body.
        // The parser assumes tokens fit the 0x104-byte destination arrays.
        cTokenV_Init(&lineTokens);
        cTokenV_SetInputBuffer(&lineTokens, input, strlen(input));
        cToken_ReadUntilDelimiter(&lineTokens, firstTimeOrLabel, fxSequenceParserStrings.timeSeparator, 1);
        cToken_ReadUntilDelimiter(&lineTokens, commandText, fxSequenceParserStrings.lineEnd, 2);
        cTokenS_Init(&rangeTokens);
        Buffer_SetData(&rangeTokens, firstTimeOrLabel, strlen(firstTimeOrLabel));
        cToken_ReadUntilDelimiter(&rangeTokens, firstTimeOrLabel, fxSequenceParserStrings.rangeSeparator, 1);
        cToken_ReadUntilDelimiter(&rangeTokens, lastTime, fxSequenceParserStrings.rangeSeparator, 1);
        String_TrimCharacterSet(firstTimeOrLabel, fxSequenceParserStrings.whitespace);
        String_TrimCharacterSet(lastTime, fxSequenceParserStrings.whitespace);
        String_TrimCharacterSet(commandText, fxSequenceParserStrings.whitespace);
        // Phase 2: encode labels/immediate commands with sentinel times; ordinary
        // commands retain both range endpoints for later randomization.
        // First-character test: for an ASCII first byte (0..0x7F) the C locale class table is indexed by it and
        // bit 0 of the entry decides "this line starts with a label"; a byte >= 0x80 (negative as signed char)
        // is never a label. The meaning of table bit 0 is not established here (it is the table's first class bit).
        // Line kinds: label (value = kLabelCommandTime), empty first field (value = kImmediateCommandTime),
        // otherwise "<first>[-<last>]: <command>" with decimal frame numbers.
        // Non-ASCII bytes bypass the locale classification table.
        unsigned char firstByte = firstTimeOrLabel[0];
        int isNonAscii = 1;
        node.val.sourceLine = sourceLine;
        if ((signed char)firstByte >= 0 && (signed char)firstByte < 0x100) isNonAscii = 0;
        int startsLabel = isNonAscii ? 0 : (((unsigned short*)*(void**)((char*)s_cLocaleData.table + 8))[(signed char)firstByte] & 1);
        if (startsLabel) {
            node.val.value = kLabelCommandTime;
            strcpy(node.val.commandText, firstTimeOrLabel);
        } else if ((signed char)firstByte == 0) {
            node.val.value = kImmediateCommandTime;
            strcpy(node.val.commandText, commandText);
        } else {
            node.val.earliestFrame = String_ParseDecimalInt(firstTimeOrLabel);
            node.val.latestFrame = lastTime[0] ? String_ParseDecimalInt(lastTime) : node.val.earliestFrame;
            node.val.value = (int)node.val.earliestFrame;
            strcpy(node.val.commandText, commandText);
        }
        {
            // Phase 3: copy into a guarded node before splicing it at the front.
            // Walking prev from the tail later restores file order.
            Link8* head = (Link8*)((char*)&((CFxSequence*)self)->firstCommandNode - sizeof(void*));
            AllocDEC8 guard(head);
            Entry120* allocatedNode = guard.allocated;
            new(&allocatedNode->val) Entry114(node.val);
            guard.cleanup = 0;
            Link8* n = &allocatedNode->link;
            head->next->prev = n;
            n->next = head->next;
            head->next = n;
            n->prev = head;
            ++(*(int*)&((CFxSequence*)self)->commandCount);
        }
        Link8** cur = (Link8**)&((CFxSequence*)self)->nextCommand; // +0x1CC
        if (*cur == (Link8*)((char*)&((CFxSequence*)self)->firstCommandNode - sizeof(void*))) *cur = ((Link8*)((char*)&((CFxSequence*)self)->firstCommandNode - sizeof(void*)))->prev;
    }
}
#pragma opt_common_subs reset
#pragma opt_propagation reset
#pragma opt_lifetimes reset
#pragma use_lmw_stmw reset

extern float kFloat_neg1_FxBox, kFloat_1_SoundEmitter;
extern unsigned char s_cameraTransform[];
void Matrix_CopyAffine(void*, const char*);
double atof(void*);
void Matrix_TransformDirection(void*, float*, const char*);
void __as__7WVectorFRC7WVector(void*, void*);
int CString_Compare(const char*, const char*);
void* CFxSpray_CreateFromScript(void*, void*, int, unsigned char);
void CFxSpray_SetWindVector(void*, void*);
int CFxRegistryTree_FindInsertionPosition(void*, void*, void*, unsigned char*, unsigned char*);
void* FxBoxTree_InsertNode(void*, int, unsigned char, unsigned char, void*);
void CFxBox_RegisterSpray(void*, void*);
float CFxSpray_ParseScaledFloat(void*, const char*);
void* appMallocDummy__FUl(unsigned int);
void Sound_PlayByName(void*, const char*, void*, int, int, int, float, float, float);
int String_CompareCString(const char*, const char*);
void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
// Choose times in [earliestFrame, latestFrame) and sort timed commands within label/immediate boundaries.
void CFxSequence_RandomizeAndSortCommandTimes(State*);
extern void* g_soundManager;
extern unsigned int s_fxRegistryDefaultMappedWord;
extern float kFloat_200_FxSequence, kFloat_2_FxSequence, kFloat_neg2_FxSequence;
typedef Vec3f Vec190;
// Track payloads follow two links in the owning spray's circular lists.
// Consumer evidence: fx_spray_particles.cpp's axis-rate offsets are node+08 axis, +14/+18
// start time/rate, +1C/+20 end time/rate. It linearly interpolates b..d over a..c,
// tests inclusive time endpoints, and applies rate * scaledDeltaTime as an angle.
// Pivot-orbit consumers subtract/add v around rotation and use w as its axis.
// Thus v=axis for the short payload, v=pivot and w=axis for the orbit payload.
// Times use emitter elapsed-time units; rates use angle units per elapsed-time unit.
// Both integer times truncate the effect-box-scaled script value toward zero.
// Preserve the payload storage types and constructor copy order.
struct SequenceAxisRateTrack {
    Vec190 axis; // rotation axis; never normalized by this parser.
    int startTime; float startAngularRate; int endTime; float endAngularRate; // start time/rate, end time/rate.
    // Leave fields for the command parser to populate.
    SequenceAxisRateTrack() {}
    // Copy a single-vector track payload before linking its node.
    SequenceAxisRateTrack(const SequenceAxisRateTrack& t):axis(t.axis),startTime(t.startTime),startAngularRate(t.startAngularRate),endTime(t.endTime),endAngularRate(t.endAngularRate) {}
};
struct SequencePivotOrbitTrack {
    Vec190 pivot,axis; // pivot in emitter-position coordinates, then rotation axis.
    int startTime; float startAngularRate; int endTime; float endAngularRate; // start time/rate, end time/rate.
    // Leave fields for the command parser to populate.
    SequencePivotOrbitTrack() {}
    // Copy a two-vector track payload before linking its node.
    SequencePivotOrbitTrack(const SequencePivotOrbitTrack& t):pivot(t.pivot),axis(t.axis),startTime(t.startTime),startAngularRate(t.startAngularRate),endTime(t.endTime),endAngularRate(t.endAngularRate) {}
};
struct Action190 { int type; char childSpray[kCommandTextCapacity]; bool attached; char pad[3]; float value; int count; };
struct Link190 { Link190* next; Link190* prev; };
struct Guard190 {
    void* p;
    // Own an allocation until the caller clears p after successful construction.
    Guard190(void* value):p(value) {}
    // Free a node whose construction/insertion did not commit.
    ~Guard190() { if(p) appFreeUni__FPv(p); }
};
struct Source190 {
    unsigned char data[0x18];
    // Initialize the script-command tokenizer in this stack storage.
    Source190() { cTokenV_Init((BigString*)this); }
    // Release tokenizer state without freeing the stack object.
    ~Source190() { cToken_dtor(this,0); }
};
struct Source190Storage { unsigned char pad[0xd8]; Source190 source; };
// Kept as a scoped macro: an inline template changes the caller's register lifetimes.
// Copy before linking, so the guard frees the allocation if construction fails.
#define APPEND_SPRAY_TRACK(Track, value, listOffset) { \
    char* owner=(char*)(*(void**)&((CFxSequence*)self)->currentSpray); \
    Link190* head=(Link190*)(owner+listOffset); \
    Link190* node=(Link190*)appMallocDummy__FUl(sizeof(Track)+8); \
    Guard190 guard(node); \
    Track* data=(Track*)(node+1); \
    new(data) Track(value); \
    guard.p=0; \
    head->next->prev=node; \
    node->next=head->next; \
    head->next=node; \
    node->prev=head; \
    ++*(unsigned int*)(owner+listOffset-4); \
}

enum SequenceTrackArgument {
    kAxisTrackStartTimeArgument = 3, kAxisTrackStartRateArgument = 4,
    kAxisTrackEndTimeArgument = 5, kAxisTrackEndRateArgument = 6,
    kOrbitTrackStartTimeArgument = 6, kOrbitTrackStartRateArgument = 7,
    kOrbitTrackEndTimeArgument = 8, kOrbitTrackEndRateArgument = 9
};

#pragma push
#pragma opt_common_subs off
// Decode axis and inclusive start/end time/rate pairs into caller-owned track storage.
static inline void DecodeSequenceAxisRateTrack(void* self, char args[][kCommandTextCapacity], SequenceAxisRateTrack& track) {
    sscanf(args[0],(char*)fxSequenceParserStrings.floatFormat,&track.axis.x);
    sscanf(args[1],(char*)fxSequenceParserStrings.floatFormat,&track.axis.y);
    sscanf(args[2],(char*)fxSequenceParserStrings.floatFormat,&track.axis.z);
    track.startTime=(int)CFxSpray_ParseScaledFloat(((CFxSequence*)self)->sequenceOwner,args[kAxisTrackStartTimeArgument]);
    track.startAngularRate=CFxSpray_ParseScaledFloat(((CFxSequence*)self)->sequenceOwner,args[kAxisTrackStartRateArgument]);
    track.endTime=(int)CFxSpray_ParseScaledFloat(((CFxSequence*)self)->sequenceOwner,args[kAxisTrackEndTimeArgument]);
    track.endAngularRate=CFxSpray_ParseScaledFloat(((CFxSequence*)self)->sequenceOwner,args[kAxisTrackEndRateArgument]);
}
#pragma pop

#pragma push
#pragma opt_common_subs off
// Decode pivot/axis and inclusive start/end time/rate pairs into caller-owned orbit storage.
static inline void DecodeSequencePivotOrbitTrack(void* self, char args[][kCommandTextCapacity], SequencePivotOrbitTrack& track) {
    sscanf(args[0],(char*)fxSequenceParserStrings.floatFormat,&track.pivot.x);
    sscanf(args[1],(char*)fxSequenceParserStrings.floatFormat,&track.pivot.y);
    sscanf(args[2],(char*)fxSequenceParserStrings.floatFormat,&track.pivot.z);
    sscanf(args[3],(char*)fxSequenceParserStrings.floatFormat,&track.axis.x);
    sscanf(args[4],(char*)fxSequenceParserStrings.floatFormat,&track.axis.y);
    sscanf(args[5],(char*)fxSequenceParserStrings.floatFormat,&track.axis.z);
    track.startTime=(int)CFxSpray_ParseScaledFloat(((CFxSequence*)self)->sequenceOwner,args[kOrbitTrackStartTimeArgument]);
    track.startAngularRate=CFxSpray_ParseScaledFloat(((CFxSequence*)self)->sequenceOwner,args[kOrbitTrackStartRateArgument]);
    track.endTime=(int)CFxSpray_ParseScaledFloat(((CFxSequence*)self)->sequenceOwner,args[kOrbitTrackEndTimeArgument]);
    track.endAngularRate=CFxSpray_ParseScaledFloat(((CFxSequence*)self)->sequenceOwner,args[kOrbitTrackEndRateArgument]);
}
#pragma pop

// Script command schema (key = arg0, arg1, ...; up to kCommandArgumentCount args, each trimmed). Keys are
// compared against fxSequenceParserStrings; the args below are positional, 0-based. Units are those of the
// script text; values read with CFxSpray_ParseScaledFloat are scaled by the owning effect box.
//   spray       args[0..]  = spray definition passed to CFxSpray_CreateFromScript; args[1..3] = position xyz,
//                            args[4..6] = velocity xyz, args[7] = "strong bond" flag text (particlesFollowEmitter),
//                            args[8] = "camera side" flag text (position/velocity rotated by camera basis).
//   addSpawn    args[0] child spray name; args[1] attached flag text; args[2] mode: interval (args[3] value,
//               args[4] count), age (args[3] value), on-death, on-ground.
//   addRevolution / addParentRevolution  args[0..2] pivot xyz, args[3..5] axis xyz;
//                                      args[6..9] start time/rate, end time/rate (times truncated).
//   addRotation / addNormalRotation      args[0..2] axis xyz; args[3..6] start time/rate, end time/rate.
//               Rates are interpolated in emitter time, then multiplied by the scaled update delta.
//   attachedToParent  args[0] on/off; args[1..3] nonzero -> set X/Y/Z bit of parentRotationAxisMask.
//   sound       args[0] wave name (".wav"-style extension stripped); args[1] range (default -1: negative = play
//               non-positional, otherwise positional at the sequence position relative to the camera).
//   jumpToLabel args[0] label name: re-sorts commands and moves the timeline to the label's time.
//   stop        ends the sequence (next command = list end).
// Results: return 1 = command consumed, 0 = stop/retry this update; *result = 1 flags an unknown key or a failed
// spray load. Track commands without a current spray do nothing.
#pragma opt_common_subs off
// Execute a script command: create a spray, append a track, attach it, play sound, jump or stop.
int CFxSequence_ExecuteScriptCommand(void* self, signed char* input, unsigned char* result) {
    // Borrow input; result is required and cleared before parsing. Return 1 to
    // advance, 0 to retry/stop this update. result=1 marks spray-load failure or
    // an unknown command; a jump near the current time may return 0 with result=0.
    __alloca(0);
    signed char first=*input;
    *result=0;
    if (first == 0 || first == kCommentPrefix) return 1;
    Source190Storage storage;
    Source190& source=storage.source;
    BigString& src=*(BigString*)&source;
    char key[kCommandTextCapacity];
    char args[kCommandArgumentCount][kCommandTextCapacity];
    char scratch[kCommandTextCapacity];
    // Phase 1: tokenize the key and up to sixteen arguments into caller-frame buffers.
    cTokenV_SetInputBuffer(&src, (char*)input, strlen((char*)input));
    cToken_ReadUntilDelimiter(&src, key, fxSequenceParserStrings.assignment, 1);
    String_TrimCharacterSet(key, fxSequenceParserStrings.whitespace);
    for (int i=0;i<kCommandArgumentCount;i++) {
        cToken_ReadUntilDelimiter(&src, args[i], fxSequenceParserStrings.argumentSeparator, 1);
        String_TrimCharacterSet(args[i], fxSequenceParserStrings.argumentTrim);
    }
    // Phase 2: dispatch one command; each add* copies its stack payload before linking.
    // Dispatch map: spray creates/registers the current emitter; add* copies a
    // track onto that emitter; attachedToParent sets axis bits; sound/jump/stop
    // act on the sequence. add* without a current spray succeeds without work.
    int ok=1;
    if (!*key) { return 1; }
    if (!CString_Compare(key, (char*)fxSequenceParserStrings.spray)) {
        // 0x8010E190: disabled playback skips spray creation.
        if ((*(unsigned char*)&((CFxSequence*)self)->playbackEnabled)) {
            void* obj=CFxSpray_CreateFromScript(((CFxSequence*)self)->sequenceOwner, args, 0, (*(unsigned char*)&((CFxSequence*)self)->bypassResourceCache));
            if (!obj) { *result=1; return 0; }
            (*(bool*)&((CFxSpray*)obj)->particlesFollowEmitter)=CString_Compare(args[7], (char*)fxSequenceParserStrings.strongBond)==0;
            (*(void**)&((CFxSpray*)obj)->parentSequence)=self;
            ((SprayAttachmentView*)obj)->attachmentX=((CFxSequence*)self)->attachmentPositionX;
            ((SprayAttachmentView*)obj)->attachmentY=((CFxSequence*)self)->attachmentPositionY;
            ((CFxSpray*)obj)->unk_4=(*(unsigned char*)&((CFxSequence*)self)->unk_4);
            CFxSpray_SetWindVector(obj,&((SequenceAttachmentView*)self)->wind);
            // cameraSide rotates position and velocity by the stored camera basis.
            // Otherwise position is copied directly, but velocity is still rotated.
            // Matrix_TransformDirection omits translation in both branches.
            if (!CString_Compare(args[8],(char*)fxSequenceParserStrings.cameraSide)) {
                Matrix_CopyAffine((char*)((SequenceAttachmentView*)self)->cameraMatrix,(char*)s_cameraTransform);
                float z=(float)atof(args[3]);
                float y=(float)atof(args[2]);
                float x=(float)atof(args[1]);
                Vec190 v={x,y,z}, out;
                void* dest=&((SprayAttachmentView*)obj)->position;
                Matrix_TransformDirection(&out,&v.x,(char*)((SequenceAttachmentView*)self)->cameraMatrix);
                *(WVector*)dest = *(WVector*)&out;
                float z2=(float)atof(args[6]);
                float y2=(float)atof(args[5]);
                float x2=(float)atof(args[4]);
                Vec190 w={x2,y2,z2}, out2;
                void* dest2=&((SprayAttachmentView*)obj)->velocity;
                Matrix_TransformDirection(&out2,&w.x,(char*)((SequenceAttachmentView*)self)->cameraMatrix);
                *(WVector*)dest2 = *(WVector*)&out2;
            } else {
                float z=(float)atof(args[3]);
                float y=(float)atof(args[2]);
                float x=(float)atof(args[1]);
                Vec190 v={x,y,z};
                *(WVector*)(&((SprayAttachmentView*)obj)->position) = *(WVector*)&v;
                float z2=(float)atof(args[6]);
                float y2=(float)atof(args[5]);
                float x2=(float)atof(args[4]);
                Vec190 w={x2,y2,z2}, out;
                void* dest=&((SprayAttachmentView*)obj)->velocity;
                Matrix_TransformDirection(&out,&w.x,(char*)((SequenceAttachmentView*)self)->cameraMatrix);
                *(WVector*)dest = *(WVector*)&out;
            }
            ((CFxSpray*)obj)->drawEnabled=(*(unsigned char*)&((CFxSequence*)self)->playbackEnabled);
            // Register after attachment/wind setup; the effect box receives the spray.
            void* found;
            unsigned char a,b;
            int position=CFxRegistryTree_FindInsertionPosition(&((CFxSequence*)self)->sprayCount,&obj,&found,&a,&b);
            void** slot=&((RegistryNode*)found)->key;
            if (!found || (unsigned int)*slot < (unsigned int)obj) {
                void* pair[2]={obj,(void*)s_fxRegistryDefaultMappedWord};
                void* n=FxBoxTree_InsertNode(&((CFxSequence*)self)->sprayCount,position,a,b,pair);
                slot=&((RegistryNode*)n)->key;
            }
            slot[1]=obj;
            CFxBox_RegisterSpray(((CFxSequence*)self)->sequenceOwner,obj);
            (*(void**)&((CFxSequence*)self)->currentSpray)=obj;
        }
    } else if (!CString_Compare(key,(char*)fxSequenceParserStrings.addSpawn)) {
        if ((*(void**)&((CFxSequence*)self)->currentSpray)) {
            Action190 action;
            strcpy(action.childSpray,args[0]);
            action.attached=CString_Compare(args[1],(char*)fxSequenceParserStrings.attached)==0;
            if (!CString_Compare(args[2],(char*)fxSequenceParserStrings.interval)) {
                action.type=kSpawnAtInterval;
                action.value=CFxSpray_ParseScaledFloat(((CFxSequence*)self)->sequenceOwner,args[3]);
                action.count=(int)CFxSpray_ParseScaledFloat(((CFxSequence*)self)->sequenceOwner,args[4]);
            } else if (!CString_Compare(args[2],(char*)fxSequenceParserStrings.age)) {
                action.type=kSpawnAtAge;
                action.value=CFxSpray_ParseScaledFloat(((CFxSequence*)self)->sequenceOwner,args[3]);
            } else if (!CString_Compare(args[2],(char*)fxSequenceParserStrings.onDeath)) action.type=kSpawnOnDeath;
            else if (!CString_Compare(args[2],(char*)fxSequenceParserStrings.onGround)) action.type=kSpawnOnGround;
            // The spray owns the copied list node; action is only a stack payload.
            APPEND_SPRAY_TRACK(Action190, action, kSprayActionListOffset);
        }
    // Orbit/rotation tracks share the start-time/rate, end-time/rate policy described above.
    } else if (!CString_Compare(key,(char*)fxSequenceParserStrings.addRevolution)) {
        if ((*(void**)&((CFxSequence*)self)->currentSpray)) {
            SequencePivotOrbitTrack track;
            DecodeSequencePivotOrbitTrack(self, args, track);
            APPEND_SPRAY_TRACK(SequencePivotOrbitTrack, track, FX_OFFSET(CFxSpray, emitterOrbitTrackCount) + sizeof(unsigned int));
        }
    } else if (!CString_Compare(key,(char*)fxSequenceParserStrings.addRotation)) {
        if ((*(void**)&((CFxSequence*)self)->currentSpray)) {
            SequenceAxisRateTrack track;
            DecodeSequenceAxisRateTrack(self, args, track);
            APPEND_SPRAY_TRACK(SequenceAxisRateTrack, track, FX_OFFSET(CFxSpray, particleOrbitTrackCount) + sizeof(unsigned int));
        }
    } else if (!CString_Compare(key,(char*)fxSequenceParserStrings.addNormalRotation)) {
        if ((*(void**)&((CFxSequence*)self)->currentSpray)) {
            SequenceAxisRateTrack track;
            DecodeSequenceAxisRateTrack(self, args, track);
            APPEND_SPRAY_TRACK(SequenceAxisRateTrack, track, FX_OFFSET(CFxSpray, velocityRotationTrackCount) + sizeof(unsigned int));
        }
    } else if (!CString_Compare(key,(char*)fxSequenceParserStrings.addParentRevolution)) {
        if ((*(void**)&((CFxSequence*)self)->currentSpray)) {
            SequencePivotOrbitTrack track;
            DecodeSequencePivotOrbitTrack(self, args, track);
            APPEND_SPRAY_TRACK(SequencePivotOrbitTrack, track, kSprayParentOrbitListOffset);
        }
    } else if (!String_CompareCString(key,(char*)fxSequenceParserStrings.attachedToParent)) {
        if ((*(void**)&((CFxSequence*)self)->currentSpray)) {
            ((CFxSpray*)(*(void**)&((CFxSequence*)self)->currentSpray))->attachedToParent=0;
            if (!String_CompareCString(args[0],(char*)fxSequenceParserStrings.on)) ((CFxSpray*)(*(void**)&((CFxSequence*)self)->currentSpray))->attachedToParent=1;
            int xEnabled=0,yEnabled=0,zEnabled=0;
            sscanf(args[1],(char*)fxSequenceParserStrings.integerFormat,&xEnabled);
            sscanf(args[2],(char*)fxSequenceParserStrings.integerFormat,&yEnabled);
            sscanf(args[3],(char*)fxSequenceParserStrings.integerFormat,&zEnabled);
            (*(int*)&((CFxSpray*)(*(void**)&((CFxSequence*)self)->currentSpray))->parentRotationAxisMask)=0;
            if(xEnabled) (*(int*)&((CFxSpray*)(*(void**)&((CFxSequence*)self)->currentSpray))->parentRotationAxisMask)|=kParentAxisX;
            if(yEnabled) (*(int*)&((CFxSpray*)(*(void**)&((CFxSequence*)self)->currentSpray))->parentRotationAxisMask)|=kParentAxisY;
            if(zEnabled) (*(int*)&((CFxSpray*)(*(void**)&((CFxSequence*)self)->currentSpray))->parentRotationAxisMask)|=kParentAxisZ;
        }
    } else if (!CString_Compare(key,(char*)fxSequenceParserStrings.sound)) {
        // Sound coordinates are camera-relative; negative range selects non-positional playback.
        float value=kFloat_neg1_FxBox;
        strcpy(scratch,args[0]);
        char* cut=strstr(scratch,(char*)fxSequenceParserStrings.waveExtension);
        if(cut) *cut=0;
        sscanf(args[1],(char*)fxSequenceParserStrings.floatFormat,&value);
        if(value<kFloat_0_SoundEmitter) Sound_PlayByName(g_soundManager,scratch,0,0,0,0,kFloat_0_5_FxSequence,kFloat_200_FxSequence,kFloat_1_SoundEmitter);
        else {
            Vec190 v;
            __mi__FRC7WVectorRC7WVector(&v,&((CFxSequence*)self)->worldPosition,&((CameraTransformView*)s_cameraTransform)->position);
            Sound_PlayByName(g_soundManager,scratch,&v,5,0,0,kFloat_0_SoundEmitter,value,kFloat_1_SoundEmitter);
        }
    } else if (!CString_Compare(key,(char*)fxSequenceParserStrings.jumpToLabel)) {
        // Jump searches command text in execution order; near-time jumps request a retry.
        Node* end=(Node*)((char*)&((CFxSequence*)self)->firstCommandNode - sizeof(void*));
        for(Node* n=(*(Node**)&((CFxSequence*)self)->firstCommandNode);n!=end;n=n->prev) {
            if(!CString_Compare((char*)n->commandText,args[0])) {
                CFxSequence_RandomizeAndSortCommandTimes((State*)self);
                Node* next=n->prev;
                (*(Node**)&((CFxSequence*)self)->nextCommand)=next;
                float time=(float)next->earliestFrame;
                float delta=((CFxSequence*)self)->elapsedTime-time;
                float scale=((EffectBoxTimeScale*)((CFxSequence*)self)->sequenceOwner)->timeScale;
                if(delta<kFloat_2_FxSequence*scale && delta>kFloat_neg2_FxSequence*scale) {
                    return 0;
                }
                ((CFxSequence*)self)->elapsedTime=time;
                break;
            }
        }
    } else if (!CString_Compare(key,(char*)fxSequenceParserStrings.stop)) {
        ((CFxSequence*)self)->nextCommand=(char*)&((CFxSequence*)self)->firstCommandNode - sizeof(void*);
    } else {
        *result=1; return 0;
    }
    return ok;
}

#pragma opt_common_subs off
#pragma use_lmw_stmw on
// CodeWarrior string ABI: the high flag bit selects heap length/data instead of inline bytes.
inline unsigned int lengthDCF(const SequenceNameStringRep* s) {
    unsigned int result;
    if ((s->flag >> 31) == 0) result = *(const unsigned char*)s & 0x7f;
    else result = s->heapLength;
    return result;
}
// Return the smaller length by reference for the original string-copy expression.
inline const unsigned int& minDCF(const unsigned int& a, const unsigned int& b) {
    return a < b ? a : b;
}
struct EmptyDCF { unsigned char byte; };
// Copy the reconstructed small/heap string representation into the destination.
inline void copyDCF(SequenceNameStringRep* dest, const SequenceNameStringRep& arg) {
        if (!(arg.flag >> 31)) {
            unsigned int f = arg.flag, b = arg.heapLength, e = arg.heapDataAddress;
            dest->flag = f; dest->heapLength = b; dest->heapDataAddress = e;
        }
        else {
            dest->flag = 0; dest->heapLength = 0; dest->heapDataAddress = 0;
            String_ReserveCapacity(dest, arg.heapLength);
            EmptyDCF allocator;
            EmptyDCF copy = allocator;
            String_ReplaceRange(dest, 0, 0, arg.heapDataAddress, arg.heapDataAddress + arg.heapLength, (unsigned char*)&copy);
        }
}
struct StringDCF : SequenceNameStringRep {
    // Copy the tree's string key, including any owned heap text.
    StringDCF(const SequenceNameStringRep& arg) { copyDCF(this,arg); }
    // Release only heap-form key storage.
    ~StringDCF() {
        if (flag >> 31) appFreeUni__FPv((void*)heapDataAddress);
    }
};
struct PairDCF {
    StringDCF range;
    unsigned int value;
    // Create a copied key paired with the registry's default mapped word.
    PairDCF(const SequenceNameStringRep& arg) : range(arg), value(s_fxSequenceNameDefaultValue) {}
};
// Find a sequence name in the ordered string map, inserting the default mapped value on a miss.
void* FxSequenceNameTree_GetOrInsertValue(void* self, SequenceNameStringRep* arg) {
    unsigned char insertBefore, updateTail;
    void* foundBase;
    void* insertionParent = StringTree_FindInsertionPosition(self, arg, &foundBase, &insertBefore, &updateTail);
    SequenceNameStringRep* found;
    // 0x8010DCF0: finish lookup or insertion before returning the value.
    do {
        if (foundBase != 0) {
            found = &((NameTreeNode*)foundBase)->key;
            // Compare borrowed bytes over the common prefix, then compare lengths.
            unsigned int requestedLength;
            unsigned int requestedBytes;
            if ((arg->flag >> 31) == 0) { requestedLength = *(unsigned char*)arg & 0x7f; requestedBytes = (unsigned int)((char*)arg + 1); }
            else { requestedBytes = arg->heapDataAddress; requestedLength = arg->heapLength; }
            unsigned int candidateLengthFirstRead;
            unsigned int requestedLengthCopy = requestedLength;
            candidateLengthFirstRead = lengthDCF(found);
            unsigned int candidateLengthSecondRead, candidateBytes;
            if (!(found->flag >> 31)) { candidateBytes = (unsigned int)found + 1; candidateLengthSecondRead = *(unsigned char*)found & 0x7f; }
            else { candidateBytes = found->heapDataAddress; candidateLengthSecondRead = found->heapLength; }
            unsigned int candidateLengthCopy = candidateLengthSecondRead;
            unsigned int candidateLength = minDCF(candidateLengthFirstRead,candidateLengthCopy);
            int prefixComparison = memcmp((void*)candidateBytes, (void*)requestedBytes, minDCF(requestedLengthCopy,candidateLength));
            int keyComparison;
            if (prefixComparison == 0) {
                if (candidateLength < requestedLength) keyComparison = -1;
                else if (requestedLength == candidateLength) keyComparison = 0;
                else keyComparison = 1;
            } else {
                keyComparison = prefixComparison;
            }
            if (keyComparison >= 0) {
                break;
            }
        }
        {
            PairDCF pair(*arg);
            void* insertedNode = NamedObjectMap_InsertNode(self, insertionParent, insertBefore, updateTail, &pair);
            found = &((NameTreeNode*)insertedNode)->key;
        }
    } while (0);
    return found + 1;
}
#pragma opt_common_subs reset
#pragma use_lmw_stmw reset

struct PayloadF028 {
    struct Storage { unsigned int words[0x45]; } data;
    // Copy a command payload while preserving the original exception scope.
    PayloadF028(const PayloadF028& other) try : data(other.data) {} catch (...) { throw; }
};
struct AllocF028 {
    unsigned char allocator;
    Node* result;
    Node* allocated;
    unsigned int reserved;
    Node* head;
    Node* cleanup;
    // Allocate and guard the replacement node used during timeline sorting.
    AllocF028(Node* h): allocator(0) {
        allocated = (Node*)appMallocDummy__FUl(0x11c);
        result = allocated;
        head = h;
        cleanup = allocated;
    }
    // Free a replacement node if its insertion did not commit.
    ~AllocF028() { if(cleanup) appFreeUni__FPv(cleanup); }
};
// Unlink and free one command node; its inline payload has no separate allocation.
inline void eraseF028(Node* node) {
    node->next->prev = node->prev;
    node->prev->next = node->next;
    appFreeUni__FPv(node);
}
#pragma cpp_exceptions on
#pragma opt_common_subs on
#pragma use_lmw_stmw on
// Choose times in [earliestFrame, latestFrame) and sort timed commands within label/immediate boundaries.
void CFxSequence_RandomizeAndSortCommandTimes(State* s) {
    // Each insertion copies a payload, links the replacement, then erases the old
    // node. Count is unchanged after that pair; labels/immediate nodes are barriers.
    // Unequal endpoints require latestFrame > earliestFrame for a meaningful range.
    Node* end = (Node*)&s->sentinel;
    Node* scan;
    Node* cur = s->sentinel.prev;
    while (cur != end) {
        unsigned int v = cur->value;
        if ((v + kCommandSentinelBias) != kLabelBiasedTime && (v + kCommandSentinelBias) != kImmediateBiasedTime) {
            // Phase 1: choose a time without including the upper endpoint.
            unsigned int lo = cur->earliestFrame;
            if (cur->latestFrame != lo) {
                unsigned int n = rand();
                cur->value = lo + n % (cur->latestFrame - lo);
            }
            // Phase 2: locate this segment's barrier, then find the insertion point.
            Node* head = s->sentinel.prev;
            scan = cur;
            Node* other = cur;
            while (scan != head) {
                other = other->next;
                unsigned int ov = other->value;
                if ((ov + kCommandSentinelBias) == kLabelBiasedTime || (ov + kCommandSentinelBias) == kImmediateBiasedTime) break;
                scan = scan->next;
            }
            while (scan != cur) {
                if (scan->value > cur->value) {
                    {
                    AllocF028 guard((Node*)&s->sentinel);
                    Node* node = guard.allocated;
                    PayloadF028* data = (PayloadF028*)((char*)node + 8);
                    new(data) PayloadF028(*(PayloadF028*)((char*)cur + 8));
                    guard.cleanup = 0;
                    scan->next->prev = node;
                    node->next = scan->next;
                    scan->next = node;
                    node->prev = scan;
                    ++s->count;
                    }
                    scan = cur->prev;
                    eraseF028(cur);
                    --s->count;
                    cur = scan->next;
                    break;
                }
                scan = scan->prev;
            }
        }
        cur = cur->prev;
    }
}
#pragma opt_common_subs reset
#pragma use_lmw_stmw reset

#pragma cpp_exceptions reset
// Wait for playback/resume, execute due commands, retry blocked commands, and retain failed-command details.
int CFxSequence_UpdateTimeline(State* s, float dt, unsigned char* flag) {
    // dt, elapsed and the resume timer share caller-supplied timeline units;
    // scheduled integer times are compared to elapsed rounded via +0.5.
    // Return 0 when nextCommand reaches the sentinel, 1 while waiting/retrying.
    // flag is required. Commands run before this call adds dt to elapsed.
    // Phase 1: wait for playback or a strictly negative resume-timer crossing.
    *flag = 0;
    if (!s->playbackEnabled) {
        float tm = s->timer;
        float zero = kFloat_0_SoundEmitter;
        if (tm > zero) {
            float t = tm - dt;
            s->timer = t;
            if (t < zero) s->playbackEnabled = 1;
            else return 1;
        } else return 1;
    }
    // Phase 2: skip labels and execute every currently due/immediate command.
    float half = kFloat_0_5_FxSequence;
    Node* end = (Node*)&s->sentinel;
    for (;;) {
        Node* n = (*(Node**)&((CFxSequence*)s)->nextCommand);
        if (n == end) {
            s->elapsed += dt;
            return 0;
        }
        unsigned int value = n->value;
        if ((value + kCommandSentinelBias) == kLabelBiasedTime) {
            (*(Node**)&((CFxSequence*)s)->nextCommand) = n->prev;
            continue;
        }
        // 0x8010F1DC: commands whose time has arrived are processed before advancing.
        if (!((value + kCommandSentinelBias) != kImmediateBiasedTime && value > Runtime_ConvertDoubleToUnsigned(half + s->elapsed))) {
            (*(Node**)&((CFxSequence*)s)->nextCommand) = n->prev;
            if ((signed char)n->commandText[0] == 0) continue;
            if (!CFxSequence_ExecuteScriptCommand(s, (signed char*)n->commandText, flag)) {
                // Phase 3a: back up from the command-selected cursor for retry.
                Node* cur = (*(Node**)&((CFxSequence*)s)->nextCommand);
                (*(Node**)&((CFxSequence*)s)->nextCommand) = cur->next;
            } else {
                if (*flag == 0) continue;
                // Phase 3b: retain failed-command metadata and its 0x104-byte text.
                int i = 0x82;
                unsigned char* src = n->commandText - 1;
                unsigned char* dst = s->failedCommandPrefix - 1;
                s->failedCommandMetadata[0] = n->sourceLine;
                s->failedCommandMetadata[1] = n->earliestFrame;
                s->failedCommandMetadata[2] = n->latestFrame;
                s->failedCommandMetadata[3] = n->value;
                unsigned char hi, lo;
                do {
                    lo = src[1];
                    src += 2;
                    hi = *src;
                    dst[1] = lo;
                    dst += 2;
                    *dst = hi;
                } while (--i);
            }
        }
        s->elapsed += dt;
        return 1;
    }
}


#undef APPEND_SPRAY_TRACK
} // extern "C"
