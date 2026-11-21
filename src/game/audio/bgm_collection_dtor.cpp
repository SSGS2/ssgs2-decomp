extern "C" unsigned char s_bpCtrlDtorRegistration_80316848[];
extern "C" unsigned char s_bpCtrlHandleDtorRegistration_BgmCollection[];
extern "C" unsigned char s_inputManagerDtorRegistration_BgmCollection[];
// Translation unit 0x80318428-0x80318714 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
#include "sdk_protos.h"
#include "classes/CBgmCollection.h"

struct Virtual80318428 {
    virtual void destroy(int);
};
struct Prefix80318428 { int padding; };
struct VirtualOffset80318428 : Prefix80318428 {
    virtual void destroy(int);
};
// These adapters express two existing vtable locations: at object +0, or +4
// after Prefix80318428 for trackTable. They do not construct replacement objects.
extern "C" {

extern signed char s_bpCtrlInitialized;
extern signed char s_pointerPairListInitialized;
extern signed char s_bpCtrlHandleInitialized;
struct Handle80318428 { int first; int second; };
extern Handle80318428 s_bpCtrlHandle;
extern void* g_soundManager;
extern unsigned char s_inputManagerDtorRegistration_BgmCollection[];
extern unsigned char __vt__14CBgmCollection[];
extern unsigned char s_bpCtrl[];
extern unsigned char s_messageDrawPairList[];

void __ct__7CBpCtrlFv(void*);
void CBpCtrl_dtor();
void CBpCtrl_DeleteEntryById(void*, int);
void PairVector_InitReserved(void*);
void MessageDrawPairList_Dtor();
void PointerPairList_RemoveByKey(void*, void*);
void Layout_Destroy(void*, int);
void CSharedFileHandle_Destroy(void*, int);
void SoundManager_RequestBgmStop(void*, int);
void CBpCtrlHandle_Dtor();
void CBpCtrlHandle_ReleaseEntry(void*);
void String_ReplaceRange(void*, int, int, int, int, void*);
void appFreeUni__FPv(void*);
void __dl__FPv(void*);

enum BgmCleanupConstants { kNoBpEntry = -1, kBpRegistrationOffset = 0xC,
                          kDrawListRegistrationOffset = 0x18,
                          kBpHandleRegistrationOffset = 0x24 };

// Unregister the collection and release its UI, music, shared file, and owned text.
// All released members are owned; the global sound/controller/list singletons are borrowed.
// deleteStorage > 0 also frees self. Returns the original address, possibly freed.
void* CBgmCollection_dtor(CBgmCollection* self, int deleteStorage) {
    unsigned char* destructorRegistrationRecords = s_inputManagerDtorRegistration_BgmCollection;
    if (self != 0) {
        self->vtbl = __vt__14CBgmCollection;
        // Unregister the BP entry first; lazily create its global owner if necessary.
        if (self->bpEntryId != kNoBpEntry) {
            if (((int&)Sing_805DC5F0()) == 0) {
                if (!s_bpCtrlInitialized) {
                    __ct__7CBpCtrlFv(s_bpCtrl);
                    __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, (destructorRegistrationRecords + kBpRegistrationOffset));
                    s_bpCtrlInitialized = 1;
                }
                ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
            }
            CBpCtrl_DeleteEntryById((void*)((int&)Sing_805DC5F0()), self->bpEntryId);
            self->bpEntryId = kNoBpEntry;
        }
        // Remove the draw registration before destroying the resource and draw objects.
        // alternateTextTree is actually passed to CMsgDraw operations by the menu code.
        if (((int&)Sing_805DC650()) == 0) {
            if (!s_pointerPairListInitialized) {
                PairVector_InitReserved(s_messageDrawPairList);
                __register_global_object(s_messageDrawPairList, (void*)MessageDrawPairList_Dtor, (destructorRegistrationRecords + kDrawListRegistrationOffset));
                s_pointerPairListInitialized = 1;
            }
            ((int&)Sing_805DC650()) = (int)s_messageDrawPairList;
        }
        PointerPairList_RemoveByKey((void*)((int&)Sing_805DC650()), self->messageDraw);
        // Keep the repeated null tests: only expand formatting, not cleanup structure.
        if (self->messageResource) {
            if (self->messageResource) ((Virtual80318428*)self->messageResource)->destroy(1);
            self->messageResource = 0;
        }
        if (self->messageDraw) {
            if (self->messageDraw) ((Virtual80318428*)self->messageDraw)->destroy(1);
            self->messageDraw = 0;
        }
        if (self->trackTable) {
            if (self->trackTable) ((VirtualOffset80318428*)self->trackTable)->destroy(1);
            self->trackTable = 0;
        }
        if (self->textureState) {
            if (self->textureState) ((Virtual80318428*)self->textureState)->destroy(1);
            self->textureState = 0;
        }
        if (self->layout) {
            Layout_Destroy(self->layout, 1);
            self->layout = 0;
        }
        if (self->sharedFile) {
            CSharedFileHandle_Destroy(self->sharedFile, 1);
            self->sharedFile = 0;
        }
        // UI/resource pointers are now null; stop music and release the global BP handle.
        SoundManager_RequestBgmStop(g_soundManager, 1);
        if (((int&)Sing_805DC5F8()) == 0) {
            if (!s_bpCtrlHandleInitialized) {
                s_bpCtrlHandle.second = kNoBpEntry;
                s_bpCtrlHandle.first = kNoBpEntry;
                __register_global_object(&s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor, (destructorRegistrationRecords + kBpHandleRegistrationOffset));
                s_bpCtrlHandleInitialized = 1;
            }
            ((int&)Sing_805DC5F8()) = (int)&s_bpCtrlHandle;
        }
        CBpCtrlHandle_ReleaseEntry((void*)((int&)Sing_805DC5F8()));
        // Clear the current text using the original opaque allocator-byte argument.
        // Retain this volatile stack-byte copy; it is not a meaningful menu state value.
        union ByteStorage { unsigned char byte; unsigned int alignment; } value;
        volatile unsigned char result;
        value.byte = result;
        {
            String_ReplaceRange(&self->stringFlags, 0, -1, 0, 0, &value);
        }
        // Finally release the texture pack and any heap string, then optional self storage.
        if (self->textureResource) {
            if (self->textureResource) ((Virtual80318428*)self->textureResource)->destroy(1);
            self->textureResource = 0;
        }
        // Preserve the original member-address guard before freeing heap-backed text.
        if (&self->stringFlags) {
            if ((self->stringFlags >> 31) != 0) appFreeUni__FPv(self->stringData);
        }
        if (deleteStorage > 0) __dl__FPv(self);
    }
    return self;
}

} // extern "C"
