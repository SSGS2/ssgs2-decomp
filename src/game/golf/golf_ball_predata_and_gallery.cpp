#include <decomp/static_init.h>
#include "classes/CGolfBg.h"
// Translation unit 0x801203D4-0x80121B14 (13 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "classes/CSharedDoc.h"
#include "classes/CGolfDoc.h"
#include "sdk_protos.h"
#include "game/wvector.h"
#include "game/golfball.h"
#include "iff/IffStruct.h"

extern "C" void* __nwa__FUl(unsigned int);
// Route array allocations through the existing game allocator ABI.
inline void* operator new[](size_t size) { return __nwa__FUl(size); }

extern "C" {

extern unsigned char s_galleryTypeInfo;
extern unsigned char s_actorTypeInfo;
extern const float kFloat_6_2831855;
extern float s_golfBallTwoPi;
extern float s_galleryFullTurnRadians;

extern const char gallerySpeechStrings[];
extern const char s_galleryActorFactoryName[];
extern const char s_galleryTaskFactoryName[];
extern const char __vt__7CGolfBg[];

void* CGolfBall_GetOrAllocatePreDataAt(void*, int);
void* __nwa__FUl(unsigned int);
void* __nw__FUl(unsigned int);
void appFreeUni__FPv(void*);
void IntPair_Set(void*, const char*, void*);
void* ActorFactoryRegistry_GetInstance();
void StringMap_SetByName(void*, void*, const char*);
void* TaskRegistry_GetInstance();
void TaskRegistry_AppendFactory(void*, const char*);
void __ct__18NativeBase80201D18Fv(void*);
void appFreeUni__FPv(void*);
void* CGallery_Create();
void __dt__18NativeBase80201D18Fv(void*, int);
void __dl__FPv(void*);
void __dla__FPv(void*);
void* memset(void*, int, unsigned int);
void* memcpy(void*, const void*, unsigned int);
void __as__7WVectorFRC7WVector(void*, void*);
void Vec3_Zero(void*);
void Vec2_Zero(void*);
extern const float kGolfBallDefaultRadius;
extern const float kGolfBallMass;
extern const float kFloat_0_GolfBallState;
extern float kFloat_0_Gallery;
extern const float kFloat_0_5_GallerySpeechSound;
extern const float kFloat_200_GallerySpeechSound;
extern const float kFloat_1_Gallery;
extern float kGallerySpeechLongCooldown;
extern void* g_sharedDoc;
extern void* g_soundManager;
int strcmp(const char*, const char*);
void Sound_PlayByName(void*, const char*, int, int, const char*, int, float, float, float);
void Sound_StopByAlias(void*, const char*);
char* strcpy(char*, const char*);
unsigned int ItemInventory_GetBallIdAt(void*, int);
const char* String_FormatTemporary(const char*, ...);
int sprintf(char*, const char*, ...);

struct GalleryState {
    char actorStorage[0x18];
    float speechCooldown;
    char soundAlias[1]; // Variable-sized name storage; its full extent is not established here.
};
struct GallerySpeechPool {
    char applause[16];
    char cheer[14];
    char ex6Format[9];
    char ex4Format[9];
    char disappointment[16];
    char groupFive[14];
    char ex3Format[9];
    char actorType[9];
};
enum BallRecordStorage {
    kInlineBallRecords = 900,
    kBallRecordBlockSize = 60,
    kBallRecordBytes = sizeof(sExpected),
    kBallRecordBlockBytes = kBallRecordBlockSize * kBallRecordBytes,
    kArrayCookieBytes = 16
};
struct HoleInCrypt {
    unsigned int encodedValue, addressCheck, randomKey;
    unsigned short salt;
    unsigned char suffix, padding;
};
enum HoleInEncodingOffset {
    kBallHighAddressBase = 0x20000,
    kEncodedValueFromHighBase = IFF_OFFSET(CGolfBall, m_holeInCrypt) + IFF_OFFSET(HoleInCrypt, encodedValue) - kBallHighAddressBase,
    kAddressCheckFromHighBase = IFF_OFFSET(CGolfBall, m_holeInCrypt) + IFF_OFFSET(HoleInCrypt, addressCheck) - kBallHighAddressBase,
    kRandomKeyFromHighBase = IFF_OFFSET(CGolfBall, m_holeInCrypt) + IFF_OFFSET(HoleInCrypt, randomKey) - kBallHighAddressBase
};
// These IDs are established only as response categories; the originating event
// meanings have not been proved. Keep distinct IDs even when speech is identical.
enum GalleryResponseId {
    kGalleryIgnored0 = 0, kGalleryIgnored1 = 1, kGalleryIgnored2 = 2,
    kGalleryApplause3 = 3, kGalleryApplause4 = 4,
    kGalleryCheer5 = 5, kGalleryCheerLongCooldown6 = 6,
    kGalleryCheer7 = 7, kGalleryCheer8 = 8,
    kGalleryBallVariant9 = 9, kGalleryBallVariant10 = 10,
    kGalleryCheer11 = 11, kGalleryCheer12 = 12,
    kGalleryDisappointment13 = 13, kGalleryCheer14 = 14,
    kGalleryGroupFive15 = 15, kGalleryIgnored16 = 16,
    kGalleryBallVariant17 = 17
};
enum GallerySpeechConstants {
    kGalleryFirstAllowedTutorialStage = 5,
    kGalleryNormalPlayStage = 6,
    kGalleryVariantBallInventorySlot = 11,
    kRandomKeyByteRange254 = 254, kRandomKeyByteRange255 = 255,
    kHoleInKeyFixedBits = 0x1234F000, kHoleInSaltFixedBits = 0xF000
};
// Overlay only the ball-ID slot used here, retaining the observed 0x1D4 stride.
struct GalleryPlayerBallSlot {
    char beforeBallId[0x88];
    unsigned int ballId;
    char afterBallId[0x148];
};
typedef char GalleryPlayerBallStride[(sizeof(GalleryPlayerBallSlot) == 0x1D4) ? 1 : -1];

struct GolfBackgroundBaseView { void* vtable; unsigned int prefix; int enabled; };

// Read the current tutorial stage without mutating the gallery or consuming randomness.
static inline int GallerySpeechStage() {
    return ((CGolfDoc*)((CSharedDoc*)g_sharedDoc)->golfDoc)->tutorialStage;
}

// Play the selected alias; a positive cooldown suppresses repeated aliases only.
// longCooldown selects the existing 3.0 scalar instead of the existing zero scalar.
// Keep alias selection in the caller and read cooldown only at the original stores.
// This helper's return ends only playback; each event case then immediately ends.
static inline void PlayGallerySpeech(char* obj, const char*& speechName, bool longCooldown)
{
    if (strcmp((obj + IFF_OFFSET(GalleryState, soundAlias)), speechName) == 0) {
        if (*(float*)(obj + IFF_OFFSET(GalleryState, speechCooldown)) > kFloat_0_Gallery) return;
        *(float*)(obj + IFF_OFFSET(GalleryState, speechCooldown)) = (longCooldown ? kGallerySpeechLongCooldown : kFloat_0_Gallery);
        Sound_PlayByName(g_soundManager, (obj + IFF_OFFSET(GalleryState, soundAlias)), 0, 0, (obj + IFF_OFFSET(GalleryState, soundAlias)), 0, kFloat_0_5_GallerySpeechSound, kFloat_200_GallerySpeechSound, kFloat_1_Gallery);
    } else {
        if (*((obj + IFF_OFFSET(GalleryState, soundAlias)))) Sound_StopByAlias(g_soundManager, (obj + IFF_OFFSET(GalleryState, soundAlias)));
        *(float*)(obj + IFF_OFFSET(GalleryState, speechCooldown)) = (longCooldown ? kGallerySpeechLongCooldown : kFloat_0_Gallery);
        strcpy((obj + IFF_OFFSET(GalleryState, soundAlias)), speechName);
        Sound_PlayByName(g_soundManager, (obj + IFF_OFFSET(GalleryState, soundAlias)), 0, 0, (obj + IFF_OFFSET(GalleryState, soundAlias)), 0, kFloat_0_5_GallerySpeechSound, kFloat_200_GallerySpeechSound, kFloat_1_Gallery);
    }
}

// Event[1] response map (event meanings are unconfirmed by this unit):
// 0/1/2/16 ignored; 3/4 applause; 5/7/8/11/12/14 cheer; 6 cheer with cooldown 3;
// 9/10 special-ball ex6/ex4 variants 1..3 else cheer; 13 disappointment;
// 15 groupFive; 17 special-ball ex3 variants 1..4 else groupFive. Other IDs ignored.
// self owns alias storage; event and formatted temporary text are borrowed. The
// alias must fit the actor's trailing allocation; no bounded copy is added here.
// Choose crowd speech for a golf event, suppressing speech before active play.
void CGallery_HandleMsg(void* self, const unsigned int* event) {
    unsigned int type = event[1];
    char* obj = (char*)self;
    const char* speechName;
    CGolfDoc* currentGolfDoc;
    switch (type) {
    case kGalleryIgnored0: break;
    case kGalleryIgnored1: break;
    case kGalleryIgnored2: break;
    case kGalleryIgnored16: break;
    case kGalleryApplause3: {
        speechName = gallerySpeechStrings;
        int stage = GallerySpeechStage();
        if (!(stage >= kGalleryNormalPlayStage || stage == kGalleryFirstAllowedTutorialStage)) return;
        PlayGallerySpeech(obj, speechName, false);
    } break;
    case kGalleryApplause4: {
        speechName = gallerySpeechStrings;
        int stage = GallerySpeechStage();
        if (!(stage >= kGalleryNormalPlayStage || stage == kGalleryFirstAllowedTutorialStage)) return;
        PlayGallerySpeech(obj, speechName, false);
    } break;
    case kGalleryCheer5: {
        speechName = (gallerySpeechStrings + IFF_OFFSET(GallerySpeechPool, cheer));
        int stage = GallerySpeechStage();
        if (!(stage >= kGalleryNormalPlayStage || stage == kGalleryFirstAllowedTutorialStage)) return;
        PlayGallerySpeech(obj, speechName, false);
    } break;
    case kGalleryCheerLongCooldown6: {
        speechName = (gallerySpeechStrings + IFF_OFFSET(GallerySpeechPool, cheer));
        int stage = GallerySpeechStage();
        if (!(stage >= kGalleryNormalPlayStage || stage == kGalleryFirstAllowedTutorialStage)) return;
        PlayGallerySpeech(obj, speechName, true);
    } break;
    case kGalleryCheer7: {
        speechName = (gallerySpeechStrings + IFF_OFFSET(GallerySpeechPool, cheer));
        int stage = GallerySpeechStage();
        if (!(stage >= kGalleryNormalPlayStage || stage == kGalleryFirstAllowedTutorialStage)) return;
        PlayGallerySpeech(obj, speechName, false);
    } break;
    case kGalleryCheer8: {
        speechName = (gallerySpeechStrings + IFF_OFFSET(GallerySpeechPool, cheer));
        int stage = GallerySpeechStage();
        if (!(stage >= kGalleryNormalPlayStage || stage == kGalleryFirstAllowedTutorialStage)) return;
        PlayGallerySpeech(obj, speechName, false);
    } break;
    case kGalleryBallVariant9: {
        // Select a variant before checking stage; only the special ball draws rand().
        char* manager = (char*)g_sharedDoc;
        currentGolfDoc = (CGolfDoc*)((CSharedDoc*)manager)->golfDoc;
        unsigned int specialBallId = ItemInventory_GetBallIdAt(((CSharedDoc*)manager)->itemInventory, kGalleryVariantBallInventorySlot);
        if (specialBallId == ((GalleryPlayerBallSlot*)manager)[currentGolfDoc->currentPlayer].ballId) {
            speechName = String_FormatTemporary((gallerySpeechStrings + IFF_OFFSET(GallerySpeechPool, ex6Format)), rand() % 3 + 1);
            int stage = GallerySpeechStage();
            if (!(stage >= kGalleryNormalPlayStage || stage == kGalleryFirstAllowedTutorialStage)) return;
            PlayGallerySpeech(obj, speechName, false);
        } else {
            speechName = (gallerySpeechStrings + IFF_OFFSET(GallerySpeechPool, cheer));
            int stage = GallerySpeechStage();
            if (!(stage >= kGalleryNormalPlayStage || stage == kGalleryFirstAllowedTutorialStage)) return;
            PlayGallerySpeech(obj, speechName, false);
        }
    } break;
    case kGalleryBallVariant10: {
        // Select a variant before checking stage; only the special ball draws rand().
        char* manager = (char*)g_sharedDoc;
        currentGolfDoc = (CGolfDoc*)((CSharedDoc*)manager)->golfDoc;
        unsigned int specialBallId = ItemInventory_GetBallIdAt(((CSharedDoc*)manager)->itemInventory, kGalleryVariantBallInventorySlot);
        if (specialBallId == ((GalleryPlayerBallSlot*)manager)[currentGolfDoc->currentPlayer].ballId) {
            speechName = String_FormatTemporary((gallerySpeechStrings + IFF_OFFSET(GallerySpeechPool, ex4Format)), rand() % 3 + 1);
            int stage = GallerySpeechStage();
            if (!(stage >= kGalleryNormalPlayStage || stage == kGalleryFirstAllowedTutorialStage)) return;
            PlayGallerySpeech(obj, speechName, false);
        } else {
            speechName = (gallerySpeechStrings + IFF_OFFSET(GallerySpeechPool, cheer));
            int stage = GallerySpeechStage();
            if (!(stage >= kGalleryNormalPlayStage || stage == kGalleryFirstAllowedTutorialStage)) return;
            PlayGallerySpeech(obj, speechName, false);
        }
    } break;
    case kGalleryCheer11: {
        speechName = (gallerySpeechStrings + IFF_OFFSET(GallerySpeechPool, cheer));
        int stage = GallerySpeechStage();
        if (!(stage >= kGalleryNormalPlayStage || stage == kGalleryFirstAllowedTutorialStage)) return;
        PlayGallerySpeech(obj, speechName, false);
    } break;
    case kGalleryCheer12: {
        speechName = (gallerySpeechStrings + IFF_OFFSET(GallerySpeechPool, cheer));
        int stage = GallerySpeechStage();
        if (!(stage >= kGalleryNormalPlayStage || stage == kGalleryFirstAllowedTutorialStage)) return;
        PlayGallerySpeech(obj, speechName, false);
    } break;
    case kGalleryDisappointment13: {
        speechName = (gallerySpeechStrings + IFF_OFFSET(GallerySpeechPool, disappointment));
        int stage = GallerySpeechStage();
        if (!(stage >= kGalleryNormalPlayStage || stage == kGalleryFirstAllowedTutorialStage)) return;
        PlayGallerySpeech(obj, speechName, false);
    } break;
    case kGalleryCheer14: {
        speechName = (gallerySpeechStrings + IFF_OFFSET(GallerySpeechPool, cheer));
        int stage = GallerySpeechStage();
        if (!(stage >= kGalleryNormalPlayStage || stage == kGalleryFirstAllowedTutorialStage)) return;
        PlayGallerySpeech(obj, speechName, false);
    } break;
    case kGalleryGroupFive15: {
        speechName = (gallerySpeechStrings + IFF_OFFSET(GallerySpeechPool, groupFive));
        int stage = GallerySpeechStage();
        if (!(stage >= kGalleryNormalPlayStage || stage == kGalleryFirstAllowedTutorialStage)) return;
        PlayGallerySpeech(obj, speechName, false);
    } break;
    case kGalleryBallVariant17: {
        // Select a variant before checking stage; only the special ball draws rand().
        char* manager = (char*)g_sharedDoc;
        currentGolfDoc = (CGolfDoc*)((CSharedDoc*)manager)->golfDoc;
        unsigned int specialBallId = ItemInventory_GetBallIdAt(((CSharedDoc*)manager)->itemInventory, kGalleryVariantBallInventorySlot);
        if (specialBallId == ((GalleryPlayerBallSlot*)manager)[currentGolfDoc->currentPlayer].ballId) {
            speechName = String_FormatTemporary((gallerySpeechStrings + IFF_OFFSET(GallerySpeechPool, ex3Format)), rand() % 4 + 1);
            int stage = GallerySpeechStage();
            if (!(stage >= kGalleryNormalPlayStage || stage == kGalleryFirstAllowedTutorialStage)) return;
            PlayGallerySpeech(obj, speechName, false);
        } else {
            speechName = (gallerySpeechStrings + IFF_OFFSET(GallerySpeechPool, groupFive));
            int stage = GallerySpeechStage();
            if (!(stage >= kGalleryNormalPlayStage || stage == kGalleryFirstAllowedTutorialStage)) return;
            PlayGallerySpeech(obj, speechName, false);
        }
    } break;
    default: break;
    }
}

// The simulation-record constructor performs no initialization.
void GolfBallPreData_DefaultConstructNoop() {}

struct Record8012165C {
	char data[0x74];
	// Preserve the simulation record's intentionally empty initialization.
	Record8012165C() { GolfBallPreData_DefaultConstructNoop(); }
};

} // extern "C"

