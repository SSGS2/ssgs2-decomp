// B2D script files: registering a file into a script slot, finding and running its named
// script labels, and building the opcode handler index for each slot.
// Also the input key lists that scripts use (call labels and user data), the fixed-point
// sine and cosine tables, and small sprite setters.

#include <decomp/static_init.h>
extern "C" {
extern unsigned char b2dFileLabelHookNames[];
}
#include "sdk_protos.h"
#include "classes/BpSpriteDrawView.h"

extern "C" {

// B2D commands have a three-word minimum header followed by additional argument words.
// Offsets are bytes from file base (stringsOffset + firstArgument for label names).
// argumentWords includes firstArgument; even zero arguments occupy the 12-byte header.
// Registered file memory and all returned label pointers are borrowed and must outlive use.
struct B2dFileHeader {
    int signature;
    int commandsOffset;
    int stringsOffset;
    int commandCount;
};
struct B2dCommand {
    int opcode;
    int argumentWords;
    int firstArgument;
};
// The descriptor's third word is 0x00050000 (scratch 5, draw objects 0); on the Wii the first short is the high
// half of the word.
struct B2dCommandHandler {
    int opcode;
    int (*execute)(int, void*);
    short scratchUnits16Bytes;
    short drawObjectCount;
};
struct B2dScriptSlot {
    int handle;
    void* file;
    void* scripts[0x80];
    int currentScript;
    int executionState;
    char labelControl[0x12];
    char alignment[2];
};
struct B2dInputKeyList {
    int handle;
    int state;
    void* userData;
    int callLabels[0x12];
};
typedef char B2dCommandHandlerSizeCheck[sizeof(B2dCommandHandler) == 12 ? 1 : -1];
struct BpSpriteState {
    short objectKind;
    short flags;
};
// Handler scratchUnits16Bytes counts 16-byte work units, not words or bytes.
enum B2dLimits {
    kB2dSlotCount = 40,
    kB2dScriptsPerSlot = 128,
    kB2dCallLabelCount = 18,
    kB2dHandlerCount = 38,
    kB2dSlotBytes = sizeof(B2dScriptSlot),
    kB2dKeyListBytes = sizeof(B2dInputKeyList),
    kB2dSlotArrayBytes = kB2dSlotCount * kB2dSlotBytes,
    kB2dCommandHeaderBytes = sizeof(B2dCommand),
    kB2dWordBytes = sizeof(int),
    kB2dScriptsOffset = (int)&((B2dScriptSlot*)0)->scripts,
    kB2dDrawObjectBytes = 0x1C8,
    kB2dScratchUnitBytes = 16,
    kB2dControlBytesOffset = (int)&((B2dScriptSlot*)0)->labelControl
};
enum B2dOpcode {
    kB2dScriptLabel = 1,
    kB2dEndLine = 2,
    kB2dFirstCallLabel = 0x13,
    kB2dLastCallLabel = 0x24
};
enum B2dControlFlags {
    kB2dLabelActive = 1,
    kB2dControlledExecution = 6,
    kB2dTransientControl = 14
};
enum BpSpriteFlags {
    kBpSpriteDisabled = 0x10
};
enum FixedPointTrig {
    kTrigTableCount = 1024,
    kTrigIndexMask = kTrigTableCount - 1,
    kTrigFullTurnDegrees = 360,
    kTrigFixedScale = 4096
};
// Explicit byte sizes guard private views used by the opaque C interfaces below.
typedef char B2dSlotSizeCheck[sizeof(B2dScriptSlot) == 0x224 ? 1 : -1];
typedef char B2dKeyListSizeCheck[sizeof(B2dInputKeyList) == 0x54 ? 1 : -1];

extern float kFloat_6_2831855;
extern float s_b2dFileTwoPi;
extern float s_initializedTwoPi_800C0700;
extern float s_bpSpriteTwoPi;
extern double kDouble_6_283185307_FixedPointTrig;
extern double kDouble_0_0009765625_FixedPointTrig;
extern double kDouble_4096_FixedPointTrig;

extern char b2dSpriteSourceFileName[];
extern char bpSpriteSetDrawFlagFunctionName[];
extern char bpSpriteSetAlphaFunctionName[];
extern char kBpInputSourceFilename[];
extern char b2dInputResetKeyListFunctionName[];
extern char b2dInpGetInfoPtrFunctionName[];
extern char b2dInputSetCallLabelFunctionName[];
extern int s_b2dCommandHandlerDescriptors[];
extern void* s_fixedPointSinTable[];
extern int s_fixedPointCosTable[];
extern void* s_b2dCommandHandlers[];
extern void* s_b2dInputKeyLists[];
extern int g_bpCommandSlots;
extern void* g_b2dInputCallback;
// Additional reset-only callback state words; no read-side semantics are established.
extern int s_b2dInputCallbackStateWord;
extern int s_b2dInputCallbackStateStorage;
extern char b2dSourceFileName[];
extern char b2dFlagControlAllDiagnostic[];
extern char b2dFileRegisterFunctionName[];
extern char b2dGetLabelDataFunctionName[];
extern char b2dGetCommandListFunctionName[];
extern char b2dFileResetFunctionName[];

int BpCtrl_InvokeGlobalHook(int, void*, int, void*);
void* memset(void*, int, unsigned int);
void* __nwa__FUl(unsigned int);
int B2dFile_Reset(int);
int B2dFile_ExecuteNamedScript(int, void*);
int B2dInp_SetCallLabel(int, int, void*);
int B2dInp_ResetKeyList(int);

#pragma push
#pragma use_lmw_stmw on
// Byte view across adjacent diagnostic strings, preserving the original symbol plus offset relocations.
struct ScriptDiagnosticLayout {
    char flagControlAll[0x10];  // _B2dFlgCtlAll
    char getLabelPointer[0x5C]; // B2dFileGetLabelPtr, followed by intervening diagnostics
    char getCommandList[0x14];  // pB2dFileGetCmdList
    char getDataPointer[0x10];  // B2dFileGetDatPtr
};
struct ScriptMessages {
    char text[0x100];
};
enum ScriptDiagnosticOffsets {
    kDiagnosticGetLabelPointer = (int)&((ScriptDiagnosticLayout*)0)->getLabelPointer,
    kDiagnosticGetCommandList = (int)&((ScriptDiagnosticLayout*)0)->getCommandList,
    kDiagnosticGetDataPointer = (int)&((ScriptDiagnosticLayout*)0)->getDataPointer
};
// Find a named script label by resolving each command's name offset through the registered file.
static inline int* FindScript(int index, void* name, ScriptMessages* msg) {
    if (index < 0 || index >= kB2dSlotCount) {
        BpCtrl_InvokeGlobalHook(-1, b2dSourceFileName, 0x3E8, ((char*)(msg->text + kDiagnosticGetDataPointer)));
        return 0;
    } else {
        int stride = index * kB2dSlotBytes;
        int i = 0;
        int offset = 0;
        for (; i < kB2dScriptsPerSlot; ++i, offset += 4) {
            if (g_bpCommandSlots == 0)
                g_bpCommandSlots = (int)__nwa__FUl(kB2dSlotArrayBytes);
            // The byte iterator selects scripts[0] of a shifted layout view.
            char* slot = (char*)g_bpCommandSlots + (offset + stride);
            int* entry = (int*)((B2dScriptSlot*)slot)->scripts[0];
            if (entry == 0)
                return 0;
            int labelNameOffset = ((B2dCommand*)entry)->firstArgument;
            void* data;
            if (index < 0 || index >= kB2dSlotCount) {
                BpCtrl_InvokeGlobalHook(-1, b2dSourceFileName, 0x90, ((char*)(msg->text + kDiagnosticGetLabelPointer)));
                data = 0;
            } else {
                if (g_bpCommandSlots == 0)
                    g_bpCommandSlots = (int)__nwa__FUl(kB2dSlotArrayBytes);
                char* f = (char*)((B2dScriptSlot*)(stride + g_bpCommandSlots))->file;
                if (f == 0) {
                    BpCtrl_InvokeGlobalHook(-2, b2dSourceFileName, 0x98,
                                            ((char*)(msg->text + kDiagnosticGetLabelPointer)));
                    data = 0;
                } else {
                    data = (void*)(((B2dFileHeader*)f)->stringsOffset + (labelNameOffset + (int)f));
                }
            }
            if (!strcmp((const char*)data, (const char*)name))
                return entry;
        }
    }
    return 0;
}
// Clear active labels, resolve a script name and dispatch commands until a handler stops execution.
// Return -1 when the label is absent, otherwise 0 even when a handler returns an error.
// Files must contain valid opcodes: the original negative-and-too-large guard is ineffective.
int B2dFile_ExecuteNamedScript(int index, void* name) {
    ScriptMessages* msg;
    msg = (ScriptMessages*)b2dFlagControlAllDiagnostic;
    int* words = FindScript(index, name, msg);
    if (words == 0)
        return -1;
    if (index < 0 || index >= kB2dSlotCount) {
        BpCtrl_InvokeGlobalHook(-1, b2dSourceFileName, 0x62, msg->text);
    } else {
        if (g_bpCommandSlots == 0)
            g_bpCommandSlots = (int)__nwa__FUl(kB2dSlotArrayBytes);
        char* p = (char*)g_bpCommandSlots + index * kB2dSlotBytes;
        for (int i = 0; i < kB2dCallLabelCount; ++i)
            ((B2dScriptSlot*)p)->labelControl[i] &= ~kB2dLabelActive;
    }
    // Phase 2: reset input labels, then consume inline callback labels or dispatch handlers.
    B2dInp_ResetKeyList(index);
    int result;
    for (;;) {
        int type = words[0];
        if (type >= kB2dFirstCallLabel && type <= kB2dLastCallLabel) {
            int label;
            int dataOffset;
            dataOffset = words[2];
            label = type - kB2dFirstCallLabel;
            void* data;
            if (index < 0 || index >= kB2dSlotCount) {
                BpCtrl_InvokeGlobalHook(-1, b2dSourceFileName, 0x90, ((char*)(msg->text + kDiagnosticGetLabelPointer)));
                data = 0;
            } else {
                if (g_bpCommandSlots == 0)
                    g_bpCommandSlots = (int)__nwa__FUl(kB2dSlotArrayBytes);
                char* f = *(char**)(index * kB2dSlotBytes + g_bpCommandSlots + 4);
                if (f == 0) {
                    BpCtrl_InvokeGlobalHook(-2, b2dSourceFileName, 0x98,
                                            ((char*)(msg->text + kDiagnosticGetLabelPointer)));
                    data = 0;
                } else {
                    data = (void*)(((B2dFileHeader*)f)->stringsOffset + (dataOffset + (int)f));
                }
            }
            B2dInp_SetCallLabel(index, label, data);
            result = 0;
        } else {
            void* item;
            if (type < 0 && type >= kB2dHandlerCount) {
                BpCtrl_InvokeGlobalHook(-1, b2dSourceFileName, 0x38C, ((char*)(msg->text + kDiagnosticGetCommandList)));
                item = 0;
            } else {
                item = s_b2dCommandHandlers[type];
            }
            if (item == 0)
                break;
            result = ((int (*)(int, void*))((B2dCommandHandler*)item)->execute)(index, (void*)words);
        }
        if (result < 0 || result == 1)
            break;
        int n = words[1];
        words = (int*)((char*)words + kB2dCommandHeaderBytes + (n < 1 ? 0 : n - 1) * 4);
    }
    return 0;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Run a named script with temporary label-control bits, clearing those bits after execution.
// Same return convention as ExecuteNamedScript. Bits 1/2 are set before lookup;
// bits 1/2/3 are cleared afterward even when lookup fails. Bit 0 tracks label activation.
int B2dFile_ExecuteNamedScriptWithControlFlags(int index, void* name) {
    char* msg = b2dFlagControlAllDiagnostic;
    char* entry;
    if (index < 0 || index >= kB2dSlotCount) {
        BpCtrl_InvokeGlobalHook(-1, b2dSourceFileName, 0x62, msg);
    } else {
        if (0 == g_bpCommandSlots)
            g_bpCommandSlots = (int)__nwa__FUl(kB2dSlotArrayBytes);
        char* p = (char*)g_bpCommandSlots + index * kB2dSlotBytes;
        for (int i = 0; i < kB2dCallLabelCount; ++i)
            ((B2dScriptSlot*)p)->labelControl[i] |= kB2dControlledExecution;
    }
    // Resolve while control bits are installed. Exhaustion clears entry in the
    // loop condition; a successful strcmp exits with the borrowed command intact.
    if (index < 0 || index >= kB2dSlotCount) {
        BpCtrl_InvokeGlobalHook(-1, b2dSourceFileName, 0x3E8, ((char*)(msg + kDiagnosticGetDataPointer)));
        entry = 0;
    } else {
        int stride = index * kB2dSlotBytes;
        int i = 0;
        int offset = 0;
        for (; i < kB2dScriptsPerSlot || (entry = 0, false); ++i, offset += 4) {
            if (g_bpCommandSlots == 0)
                g_bpCommandSlots = (int)__nwa__FUl(kB2dSlotArrayBytes);
            // The byte iterator selects scripts[0] of a shifted layout view.
            char* slot = (char*)g_bpCommandSlots + stride + offset;
            entry = (char*)((B2dScriptSlot*)slot)->scripts[0];
            if (entry == 0) {
                entry = 0;
                break;
            }
            int dataOffset = ((B2dCommand*)entry)->firstArgument;
            void* data;
            if (index < 0 || index >= kB2dSlotCount) {
                BpCtrl_InvokeGlobalHook(-1, b2dSourceFileName, 0x90, ((char*)(msg + kDiagnosticGetLabelPointer)));
                data = 0;
            } else {
                if (g_bpCommandSlots == 0)
                    g_bpCommandSlots = (int)__nwa__FUl(kB2dSlotArrayBytes);
                char* f = (char*)((B2dScriptSlot*)((char*)g_bpCommandSlots + stride))->file;
                if (f == 0) {
                    BpCtrl_InvokeGlobalHook(-2, b2dSourceFileName, 0x98, ((char*)(msg + kDiagnosticGetLabelPointer)));
                    data = 0;
                } else {
                    data = f + dataOffset + ((B2dFileHeader*)f)->stringsOffset;
                }
            }
            if (!strcmp((const char*)data, (const char*)name))
                break;
        }
    }
    // Phase 2: execute the resolved label with the same callback/handler rules as above.
    int result;
    if (entry == 0) {
        result = -1;
    } else {
        if (index < 0 || index >= kB2dSlotCount) {
            BpCtrl_InvokeGlobalHook(-1, b2dSourceFileName, 0x62, msg);
        } else {
            if (g_bpCommandSlots == 0)
                g_bpCommandSlots = (int)__nwa__FUl(kB2dSlotArrayBytes);
            char* p = (char*)g_bpCommandSlots + index * kB2dSlotBytes;
            for (int i = 0; i < kB2dCallLabelCount; ++i)
                ((B2dScriptSlot*)p)->labelControl[i] &= ~kB2dLabelActive;
        }
        B2dInp_ResetKeyList(index);
        int* words = (int*)entry;
        for (;;) {
            int type = words[0];
            int r;
            if (type >= kB2dFirstCallLabel && type <= kB2dLastCallLabel) {
                int dataOffset = words[2];
                int label = type - kB2dFirstCallLabel;
                void* data;
                if (index < 0 || index >= kB2dSlotCount) {
                    BpCtrl_InvokeGlobalHook(-1, b2dSourceFileName, 0x90, ((char*)(msg + kDiagnosticGetLabelPointer)));
                    data = 0;
                } else {
                    if (g_bpCommandSlots == 0)
                        g_bpCommandSlots = (int)__nwa__FUl(kB2dSlotArrayBytes);
                    char* f = *(char**)((char*)g_bpCommandSlots + index * kB2dSlotBytes + 4);
                    if (f == 0) {
                        BpCtrl_InvokeGlobalHook(-2, b2dSourceFileName, 0x98,
                                                ((char*)(msg + kDiagnosticGetLabelPointer)));
                        data = 0;
                    } else
                        data = f + dataOffset + ((B2dFileHeader*)f)->stringsOffset;
                }
                B2dInp_SetCallLabel(index, label, data);
                r = 0;
            } else {
                void* item;
                if (type < 0 && type >= kB2dHandlerCount) {
                    BpCtrl_InvokeGlobalHook(-1, b2dSourceFileName, 0x38C, ((char*)(msg + kDiagnosticGetCommandList)));
                    item = 0;
                } else {
                    item = s_b2dCommandHandlers[type];
                }
                if (item == 0)
                    break;
                r = ((int (*)(int, void*))((B2dCommandHandler*)item)->execute)(index, words);
            }
            if (r < 0 || r == 1)
                break;
            int n = words[1];
            words = (int*)((char*)words + kB2dCommandHeaderBytes + (n > 1 ? (n - 1) * 4 : 0));
        }
        result = 0;
    }
    // Cleanup is unconditional, including failed name lookup and stopped handlers.
    if (index < 0 || index >= kB2dSlotCount) {
        BpCtrl_InvokeGlobalHook(-1, b2dSourceFileName, 0x62, msg);
    } else {
        if (g_bpCommandSlots == 0)
            g_bpCommandSlots = (int)__nwa__FUl(kB2dSlotArrayBytes);
        char* p = (char*)g_bpCommandSlots + index * kB2dSlotBytes;
        for (int i = 0; i < kB2dCallLabelCount; ++i)
            ((B2dScriptSlot*)p)->labelControl[i] &= ~kB2dTransientControl;
    }
    // Phase 3 complete: temporary control bits have been cleared for every call label.
    return result;
}
#pragma pop

// Invalidate a script slot and clear its script pointers and control bytes.
int B2dFile_Reset(int index) {
    if (index < 0 || index >= kB2dSlotCount) {
        BpCtrl_InvokeGlobalHook(-1, b2dSourceFileName, 0x4B0, b2dFileResetFunctionName);
        return -1;
    }
    if (g_bpCommandSlots == 0)
        g_bpCommandSlots = (int)__nwa__FUl(kB2dSlotArrayBytes);
    int off = index * kB2dSlotBytes;
    char* p = (char*)g_bpCommandSlots + off;
    ((B2dScriptSlot*)p)->handle = -1;
    ((B2dScriptSlot*)p)->file = 0;
    ((B2dScriptSlot*)p)->currentScript = -1;
    int* slotWord = (int*)p;
    for (int i = 0; i < kB2dScriptsPerSlot; ++i) {
        slotWord[kB2dScriptsOffset / sizeof(int)] = 0;
        slotWord++;
    }
    for (int i = kB2dControlBytesOffset; i < kB2dSlotBytes; ++i)
        p[i] = 0;
    return 0;
}

#pragma push
#pragma use_lmw_stmw on
// Rotate integer x/y pairs around a pivot using 1024-entry sine/cosine tables scaled by 4096.
// points holds count writable pairs in the pivot's coordinate space. Degrees are quantized
// to a table index; integer division truncates each rotated coordinate toward zero.
void Math_RotateIntegerPoints2D(int* points, int count, int pivotX, int pivotY, int angleDegrees) {
    if (angleDegrees != 0) {
        int index = (angleDegrees * kTrigTableCount) / kTrigFullTurnDegrees;
        index = index & kTrigIndexMask;
        int sinValue = ((int*)s_fixedPointSinTable)[index];
        int cosValue = s_fixedPointCosTable[index];
        int* point = points;
        for (int i = 0; i < count; ++i) {
            int dx = point[0] - pivotX;
            int dy = point[1] - pivotY;
            point[0] = pivotX + (dx * cosValue - dy * sinValue) / kTrigFixedScale;
            point[1] = pivotY + (dx * sinValue + dy * cosValue) / kTrigFixedScale;
            point += 2;
        }
    }
}
#pragma pop

// Reset every script slot and build the opcode-to-handler index from the descriptor table.
int B2dFile_InitializeSlotsAndIndex() {
    for (int i = 0; i < kB2dSlotCount; i++)
        B2dFile_Reset(i);
    memset(s_b2dCommandHandlers, 0, kB2dHandlerCount * sizeof(void*));
    B2dCommandHandler** dst = (B2dCommandHandler**)s_b2dCommandHandlers;
    B2dCommandHandler* src = (B2dCommandHandler*)s_b2dCommandHandlerDescriptors;
    B2dCommandHandler* descriptor = src;
    int index;
    while ((index = descriptor->opcode) >= 0) {
        ++descriptor;
        dst[index] = src;
        ++src;
    }
    return 0;
}
void* __nwa__FUl(unsigned int);

// Collect opcode-1 script labels from variable-length commands, checking the output capacity.
// Null output performs a count-only pass. Otherwise return count or -1 after capacity is
// exhausted; earlier output entries remain written and point directly into borrowed file data.
int B2dFile_CollectLabelData(void* object, void** output, int outputCapacity) {
    void** outputCursor = output;
    char* entry = (char*)object + ((B2dFileHeader*)object)->commandsOffset;
    int commandIndex = 0;
    int labelCount = 0;

    for (; commandIndex < ((B2dFileHeader*)object)->commandCount; commandIndex++) {
        if (((B2dCommand*)entry)->opcode == kB2dScriptLabel) {
            if (output) {
                if (outputCapacity <= labelCount) {
                    BpCtrl_InvokeGlobalHook(-1, b2dSourceFileName, 0x494, b2dGetLabelDataFunctionName);
                    return -1;
                }
                *outputCursor = entry;
                outputCursor = 1 + outputCursor;
            }
            labelCount = (labelCount + 1);
        }
        int argumentWords = ((B2dCommand*)entry)->argumentWords;
        entry = entry + kB2dCommandHeaderBytes + ((1 < argumentWords) ? (argumentWords - 1) * 4 : 0);
    }
    return labelCount;
}

// Reset a slot, register its B2D file and index up to 128 script labels.
// Return -1 for a bad slot, 0 otherwise. The label collector's overflow result is ignored;
// callers must provide a valid file with no more than kB2dScriptsPerSlot labels.
int B2dFile_Register(int index, void* fileData) {
    if (index < 0 || index >= kB2dSlotCount) {
        BpCtrl_InvokeGlobalHook(-1, b2dSourceFileName, 0x4D5, b2dFileRegisterFunctionName);
        return -1;
    }
    B2dFile_Reset(index);
    if (g_bpCommandSlots == 0)
        g_bpCommandSlots = (int)__nwa__FUl(kB2dSlotArrayBytes);
    char* entry = (char*)g_bpCommandSlots + index * kB2dSlotBytes;
    ((B2dScriptSlot*)entry)->handle = index;
    ((B2dScriptSlot*)entry)->file = fileData;
    B2dFile_CollectLabelData(fileData, ((B2dScriptSlot*)entry)->scripts, kB2dScriptsPerSlot);
    return 0;
}

#pragma use_lmw_stmw on
// Clear every input list's state, user data and call labels, then reset callback state.
void B2dInpResetKeyList() {
    int zero = 0;
    B2dInputKeyList* p = (B2dInputKeyList*)s_b2dInputKeyLists;
    for (int i = 0; i < kB2dSlotCount; i++) {
        p->state = zero;
        p->userData = (void*)zero;
        if (i < 0 || i >= kB2dSlotCount)
            BpCtrl_InvokeGlobalHook(-1, kBpInputSourceFilename, 0x82, b2dInputResetKeyListFunctionName);
        else
            for (int j = 0; j < kB2dCallLabelCount; j++)
                p->callLabels[j] = zero;
        ++p;
    }
    g_b2dInputCallback = 0;
    s_b2dInputCallbackStateWord = 0;
    s_b2dInputCallbackStateStorage = 0;
}
#pragma use_lmw_stmw reset

#pragma opt_propagation off
// Clear one input list's 18 call-label words and return its record address.
int B2dInp_ResetKeyList(int index) {
    if (index < 0 || index >= kB2dSlotCount)
        return BpCtrl_InvokeGlobalHook(-1, kBpInputSourceFilename, 0x82, b2dInputResetKeyListFunctionName);
    char* p = (char*)s_b2dInputKeyLists + index * kB2dKeyListBytes;
    int zero = 0;
    ((B2dInputKeyList*)p)->callLabels[0] = zero;
    ((B2dInputKeyList*)p)->callLabels[1] = zero;
    ((B2dInputKeyList*)p)->callLabels[2] = zero;
    ((B2dInputKeyList*)p)->callLabels[3] = zero;
    ((B2dInputKeyList*)p)->callLabels[4] = zero;
    ((B2dInputKeyList*)p)->callLabels[5] = zero;
    ((B2dInputKeyList*)p)->callLabels[6] = zero;
    ((B2dInputKeyList*)p)->callLabels[7] = zero;
    ((B2dInputKeyList*)p)->callLabels[8] = zero;
    ((B2dInputKeyList*)p)->callLabels[9] = zero;
    ((B2dInputKeyList*)p)->callLabels[10] = zero;
    ((B2dInputKeyList*)p)->callLabels[11] = zero;
    ((B2dInputKeyList*)p)->callLabels[12] = zero;
    ((B2dInputKeyList*)p)->callLabels[13] = zero;
    ((B2dInputKeyList*)p)->callLabels[14] = zero;
    ((B2dInputKeyList*)p)->callLabels[15] = zero;
    ((B2dInputKeyList*)p)->callLabels[16] = zero;
    ((B2dInputKeyList*)p)->callLabels[17] = zero;
    return (int)p;
}
#pragma opt_propagation reset

// Set user data on a valid registered input key list.
void B2dInp_SetUserData(int index, void* userData) {
    void* keyList;
    if (index < 0 || index >= kB2dSlotCount) {
        BpCtrl_InvokeGlobalHook(index, kBpInputSourceFilename, 0x3F, b2dInpGetInfoPtrFunctionName);
        keyList = 0;
    } else {
        void* entry = (char*)s_b2dInputKeyLists + index * kB2dKeyListBytes;
        keyList = entry;
        if (((B2dInputKeyList*)entry)->handle < 0) {
            BpCtrl_InvokeGlobalHook(((B2dInputKeyList*)entry)->handle, kBpInputSourceFilename, 0x43,
                                    b2dInpGetInfoPtrFunctionName);
            keyList = 0;
        }
    }
    if (keyList)
        ((B2dInputKeyList*)keyList)->userData = userData;
}

// Store the callback label value after validating the key-list handle and label index.
// labelValue is borrowed; return 0 on write, -1 for an invalid label index, -2 for no list.
int B2dInp_SetCallLabel(int index, int labelIndex, void* labelValue) {
    int* entry;
    if (index < 0 || index >= kB2dSlotCount) {
        BpCtrl_InvokeGlobalHook(index, kBpInputSourceFilename, 0x3F, b2dInpGetInfoPtrFunctionName);
        entry = 0;
    } else {
        entry = (int*)((char*)s_b2dInputKeyLists + index * kB2dKeyListBytes);
        int handle = entry[0];
        if (handle < 0) {
            BpCtrl_InvokeGlobalHook(handle, kBpInputSourceFilename, 0x43, b2dInpGetInfoPtrFunctionName);
            entry = 0;
        }
    }
    int* labels;
    if (entry == 0)
        labels = 0;
    else
        labels = entry + 3;
    if (labels == 0)
        return -2;
    if (labelIndex < 0 || labelIndex >= kB2dCallLabelCount) {
        BpCtrl_InvokeGlobalHook(-1, kBpInputSourceFilename, 0x70, b2dInputSetCallLabelFunctionName);
        return -1;
    }
    labels[labelIndex] = (int)labelValue;
    return 0;
}

// Fill the 1024-entry sine/cosine tables with signed fixed-point values scaled by 4096.
void Math_InitFixedPointTrigTables() {
    int* sinTable;
    int* cosTable;
    cosTable = s_fixedPointCosTable;
    sinTable = (int*)s_fixedPointSinTable;
    double scale = kDouble_4096_FixedPointTrig;
    double step = kDouble_6_283185307_FixedPointTrig * kDouble_0_0009765625_FixedPointTrig;
    for (int i = 0; i < kTrigTableCount; i++) {
        double angle = step * (double)i;
        *sinTable++ = (int)(scale * sin(angle));
        *cosTable++ = (int)(scale * cos(angle));
    }
}

#pragma push
#pragma use_lmw_stmw on
// Sum handler draw objects and maximum per-line scratch demand; optionally return label counts.
// Return total allocation bytes or -1 for a missing handler. The scratch output counts
// 16-byte units, not 32-bit words. Only opcode-2 boundaries commit a line's scratch maximum;
// a final unterminated line is not included. Outputs are untouched on an early failure.
int B2dFile_CalculateCommandWorkSize(void* object, int* outDrawObjectCount, int* outScratchUnits, int* outLabelCount) {
    int entryIndex = 0;
    int lineScratchUnits = 0;
    int maxLineScratchUnits = 0;
    int drawObjectCount = 0;
    int labelCount = 0;
    char* entry = (char*)object + ((B2dFileHeader*)object)->commandsOffset;
    while (entryIndex < ((B2dFileHeader*)object)->commandCount) {
        int type = ((B2dCommand*)entry)->opcode;
        switch (type) {
        case kB2dScriptLabel:
            labelCount = labelCount + 1;
            break;
        case kB2dEndLine:
            if (maxLineScratchUnits < lineScratchUnits)
                maxLineScratchUnits = lineScratchUnits;
            lineScratchUnits = 0;
            break;
        default:
            if (type >= kB2dFirstCallLabel && type <= kB2dLastCallLabel) {
            } else {
                int* info;
                /* original 0x800BFFA4-0x800BFFCC: one assert, guarded by (type < 0 && type >= kB2dHandlerCount)
                   like the 0x38C assert above, so it never fires and every type reaches the table lookup */
                if (type < 0 && type >= kB2dHandlerCount) {
                    BpCtrl_InvokeGlobalHook(-1, b2dSourceFileName, 0x38C, b2dGetCommandListFunctionName);
                    info = 0;
                } else {
                    info = (int*)s_b2dCommandHandlers[type];
                }
                if (info == 0)
                    return -1;
                lineScratchUnits = lineScratchUnits + ((B2dCommandHandler*)info)->scratchUnits16Bytes;
                drawObjectCount = drawObjectCount + ((B2dCommandHandler*)info)->drawObjectCount;
            }
            break;
        }
        int size = ((B2dCommand*)entry)->argumentWords;
        entryIndex = entryIndex + 1;
        entry = entry + kB2dCommandHeaderBytes + (size > 1 ? size - 1 : 0) * 4;
    }
    if (outScratchUnits != 0)
        *outScratchUnits = maxLineScratchUnits;
    if (outDrawObjectCount != 0)
        *outDrawObjectCount = drawObjectCount;
    if (outLabelCount != 0)
        *outLabelCount = labelCount;
    return drawObjectCount * kB2dDrawObjectBytes + (maxLineScratchUnits << 4);
}
#pragma pop

// Empty module hook retained for linkage.
void B2dFile_EmptyModuleHook() {}
// Initialize this B2D module's two-pi value.
void B2dFile_InitializeTwoPi() {
    s_b2dFileTwoPi = kFloat_6_2831855;
}
// Empty sprite module hook retained for linkage.
void BpSprite_EmptyModuleHook() {}
// Initialize this sprite module's two-pi value.
void BpSprite_InitializeTwoPi() {
    s_bpSpriteTwoPi = kFloat_6_2831855;
}

// Initialize the fixed-point trig module's two-pi value.
void FixedPointTrig_InitializeTwoPi() {
    s_initializedTwoPi_800C0700 = kFloat_6_2831855;
}

// Return one sine-table word with the index wrapped to 1024 samples; ABI exposes the word as void*.
void* Trig_GetSinTableEntry(unsigned int index) {
    return s_fixedPointSinTable[(index & kTrigIndexMask)];
}

// Set a pooled draw object's opacity, reporting a null object through the diagnostic hook.
void BpSprite_SetOpacity(void* object, int value) {
    if (object == 0) {
        BpCtrl_InvokeGlobalHook(-1, b2dSpriteSourceFileName, 0xD6, bpSpriteSetAlphaFunctionName);
        return;
    }
    ((BpSpriteDrawView*)object)->opacity = value; // 0x800C0BEC: pooled draw object's opacity
}

// Enable or disable a sprite by clearing or setting its disabled flag.
int BpSprite_SetEnabled(void* object, int value) {
    if (object == 0) {
        BpCtrl_InvokeGlobalHook(-1, b2dSpriteSourceFileName, 0x99, bpSpriteSetDrawFlagFunctionName);
        return -1;
    }
    switch (value) {
    case 1:
        ((BpSpriteState*)object)->flags &= ~kBpSpriteDisabled;
        break;
    case 0:
        ((BpSpriteState*)object)->flags |= kBpSpriteDisabled;
        break;
    }
    return 0;
}

// Replace the global B2D input callback.
void B2d_SetInputCallback(void* value) {
    g_b2dInputCallback = value;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462094", _ctors_u800BF4E0_0, B2dFile_InitializeTwoPi);
STATIC_INIT_ENTRY(".CRT$XCU80462094", _ctors_u800BF4E0_1, FixedPointTrig_InitializeTwoPi);
STATIC_INIT_ENTRY(".CRT$XCU80462094", _ctors_u800BF4E0_2, BpSprite_InitializeTwoPi);
#pragma pop
