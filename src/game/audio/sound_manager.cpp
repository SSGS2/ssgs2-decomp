// Sound manager: buffered effects, two-slot music/voice streams, bank/name caches,
// output routing and fades. Shared record schemas: SoundPlaybackState.h (0x64),
// SoundChannelGroup.h (selected + two records), SoundManagerState.h, FloatTween.h.
// Mod map: output mode/bank switch = SoundManager_SetOutputMode transition map;
// fades in seconds = SoundStreamPlayback_Start/StopWithOptionalFade and stream
// update; saved music/effect percentages = ApplySavedAudioSettings; remote send
// selection = Sound_PlayByNameWithParameters, consumed in gtl_sound_buffer_envelope_lfo.cpp.
// pointers. Required manager/resources remain live for each call; string/resource
// ownership is described at reset/load boundaries. Manager operations must be
// serialized: per-map locks protect cursors, not whole multi-step transitions.
#include "classes/FloatTween.h"
#include "classes/GtlSoundStreamInterface.h"
#include "classes/SoundChannelGroup.h"
#include "classes/FilenameString.h"
#include "classes/SoundPlaybackState.h"
#include "classes/SoundManagerState.h"
#include "classes/SoundPlaybackInstanceView.h"
#include "classes/CHashPath.h"
#include "classes/GtlSoundBuffer.h"
#define AsSoundBufferPlayback(p) ((SoundPlaybackInstanceView*)(p))
// Stream states are shared by both two-slot music/voice groups.
enum SoundStreamState {
    kStreamIdle = 0,
    kStreamLoadingBank = 1,
    kStreamPlaying = 2,
    kStreamFadingOut = 3
};
enum SoundOutputMode {
    kSoundMono = 0,
    kSoundStereo = 1,
    kSoundDolbyProLogicII = 2
};
enum SoundOutputMask {
    kOutputMono = 1,
    kOutputStereo = 2,
    kOutputDolbyProLogicII = 4
};
static const int kPlayLooped = 1;
static const int kPlayOnce = 2;
enum SoundFadeMode {
    kFadeDefault = -1,
    kFadeIn = 1,
    kFadeOut = 2
};
enum SoundStatusBits {
    kSoundStatusPlaying = 1,
    kSoundStatusFinished = 8,
    kSoundStatusPending = 0x40000
};
enum {
    kInlineStringLengthMask = 0x7F,
    kHeapStringFlagShift = 31,
    kSavedSoundModeShift = 28,
    kSavedSoundModeMask = 3,
    kStreamSlotBytes = sizeof(SoundPlaybackState),
    kVoiceGroupOffset = sizeof(SoundChannelGroup),
    kMusicVolumeTweenOffset = sizeof(FloatTween),
    kAvatarVoiceCount = 8,
    kPathPartBufferBytes = 0x104,
    kPlaybackNameReserveCharacters = 0x40,
    kPlaybackPathReserveCharacters = 0x10,
    kPlaybackSlotsPerGroup = 2
};

// A slot-origin pointer is four bytes BEFORE its playback record. For slot 0
// it is the group header; advancing by 0x64 reaches the next slot's origin.
// Only the actual group header owns selected. Never read selected at slot 1's
// origin: it overlaps the previous record's tail. Record fields below are relative
// to entry=origin+4. String objects are 12 bytes; fade is a FloatTween at +0x24.
enum PlaybackRecordByteOffset {
    kPlaybackRecordFromSlotOrigin = 4,
    kPlaybackNameOffset = (int)&((SoundPlaybackState*)0)->nameFlags,
    kPlaybackFilePathOffset = (int)&((SoundPlaybackState*)0)->filePathFlags,
    kPlaybackResourcePathOffset = (int)&((SoundPlaybackState*)0)->resourcePathFlags,
    kPlaybackFadeOffset = (int)&((SoundPlaybackState*)0)->fadeMode
};

// Borrow a playback record from the address four bytes before it; no group index is read.
static inline SoundPlaybackState* PlaybackAtSlotOrigin(void* origin) {
    return (SoundPlaybackState*)((unsigned char*)origin + kPlaybackRecordFromSlotOrigin);
}

// Only the audio options slice of the save-slot payload is used in this unit.
// SavePlayerRecord's shared header currently leaves this slice as padding.
struct SavedAudioOptionsView {
    char pad000[0x4E0];
    int musicVolumePercent;
    int effectVolumePercent;
    unsigned int optionBits; // sound mode occupies bits 28..29
};
struct SoundNameMapView {
    void* head;
    void* cursor;
    void* chunks;
    void* freeNodes;
    unsigned int chunkSize;
    unsigned int hashMask;
    int bucketCount;
    void** buckets;
    unsigned char mutex[0x18];
};
struct SoundManagerResources {
    SoundChannelGroup music;
    SoundChannelGroup voice;
    SoundNameMapView buffers;
    SoundNameMapView banks;
};
struct SoundListLink {
    void* value;
    char pad004[0xC];
    void* next;
};
struct SoundResourceHeader {
    unsigned int references;
    void** vtable;
    void* data;
};
struct SoundInstanceTextView {
    char name[0x40];
    char alias[0x20];
    float position[3];
};
struct SoundCameraTransformView {
    char pad000[0x74];
    float worldToListener[16];
};
struct SoundNamePrefixes {
    char caddie[0xB]; // "SE_caddie_"
    char avatar[0xB]; // "SE_avatar_"
    char course[0xB]; // "SE_course_"
};
// Compile-time layout checks protect these local views when fields are edited.
typedef char SavedAudio_musicOffset[((int)&((SavedAudioOptionsView*)0)->musicVolumePercent == 0x4E0) ? 1 : -1];
typedef char SavedAudio_optionsOffset[((int)&((SavedAudioOptionsView*)0)->optionBits == 0x4E8) ? 1 : -1];
typedef char SoundMap_size[(sizeof(SoundNameMapView) == 0x38) ? 1 : -1];
typedef char SoundResources_buffersOffset[((int)&((SoundManagerResources*)0)->buffers == 0x198) ? 1 : -1];
typedef char SoundResources_banksOffset[((int)&((SoundManagerResources*)0)->banks == 0x1D0) ? 1 : -1];
typedef char SoundLink_nextOffset[((int)&((SoundListLink*)0)->next == 0x10) ? 1 : -1];

// Derive byte addresses from the shared layout where direct member addressing
// changes CodeWarrior's register allocation in the inlined path-building code.
enum {
    kFilenameDriveOffset = (int)&((FilenameString*)0)->drive,
    kFilenameDirectoryOffset = (int)&((FilenameString*)0)->directory,
    kFilenameStemOffset = (int)&((FilenameString*)0)->stem,
    kFilenameExtensionOffset = (int)&((FilenameString*)0)->extension,
    kFilenameDriveDirectoryOffset = (int)&((FilenameString*)0)->driveDirectory,
    kFilenameFileNameOffset = (int)&((FilenameString*)0)->fileName,
    kBufferListHeadOffset = (int)&((SoundManagerResources*)0)->buffers.head,
    kBufferListCursorOffset = (int)&((SoundManagerResources*)0)->buffers.cursor,
    kBufferListMutexOffset = (int)&((SoundManagerResources*)0)->buffers.mutex,
    kBankListHeadOffset = (int)&((SoundManagerResources*)0)->banks.head
};

#define AsSoundResources(p) ((SoundManagerResources*)(p))

extern "C" void appFreeUni__FPv(void*);

// Own a completed string only while its enclosing operation can unwind.
struct EH5StringCleanup {
    const unsigned int* words;
    bool active;
    // Guard the borrowed string-layout words until explicit cleanup is disarmed.
    explicit EH5StringCleanup(const void* p) : words((const unsigned int*)p), active(true) {}
    // Disarm automatic string cleanup after ownership has been handled explicitly.
    void release() {
        active = false;
    }
    // Release the guarded string allocation only while cleanup remains armed.
    ~EH5StringCleanup() {
        if (active && (words[0] & 0x80000000u))
            appFreeUni__FPv((void*)words[2]);
    }
};

#include "game/singletons.h"
#include "classes/CSoundSourceListNode.h"
#include "classes/CSharedFileHandle.h"
#include "classes/CSharedFilePtr.h"
#define AsCSoundSourceListNode(p) ((CSoundSourceListNode*)(p))