// Initialize overflow ownership, randomized hole-in encoding keys, and physical ball parameters.
inline CGolfBall::CGolfBall() {
	char* obj = (char*)this;
	{
		{
			(*(void**)&((CGolfBall*)obj)->m_pPreDataOverflow) = 0;

			// Keep each rand() expression in its original order: key bytes consume global RNG state.
			unsigned int randomKeyUpperBytes = ((rand() % kRandomKeyByteRange254) << 8) | (((rand() % kRandomKeyByteRange255) << 24) | ((rand() % kRandomKeyByteRange254) << 16));
			unsigned int randomKeyLowByte = rand() % kRandomKeyByteRange255;
			// These aliases preserve the native merge-expression schedule after rand().
			__typeof__(randomKeyLowByte) keyLowByteCopy = randomKeyLowByte;
			__typeof__(keyLowByteCopy | randomKeyUpperBytes) keyMergeTypeProbe = randomKeyUpperBytes | keyLowByteCopy;
			__typeof__(keyMergeTypeProbe) mergedRandomKey = keyLowByteCopy | randomKeyUpperBytes;
			randomKeyUpperBytes = mergedRandomKey;
			*(unsigned int*)(obj + IFF_OFFSET(CGolfBall, m_holeInCrypt) + IFF_OFFSET(HoleInCrypt, randomKey)) = kHoleInKeyFixedBits | randomKeyUpperBytes;
			unsigned int saltLowByte = rand() % kRandomKeyByteRange255;
			unsigned int saltHighByte = (rand() % kRandomKeyByteRange254) << 8;
			__typeof__(saltLowByte | saltHighByte) saltMergeTypeProbe = saltLowByte | saltHighByte;
			__typeof__(saltMergeTypeProbe) mergedSalt = saltHighByte | saltLowByte;
			*(unsigned short*)(obj + IFF_OFFSET(CGolfBall, m_holeInCrypt) + IFF_OFFSET(HoleInCrypt, salt)) = (unsigned short)(kHoleInSaltFixedBits | (mergedSalt));
			*(unsigned char*)(obj + IFF_OFFSET(CGolfBall, m_holeInCrypt) + IFF_OFFSET(HoleInCrypt, suffix)) = (unsigned char)(kHoleInKeyFixedBits | (rand() % kRandomKeyByteRange255));
			
			float defaultRadius = kGolfBallDefaultRadius;
			((CGolfBall*)obj)->m_radius = defaultRadius;
			((CGolfBall*)obj)->m_mass = (kGolfBallMass);
		}
	}
}

