// Sound channel group reset: clears the stream slots of one sound channel group, with their
// playback records, names and file paths.
// The sound manager calls it for the music and voice groups. Reserved string capacity is kept.

#include "classes/SoundPlaybackState.h"
#include "classes/GtlSoundStreamInterface.h"
#include "classes/CSharedFileHandle.h"

extern "C" {
void String_ReplaceRange(void*, int, unsigned int, const char*, const char*, unsigned char*);
void FloatTween_Start(void*, int, float, float);
void String_ReserveCapacity(void*, int);
int strlen(const char*);

extern const float kFloat_0_soundPlayback;
extern const float kFloat_1_SoundPlayback;
extern char s_soundPlaybackEmptyResourcePath[4];
extern char s_soundPlaybackEmptyFilePath[4];
extern char s_soundPlaybackEmptyName[4];

// Operates on ONE SoundChannelGroup: selected index at +0, then two 0x64-byte
// records at +4/+0x68. sound_manager.cpp invokes it separately for music and voice.
// recordBase is the true playback-record origin. groupSlotCursor remains four
// bytes before it; e.g. cursor +0x64 is this record's basePitch, not the next
// record's source pointer. Do not reinterpret the two cursors as separate arrays.
// All three empty-string symbols below supply the same empty reset text;
// retain their distinct addresses because relocation identity is part of the ABI.
enum StreamResetConstants {
    kStreamSlotsPerGroup = 2,
    kPlaybackRecordBytes = 0x64,
    kGroupPrefixBytes = 4,
    kInlineStringLengthMask = 0x7F,
    kHeapStringFlagShift = 31,
    kNameReserveBytes = 0x40,
    kPathReserveBytes = 0x10,
    kResetTweenMode = -1
};

// Offsets from the cursor four bytes before a SoundPlaybackState. Keeping that
// origin preserves the matched addressing; these names describe the shared layout.
enum StreamSlotCursorOffset {
    kCursorStream = 0x04,
    kCursorVolume = 0x08,
    kCursorPitch = 0x0C,
    kCursorPlayState = 0x10,
    kCursorNameHeapLength = 0x18,
    kCursorLoopEnabled = 0x20,
    kCursorFadeInEnabled = 0x21,
    kCursorUseDolbyBank = 0x22,
    kCursorTransitionDuration = 0x24,
    kCursorFilePathHeapLength = 0x48,
    kCursorResourcePathHeapLength = 0x54,
    kCursorSharedFile = 0x5C,
    kCursorBaseVolume = 0x60,
    kCursorBasePitch = 0x64
};
// Offsets from the actual record origin; these refer to complete subobjects.
enum StreamRecordOffset {
    kRecordName = 0x10,
    kRecordFadeTween = 0x24,
    kRecordFilePath = 0x40,
    kRecordResourcePath = 0x4C
};

// Decode a tagged MW string length; ownership/capacity are unchanged.
#pragma push
#pragma always_inline on
inline unsigned int PlaybackStringLength(const unsigned char* text, const unsigned int* heapLength) {
    if ((*(const unsigned int*)text >> kHeapStringFlagShift) == 0)
        return text[0] & kInlineStringLengthMask;
    return *heapLength;
}
#pragma pop

// Stop/release a group's streams and files, then restore both slots' defaults.
// self is a live, constructed group owned by the caller; the group and its string
// objects survive. Streams lose one retained reference (destroyed at zero); shared
// file wrappers are destroyed outright. String capacity is reserved, not released.
void SoundChannelGroup_ResetStreamSlots(unsigned char* self) {
    // Historical shared symbol: this 1.0 value is the default audio volume/pitch ratio.
    float defaultRatio = kFloat_1_SoundPlayback;
    float zeroValue;

    struct {
        unsigned char argument;
        char padding[3];
        unsigned char source;
    } nameAllocator, filePathAllocator, resourcePathAllocator, secondNameAllocator, secondFilePathAllocator,
        secondResourcePathAllocator;

    unsigned char* groupSlotCursor;
    unsigned char* recordBase;

    // Phase 1: release resources and reset both records. The second pass below
    // revisits these SAME records; it does not initialize another channel group.
    {
        unsigned char nameAllocatorByte = nameAllocator.source;
        groupSlotCursor = self;
        zeroValue = kFloat_0_soundPlayback;
        recordBase = self + kGroupPrefixBytes;
        unsigned char filePathAllocatorByte = filePathAllocator.source;
        unsigned i = 0;
        unsigned char resourcePathAllocatorByte = resourcePathAllocator.source;
        for (; i < kStreamSlotsPerGroup; i++) {
            void* stream = AsSoundPlaybackState(groupSlotCursor + kGroupPrefixBytes)->source;
            if (stream != 0) {
                ((gtl::ISoundStream*)stream)->Stop(0);
                stream = AsSoundPlaybackState(groupSlotCursor + kGroupPrefixBytes)->source;
                int refs = --*(int*)stream;
                if (refs == 0 && stream != 0) {
                    void** streamVtable = *(void***)((char*)stream + 4);
                    // Reference-count prefix is followed by the vtable at +4;
                    // slot 2 destroys the stream with a deleting flag of 1.
                    ((void (*)(void*, int))streamVtable[2])(stream, 1);
                }
            }
            void* sharedFile = AsSoundPlaybackState(groupSlotCursor + kGroupPrefixBytes)->sharedFile;
            if (sharedFile != 0) {
                CSharedFileHandle::Destroy(sharedFile, 1);
                AsSoundPlaybackState(groupSlotCursor + kGroupPrefixBytes)->sharedFile = 0;
            }
            AsSoundPlaybackState(groupSlotCursor + kGroupPrefixBytes)->source = 0;
            AsSoundPlaybackState(groupSlotCursor + kGroupPrefixBytes)->volume = defaultRatio;
            AsSoundPlaybackState(groupSlotCursor + kGroupPrefixBytes)->pitch = defaultRatio;
            AsSoundPlaybackState(groupSlotCursor + kGroupPrefixBytes)->playState = 0;

            // Reset name and paths through the tagged-string ABI. High bit clear:
            // first byte stores the 7-bit inline length; otherwise use heap length.
            // The allocator source bytes remain unspecified tokens,
            // not characters read from a filename or terminators to initialize.
            unsigned int stringLength;
            stringLength = PlaybackStringLength(recordBase + kRecordName,
                                                (unsigned int*)(groupSlotCursor + kCursorNameHeapLength));
            nameAllocator.argument = nameAllocatorByte;
            int replacementLength = strlen(s_soundPlaybackEmptyName);
            String_ReplaceRange(recordBase + kRecordName, 0, stringLength, s_soundPlaybackEmptyName,
                                s_soundPlaybackEmptyName + replacementLength, &nameAllocator.argument);

            AsSoundPlaybackState(groupSlotCursor + kGroupPrefixBytes)->loopEnabled = 0;
            AsSoundPlaybackState(groupSlotCursor + kGroupPrefixBytes)->fadeInEnabled = 0;
            AsSoundPlaybackState(groupSlotCursor + kGroupPrefixBytes)->useDolbyBank = 0;
            AsSoundPlaybackState(groupSlotCursor + kGroupPrefixBytes)->transitionDuration = zeroValue;

            stringLength = PlaybackStringLength(recordBase + kRecordFilePath,
                                                (unsigned int*)(groupSlotCursor + kCursorFilePathHeapLength));
            filePathAllocator.argument = filePathAllocatorByte;
            replacementLength = strlen(s_soundPlaybackEmptyFilePath);
            String_ReplaceRange(recordBase + kRecordFilePath, 0, stringLength, s_soundPlaybackEmptyFilePath,
                                s_soundPlaybackEmptyFilePath + replacementLength, &filePathAllocator.argument);

            stringLength = PlaybackStringLength(recordBase + kRecordResourcePath,
                                                (unsigned int*)(groupSlotCursor + kCursorResourcePathHeapLength));
            resourcePathAllocator.argument = resourcePathAllocatorByte;
            replacementLength = strlen(s_soundPlaybackEmptyResourcePath);
            String_ReplaceRange(recordBase + kRecordResourcePath, 0, stringLength, s_soundPlaybackEmptyResourcePath,
                                s_soundPlaybackEmptyResourcePath + replacementLength, &resourcePathAllocator.argument);

            FloatTween_Start(recordBase + kRecordFadeTween, kResetTweenMode, kFloat_0_soundPlayback,
                             kFloat_0_soundPlayback);

            AsSoundPlaybackState(groupSlotCursor + kGroupPrefixBytes)->baseVolume = defaultRatio;
            AsSoundPlaybackState(groupSlotCursor + kGroupPrefixBytes)->basePitch = defaultRatio;

            String_ReserveCapacity(recordBase + kRecordName, kNameReserveBytes);
            String_ReserveCapacity(recordBase + kRecordFilePath, kPathReserveBytes);
            String_ReserveCapacity(recordBase + kRecordResourcePath, kPathReserveBytes);

            groupSlotCursor += kPlaybackRecordBytes;
            recordBase += kPlaybackRecordBytes;
        }
    }

    // Phase 2: select slot zero and repeat the scalar/string/tween defaults.
    // Stream/file cleanup belongs only to phase 1; neither loop can be elided in
    // a matching source edit. Keep statement/declaration order for CodeWarrior.
    {
        *(int*)self = 0;
        float secondPassRatio = kFloat_1_SoundPlayback;
        groupSlotCursor = self;
        recordBase = self + kGroupPrefixBytes;
        unsigned char secondNameAllocatorByte = secondNameAllocator.source;
        unsigned i = 0;
        float secondPassZero = kFloat_0_soundPlayback;
        unsigned char secondFilePathAllocatorByte = secondFilePathAllocator.source;
        unsigned char secondResourcePathAllocatorByte = secondResourcePathAllocator.source;
        for (; i < kStreamSlotsPerGroup; i++) {
            AsSoundPlaybackState(groupSlotCursor + kGroupPrefixBytes)->source = 0;
            AsSoundPlaybackState(groupSlotCursor + kGroupPrefixBytes)->volume = secondPassRatio;
            AsSoundPlaybackState(groupSlotCursor + kGroupPrefixBytes)->pitch = secondPassRatio;
            AsSoundPlaybackState(groupSlotCursor + kGroupPrefixBytes)->playState = 0;

            unsigned int stringLength;
            stringLength = PlaybackStringLength(recordBase + kRecordName,
                                                (unsigned int*)(groupSlotCursor + kCursorNameHeapLength));
            secondNameAllocator.argument = secondNameAllocatorByte;
            int replacementLength = strlen(s_soundPlaybackEmptyName);
            String_ReplaceRange(recordBase + kRecordName, 0, stringLength, s_soundPlaybackEmptyName,
                                s_soundPlaybackEmptyName + replacementLength, &secondNameAllocator.argument);

            AsSoundPlaybackState(groupSlotCursor + kGroupPrefixBytes)->loopEnabled = 0;
            AsSoundPlaybackState(groupSlotCursor + kGroupPrefixBytes)->fadeInEnabled = 0;
            AsSoundPlaybackState(groupSlotCursor + kGroupPrefixBytes)->useDolbyBank = 0;
            AsSoundPlaybackState(groupSlotCursor + kGroupPrefixBytes)->transitionDuration = secondPassZero;

            stringLength = PlaybackStringLength(recordBase + kRecordFilePath,
                                                (unsigned int*)(groupSlotCursor + kCursorFilePathHeapLength));
            secondFilePathAllocator.argument = secondFilePathAllocatorByte;
            replacementLength = strlen(s_soundPlaybackEmptyFilePath);
            String_ReplaceRange(recordBase + kRecordFilePath, 0, stringLength, s_soundPlaybackEmptyFilePath,
                                s_soundPlaybackEmptyFilePath + replacementLength, &secondFilePathAllocator.argument);

            stringLength = PlaybackStringLength(recordBase + kRecordResourcePath,
                                                (unsigned int*)(groupSlotCursor + kCursorResourcePathHeapLength));
            secondResourcePathAllocator.argument = secondResourcePathAllocatorByte;
            replacementLength = strlen(s_soundPlaybackEmptyResourcePath);
            String_ReplaceRange(recordBase + kRecordResourcePath, 0, stringLength, s_soundPlaybackEmptyResourcePath,
                                s_soundPlaybackEmptyResourcePath + replacementLength,
                                &secondResourcePathAllocator.argument);

            FloatTween_Start(recordBase + kRecordFadeTween, kResetTweenMode, kFloat_0_soundPlayback,
                             kFloat_0_soundPlayback);

            AsSoundPlaybackState(groupSlotCursor + kGroupPrefixBytes)->baseVolume = secondPassRatio;
            AsSoundPlaybackState(groupSlotCursor + kGroupPrefixBytes)->basePitch = secondPassRatio;

            String_ReserveCapacity(recordBase + kRecordName, kNameReserveBytes);
            String_ReserveCapacity(recordBase + kRecordFilePath, kPathReserveBytes);
            String_ReserveCapacity(recordBase + kRecordResourcePath, kPathReserveBytes);

            groupSlotCursor += kPlaybackRecordBytes;
            recordBase += kPlaybackRecordBytes;
        }
    }
}
} // extern "C"
