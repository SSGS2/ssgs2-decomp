// BGM Collection teardown: unregister the collection's BP and message-draw
// registrations, destroy the UI, track, layout and texture objects it owns,
// stop the background music, and release the shared file and its text.
#include "game/singletons.h"
#include "sdk_protos.h"
#include "classes/CBgmCollection.h"
#include "classes/CSharedFileHandle.h"
#include "classes/CBpCtrl.h"
#include "classes/CBpCtrlHandle.h"

// Objects the collection owns and deletes through their second vtable slot.
// The slot's name is inferred from the calls below.
struct DestroyableObject {
    virtual void destroy(int);
};
// trackTable keeps its vtable pointer in its second word.
struct OffsetObject {
    int padding;
};
struct OffsetDestroyableObject : OffsetObject {
    virtual void destroy(int);
};

extern "C" {

extern signed char s_bpCtrlInitialized;
extern signed char s_pointerPairListInitialized;
extern signed char s_bpCtrlHandleInitialized;
struct BpCtrlHandle {
    int first;
    int second;
};
extern BpCtrlHandle s_bpCtrlHandle;
extern void* g_soundManager;
// Registration slots for the destructors of the globals created on demand here.
extern unsigned char s_inputManagerDtorRegistration_BgmCollection[];
extern unsigned char __vt__14CBgmCollection[];
extern unsigned char s_bpCtrl[];
extern unsigned char s_messageDrawPairList[];

void CBpCtrl_dtor();
void PairVector_InitReserved(void*);
void MessageDrawPairList_Dtor();
void PointerPairList_RemoveByKey(void*, void*);
void Layout_Destroy(void*, int);
void SoundManager_RequestBgmStop(void*, int);
void CBpCtrlHandle_Dtor();
void String_ReplaceRange(void*, int, int, int, int, void*);
void appFreeUni__FPv(void*);
void __dl__FPv(void*);

enum BgmCleanupConstants {
    kNoBpEntry = -1,
    kBpRegistrationOffset = 0xC,
    kDrawListRegistrationOffset = 0x18,
    kBpHandleRegistrationOffset = 0x24
};

} // extern "C"

// Destroy the collection and every object it owns. A deleteStorage above zero
// also frees the object; either way the address is returned, freed or not.
// Named Destroy rather than a destructor: the destructor form changes the prologue.
void* CBgmCollection::Destroy(int deleteStorage) {
    unsigned char* destructorRegistrationRecords = s_inputManagerDtorRegistration_BgmCollection;
    // The null guard is part of the original teardown.
    if (this != 0) {
        this->vtbl = __vt__14CBgmCollection;
        // Unregister the BP entry first; lazily create its global owner if necessary.
        if (this->bpEntryId != kNoBpEntry) {
            if (Sing_805DC5F0() == 0) {
                if (!s_bpCtrlInitialized) {
                    ((CBpCtrl*)s_bpCtrl)->__ct();
                    __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor,
                                             destructorRegistrationRecords + kBpRegistrationOffset);
                    s_bpCtrlInitialized = 1;
                }
                Sing_805DC5F0() = s_bpCtrl;
            }
            ((CBpCtrl*)Sing_805DC5F0())->DeleteEntryById(this->bpEntryId);
            this->bpEntryId = kNoBpEntry;
        }
        // Remove the draw registration before destroying the resource and draw objects.
        if (Sing_805DC650() == 0) {
            if (!s_pointerPairListInitialized) {
                PairVector_InitReserved(s_messageDrawPairList);
                __register_global_object(s_messageDrawPairList, (void*)MessageDrawPairList_Dtor,
                                         destructorRegistrationRecords + kDrawListRegistrationOffset);
                s_pointerPairListInitialized = 1;
            }
            Sing_805DC650() = s_messageDrawPairList;
        }
        PointerPairList_RemoveByKey(Sing_805DC650(), this->messageDraw);
        // The doubled guard is how a deleting-destructor call expands here.
        if (this->messageResource) {
            if (this->messageResource)
                ((DestroyableObject*)this->messageResource)->destroy(1);
            this->messageResource = 0;
        }
        if (this->messageDraw) {
            if (this->messageDraw)
                ((DestroyableObject*)this->messageDraw)->destroy(1);
            this->messageDraw = 0;
        }
        if (this->trackTable) {
            if (this->trackTable)
                ((OffsetDestroyableObject*)this->trackTable)->destroy(1);
            this->trackTable = 0;
        }
        if (this->textureState) {
            if (this->textureState)
                ((DestroyableObject*)this->textureState)->destroy(1);
            this->textureState = 0;
        }
        if (this->layout) {
            Layout_Destroy(this->layout, 1);
            this->layout = 0;
        }
        if (this->sharedFile) {
            CSharedFileHandle::Destroy(this->sharedFile, 1);
            this->sharedFile = 0;
        }
        // The UI and resources are gone; stop the music and release the BP handle.
        SoundManager_RequestBgmStop(g_soundManager, 1);
        if (Sing_805DC5F8() == 0) {
            if (!s_bpCtrlHandleInitialized) {
                s_bpCtrlHandle.second = kNoBpEntry;
                s_bpCtrlHandle.first = kNoBpEntry;
                __register_global_object(&s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor,
                                         destructorRegistrationRecords + kBpHandleRegistrationOffset);
                s_bpCtrlHandleInitialized = 1;
            }
            Sing_805DC5F8() = &s_bpCtrlHandle;
        }
        ((CBpCtrlHandle*)Sing_805DC5F8())->ReleaseEntry();
        // Clear the text; the byte argument is copied from the stack, as the call site does.
        union ByteStorage {
            unsigned char byte;
            unsigned int alignment;
        } value;
        volatile unsigned char result;
        value.byte = result;
        String_ReplaceRange(&this->stringFlags, 0, -1, 0, 0, &value);
        // Release the texture pack, then the heap-backed text.
        if (this->textureResource) {
            if (this->textureResource)
                ((DestroyableObject*)this->textureResource)->destroy(1);
            this->textureResource = 0;
        }
        // The member address guard is how the inline string destructor expands here.
        if (&this->stringFlags) {
            if ((this->stringFlags >> 31) != 0)
                appFreeUni__FPv(this->stringData);
        }
        if (deleteStorage > 0)
            __dl__FPv(this);
    }
    return this;
}