// Route ball singleton allocation through the existing game allocator ABI.
inline void* operator new(size_t size, int) { return __nw__FUl(size); }

// Lazily allocate and return the single ball simulation object.
CGolfBall* GolfBall() {
	static CGolfBall* ball;
	if (ball == 0) {
		ball = new (0) CGolfBall;
	}
	return ball;
}

extern "C" {

#pragma opt_propagation off
#pragma opt_strength_reduction off
#pragma opt_common_subs off
#pragma opt_lifetimes off
// Clear shot history, reset motion and contact state, and encode a zero hole-in value.
void CGolfBall_Reset(void* self, void* arg) {
	char* obj = (char*)self;
	// Phase 1: discard history/overflow and install the new starting position.
	memset(((CGolfBall*)obj)->m_preData, 0, sizeof(((CGolfBall*)obj)->m_preData));
	void* array = (*(void**)&((CGolfBall*)obj)->m_pPreDataOverflow);
	((CGolfBall*)obj)->m_overflowBlocks = 0;
	if (array != 0) {
		if (array != 0) __dla__FPv((char*)array - kArrayCookieBytes);
		(*(void**)&((CGolfBall*)obj)->m_pPreDataOverflow) = 0;
	}
	*(WVector*)self = *(WVector*)arg;
	((CGolfBall*)obj)->m_preData[0].m_pos = *(WVector*)arg;
	Vec3_Zero(&((CGolfBall*)obj)->m_velocity);
	Vec2_Zero(((CGolfBall*)obj)->m_rotation);
	// The highByteOffset-base form preserves CodeWarrior's large-offset addressing sequence.
	char* base = obj + kBallHighAddressBase;
	unsigned int randomKey = *(unsigned int*)(base + kRandomKeyFromHighBase);
	((CGolfBall*)obj)->m_state = 0;
	((CGolfBall*)obj)->m_powerFlags = 0;
	((CGolfBall*)obj)->m_flags = 0;
	((CGolfBall*)obj)->m_recoveryRequired = 0;
	((CGolfBall*)obj)->m_groundType = 1;
	((CGolfBall*)obj)->m_initCurve = kFloat_0_GolfBallState;
	((CGolfBall*)obj)->m_initSpin = kFloat_0_GolfBallState;
	((CGolfBall*)obj)->m_curve = kFloat_0_GolfBallState;
	((CGolfBall*)obj)->m_spin = kFloat_0_GolfBallState;
	((CGolfBall*)obj)->m_lie = 0;
	// Phase 2: encode zero using the random key, plus an address-bound byte checksum.
	unsigned int zeroHoleIn = 0;
	unsigned char* encodedBytes = (unsigned char*)(base + kEncodedValueFromHighBase);
	*(unsigned int*)(base + kEncodedValueFromHighBase) = ~(zeroHoleIn ^ randomKey);
	unsigned char* addressBytes = (unsigned char*)&encodedBytes;
	int byteIndex = 0;
	int highByteOffset = kBallHighAddressBase + byteIndex;
	int addressCheckOffset = highByteOffset + kAddressCheckFromHighBase;
	unsigned char pointerByte = addressBytes[0];
	unsigned char encodedByte = encodedBytes[0];
	obj[addressCheckOffset] = encodedByte ^ pointerByte;
	byteIndex = 1;
	highByteOffset = kBallHighAddressBase + byteIndex;
	addressCheckOffset = highByteOffset + kAddressCheckFromHighBase;
	pointerByte = addressBytes[1];
	encodedByte = encodedBytes[1];
	obj[addressCheckOffset] = encodedByte ^ pointerByte;
	byteIndex = 2;
	highByteOffset = kBallHighAddressBase + byteIndex;
	addressCheckOffset = highByteOffset + kAddressCheckFromHighBase;
	pointerByte = addressBytes[2];
	encodedByte = encodedBytes[2];
	obj[addressCheckOffset] = encodedByte ^ pointerByte;
	byteIndex = 3;
	highByteOffset = kBallHighAddressBase + byteIndex;
	addressCheckOffset = highByteOffset + kAddressCheckFromHighBase;
	pointerByte = addressBytes[3];
	encodedByte = encodedBytes[3];
	obj[addressCheckOffset] = encodedByte ^ pointerByte;
	// Phase 3: invalidate contact/event indices; preserve the original explicit stores.
	((CGolfBall*)obj)->m_contactMaterialIndex = -1;
	((CGolfBall*)obj)->m_bounceCount = 0;
	((CGolfBall*)obj)->m_shotStage = 0;
	((CGolfBall*)obj)->m_preDataCount = 0;
	((CGolfBall*)obj)->m_eventPreDataIndex[0] = -1;
	((CGolfBall*)obj)->m_eventPreDataIndex[1] = -1;
	((CGolfBall*)obj)->m_eventPreDataIndex[2] = -1;
	((CGolfBall*)obj)->m_eventPreDataIndex[3] = -1;
	((CGolfBall*)obj)->m_eventPreDataIndex[4] = -1;
	((CGolfBall*)obj)->m_eventPreDataIndex[5] = -1;
	((CGolfBall*)obj)->m_eventPreDataIndex[6] = -1;
	((CGolfBall*)obj)->m_eventPreDataIndex[9] = -1;
	((CGolfBall*)obj)->m_eventPreDataIndex[7] = -1;
	((CGolfBall*)obj)->m_eventPreDataIndex[8] = -1;
	((CGolfBall*)obj)->m_eventPreDataIndex[10] = -1;
	((CGolfBall*)obj)->m_eventPreDataIndex[11] = -1;
	((CGolfBall*)obj)->m_eventPreDataIndex[12] = -1;
	((CGolfBall*)obj)->m_eventPreDataIndex[13] = -1;
	((CGolfBall*)obj)->m_eventPreDataIndex[14] = -1;
	((CGolfBall*)obj)->m_firstContactIndex = -1;
	((CGolfBall*)obj)->m_firstContactId = -1;
	((CGolfBall*)obj)->m_lastContactIndex = -1;
	((CGolfBall*)obj)->m_lastContactId = -1;
}

#pragma opt_propagation reset
#pragma opt_strength_reduction reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset

// Register the crowd actor factory and initialize its cached full-circle angle.
void CGallery_RegisterType() {
	const char* name = (gallerySpeechStrings + IFF_OFFSET(GallerySpeechPool, actorType));
	s_galleryFullTurnRadians = kFloat_6_2831855;
	IntPair_Set(&s_galleryTypeInfo, name, &s_actorTypeInfo);
	StringMap_SetByName(ActorFactoryRegistry_GetInstance(), (void*)CGallery_Create, s_galleryActorFactoryName);
	TaskRegistry_AppendFactory(TaskRegistry_GetInstance(), s_galleryTaskFactoryName);
}

} // extern "C"