extern "C" {

void OSLockMutex(void*);
void OSUnlockMutex(void*);
int strlen(const char*);
int memcmp(const void*, const void*, unsigned int);
void RemoteAudio_RequestEnableAllLazy();
void gtl_RemoteAudio_EnsureInitializedAndDisableAll();

unsigned char SCGetSoundMode();
// Borrowed world-space XYZ input, copied into the instance on the spatial path.
struct SoundPositionVector {
    float x, y, z;
};
// x/y are minimum/maximum spatial distances in the engine's world units.
struct SoundPlaybackScale2 {
    float x, y;
};
unsigned char* SoundManager_CreateSoundInstance(unsigned char*, const char* = 0, const SoundPositionVector* = 0,
                                                unsigned int = 0, const char* = 0, const SoundPlaybackScale2* = 0,
                                                float = 0.0f, float = 0.0f, float = 0.0f);
void* SoundManager_FindFirstBufferByAliasPrefixIgnoreCase(void*, const char*);
int SoundChannelGroup_IsSelectedStreamNonIdleWithMatchingStem(void*, const char*);
void SetPitch__Q23gtl12ISoundBufferFf(void*, float);
void SetVolume__Q23gtl12ISoundStreamFf(void*, float);
void SetPitch__Q23gtl12ISoundStreamFf(void*, float);

// Tail-forwarder to the C library case-insensitive comparison (__msl_strnicmp).
int String_ComparePrefixIgnoreCase(void*, const void*, int);
void FloatTween_Start(void*, int, float, float);
int SoundPlayback_MatchesFileName(void*, const char*);
int SoundPlaybackState_BeginBankLoad(void*, const char*, int);
void SoundStreamPlayback_Start(unsigned char*, int, float, int, int);
void* SharedFile_Create(void*, int);
void* FilenameString_Ctor(void*, const char*);
const char* FilenameString_SplitAndGetStem(void*);
void* FilenameString_dtor(unsigned char*, int);
void* SaveData_GetCurrentRecord(void*);
void SaveData_Ctor(void*);
void SoundManager_SetOutputMode(unsigned char*, int);
void Audio_SetStreamVolumeWithFade(float, float);
void appFreeUni__FPv(void*);
void __dl__FPv(void*);
void StdString_ReserveCapacity(void*, unsigned int);
void StdString_ReplaceRange(void*, int, int, const char*, const char*, void*);
void FileIO_SplitPath(const char*, char*, char*, char*, char*);
void String_ReplaceRange(void*, int, unsigned int, const char*, const char*, unsigned char*);
void SoundManager_UpdateStreamChannels(void*, float) __attribute__((noinline));
void SoundBufferList_PruneFinishedPlayback(void*);
int FloatTween_Update(void*, float);
void* Matrix_TransformPoint(void*, void*, void*);
void GtlAudio_SetCachedOutputOption(int);
void gtl_Audio_SetOutputMode(int);
void AudioMix_SetPreserveStereo(int);
void String_ReserveCapacity(void*, int);
void* CSharedFilePtr_GetLoadedBuffer(void*);
int gtl_Audio_CreateSoundResource(void*, void**, int);
int gtl_Audio_CreateSoundStreamResource(void*, void**);
int gtl_Audio_CreateSoundStream(void*, void*);
void* memcpy(void*, const void*, unsigned int);

extern float kFloat_0_soundPlayback;
extern const float kFloat_1_SoundPlayback;
extern const float kFloat_160_SoundManager;
extern float kFloat_32_SoundManager;
extern const float kFloat_960_SoundManager;
extern const float kDefaultSoundUpdateSeconds;
extern const double kSignedIntToDoubleBias_SoundManager;
extern const float kFloat_100_SoundManager;
extern float s_soundListenerZOffset;
extern unsigned char s_firstSoundQueryNameLengthInitialized;
extern signed char s_saveDataInitialized;
extern const char s_saveData[];
extern int s_firstSoundQueryNameLength;
extern const unsigned int kSoundParameterDescriptorWord0;
extern const unsigned int kSoundParameterDescriptorUnityBits;
extern const unsigned int kSoundParameterChainHeader;
extern char __vt__20CSoundSourceListNode[];
extern void* s_avatarVoiceNames[];
extern char __vt__14filenamestring[];
extern const SoundNamePrefixes s_soundNamePrefixes;
extern char s_soundPlaybackEmptyName[4];
extern char s_soundPlaybackEmptyFilePath[4];
extern char s_soundPlaybackEmptyResourcePath[4];
extern char s_standardSoundBankSuffix[4];
extern char s_dolbySoundBankSuffix[6];
extern char s_streamMetadataExtension[5];
extern char s_streamSampleExtension[5];
struct RcObj {
    unsigned int refs;
    void* vtable;
};

// Empty allocator tags are passed by value to preserve the Metrowerks string ABI.
struct StrAlloc {};
typedef void (*StrAssignFn)(void*, int, int, const char*, const char*, StrAlloc);
// Replace a string range with a by-value empty allocator tag.
inline void AssignStringRange(void* s_, int a_, int b_, const char* f_, const char* l_) {
    StrAlloc al;
    ((StrAssignFn)StdString_ReplaceRange)(s_, a_, b_, f_, l_, al);
}
// A lexical allocator tag preserves argument evaluation and cleanup in this path.
// The last macro argument is intentionally unused; an inline-helper trial changed code and unwind relocations.
#define StdString_ReplaceRange(s_, a_, b_, f_, l_, al_)                 \
    {                                                                   \
        StrAlloc tag;                                                   \
        ((StrAssignFn)StdString_ReplaceRange)(s_, a_, b_, f_, l_, tag); \
    }
// Split the input path, cache its components, remove the drive colon, and return the filename stem.
// Borrow self; return a pointer into its stem string, valid until that string is
// modified/destroyed. Cached strings own their own inline or heap character data.
// Component map: input = original path; drive = split drive with trailing colon
// overwritten by NUL; directory/stem/extension = split results; driveDirectory =
// original split drive + directory; fileName = stem + extension (no directory).
// The drive terminator overwrite does not reduce its stored string length.
const char* FilenameString_SplitAndGetStem(void* self) {
    // Phase 1: split borrowed input into four bounded scratch buffers.
    char extension[kPathPartBufferBytes] = {0};
    char stem[kPathPartBufferBytes] = {0};
    char directory[kPathPartBufferBytes] = {0};
    char drive[kPathPartBufferBytes] = {0};
    char* filenameBytes = (char*)self;
    const char* inputText;
    if ((AsFilenameString(filenameBytes)->input.flags >> kHeapStringFlagShift) == 0)
        inputText = filenameBytes + 1;
    else
        inputText = AsFilenameString(filenameBytes)->input.data;
    FileIO_SplitPath(inputText, drive, directory, stem, extension);

    // Phase 2: replace drive, directory, stem and extension caches. Repeated
    // inline/heap branches select the old erase length, not the new component length.
    unsigned int length;
    if ((AsFilenameString(filenameBytes)->drive.flags >> kHeapStringFlagShift) == 0)
        length = ((unsigned char*)&AsFilenameString(filenameBytes)->drive.flags)[0] & kInlineStringLengthMask;
    else
        length = AsFilenameString(filenameBytes)->drive.length;
    StdString_ReplaceRange(filenameBytes + kFilenameDriveOffset, 0, (int)length, drive, drive + strlen(drive), &t0);

    if ((AsFilenameString(filenameBytes)->directory.flags >> kHeapStringFlagShift) == 0)
        length = ((unsigned char*)&AsFilenameString(filenameBytes)->directory.flags)[0] & kInlineStringLengthMask;
    else
        length = AsFilenameString(filenameBytes)->directory.length;
    StdString_ReplaceRange(filenameBytes + kFilenameDirectoryOffset, 0, (int)length, directory,
                           directory + strlen(directory), &t1);

    if ((AsFilenameString(filenameBytes)->stem.flags >> kHeapStringFlagShift) == 0)
        length = ((unsigned char*)&AsFilenameString(filenameBytes)->stem.flags)[0] & kInlineStringLengthMask;
    else
        length = AsFilenameString(filenameBytes)->stem.length;
    StdString_ReplaceRange(filenameBytes + kFilenameStemOffset, 0, (int)length, stem, stem + strlen(stem), &t2);

    if ((AsFilenameString(filenameBytes)->extension.flags >> kHeapStringFlagShift) == 0)
        length = ((unsigned char*)&AsFilenameString(filenameBytes)->extension.flags)[0] & kInlineStringLengthMask;
    else
        length = AsFilenameString(filenameBytes)->extension.length;
    StdString_ReplaceRange(filenameBytes + kFilenameExtensionOffset, 0, (int)length, extension,
                           extension + strlen(extension), &t3);

    // Phase 3: hide the drive colon in the drive cache only. The scratch drive
    // still has it, so driveDirectory below preserves the original path syntax.
    unsigned int isInline = (AsFilenameString(filenameBytes)->drive.flags >> kHeapStringFlagShift) == 0;
    length = isInline ? (((unsigned char*)&AsFilenameString(filenameBytes)->drive.flags)[0] & kInlineStringLengthMask)
                      : AsFilenameString(filenameBytes)->drive.length;
    if (length) {
        unsigned int partLength =
            isInline ? (((unsigned char*)&AsFilenameString(filenameBytes)->drive.flags)[0] & kInlineStringLengthMask)
                     : AsFilenameString(filenameBytes)->drive.length;
        char* partText =
            isInline ? filenameBytes + kFilenameDriveOffset + 1 : AsFilenameString(filenameBytes)->drive.data;
        partText[partLength - 1] = 0;
    }

    // Phase 4: assemble driveDirectory from the unmodified split drive and directory.
    if ((AsFilenameString(filenameBytes)->driveDirectory.flags >> kHeapStringFlagShift) == 0)
        length = ((unsigned char*)&AsFilenameString(filenameBytes)->driveDirectory.flags)[0] & kInlineStringLengthMask;
    else
        length = AsFilenameString(filenameBytes)->driveDirectory.length;
    StdString_ReplaceRange(filenameBytes + kFilenameDriveDirectoryOffset, 0, (int)length, drive, drive + strlen(drive),
                           &t4);
    if ((AsFilenameString(filenameBytes)->driveDirectory.flags >> kHeapStringFlagShift) == 0)
        length = ((unsigned char*)&AsFilenameString(filenameBytes)->driveDirectory.flags)[0] & kInlineStringLengthMask;
    else
        length = AsFilenameString(filenameBytes)->driveDirectory.length;
    StdString_ReplaceRange(filenameBytes + kFilenameDriveDirectoryOffset, (int)length, 0, directory,
                           directory + strlen(directory), &t5);

    // Phase 5: copy stem to fileName, then append extension. The three-word fast
    // path applies only when both strings are inline; otherwise replace by range.
    if ((AsFilenameString(filenameBytes)->fileName.flags >> kHeapStringFlagShift) == 0 &&
        (AsFilenameString(filenameBytes)->stem.flags >> kHeapStringFlagShift) == 0) {
        AsFilenameString(filenameBytes)->fileName.flags = AsFilenameString(filenameBytes)->stem.flags;
        AsFilenameString(filenameBytes)->fileName.length = AsFilenameString(filenameBytes)->stem.length;
        (*(unsigned int*)&AsFilenameString(filenameBytes)->fileName.data) =
            (*(unsigned int*)&AsFilenameString(filenameBytes)->stem.data);
    } else {
        unsigned int oldLength =
            (AsFilenameString(filenameBytes)->fileName.flags >> kHeapStringFlagShift)
                ? AsFilenameString(filenameBytes)->fileName.length
                : (((unsigned char*)&AsFilenameString(filenameBytes)->fileName.flags)[0] & kInlineStringLengthMask);
        unsigned int partLength;
        char* partText;
        if ((AsFilenameString(filenameBytes)->stem.flags >> kHeapStringFlagShift) == 0) {
            partLength = ((unsigned char*)&AsFilenameString(filenameBytes)->stem.flags)[0] & kInlineStringLengthMask;
            partText = filenameBytes + kFilenameStemOffset + 1;
        } else {
            partLength = AsFilenameString(filenameBytes)->stem.length;
            partText = AsFilenameString(filenameBytes)->stem.data;
        }
        StdString_ReplaceRange(filenameBytes + kFilenameFileNameOffset, 0, (int)oldLength, partText,
                               partText + partLength, &t6);
    }
    length = (AsFilenameString(filenameBytes)->fileName.flags >> kHeapStringFlagShift)
                 ? AsFilenameString(filenameBytes)->fileName.length
                 : (((unsigned char*)&AsFilenameString(filenameBytes)->fileName.flags)[0] & kInlineStringLengthMask);
    unsigned int partLength;
    char* partText;
    if ((AsFilenameString(filenameBytes)->extension.flags >> kHeapStringFlagShift) == 0) {
        partLength = ((unsigned char*)&AsFilenameString(filenameBytes)->extension.flags)[0] & kInlineStringLengthMask;
        partText = filenameBytes + kFilenameExtensionOffset + 1;
    } else {
        partLength = AsFilenameString(filenameBytes)->extension.length;
        partText = AsFilenameString(filenameBytes)->extension.data;
    }
    StdString_ReplaceRange(filenameBytes + kFilenameFileNameOffset, (int)length, 0, partText, partText + partLength,
                           &t7);
    if ((AsFilenameString(filenameBytes)->stem.flags >> kHeapStringFlagShift) == 0)
        return filenameBytes + kFilenameStemOffset + 1;
    return AsFilenameString(filenameBytes)->stem.data;
}

#undef StdString_ReplaceRange

// Read the logical character count from inline or heap string storage.
static unsigned int get_string_length(void* p) {
    return (AsMWStringStorage(p)->flags >> kHeapStringFlagShift)
               ? AsMWStringStorage(p)->length
               : (*(unsigned char*)&AsMWStringStorage(p)->flags & kInlineStringLengthMask);
}
// Replace an entire string with the supplied text.
static void reset_string(void* p, const char* src) {
    unsigned int oldLength = get_string_length(p);
    unsigned char allocatorTag;
    unsigned int len = strlen(src);
    String_ReplaceRange(p, 0, oldLength, src, src + len, &allocatorTag);
}
struct VP4Pre_ {
    int pad;
};
struct VP4Obj_ : VP4Pre_ {
    virtual void destroy(int);
};
// Stop a stream and release its reference; the caller resets the stored pointer.
static void reset_rc(void** p) {
    void* obj = *p;
    if (obj) {
        ((gtl::ISoundStream*)obj)->Stop(0);
        obj = *p;
        if (--*(int*)obj == 0 && obj != 0) {
            ((VP4Obj_*)obj)->destroy(1);
        }
    }
}

// Release the stopped record and restore its owned strings, gains and load defaults.
// self is the playback record itself; source/sharedFile ownership is consumed here.
static inline void ResetStoppedPlaybackRecord(unsigned char* self) {
    reset_rc((void**)self);
    if (AsSoundPlaybackState(self)->sharedFile) {
        CSharedFileHandle::Destroy(AsSoundPlaybackState(self)->sharedFile, 1);
        AsSoundPlaybackState(self)->sharedFile = 0;
    }
    AsSoundPlaybackState(self)->source = 0;
    AsSoundPlaybackState(self)->volume = kFloat_1_SoundPlayback;
    AsSoundPlaybackState(self)->pitch = kFloat_1_SoundPlayback;
    AsSoundPlaybackState(self)->playState = kStreamIdle;
    unsigned int cap;
    if (AsSoundPlaybackState(self)->nameFlags >> kHeapStringFlagShift)
        cap = AsSoundPlaybackState(self)->nameLength;
    else
        cap = ((char*)&AsSoundPlaybackState(self)->nameFlags)[0] & kInlineStringLengthMask;
    unsigned char allocatorTag;
    String_ReplaceRange(self + 0x10, 0, cap, s_soundPlaybackEmptyName,
                        s_soundPlaybackEmptyName + strlen(s_soundPlaybackEmptyName), &allocatorTag);
    AsSoundPlaybackState(self)->loopEnabled = AsSoundPlaybackState(self)->fadeInEnabled =
        AsSoundPlaybackState(self)->useDolbyBank = 0;
    AsSoundPlaybackState(self)->transitionDuration = kFloat_0_soundPlayback;
    if (AsSoundPlaybackState(self)->filePathFlags >> kHeapStringFlagShift)
        cap = AsSoundPlaybackState(self)->filePathLength;
    else
        cap = ((char*)&AsSoundPlaybackState(self)->filePathFlags)[0] & kInlineStringLengthMask;
    String_ReplaceRange(self + 0x40, 0, cap, s_soundPlaybackEmptyFilePath,
                        s_soundPlaybackEmptyFilePath + strlen(s_soundPlaybackEmptyFilePath), &allocatorTag);
    if (AsSoundPlaybackState(self)->resourcePathFlags >> kHeapStringFlagShift)
        cap = AsSoundPlaybackState(self)->resourcePathLength;
    else
        cap = ((char*)&AsSoundPlaybackState(self)->resourcePathFlags)[0] & kInlineStringLengthMask;
    String_ReplaceRange(self + 0x4c, 0, cap, s_soundPlaybackEmptyResourcePath,
                        s_soundPlaybackEmptyResourcePath + strlen(s_soundPlaybackEmptyResourcePath), &allocatorTag);
    FloatTween_Start(self + 0x24, kFadeDefault, kFloat_0_soundPlayback, kFloat_0_soundPlayback);
    AsSoundPlaybackState(self)->baseVolume = kFloat_1_SoundPlayback;
    AsSoundPlaybackState(self)->basePitch = kFloat_1_SoundPlayback;
    String_ReserveCapacity(self + 0x10, 0x40);
    String_ReserveCapacity(self + 0x40, 0x10);
    String_ReserveCapacity(self + 0x4c, 0x10);
}

// Preserve the exported call boundary after extracting its reset phase.
void SoundPlayback_StopWithOptionalFade(unsigned char*, float, int) __attribute__((noinline));
// Fade a stream out when possible; otherwise release it and reset all playback fields.
void SoundPlayback_StopWithOptionalFade(unsigned char* self, float fadeSeconds, int allowFade) {
    if (allowFade && fadeSeconds > kFloat_0_soundPlayback &&
        AsSoundPlaybackState(self)->playState != kStreamLoadingBank) {
        AsSoundPlaybackState(self)->playState = kStreamFadingOut;
        FloatTween_Start(self + 0x24, kFadeOut, kFloat_0_soundPlayback, fadeSeconds);
        return;
    }
    FloatTween_Start(self + 0x24, kFadeDefault, kFloat_0_soundPlayback, kFloat_0_soundPlayback);
    ResetStoppedPlaybackRecord(self);
}

// Stop or fade the selected slot of a two-slot stream group.
void SoundChannelGroup_StopWithFade(unsigned char* self, float fadeSeconds, int allowFade) {
    // Select once; all subsequent addresses are relative to the playback record.
    unsigned char* record = self + kStreamSlotBytes * *(int*)self + kPlaybackRecordFromSlotOrigin;
    if (allowFade && fadeSeconds > kFloat_0_soundPlayback &&
        AsSoundPlaybackState(record)->playState != kStreamLoadingBank) {
        AsSoundPlaybackState(record)->playState = kStreamFadingOut;
        FloatTween_Start(record + kPlaybackFadeOffset, kFadeOut, kFloat_0_soundPlayback, fadeSeconds);
        return;
    }
    // Immediate stop: release handles before clearing names and resetting gains.
    FloatTween_Start(record + kPlaybackFadeOffset, kFadeDefault, kFloat_0_soundPlayback, kFloat_0_soundPlayback);
    reset_rc((void**)record);
    if (AsSoundPlaybackState(record)->sharedFile) {
        CSharedFileHandle::Destroy(AsSoundPlaybackState(record)->sharedFile, 1);
        AsSoundPlaybackState(record)->sharedFile = 0;
    }
    AsSoundPlaybackState(record)->source = 0;
    AsSoundPlaybackState(record)->volume = kFloat_1_SoundPlayback;
    AsSoundPlaybackState(record)->pitch = kFloat_1_SoundPlayback;
    AsSoundPlaybackState(record)->playState = kStreamIdle;
    unsigned int cap;
    if (AsSoundPlaybackState(record)->nameFlags >> kHeapStringFlagShift)
        cap = AsSoundPlaybackState(record)->nameLength;
    else
        cap = ((char*)&AsSoundPlaybackState(record)->nameFlags)[0] & kInlineStringLengthMask;
    unsigned char allocatorTag;
    String_ReplaceRange(record + kPlaybackNameOffset, 0, cap, s_soundPlaybackEmptyName,
                        s_soundPlaybackEmptyName + strlen(s_soundPlaybackEmptyName), &allocatorTag);
    (*(char*)&AsSoundPlaybackState(record)->loopEnabled) = 0;
    (*(char*)&AsSoundPlaybackState(record)->fadeInEnabled) = 0;
    (*(char*)&AsSoundPlaybackState(record)->useDolbyBank) = 0;
    AsSoundPlaybackState(record)->transitionDuration = kFloat_0_soundPlayback;
    if (AsSoundPlaybackState(record)->filePathFlags >> kHeapStringFlagShift)
        cap = AsSoundPlaybackState(record)->filePathLength;
    else
        cap = ((char*)&AsSoundPlaybackState(record)->filePathFlags)[0] & kInlineStringLengthMask;
    String_ReplaceRange(record + kPlaybackFilePathOffset, 0, cap, s_soundPlaybackEmptyFilePath,
                        s_soundPlaybackEmptyFilePath + strlen(s_soundPlaybackEmptyFilePath), &allocatorTag);
    if (AsSoundPlaybackState(record)->resourcePathFlags >> kHeapStringFlagShift)
        cap = AsSoundPlaybackState(record)->resourcePathLength;
    else
        cap = ((char*)&AsSoundPlaybackState(record)->resourcePathFlags)[0] & kInlineStringLengthMask;
    String_ReplaceRange(record + kPlaybackResourcePathOffset, 0, cap, s_soundPlaybackEmptyResourcePath,
                        s_soundPlaybackEmptyResourcePath + strlen(s_soundPlaybackEmptyResourcePath), &allocatorTag);
    FloatTween_Start(record + kPlaybackFadeOffset, kFadeDefault, kFloat_0_soundPlayback, kFloat_0_soundPlayback);
    AsSoundPlaybackState(record)->baseVolume = kFloat_1_SoundPlayback;
    AsSoundPlaybackState(record)->basePitch = kFloat_1_SoundPlayback;
    String_ReserveCapacity(record + kPlaybackNameOffset, kPlaybackNameReserveCharacters);
    String_ReserveCapacity(record + kPlaybackFilePathOffset, kPlaybackPathReserveCharacters);
    String_ReserveCapacity(record + kPlaybackResourcePathOffset, kPlaybackPathReserveCharacters);
}

// Stop the selected music stream using the manager transition duration.
void SoundManager_RequestBgmStop(unsigned char* self, int allowFade) {
    float fadeSeconds = AsSoundManagerState(self)->transitionDuration;
    unsigned char* base = (*(unsigned char**)&AsSoundManagerState(self)->state);
    unsigned char* item = base + kStreamSlotBytes * *(int*)base;
    if (allowFade && fadeSeconds > kFloat_0_soundPlayback && AsSoundChannelGroup(item)->channels[0].playState != 1) {
        AsSoundChannelGroup(item)->channels[0].playState = 3;
        FloatTween_Start(item + 0x28, kFadeOut, kFloat_0_soundPlayback, fadeSeconds);
        return;
    }
    FloatTween_Start(item + 0x28, kFadeDefault, kFloat_0_soundPlayback, kFloat_0_soundPlayback);
    reset_rc((void**)(item + 4));
    if (AsSoundChannelGroup(item)->channels[0].sharedFile) {
        CSharedFileHandle::Destroy(AsSoundChannelGroup(item)->channels[0].sharedFile, 1);
        AsSoundChannelGroup(item)->channels[0].sharedFile = 0;
    }
    AsSoundChannelGroup(item)->channels[0].source = 0;
    AsSoundChannelGroup(item)->channels[0].volume = kFloat_1_SoundPlayback;
    AsSoundChannelGroup(item)->channels[0].pitch = kFloat_1_SoundPlayback;
    AsSoundChannelGroup(item)->channels[0].playState = 0;
    unsigned int cap;
    if (AsSoundChannelGroup(item)->channels[0].nameFlags >> kHeapStringFlagShift)
        cap = AsSoundChannelGroup(item)->channels[0].nameLength;
    else
        cap = item[0x14] & kInlineStringLengthMask;
    int nameAllocatorTag;
    String_ReplaceRange(item + 0x14, 0, cap, s_soundPlaybackEmptyName,
                        s_soundPlaybackEmptyName + strlen(s_soundPlaybackEmptyName), (unsigned char*)&nameAllocatorTag);
    item[0x20] = 0;
    item[0x21] = 0;
    item[0x22] = 0;
    AsSoundChannelGroup(item)->channels[0].transitionDuration = kFloat_0_soundPlayback;
    if (AsSoundChannelGroup(item)->channels[0].filePathFlags >> kHeapStringFlagShift)
        cap = AsSoundChannelGroup(item)->channels[0].filePathLength;
    else
        cap = item[0x44] & kInlineStringLengthMask;
    int filePathAllocatorTag;
    String_ReplaceRange(item + 0x44, 0, cap, s_soundPlaybackEmptyFilePath,
                        s_soundPlaybackEmptyFilePath + strlen(s_soundPlaybackEmptyFilePath),
                        (unsigned char*)&filePathAllocatorTag);
    if (AsSoundChannelGroup(item)->channels[0].resourcePathFlags >> kHeapStringFlagShift)
        cap = AsSoundChannelGroup(item)->channels[0].resourcePathLength;
    else
        cap = item[0x50] & kInlineStringLengthMask;
    int resourcePathAllocatorTag;
    String_ReplaceRange(item + 0x50, 0, cap, s_soundPlaybackEmptyResourcePath,
                        s_soundPlaybackEmptyResourcePath + strlen(s_soundPlaybackEmptyResourcePath),
                        (unsigned char*)&resourcePathAllocatorTag);
    FloatTween_Start(item + 0x28, kFadeDefault, kFloat_0_soundPlayback, kFloat_0_soundPlayback);
    AsSoundChannelGroup(item)->channels[0].baseVolume = kFloat_1_SoundPlayback;
    AsSoundChannelGroup(item)->channels[0].basePitch = kFloat_1_SoundPlayback;
    String_ReserveCapacity(item + 0x14, 0x40);
    String_ReserveCapacity(item + 0x44, 0x10);
    String_ReserveCapacity(item + 0x50, 0x10);
}

// Create/find a named sound, apply one unity-gain send descriptor and play it.
// parameterModeToken is an INTEGER in an ABI pointer slot; no array is read and
// there is no parameterValues length. The unsigned-short pointer addition below
// computes mode = token+2 bytes, not *(token+1). Tokens 0..3 select remote sends
// 0..3 (modes 2..5); consumer modes 0/1 mean main/aux, other modes are ignored.
// CGolfPlayer_PlaySwingSound (golf_player_actor.cpp) supplies the player-entry leading word.
// Stack schema (all borrowed synchronously by gtl_sound_buffer_envelope_lfo.cpp ApplyParameterEntries):
// entry={s32 mode, Value*}; Value={u16 valid,u16 padding,Scalar*};
// Scalar={opaque u32,float gain}. ChainHeader 0x00010000 sets valid=1 on PPC BE;
// DescriptorWord0=0, UnityBits=0x3F800000 encode Scalar {0,1.0f}.
// The fourth argument to ApplyParameterEntries is a preserved extra ABI argument;
// the consumer takes three and reads count=1. No descriptor pointer escapes.
// type is forwarded unchanged to CreateSoundInstance (its buffer/spatial policy),
// not a send mode. Disabled manager/create failure/not-started/null-buffer returns
// before applying or playing; parameter/play statuses are ignored. Existing sound
// instance ownership remains in the manager; this function returns no status.
void Sound_PlayByNameWithParameters(unsigned char* soundManager, unsigned short* parameterModeToken, const char* name,
                                    const SoundPositionVector* positionVector, unsigned int type, const char* image,
                                    const SoundPlaybackScale2* scale, float minimumGain, float attenuationRange,
                                    float volumeScale) {
    int parameterChain[2];
    if (0 == AsSoundManagerState(soundManager)->enabled)
        return;
    unsigned char* instance = SoundManager_CreateSoundInstance(soundManager, name, positionVector, type, image, scale,
                                                               minimumGain, attenuationRange, volumeScale);
    if (instance == 0)
        return;
    if (AsSoundBufferPlayback(instance)->playbackStarted != 1)
        return;
    void* soundBuffer = AsSoundBufferPlayback(instance)->soundBuffer;
    if (0 == soundBuffer)
        return;

    // Build one send-mode entry and its validity/scalar wrappers in call-local storage.
    int descriptorWords[2];
    descriptorWords[0] = kSoundParameterDescriptorWord0;

    int parameterEntry[2];
    descriptorWords[1] = kSoundParameterDescriptorUnityBits;
    parameterChain[0] = kSoundParameterChainHeader;
    parameterEntry[0] = (unsigned int)(1 + parameterModeToken);
    parameterEntry[1] = (unsigned int)parameterChain;
    parameterChain[1] = (unsigned int)descriptorWords;
    ((gtl::ISoundBuffer*)soundBuffer)->ApplyParameterEntries((SoundParameterEntry*)parameterEntry, 1, parameterChain);
    unsigned int mode = kPlayOnce;
    if (AsSoundBufferPlayback(instance)->playbackFlags & kPlayLooped)
        mode = kPlayLooped;
    ((gtl::ISoundBuffer*)soundBuffer)->Play(mode);
}

// Query the first case-insensitive alias-prefix buffer match. If no buffer matches,
// test the selected music/voice stream's filename stem for equality ignoring case;
// those stream tests accept any non-idle state, including loading and fading.
// A matching buffer suppresses the stream fallback even when it is stopped.
// The first supplied query length is cached globally and gates all later queries.
int SoundManager_IsAliasPrefixOrStreamStemActive(void* self, const char* name) {
    signed char cached = (signed char)s_firstSoundQueryNameLengthInitialized;
    if (!cached) {
        s_firstSoundQueryNameLength = strlen(name);
        s_firstSoundQueryNameLengthInitialized = 1;
    }
    if (s_firstSoundQueryNameLength == 0)
        return 0;
    void* result = (void*)SoundManager_FindFirstBufferByAliasPrefixIgnoreCase(self, name);
    if (result != 0) {
        void* obj = AsSoundBufferPlayback(result)->soundBuffer;
        if (obj == 0)
            return 0;
        int flags;
        if (((gtl::ISoundBuffer*)obj)->GetStatusFlags(&flags) && (flags & kSoundStatusPlaying))
            return 1;
        return 0;
    }
    void* list = AsSoundManagerState(self)->state;
    if (SoundChannelGroup_IsSelectedStreamNonIdleWithMatchingStem(list, name))
        return 1;
    return SoundChannelGroup_IsSelectedStreamNonIdleWithMatchingStem(&AsSoundResources(list)->voice, name);
}

struct Z9B {
    char pad[4];
};
class Z9 : public Z9B {
public:
    virtual void v08(int);
};
// Release the buffer, bank resource and owned name, then optionally delete the node.
CSoundSourceListNode::~CSoundSourceListNode() {
    char* self = (char*)this;
    void* buffer = AsCSoundSourceListNode(self)->soundBuffer;
    AsCSoundSourceListNode(self)->nodeVtable = __vt__20CSoundSourceListNode;
    if (buffer != 0) {
        unsigned int refs = --*(unsigned int*)buffer;
        if (refs == 0 && buffer != 0) {
            ((Z9*)buffer)->v08(1);
        }
        AsCSoundSourceListNode(self)->soundBuffer = 0;
    }
    void* resource = AsCSoundSourceListNode(self)->soundBufferResource;
    if (resource != 0) {
        unsigned int refs = --*(unsigned int*)resource;
        if (refs == 0 && resource != 0) {
            ((Z9*)resource)->v08(1);
        }
        AsCSoundSourceListNode(self)->soundBufferResource = 0;
    }
    if ((self + 8) != 0 && (AsCSoundSourceListNode(self)->stringStorageFlags >> kHeapStringFlagShift))
        appFreeUni__FPv(AsCSoundSourceListNode(self)->stringHeap);
}

// Enable or suppress subsequent sound-play requests.
void SoundManager_SetEnabled(unsigned char* self, unsigned char enabled) {
    AsSoundManagerState(self)->enabled = enabled;
}

#pragma dont_inline on
// Return whether this buffer playback instance is active.
unsigned char SoundBufferPlayback_IsActive(unsigned char* playback) {
    return AsSoundBufferPlayback(playback)->active;
}
#pragma dont_inline off

struct SoundValuePairView {
    char pad00[4];
    float primaryValue;
    char pad08[4];
    float alternateValue;
};
// Select the primary or alternate float from a two-value record; callers determine their meaning.
float SoundValuePair_GetSelectedValue(SoundValuePairView* values, int useAlternate) {
    if (useAlternate)
        return values->alternateValue;
    return values->primaryValue;
}

// Return the supplied collection pointer unchanged.
void* SoundPlaybackList_GetCollection(void* value) {
    return value;
}

// Forward a lazy request to enable audio on all Wii remotes.
void RemoteAudio_RequestEnableAllLazyForwarder() {
    RemoteAudio_RequestEnableAllLazy();
}
// Initialize remote audio if needed, then disable every remote speaker.
void HomeButtonMenu_DisableRemoteAudio() {
    gtl_RemoteAudio_EnsureInitializedAndDisableAll();
}

// Map the Wii system sound setting to mono, stereo or surround, defaulting to stereo.
int SaveData_GetSystemSoundMode() {
    unsigned char value = SCGetSoundMode();
    switch (value) {
    case kSoundMono:
        return kSoundMono;
    case kSoundStereo:
        return kSoundStereo;
    case kSoundDolbyProLogicII:
        return kSoundDolbyProLogicII;
    default:
        return kSoundStereo;
    }
}

// Create or reuse a named buffer and play it when its playback-started flag permits.
void Sound_PlayByName(unsigned char* arg0, const char* name, const SoundPositionVector* positionVector,
                      unsigned int type, const char* image, const SoundPlaybackScale2* scale, float minimumGain,
                      float attenuationRange, float volumeScale) {
    if (AsSoundManagerState(arg0)->enabled == 0)
        return;
    unsigned char* data = SoundManager_CreateSoundInstance(arg0, name, positionVector, type, image, scale, minimumGain,
                                                           attenuationRange, volumeScale);
    if (data == 0)
        return;
    if (AsSoundBufferPlayback(data)->playbackStarted != 1)
        return;
    void* ptr = AsSoundBufferPlayback(data)->soundBuffer;
    if (ptr == 0)
        return;
    ((gtl::ISoundBuffer*)ptr)->Play(((AsSoundBufferPlayback(data)->playbackFlags & 1) != 0) ? kPlayLooped : kPlayOnce);
}

// Select the normal or alternate manager volume value.
float SoundManager_GetVolumeByMode(unsigned char* self, int useAlternate) {
    if (useAlternate)
        return ((FloatTween*)(self + kMusicVolumeTweenOffset))->target;
    return AsSoundManagerState(self)->volume;
}

// Pause both primary music slots and increment the nested pause count.
void SoundManager_PausePrimaryStreams(unsigned char* self) {
    unsigned int i = 0;
    unsigned char* data = (*(unsigned char**)&AsSoundManagerState(self)->state);
    for (; i < 2; ++i) {
        void* ptr = AsSoundChannelGroup(data)->channels[0].source;
        if (ptr != 0)
            ((gtl::ISoundStream*)ptr)->Pause();
        data += kStreamSlotBytes;
    }
    ++AsSoundManagerState(self)->pauseDepth;
}

// Resume primary music slots only when their nested pause count reaches zero.
int SoundManager_ResumePrimaryStreams(unsigned char* self) {
    int count = AsSoundManagerState(self)->pauseDepth - 1;
    AsSoundManagerState(self)->pauseDepth = count;
    if (count > 0)
        return 0;
    unsigned int i;
    unsigned char* data = (*(unsigned char**)&AsSoundManagerState(self)->state);
    AsSoundManagerState(self)->pauseDepth = 0;
    i = 0;
    for (; i < 2; ++i) {
        void* ptr = AsSoundChannelGroup(data)->channels[0].source;
        if (ptr != 0)
            ((gtl::ISoundStream*)ptr)->Resume();
        data += kStreamSlotBytes;
    }
    return 1;
}

// Replace the destination string with the caddie sound prefix followed by the supplied name.
int Sound_BuildCaddieBankName(const char* caddieName, void* destination) {
    if (caddieName == 0)
        return 0;
    const char* literal = s_soundNamePrefixes.caddie;
    unsigned int len;
    if (AsMWStringStorage(destination)->flags >> kHeapStringFlagShift)
        len = AsMWStringStorage(destination)->length;
    else
        len = *(unsigned char*)destination & kInlineStringLengthMask;
    int allocatorTagWord = 0;
    unsigned char allocatorTag = (unsigned char)allocatorTagWord;
    String_ReplaceRange(destination, 0, len, literal, literal + strlen(literal), &allocatorTag);
    unsigned int oldLen;
    if (AsMWStringStorage(destination)->flags >> kHeapStringFlagShift)
        oldLen = AsMWStringStorage(destination)->length;
    else
        oldLen = *(unsigned char*)destination & kInlineStringLengthMask;
    String_ReplaceRange(destination, oldLen, 0, caddieName, caddieName + strlen(caddieName), &allocatorTag);
    return 1;
}

// Build the SE_course_ bank name followed by courseName; return 0 only for a null name.
int SoundName_BuildCourseSePrefix(const char* courseName, void* destination) {
    if (courseName == 0)
        return 0;
    const char* literal = s_soundNamePrefixes.course;
    unsigned int len = *(unsigned char*)destination & kInlineStringLengthMask;
    if (AsMWStringStorage(destination)->flags >> kHeapStringFlagShift)
        len = AsMWStringStorage(destination)->length;
    unsigned char temp[0x18];
    temp[8] = temp[12];
    String_ReplaceRange(destination, 0, len, literal, literal + strlen(literal), temp + 8);
    unsigned int oldLen = *(unsigned char*)destination & kInlineStringLengthMask;
    if (AsMWStringStorage(destination)->flags >> kHeapStringFlagShift)
        oldLen = AsMWStringStorage(destination)->length;
    temp[0] = temp[4];
    String_ReplaceRange(destination, (int)oldLen, 0, courseName, courseName + strlen(courseName), temp);
    return 1;
}

// Construct the input string and initialize the seven cached path-component strings.
void* FilenameString_Ctor(void* self, const char* path) {
    unsigned int zero = 0;
    AsFilenameString(self)->input.flags = zero;
    AsFilenameString(self)->input.length = zero;
    (*(unsigned int*)&AsFilenameString(self)->input.data) = zero;
    unsigned int len = strlen(path);
    StdString_ReserveCapacity(self, len);
    unsigned char temp[0x8];
    temp[0] = temp[4];
    StdString_ReplaceRange(self, 0, 0, path, path + len, temp);
    AsFilenameString(self)->drive.flags = zero;
    AsFilenameString(self)->vtable = &__vt__14filenamestring;
    AsFilenameString(self)->drive.length = zero;
    (*(unsigned int*)&AsFilenameString(self)->drive.data) = zero;
    AsFilenameString(self)->directory.flags = zero;
    AsFilenameString(self)->directory.length = zero;
    (*(unsigned int*)&AsFilenameString(self)->directory.data) = zero;
    AsFilenameString(self)->stem.flags = zero;
    AsFilenameString(self)->stem.length = zero;
    (*(unsigned int*)&AsFilenameString(self)->stem.data) = zero;
    AsFilenameString(self)->extension.flags = zero;
    AsFilenameString(self)->extension.length = zero;
    (*(unsigned int*)&AsFilenameString(self)->extension.data) = zero;
    AsFilenameString(self)->driveDirectory.flags = zero;
    AsFilenameString(self)->driveDirectory.length = zero;
    (*(unsigned int*)&AsFilenameString(self)->driveDirectory.data) = zero;
    AsFilenameString(self)->unk_48.flags = zero;
    AsFilenameString(self)->unk_48.length = zero;
    (*(unsigned int*)&AsFilenameString(self)->unk_48.data) = zero;
    AsFilenameString(self)->fileName.flags = zero;
    AsFilenameString(self)->fileName.length = zero;
    (*(unsigned int*)&AsFilenameString(self)->fileName.data) = zero;
    return self;
}

// Scope lock; nested locking mirrors the recursive mutex protocol of these maps.
struct WInstanceLock {
    void* ptr;
    // Acquire the mutex at this helper's own address, preserving the original ABI path.
    WInstanceLock() : ptr(this) {
        OSLockMutex(ptr);
    }
    // Acquire the supplied map mutex for the current lexical scope.
    WInstanceLock(void* p) : ptr(p) {
        OSLockMutex(ptr);
    }
    // Release the map mutex when this scope exits.
    ~WInstanceLock() {
        OSUnlockMutex(ptr);
    }
};
struct PlaybackResetNode {
    void* owner;
    const char* key;
    unsigned char flags;
    char pad09[3];
    PlaybackResetNode* prev;
    PlaybackResetNode* next;
    PlaybackResetNode* bucketNext;
};
struct PlaybackResetMap {
    PlaybackResetNode* head;
    PlaybackResetNode* bucketHead;
    PlaybackResetNode* ignored08;
    PlaybackResetNode* cursor;
    void* ignored10;
    unsigned int mask;
    int buckets;
    PlaybackResetNode** table;
    char mutex[4];
};
// Choose a new circular-list head after removing the current node.
static inline PlaybackResetNode* ResetMap_NewHead(PlaybackResetNode* cur, PlaybackResetNode* h) {
    if (cur != h)
        return h;
    if (cur->next == h)
        return 0;
    return cur->next;
}
// Unlink one node under the recursive map mutex and return its successor.
static inline PlaybackResetNode* ResetMap_Unlink(PlaybackResetNode* cur, void* m) {
    OSLockMutex(m);
    cur->next->prev = cur->prev;
    cur->prev->next = cur->next;
    PlaybackResetNode* next = cur->next;
    if (next == cur)
        next = 0;
    OSUnlockMutex(m);
    return next;
}
// Insert a node before the circular-list head under the map mutex.
static inline PlaybackResetNode* ResetMap_Insert(PlaybackResetNode* cur, PlaybackResetNode* h, void* m) {
    OSLockMutex(m);
    if (!h) {
        cur->prev = cur;
        cur->next = cur;
        OSUnlockMutex(m);
        return cur;
    }
    cur->next = h;
    cur->prev = h->prev;
    h->prev = cur;
    cur->prev->next = cur;
    OSUnlockMutex(m);
    return h;
}
// Recycle active nodes into the free list and clear all hash buckets.
static inline void ResetMap_Clear(PlaybackResetMap* self) {
    WInstanceLock lock(self->mutex);
    if (self->head && self->cursor) {
        self->head->prev->next = self->cursor->next;
        self->cursor->next->prev = self->head->prev;
        self->cursor->next = self->head;
        self->head->prev = self->cursor;
        self->head = 0;
    } else if (self->head) {
        void* m = self->mutex;
        PlaybackResetNode* n;
        while ((n = self->head) != 0) {
            PlaybackResetNode* next = ResetMap_Unlink(n, m);
            PlaybackResetNode* h = self->head;
            PlaybackResetNode* free = self->cursor;
            h = ResetMap_Insert(h, free, m);
            self->cursor = h;
            self->head = next;
        }
    }
    for (int i = 0; i < self->buckets; ++i)
        self->table[i] = 0;
}
struct A33Fill {
    unsigned char c;
};
// Read string length using the playback-record and group-origin addresses.
static inline unsigned int ResetMap_StringLength(unsigned char* src, int off, unsigned char* item) {
    if ((*(unsigned int*)src >> kHeapStringFlagShift) == 0)
        return src[0] & kInlineStringLengthMask;
    return *(unsigned int*)(item + off);
}
// Replace a stream string; groupOrigin is four bytes before the playback record.
static inline void ResetMap_AssignString(unsigned char* it, unsigned char* p, int off, const char* lit) {
    A33Fill fill;
    unsigned int cap;
    if ((*(unsigned int*)(p + off) >> kHeapStringFlagShift) == 0)
        cap = p[off] & kInlineStringLengthMask;
    else
        cap = *(unsigned int*)(it + off + 8);
    A33Fill copy = fill;
    String_ReplaceRange(p + off, 0, cap, lit, lit + strlen(lit), &copy.c);
}
// Reset stream flags, strings, fade values and default gains.
static inline void ResetMap_ResetStream(unsigned char* it, unsigned char* p, float one, float zero) {
    AsSoundChannelGroup(it)->channels[0].source = 0;
    AsSoundChannelGroup(it)->channels[0].volume = one;
    AsSoundChannelGroup(it)->channels[0].pitch = one;
    AsSoundChannelGroup(it)->channels[0].playState = 0;
    ResetMap_AssignString(it, p, 0x10, s_soundPlaybackEmptyName);
    (*(char*)&AsSoundChannelGroup(it)->channels[0].loopEnabled) = 0;
    (*(char*)&AsSoundChannelGroup(it)->channels[0].fadeInEnabled) = 0;
    (*(char*)&AsSoundChannelGroup(it)->channels[0].useDolbyBank) = 0;
    AsSoundChannelGroup(it)->channels[0].transitionDuration = zero;
    ResetMap_AssignString(it, p, 0x40, s_soundPlaybackEmptyFilePath);
    ResetMap_AssignString(it, p, 0x4c, s_soundPlaybackEmptyResourcePath);
    FloatTween_Start(p + 0x24, kFadeDefault, zero, zero);
    AsSoundChannelGroup(it)->channels[0].baseVolume = one;
    AsSoundChannelGroup(it)->channels[0].basePitch = one;
    String_ReserveCapacity(p + 0x10, 0x40);
    String_ReserveCapacity(p + 0x40, 0x10);
    String_ReserveCapacity(p + 0x4c, 0x10);
}
// Advance the buffer-list cursor under its mutex and return the current value.
static inline void* ResetMap_NextBuffer(unsigned char* data, void* lk) {
    OSLockMutex(lk);
    void* node = *(void**)(data + kBufferListCursorOffset);
    if (node != 0) {
        void* next = ((SoundListLink*)node)->next;
        void* item = *(void**)node;
        if (next == *(void**)(data + kBufferListHeadOffset))
            next = 0;
        *(void**)(data + kBufferListCursorOffset) = next;
        OSUnlockMutex(lk);
        return item;
    }
    OSUnlockMutex(lk);
    return 0;
}

// Reset stream groups and resource lists, then restore output mode and volume from saved options.
void SoundManager_ResetPlaybackState(unsigned char* self) {
    unsigned int i;
    unsigned char* items = (*(unsigned char**)&AsSoundManagerState(self)->state);
    AsSoundChannelGroup(items)->selected = 0;

    {
        unsigned char* src;
        float one;

        one = kFloat_1_SoundPlayback;

        float zero;
        zero = kFloat_0_soundPlayback;

        unsigned char* item;
        src = (4 + items);
        item = items;
        for (i = 0; 2 > i; ++i, item += kStreamSlotBytes, src += kStreamSlotBytes) {

            PlaybackAtSlotOrigin(item)->source = 0;
            PlaybackAtSlotOrigin(item)->volume = one;
            PlaybackAtSlotOrigin(item)->pitch = one;
            PlaybackAtSlotOrigin(item)->playState = kStreamIdle;
            ResetMap_AssignString(item, src, kPlaybackNameOffset, s_soundPlaybackEmptyName);
            PlaybackAtSlotOrigin(item)->loopEnabled = 0;
            PlaybackAtSlotOrigin(item)->fadeInEnabled = 0;

            PlaybackAtSlotOrigin(item)->useDolbyBank = 0;
            PlaybackAtSlotOrigin(item)->transitionDuration = zero;
            ResetMap_AssignString(item, src, kPlaybackFilePathOffset, s_soundPlaybackEmptyFilePath);
            ResetMap_AssignString(item, src, kPlaybackResourcePathOffset, s_soundPlaybackEmptyResourcePath);
            FloatTween_Start(&AsSoundPlaybackState(src)->fadeMode, kFadeDefault, kFloat_0_soundPlayback,
                             kFloat_0_soundPlayback);
            PlaybackAtSlotOrigin(item)->baseVolume = one;
            PlaybackAtSlotOrigin(item)->basePitch = one;
            String_ReserveCapacity(&AsSoundPlaybackState(src)->nameFlags, kPlaybackNameReserveCharacters);
            String_ReserveCapacity(&AsSoundPlaybackState(src)->filePathFlags, kPlaybackPathReserveCharacters);
            String_ReserveCapacity(&AsSoundPlaybackState(src)->resourcePathFlags, kPlaybackPathReserveCharacters);
        }
    }
    AsSoundResources(items)->voice.selected = 0;
    {
        float one;
        float zf;
        unsigned char* src;
        zf = kFloat_0_soundPlayback;

        unsigned char* item;
        item = items + kVoiceGroupOffset;
        src = (unsigned char*)&AsSoundResources(items)->voice.channels[0];
        one = kFloat_1_SoundPlayback;
        for (i = 0; 2 > i; ++i, item += kStreamSlotBytes, src += kStreamSlotBytes) {

            PlaybackAtSlotOrigin(item)->source = 0;
            PlaybackAtSlotOrigin(item)->pitch = one;
            PlaybackAtSlotOrigin(item)->volume = one;
            PlaybackAtSlotOrigin(item)->playState = kStreamIdle;
            ResetMap_AssignString(item, src, kPlaybackNameOffset, s_soundPlaybackEmptyName);

            PlaybackAtSlotOrigin(item)->fadeInEnabled = 0;
            PlaybackAtSlotOrigin(item)->loopEnabled = 0;
            PlaybackAtSlotOrigin(item)->useDolbyBank = 0;
            PlaybackAtSlotOrigin(item)->transitionDuration = zf;

            ResetMap_AssignString(item, src, kPlaybackFilePathOffset, s_soundPlaybackEmptyFilePath);
            ResetMap_AssignString(item, src, kPlaybackResourcePathOffset, s_soundPlaybackEmptyResourcePath);
            FloatTween_Start(&AsSoundPlaybackState(src)->fadeMode, kFadeDefault, kFloat_0_soundPlayback,
                             kFloat_0_soundPlayback);
            PlaybackAtSlotOrigin(item)->basePitch = one;
            PlaybackAtSlotOrigin(item)->baseVolume = one;
            String_ReserveCapacity(&AsSoundPlaybackState(src)->nameFlags, kPlaybackNameReserveCharacters);
            String_ReserveCapacity(&AsSoundPlaybackState(src)->filePathFlags, kPlaybackPathReserveCharacters);
            String_ReserveCapacity(&AsSoundPlaybackState(src)->resourcePathFlags, kPlaybackPathReserveCharacters);
        }
    }
    ResetMap_Clear((PlaybackResetMap*)(items + kBufferListHeadOffset));
    ResetMap_Clear((PlaybackResetMap*)(items + kBankListHeadOffset));
    FloatTween_Start(self, kFadeDefault, kFloat_1_SoundPlayback, kFloat_0_soundPlayback);
    FloatTween_Start(self + kMusicVolumeTweenOffset, kFadeDefault, kFloat_1_SoundPlayback, kFloat_0_soundPlayback);

    float w160 = kFloat_160_SoundManager;
    float h32 = kFloat_32_SoundManager;
    AsSoundManagerState(self)->outputMode = 1;
    AsSoundManagerState(self)->transitionDuration = kFloat_1_SoundPlayback;
    AsSoundManagerState(self)->enabled = 1;
    AsSoundManagerState(self)->playbackPauseDepth = 0;

    AsSoundManagerState(self)->unk_50 = w160; /* initialized to 160; later use is not established here */
    (*(unsigned int*)&AsSoundManagerState(self)->pauseDepth) = 0;
    AsSoundManagerState(self)->unk_54 =
        kFloat_960_SoundManager; // initialized to 960; later use is not established here
    AsSoundManagerState(self)->unk_4C = (w160 - h32);
    if (!((int&)GetSaveDataSlot())) {
        if (0 == s_saveDataInitialized) {
            SaveData_Ctor((void*)s_saveData);
            s_saveDataInitialized = 1;
        }
        ((int&)GetSaveDataSlot()) = (int)s_saveData;
    }
    unsigned char* manager = (unsigned char*)SaveData_GetCurrentRecord((void*)((int&)GetSaveDataSlot()));
    SoundManager_SetOutputMode(self, (((SavedAudioOptionsView*)manager)->optionBits >> kSavedSoundModeShift) &
                                         kSavedSoundModeMask);

    FloatTween_Start(self, kFadeDefault,
                     ((float)((SavedAudioOptionsView*)manager)->effectVolumePercent / kFloat_100_SoundManager),
                     kFloat_0_soundPlayback);
    {

        unsigned char* data = (*(unsigned char**)&AsSoundManagerState(self)->state);
        void* item;
        float factor = AsSoundManagerState(self)->volumeScale;
        {
            WInstanceLock first(data + kBufferListMutexOffset);
            *(void**)(data + kBufferListCursorOffset) = *(void**)(data + kBufferListHeadOffset);
            void* lk = data + kBufferListMutexOffset;
            item = ResetMap_NextBuffer(data, lk);
        }
        if (item) {
            void* lk = data + kBufferListMutexOffset;
            while (0 != item) {
                float value = AsSoundBufferPlayback(item)->volume;
                if (0 != item) {
                    if (AsSoundBufferPlayback(item)->soundBuffer != 0) {
                        AsSoundBufferPlayback(item)->volume = value;
                        ((gtl::ISoundBuffer*)(AsSoundBufferPlayback(item)->soundBuffer))
                            ->SetVolume(factor * (AsSoundBufferPlayback(item)->baseVolume * value));
                    }
                }
                item = ResetMap_NextBuffer(data, lk);
            }
        }
    }
    float ratio2 = (float)((SavedAudioOptionsView*)manager)->musicVolumePercent / kFloat_100_SoundManager;
    FloatTween_Start(self + kMusicVolumeTweenOffset, kFadeDefault, ratio2, kFloat_0_soundPlayback);
    {
        unsigned char* data = (*(unsigned char**)&AsSoundManagerState(self)->state);

        int idx = *(int*)data;

        unsigned char* e = data + idx * kStreamSlotBytes;
        *(float*)(e + 0x08) = (AsSoundManagerState(self)->volume);
        if (*(void**)(e + 0x04) != 0) {
            ((gtl::ISoundStream*)(*(void**)(e + 0x04)))
                ->SetVolume(*(float*)(e + 0x60) * *(float*)(e + 0x08) * *(float*)(e + 0x2c));
        }
    }
    Audio_SetStreamVolumeWithFade(ratio2, kFloat_0_soundPlayback);
}
struct DtorRef {
    int refs;
    virtual ~DtorRef();
};
struct DtorOwner {
    char pad00[0x80];
    DtorRef* ref;
};
struct DtorNode {
    DtorOwner* owner;
    const char* key;
    unsigned char flags;
    char pad09[3];
    DtorNode* prev;
    DtorNode* next;
    DtorNode* bucketNext;
};
struct DtorMap {
    // Three independent traversals: table[]/bucketNext is the name-hash chain;
    // head/prev/next is the active ring; freeHead/prev/next is the reusable-node ring.
    // iterationNext points to the next active entry; it is separate from hash buckets.
    DtorNode* head;
    DtorNode* iterationNext;
    DtorNode* ignored08;
    DtorNode* freeHead;
    void* ignored10;
    unsigned int mask;
    int buckets;
    DtorNode** table;
    char mutex[0x10];
    void* lock;
};
// Reset and advance the map cursor, returning the first buffered sound.
static inline DtorOwner* BufferMap_First(void* m, DtorMap* self) {
    WInstanceLock a(m);
    self->iterationNext = self->head;
    OSLockMutex(m);
    DtorNode* n = self->iterationNext;
    DtorOwner* owner;
    if (n) {
        DtorNode* next = n->next;
        owner = n->owner;
        self->iterationNext = next == self->head ? 0 : next;
        OSUnlockMutex(m);
    } else {
        OSUnlockMutex(m);
        owner = 0;
    }
    return owner;
}
// Remove a node from the bucket selected by its name hash.
static inline void BufferMap_Unbucket(DtorMap* o, DtorNode* cur) {
    WInstanceLock bl(o->mutex);
    void* m = o->mutex;
    const unsigned char* s = (const unsigned char*)cur->key;
    OSLockMutex(m);
    int len = strlen((const char*)s);
    unsigned int hi = (s[0] & 0x2a) | (s[len > 0 ? len - 1 : 0] & 0x15);
    unsigned int hash = (len << 6) | hi;
    unsigned int mask = o->mask;
    unsigned int idx = hash & mask;
    OSUnlockMutex(m);
    DtorNode** b = o->table;
    DtorNode* p = b[idx];
    if (p == cur)
        b[idx] = cur->bucketNext;
    else
        while (p) {
            if (p->bucketNext == cur) {
                p->bucketNext = cur->bucketNext;
                return;
            }
            p = p->bucketNext;
        }
}
// Select the remaining circular-list head after removing a node.
static inline DtorNode* BufferMap_NewHead(DtorNode* cur, DtorNode* h) {
    if (cur != h)
        return h;
    if (cur->next == h)
        return 0;
    return cur->next;
}
// Unlink a circular-list node while holding its map mutex.
static inline DtorNode* BufferMap_Unlink(DtorNode* cur, void* m) {
    OSLockMutex(m);
    cur->next->prev = cur->prev;
    cur->prev->next = cur->next;
    DtorNode* next = cur->next;
    if (next == cur)
        next = 0;
    OSUnlockMutex(m);
    return next;
}
// Insert a detached map node into the circular free list.
static inline DtorNode* BufferMap_InsertFree(DtorNode* cur, DtorNode* h, void* m) {
    OSLockMutex(m);
    if (!h) {
        cur->prev = cur;
        cur->next = cur;
        OSUnlockMutex(m);
        return cur;
    }
    cur->next = h;
    cur->prev = h->prev;
    h->prev = cur;
    cur->prev->next = cur;
    OSUnlockMutex(m);
    return h;
}
// Unhash a node, free an owned key and recycle the node, maintaining the cursor.
// This consumes the node/key membership only; the caller still owns cur->owner.
static inline void BufferMap_Remove(DtorMap* o, DtorNode* cur, DtorNode* head, int zero) {
    DtorNode* it = o->iterationNext;
    if (it == cur)
        o->iterationNext = it->next == head ? 0 : it->next;
    if (cur->key) {
        BufferMap_Unbucket(o, cur);
        if (cur->flags)
            appFreeUni__FPv((void*)cur->key);
    }
    DtorNode* h = o->head;
    void* m = o->mutex;
    OSLockMutex(m);
    cur->next->prev = cur->prev;
    cur->prev->next = cur->next;
    h = BufferMap_NewHead(cur, h);
    OSUnlockMutex(m);
    o->head = h;
    m = o->mutex;
    h = o->freeHead;
    OSLockMutex(m);
    if (!h) {
        cur->prev = cur;
        cur->next = cur;
        OSUnlockMutex(m);
    } else {
        cur->next = h;
        cur->prev = h->prev;
        h->prev = cur;
        cur->prev->next = cur;
        OSUnlockMutex(m);
        cur = h;
    }
    o->freeHead = cur;
    if (!o->head)
        o->iterationNext = (DtorNode*)zero;
}
// Find and remove the map node holding the requested sound object.
static inline void BufferMap_RemoveValue(DtorMap* o, DtorOwner* owner, int zero) {
    WInstanceLock scan(o->mutex);
    DtorNode* head = o->head;
    DtorNode* cur = head;
    if (!cur)
        return;
    do {
        if (cur->owner == owner) {
            BufferMap_Remove(o, cur, head, zero);
            return;
        }
        cur = cur->next;
    } while (cur != head);
}
// Move all active nodes to the free list and clear hash buckets.
// Clear recycles nodes, without stopping sound references or freeing owner wrappers.
// Those operations belong to BufferMap_StopSound and the outer teardown loop.
static inline void BufferMap_Clear(DtorMap* self) {
    WInstanceLock lock(self->mutex);
    if (self->head && self->freeHead) {
        self->head->prev->next = self->freeHead->next;
        self->freeHead->next->prev = self->head->prev;
        self->freeHead->next = self->head;
        self->head->prev = self->freeHead;
        self->head = 0;
    } else if (self->head) {
        void* m = self->mutex;
        DtorNode* n;
        while ((n = self->head) != 0) {
            DtorNode* next = BufferMap_Unlink(n, m);
            DtorNode* h = self->head;
            DtorNode* free = self->freeHead;
            h = BufferMap_InsertFree(h, free, m);
            self->freeHead = h;
            self->head = next;
        }
    }
    for (int i = 0; i < self->buckets; ++i)
        self->table[i] = 0;
}
#pragma use_lmw_stmw on
// Stop a buffer, release its sound reference and clear the reference field.
static inline void BufferMap_StopSound(DtorOwner* node) {
    if (node->ref) {
        ((gtl::ISoundBuffer*)(node->ref))->Stop(0);
        DtorRef* p = node->ref;
        if (p) {
            if (--p->refs == 0)
                delete p;
            node->ref = 0;
        }
    }
}
#pragma use_lmw_stmw reset
void SoundChannelGroup_ResetStreamSlots(void*);
void* SoundPlaybackGroup_dtor(void*, int);
struct NamedSoundReferenceMap;
struct PlaybackResourceMap;
void SoundBufferList_StopAndClear(DtorMap*);
void SoundManager_ReleaseAllNamedObjectReferences(NamedSoundReferenceMap*);
void* PlaybackResourceMap_dtor(PlaybackResourceMap*, int);
DtorMap* BufferMap_dtor(DtorMap*, int);

struct W780Node {
    DtorOwner* owner;
    const char* key;
    unsigned char flags;
    char pad09[3];
    W780Node* prev;
    W780Node* next;
    W780Node* bucketNext;
};
struct W780State {
    char pad000[0x198];
    W780Node* head;
    W780Node* cursor;
    char pad1a0[4];
    W780Node* tail;
    char pad1a8[4];
    unsigned int mask;
    int count;
    W780Node** table;
    char mutex[0x10];
};

struct W780Fill {
    unsigned char c;
};
// Destructor-path string reset with its original allocator-tag lifetime.
static inline void ManagerDtor_AssignString(unsigned char* it, unsigned char* p, int off, const char* lit) {
    W780Fill fill;
    unsigned int cap;
    if ((*(unsigned int*)(p + off) >> kHeapStringFlagShift) == 0)
        cap = p[off] & kInlineStringLengthMask;
    else
        cap = *(unsigned int*)(it + off + 8);
    W780Fill copy = fill;
    String_ReplaceRange(p + off, 0, cap, lit, lit + strlen(lit), &copy.c);
}

// Reset one released slot in all four destructor sweeps.
// A macro preserves caller-local inlining order; the equivalent inline helper
// changes register allocation in BufferMap_Clear earlier in SoundManager_Dtor.
#define RESET_MANAGER_DTOR_SLOT(item, src, one, zf)                                                            \
    do {                                                                                                       \
        PlaybackAtSlotOrigin(item)->source = 0;                                                                \
        PlaybackAtSlotOrigin(item)->volume = one;                                                              \
        PlaybackAtSlotOrigin(item)->pitch = one;                                                               \
        PlaybackAtSlotOrigin(item)->playState = kStreamIdle;                                                   \
        ManagerDtor_AssignString(item, src, kPlaybackNameOffset, s_soundPlaybackEmptyName);                    \
        PlaybackAtSlotOrigin(item)->loopEnabled = 0;                                                           \
        PlaybackAtSlotOrigin(item)->fadeInEnabled = 0;                                                         \
        PlaybackAtSlotOrigin(item)->useDolbyBank = 0;                                                          \
        PlaybackAtSlotOrigin(item)->transitionDuration = zf;                                                   \
        ManagerDtor_AssignString(item, src, kPlaybackFilePathOffset, s_soundPlaybackEmptyFilePath);            \
        ManagerDtor_AssignString(item, src, kPlaybackResourcePathOffset, s_soundPlaybackEmptyResourcePath);    \
        FloatTween_Start(&AsSoundPlaybackState(src)->fadeMode, kFadeDefault, kFloat_0_soundPlayback,           \
                         kFloat_0_soundPlayback);                                                              \
        PlaybackAtSlotOrigin(item)->baseVolume = one;                                                          \
        PlaybackAtSlotOrigin(item)->basePitch = one;                                                           \
        String_ReserveCapacity(&AsSoundPlaybackState(src)->nameFlags, kPlaybackNameReserveCharacters);         \
        String_ReserveCapacity(&AsSoundPlaybackState(src)->filePathFlags, kPlaybackPathReserveCharacters);     \
        String_ReserveCapacity(&AsSoundPlaybackState(src)->resourcePathFlags, kPlaybackPathReserveCharacters); \
    } while (0)

// Stop and release the stream and its shared-file handle.
static inline void ManagerDtor_ReleaseStream(unsigned char* it) {
    void* rc = AsSoundChannelGroup(it)->channels[0].source;
    if (rc) {
        ((gtl::ISoundStream*)rc)->Stop(0);
        int* r = (*(int**)&AsSoundChannelGroup(it)->channels[0].source);
        if (--*r == 0 && r != 0) {
            void** vtable = ((SoundResourceHeader*)r)->vtable;
            ((void (*)(void*, int))vtable[2])(r, 1);
        }
    }
    if (AsSoundChannelGroup(it)->channels[0].sharedFile) {
        CSharedFileHandle::Destroy(AsSoundChannelGroup(it)->channels[0].sharedFile, 1);
        AsSoundChannelGroup(it)->channels[0].sharedFile = 0;
    }
}

// Stop and release both stream groups, buffer instances and sound banks, then destroy manager storage.
// Address map for the repeated slot loops: group origin contains selected at +0;
// channels begin at +4 with stride 0x64. item = groupOrigin + i*0x64 is a SHIFTED
// origin; PlaybackAtSlotOrigin(item) returns the actual SoundPlaybackState.
// src = item+4 is that slot's SoundPlaybackState base. Thus src+0x10/0x40/0x4C
// are name/filePath/resourcePath strings; src+0x24 is the fade tween.
// Raw item[0x20..0x22] correspond to slot loop/fade-in/Dolby flags at +0x1C..0x1E.
// Reserves are character capacities (64/16/16); they do not change string length.
void* SoundManager_Dtor(void* selfArg, int deleting) {
    unsigned char* self = (unsigned char*)selfArg;
    if (self) {
        unsigned int i;

        // Phase 1: stop/remove buffered sound payloads, then clear their map.
        DtorMap* map;
        void* m;
        DtorOwner* node;

        map = (DtorMap*)((*(unsigned char**)&AsSoundManagerState(self)->state) + kBufferListHeadOffset);
        m = map->mutex;
        for (;;) {
            {
                WInstanceLock nextLock(m);
                map->iterationNext = map->head;
                OSLockMutex(m);
                DtorNode* n = map->iterationNext;
                if (!(!(!(!(n))))) {
                    // Preserve next-link alias lifetimes across the inlined lock cleanup.
                    __typeof__(n->next) observedNext = n->next;
                    __typeof__(n->next) savedNext = observedNext;
                    __typeof__(n->next) cleanupNext = savedNext;
                    __typeof__(n->owner) savedOwner = n->owner;
                    DtorNode* next = n->next;
                    map->iterationNext = next == map->head ? 0 : next;
                    node = savedOwner;
                    OSUnlockMutex(m);
                } else {
                    OSUnlockMutex(m);
                    node = 0;
                }
            }
            if (node == 0)
                break;
            if (node) {
                BufferMap_StopSound(node);
                BufferMap_RemoveValue(map, node, 0);
                if (node)
                    __dl__FPv(node);
            }
        }
        BufferMap_Clear(map);
        {
            // Phase 2: primary/music group, two slots. First release live streams
            // and files/reset fields, then reset selected index and initialize again.
            // The repeated second sweep is existing behavior, including reserves.
            unsigned char* itemsA = (*(unsigned char**)&AsSoundManagerState(self)->state);
            {
                float one;
                unsigned char* src;
                unsigned char* item;
                item = itemsA;
                one = 1.0f;
                src = itemsA + 4;
                float zf;
                zf = kFloat_0_soundPlayback;
                for (i = 0; i < 2; ++i, item += kStreamSlotBytes, src += kStreamSlotBytes) {
                    if (PlaybackAtSlotOrigin(item)->source) {
                        ((gtl::ISoundStream*)(PlaybackAtSlotOrigin(item)->source))->Stop(0);
                        DtorRef* rc = (*(DtorRef**)&PlaybackAtSlotOrigin(item)->source);
                        if (--rc->refs == 0)
                            delete rc;
                    }
                    if (PlaybackAtSlotOrigin(item)->sharedFile) {
                        CSharedFileHandle::Destroy(PlaybackAtSlotOrigin(item)->sharedFile, 1);
                        PlaybackAtSlotOrigin(item)->sharedFile = 0;
                    }
                    RESET_MANAGER_DTOR_SLOT(item, src, one, zf);
                }
            }
            float zf;
            unsigned char* src;
            // Reset the group's selected-slot index; second sweep owns no live stream.
            *(int*)itemsA = 0;
            unsigned char* item;
            float one;
            one = 1.0f;
            item = itemsA;
            zf = kFloat_0_soundPlayback;
            src = (4 + itemsA);
            for (i = 0; 2 > i; ++i, item += kStreamSlotBytes, src += kStreamSlotBytes) {
                RESET_MANAGER_DTOR_SLOT(item, src, one, zf);
            }
        }
        {
            float one;
            // Phase 3: repeat both sweeps for the voice group at +0xCC.
            unsigned char* itemsB = (*(unsigned char**)&AsSoundManagerState(self)->state) + kVoiceGroupOffset;
            {
                float zf;
                float one;
                unsigned char* src;
                unsigned char* item;
                item = itemsB;
                one = 1.0f;
                src = (4 + itemsB);
                zf = kFloat_0_soundPlayback;
                for (i = 0; 2 > i; ++i, item += kStreamSlotBytes, src += kStreamSlotBytes) {
                    if (PlaybackAtSlotOrigin(item)->source) {
                        ((gtl::ISoundStream*)(PlaybackAtSlotOrigin(item)->source))->Stop(0);
                        DtorRef* rc = (*(DtorRef**)&PlaybackAtSlotOrigin(item)->source);
                        if (--rc->refs == 0)
                            delete rc;
                    }
                    if (PlaybackAtSlotOrigin(item)->sharedFile) {
                        CSharedFileHandle::Destroy(PlaybackAtSlotOrigin(item)->sharedFile, 1);
                        PlaybackAtSlotOrigin(item)->sharedFile = 0;
                    }
                    RESET_MANAGER_DTOR_SLOT(item, src, one, zf);
                }
            }
            *(int*)itemsB = 0;

            unsigned char* src;
            unsigned char* item;
            one = 1.0f;
            float zf;
            item = itemsB;
            zf = kFloat_0_soundPlayback;
            src = itemsB + 4;
            for (i = 0; 2 > i; ++i, item += kStreamSlotBytes, src += kStreamSlotBytes) {
                RESET_MANAGER_DTOR_SLOT(item, src, one, zf);
            }
        }
        // Phase 4: destroy initialized slot/map storage, then the shared state block.
        // deleting > 0 additionally frees self; the return value remains selfArg.
        unsigned char* data = (*(unsigned char**)&AsSoundManagerState(self)->state);
        if (data) {
            SoundChannelGroup_ResetStreamSlots(data);
            SoundChannelGroup_ResetStreamSlots(data + kVoiceGroupOffset);
            SoundBufferList_StopAndClear((DtorMap*)(data + kBufferListHeadOffset));
            SoundManager_ReleaseAllNamedObjectReferences((NamedSoundReferenceMap*)(data + kBankListHeadOffset));
            PlaybackResourceMap_dtor((PlaybackResourceMap*)(data + kBankListHeadOffset), -1);
            BufferMap_dtor((DtorMap*)(data + kBufferListHeadOffset), -1);
            SoundPlaybackGroup_dtor(data + kVoiceGroupOffset, -1);
            SoundPlaybackGroup_dtor(data, -1);
            __dl__FPv(data);
        }
        (*(int*)&AsSoundManagerState(self)->state) = 0;
        if ((deleting) > 0)
            __dl__FPv(self);
    }
    return selfArg;
}

#pragma use_lmw_stmw on
#undef RESET_MANAGER_DTOR_SLOT
// Release buffered sounds, pooled map nodes and hash buckets, then optionally delete the map.
DtorMap* BufferMap_dtor(DtorMap* self, int deleting) {
    if (self) {
        DtorOwner* node;
        int zero = 0;
        void* m = self->mutex;
        for (;;) {
            node = BufferMap_First(m, self);
            if (!node)
                break;
            if (node) {
                BufferMap_StopSound(node);
                BufferMap_RemoveValue(self, node, zero);
                if (node)
                    __dl__FPv(node);
            }
        }
        BufferMap_Clear(self);
        if (self) {
            WInstanceLock baseLock(self->mutex);
            if (self->ignored08) {
                void* baseMutex = self->mutex;
                DtorNode* n;
                while ((n = self->ignored08) != 0) {
                    self->ignored08 = BufferMap_Unlink(n, baseMutex);
                    appFreeUni__FPv(n);
                }
            }
            if (self->table) {
                appFreeUni__FPv(self->table);
                self->table = 0;
            }
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}
#pragma use_lmw_stmw reset

// Read a stream string length using the group-origin fallback word.
static inline unsigned int ClearChannels_StringLength(unsigned char* src, int off, unsigned char* item) {
    if ((*(unsigned int*)src >> kHeapStringFlagShift) == 0)
        return src[0] & kInlineStringLengthMask;
    return *(int*)(item + off);
}
// Destroy buffered sounds and reset both voice slots while retaining the map allocation.
void SoundManager_StopAndClearChannels(void* selfArg) {
    DtorOwner* node;
    DtorMap* self = (DtorMap*)((*(unsigned char**)&AsSoundManagerState(selfArg)->state) + kBufferListHeadOffset);

    void* m = self->mutex;
    for (;;) {
        node = BufferMap_First(m, self);
        if (0 == node)
            break;
        if (node) {
            BufferMap_StopSound(node);
            int zero = 0;
            BufferMap_RemoveValue(self, node, zero);
            if (node)
                __dl__FPv(node);
        }
    }
    BufferMap_Clear(self);
    unsigned char* voiceGroupOrigin = (*(unsigned char**)&AsSoundManagerState(selfArg)->state) + kVoiceGroupOffset;
    float unitMultiplier = kFloat_1_SoundPlayback;
    unsigned char* playbackBytes = kPlaybackRecordFromSlotOrigin + voiceGroupOrigin;
    unsigned char* slotOrigin = voiceGroupOrigin;
    float zero = kFloat_0_soundPlayback;
    // Release shared files and initialize both playback records; slotOrigin is a slot origin, never a group.
    for (unsigned int i = 0; kPlaybackSlotsPerGroup > i;
         ++i, slotOrigin += kStreamSlotBytes, playbackBytes += kStreamSlotBytes) {
        reset_rc((void**)playbackBytes);
        if (PlaybackAtSlotOrigin(slotOrigin)->sharedFile) {
            CSharedFileHandle::Destroy(PlaybackAtSlotOrigin(slotOrigin)->sharedFile, 1);
            PlaybackAtSlotOrigin(slotOrigin)->sharedFile = 0;
        }
        PlaybackAtSlotOrigin(slotOrigin)->source = 0;
        PlaybackAtSlotOrigin(slotOrigin)->volume = unitMultiplier;
        PlaybackAtSlotOrigin(slotOrigin)->pitch = unitMultiplier;
        PlaybackAtSlotOrigin(slotOrigin)->playState = kStreamIdle;
        ResetMap_AssignString(slotOrigin, playbackBytes, kPlaybackNameOffset, s_soundPlaybackEmptyName);
        PlaybackAtSlotOrigin(slotOrigin)->loopEnabled = 0;
        PlaybackAtSlotOrigin(slotOrigin)->fadeInEnabled = 0;
        PlaybackAtSlotOrigin(slotOrigin)->useDolbyBank = 0;
        PlaybackAtSlotOrigin(slotOrigin)->transitionDuration = zero;
        ResetMap_AssignString(slotOrigin, playbackBytes, kPlaybackFilePathOffset, s_soundPlaybackEmptyFilePath);

        ResetMap_AssignString(slotOrigin, playbackBytes, kPlaybackResourcePathOffset, s_soundPlaybackEmptyResourcePath);
        FloatTween_Start(playbackBytes + kPlaybackFadeOffset, kFadeDefault, kFloat_0_soundPlayback,
                         kFloat_0_soundPlayback);
        PlaybackAtSlotOrigin(slotOrigin)->baseVolume = unitMultiplier;
        PlaybackAtSlotOrigin(slotOrigin)->basePitch = unitMultiplier;
        String_ReserveCapacity(playbackBytes + kPlaybackNameOffset, kPlaybackNameReserveCharacters);
        String_ReserveCapacity(playbackBytes + kPlaybackFilePathOffset, kPlaybackPathReserveCharacters);
        String_ReserveCapacity(playbackBytes + kPlaybackResourcePathOffset, kPlaybackPathReserveCharacters);
    }
    // Repeat the scalar/string initialization after resetting the real group selection.
    slotOrigin = voiceGroupOrigin;
    AsSoundChannelGroup(voiceGroupOrigin)->selected = 0;
    playbackBytes = (voiceGroupOrigin + kPlaybackRecordFromSlotOrigin);
    for (unsigned int i = 0; i < kPlaybackSlotsPerGroup;
         ++i, slotOrigin += kStreamSlotBytes, playbackBytes += kStreamSlotBytes) {

        PlaybackAtSlotOrigin(slotOrigin)->source = 0;
        PlaybackAtSlotOrigin(slotOrigin)->volume = kFloat_1_SoundPlayback;
        PlaybackAtSlotOrigin(slotOrigin)->pitch = kFloat_1_SoundPlayback;
        PlaybackAtSlotOrigin(slotOrigin)->playState = kStreamIdle;
        ResetMap_AssignString(slotOrigin, playbackBytes, kPlaybackNameOffset, s_soundPlaybackEmptyName);
        PlaybackAtSlotOrigin(slotOrigin)->loopEnabled = 0;
        PlaybackAtSlotOrigin(slotOrigin)->fadeInEnabled = 0;
        PlaybackAtSlotOrigin(slotOrigin)->useDolbyBank = 0;
        PlaybackAtSlotOrigin(slotOrigin)->transitionDuration = kFloat_0_soundPlayback;
        ResetMap_AssignString(slotOrigin, playbackBytes, kPlaybackFilePathOffset, s_soundPlaybackEmptyFilePath);
        ResetMap_AssignString(slotOrigin, playbackBytes, kPlaybackResourcePathOffset, s_soundPlaybackEmptyResourcePath);
        FloatTween_Start(playbackBytes + kPlaybackFadeOffset, kFadeDefault, kFloat_0_soundPlayback,
                         kFloat_0_soundPlayback);
        PlaybackAtSlotOrigin(slotOrigin)->basePitch = kFloat_1_SoundPlayback;
        String_ReserveCapacity(playbackBytes + kPlaybackNameOffset, kPlaybackNameReserveCharacters);
        String_ReserveCapacity(playbackBytes + kPlaybackFilePathOffset, kPlaybackPathReserveCharacters);
        PlaybackAtSlotOrigin(slotOrigin)->baseVolume = kFloat_1_SoundPlayback;
        String_ReserveCapacity(playbackBytes + kPlaybackResourcePathOffset, kPlaybackPathReserveCharacters);
    }
}

struct Map802A8CNode {
    Map802A8CNode* owner;
    char pad04[4];
    unsigned char flags;
    char pad09[3];
    Map802A8CNode* prev;
    Map802A8CNode* next;
    Map802A8CNode* bucketNext;
    const char* key;
};
struct Map802A8C {
    Map802A8CNode* head;
    Map802A8CNode* bucketHead;
    char pad08[4];
    Map802A8CNode* cursor;
    char pad10[4];
    unsigned int mask;
    unsigned int buckets;
    Map802A8CNode** table;
    WInstanceLock lock;
};

#pragma use_lmw_stmw on
// Unlink a node and return the surviving circular-list head.
static inline DtorNode* ClearBuffers_Unlink(DtorNode* cur, DtorNode* h, void* m) {
    OSLockMutex(m);
    cur->next->prev = cur->prev;
    cur->prev->next = cur->next;
    h = BufferMap_NewHead(cur, h);
    OSUnlockMutex(m);
    return h;
}
// Find a buffered sound, remove its hash entry and recycle the list node.
static inline void ClearBuffers_RemoveValue(DtorOwner* owner, DtorMap* o, int zero) {
    WInstanceLock scan(o->mutex);
    DtorNode* head = o->head;
    DtorNode* cur = head;
    if (!cur)
        return;
    do {
        if (cur->owner == owner) {
            DtorNode* it = o->iterationNext;
            if (it == cur)
                o->iterationNext = it->next == head ? 0 : it->next;
            if (cur->key) {
                BufferMap_Unbucket(o, cur);
                if (cur->flags)
                    appFreeUni__FPv((void*)cur->key);
            }
            DtorNode* h = o->head;
            void* m = o->mutex;
            h = ClearBuffers_Unlink(cur, h, m);
            o->head = h;
            void* insertMutex = o->mutex;
            DtorNode* free = o->freeHead;
            OSLockMutex(insertMutex);
            if (!free) {
                cur->prev = cur;
                cur->next = cur;
                OSUnlockMutex(insertMutex);
            } else {
                cur->next = free;
                cur->prev = free->prev;
                free->prev = cur;
                cur->prev->next = cur;
                OSUnlockMutex(insertMutex);
                cur = free;
            }
            o->freeHead = cur;
            if (!o->head)
                o->iterationNext = (DtorNode*)zero;
            return;
        }
        cur = cur->next;
    } while (cur != head);
}
// Recycle remaining active entries and clear hash buckets.
static inline void ClearBuffers_ClearMap(DtorMap* self) {
    WInstanceLock lock(self->mutex);
    if (self->head && self->freeHead) {
        self->head->prev->next = self->freeHead->next;
        self->freeHead->next->prev = self->head->prev;
        self->freeHead->next = self->head;
        self->head->prev = self->freeHead;
        self->head = 0;
    } else if (self->head) {
        void* m = self->mutex;
        DtorNode* n;
        while ((n = self->head) != 0) {
            DtorNode* next = BufferMap_Unlink(n, m);
            DtorNode* h = self->head;
            DtorNode* free = self->freeHead;
            h = BufferMap_InsertFree(h, free, m);
            self->freeHead = h;
            self->head = next;
        }
    }
    for (int i = 0; i < self->buckets; ++i)
        self->table[i] = 0;
}
// Stop and delete every buffered sound and recycle its map entry.
void SoundBufferList_StopAndClear(DtorMap* self) {
    void* m = self->mutex;
    int zero = 0;
    DtorOwner* node;
    for (;;) {
        node = BufferMap_First(m, self);
        if (!node)
            break;
        if (node) {
            BufferMap_StopSound(node);
            ClearBuffers_RemoveValue(node, self, zero);
            if (node)
                __dl__FPv(node);
        }
    }
    ClearBuffers_ClearMap(self);
}
#pragma use_lmw_stmw reset
// Borrowed circular map links: node +00 is the playback payload, +10 is next.
// head is a node address, not an integer playback value. cursor==0 ends iteration;
// reaching head wraps once and clears cursor. Lock at +20 protects cursor updates.
struct PlaybackIterationNode {
    PlaybackIterationNode* payload;
    char pad04[0x0c];
    PlaybackIterationNode* iterationLink;
};
struct PlaybackIterationList {
    PlaybackIterationNode* head;
    PlaybackIterationNode* cursor;
    char pad08[0x18];
    WInstanceLock lock;
};

// Return the next playback object and advance the mutex-protected circular-list cursor.
// Returned payload is NOT retained. Caller owns iteration lifetime: removing
// nodes/payloads after unlocking must obey the map's cursor-before-delete rule.
void* SoundPlaybackList_NextIteration(PlaybackIterationList* list) {

    void* lock = &list->lock;
    OSLockMutex(lock);
    PlaybackIterationNode* node = list->cursor;
    if (node) {
        // Keep the three next-link aliases and two head aliases in this order:
        // their expression lifetimes affect the inlined UpdatePlayback exception
        // entry. They borrow one link/head, never additional nodes or owners.
        __typeof__(node->iterationLink) observedNext = node->iterationLink;
        __typeof__(node->iterationLink) savedNext = observedNext;
        __typeof__(node->iterationLink) cleanupNext = savedNext;
        PlaybackIterationNode* result = node->payload;
        __typeof__(list->head) observedHead = list->head;
        __typeof__(list->head) head = observedHead;
        PlaybackIterationNode* next = node->iterationLink;
        list->cursor = next == head ? 0 : next;
        OSUnlockMutex(lock);
        return result;
    }
    OSUnlockMutex(lock);
    return 0;
}

struct NamedSoundReference {
    int refcount;
    virtual ~NamedSoundReference();
};
struct NamedSoundReferenceNode {
    NamedSoundReference* owner;
    const char* key;
    unsigned char flags;
    char pad09[3];
    NamedSoundReferenceNode* prev;
    NamedSoundReferenceNode* next;
    NamedSoundReferenceNode* bucketNext;
};
struct NamedSoundReferenceMap {
    NamedSoundReferenceNode* head;
    NamedSoundReferenceNode* iter;
    char pad08[4];
    NamedSoundReferenceNode* free;
    char pad10[4];
    unsigned int mask;
    char pad18[4];
    NamedSoundReferenceNode** table;
    char mutex[4];
};

// Remove a named-reference entry from its name hash chain.
static inline void NamedReferences_Unbucket(NamedSoundReferenceMap* o, NamedSoundReferenceNode* cur) {
    WInstanceLock bl(o->mutex);
    void* m = o->mutex;
    const unsigned char* s = (const unsigned char*)cur->key;
    OSLockMutex(m);
    int len = strlen((const char*)s);
    unsigned int hi = (s[0] & 0x2a) | (s[len > 0 ? len - 1 : 0] & 0x15);
    unsigned int hash = (len << 6) | hi;
    unsigned int mask = o->mask;
    unsigned int idx = hash & mask;
    OSUnlockMutex(m);
    NamedSoundReferenceNode** b = o->table;
    NamedSoundReferenceNode* p = b[idx];
    if (p == cur) {
        b[idx] = cur->bucketNext;
    } else {
        while (p) {
            if (p->bucketNext == cur) {
                p->bucketNext = cur->bucketNext;
                return;
            }
            p = p->bucketNext;
        }
    }
}

// Choose the head after removing a named-reference entry.
static inline NamedSoundReferenceNode* NamedReferences_NewHead(NamedSoundReferenceNode* cur,
                                                               NamedSoundReferenceNode* h) {
    if (cur != h)
        return h;
    if (cur->next == h)
        return 0;
    return cur->next;
}

// Return a named-reference entry to the free list.
static inline NamedSoundReferenceNode* NamedReferences_InsertFree(NamedSoundReferenceMap* o,
                                                                  NamedSoundReferenceNode* cur) {
    void* m = o->mutex;
    NamedSoundReferenceNode* f = o->free;
    OSLockMutex(m);
    if (!f) {
        cur->prev = cur;
        cur->next = cur;
        OSUnlockMutex(m);
        return cur;
    }
    cur->next = f;
    cur->prev = f->prev;
    f->prev = cur;
    cur->prev->next = cur;
    OSUnlockMutex(m);
    return f;
}

// Remove a named-reference entry and release its owned name.
static inline void NamedReferences_Remove(NamedSoundReferenceMap* o, NamedSoundReferenceNode* cur,
                                          NamedSoundReferenceNode* head, int zero) {
    NamedSoundReferenceNode* it = o->iter;
    if (it == cur)
        o->iter = it->next == head ? 0 : it->next;
    if (cur->key) {
        NamedReferences_Unbucket(o, cur);
        if (cur->flags)
            appFreeUni__FPv((void*)cur->key);
    }
    NamedSoundReferenceNode* h;
    void* m = o->mutex;
    h = o->head;
    OSLockMutex(m);
    cur->next->prev = cur->prev;
    cur->prev->next = cur->next;
    h = NamedReferences_NewHead(cur, h);
    OSUnlockMutex(m);
    o->head = h;
    m = o->mutex;
    h = o->free;
    OSLockMutex(m);
    if (!h) {
        cur->prev = cur;
        cur->next = cur;
        OSUnlockMutex(m);
    } else {
        cur->next = h;
        cur->prev = h->prev;
        h->prev = cur;
        cur->prev->next = cur;
        OSUnlockMutex(m);
        cur = h;
    }
    o->free = cur;
    if (!o->head)
        o->iter = (NamedSoundReferenceNode*)zero;
}

// Reset the iterator and return the first named object under the mutex.
static inline NamedSoundReference* NamedReferences_First(NamedSoundReferenceMap* o, void* m) {
    WInstanceLock outer(m);
    o->iter = o->head;
    OSLockMutex(m);
    NamedSoundReferenceNode* node = o->iter;
    NamedSoundReference* owner;
    if (node) {
        NamedSoundReferenceNode* next = node->next;
        owner = node->owner;
        o->iter = next == o->head ? 0 : next;
        OSUnlockMutex(m);
    } else {
        OSUnlockMutex(m);
        owner = 0;
    }
    return owner;
}

// Find the entry for an object and remove it from the named map.
static inline void NamedReferences_RemoveValue(NamedSoundReferenceMap* o, void* m, NamedSoundReference* owner,
                                               int zero) {
    WInstanceLock scan(m);
    NamedSoundReferenceNode* head = o->head;
    NamedSoundReferenceNode* cur = head;
    if (!cur)
        return;
    do {
        if (cur->owner == owner) {
            NamedReferences_Remove(o, cur, head, zero);
            return;
        }
        cur = cur->next;
    } while (cur != head);
}

#pragma use_lmw_stmw on
// Remove each named object and release all its remaining references.
void SoundManager_ReleaseAllNamedObjectReferences(NamedSoundReferenceMap* arg0) {
    int zero = 0;
    void* m = arg0->mutex;
    for (;;) {
        NamedSoundReference* owner = NamedReferences_First(arg0, m);
        if (!owner)
            return;
        NamedReferences_RemoveValue(arg0, m, owner, zero);
        int rc;
        do {
            if (--owner->refcount == 0) {
                delete owner;
                rc = 0;
            } else
                rc = owner->refcount;
        } while (rc);
    }
}
#pragma use_lmw_stmw reset

// Reset the circular-list cursor and return its first playback object under the mutex.
void* SoundPlaybackList_BeginIteration(PlaybackIterationList* arg0) {
    PlaybackIterationNode* result;
    void* lock = &arg0->lock;
    WInstanceLock outer(lock);
    arg0->cursor = arg0->head;
    OSLockMutex(lock);
    if (arg0->cursor) {
        // Same borrowed link aliases as NextIteration; retain their inline lifetimes.
        __typeof__(arg0->cursor->iterationLink) observedNext = arg0->cursor->iterationLink;
        __typeof__(arg0->cursor->iterationLink) savedNext = observedNext;
        __typeof__(arg0->cursor->iterationLink) cleanupNext = savedNext;
        result = arg0->cursor->payload;
        PlaybackIterationNode* nextNode = arg0->cursor->iterationLink;
        arg0->cursor = nextNode == arg0->head ? 0 : nextNode;
        OSUnlockMutex(lock);
    } else {
        OSUnlockMutex(lock);
        result = 0;
    }
    return result;
}

#pragma dont_inline on
// Test whether any requested playback flag is set.
int SoundPlayback_HasAnyFlags(unsigned char* playback, unsigned int mask) {
    return (AsSoundBufferPlayback(playback)->playbackFlags & mask) != 0;
}
#pragma dont_inline off

// Apply pitch times the resource pitch scale to the selected music stream.
void SoundManager_SetStreamPitch(unsigned char* self, float pitch) {
    unsigned char (*data)[kStreamSlotBytes] =
        (unsigned char (*)[kStreamSlotBytes])(*(unsigned char**)&AsSoundManagerState(self)->state);
    unsigned char* entry = (unsigned char*)data[*(int*)data];
    AsSoundChannelGroup(entry)->channels[0].pitch = pitch;
    if (AsSoundChannelGroup(entry)->channels[0].source == 0)
        return;
    ((gtl::ISoundStream*)(AsSoundChannelGroup(entry)->channels[0].source))
        ->SetPitch(AsSoundChannelGroup(entry)->channels[0].basePitch * AsSoundChannelGroup(entry)->channels[0].pitch);
}

#pragma peephole off
// Report whether either primary music slot has a non-idle playback state.
int SoundManager_HasActiveMusicSlot(unsigned char* self) {
    unsigned char* data = (*(unsigned char**)&AsSoundManagerState(self)->state);
    if (AsSoundResources(data)->music.channels[0].playState != 0 ||
        AsSoundResources(data)->music.channels[1].playState != 0)
        return 1;
    return 0;
}
#pragma peephole reset

#pragma dont_inline on
// Apply manager volume, resource gain and current fade value to the selected music stream.
void SoundManager_ApplyStreamVolume(unsigned char* self) {
    int idx;
    unsigned char* data = (*(unsigned char**)&AsSoundManagerState(self)->state);
    float f0 = AsSoundManagerState(self)->volume;
    idx = *(int*)data;
    data = data + idx * kStreamSlotBytes;
    AsSoundChannelGroup(data)->channels[0].volume = f0;
    if (AsSoundChannelGroup(data)->channels[0].source == 0)
        return;
    float f1 = AsSoundChannelGroup(data)->channels[0].baseVolume;
    f0 = AsSoundChannelGroup(data)->channels[0].volume;
    float f2 = AsSoundChannelGroup(data)->channels[0].fadeValue;
    f0 = f1 * f0;
    f1 = f0 * f2;
    ((gtl::ISoundStream*)(AsSoundChannelGroup(data)->channels[0].source))->SetVolume(f1);
}
#pragma dont_inline off

// Scale manager music volume before applying resource gain and the current fade value.
void SoundManager_ScaleStreamVolume(unsigned char* self, float volumeMultiplier) {
    unsigned char* data = (*(unsigned char**)&AsSoundManagerState(self)->state);
    float f0 = AsSoundManagerState(self)->volume;
    int idx = *(int*)data;
    f0 = volumeMultiplier * f0;
    data += idx * kStreamSlotBytes;
    AsSoundChannelGroup(data)->channels[0].volume = f0;
    if (AsSoundChannelGroup(data)->channels[0].source == 0)
        return;
    float f1 = AsSoundChannelGroup(data)->channels[0].baseVolume;
    f0 = AsSoundChannelGroup(data)->channels[0].volume;
    float f2 = AsSoundChannelGroup(data)->channels[0].fadeValue;
    f0 = f1 * f0;
    f1 = f0 * f2;
    ((gtl::ISoundStream*)(AsSoundChannelGroup(data)->channels[0].source))->SetVolume(f1);
}

// Start a music-volume transition, applying an immediate change directly to the selected stream.
void SoundManager_SetStreamVolume(unsigned char* self, float volume, float fadeSeconds) {
    float zero = kFloat_0_soundPlayback;
    if (fadeSeconds > zero) {
        FloatTween_Start(self + kMusicVolumeTweenOffset, kFadeDefault, volume, fadeSeconds);
    } else {
        FloatTween_Start(self + kMusicVolumeTweenOffset, kFadeDefault, volume, zero);
        unsigned char* data = (*(unsigned char**)&AsSoundManagerState(self)->state);
        data += *(int*)data * kStreamSlotBytes;
        AsSoundChannelGroup(data)->channels[0].volume = AsSoundManagerState(self)->volume;
        if ((*(int*)&AsSoundChannelGroup(data)->channels[0].source) != 0) {
            float f1 = AsSoundChannelGroup(data)->channels[0].baseVolume;
            float f0 = AsSoundChannelGroup(data)->channels[0].volume;
            float f2 = AsSoundChannelGroup(data)->channels[0].fadeValue;
            f0 = f0 * f1;
            f1 = f0 * f2;
            ((gtl::ISoundStream*)(AsSoundChannelGroup(data)->channels[0].source))->SetVolume(f1);
        }
    }
    Audio_SetStreamVolumeWithFade(volume, fadeSeconds);
}

struct PlaybackNodeLink {
    char pad00[0x0c];
    PlaybackNodeLink* prev;
    PlaybackNodeLink* next;
};
enum {
    kSoundMapLockOffset = 0x20
}; // map mutex follows its bucket pointer
struct PlaybackNodeList {
    char pad00[8];
    PlaybackNodeLink* head;
    char pad0c[0x10];
    void* buckets;
    char lock[4];
};

// Free pooled nodes and the hash bucket table, optionally deleting the collection.
void* dtor_802B3A20(PlaybackNodeList* arg0, int arg1) {
    if (arg0 != 0) {
        {
            WInstanceLock outer((char*)arg0 + kSoundMapLockOffset);
            PlaybackNodeLink* node = arg0->head;
            while (node != 0) {
                void* lock = (char*)arg0 + kSoundMapLockOffset;
                OSLockMutex(lock);
                PlaybackNodeLink* prev = node->prev;
                PlaybackNodeLink* next = node->next;
                next->prev = prev;
                prev->next = next;
                PlaybackNodeLink* following = node->next;
                if (node == following)
                    following = 0;
                OSUnlockMutex(lock);
                arg0->head = following;
                appFreeUni__FPv(node);
                node = arg0->head;
            }
            if (arg0->buckets != 0) {
                appFreeUni__FPv(arg0->buckets);
                arg0->buckets = 0;
            }
        }
        if (arg1 > 0)
            __dl__FPv(arg0);
    }
    return arg0;
}

// Free pooled nodes and the hash bucket table using this collection cleanup variant.
void* dtor_802B3C28(PlaybackNodeList* arg0, int arg1) {
    if (arg0 != 0) {
        {
            WInstanceLock outer((char*)arg0 + kSoundMapLockOffset);
            PlaybackNodeLink* node = arg0->head;
            while (node != 0) {
                void* lock = (char*)arg0 + kSoundMapLockOffset;
                OSLockMutex(lock);
                PlaybackNodeLink* next = node->next;
                PlaybackNodeLink* prev = node->prev;
                next->prev = prev;
                prev->next = next;
                PlaybackNodeLink* following = node->next;
                if (node == following)
                    following = 0;
                OSUnlockMutex(lock);
                arg0->head = following;
                appFreeUni__FPv(node);
                node = arg0->head;
            }
            if (arg0->buckets != 0) {
                appFreeUni__FPv(arg0->buckets);
                arg0->buckets = 0;
            }
        }
        if (arg1 > 0)
            __dl__FPv(arg0);
    }
    return arg0;
}

// Read the destination string length for avatar sound-name construction.
static inline unsigned int AvatarName_StringLength(void* p) {
    return ((AsMWStringStorage(p)->flags >> kHeapStringFlagShift) == 0) ? (*(unsigned char*)p & kInlineStringLengthMask)
                                                                        : AsMWStringStorage(p)->length;
}

// Build an avatar sound name from one of eight avatar-name table entries.
int SoundName_BuildAvatarPrefix(unsigned int index, void* destination) {
    if (index >= kAvatarVoiceCount)
        return 0;
    const char* literal = s_soundNamePrefixes.avatar;
    unsigned int len = AvatarName_StringLength(destination);
    unsigned char temp[0x10];
    temp[8] = temp[12];
    String_ReplaceRange(destination, 0, len, literal, literal + strlen(literal), temp + 8);
    const char* avatarName = ((const char**)&s_avatarVoiceNames)[index];
    unsigned int oldLen = AvatarName_StringLength(destination);
    temp[0] = temp[4];
    String_ReplaceRange(destination, (int)oldLen, 0, avatarName, avatarName + strlen(avatarName), temp);
    return 1;
}

struct FilenameOwnedStringView {
    unsigned int flags;
    unsigned int len;
    void* data;
};

struct IterNode {
    IterNode* item;
    char pad04[0x0c];
    IterNode* next;
};
struct IterOwner {
    char pad000[0x198];
    IterNode* head;
    IterNode* cursor;
    char pad1a0[0x18];
    WInstanceLock lock;
};

struct SoundBufferInstanceView {
    char name[0x40];
    char alias[0x20];
    float position[3];
    unsigned char spatialized;
    unsigned char playbackStarted;
    unsigned char active;
    char pad06f;
    unsigned int playbackFlags;
    float minDistance, maxDistance;
    unsigned int reserved;
    void* soundBuffer;
    float volume, baseVolume, basePitch;
};
typedef char SoundBufferInstance_size[(sizeof(SoundBufferInstanceView) == 0x90) ? 1 : -1];
struct SoundObjectListNode {
    SoundBufferInstanceView* item;
    unsigned int value;
    unsigned char flag;
    char pad09[3];
    SoundObjectListNode* prev;
    SoundObjectListNode* next;
    char pad14[4];
};
struct SoundObjectList {
    SoundObjectListNode* head;
    SoundObjectListNode* cursor;
    SoundObjectListNode* chunks;
    SoundObjectListNode* free;
    int chunkSize;
    char pad14[0xc];
    char lock[4];
    char pad24[0x14];
};
struct SoundObjectLists {
    char pad000[0x198];
    SoundObjectList list;
    SoundObjectList second;
};
struct SoundResourcePathString {
    unsigned int word;
    unsigned int len;
    char* heap;
};
struct SoundResourceAllocatorTag {
    char c;
};
struct SoundBufferDescription {
    unsigned int w[7];
};
struct SoundBufferReference {
    int refs;
    void** vt;
};

extern "C" {
void Vec3_Zero(void*);
void __as__7WVectorFRC7WVector(void*, const SoundPositionVector*);
float Magnitude__7WVectorCFv(const SoundPositionVector*);
void strncpy(void*, const void*, int);
void* appMallocUni__FUl(unsigned int);
void* __nw__FUl(unsigned int);
int gtl_Audio_CreateSoundBuffer(void*, void*);
void* memset(void*, int, unsigned int);
extern const SoundBufferDescription kDefaultSoundBufferDescription;
extern char s_soundResourceNameSeparator[4];
extern const float kFloat_1000_SoundAttenuation;
}

// Return a list value and advance its circular-list cursor under the mutex.
static inline SoundObjectListNode* SoundBankList_Next(SoundObjectList* l) {
    void* lk = l->lock;
    OSLockMutex(lk);
    SoundObjectListNode* node = l->cursor;
    if (node != 0) {
        SoundObjectListNode* next = node->next;
        SoundObjectListNode* item = (SoundObjectListNode*)node->item;
        if (next == l->head)
            next = 0;
        l->cursor = next;
        OSUnlockMutex(lk);
        return item;
    }
    OSUnlockMutex(lk);
    return 0;
}

// Reset the list cursor and retrieve the first sound or bank object.
static inline SoundObjectListNode* SoundBankList_First(SoundObjectList* l) {
    WInstanceLock outer(l->lock);
    l->cursor = l->head;
    return SoundBankList_Next(l);
}

// Insert a node before a circular-list head under the supplied mutex.
static inline SoundObjectListNode* SoundBankList_Insert(SoundObjectListNode* head, SoundObjectListNode* n, void* lk) {
    OSLockMutex(lk);
    if (head == 0) {
        n->prev = n;
        n->next = n;
        OSUnlockMutex(lk);
        return n;
    }
    n->next = head;
    n->prev = head->prev;
    head->prev = n;
    n->prev->next = n;
    OSUnlockMutex(lk);
    return head;
}

// Find the first sound instance whose name begins with the supplied text.
static inline SoundBufferInstanceView* SoundInstance_FindReusable(unsigned char* self, const char* name) {
    SoundObjectLists* owner = (*(SoundObjectLists**)&AsSoundManagerState(self)->state);
    if (name == 0)
        return 0;
    int len = strlen(name);
    if (len == 0)
        return 0;
    SoundBufferInstanceView* item = (SoundBufferInstanceView*)SoundBankList_First(&owner->list);
    while (item != 0) {
        if (String_ComparePrefixIgnoreCase(item, name, len) == 0)
            return item;
        item = (SoundBufferInstanceView*)SoundBankList_Next(&owner->list);
    }
    return 0;
}

// Release heap storage for a temporary resource path.
static inline void SoundResource_FreePath(SoundResourcePathString& s) {
    if (s.word >> kHeapStringFlagShift)
        appFreeUni__FPv(s.heap);
}

// Read the inline or heap length of a temporary resource path.
static inline unsigned int SoundResource_PathLength(SoundResourcePathString& s) {
    return !(s.word >> kHeapStringFlagShift) ? (*(unsigned char*)&s & kInlineStringLengthMask) : s.len;
}

// Append the resource-name separator and search loaded banks for a matching entry.
static inline int SoundResource_FindBankEntry(unsigned char* self, const char* name, void*& outColl,
                                              unsigned int& outIndex) {
    SoundObjectLists* owner = (*(SoundObjectLists**)&AsSoundManagerState(self)->state);
    SoundResourcePathString str = {};
    EH5StringCleanup eh5Cleanup41(&str);
    SoundResourceAllocatorTag t0;
    unsigned int size = SoundResource_PathLength(str);
    SoundResourceAllocatorTag t1 = t0;
    String_ReplaceRange(&str, 0, size, name, name + strlen(name), (unsigned char*)&t1);
    SoundResourceAllocatorTag t2;
    size = SoundResource_PathLength(str);
    SoundResourceAllocatorTag t3 = t2;
    String_ReplaceRange(&str, size, 0, s_soundResourceNameSeparator,
                        s_soundResourceNameSeparator + strlen(s_soundResourceNameSeparator), (unsigned char*)&t3);
    unsigned int isLong = str.word >> kHeapStringFlagShift;
    const char* path = !isLong ? (char*)&str + 1 : str.heap;
    int pathLen = strlen(path);
    if (pathLen == 0) {
        if (isLong) {
            eh5Cleanup41.release();
            appFreeUni__FPv(str.heap);
        }
        return 0;
    }
    SoundObjectListNode* item = SoundBankList_First(&owner->second);
    while (item != 0) {
        void* coll = AsCSoundSourceListNode(item)->soundBufferResource;
        unsigned int count = ((unsigned int (*)(void*))(((SoundResourceHeader*)coll)->vtable)[3])(coll);
        for (unsigned int i = 0; i < count; i++) {
            ((const char* (*)(void*, unsigned int))(((SoundResourceHeader*)coll)->vtable)[4])(coll, i);
            const char* entry =
                ((const char* (*)(void*, unsigned int))(((SoundResourceHeader*)coll)->vtable)[4])(coll, i);
            if (String_ComparePrefixIgnoreCase((void*)path, entry, pathLen) == 0) {
                outColl = coll;
                outIndex = i;
                {
                    eh5Cleanup41.release();
                    SoundResource_FreePath(str);
                }
                return 1;
            }
        }
        item = SoundBankList_Next(&owner->second);
    }
    {
        eh5Cleanup41.release();
        SoundResource_FreePath(str);
    }
    return 0;
}

// Allocate a node chunk if needed, then detach one instance-list free node.
static inline SoundObjectListNode* SoundInstance_AllocateNode(SoundObjectLists* owner) {
    WInstanceLock outer(owner->list.lock);
    if (owner->list.free == 0) {
        SoundObjectListNode* chunk = (SoundObjectListNode*)appMallocUni__FUl(owner->list.chunkSize * 0x18);
        void* lk1 = owner->list.lock;
        owner->list.chunks = SoundBankList_Insert(owner->list.chunks, chunk, lk1);
        int i = 1;
        if (owner->list.chunkSize > 1) {
            void* lk2 = owner->list.lock;
            for (SoundObjectListNode* p = chunk + 1; i < owner->list.chunkSize; p++, i++) {
                owner->list.free = SoundBankList_Insert(owner->list.free, p, lk2);
            }
        }
    }
    SoundObjectListNode* n = owner->list.free;
    void* lk = owner->list.lock;
    OSLockMutex(lk);
    n->next->prev = n->prev;
    n->prev->next = n->next;
    SoundObjectListNode* next = n->next;
    if (next == n)
        next = 0;
    OSUnlockMutex(lk);
    owner->list.free = next;
    return n;
}

// Create a bank-backed buffer, initialize its gains and attenuation, and add it to the instance list.
static inline SoundBufferInstanceView* SoundInstance_Create(unsigned char* self, void* coll, unsigned int index,
                                                            const char* name, const char* image) {
    SoundObjectLists* owner = (*(SoundObjectLists**)&AsSoundManagerState(self)->state);
    // Slot 6 (gtl_CGSPSoundBufferResource_CopyBaseParams) copies the whole 40-byte base-parameter block; only
    // volume/pitch are read.
    SoundPlaybackScale2 size;
    SoundBufferDescription desc = kDefaultSoundBufferDescription;
    SoundBufferReference* ref = 0;
    if (!gtl_Audio_CreateSoundBuffer(&desc, &ref))
        return 0;
    if (!((int (*)(void*, unsigned int, void*))(((SoundResourceHeader*)coll)->vtable)[5])(coll, index, ref)) {
        SoundBufferReference* r = ref;
        if (r != 0) {
            if (--r->refs == 0 && r != 0)
                ((void (*)(void*, int))r->vt[2])(r, 1);
            ref = 0;
        }
        return 0;
    }
    SoundBufferInstanceView* item = (SoundBufferInstanceView*)__nw__FUl(0x90);
    memset(item->name, 0, 0x40);
    memset(item->alias, 0, 0x20);
    Vec3_Zero(item->position);
    item->active = 0;
    item->playbackStarted = 0;
    item->spatialized = 0;
    item->playbackFlags = 0;
    item->reserved = 0;
    item->soundBuffer = 0;
    item->volume = 1.0f;
    item->baseVolume = 1.0f;
    item->basePitch = 1.0f;
    item->minDistance = 100.0f;
    item->maxDistance = 1000.0f;
    item->soundBuffer = ref;
    if (((int (*)(void*, unsigned int, SoundPlaybackScale2*))(((SoundResourceHeader*)coll)->vtable)[6])(coll, index,
                                                                                                        &size)) {
        item->baseVolume = size.x;
        item->basePitch = size.y;
    }
    if (name)
        strncpy(item->name, name, 0x40);
    if (image)
        strncpy(item->alias, image, 0x20);
    {
        WInstanceLock outer(owner->list.lock);
        SoundObjectListNode* n = SoundInstance_AllocateNode(owner);
        n->item = item;
        n->flag = 0;
        n->value = 0;
        void* lk = owner->list.lock;
        owner->list.head = SoundBankList_Insert(owner->list.head, n, lk);
    }
    return item;
}

// Reuse or create a sound buffer, configure spatial attenuation and gains, and mark it active.
// name/alias and vector/distance inputs are borrowed; the returned instance remains
// owned by the manager's buffer list. Null means lookup/allocation failed.
// type 5 requires a non-null vector and nonzero attenuationRange: it computes
// max(minimumGain, 1 - length(positionVector)/attenuationRange), then volumeScale.
// Other types use a supplied world position for spatial playback; a null position
// selects nonspatial playback. scale, when present, supplies min/max distances.
// These three float parameters are gains/range, not elapsed time or playback rate.
extern "C" unsigned char* SoundManager_CreateSoundInstance(unsigned char* self, const char* name,
                                                           const SoundPositionVector* positionVector, unsigned int type,
                                                           const char* image, const SoundPlaybackScale2* scale,
                                                           float minimumGain, float attenuationRange,
                                                           float volumeScale) {
    SoundBufferInstanceView* item = SoundInstance_FindReusable(self, name);
    if (item != 0 && (item->active == 0 || item->playbackStarted == 1)) {
        if (item->soundBuffer != 0) {
            int flag;
            if (((gtl::ISoundBuffer*)(item->soundBuffer))->GetStatusFlags(&flag) && (flag & kSoundStatusPlaying))
                item = 0;
        }
    } else {
        item = 0;
    }
    if (item == 0) {
        void* coll;
        unsigned int index;
        if (!SoundResource_FindBankEntry(self, name, coll, index))
            return 0;
        item = SoundInstance_Create(self, coll, index, name, image);
        if (item == 0)
            return 0;
    }
    item->playbackFlags = type;
    item->active = 1;
    item->volume = 1.0f;
    item->minDistance = 100.0f;
    item->maxDistance = 1000.0f;
    if (positionVector != 0 && type != 5) {
        __as__7WVectorFRC7WVector(item->position, positionVector);
        item->spatialized = 1;
        item->playbackStarted = 0;
        if (scale) {
            item->minDistance = scale->x;
            item->maxDistance = scale->y;
        }
        if (item != 0 && item->soundBuffer != 0) {
            ((gtl::ISoundBuffer*)(item->soundBuffer))->SetMinDistance(1, item->minDistance);
            ((gtl::ISoundBuffer*)(item->soundBuffer))->SetMaxDistance(1, item->maxDistance);
        }
        if (item != 0 && item->soundBuffer != 0)
            ((gtl::ISoundBuffer*)(item->soundBuffer))->SetSpatialMode(1, 1);
    } else {
        item->spatialized = 0;
        item->playbackStarted = 1;
        if (type == 5) {
            float ratio = (attenuationRange - Magnitude__7WVectorCFv(positionVector)) / attenuationRange;
            item->volume *= *(minimumGain < ratio ? &ratio : &minimumGain);
            item->playbackFlags = 0;
        }
        if (item != 0 && item->soundBuffer != 0)
            ((gtl::ISoundBuffer*)(item->soundBuffer))->SetSpatialMode(2, 1);
    }
    float out = item->volume * volumeScale;
    float factor = *(float*)(self + 4);
    if (item != 0 && item->soundBuffer != 0) {
        item->volume = out;
        ((gtl::ISoundBuffer*)(item->soundBuffer))->SetVolume(factor * (item->baseVolume * out));
    }
    if (item != 0 && item->soundBuffer != 0)
        ((void (*)(void*, float))SetPitch__Q23gtl12ISoundBufferFf)(item->soundBuffer,
                                                                   item->basePitch * kFloat_1_SoundPlayback);
    return (unsigned char*)item;
}

// Return the buffer cursor value and advance to the next node under the mutex.
static inline void* BufferIterator_Next(unsigned char* data) {
    void* item;
    void* lk = data + kBufferListMutexOffset;
    OSLockMutex(lk);
    void* node = *(void**)(data + kBufferListCursorOffset);
    if (node != 0) {
        void* next = ((SoundListLink*)node)->next;
        item = *(void**)node;
        if (next == *(void**)(data + kBufferListHeadOffset))
            next = 0;
        *(void**)(data + kBufferListCursorOffset) = next;
        OSUnlockMutex(lk);
        return item;
    }
    OSUnlockMutex(lk);
    return 0;
}

// Reset the buffer iterator to the list head and return its first value.
static inline void* BufferIterator_First(unsigned char* data) {
    WInstanceLock outer(data + kBufferListMutexOffset);
    *(void**)(data + kBufferListCursorOffset) = *(void**)(data + kBufferListHeadOffset);
    return BufferIterator_Next(data);
}

// Find the first buffer whose alias begins with the requested name, ignoring case.
void* SoundManager_FindFirstBufferByAliasPrefixIgnoreCase(void* selfArg, const char* name) {
    unsigned char* self = (unsigned char*)selfArg;
    unsigned char* data = (*(unsigned char**)&AsSoundManagerState(self)->state);
    if (name == 0)
        return 0;
    int length = strlen(name);
    if (length == 0)
        return 0;
    void* item;
    {
        WInstanceLock outer(data + kBufferListMutexOffset);
        *(void**)(data + kBufferListCursorOffset) = *(void**)(data + kBufferListHeadOffset);
        item = BufferIterator_Next(data);
    }
    while (item != 0) {
        if (String_ComparePrefixIgnoreCase(((SoundInstanceTextView*)item)->alias, (void*)name, length) == 0)
            return item;
        item = BufferIterator_Next(data);
    }
    return 0;
}

// Advance a buffer cursor using the caller-supplied mutex.
static inline void* VolumeIterator_Next(unsigned char* data, void* lk) {
    OSLockMutex(lk);
    void* node = *(void**)(data + kBufferListCursorOffset);
    if (node != 0) {
        void* next = ((SoundListLink*)node)->next;
        void* item = *(void**)node;
        if (next == *(void**)(data + kBufferListHeadOffset))
            next = 0;
        *(void**)(data + kBufferListCursorOffset) = next;
        OSUnlockMutex(lk);
        return item;
    }
    OSUnlockMutex(lk);
    return 0;
}

// Apply manager effect volume times per-buffer resource gain and volume to every buffer.
void SoundManager_ApplyBufferVolumes(unsigned char* self) {
    register unsigned char* owner = (*(unsigned char**)&AsSoundManagerState(self)->state);
    float factor = AsSoundManagerState(self)->volumeScale;
    register void* item;
    {
        WInstanceLock first(owner + kBufferListMutexOffset);
        *(void**)(owner + kBufferListCursorOffset) = *(void**)(owner + kBufferListHeadOffset);
        void* lk = owner + kBufferListMutexOffset;
        item = VolumeIterator_Next(owner, lk);
    }
    if (item != 0) {
        void* lk = owner + kBufferListMutexOffset;
        while (item != 0) {
            float value = AsSoundBufferPlayback(item)->volume;
            if (item != 0 && AsSoundBufferPlayback(item)->soundBuffer != 0) {
                AsSoundBufferPlayback(item)->volume = value;
                ((gtl::ISoundBuffer*)(AsSoundBufferPlayback(item)->soundBuffer))
                    ->SetVolume(factor * (AsSoundBufferPlayback(item)->baseVolume * value));
            }
            item = VolumeIterator_Next(owner, lk);
        }
    }
}

// Retain the native type name because exception relocations reference its destructor.
struct AF454Key {
    unsigned int w[25];
    // Construct the temporary filename key with caller-owned allocator-tag scratch.
    AF454Key(const char* name, unsigned char* temps) {
        unsigned int zero = 0;
        w[0] = zero;
        w[1] = zero;
        w[2] = zero;
        unsigned int len = strlen(name);
        StdString_ReserveCapacity(w, len);
        temps[0] = temps[4];
        StdString_ReplaceRange(w, 0, 0, name, name + len, temps);
        w[3] = zero;
        AsFilenameString(w)->vtable = (void*)__vt__14filenamestring;
        w[4] = zero;
        w[5] = zero;
        w[6] = zero;
        w[7] = zero;
        w[8] = zero;
        w[9] = zero;
        w[10] = zero;
        w[11] = zero;
        w[12] = zero;
        w[13] = zero;
        w[14] = zero;
        w[15] = zero;
        w[16] = zero;
        w[17] = zero;
        w[18] = zero;
        w[19] = zero;
        w[20] = zero;
        w[21] = zero;
        w[22] = zero;
        w[23] = zero;
    }
    // Destroy the temporary filename component strings in their required order.
    ~AF454Key() {
        FilenameOwnedStringView* s = (FilenameOwnedStringView*)w;
        if (s[7].flags >> kHeapStringFlagShift)
            __dl__FPv(s[7].data);
        if (s[6].flags >> kHeapStringFlagShift)
            __dl__FPv(s[6].data);
        if (s[5].flags >> kHeapStringFlagShift)
            __dl__FPv(s[5].data);
        if (s[4].flags >> kHeapStringFlagShift)
            __dl__FPv(s[4].data);
        if (s[3].flags >> kHeapStringFlagShift)
            __dl__FPv(s[3].data);
        if (s[2].flags >> kHeapStringFlagShift)
            __dl__FPv(s[2].data);
        if (s[1].flags >> kHeapStringFlagShift)
            __dl__FPv(s[1].data);
        if (s[0].flags >> kHeapStringFlagShift)
            __dl__FPv(s[0].data);
    }
};
typedef AF454Key PlayingNameKey;

// Split and cache the queried filename with the original scoped cleanup lifetimes.
static inline const char* PlayingName_ParsePath(PlayingNameKey* key, unsigned char* temps, char* a, char* b, char* c,
                                                char* d) {
    char* s = (char*)key;
    const char* src;
    if ((AsFilenameString(s)->input.flags >> kHeapStringFlagShift) == 0)
        src = s + 1;
    else
        src = AsFilenameString(s)->input.data;
    FileIO_SplitPath(src, a, b, c, d);

    unsigned int cap;
    if ((AsFilenameString(s)->drive.flags >> kHeapStringFlagShift) == 0)
        cap = ((unsigned char*)&AsFilenameString(s)->drive.flags)[0] & kInlineStringLengthMask;
    else
        cap = AsFilenameString(s)->drive.length;
    temps[0x0c] = temps[0x08];
    {
        unsigned int n = strlen(a);
        StdString_ReplaceRange(s + kFilenameDriveOffset, 0, (int)cap, a, a + n, temps + 0x0c);
    }
    if ((AsFilenameString(s)->directory.flags >> kHeapStringFlagShift) == 0)
        cap = ((unsigned char*)&AsFilenameString(s)->directory.flags)[0] & kInlineStringLengthMask;
    else
        cap = AsFilenameString(s)->directory.length;
    temps[0x14] = temps[0x10];
    {
        unsigned int n = strlen(b);
        StdString_ReplaceRange(s + kFilenameDirectoryOffset, 0, (int)cap, b, b + n, temps + 0x14);
    }
    if ((AsFilenameString(s)->stem.flags >> kHeapStringFlagShift) == 0)
        cap = ((unsigned char*)&AsFilenameString(s)->stem.flags)[0] & kInlineStringLengthMask;
    else
        cap = AsFilenameString(s)->stem.length;
    temps[0x1c] = temps[0x18];
    {
        unsigned int n = strlen(c);
        StdString_ReplaceRange(s + kFilenameStemOffset, 0, (int)cap, c, c + n, temps + 0x1c);
    }
    if ((AsFilenameString(s)->extension.flags >> kHeapStringFlagShift) == 0)
        cap = ((unsigned char*)&AsFilenameString(s)->extension.flags)[0] & kInlineStringLengthMask;
    else
        cap = AsFilenameString(s)->extension.length;
    temps[0x24] = temps[0x20];
    {
        unsigned int n = strlen(d);
        StdString_ReplaceRange(s + kFilenameExtensionOffset, 0, (int)cap, d, d + n, temps + 0x24);
    }

    {
        unsigned int small = !(AsFilenameString(s)->drive.flags >> kHeapStringFlagShift);
        unsigned int size = small ? (((unsigned char*)&AsFilenameString(s)->drive.flags)[0] & kInlineStringLengthMask)
                                  : AsFilenameString(s)->drive.length;
        if (size != 0) {
            unsigned int index =
                (small ? (((unsigned char*)&AsFilenameString(s)->drive.flags)[0] & kInlineStringLengthMask)
                       : AsFilenameString(s)->drive.length) -
                1;
            char* p = small ? s + kFilenameDriveOffset + 1 : AsFilenameString(s)->drive.data;
            p[index] = 0;
        }
    }

    if ((AsFilenameString(s)->driveDirectory.flags >> kHeapStringFlagShift) == 0)
        cap = ((unsigned char*)&AsFilenameString(s)->driveDirectory.flags)[0] & kInlineStringLengthMask;
    else
        cap = AsFilenameString(s)->driveDirectory.length;
    temps[0x2c] = temps[0x28];
    {
        unsigned int n = strlen(a);
        StdString_ReplaceRange(s + kFilenameDriveDirectoryOffset, 0, (int)cap, a, a + n, temps + 0x2c);
    }
    if ((AsFilenameString(s)->driveDirectory.flags >> kHeapStringFlagShift) == 0)
        cap = ((unsigned char*)&AsFilenameString(s)->driveDirectory.flags)[0] & kInlineStringLengthMask;
    else
        cap = AsFilenameString(s)->driveDirectory.length;
    temps[0x34] = temps[0x30];
    {
        unsigned int n = strlen(b);
        StdString_ReplaceRange(s + kFilenameDriveDirectoryOffset, (int)cap, 0, b, b + n, temps + 0x34);
    }

    if ((AsFilenameString(s)->fileName.flags >> kHeapStringFlagShift) == 0 &&
        (AsFilenameString(s)->stem.flags >> kHeapStringFlagShift) == 0) {
        AsFilenameString(s)->fileName.flags = AsFilenameString(s)->stem.flags;
        AsFilenameString(s)->fileName.length = AsFilenameString(s)->stem.length;
        (*(unsigned int*)&AsFilenameString(s)->fileName.data) = (*(unsigned int*)&AsFilenameString(s)->stem.data);
    } else {
        unsigned int oldcap =
            (AsFilenameString(s)->fileName.flags >> kHeapStringFlagShift)
                ? AsFilenameString(s)->fileName.length
                : (((unsigned char*)&AsFilenameString(s)->fileName.flags)[0] & kInlineStringLengthMask);
        unsigned int n;
        char* p;
        if ((AsFilenameString(s)->stem.flags >> kHeapStringFlagShift) == 0) {
            n = ((unsigned char*)&AsFilenameString(s)->stem.flags)[0] & kInlineStringLengthMask;
            p = s + kFilenameStemOffset + 1;
        } else {
            p = AsFilenameString(s)->stem.data;
            n = AsFilenameString(s)->stem.length;
        }
        temps[0x3c] = temps[0x38];
        StdString_ReplaceRange(s + kFilenameFileNameOffset, 0, (int)oldcap, p, p + n, temps + 0x3c);
    }
    if ((AsFilenameString(s)->fileName.flags >> kHeapStringFlagShift) == 0)
        cap = ((unsigned char*)&AsFilenameString(s)->fileName.flags)[0] & kInlineStringLengthMask;
    else
        cap = AsFilenameString(s)->fileName.length;
    unsigned int n;
    char* p;
    if ((AsFilenameString(s)->extension.flags >> kHeapStringFlagShift) == 0) {
        n = ((unsigned char*)&AsFilenameString(s)->extension.flags)[0] & kInlineStringLengthMask;
        p = s + kFilenameExtensionOffset + 1;
    } else {
        p = AsFilenameString(s)->extension.data;
        n = AsFilenameString(s)->extension.length;
    }
    temps[0x44] = temps[0x40];
    StdString_ReplaceRange(s + kFilenameFileNameOffset, (int)cap, 0, p, p + n, temps + 0x44);
    if ((AsFilenameString(s)->stem.flags >> kHeapStringFlagShift) == 0)
        return s + kFilenameStemOffset + 1;
    return AsFilenameString(s)->stem.data;
}

struct PlayingNamePathPart {
    char v[kPathPartBufferBytes]; /* Zero the split-path scratch buffer. */
    PlayingNamePathPart() : v() {}
};

// Retain the native type name because exception relocations reference its destructor.
struct AF454Frame {
    unsigned char temps[72];
    PlayingNameKey key;
    PlayingNamePathPart a, b, c, d;
    // Construct the filename lookup key alongside its allocator-tag scratch.
    AF454Frame(const char* name) : key(name, temps) {}
};
typedef AF454Frame PlayingNameFrame;

#pragma use_lmw_stmw on
// Compare the selected stream filename stem and report whether its state is non-idle.
int SoundChannelGroup_IsSelectedStreamNonIdleWithMatchingStem(void* selfArg, const char* name) {
    int result = 0;
    unsigned char* self = (unsigned char*)selfArg;
    if (name == 0)
        return 0;
    unsigned char* entry = self + *(unsigned int*)self * kStreamSlotBytes;
    if (entry == 0)
        return result;
    {
        PlayingNameFrame frame(name);
        const char* part = PlayingName_ParsePath(&frame.key, frame.temps, frame.a.v, frame.b.v, frame.c.v, frame.d.v);
        unsigned int length = strlen(part);
        if (length == 0)
            return result;
        unsigned int fieldFlag = *(unsigned int*)(entry + 0x14) >> kHeapStringFlagShift;
        int fieldInline = !fieldFlag;
        unsigned int fieldLen;
        if (fieldInline)
            fieldLen = entry[0x14] & kInlineStringLengthMask;
        else
            fieldLen = *(unsigned int*)(entry + 0x18);
        if (length != fieldLen)
            return result;
        const char* fieldText;
        if (fieldInline)
            fieldText = (const char*)(entry + 0x15);
        else
            fieldText = *(char**)(entry + 0x1c);
        result = String_ComparePrefixIgnoreCase((void*)part, fieldText, (int)length) == 0;
    }
    if (result)
        return *(int*)(self + *(unsigned int*)self * kStreamSlotBytes + 0x10) != 0;
    return 0;
}
#pragma use_lmw_stmw reset

// Advance the pause/resume buffer iterator, returning its current node.
inline void PauseIterator_Next(IterOwner*& owner, IterNode*& node) {
    void* lockPtr = &owner->lock;
    OSLockMutex(lockPtr);
    if (owner->cursor != 0) {
        IterNode* next = owner->cursor->next;
        node = owner->cursor->item;
        owner->cursor = next == owner->head ? 0 : next;
        OSUnlockMutex(lockPtr);
    } else {
        OSUnlockMutex(lockPtr);
        node = 0;
    }
}
// Pause all buffered sounds and both music slots, tracking nested pause requests.
void SoundManager_PausePlayback(unsigned char* self) {
    IterNode* node;
    IterOwner* owner = (*(IterOwner**)&AsSoundManagerState(self)->state);
    {
        WInstanceLock first(&owner->lock);
        owner->cursor = owner->head;
        PauseIterator_Next(owner, node);
    }
    while (node != 0) {
        void* obj = AsSoundBufferPlayback(node)->soundBuffer;
        if (obj)
            ((gtl::ISoundBuffer*)obj)->Pause();
        PauseIterator_Next(owner, node);
    }
    owner = (IterOwner*)((*(unsigned char**)&AsSoundManagerState(self)->state) + kVoiceGroupOffset);
    for (unsigned int i = 0; i < 2; ++i, owner = (IterOwner*)((char*)owner + kStreamSlotBytes)) {
        void* ptr = *(void**)((char*)owner + 4);
        if (ptr)
            ((gtl::ISoundStream*)ptr)->Pause();
    }
    ++(*(int*)&AsSoundManagerState(self)->playbackPauseDepth);
}

// One step of a DtorMap bucket walk over the map at data+0x1d0: locks the
// map mutex, takes the current head, advances the cursor and unlocks again.
// Return the next bank object from the mutex-protected map iterator.
static inline void* BankRelease_Next(DtorMap* map) {
    void* m = map->mutex;
    OSLockMutex(m);
    DtorNode* node = map->iterationNext;
    void* owner;
    if (node) {
        DtorNode* next = node->next;
        owner = node->owner;
        map->iterationNext = next == map->head ? 0 : next;
        OSUnlockMutex(m);
    } else {
        OSUnlockMutex(m);
        owner = 0;
    }
    return owner;
}

// Read a bank name length from the string following its reference-count header.
static inline unsigned int BankRelease_NameLength(unsigned char* o) {
    if ((*(unsigned int*)(o + 8) >> kHeapStringFlagShift) == 0)
        return *(unsigned char*)(o + 8) & kInlineStringLengthMask;
    return *(unsigned int*)(o + 0xc);
}

// Return the smaller count by reference, preserving the original expression lifetimes.
static inline const unsigned int& BankRelease_Min(const unsigned int& a, const unsigned int& b) {
    return a < b ? a : b;
}

// Compare a bank name lexicographically, then compare full lengths.
static inline int BankRelease_CompareName(unsigned char* o, const char* name) {
    unsigned int size, size2t;
    unsigned int keyref = strlen(name);
    unsigned int keylen = keyref;
    unsigned int size1 = (*(unsigned int*)(o + 8) >> kHeapStringFlagShift) == 0
                             ? (*(unsigned char*)(o + 8) & kInlineStringLengthMask)
                             : *(unsigned int*)(o + 0xc);
    const char* data;
    unsigned int size2;
    if ((*(unsigned int*)(o + 8) >> kHeapStringFlagShift) == 0) {
        data = (const char*)(o + 9);
        size2 = *(unsigned char*)(o + 8) & kInlineStringLengthMask;
    } else {
        data = *(const char**)(o + 0x10);
        size2 = *(unsigned int*)(o + 0xc);
    }
    const unsigned int& m1 = BankRelease_Min(size1, size2t = size2);
    size = m1;
    const unsigned int& count = BankRelease_Min(keyref, size);
    int cmp = memcmp((const void*)data, (const void*)name, count);
    if (cmp == 0) {
        if (size < keylen)
            return -1;
        if (size == keylen)
            return 0;
        return 1;
    }
    return cmp;
}

// Reset the bank map cursor and return its first object.
static inline void* BankRelease_First(DtorMap* map) {
    WInstanceLock lock(map->mutex);
    map->iterationNext = map->head;
    return BankRelease_Next(map);
}

// Search by name and preserve the explicit null result on exhaustion.
static inline void BankRelease_Search(DtorMap* map, const char* name, void*& owner) {
    while (owner != 0) {
        if (BankRelease_CompareName((unsigned char*)owner, name) != 0) {
            owner = BankRelease_Next(map);
            continue;
        }
        return;
    }
    owner = 0;
}

// Keep map removal at the same inline depth as name comparison so stack slots retain their order.
static inline void BankRelease_RemoveUnused(DtorMap* map, DtorOwner* owner) {
    BufferMap_RemoveValue(map, owner, 0);
}

#pragma use_lmw_stmw on
// Release one reference to the matching bank and remove it when no references remain.
int SoundManager_ReleaseSoundBank(void* selfArg, const char* name) {
    if (name == 0)
        return 0;
    unsigned char* data = (*(unsigned char**)&AsSoundManagerState(selfArg)->state);
    DtorMap* map = (DtorMap*)(data + kBankListHeadOffset);
    void* owner = BankRelease_First(map);
    BankRelease_Search(map, name, owner);
    int state;
    if (owner != 0) {
        if (*(int*)((char*)owner + 0) == 0) {
            if (owner != 0) {
                ((VP4Obj_*)owner)->destroy(1);
            }
            state = 0;
        } else {
            state = *(volatile int*)((char*)owner + 0);
        }
        if (state == 1) {
            BankRelease_RemoveUnused(map, (DtorOwner*)owner);
        }
        {
            int refs = *(int*)((char*)owner + 0) - 1;
            *(int*)((char*)owner + 0) = refs;
            if (refs == 0 && owner != 0) {
                ((VP4Obj_*)owner)->destroy(1);
            }
        }
        return 1;
    }
    return 0;
}
#pragma use_lmw_stmw reset

// Resume buffered sounds and music slots when the nested playback pause count reaches zero.
int SoundManager_ResumePlayback(unsigned char* self) {
    int count = (*(int*)&AsSoundManagerState(self)->playbackPauseDepth) - 1;
    (*(int*)&AsSoundManagerState(self)->playbackPauseDepth) = count;
    if (count > 0)
        return 0;
    IterNode* item;
    IterOwner* owner = (*(IterOwner**)&AsSoundManagerState(self)->state);
    IterNode* zero = 0;
    (*(IterNode**)&AsSoundManagerState(self)->playbackPauseDepth) = zero;
    {
        WInstanceLock outer(&owner->lock);
        owner->cursor = owner->head;
        PauseIterator_Next(owner, item);
    }
    while (item != 0) {
        void* obj = AsSoundBufferPlayback(item)->soundBuffer;
        if (obj != 0)
            ((gtl::ISoundBuffer*)obj)->Resume();
        PauseIterator_Next(owner, item);
    }
    unsigned char* entries = (*(unsigned char**)&AsSoundManagerState(self)->state) + kVoiceGroupOffset;
    for (unsigned int i = 0; i < 2; ++i, entries += kStreamSlotBytes) {
        void* ptr = *(void**)(entries + 4);
        if (ptr != 0)
            ((gtl::ISoundStream*)ptr)->Resume();
    }
    return 1;
}

// Start an effect-volume transition or immediately reapply volume to every buffer.
void SoundManager_SetBufferVolume(unsigned char* self, float volume, float fadeSeconds) {
    if (fadeSeconds > kFloat_0_soundPlayback) {
        FloatTween_Start(self, kFadeDefault, volume, fadeSeconds);
        return;
    }
    FloatTween_Start(self, kFadeDefault, volume, kFloat_0_soundPlayback);
    void* item;
    unsigned char* data = (*(unsigned char**)&AsSoundManagerState(self)->state);
    float factor = AsSoundManagerState(self)->volumeScale;
    {
        WInstanceLock outer(data + kBufferListMutexOffset);
        *(void**)(data + kBufferListCursorOffset) = *(void**)(data + kBufferListHeadOffset);
        item = BufferIterator_Next(data);
    }
    while (item != 0) {
        float value = AsSoundBufferPlayback(item)->volume;
        if (item != 0 && AsSoundBufferPlayback(item)->soundBuffer != 0) {
            AsSoundBufferPlayback(item)->volume = value;
            ((gtl::ISoundBuffer*)(AsSoundBufferPlayback(item)->soundBuffer))
                ->SetVolume(factor * (AsSoundBufferPlayback(item)->baseVolume * value));
        }
        item = BufferIterator_Next(data);
    }
}

// Advance streams and fades, prune finished buffers, and update active spatial sounds for the listener.
void SoundManager_UpdatePlayback(unsigned char* self, void* listener, float deltaSeconds) {
    if (deltaSeconds <= kFloat_0_soundPlayback)
        deltaSeconds = kDefaultSoundUpdateSeconds;
    unsigned char* data = (*(unsigned char**)&AsSoundManagerState(self)->state);
    SoundManager_UpdateStreamChannels(data, deltaSeconds);
    SoundManager_UpdateStreamChannels(data + kVoiceGroupOffset, deltaSeconds);
    SoundBufferList_PruneFinishedPlayback(data + kBufferListHeadOffset);
    void* iterator = SoundPlaybackList_GetCollection(data + kBufferListHeadOffset);
    void* item = SoundPlaybackList_BeginIteration((PlaybackIterationList*)iterator);
    while (item != 0) {
        if (listener != 0 && SoundBufferPlayback_IsActive((unsigned char*)item) &&
            AsSoundBufferPlayback(item)->spatialized) {
            void* obj = AsSoundBufferPlayback(item)->soundBuffer;
            if (obj != 0) {
                float vec[3];
                Matrix_TransformPoint(vec, ((SoundInstanceTextView*)item)->position,
                                      ((SoundCameraTransformView*)listener)->worldToListener);
                float pos[3] = {vec[0], vec[1], vec[2] + s_soundListenerZOffset};
                ((gtl::ISoundBuffer*)obj)->SetPosition(pos, 1);
                if (!AsSoundBufferPlayback(item)->playbackStarted) {

                    ((gtl::ISoundBuffer*)obj)
                        ->Play((SoundPlayback_HasAnyFlags((unsigned char*)item, 1) ? kPlayLooped : kPlayOnce));
                    AsSoundBufferPlayback(item)->playbackStarted = 1;
                }
            }
        }
        item = SoundPlaybackList_NextIteration((PlaybackIterationList*)iterator);
    }
    if (FloatTween_Update(self, deltaSeconds))
        SoundManager_ApplyBufferVolumes(self);
    if (FloatTween_Update(self + kMusicVolumeTweenOffset, deltaSeconds))
        SoundManager_ApplyStreamVolume(self);
}

// Configure fade-in, apply stream gains, and start or restart playback with the requested loop mode.
void SoundStreamPlayback_Start(unsigned char* self, int loop, float fadeSeconds, int fadeIn, int restart) {
    if (fadeIn != 0 && fadeSeconds > kFloat_0_soundPlayback) {
        FloatTween_Start(self + 0x24, kFadeIn, kFloat_1_SoundPlayback, fadeSeconds);
    } else {
        FloatTween_Start(self + 0x24, kFadeDefault, kFloat_1_SoundPlayback, kFloat_0_soundPlayback);
    }
    if (AsSoundPlaybackState(self)->playState == kStreamFadingOut)
        AsSoundPlaybackState(self)->playState = kStreamPlaying;
    if (restart != 0 || AsSoundPlaybackState(self)->playState == kStreamIdle) {
        if (AsSoundPlaybackState(self)->source != 0 && AsSoundPlaybackState(self)->source != 0) {
            float f1 = AsSoundPlaybackState(self)->baseVolume;
            float f0 = AsSoundPlaybackState(self)->volume;

            float f2 = AsSoundPlaybackState(self)->fadeValue;
            f0 = f0 * f1;
            f1 = f0 * f2;
            ((gtl::ISoundStream*)(AsSoundPlaybackState(self)->source))->SetVolume(f1);
            if (AsSoundPlaybackState(self)->source != 0)
                ((gtl::ISoundStream*)(AsSoundPlaybackState(self)->source))
                    ->SetPitch(AsSoundPlaybackState(self)->basePitch * AsSoundPlaybackState(self)->pitch);
            void* obj2 = AsSoundPlaybackState(self)->source;
            int mode = kPlayOnce;
            if (loop != 0)
                mode = kPlayLooped;
            ((gtl::ISoundStream*)obj2)->Play(mode);
            AsSoundPlaybackState(self)->playState = kStreamPlaying;
        }
    }
    AsSoundPlaybackState(self)->loopEnabled = loop;
    AsSoundPlaybackState(self)->fadeInEnabled = fadeIn;
    AsSoundPlaybackState(self)->transitionDuration = fadeSeconds;
}

// Apply the selected hardware output mask before deciding whether music needs another bank.
static inline void ConfigureSoundHardwareOutput(int& mode, unsigned int& mask) {
    GtlAudio_SetCachedOutputOption(0);
    switch (mode) {
    case kSoundMono: /* Route the hardware mix to mono. */
        mask = kOutputMono;
        break;
    case kSoundStereo: /* Route the hardware mix to stereo. */
        mask = kOutputStereo;
        break;
    case kSoundDolbyProLogicII: /* Select the Dolby-capable mix/bank variant. */
        mask = kOutputDolbyProLogicII;
        break;
    }
    gtl_Audio_SetOutputMode(mask);
}

// Set mono, stereo or Dolby output, restarting music with the matching bank variant and clearing voices.
// Transition map (required live manager/state, serialized with playback updates):
// 1. ALWAYS configure hardware first. mode 0/1/2 -> mask 1/2/4; other modes use
//    stereo hardware but are still cached as supplied (no validation/failure).
// 2. Unchanged cached mode skips music/voice changes; final preserve-stereo call
//    still runs. Changed mode is published BEFORE loading; there is no rollback.
// 3. Selected music idle -> skip music. Empty name -> stop/fade selected. Same
//    stem + bank variant -> update gain and Start(restart=0), cancelling fade-out.
//    Different variant -> stop/fade old slot; reuse alternate if stem/bank match,
//    then select it. Positive allowed fades can leave both slots live (crossfade).
// 4. Otherwise select first idle slot, or toggle and immediately reset that slot.
//    BeginBankLoad refuses non-idle/source-present slots or missing metadata/sample
//    path: selected index/output mode stay changed; no replacement sound starts.
//    Missing paths may leave cached path strings partly changed. On acceptance,
//    slot enters LoadingBank; Start stores loop/fade settings. UpdateStreamChannels
//    completes async metadata/sample loading and starts it. Missing data, zero
//    size, resource/create/bind failures leave LoadingBank for retry, not rollback.
//    Temporary sound resources are released on failure; sharedFile stays slot-owned.
// 5. For EVERY changed mode, release/reset both voice slots, select voice slot 0,
//    then repeat the string/gain reset pass (keep duplicate reset/order). This
//    happens even when music restart failed. Finally set preserve-stereo=(mode!=0).
// Borrowed music name points into old slot storage: do not clear/reallocate its
// name earlier or move string destruction across bank-load calls. Source/sharedFile
// references belong to slots until reset; no success status is exposed to caller.
void SoundManager_SetOutputMode(unsigned char* self, int mode) {
    unsigned int mask = kOutputStereo;
    ConfigureSoundHardwareOutput(mode, mask);
    if ((*(int*)&AsSoundManagerState(self)->outputMode) != mode) {
        (*(int*)&AsSoundManagerState(self)->outputMode) = mode;
        unsigned char* musicGroupOrigin;
        const char* resourceName;
        int fadeIn;
        unsigned int useDolbyBank;
        int loop;
        float fadeSeconds;
        useDolbyBank = (mode == kSoundDolbyProLogicII);
        float streamVolume;
        musicGroupOrigin = (*(unsigned char**)&AsSoundManagerState(self)->state);
        streamVolume = AsSoundManagerState(self)->volume;
        unsigned char* selectedSlotOrigin = musicGroupOrigin + *(int*)musicGroupOrigin * kStreamSlotBytes;
        unsigned char* selectedPlayback = selectedSlotOrigin + 4;
        unsigned char* alternateSlotOrigin;
        unsigned char* alternatePlayback;
        unsigned char* loadedPlayback;
        if (PlaybackAtSlotOrigin(selectedSlotOrigin)->playState != 0) {
            if ((AsSoundPlaybackState(selectedPlayback)->nameFlags >> kHeapStringFlagShift) == 0)
                resourceName = (const char*)(selectedPlayback + 0x11);
            else
                resourceName = (*(const char**)&AsSoundPlaybackState(selectedPlayback)->nameData);
            fadeSeconds = AsSoundPlaybackState(selectedPlayback)->transitionDuration;
            fadeIn = AsSoundPlaybackState(selectedPlayback)->fadeInEnabled;
            loop = AsSoundPlaybackState(selectedPlayback)->loopEnabled;
            if (resourceName == 0 || strlen(resourceName) == 0) {
                SoundChannelGroup_StopWithFade(musicGroupOrigin, fadeSeconds, fadeIn);
            } else if (SoundPlayback_MatchesFileName(selectedPlayback, resourceName) != 0 &&
                       useDolbyBank == AsSoundPlaybackState(selectedPlayback)->useDolbyBank) {
                AsSoundPlaybackState(selectedPlayback)->volume = streamVolume;
                if (AsSoundPlaybackState(selectedPlayback)->source != 0) {
                    float currentVolume;
                    float baseVolume;
                    baseVolume = AsSoundPlaybackState(selectedPlayback)->baseVolume;
                    currentVolume = AsSoundPlaybackState(selectedPlayback)->volume;
                    float fadeGain;
                    fadeGain = AsSoundPlaybackState(selectedPlayback)->fadeValue;
                    __typeof__(baseVolume * currentVolume) scaledVolume = (baseVolume * currentVolume);
                    ((void (*)(void*, float))SetVolume__Q23gtl12ISoundStreamFf)(
                        AsSoundPlaybackState(selectedPlayback)->source, (scaledVolume * fadeGain));
                }
                SoundStreamPlayback_Start(selectedPlayback, loop, fadeSeconds, fadeIn, 0);
            } else {
                SoundChannelGroup_StopWithFade(musicGroupOrigin, fadeSeconds, fadeIn);
                alternateSlotOrigin = musicGroupOrigin + ((*(int*)musicGroupOrigin + 1) & 1) * kStreamSlotBytes;
                alternatePlayback = (4 + alternateSlotOrigin);
                if (SoundPlayback_MatchesFileName(alternatePlayback, resourceName) != 0 &&
                    useDolbyBank == AsSoundPlaybackState(alternatePlayback)->useDolbyBank) {
                    AsSoundPlaybackState(alternatePlayback)->volume = streamVolume;
                    if (*(void**)alternatePlayback != 0) {
                        float currentVolume;
                        float baseVolume;
                        baseVolume = AsSoundPlaybackState(alternatePlayback)->baseVolume;
                        currentVolume = *(float*)(4 + alternatePlayback);
                        float fadeGain;
                        fadeGain = AsSoundPlaybackState(alternatePlayback)->fadeValue;
                        __typeof__(currentVolume * baseVolume) scaledVolume = (baseVolume * currentVolume);
                        ((void (*)(void*, float))SetVolume__Q23gtl12ISoundStreamFf)(*(void**)alternatePlayback,
                                                                                    (scaledVolume * fadeGain));
                    }
                    SoundStreamPlayback_Start(alternatePlayback, loop, fadeSeconds, fadeIn, 0);
                    *(int*)musicGroupOrigin = (*(int*)musicGroupOrigin + 1) & 1;
                } else {
                    unsigned char string[0x68];
                    unsigned int found = 0;

                    unsigned char* scan = musicGroupOrigin;
                    for (int i = 0; 2 > i; i++, scan += kStreamSlotBytes) {
                        if (*(int*)(scan + 0x10) == 0) {
                            found = 1;
                            *(int*)musicGroupOrigin = (i & 1);
                            break;
                        }
                    }
                    if (!found) {
                        *(int*)musicGroupOrigin = (*(int*)musicGroupOrigin + 1) & 1;
                        SoundPlayback_StopWithOptionalFade(musicGroupOrigin +
                                                               *(int*)musicGroupOrigin * kStreamSlotBytes + 4,
                                                           kFloat_0_soundPlayback, 0);
                    }
                    FilenameString_Ctor(string, resourceName);
                    const char* text = FilenameString_SplitAndGetStem(string);
                    loadedPlayback = musicGroupOrigin + *(int*)musicGroupOrigin * kStreamSlotBytes + 4;
                    if (SoundPlaybackState_BeginBankLoad(loadedPlayback, text, useDolbyBank) != 0) {
                        AsSoundPlaybackState(loadedPlayback)->volume = streamVolume;
                        if (AsSoundPlaybackState(loadedPlayback)->source != 0) {
                            float currentVolume;
                            float baseVolume;
                            float fadeGain;
                            baseVolume = AsSoundPlaybackState(loadedPlayback)->baseVolume;
                            currentVolume = AsSoundPlaybackState(loadedPlayback)->volume;
                            fadeGain = AsSoundPlaybackState(loadedPlayback)->fadeValue;
                            __typeof__(baseVolume * currentVolume) scaledVolume = (baseVolume * currentVolume);
                            ((void (*)(void*, float))SetVolume__Q23gtl12ISoundStreamFf)(
                                AsSoundPlaybackState(loadedPlayback)->source, (scaledVolume * fadeGain));
                        }
                        SoundStreamPlayback_Start(loadedPlayback, loop, fadeSeconds, fadeIn, 1);
                    }
                    FilenameString_dtor(string, -1);
                }
            }
        }
        unsigned char* resources = (*(unsigned char**)&AsSoundManagerState(self)->state);
        unsigned char* voiceGroupOrigin = resources + kVoiceGroupOffset;
        {
            float zero = kFloat_0_soundPlayback;
            unsigned char* playbackBytes = voiceGroupOrigin + 4;
            unsigned char* slotOrigin = voiceGroupOrigin;
            float one = kFloat_1_SoundPlayback;
            for (unsigned int i = 0; 2 > i; i++, slotOrigin += kStreamSlotBytes, playbackBytes += kStreamSlotBytes) {
                if (PlaybackAtSlotOrigin(slotOrigin)->source != 0)
                    reset_rc((void**)(slotOrigin + 4));
                if (PlaybackAtSlotOrigin(slotOrigin)->sharedFile != 0) {
                    CSharedFileHandle::Destroy(PlaybackAtSlotOrigin(slotOrigin)->sharedFile, 1);
                    PlaybackAtSlotOrigin(slotOrigin)->sharedFile = 0;
                }
                ResetMap_ResetStream(slotOrigin, playbackBytes, one, zero);
            }
        }
        AsSoundChannelGroup(voiceGroupOrigin)->selected = 0;
        {
            float one = kFloat_1_SoundPlayback;
            float zero = kFloat_0_soundPlayback;
            unsigned char* slotOrigin = voiceGroupOrigin;
            unsigned char* playbackBytes = (voiceGroupOrigin + 4);
            for (unsigned int i = 0; i < 2; i++, slotOrigin += kStreamSlotBytes, playbackBytes += kStreamSlotBytes) {
                ResetMap_ResetStream(slotOrigin, playbackBytes, one, zero);
            }
        }
    }
    AudioMix_SetPreserveStereo(AsSoundManagerState(self)->outputMode != 0);
}

#pragma dont_inline on
// Destroy cached filename strings in reverse order and optionally delete the filename object.
void* FilenameString_dtor(unsigned char* self, int deleting) {
    if (self != 0) {
        FilenameOwnedStringView* s = (FilenameOwnedStringView*)self;
        if (&s[7] != 0 && (s[7].flags >> kHeapStringFlagShift))
            __dl__FPv(s[7].data);
        if (&s[6] != 0 && (s[6].flags >> kHeapStringFlagShift))
            __dl__FPv(s[6].data);
        if (&s[5] != 0 && (s[5].flags >> kHeapStringFlagShift))
            __dl__FPv(s[5].data);
        if (&s[4] != 0 && (s[4].flags >> kHeapStringFlagShift))
            __dl__FPv(s[4].data);
        if (&s[3] != 0 && (s[3].flags >> kHeapStringFlagShift))
            __dl__FPv(s[3].data);
        if (&s[2] != 0 && (s[2].flags >> kHeapStringFlagShift))
            __dl__FPv(s[2].data);
        if (&s[1] != 0 && (s[1].flags >> kHeapStringFlagShift))
            __dl__FPv(s[1].data);
        if (&s[0] != 0 && (s[0].flags >> kHeapStringFlagShift))
            __dl__FPv(s[0].data);
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}
#pragma dont_inline off

// Read sound mode and signed percentage volumes from the current save record and apply optional fades.
void SoundManager_ApplySavedAudioSettings(unsigned char* self, float fadeSeconds) {
    unsigned char* savedRecord;
    if (((int&)GetSaveDataSlot()) == 0) {
        if (s_saveDataInitialized == 0) {
            SaveData_Ctor((void*)s_saveData);
            s_saveDataInitialized = 1;
        }
        ((int&)GetSaveDataSlot()) = (int)s_saveData;
    }
    savedRecord = (unsigned char*)SaveData_GetCurrentRecord((void*)((int&)GetSaveDataSlot()));
    SoundManager_SetOutputMode(self, (((SavedAudioOptionsView*)savedRecord)->optionBits >> kSavedSoundModeShift) &
                                         kSavedSoundModeMask);
    int volumePercent = ((SavedAudioOptionsView*)savedRecord)->effectVolumePercent;
    float volumeRatio = (float)volumePercent / kFloat_100_SoundManager;
    if (fadeSeconds > kFloat_0_soundPlayback) {
        FloatTween_Start(self, kFadeDefault, volumeRatio, fadeSeconds);
    } else {
        FloatTween_Start(self, kFadeDefault, volumeRatio, 0.0f);
        SoundManager_ApplyBufferVolumes(self);
    }
    volumePercent = ((SavedAudioOptionsView*)savedRecord)->musicVolumePercent;
    volumeRatio = (float)volumePercent / kFloat_100_SoundManager;
    if (fadeSeconds > kFloat_0_soundPlayback) {
        FloatTween_Start(self + kMusicVolumeTweenOffset, kFadeDefault, volumeRatio, fadeSeconds);
    } else {
        FloatTween_Start(self + kMusicVolumeTweenOffset, kFadeDefault, volumeRatio, 0.0f);
        unsigned char* arr = (*(unsigned char**)&AsSoundManagerState(self)->state);
        unsigned int index = *(unsigned int*)arr;
        unsigned char* entry = arr + index * kStreamSlotBytes;
        AsSoundChannelGroup(entry)->channels[0].volume = AsSoundManagerState(self)->volume;
        void* obj = AsSoundChannelGroup(entry)->channels[0].source;
        if (obj != 0)
            ((gtl::ISoundStream*)obj)
                ->SetVolume(AsSoundChannelGroup(entry)->channels[0].baseVolume *
                            AsSoundChannelGroup(entry)->channels[0].volume *
                            AsSoundChannelGroup(entry)->channels[0].fadeValue);
    }
    Audio_SetStreamVolumeWithFade(volumeRatio, fadeSeconds);
}

// Reuse or load a voice stream unless both primary music slots are occupied.
void SoundManager_PlayVoiceByName(unsigned char* self, const char* name) {
    if (!AsSoundManagerState(self)->enabled)
        return;
    int active = 0;
    unsigned char* data = (*(unsigned char**)&AsSoundManagerState(self)->state);
    if (AsSoundResources(data)->music.channels[0].playState != 0)
        active = 1;
    if (AsSoundResources(data)->music.channels[1].playState != 0)
        ++active;
    if (active == 2)
        return;
    float value = AsSoundManagerState(self)->volumeScale;
    float duration = AsSoundManagerState(self)->transitionDuration;
    unsigned char* entries = data + kVoiceGroupOffset;
    if (name == 0 || strlen(name) == 0) {
        SoundChannelGroup_StopWithFade(entries, duration, 0);
        return;
    }
    unsigned char* entry = entries + *(int*)entries * kStreamSlotBytes + 4;
    if (SoundPlayback_MatchesFileName(entry, name) && !entry[0x1e]) {
        AsSoundPlaybackState(entry)->volume = value;
        if (AsSoundPlaybackState(entry)->source)
            ((gtl::ISoundStream*)(AsSoundPlaybackState(entry)->source))
                ->SetVolume(AsSoundPlaybackState(entry)->baseVolume * AsSoundPlaybackState(entry)->volume *
                            AsSoundPlaybackState(entry)->fadeValue);
        SoundStreamPlayback_Start(entry, 0, duration, 0, 0);
        return;
    }
    SoundChannelGroup_StopWithFade(entries, duration, 0);
    entry = entries + ((*(int*)entries + 1) & 1) * kStreamSlotBytes + 4;
    if (SoundPlayback_MatchesFileName(entry, name) && !entry[0x1e]) {
        AsSoundPlaybackState(entry)->volume = value;
        if (AsSoundPlaybackState(entry)->source)
            ((gtl::ISoundStream*)(AsSoundPlaybackState(entry)->source))
                ->SetVolume(AsSoundPlaybackState(entry)->baseVolume * AsSoundPlaybackState(entry)->volume *
                            AsSoundPlaybackState(entry)->fadeValue);
        SoundStreamPlayback_Start(entry, 0, duration, 0, 0);
        *(int*)entries = (*(int*)entries + 1) & 1;
        return;
    }
    int found = 0;
    for (int i = 0; i < 2; ++i) {
        if (*(int*)(entries + i * kStreamSlotBytes + 0x10) == 0) {
            *(int*)entries = i & 1;
            found = 1;
            break;
        }
    }
    if (!found) {
        *(int*)entries = (*(int*)entries + 1) & 1;
        SoundPlayback_StopWithOptionalFade(entries + *(int*)entries * kStreamSlotBytes + 4, 0.0f, 0);
    }
    unsigned char string[sizeof(FilenameString)];
    FilenameString_Ctor(string, name);
    const char* text = FilenameString_SplitAndGetStem(string);
    entry = entries + *(int*)entries * kStreamSlotBytes + 4;
    if (SoundPlaybackState_BeginBankLoad(entry, text, 0)) {
        AsSoundPlaybackState(entry)->volume = value;
        if (AsSoundPlaybackState(entry)->source)
            ((gtl::ISoundStream*)(AsSoundPlaybackState(entry)->source))
                ->SetVolume(AsSoundPlaybackState(entry)->baseVolume * AsSoundPlaybackState(entry)->volume *
                            AsSoundPlaybackState(entry)->fadeValue);
        SoundStreamPlayback_Start(entry, 0, duration, 0, 1);
    }
    FilenameString_dtor(string, -1);
}

struct PlaybackResourceMapNode {
    void* key;
    const char* name;
    unsigned char flag;
    char pad09[3];
    PlaybackResourceMapNode* prev;
    PlaybackResourceMapNode* next;
    PlaybackResourceMapNode* bucketNext;
};
struct PlaybackResourceMap {
    PlaybackResourceMapNode* head;
    PlaybackResourceMapNode* child;
    PlaybackResourceMapNode* list;
    PlaybackResourceMapNode* other;
    char pad10[4];
    unsigned int mask;
    char pad18[4];
    PlaybackResourceMapNode** buckets;
    WInstanceLock lock;
};

// Remove a bank-map node from its name hash chain.
static inline void BankMap_Unbucket(PlaybackResourceMap* self, PlaybackResourceMapNode* h) {
    WInstanceLock bucketLock(&self->lock);
    void* lk = &self->lock;
    OSLockMutex(lk);
    const unsigned char* name = (const unsigned char*)h->name;
    int len = strlen((const char*)name);
    unsigned int idx = ((len << 6) | ((name[0] & 0x2a) | (name[len > 0 ? len - 1 : 0] & 0x15))) & self->mask;
    OSUnlockMutex(lk);
    PlaybackResourceMapNode** buckets = self->buckets;
    PlaybackResourceMapNode* b = buckets[idx];
    if (b == h) {
        buckets[idx] = h->bucketNext;
    } else {
        for (; b != 0; b = b->bucketNext) {
            if (b->bucketNext == h) {
                b->bucketNext = h->bucketNext;
                return;
            }
        }
    }
}

// Unlink a node and choose the surviving circular-list head.
static inline PlaybackResourceMapNode* BankMap_Unlink(void* lk, PlaybackResourceMapNode* hd,
                                                      PlaybackResourceMapNode* h) {
    OSLockMutex(lk);
    h->next->prev = h->prev;
    h->prev->next = h->next;
    PlaybackResourceMapNode* r;
    if (h != hd)
        r = hd;
    else if (h->next == hd)
        r = 0;
    else
        r = h->next;
    OSUnlockMutex(lk);
    return r;
}

// Insert a released bank-map node into the circular free list.
static inline PlaybackResourceMapNode* BankMap_InsertFree(void* lk, PlaybackResourceMapNode* o,
                                                          PlaybackResourceMapNode* h) {
    OSLockMutex(lk);
    if (o == 0) {
        h->prev = h;
        h->next = h;
        OSUnlockMutex(lk);
        return h;
    }
    h->next = o;
    h->prev = o->prev;
    o->prev = h;
    h->prev->next = h;
    OSUnlockMutex(lk);
    return o;
}

// Find an object, free its owned key and recycle the bank-map entry.
static inline void BankMap_RemoveValue(PlaybackResourceMap* self, void* lk0, PlaybackResourceMapNode* r30,
                                       PlaybackResourceMapNode* zero) {
    WInstanceLock scan(lk0);
    PlaybackResourceMapNode* first = self->head;
    PlaybackResourceMapNode* h = first;
    if (h == 0)
        return;
    do {
        if (h->key == r30) {
            PlaybackResourceMapNode* c = self->child;
            if (c == h)
                self->child = c->next == first ? 0 : c->next;
            if (h->name != 0) {
                BankMap_Unbucket(self, h);
                if (h->flag)
                    appFreeUni__FPv((void*)h->name);
            }
            void* lk2 = &self->lock;
            self->head = BankMap_Unlink(lk2, self->head, h);
            void* lk3 = &self->lock;
            self->other = BankMap_InsertFree(lk3, self->other, h);
            if (self->head == 0)
                self->child = zero;
            return;
        }
        h = h->next;
    } while (h != first);
}

// Release all named references, then free the collection node pool and bucket table.
void* PlaybackResourceMap_dtor(PlaybackResourceMap* self, int flags) {
    if (self != 0) {
        void* lk0 = &self->lock;
        PlaybackResourceMapNode* zero = 0;
        for (;;) {
            PlaybackResourceMapNode* r30;
            {
                WInstanceLock outer(lk0);
                self->child = self->head;
                OSLockMutex(lk0);
                PlaybackResourceMapNode* node = self->child;
                if (node != 0) {
                    r30 = (PlaybackResourceMapNode*)node->key;
                    self->child = node->next == self->head ? 0 : node->next;
                    OSUnlockMutex(lk0);
                } else {
                    OSUnlockMutex(lk0);
                    r30 = 0;
                }
            }
            if (r30 == 0)
                break;
            BankMap_RemoveValue(self, lk0, r30, zero);
            do {
                if (--*(int*)r30 == 0) {
                    if (r30 != 0) {
                        void** vt = ((SoundResourceHeader*)r30)->vtable;
                        ((void (*)(void*, int))vt[2])(r30, 1);
                    }
                    break;
                }
            } while (*(int*)r30 != 0);
        }
        if (self != 0) {
            WInstanceLock cleanupLock(&self->lock);
            PlaybackResourceMapNode* cur;
            if (self->list != 0)
                while ((cur = self->list) != 0) {
                    void* lk = &self->lock;
                    OSLockMutex(lk);
                    cur->next->prev = cur->prev;
                    cur->prev->next = cur->next;
                    PlaybackResourceMapNode* nxt = cur->next;
                    if (cur == nxt)
                        nxt = 0;
                    OSUnlockMutex(lk);
                    self->list = nxt;
                    appFreeUni__FPv(cur);
                }
            if (self->buckets != 0) {
                appFreeUni__FPv(self->buckets);
                self->buckets = 0;
            }
        }
        if (flags > 0)
            __dl__FPv(self);
    }
    return self;
}

// Advance the buffer iterator before a candidate can be deleted.
static inline void* PruneBuffers_Next(PlaybackResourceMap* self) {
    void* lk = &self->lock;
    OSLockMutex(lk);
    PlaybackResourceMapNode* n = self->child;
    if (n) {
        PlaybackResourceMapNode* next = n->next;
        void* item = n->key;
        if (next == self->head)
            next = 0;
        self->child = next;
        OSUnlockMutex(lk);
        return item;
    }
    OSUnlockMutex(lk);
    return 0;
}
// Hash name length and selected first/last character bits under the mutex.
static inline unsigned int PruneBuffers_NameHash(PlaybackResourceMap* self, const unsigned char* s) {
    void* m = &self->lock;
    OSLockMutex(m);
    int len = strlen((const char*)s);
    unsigned int hi = (s[0] & 0x2a) | (s[len > 0 ? len - 1 : 0] & 0x15);
    unsigned int hash = (len << 6) | hi;
    unsigned int mask = self->mask;
    unsigned int idx = hash & mask;
    OSUnlockMutex(m);
    return idx;
}
// Remove a buffer entry from the bucket for its key.
static inline void PruneBuffers_Unbucket(PlaybackResourceMap* self, PlaybackResourceMapNode* n) {
    WInstanceLock bucket(&self->lock);
    unsigned int idx = PruneBuffers_NameHash(self, (const unsigned char*)n->name);
    PlaybackResourceMapNode** b = self->buckets;
    PlaybackResourceMapNode* p = b[idx];
    if (p == n)
        b[idx] = n->bucketNext;
    else {
        while (p) {
            if (p->bucketNext == n) {
                p->bucketNext = n->bucketNext;
                return;
            }
            p = p->bucketNext;
        }
    }
}
// Choose the remaining list head after removing a finished buffer.
static inline PlaybackResourceMapNode* PruneBuffers_NewHead(PlaybackResourceMapNode* n, PlaybackResourceMapNode* h) {
    if (n != h)
        return h;
    if (n->next == h)
        return 0;
    return n->next;
}
// Return a detached node to the free list under the map mutex.
static inline PlaybackResourceMapNode* PruneBuffers_InsertFree(void* m, PlaybackResourceMapNode* h,
                                                               PlaybackResourceMapNode* n) {
    OSLockMutex(m);
    if (!h) {
        n->prev = n;
        n->next = n;
        OSUnlockMutex(m);
        return n;
    }
    n->next = h;
    n->prev = h->prev;
    h->prev = n;
    n->prev->next = n;
    OSUnlockMutex(m);
    return h;
}
// Unhash a finished-buffer entry, release its owned name and recycle the node.
static inline void PruneBuffers_RemoveNode(PlaybackResourceMap* self, PlaybackResourceMapNode* n,
                                           PlaybackResourceMapNode* first) {
    PlaybackResourceMapNode* c = self->child;
    if (c == n) {
        PlaybackResourceMapNode* next = c->next;
        if (next == first)
            next = 0;
        self->child = next;
    }
    if (n->name) {
        PruneBuffers_Unbucket(self, n);
        if (n->flag)
            appFreeUni__FPv((void*)n->name);
    }
    void* m = &self->lock;
    PlaybackResourceMapNode* h = self->head;
    OSLockMutex(m);
    n->next->prev = n->prev;
    n->prev->next = n->next;
    h = PruneBuffers_NewHead(n, h);
    OSUnlockMutex(m);
    void* m2 = &self->lock;
    self->head = h;
    self->other = PruneBuffers_InsertFree(m2, self->other, n);
    if (!self->head)
        self->child = 0;
}
// Find and remove the entry for a finished buffer.
static inline void PruneBuffers_RemoveValue(PlaybackResourceMap* self, void* item) {
    WInstanceLock scan(&self->lock);
    PlaybackResourceMapNode* first = self->head;
    PlaybackResourceMapNode* n = first;
    if (!n)
        return;
    do {
        if (n->key == item) {
            PruneBuffers_RemoveNode(self, n, first);
            return;
        }
        n = n->next;
    } while (n != first);
}
#pragma use_lmw_stmw on
// Stop and delete buffers that finished playback or no longer report valid status.
void SoundBufferList_PruneFinishedPlayback(void* arg0) {
    PlaybackResourceMap* self = (PlaybackResourceMap*)arg0;
    int flags = 8;
    void* item;
    {
        WInstanceLock outer(&self->lock);
        self->child = self->head;
        item = PruneBuffers_Next(self);
    }
    while (item) {
        void* next = PruneBuffers_Next(self);
        if (AsSoundBufferPlayback(item)->playbackStarted && AsSoundBufferPlayback(item)->soundBuffer &&
            (!((gtl::ISoundBuffer*)(AsSoundBufferPlayback(item)->soundBuffer))->GetStatusFlags(&flags) ||
             (flags & kSoundStatusFinished))) {
            if (item) {
                if (AsSoundBufferPlayback(item)->soundBuffer) {
                    ((gtl::ISoundBuffer*)(AsSoundBufferPlayback(item)->soundBuffer))->Stop(0);
                    void* obj = AsSoundBufferPlayback(item)->soundBuffer;
                    if (obj) {
                        if (--*(unsigned int*)obj == 0) {
                            delete (DtorRef*)obj;
                        }
                        AsSoundBufferPlayback(item)->soundBuffer = 0;
                    }
                }
                PruneBuffers_RemoveValue(self, item);
                if (item)
                    __dl__FPv(item);
            }
        }
        item = next;
    }
}
#pragma use_lmw_stmw reset

// Inlined filename construction/parsing variants retain distinct exception-cleanup lifetimes.
// Construct the filename cache with caller-owned allocator-tag scratch storage.
inline void* FilenameInline_Construct(void* arg0, const char* arg1, unsigned char* temp) {
    unsigned int zero = 0;
    AsFilenameString(arg0)->input.flags = zero;
    AsFilenameString(arg0)->input.length = zero;
    (*(unsigned int*)&AsFilenameString(arg0)->input.data) = zero;
    unsigned int len = strlen(arg1);
    StdString_ReserveCapacity(arg0, len);
    temp[68] = temp[64];
    StdString_ReplaceRange(arg0, 0, 0, arg1, arg1 + len, temp + 68);
    AsFilenameString(arg0)->drive.flags = zero;
    AsFilenameString(arg0)->vtable = &__vt__14filenamestring;
    AsFilenameString(arg0)->drive.length = zero;
    (*(unsigned int*)&AsFilenameString(arg0)->drive.data) = zero;
    AsFilenameString(arg0)->directory.flags = zero;
    AsFilenameString(arg0)->directory.length = zero;
    (*(unsigned int*)&AsFilenameString(arg0)->directory.data) = zero;
    AsFilenameString(arg0)->stem.flags = zero;
    AsFilenameString(arg0)->stem.length = zero;
    (*(unsigned int*)&AsFilenameString(arg0)->stem.data) = zero;
    AsFilenameString(arg0)->extension.flags = zero;
    AsFilenameString(arg0)->extension.length = zero;
    (*(unsigned int*)&AsFilenameString(arg0)->extension.data) = zero;
    AsFilenameString(arg0)->driveDirectory.flags = zero;
    AsFilenameString(arg0)->driveDirectory.length = zero;
    (*(unsigned int*)&AsFilenameString(arg0)->driveDirectory.data) = zero;
    *((unsigned int*)arg0 + 18) = zero;
    *((unsigned int*)arg0 + 19) = zero;
    *((unsigned int*)arg0 + 20) = zero;
    AsFilenameString(arg0)->fileName.flags = zero;
    AsFilenameString(arg0)->fileName.length = zero;
    (*(unsigned int*)&AsFilenameString(arg0)->fileName.data) = zero;
    return arg0;
}
// Caller-owned tag slots keep allocator objects alive across the inlined path split.
enum FilenameTagSlot {
    kDriveTag = 56,
    kDirectoryTag = 48,
    kStemTag = 40,
    kExtensionTag = 32,
    kDriveDirectoryTag = 24,
    kDirectoryAppendTag = 16,
    kFileNameTag = 8,
    kExtensionAppendTag = 0
};
// Replace a path component with NUL-terminated text, borrowing its allocator-tag slot.
inline void FilenameInline_AssignPart(void* destination, int position, int erasedLength, const char* text,
                                      unsigned char* tags, int slot) {
    tags[slot] = tags[slot + 4];
    unsigned int length = strlen(text);
    const char* end = text;
    StdString_ReplaceRange(destination, position, erasedLength, text, end + length, tags + slot);
}
// Replace a path component from a borrowed byte range using the specified allocator tag.
inline void FilenameInline_ReplacePart(void* destination, int position, int erasedLength, const char* first,
                                       const char* last, unsigned char* tags, int slot) {
    tags[slot] = tags[slot + 4];
    StdString_ReplaceRange(destination, position, erasedLength, first, last, tags + slot);
}
// Remove the final drive character when the drive string is nonempty.
inline void FilenameInline_RemoveDriveColon(char* s) {
    unsigned int small = !(*(unsigned int*)s >> kHeapStringFlagShift);
    unsigned int size = small ? (*(unsigned char*)s & kInlineStringLengthMask) : *(unsigned int*)(s + 4);
    if (size) {
        unsigned int index = (small ? (*(unsigned char*)s & kInlineStringLengthMask) : *(unsigned int*)(s + 4)) - 1;
        char* ptr = small ? s + 1 : *(char**)(s + 8);
        ptr[index] = 0;
    }
}
// Split input into cached path parts and return the stem, preserving allocator-tag slots.
inline const char* FilenameInline_Parse(void* self, unsigned char* temps) {
    char d[kPathPartBufferBytes] = {0};
    char c[kPathPartBufferBytes] = {0};
    char b[kPathPartBufferBytes] = {0};
    char a[kPathPartBufferBytes] = {0};
    char* s = (char*)self;
    const char* src;
    if ((AsFilenameString(s)->input.flags >> kHeapStringFlagShift) == 0)
        src = s + 1;
    else
        src = AsFilenameString(s)->input.data;
    FileIO_SplitPath(src, a, b, c, d);

    unsigned int cap;
    if ((AsFilenameString(s)->drive.flags >> kHeapStringFlagShift) == 0)
        cap = ((unsigned char*)&AsFilenameString(s)->drive.flags)[0] & kInlineStringLengthMask;
    else
        cap = AsFilenameString(s)->drive.length;
    FilenameInline_AssignPart(s + kFilenameDriveOffset, 0, (int)cap, a, temps, kDriveTag);

    if ((AsFilenameString(s)->directory.flags >> kHeapStringFlagShift) == 0)
        cap = ((unsigned char*)&AsFilenameString(s)->directory.flags)[0] & kInlineStringLengthMask;
    else
        cap = AsFilenameString(s)->directory.length;
    FilenameInline_AssignPart(s + kFilenameDirectoryOffset, 0, (int)cap, b, temps, kDirectoryTag);

    if ((AsFilenameString(s)->stem.flags >> kHeapStringFlagShift) == 0)
        cap = ((unsigned char*)&AsFilenameString(s)->stem.flags)[0] & kInlineStringLengthMask;
    else
        cap = AsFilenameString(s)->stem.length;
    FilenameInline_AssignPart(s + kFilenameStemOffset, 0, (int)cap, c, temps, kStemTag);

    if ((AsFilenameString(s)->extension.flags >> kHeapStringFlagShift) == 0)
        cap = ((unsigned char*)&AsFilenameString(s)->extension.flags)[0] & kInlineStringLengthMask;
    else
        cap = AsFilenameString(s)->extension.length;
    FilenameInline_AssignPart(s + kFilenameExtensionOffset, 0, (int)cap, d, temps, kExtensionTag);

    FilenameInline_RemoveDriveColon((char*)s + kFilenameDriveOffset);

    if ((AsFilenameString(s)->driveDirectory.flags >> kHeapStringFlagShift) == 0)
        cap = ((unsigned char*)&AsFilenameString(s)->driveDirectory.flags)[0] & kInlineStringLengthMask;
    else
        cap = AsFilenameString(s)->driveDirectory.length;
    FilenameInline_AssignPart(s + kFilenameDriveDirectoryOffset, 0, (int)cap, a, temps, kDriveDirectoryTag);
    if ((AsFilenameString(s)->driveDirectory.flags >> kHeapStringFlagShift) == 0)
        cap = ((unsigned char*)&AsFilenameString(s)->driveDirectory.flags)[0] & kInlineStringLengthMask;
    else
        cap = AsFilenameString(s)->driveDirectory.length;
    FilenameInline_AssignPart(s + kFilenameDriveDirectoryOffset, (int)cap, 0, b, temps, kDirectoryAppendTag);

    if ((AsFilenameString(s)->fileName.flags >> kHeapStringFlagShift) == 0 &&
        (AsFilenameString(s)->stem.flags >> kHeapStringFlagShift) == 0) {
        AsFilenameString(s)->fileName.flags = AsFilenameString(s)->stem.flags;
        AsFilenameString(s)->fileName.length = AsFilenameString(s)->stem.length;
        (*(unsigned int*)&AsFilenameString(s)->fileName.data) = (*(unsigned int*)&AsFilenameString(s)->stem.data);
    } else {
        unsigned int oldcap =
            (AsFilenameString(s)->fileName.flags >> kHeapStringFlagShift)
                ? AsFilenameString(s)->fileName.length
                : (((unsigned char*)&AsFilenameString(s)->fileName.flags)[0] & kInlineStringLengthMask);
        unsigned int n;
        char* p;
        if ((AsFilenameString(s)->stem.flags >> kHeapStringFlagShift) == 0) {
            n = ((unsigned char*)&AsFilenameString(s)->stem.flags)[0] & kInlineStringLengthMask;
            p = s + kFilenameStemOffset + 1;
        } else {
            p = AsFilenameString(s)->stem.data;
            n = AsFilenameString(s)->stem.length;
        }
        FilenameInline_ReplacePart(s + kFilenameFileNameOffset, 0, (int)oldcap, p, p + n, temps, kFileNameTag);
    }
    if ((AsFilenameString(s)->fileName.flags >> kHeapStringFlagShift) == 0)
        cap = ((unsigned char*)&AsFilenameString(s)->fileName.flags)[0] & kInlineStringLengthMask;
    else
        cap = AsFilenameString(s)->fileName.length;
    unsigned int n;
    char* p;
    if ((AsFilenameString(s)->extension.flags >> kHeapStringFlagShift) == 0) {
        n = ((unsigned char*)&AsFilenameString(s)->extension.flags)[0] & kInlineStringLengthMask;
        p = s + kFilenameExtensionOffset + 1;
    } else {
        p = AsFilenameString(s)->extension.data;
        n = AsFilenameString(s)->extension.length;
    }
    FilenameInline_ReplacePart(s + kFilenameFileNameOffset, (int)cap, 0, p, p + n, temps, kExtensionAppendTag);
    if ((AsFilenameString(s)->stem.flags >> kHeapStringFlagShift) == 0)
        return s + kFilenameStemOffset + 1;
    return AsFilenameString(s)->stem.data;
}
// Retain the native type name because exception relocations reference its destructor.
struct AC464_Local {
    unsigned char data[sizeof(FilenameString)];
    // Construct the music filename cache using borrowed allocator-tag scratch.
    AC464_Local(const char* name, unsigned char* temps) {
        FilenameInline_Construct(data, name, temps);
    }
    // Release the temporary music filename cache strings.
    ~AC464_Local() {
        FilenameOwnedStringView* s = (FilenameOwnedStringView*)data;
        if (s[7].flags >> kHeapStringFlagShift)
            __dl__FPv(s[7].data);
        if (s[6].flags >> kHeapStringFlagShift)
            __dl__FPv(s[6].data);
        if (s[5].flags >> kHeapStringFlagShift)
            __dl__FPv(s[5].data);
        if (s[4].flags >> kHeapStringFlagShift)
            __dl__FPv(s[4].data);
        if (s[3].flags >> kHeapStringFlagShift)
            __dl__FPv(s[3].data);
        if (s[2].flags >> kHeapStringFlagShift)
            __dl__FPv(s[2].data);
        if (s[1].flags >> kHeapStringFlagShift)
            __dl__FPv(s[1].data);
        if (s[0].flags >> kHeapStringFlagShift)
            __dl__FPv(s[0].data);
    }
};
typedef AC464_Local MusicChangeLocal;
// Retain the native type name because exception relocations reference its destructor.
struct AC464_Frame {
    unsigned char temps[72];
    MusicChangeLocal local;
    // Construct filename-cache storage and its adjacent allocator-tag scratch.
    AC464_Frame(const char* name) : local(name, temps) {}
};
typedef AC464_Frame MusicChangeFrame;
// Read inline or heap string length for music-transition cleanup.
inline unsigned int MusicChange_StringLength(unsigned char* s) {
    unsigned int cap;
    if (!(AsMWStringStorage(s)->flags >> kHeapStringFlagShift))
        cap = *(unsigned char*)&AsMWStringStorage(s)->flags & kInlineStringLengthMask;
    else
        cap = AsMWStringStorage(s)->length;
    return cap;
}
// Reset a stream slot using three caller-supplied allocator-tag bytes.
inline void MusicChange_ResetStream(unsigned char* item, unsigned char* p, unsigned char* temp, unsigned char c16,
                                    unsigned char c8, unsigned char c0) {
    PlaybackAtSlotOrigin(item)->source = 0;
    PlaybackAtSlotOrigin(item)->volume = 1.0f;
    PlaybackAtSlotOrigin(item)->pitch = 1.0f;
    PlaybackAtSlotOrigin(item)->playState = kStreamIdle;
    unsigned int cap = MusicChange_StringLength(p + kPlaybackNameOffset);
    temp[16] = c16;
    String_ReplaceRange(p + kPlaybackNameOffset, 0, cap, s_soundPlaybackEmptyName,
                        s_soundPlaybackEmptyName + strlen(s_soundPlaybackEmptyName), temp + 16);
    (*(char*)&PlaybackAtSlotOrigin(item)->loopEnabled) = 0;
    (*(char*)&PlaybackAtSlotOrigin(item)->fadeInEnabled) = 0;
    (*(char*)&PlaybackAtSlotOrigin(item)->useDolbyBank) = 0;
    PlaybackAtSlotOrigin(item)->transitionDuration = 0.0f;
    cap = MusicChange_StringLength(p + kPlaybackFilePathOffset);
    temp[8] = c8;
    String_ReplaceRange(p + kPlaybackFilePathOffset, 0, cap, s_soundPlaybackEmptyFilePath,
                        s_soundPlaybackEmptyFilePath + strlen(s_soundPlaybackEmptyFilePath), temp + 8);
    cap = MusicChange_StringLength(p + kPlaybackResourcePathOffset);
    temp[0] = c0;
    String_ReplaceRange(p + kPlaybackResourcePathOffset, 0, cap, s_soundPlaybackEmptyResourcePath,
                        s_soundPlaybackEmptyResourcePath + strlen(s_soundPlaybackEmptyResourcePath), temp);
    FloatTween_Start(item + 0x28, kFadeDefault, 0.0f, 0.0f);
    PlaybackAtSlotOrigin(item)->baseVolume = 1.0f;
    PlaybackAtSlotOrigin(item)->basePitch = 1.0f;
    String_ReserveCapacity(item + 0x14, 0x40);
    String_ReserveCapacity(item + 0x44, 0x10);
    String_ReserveCapacity(item + 0x50, 0x10);
}
// Apply volume to a playback record whose address is four bytes after the group origin.
// Apply volume, resource gain and fade value to a playback record.
inline void MusicChange_ApplyVolume(unsigned char* entry, float value) {
    AsSoundPlaybackState(entry)->volume = value;
    if (*(void**)entry) {
        float f1 = AsSoundPlaybackState(entry)->baseVolume;
        float f0 = AsSoundPlaybackState(entry)->volume;
        float f2 = AsSoundPlaybackState(entry)->fadeValue;
        f0 = f1 * f0;
        f1 = f0 * f2;
        ((gtl::ISoundStream*)(*(void**)entry))->SetVolume(f1);
    }
}
#pragma inline_depth(8)
#pragma inline_max_size(10000)
#pragma inline_max_total_size(20000)
// Compare a filename stem with a group slot, using scoped string cleanup.
inline int MusicChange_NameMatches(unsigned char* entry, const char* name, unsigned char* temps) {
    if (!name)
        return 0;
    MusicChangeLocal local(name, temps);
    const char* part = FilenameInline_Parse(local.data, temps);
    unsigned int length = strlen(part);
    if (!length)
        return 0;
    unsigned int small = !(PlaybackAtSlotOrigin(entry)->nameFlags >> kHeapStringFlagShift);
    unsigned int size = small ? (((char*)&PlaybackAtSlotOrigin(entry)->nameFlags)[0] & kInlineStringLengthMask)
                              : PlaybackAtSlotOrigin(entry)->nameLength;
    if (length != size)
        return 0;
    const char* existing = small ? (const char*)entry + 0x15 : (*(char**)&PlaybackAtSlotOrigin(entry)->nameData);
    return !String_ComparePrefixIgnoreCase((void*)part, existing, length);
}
// Test whether the selected slot has the requested filename and a non-idle state.
inline int MusicChange_IsActive(unsigned char* state, const char* name, unsigned char* temps) {
    if (!name)
        return 0;
    if (!MusicChange_NameMatches(state + *(int*)state * kStreamSlotBytes, name, temps))
        return 0;
    return *(int*)(state + *(int*)state * kStreamSlotBytes + 0x10) != 0;
}
// Release all eight cached path strings in reverse order.
inline void MusicChange_DestroyFilename(unsigned char* data) {
    FilenameOwnedStringView* s = (FilenameOwnedStringView*)data;
    if (s[7].flags >> kHeapStringFlagShift)
        __dl__FPv(s[7].data);
    if (s[6].flags >> kHeapStringFlagShift)
        __dl__FPv(s[6].data);
    if (s[5].flags >> kHeapStringFlagShift)
        __dl__FPv(s[5].data);
    if (s[4].flags >> kHeapStringFlagShift)
        __dl__FPv(s[4].data);
    if (s[3].flags >> kHeapStringFlagShift)
        __dl__FPv(s[3].data);
    if (s[2].flags >> kHeapStringFlagShift)
        __dl__FPv(s[2].data);
    if (s[1].flags >> kHeapStringFlagShift)
        __dl__FPv(s[1].data);
    if (s[0].flags >> kHeapStringFlagShift)
        __dl__FPv(s[0].data);
}
// Compare a filename stem using caller-provided string storage and explicit cleanup.
inline int MusicChange_NameMatchesWithScratch(unsigned char* entry, unsigned char* data, const char* name,
                                              unsigned char* temps) {
    if (!name)
        return 0;
    FilenameInline_Construct(data, name, temps);
    const char* part = FilenameInline_Parse(data, temps);
    unsigned int length = strlen(part);
    if (!length) {
        MusicChange_DestroyFilename(data);
        return 0;
    }
    unsigned int small = !(PlaybackAtSlotOrigin(entry)->nameFlags >> kHeapStringFlagShift);
    unsigned int size = small ? (((char*)&PlaybackAtSlotOrigin(entry)->nameFlags)[0] & kInlineStringLengthMask)
                              : PlaybackAtSlotOrigin(entry)->nameLength;
    if (length != size) {
        MusicChange_DestroyFilename(data);
        return 0;
    }
    const char* existing = small ? (const char*)entry + 0x15 : (*(char**)&PlaybackAtSlotOrigin(entry)->nameData);
    int result = !String_ComparePrefixIgnoreCase((void*)part, existing, length);
    MusicChange_DestroyFilename(data);
    return result;
}
// Test the selected slot name/state using caller-provided filename scratch storage.
inline int MusicChange_IsActiveWithScratch(unsigned char* state, unsigned char* data, const char* name,
                                           unsigned char* temps) {
    if (!name)
        return 0;
    if (!MusicChange_NameMatchesWithScratch(state + *(int*)state * kStreamSlotBytes, data, name, temps))
        return 0;
    return *(int*)(state + *(int*)state * kStreamSlotBytes + 0x10) != 0;
}
// Crossfade or reuse music by filename and output mode, resetting voice slots when required.
void SoundManager_RequestBgmChange(unsigned char* self, const char* name, int playOnce, int allowFade) {
    if (!AsSoundManagerState(self)->enabled)
        return;
    FilenameString matchFilename;
    FilenameString loadFilename;
    unsigned char filenameAllocatorTags[72]; // Nine ABI scratch slots; no ownership.
    // Phase 1: compare the selected music stem and decide whether voices reset.
    unsigned char* state = (*(unsigned char**)&AsSoundManagerState(self)->state);
    int requestedMusicIsActive =
        MusicChange_IsActiveWithScratch(state, (unsigned char*)&matchFilename, name, filenameAllocatorTags);
    // A music replacement also cancels voice playback: first release both voice
    // handles/files, then repeat the value/string reset on the SAME two records.
    // The second pass neither releases handles again nor touches the music slots.
    if (!requestedMusicIsActive || !allowFade) {
        // Allocator-tag scratch is intentionally uninitialized in the existing ABI.
        // The copied bytes below are not bank IDs, playback flags or owned handles;
        // preserve the tag passing while editing reset policy.
        unsigned char temp1[24];
        unsigned char t1a = temp1[20], t1b = temp1[12], t1c = temp1[4];
        // slotOrigin precedes playbackRecord by four bytes; only entries owns selected.
        unsigned char* entries = (*(unsigned char**)&AsSoundManagerState(self)->state) + kVoiceGroupOffset;
        unsigned char* slotOrigin = entries;
        unsigned char* playbackRecord = entries + 4;
        for (unsigned int i = 0; i < kPlaybackSlotsPerGroup;
             ++i, slotOrigin += kStreamSlotBytes, playbackRecord += kStreamSlotBytes) {
            if (PlaybackAtSlotOrigin(slotOrigin)->source) {
                ((gtl::ISoundStream*)(PlaybackAtSlotOrigin(slotOrigin)->source))->Stop(0);
                void* obj = PlaybackAtSlotOrigin(slotOrigin)->source;
                if (--*(int*)obj == 0 && obj)
                    delete (DtorRef*)obj;
            }
            if (PlaybackAtSlotOrigin(slotOrigin)->sharedFile) {
                CSharedFileHandle::Destroy(PlaybackAtSlotOrigin(slotOrigin)->sharedFile, 1);
                PlaybackAtSlotOrigin(slotOrigin)->sharedFile = 0;
            }
            MusicChange_ResetStream(slotOrigin, playbackRecord, temp1, t1a, t1b, t1c);
        }
        *(int*)entries = 0;
        unsigned char temp2[24];
        unsigned char t2a = temp2[20], t2b = temp2[12], t2c = temp2[4];
        unsigned char* secondPassRecord = entries + 4;
        for (unsigned int i = 0; i < kPlaybackSlotsPerGroup;
             ++i, entries += kStreamSlotBytes, secondPassRecord += kStreamSlotBytes)
            MusicChange_ResetStream(entries, secondPassRecord, temp2, t2a, t2b, t2c);
    }
    // Phase 2: selected slot -> other slot -> idle slot -> forced eviction.
    unsigned int useDolbyBank = (*(int*)&AsSoundManagerState(self)->outputMode) == kSoundDolbyProLogicII;
    float targetVolume = AsSoundManagerState(self)->volume;
    float fadeSeconds = AsSoundManagerState(self)->transitionDuration;
    unsigned char* musicGroupOrigin = (*(unsigned char**)&AsSoundManagerState(self)->state);
    if (!name || strlen(name) == 0) {
        SoundChannelGroup_StopWithFade(musicGroupOrigin, fadeSeconds, allowFade);
        return;
    }
    unsigned char* playbackRecord =
        musicGroupOrigin + AsSoundChannelGroup(musicGroupOrigin)->selected * kStreamSlotBytes + 4;
    if (SoundPlayback_MatchesFileName(playbackRecord, name) &&
        useDolbyBank == AsSoundPlaybackState(playbackRecord)->useDolbyBank) {
        MusicChange_ApplyVolume(playbackRecord, targetVolume);
        SoundStreamPlayback_Start(playbackRecord, !playOnce, fadeSeconds, allowFade, 0);
        return;
    }
    SoundChannelGroup_StopWithFade(musicGroupOrigin, fadeSeconds, allowFade);
    playbackRecord =
        musicGroupOrigin + ((AsSoundChannelGroup(musicGroupOrigin)->selected + 1) & 1) * kStreamSlotBytes + 4;
    if (SoundPlayback_MatchesFileName(playbackRecord, name) &&
        useDolbyBank == AsSoundPlaybackState(playbackRecord)->useDolbyBank) {
        MusicChange_ApplyVolume(playbackRecord, targetVolume);
        SoundStreamPlayback_Start(playbackRecord, !playOnce, fadeSeconds, allowFade, 0);
        AsSoundChannelGroup(musicGroupOrigin)->selected = (AsSoundChannelGroup(musicGroupOrigin)->selected + 1) & 1;
        return;
    }
    int foundIdleSlot = 0;
    unsigned char* candidateSlotOrigin = musicGroupOrigin;
    for (int i = 0; i < kPlaybackSlotsPerGroup; ++i, candidateSlotOrigin += kStreamSlotBytes) {
        if (PlaybackAtSlotOrigin(candidateSlotOrigin)->playState == kStreamIdle) {
            AsSoundChannelGroup(musicGroupOrigin)->selected = i & 1;
            foundIdleSlot = 1;
            break;
        }
    }
    if (!foundIdleSlot) {
        AsSoundChannelGroup(musicGroupOrigin)->selected = (AsSoundChannelGroup(musicGroupOrigin)->selected + 1) & 1;
        SoundPlayback_StopWithOptionalFade(
            musicGroupOrigin + AsSoundChannelGroup(musicGroupOrigin)->selected * kStreamSlotBytes + 4, 0.0f, 0);
    }
    // Phase 3: parse/load the bank, start only on success, then release filename strings.
    FilenameString_Ctor(&loadFilename, name);
    const char* text = FilenameString_SplitAndGetStem(&loadFilename);
    playbackRecord = musicGroupOrigin + AsSoundChannelGroup(musicGroupOrigin)->selected * kStreamSlotBytes + 4;
    if (SoundPlaybackState_BeginBankLoad(playbackRecord, text, useDolbyBank)) {
        MusicChange_ApplyVolume(playbackRecord, targetVolume);
        SoundStreamPlayback_Start(playbackRecord, !playOnce, fadeSeconds, allowFade, 1);
    }
    FilenameString_dtor((unsigned char*)&loadFilename, -1);
}
#pragma inline_depth reset
#pragma inline_max_size reset
#pragma inline_max_total_size reset
// Compare the requested filename stem with the stream name, ignoring case and requiring equal lengths.
int SoundPlayback_MatchesFileName(void* selfArg, const char* name) {
    unsigned char* self = (unsigned char*)selfArg;
    if (!name)
        return 0;
    MusicChangeFrame frame(name);
    const char* part = FilenameInline_Parse(frame.local.data, frame.temps);
    unsigned int length = strlen(part);
    if (length == 0)
        return 0;
    unsigned int small = !(AsSoundPlaybackState(self)->nameFlags >> kHeapStringFlagShift);
    unsigned int size = small ? (((char*)&AsSoundPlaybackState(self)->nameFlags)[0] & kInlineStringLengthMask)
                              : AsSoundPlaybackState(self)->nameLength;
    if (length != size)
        return 0;
    const char* existing = small ? (const char*)self + 0x11 : (*(char**)&AsSoundPlaybackState(self)->nameData);
    return !String_ComparePrefixIgnoreCase((void*)part, existing, length);
}

struct BankPathAllocatorTag {
    unsigned char b; /* Preserve the empty allocator-tag constructor. */
    BankPathAllocatorTag() {}
};
// Read the inline or heap length of a bank-path string.
static inline unsigned int BankPath_Length(unsigned char* s) {
    return !(AsMWStringStorage(s)->flags >> kHeapStringFlagShift)
               ? (unsigned int)(*(unsigned char*)&AsMWStringStorage(s)->flags & kInlineStringLengthMask)
               : AsMWStringStorage(s)->length;
}
// Replace a range using a by-value allocator tag and the source text length.
static inline void BankPath_Replace(unsigned char* s, unsigned int pos, unsigned int n, const char* src,
                                    BankPathAllocatorTag t) {
    String_ReplaceRange(s, pos, n, src, src + strlen(src), &t.b);
}
// Replace a bank-path string with the supplied text.
static inline void BankPath_Assign(unsigned char* s, const char* src) {
    unsigned int cap = BankPath_Length(s);
    BankPath_Replace(s, 0, cap, src, BankPathAllocatorTag());
}
// Append text at the current bank-path string length.
static inline void BankPath_Append(unsigned char* s, const char* src) {
    unsigned int cap = BankPath_Length(s);
    BankPath_Replace(s, cap, 0, src, BankPathAllocatorTag());
}
// Select inline characters or the heap pointer from a bank-path string.
static inline const char* BankPath_CStr(unsigned char* s) {
    return !(AsMWStringStorage(s)->flags >> kHeapStringFlagShift) ? (const char*)&AsMWStringStorage(s)->flags + 1
                                                                  : AsMWStringStorage(s)->data;
}

#pragma use_lmw_stmw on
// Resolve metadata and sample paths, request the metadata file, and enter the loading state.
int SoundPlaybackState_BeginBankLoad(void* selfArg, const char* name, int useDolbyBank) {
    unsigned char* self = (unsigned char*)selfArg;
    if (AsSoundPlaybackState(self)->source != 0)
        return 0;
    if (AsSoundPlaybackState(self)->playState != kStreamIdle)
        return 0;
    const char* suffix = s_standardSoundBankSuffix;
    if (useDolbyBank)
        suffix = s_dolbySoundBankSuffix;
    BankPath_Assign(self + 0x40, name);
    BankPath_Append(self + 0x40, suffix);
    BankPath_Append(self + 0x40, s_streamMetadataExtension);
    const char* r = (const char*)CHashPath::FindPath(BankPath_CStr(self + 0x40));
    if (r == 0)
        return 0;
    BankPath_Assign(self + 0x40, r);
    BankPath_Assign(self + 0x4c, name);
    BankPath_Append(self + 0x4c, suffix);
    BankPath_Append(self + 0x4c, s_streamSampleExtension);
    r = (const char*)CHashPath::FindPath(BankPath_CStr(self + 0x4c));
    if (r == 0)
        return 0;
    BankPath_Assign(self + 0x4c, r);
    if (AsSoundPlaybackState(self)->sharedFile) {
        CSharedFileHandle::Destroy(AsSoundPlaybackState(self)->sharedFile, 1);
        AsSoundPlaybackState(self)->sharedFile = 0;
    }
    AsSoundPlaybackState(self)->sharedFile = SharedFile_Create((void*)BankPath_CStr(self + 0x40), 0);
    BankPath_Assign(self + 0x10, name);
    AsSoundPlaybackState(self)->useDolbyBank = (unsigned char)useDolbyBank;
    AsSoundPlaybackState(self)->playState = kStreamLoadingBank;
    return 1;
}

#pragma use_lmw_stmw reset

extern char s_soundMetadataBankDirectory[];
extern char s_soundSampleBankDirectory[];
extern char s_soundBankMetadataPathFormat[7];
extern char s_soundBankSamplePathFormat[7];
extern char s_soundBankMetadataExtension[5];
extern char s_soundBankSampleExtension[5];
void gtl_Audio_CreateSoundBufferResource(void*, void*);
int sprintf(char*, const char*, ...);

struct SoundBankLoadState {
    unsigned int refs;
    void* vt;
    unsigned int flags;
    unsigned int len;
    char* data;
    void* objA;
    void* objB;
    // Initialize the sound-bank load object from a borrowed name and tag scratch.
    SoundBankLoadState(const char* name, unsigned char* temps) {
        refs = 1;
        vt = __vt__20CSoundSourceListNode;
        flags = 0;
        len = 0;
        data = 0;
        unsigned int n = strlen(name);
        String_ReserveCapacity(&flags, n);
        temps[0] = temps[4];
        String_ReplaceRange(&flags, 0, 0, name, name + n, temps);
        objA = 0;
        objB = 0;
    }
};

// Release a resource reference and clear the caller pointer.
static inline void BankLoad_ReleaseReference(void*& p) {
    void* obj = p;
    if (obj) {
        if (--*(int*)obj == 0 && obj != 0) {
            void** vtable = ((SoundResourceHeader*)obj)->vtable;
            ((void (*)(void*, int))vtable[2])(obj, 1);
        }
        p = 0;
    }
}

// Release a newly constructed bank object after a failed load.
static inline void BankLoad_ReleaseBank(SoundBankLoadState* p) {
    if (p) {
        if (--*(int*)p == 0 && p != 0) {
            void** vtable = ((SoundResourceHeader*)p)->vtable;
            ((void (*)(void*, int))vtable[2])(p, 1);
        }
    }
}

struct SoundBankPath {
    char s[0x400];
};

// Test whether a resource path resolves through the shared path table.
static inline bool BankLoad_PathExists(SoundBankPath* pp) {
    SoundBankPath& p = *pp;
    bool r;
    if (&p == 0)
        r = false;
    else
        r = CHashPath::FindPath((const char*)&p) != 0;
    return r;
}

// Load a shared file, create its sound resource and copy the loaded bytes into it.
static inline int BankLoad_ReadResource(SoundBankPath* p, void*& out) {
    void* a = SharedFile_Create((void*)p, 1);
    int b = (int)(void*)CSharedFilePtr::GetSize(a);
    if (!b)
        return 0;
    if (!gtl_Audio_CreateSoundResource((void*)b, &out, 0))
        return 0;
    void* src = CSharedFilePtr_GetLoadedBuffer(a);
    if (!src)
        return 0;
    memcpy(((SoundResourceHeader*)out)->data, src, (unsigned int)b);
    // The .gsp/.gsb bank data is big-endian on disc.
    if (a)
        CSharedFileHandle::Destroy(a, 1);
    return 1;
}

// Allocate a bank-list node chunk as needed and pop its free list.
static inline SoundObjectListNode* BankLoad_AllocateNode(SoundObjectList* l) {
    WInstanceLock outer(l->lock);
    if (l->free == 0) {
        SoundObjectListNode* chunk = (SoundObjectListNode*)appMallocUni__FUl(l->chunkSize * 0x18);
        void* lk1 = l->lock;
        l->chunks = SoundBankList_Insert(l->chunks, chunk, lk1);
        int i = 1;
        if (l->chunkSize > 1) {
            void* lk2 = l->lock;
            for (SoundObjectListNode* p = chunk + 1; i < l->chunkSize; p++, i++) {
                l->free = SoundBankList_Insert(l->free, p, lk2);
            }
        }
    }
    SoundObjectListNode* n = l->free;
    void* lk = l->lock;
    OSLockMutex(lk);
    n->next->prev = n->prev;
    n->prev->next = n->next;
    SoundObjectListNode* next = n->next;
    if (next == n)
        next = 0;
    OSUnlockMutex(lk);
    l->free = next;
    return n;
}

// Return the supplied unsigned value unchanged.
static inline unsigned int BankLoad_Identity(unsigned int v) {
    return v;
}
// Return the smaller size by reference for the bank name comparison.
static inline const unsigned int& BankLoad_Min(const unsigned int& a, const unsigned int& b) {
    return (a < b) ? a : b;
}

// Compare the full bank name lexicographically against the requested key.
static inline int BankLoad_CompareName(SoundBankLoadState* obj, const char* key) {
    unsigned int bankNameLength, storedLengthCopy;
    unsigned int requestedLength = strlen(key);
    unsigned int requestedLengthCopy = requestedLength;
    unsigned int logicalNameLength =
        (obj->flags >> kHeapStringFlagShift) == 0 ? (*(unsigned char*)&obj->flags & kInlineStringLengthMask) : obj->len;
    const char* bankName;
    unsigned int storedNameLength;
    if ((obj->flags >> kHeapStringFlagShift) == 0) {
        bankName = (char*)obj + 9;
        storedNameLength = *(unsigned char*)&obj->flags & kInlineStringLengthMask;
    } else {
        bankName = obj->data;
        storedNameLength = obj->len;
    }
    // Both length reads describe the same full name. Keep the reference temporaries
    // in this string expansion: compare common prefix, then order by full lengths.
    const unsigned int& boundedNameLength = BankLoad_Min(logicalNameLength, storedLengthCopy = storedNameLength);
    bankNameLength = boundedNameLength;
    const unsigned int& commonPrefixLength = BankLoad_Min(requestedLength, bankNameLength);
    int prefixOrder = memcmp((const void*)bankName, (const void*)key, commonPrefixLength);
    if (prefixOrder != 0)
        return prefixOrder;
    if (bankNameLength < requestedLengthCopy)
        return -1;
    if (bankNameLength == requestedLengthCopy)
        return 0;
    return 1;
}

// Load the metadata/sample pair and create a bank resource and sound buffer.
static inline int BankLoad_CreateResources(SoundBankLoadState* obj, SoundBankPath* pathGsp, SoundBankPath* pathGsb,
                                           void*& resA, void*& resB) {
    if (!BankLoad_PathExists(pathGsp))
        return 0;
    if (!BankLoad_PathExists(pathGsb))
        return 0;
    resA = 0;
    resB = 0;
    if (!BankLoad_PathExists(pathGsp))
        return 0;
    if (!BankLoad_ReadResource(pathGsp, resA))
        return 0;
    if (!BankLoad_PathExists(pathGsb))
        return 0;
    if (!BankLoad_ReadResource(pathGsb, resB)) {
        BankLoad_ReleaseReference(resA);
        return 0;
    }
    struct AD0Pair {
        void* a;
        void* b;
    } pair;
    obj->objA = 0;
    pair.a = resA;
    pair.b = resB;
    gtl_Audio_CreateSoundBufferResource(&pair, &obj->objA);
    obj->objB = 0;
    unsigned int desc[7] = {0, 0, 0, 0, 0, 0, 0};
    if (!gtl_Audio_CreateSoundBuffer(desc, &obj->objB)) {
        BankLoad_ReleaseReference(resA);
        BankLoad_ReleaseReference(resB);
        return 0;
    }
    BankLoad_ReleaseReference(resA);
    BankLoad_ReleaseReference(resB);
    return 1;
}

#pragma use_lmw_stmw on
// Reuse a named bank or load its metadata/sample pair and add a reference-counted bank entry.
int SoundManager_AcquireSoundBank(void* self, const char* key) {
    unsigned char ctorTemps[8];
    SoundBankPath metadataPath;

    if (!key)
        return 0;
    SoundBankLoadState* obj;
    unsigned char* base = (unsigned char*)AsSoundManagerState(self)->state;
    SoundObjectList* list = (SoundObjectList*)(base + kBankListHeadOffset);
    obj = (SoundBankLoadState*)SoundBankList_First(list);
    while (obj) {
        if (BankLoad_CompareName(obj, key) == 0)
            break;
        obj = (SoundBankLoadState*)SoundBankList_Next(list);
    }
    if (obj) {
        obj->refs = (obj->refs + 1);
        return 1;
    }
    obj = new SoundBankLoadState(key, ctorTemps);
    __typeof__(obj->data) savedData = obj->data;
    const char* name = (obj->flags >> kHeapStringFlagShift) ? savedData : (char*)obj + 9;

    void* resA;
    sprintf(metadataPath.s, s_soundBankMetadataPathFormat, s_soundMetadataBankDirectory, name,
            s_soundBankMetadataExtension);
    void* resB;
    SoundBankPath samplePath;
    sprintf(samplePath.s, s_soundBankSamplePathFormat, s_soundSampleBankDirectory, name, s_soundBankSampleExtension);
    if (BankLoad_CreateResources(obj, &metadataPath, &samplePath, resA, resB)) {
        WInstanceLock outer(list->lock);
        SoundObjectListNode* n = BankLoad_AllocateNode(list);
        n->item = (SoundBufferInstanceView*)obj;
        n->flag = 0;
        void* lk = list->lock;
        n->value = 0;
        list->head = SoundBankList_Insert(list->head, n, lk);
        return 1;
    }
    BankLoad_ReleaseBank(obj);
    return 0;
}
#pragma use_lmw_stmw reset

extern const unsigned int kDefaultSoundStreamDescription[7];

struct StreamPlaybackParameters {
    void* w[7];
};

// Release a temporary stream resource reference and clear its pointer.
static inline void StreamUpdate_ReleaseReference(void*& p) {
    void* obj = p;
    if (obj) {
        if (--*(int*)obj == 0 && obj != 0) {
            void** vtable = ((SoundResourceHeader*)obj)->vtable;
            ((void (*)(void*, int))vtable[2])(obj, 1);
        }
        p = 0;
    }
}

// Read a stream string length using separate flags and heap-length addresses.
static inline unsigned int StreamUpdate_StringLength(unsigned char* flags, unsigned char* size) {
    return !(*(unsigned int*)flags >> kHeapStringFlagShift) ? (unsigned int)(*flags & kInlineStringLengthMask)
                                                            : *(unsigned int*)size;
}

// Apply per-stream volume, resource gain and the current fade multiplier.
static inline void StreamUpdate_ApplyVolume(unsigned char* item) {
    if (PlaybackAtSlotOrigin(item)->source) {
        float a = PlaybackAtSlotOrigin(item)->baseVolume;
        float b = PlaybackAtSlotOrigin(item)->volume;
        a = a * b;
        ((gtl::ISoundStream*)(PlaybackAtSlotOrigin(item)->source))
            ->SetVolume(a * PlaybackAtSlotOrigin(item)->fadeValue);
    }
}

// Release and reset a stream slot after playback or fade-out completes.
static inline void StreamUpdate_ResetSlot(unsigned char* item, unsigned char* entry, unsigned char* temp) {
    reset_rc((void**)(item + 4));
    if (PlaybackAtSlotOrigin(item)->sharedFile) {
        CSharedFileHandle::Destroy(PlaybackAtSlotOrigin(item)->sharedFile, 1);
        PlaybackAtSlotOrigin(item)->sharedFile = 0;
    }
    PlaybackAtSlotOrigin(item)->source = 0;
    PlaybackAtSlotOrigin(item)->volume = 1.0f;
    PlaybackAtSlotOrigin(item)->pitch = 1.0f;
    PlaybackAtSlotOrigin(item)->playState = kStreamIdle;
    unsigned int cap = StreamUpdate_StringLength(entry + kPlaybackNameOffset, item + 0x18);
    temp[16] = temp[20];
    String_ReplaceRange(entry + kPlaybackNameOffset, 0, cap, s_soundPlaybackEmptyName,
                        s_soundPlaybackEmptyName + strlen(s_soundPlaybackEmptyName), temp + 16);
    (*(char*)&PlaybackAtSlotOrigin(item)->loopEnabled) = 0;
    (*(char*)&PlaybackAtSlotOrigin(item)->fadeInEnabled) = 0;
    (*(char*)&PlaybackAtSlotOrigin(item)->useDolbyBank) = 0;
    PlaybackAtSlotOrigin(item)->transitionDuration = 0.0f;
    cap = StreamUpdate_StringLength(entry + kPlaybackFilePathOffset, item + 0x48);
    temp[8] = temp[12];
    String_ReplaceRange(entry + kPlaybackFilePathOffset, 0, cap, s_soundPlaybackEmptyFilePath,
                        s_soundPlaybackEmptyFilePath + strlen(s_soundPlaybackEmptyFilePath), temp + 8);
    cap = StreamUpdate_StringLength(entry + kPlaybackResourcePathOffset, item + 0x54);
    temp[0] = temp[4];
    String_ReplaceRange(entry + kPlaybackResourcePathOffset, 0, cap, s_soundPlaybackEmptyResourcePath,
                        s_soundPlaybackEmptyResourcePath + strlen(s_soundPlaybackEmptyResourcePath), temp);
    FloatTween_Start(entry + kPlaybackFadeOffset, kFadeDefault, kFloat_0_soundPlayback, kFloat_0_soundPlayback);
    PlaybackAtSlotOrigin(item)->baseVolume = 1.0f;
    PlaybackAtSlotOrigin(item)->basePitch = 1.0f;
    String_ReserveCapacity(entry + kPlaybackNameOffset, 0x40);
    String_ReserveCapacity(entry + kPlaybackFilePathOffset, 0x10);
    String_ReserveCapacity(entry + kPlaybackResourcePathOffset, 0x10);
}

// Create stream resources from the loaded metadata and sample path, then retrieve resource gains.
static inline int StreamUpdate_FinishBankLoad(unsigned char* item, unsigned char* entry) {
    void* a = CSharedFilePtr_GetLoadedBuffer(PlaybackAtSlotOrigin(item)->sharedFile);
    if (!a)
        return 0;
    void* b = (void*)CSharedFilePtr::GetSize(PlaybackAtSlotOrigin(item)->sharedFile);
    if (!b)
        return 0;
    void* buf = 0;
    if (!gtl_Audio_CreateSoundResource(b, &buf, 0))
        return 0;
    memcpy(((SoundResourceHeader*)buf)->data, a, (unsigned int)b);
    // The stream bank's .gsp is big-endian on disc.
    void* obj = 0;
    struct AAD2CKey {
        void* res;
        const char* name;
    } key;
    key.res = buf;
    const char* name;
    if (AsSoundPlaybackState(entry)->resourcePathFlags >> kHeapStringFlagShift)
        name = (*(char**)&PlaybackAtSlotOrigin(item)->resourcePathData);
    else
        name = (const char*)(entry + 0x4d);
    key.name = name;
    if (!gtl_Audio_CreateSoundStreamResource(&key, &obj)) {
        StreamUpdate_ReleaseReference(buf);
        return 0;
    }
    StreamUpdate_ReleaseReference(buf);
    StreamPlaybackParameters args = *(StreamPlaybackParameters*)kDefaultSoundStreamDescription;
    if (!gtl_Audio_CreateSoundStream(&args, entry)) {
        StreamUpdate_ReleaseReference(obj);
        return 0;
    }
    void* h = PlaybackAtSlotOrigin(item)->source;
    if (!((int (*)(void*, int, void*))(((SoundResourceHeader*)obj)->vtable)[5])(obj, 0, h)) {
        StreamUpdate_ReleaseReference(obj);
        StreamUpdate_ReleaseReference(PlaybackAtSlotOrigin(item)->source);
        return 0;
    }
    float f[24]; // the callee fills a 0x60-byte description; the first two words are used here
    if (((int (*)(void*, int, float*))(((SoundResourceHeader*)obj)->vtable)[6])(obj, 0, f)) {
        PlaybackAtSlotOrigin(item)->baseVolume = f[0];
        PlaybackAtSlotOrigin(item)->basePitch = f[1];
    }
    StreamUpdate_ReleaseReference(obj);
    return 1;
}

// Start a successfully loaded bank with its loop mode, volume and pitch.
static inline void StartLoadedStream(unsigned char* slotOrigin) {
    int loop = PlaybackAtSlotOrigin(slotOrigin)->loopEnabled;
    if (PlaybackAtSlotOrigin(slotOrigin)->source) {
        StreamUpdate_ApplyVolume(slotOrigin);
        if (PlaybackAtSlotOrigin(slotOrigin)->source)
            ((gtl::ISoundStream*)(PlaybackAtSlotOrigin(slotOrigin)->source))
                ->SetPitch(PlaybackAtSlotOrigin(slotOrigin)->basePitch * PlaybackAtSlotOrigin(slotOrigin)->pitch);
        int m = loop ? kPlayLooped : kPlayOnce;
        ((gtl::ISoundStream*)(PlaybackAtSlotOrigin(slotOrigin)->source))->Play(m);
        PlaybackAtSlotOrigin(slotOrigin)->playState = kStreamPlaying;
    }
}

// Advance kStreamPlaying and apply its transition side effects to this borrowed slot.
static inline void AdvancePlayingStream(unsigned char* slotOrigin, unsigned char* playbackBytes, float dt,
                                        unsigned char* finishedResetTags, unsigned int& flags) {
    if (FloatTween_Update(playbackBytes + kPlaybackFadeOffset, dt)) {
        StreamUpdate_ApplyVolume(slotOrigin);
    }
    if (((gtl::ISoundStream*)(PlaybackAtSlotOrigin(slotOrigin)->source))->GetStatusFlags(&flags) &&
        (flags & kSoundStatusFinished) && !(flags & kSoundStatusPlaying) && !(flags & kSoundStatusPending)) {
        FloatTween_Start(playbackBytes + kPlaybackFadeOffset, kFadeDefault, kFloat_0_soundPlayback,
                         kFloat_0_soundPlayback);
        StreamUpdate_ResetSlot(slotOrigin, playbackBytes, finishedResetTags);
    }
}

// Advance kStreamFadingOut and apply its transition side effects to this borrowed slot.
static inline void AdvanceFadingStream(unsigned char* slotOrigin, unsigned char* playbackBytes, float dt,
                                       unsigned char* fadeResetTags) {
    if (!FloatTween_Update(playbackBytes + kPlaybackFadeOffset, dt)) {
        StreamUpdate_ResetSlot(slotOrigin, playbackBytes, fadeResetTags);
    }
    StreamUpdate_ApplyVolume(slotOrigin);
}

// Advance each stream through idle, bank loading, playing and fade-out states.
// Transition table (dt is seconds):
// Idle: no work. BeginBankLoad enters LoadingBank outside this update.
// LoadingBank: wait/retry until bank creation succeeds; successful Play enters Playing.
// Playing: update fade-in/volume; finished without Playing/Pending bits resets to Idle.
// FadingOut: update fade; tween completion releases/reset to Idle, then applies volume.
// StopWithOptionalFade enters FadingOut only for a positive allowed fade and a
// non-loading slot; otherwise it releases/reset to Idle immediately.
// Slot origin = group header + index*record size; record = origin+4. Both pointers
// are borrowed views of the same slot, never separate playback owners.
void SoundManager_UpdateStreamChannels(void* data, float dt) {
    // Opaque allocator-tag scratch passed to string replacement during reset.
    // These arrays have no initialized semantic state; do not interpret their bytes
    // as stream decisions or add default values in a readability-only change.
    unsigned char finishedResetTags[24];
    unsigned char fadeResetTags[24];
    unsigned char* slotOrigin = (unsigned char*)data;
    unsigned char* playbackBytes = (slotOrigin + kPlaybackRecordFromSlotOrigin);
    for (unsigned int i = 0; i < 2; ++i, slotOrigin += kStreamSlotBytes, playbackBytes += kStreamSlotBytes) {
        unsigned int state = PlaybackAtSlotOrigin(slotOrigin)->playState;
        switch (state) {
        case kStreamIdle: // No resources are being advanced.
            break;
        case kStreamLoadingBank: { // Resolve the pending bank, then start playback.
            int bankReady = 0;
            if (StreamUpdate_StringLength(playbackBytes + kPlaybackFilePathOffset, slotOrigin + 0x48) &&
                StreamUpdate_StringLength(playbackBytes + kPlaybackResourcePathOffset, slotOrigin + 0x54) &&
                !PlaybackAtSlotOrigin(slotOrigin)->source && state == kStreamLoadingBank) {
                bankReady = StreamUpdate_FinishBankLoad(slotOrigin, playbackBytes);
            }
            if (bankReady)
                StartLoadedStream(slotOrigin);
            break;
        }
        case kStreamPlaying: { // Apply tween gains and retire a finished stream.
            unsigned int flags;
            AdvancePlayingStream(slotOrigin, playbackBytes, dt, finishedResetTags, flags);
            break;
        }
        case kStreamFadingOut: { // Complete the fade and release its owned resources.
            AdvanceFadingStream(slotOrigin, playbackBytes, dt, fadeResetTags);
            break;
        }
        }
    }
}

// Retain the native type name because exception relocations reference its destructor.
struct B09Frame {
    unsigned char temps[72];
    MusicChangeLocal local;
    // Construct the filename key with the existing shifted allocator-tag scratch base.
    B09Frame(const char* name) : local(name, temps - 4) {}
};
typedef B09Frame NamedPitchFrame;
// Compare the requested filename stem with a group slot name, ignoring case.
inline int NamedPitch_NameMatches(const char* name, unsigned char* entry) {
    if (!name)
        return 0;
    NamedPitchFrame frame(name);
    const char* part = FilenameInline_Parse(frame.local.data, frame.temps - 4);
    unsigned int length = strlen(part);
    if (!length)
        return 0;
    unsigned int small = !(AsSoundChannelGroup(entry)->channels[0].nameFlags >> kHeapStringFlagShift);
    unsigned int size =
        small ? (entry[0x14] & kInlineStringLengthMask) : AsSoundChannelGroup(entry)->channels[0].nameLength;
    if (length != size)
        return 0;
    const char* existing =
        small ? (const char*)entry + 0x15 : (*(const char**)&AsSoundChannelGroup(entry)->channels[0].nameData);
    return !String_ComparePrefixIgnoreCase((void*)part, existing, length);
}
// Test the selected voice slot name and non-idle playback state.
inline int NamedPitch_IsActive(unsigned char* items, const char* name) {
    if (!name)
        return 0;
    unsigned char* entry = items + *(int*)items * kStreamSlotBytes;
    if (!NamedPitch_NameMatches(name, entry))
        return 0;
    return *(int*)(items + *(int*)items * kStreamSlotBytes + 0x10) != 0;
}
// Apply a pitch multiplier to the selected voice slot.
inline void NamedPitch_Apply(register unsigned char* items, float value) {
    items += *(int*)items * kStreamSlotBytes;
    AsSoundChannelGroup(items)->channels[0].pitch = value;
    void* object = AsSoundChannelGroup(items)->channels[0].source;
    if (object)
        ((void (*)(void*, float))SetPitch__Q23gtl12ISoundStreamFf)(
            object, AsSoundChannelGroup(items)->channels[0].basePitch * AsSoundChannelGroup(items)->channels[0].pitch);
}
// Apply a pitch multiplier to buffers and the selected voice stream matching the requested name.
void SoundManager_SetNamedEffectPitch(unsigned char* self, const char* name, float pitchMultiplier) {
    register unsigned char* owner;
    if (!name)
        return;
    unsigned int length = strlen(name);
    if (!length)
        return;
    owner = (*(unsigned char**)&AsSoundManagerState(self)->state);
    unsigned char* node;
    void* mutex;
    {
        WInstanceLock lock(owner + kBufferListMutexOffset);
        *(void**)(owner + kBufferListCursorOffset) = *(void**)(owner + kBufferListHeadOffset);
        mutex = owner + kBufferListMutexOffset;
        node = (unsigned char*)VolumeIterator_Next(owner, mutex);
    }
    if (node) {
        mutex = owner + kBufferListMutexOffset;
        while (node) {
            if (!String_ComparePrefixIgnoreCase(node + 0x40, name, length)) {
                if (node && AsSoundBufferPlayback(node)->soundBuffer)
                    ((void (*)(void*, float))SetPitch__Q23gtl12ISoundBufferFf)(AsSoundBufferPlayback(node)->soundBuffer,
                                                                               AsSoundBufferPlayback(node)->basePitch *
                                                                                   pitchMultiplier);
            }
            node = (unsigned char*)VolumeIterator_Next(owner, mutex);
        }
    }
    unsigned char* items = (*(unsigned char**)&AsSoundManagerState(self)->state) + kVoiceGroupOffset;
    if (NamedPitch_IsActive(items, name)) {
        NamedPitch_Apply((*(unsigned char**)&AsSoundManagerState(self)->state) + kVoiceGroupOffset, pitchMultiplier);
    }
}

#pragma inline_depth(8)
#pragma inline_max_size(10000)
#pragma inline_max_total_size(10000)
// Advance the buffer cursor under a caller-supplied mutex.
inline void* NamedVolume_NextBuffer(void* mutex, unsigned char* data) {
    void* item;
    OSLockMutex(mutex);
    void* node = *(void**)(data + kBufferListCursorOffset);
    if (node) {
        void* next = ((SoundListLink*)node)->next;
        item = *(void**)node;
        if (next == *(void**)(data + kBufferListHeadOffset))
            next = 0;
        *(void**)(data + kBufferListCursorOffset) = next;
        OSUnlockMutex(mutex);
        return item;
    }
    OSUnlockMutex(mutex);
    return 0;
}
// Compare a filename stem against the selected group-origin slot, ignoring case.
inline bool NamedVolume_NameMatches(const char* name, unsigned char* item) {
    const char* key;
    unsigned int length;
    if (!name)
        return 0;
    MusicChangeFrame frame(name);
    key = FilenameInline_Parse(frame.local.data, frame.temps);
    length = strlen(key);
    if (length == 0)
        return 0;
    unsigned int small = !(AsSoundChannelGroup(item)->channels[0].nameFlags >> kHeapStringFlagShift);
    unsigned int size =
        small ? (item[0x14] & kInlineStringLengthMask) : AsSoundChannelGroup(item)->channels[0].nameLength;
    if (length != size)
        return 0;
    const char* existing =
        small ? (const char*)item + 0x15 : (*(const char**)&AsSoundChannelGroup(item)->channels[0].nameData);
    return !String_ComparePrefixIgnoreCase((void*)key, existing, length);
}
// Test whether the selected voice slot matches and has a non-idle state.
inline bool NamedVolume_IsActive(unsigned char* items, const char* name) {
    if (!name)
        return 0;
    unsigned char* item = items + *(int*)items * kStreamSlotBytes;
    if (NamedVolume_NameMatches(name, item))
        return *(int*)(items + *(int*)items * kStreamSlotBytes + 0x10) != 0;
    return 0;
}
// Apply a volume multiplier to matching buffers and the selected voice stream.
void SoundManager_SetNamedVolumeScale(unsigned char* self, const char* name, float volumeMultiplier) {
    void* mutex;
    unsigned char* state;
    if (name == 0)
        return;
    unsigned int nameLength = strlen(name);
    if (nameLength == 0)
        return;
    state = (*(unsigned char**)&AsSoundManagerState(self)->state);
    unsigned char* node;
    {
        WInstanceLock lock(state + kBufferListMutexOffset);
        *(void**)(state + kBufferListCursorOffset) = *(void**)(state + kBufferListHeadOffset);
        mutex = state + kBufferListMutexOffset;
        node = (unsigned char*)NamedVolume_NextBuffer(mutex, state);
    }
    if (node) {
        mutex = state + kBufferListMutexOffset;
        while (node) {
            if (!String_ComparePrefixIgnoreCase(node + 0x40, name, nameLength)) {
                float scale = AsSoundManagerState(self)->volumeScale;
                if (node && AsSoundBufferPlayback(node)->soundBuffer) {
                    AsSoundBufferPlayback(node)->volume = volumeMultiplier;
                    ((gtl::ISoundBuffer*)(AsSoundBufferPlayback(node)->soundBuffer))
                        ->SetVolume(scale * (AsSoundBufferPlayback(node)->baseVolume * volumeMultiplier));
                }
            }
            node = (unsigned char*)NamedVolume_NextBuffer(mutex, state);
        }
    }
    unsigned char* items = (*(unsigned char**)&AsSoundManagerState(self)->state) + kVoiceGroupOffset;
    if (NamedVolume_IsActive(items, name)) {
        items = (*(unsigned char**)&AsSoundManagerState(self)->state) + kVoiceGroupOffset;
        items += *(int*)items * kStreamSlotBytes;
        AsSoundChannelGroup(items)->channels[0].volume = volumeMultiplier * AsSoundManagerState(self)->volumeScale;
        register void* object = AsSoundChannelGroup(items)->channels[0].source;
        if (object) {
            float volume;
            float scale = AsSoundChannelGroup(items)->channels[0].baseVolume;
            volume = AsSoundChannelGroup(items)->channels[0].volume;
            float factor = AsSoundChannelGroup(items)->channels[0].fadeValue;
            volume = scale * volume;
            ((gtl::ISoundStream*)object)->SetVolume(volume * factor);
        }
    }
}

#pragma inline_depth reset
#pragma inline_max_size reset
#pragma inline_max_total_size reset

// Immediately stop and reset one voice slot, preserving its cached string allocations.
static inline void StopAlias_ResetVoice(unsigned char* item) {
    unsigned char temp[24];
    FloatTween_Start(item + 0x28, kFadeDefault, kFloat_0_soundPlayback, kFloat_0_soundPlayback);
    reset_rc((void**)(item + 4));
    if (AsSoundChannelGroup(item)->channels[0].sharedFile) {
        CSharedFileHandle::Destroy(AsSoundChannelGroup(item)->channels[0].sharedFile, 1);
        AsSoundChannelGroup(item)->channels[0].sharedFile = 0;
    }
    AsSoundChannelGroup(item)->channels[0].source = 0;
    AsSoundChannelGroup(item)->channels[0].volume = kFloat_1_SoundPlayback;
    AsSoundChannelGroup(item)->channels[0].pitch = kFloat_1_SoundPlayback;
    AsSoundChannelGroup(item)->channels[0].playState = 0;
    unsigned int cap;
    if (AsSoundChannelGroup(item)->channels[0].nameFlags >> kHeapStringFlagShift)
        cap = AsSoundChannelGroup(item)->channels[0].nameLength;
    else
        cap = ((char*)&AsSoundChannelGroup(item)->channels[0].nameFlags)[0] & kInlineStringLengthMask;
    temp[16] = temp[20];
    String_ReplaceRange(item + 0x14, 0, cap, s_soundPlaybackEmptyName,
                        s_soundPlaybackEmptyName + strlen(s_soundPlaybackEmptyName), temp + 16);
    (*(char*)&AsSoundChannelGroup(item)->channels[0].loopEnabled) = 0;
    (*(char*)&AsSoundChannelGroup(item)->channels[0].fadeInEnabled) = 0;
    (*(char*)&AsSoundChannelGroup(item)->channels[0].useDolbyBank) = 0;
    AsSoundChannelGroup(item)->channels[0].transitionDuration = kFloat_0_soundPlayback;
    if (AsSoundChannelGroup(item)->channels[0].filePathFlags >> kHeapStringFlagShift)
        cap = AsSoundChannelGroup(item)->channels[0].filePathLength;
    else
        cap = ((char*)&AsSoundChannelGroup(item)->channels[0].filePathFlags)[0] & kInlineStringLengthMask;
    temp[8] = temp[12];
    String_ReplaceRange(item + 0x44, 0, cap, s_soundPlaybackEmptyFilePath,
                        s_soundPlaybackEmptyFilePath + strlen(s_soundPlaybackEmptyFilePath), temp + 8);
    if (AsSoundChannelGroup(item)->channels[0].resourcePathFlags >> kHeapStringFlagShift)
        cap = AsSoundChannelGroup(item)->channels[0].resourcePathLength;
    else
        cap = ((char*)&AsSoundChannelGroup(item)->channels[0].resourcePathFlags)[0] & kInlineStringLengthMask;
    temp[0] = temp[4];
    String_ReplaceRange(item + 0x50, 0, cap, s_soundPlaybackEmptyResourcePath,
                        s_soundPlaybackEmptyResourcePath + strlen(s_soundPlaybackEmptyResourcePath), temp);
    FloatTween_Start(item + 0x28, kFadeDefault, kFloat_0_soundPlayback, kFloat_0_soundPlayback);
    AsSoundChannelGroup(item)->channels[0].baseVolume = kFloat_1_SoundPlayback;
    AsSoundChannelGroup(item)->channels[0].basePitch = kFloat_1_SoundPlayback;
    String_ReserveCapacity(item + 0x14, 0x40);
    String_ReserveCapacity(item + 0x44, 0x10);
    String_ReserveCapacity(item + 0x50, 0x10);
}

#pragma inline_depth(8)
#pragma inline_max_size(10000)
#pragma inline_max_total_size(20000)
// Stop all buffers and the selected voice for a null name, or stop only matching aliases.
void Sound_StopByAlias(void* selfArg, const char* name) {
    unsigned char* self = (unsigned char*)selfArg;
    unsigned char temps[72];
    if (name == 0) {
        unsigned char* state = (*(unsigned char**)&AsSoundManagerState(self)->state);
        void* item = BufferIterator_First(state);
        while (item != 0) {
            if (AsSoundBufferPlayback(item)->soundBuffer)
                ((gtl::ISoundBuffer*)(AsSoundBufferPlayback(item)->soundBuffer))->Stop(0);
            AsSoundBufferPlayback(item)->active = 0;
            item = BufferIterator_Next(state);
        }
        unsigned char* entries = (*(unsigned char**)&AsSoundManagerState(self)->state) + kVoiceGroupOffset;
        StopAlias_ResetVoice(entries + kStreamSlotBytes * *(int*)entries);
        return;
    }
    unsigned int length = strlen(name);
    if (length == 0)
        return;
    unsigned char* data = (*(unsigned char**)&AsSoundManagerState(self)->state);
    void* item = BufferIterator_First(data);
    while (item != 0) {
        if (String_ComparePrefixIgnoreCase(((SoundInstanceTextView*)item)->alias, (void*)name, length) == 0 &&
            AsSoundBufferPlayback(item)->active != 0) {
            if (AsSoundBufferPlayback(item)->soundBuffer)
                ((gtl::ISoundBuffer*)(AsSoundBufferPlayback(item)->soundBuffer))->Stop(0);
            AsSoundBufferPlayback(item)->active = 0;
        }
        item = BufferIterator_Next(data);
    }
    unsigned char* entries = (*(unsigned char**)&AsSoundManagerState(self)->state) + kVoiceGroupOffset;
    if (MusicChange_IsActive(entries, name, temps)) {
        StopAlias_ResetVoice(entries + kStreamSlotBytes * *(int*)entries);
    }
}
#pragma inline_depth reset
#pragma inline_max_size reset
#pragma inline_max_total_size reset

} // extern "C"