// Get a simulation record, clamping out-of-range indices to the first or last record.
sExpected* CGolfBall::PreData(int index) {
	char* obj = (char*)this;
	if (index < 0)
		return ((CGolfBall*)obj)->m_preData;
	int count = ((CGolfBall*)obj)->m_preDataCount;
	if (index > count) {
		if (count < kInlineBallRecords)
			return &((CGolfBall*)obj)->m_preData[count];
		void* array = (*(void**)&((CGolfBall*)obj)->m_pPreDataOverflow);
		return ((sExpected*)array + (count - kInlineBallRecords));
	}
	if (index < kInlineBallRecords)
		return &((CGolfBall*)obj)->m_preData[index];
	void* array = (*(void**)&((CGolfBall*)obj)->m_pPreDataOverflow);
	return ((sExpected*)array + (index - kInlineBallRecords));
}

extern "C" {

struct Elem80121A68 {
	unsigned int flag;
	int pad;
	void* ptr;
};

struct NativeBase80201D18 {
	char data[0x34];
	NativeBase80201D18();
	~NativeBase80201D18();
};

struct Vec80121A68 {
	Elem80121A68* data;
	int size;
	int cap;
	~Vec80121A68();
};

// Construction view of CGolfBg; canonical members: classes/CGolfBg.h.
// Retains the actor base and owned string-vector cleanup.
struct CGolfBgConstructorView : NativeBase80201D18 {
	Vec80121A68 vec;
	// Initialize the background base and an empty owned string vector.
	CGolfBgConstructorView() {
		Vec80121A68* vec = &this->vec;
		*(void**)this = (void*)__vt__7CGolfBg;
		vec->data = 0;
		vec->size = 0;
		vec->cap = 0;
		((GolfBackgroundBaseView*)this)->enabled = 1;
		int n = vec->size;
		Elem80121A68* p = vec->data + n;
		vec->size -= n;
		for (int i = n; i != 0; i--) {
			p--;
			if (p != 0 && (p->flag >> 31))
				appFreeUni__FPv(p->ptr);
		}
	}
};

// Allocate a background actor with an empty owned string vector.
void* CGolfBg_Create() {
	return new CGolfBgConstructorView;
}

// 0x80121840 tail-branches to CGolfBall_GetOrAllocatePreDataAt (0x8012184C); callers use the record (e.g. 0x80158478).
void* CGolfBall_NewPreData(void* self) {
	int* value = &((CGolfBall*)self)->m_preDataCount;
	*value += 1;
	return CGolfBall_GetOrAllocatePreDataAt(self, ((CGolfBall*)self)->m_preDataCount);
}

// Destroy the actor base and optionally free the crowd actor.
void* CGallery_dtor(void* self, int shouldDelete) {
	if (self != 0) {
		__dt__18NativeBase80201D18Fv(self, 0);
		if (shouldDelete > 0)
			__dl__FPv(self);
	}
	return self;
}

// Release overflow simulation records while preserving the inline record buffer.
void CGolfBall_ClearBuffer(void* self) {
	char* obj = (char*)self;
	void* array = *(void**)(obj + IFF_OFFSET(CGolfBall, m_pPreDataOverflow));
	((CGolfBall*)obj)->m_overflowBlocks = 0;
	if (array == 0) {
		return;
	} else {
		if (array != 0) __dla__FPv((char*)array - kArrayCookieBytes);
		*(void**)(obj + IFF_OFFSET(CGolfBall, m_pPreDataOverflow)) = 0;
	}
}

// Return the crowd actor type record.
void* CGallery_GetRTTI() {
	return &s_galleryTypeInfo;
}

// Initialize the ball module's cached full-circle angle.
void GolfBall_InitAngleConstant() {
	s_golfBallTwoPi = kFloat_6_2831855;
}

#pragma use_lmw_stmw on
// Get a valid record, growing overflow storage in blocks while retaining array cookies.
// m_preDataCount is the highest permitted index, not a buffer length. Normal use
// appends sequentially via NewPreData: 0..899 inline, then 60-record overflow blocks.
// Growth occurs only at the next exact block boundary; arbitrary forward jumps
// within m_preDataCount do not reserve missing storage. Returned record is borrowed
// and overflow pointers are invalidated by later growth, reset or ClearBuffer.
// Record pointers are 16 bytes past their array cookies. delete[] backs up itself;
// explicit __dla__ calls elsewhere subtract kArrayCookieBytes first.
void* CGolfBall_GetOrAllocatePreDataAt(void* self, int index) {
	char* obj = (char*)self;
	if (index < 0 || index > ((CGolfBall*)obj)->m_preDataCount)
		return 0;
	if (index < kInlineBallRecords)
		return &((CGolfBall*)obj)->m_preData[index];

	// Phase 1: inline indices bypass allocation; overflow grows only at its boundary.
	int count = ((CGolfBall*)obj)->m_overflowBlocks;
	if ((index - kInlineBallRecords) == count * kBallRecordBlockSize) {
	++((CGolfBall*)obj)->m_overflowBlocks;
	if ((index - kInlineBallRecords) == 0) {
		char* array = (char*)new Record8012165C[kBallRecordBlockSize];
		*(void**)(obj + IFF_OFFSET(CGolfBall, m_pPreDataOverflow)) = array;
		memset(array, 0, kBallRecordBlockBytes);
	} else {
		// Phase 2: copy the old prefix to temporary records before deleting old storage.
		char* temp = (char*)new Record8012165C[(index - kInlineBallRecords)];
		unsigned long bytes = (index - kInlineBallRecords) * sizeof(sExpected);
		memcpy(temp, *(void**)(obj + IFF_OFFSET(CGolfBall, m_pPreDataOverflow)), bytes);
		void* old = *(void**)(obj + IFF_OFFSET(CGolfBall, m_pPreDataOverflow));
		delete[] (Record8012165C*)old;
		// Phase 3: allocate one larger block span, restore prefix, zero the new 60 records.
		int newCount = index - (kInlineBallRecords - kBallRecordBlockSize);
		char* grown = (char*)new Record8012165C[newCount];
		*(void**)(obj + IFF_OFFSET(CGolfBall, m_pPreDataOverflow)) = grown;
		memcpy(grown, temp, bytes);
		memset((char*)*(void**)(obj + IFF_OFFSET(CGolfBall, m_pPreDataOverflow)) + (index - kInlineBallRecords) * kBallRecordBytes, 0, kBallRecordBlockBytes);
		delete[] (Record8012165C*)temp;
	}
	}
	return (char*)*(void**)(obj + IFF_OFFSET(CGolfBall, m_pPreDataOverflow)) + (index - kInlineBallRecords) * kBallRecordBytes;
}
#pragma use_lmw_stmw reset

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462118", _ctors_u801203D4_0, CGallery_RegisterType);
STATIC_INIT_ENTRY(".CRT$XCU80462118", _ctors_u801203D4_1, GolfBall_InitAngleConstant);
#pragma pop
