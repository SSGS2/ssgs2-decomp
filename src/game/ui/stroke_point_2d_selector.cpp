#include <decomp/static_init.h>
#include "MSL/error_message.h"
enum { kVoiceNameFormat = 0x4B, kVoiceManagerTypeName = 0xE0 };


// Twelve-byte small-string storage; a containing voice-queue entry adds a four-byte playback delay.
struct VoiceStringStorage { unsigned int flags; int length; void* heapData; };
struct VoiceRangeStorage {
    char* data;
    int count;
    int capacity;
    void* allocator;
    int firstIndex;
};
enum StrokeDirection { kStrokeDirectionLeft = 4, kStrokeDirectionRight = 8 };
enum StrokeElement { kStrokeMarker = 0x4C, kStrokeMessage = 0x4A, kStrokeMessageBackdrop = 0x4B };
enum { kBaseVoiceNumberCount = 13, kVoiceNameBufferBytes = 64, kStrokePlayerFlagSelector = 5 };
// Action IDs verified by the input/visual update paths in stroke_point_2d_dialog.cpp.
enum StrokeActionElement {
    kTomahawkButton=0x2D, kTomahawkCompanion=0x2E,
    kSpikeButton=0x2F, kSpikeCompanion=0x30, kCobraButton=0x31, kCobraCompanion=0x32,
    kPowerSegmentTwo=0x3A, kPowerSegmentThree=0x3B, kPowerRemainder=0x3C,
    kSpinLeftButton=0x3D, kSpinLeftCompanion=0x3E,
    kSpinBottomButton=0x3F, kSpinBottomCompanion=0x40,
    kSpinRightButton=0x41, kSpinRightCompanion=0x42,
    kSpinTopButton=0x43, kSpinTopCompanion=0x44, kConfirmButton=0x49
};

// Translation unit 0x802E6030-0x802E72B0 (30 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"
#include "classes/CStrokePoint2d.h"
#include "classes/CVoiceManager.h"
#include "classes/PendingVoiceEntry.h"

// Borrow voice-manager fields; no ownership transfer.
static inline CVoiceManager* AsCVoiceManager(void* p) { return (CVoiceManager*)p; }

// Borrow selector fields without changing its layout/interface pointer.
static inline CStrokePoint2d* AsCStrokePoint2d(void* p) { return (CStrokePoint2d*)p; }

void* operator new(size_t);
// Construct in caller-provided storage without allocating.
inline void* operator new(size_t, void* p) { return p; }

extern "C" {

extern char s_voiceManagerTypeInfo;
extern const char strokePointSpecialShotStrings[];
extern const float kFloat_6_2831855;
extern float s_voiceManagerRuntimeTwoPi; // Writable two-pi constant initialized during voice-manager registration.
extern char s_actorTypeInfo;
extern char voiceManagerStrings[];
extern char s_voiceManagerActorFactoryKey[]; // Actor-factory key associated with CVoiceManager_Create.
extern char s_voiceManagerTaskFactoryDescriptor[]; // Task-factory descriptor for the voice manager.
extern char __vt__14CStrokePoint2d[];
extern const float kFloat_1_805E0B38;
extern const float kFloat_0_StrokePointControls; // Original scalar value: 0.
void Layout_Init(void*, int);
void __dt__18NativeBase80201D18Fv(void*, int);
void Layout_LoadExtended(int, int, const char*);
void gtl_Vec2_Copy(void*, void*);
void IntPair_Set(void*, void*, void*);
void* ActorFactoryRegistry_GetInstance();
void StringMap_SetByName(void*, void*, const char*);
void* TaskRegistry_GetInstance();
void TaskRegistry_AppendFactory(void*, const char*);
void appFreeUni__FPv(void*);
void __dl__FPv(void*);
void CVoiceManager_Create();
void Layout_DrawWithOpacity(void*, float);
void ScreenBallPreview_DrawStrokePoint(void*);
void Layout_DrawElementWithOpacity(void*, int, float);
void MenuMessage_DrawText(void*);
void MenuMessage_Dtor(void*, int);
void Layout_Destroy(void*, int);
int Layout_Update(void*);
void SoundManager_PlayVoiceByName(void*, void*);
void String_ReplaceRange(void*, int, void*, int, void*, void*, void*, void*);
void Layout_SetElementEnabled(void*, int, int);
void Layout_GetElementPosition(void*, int, void*);
void gtl_Vec2_Copy(void*, void*);
void Layout_ReadElementRect(void*, int, void*);
void Layout_WriteElementRect(void*, int, void*);
void Layout_SetElementPlayerFlag(void*, int, int, int);
void Layout_SetElementPosition(void*, int, void*);
void Layout_GetElementScale(void*, int, void*);
void MenuMessage_Ctor(void*);
void* __nw__FUl(unsigned int);
void MenuMessage_SetPosition(void*, void*);
extern void* g_soundManager;
extern const float kFloat_0_805E0A78; // Original scalar value: 0.
extern const float kFloat_33_StrokePointGauge; // Original scalar value: 33.

// Return the voice-manager runtime type descriptor.
void* CVoiceManager_GetRTTI() {
	return &s_voiceManagerTypeInfo;
}

struct StrokePointInterfaceView;
struct StrokePointInterfaceTable {
	char pad00[0x74];
	int (*fn74)(StrokePointInterfaceView*);
	int (*fn78)(StrokePointInterfaceView*);
	int (*fn7C)(StrokePointInterfaceView*);
	char pad80[0x08];
	void (*fn88)(StrokePointInterfaceView*);
};
struct StrokePointInterfaceView {
	char layoutStorage[0x64];
	float pointX;
	float pointY;
	char savedPointStorage[8];
	float horizontalThreshold;
	float verticalThreshold;
	int inputLocked;
	int powerModeEnabled;
	int directionButtonsEnabled;
	int extendedSpecialShotsEnabled;
	int powerGaugeValue;
	void* directionSelection;
	void* savedDirectionSelection;
	void* confirmRequested;
	void* powerMode;
	void* savedPowerMode;
	void* specialShotSelection;
	void* savedSpecialShotSelection;
	char selectionAnimationStorage[0x0C];
	void* selectionResult;
	char layoutReadyStorage[4];
	int selectionChanged;
	char auxiliaryUiStorage[0x1C];
	StrokePointInterfaceTable* interfaceVtable;
};

struct StrokePointConstructionView {
	char layoutStorage[0x5C];
	float originX;
	float originY;
	float pointX;
	float pointY;
	float savedPointX;
	float savedPointY;
	float horizontalThreshold;
	float verticalThreshold;
	int inputLocked;
	int powerModeEnabled;
	int directionButtonsEnabled;
	int extendedSpecialShotsEnabled;
	int powerGaugeValue;
	int directionSelection;
	int savedDirectionSelection;
	int confirmRequested;
	int powerMode;
	int savedPowerMode;
	int specialShotSelection;
	int savedSpecialShotSelection;
	int fadeFrames;
	int selectionEnabled;
	float opacity;
	int selectionResult;
	int layoutReady;
	int selectionChanged;
	int specialShotButtonsEnabled;
	int powerModeFeedbackFrames;
	int powerModeFeedbackActive;
	float markerStartX;
	float markerEndX;
	int message;
	int selectedIndex;
	void* interfaceVtable;
};
typedef char StrokePointPositionOffset[((int)&((StrokePointInterfaceView*)0)->pointX == 0x64) ? 1 : -1];
typedef char StrokePointDirectionOffset[((int)&((StrokePointInterfaceView*)0)->directionSelection == 0x90) ? 1 : -1];
typedef char StrokePointVirtualOffset[((int)&((StrokePointInterfaceView*)0)->interfaceVtable == 0xE0) ? 1 : -1];
typedef char StrokePointMessageOffset[((int)&((StrokePointConstructionView*)0)->message == 0xD8) ? 1 : -1];
typedef char StrokePointIndexOffset[((int)&((StrokePointConstructionView*)0)->selectedIndex == 0xDC) ? 1 : -1];

// Free queued sound names and pending-voice storage, then destroy the actor base.
void* CVoiceManager_dtor(void* ptr, int flags) {
	if (ptr != 0) {
		void** items = (*(void***)&AsCVoiceManager(ptr)->pendingVoices);
		if (items != 0) {
			char* p = 0;
			int count = AsCVoiceManager(ptr)->pendingVoiceCount;
			int zero = count;
			zero = zero - count;
			AsCVoiceManager(ptr)->pendingVoiceCount = zero;
			p = (char*)items + count * 16;
			while (count != 0) {
				p = p - 0x10;
				if (p != 0 && p != 0) {
					if ((*(unsigned int*)p >> 31) != 0)
						appFreeUni__FPv(*(void**)(p + 8));
				}
				count = count - 1;
			}
			appFreeUni__FPv(AsCVoiceManager(ptr)->pendingVoices);
		}
		__dt__18NativeBase80201D18Fv(ptr, 0);
		if (flags > 0)
			__dl__FPv(ptr);
	}
	return ptr;
}

// Destroy a selected pending-voice range and release its vector storage.
void* VoiceRangeStorage_dtor(void* ptr, int flags) {
	if (ptr != 0) {
		char* base = ((VoiceRangeStorage*)ptr)->data;
		int c2 = ((VoiceRangeStorage*)ptr)->firstIndex;
		int c1 = ((VoiceRangeStorage*)ptr)->count;
		char* end;
		char* start = base + c2 * 0x10;
		end = start + c1 * 0x10;
		while (end > start) {
			end -= 0x10;
			if (end != 0 && end != 0) {
				if ((*(unsigned int*)end >> 31) != 0)
					appFreeUni__FPv(*(void**)(end + 8));
			}
		}
		((VoiceRangeStorage*)ptr)->count = 0;
		if (ptr != 0) {
			int count = ((VoiceRangeStorage*)ptr)->count;
			char* base2 = ((VoiceRangeStorage*)ptr)->data;
			if (base2 != 0) {
				((VoiceRangeStorage*)ptr)->count = count;
				while (count != 0) {
					base2 -= 0x10;
					if (base2 != 0 && base2 != 0) {
						if ((*(unsigned int*)base2 >> 31) != 0)
							appFreeUni__FPv(*(void**)(base2 + 8));
					}
					count = count - 1;
				}
				appFreeUni__FPv(((VoiceRangeStorage*)ptr)->data);
			}
		}
		if (flags > 0)
			__dl__FPv(ptr);
	}
	return ptr;
}

// Initialize the stroke-point selector with no selection, no message and zeroed UI state.
void* CStrokePoint2d_Ctor(StrokePointConstructionView* self) {
	Layout_Init(self, 5);
	float zero = kFloat_0_StrokePointControls;
	int izero = 0;
	self->originX = zero;
	self->interfaceVtable = (StrokePointInterfaceTable*)__vt__14CStrokePoint2d;
	self->originY = zero;
	self->pointX = zero;
	self->pointY = zero;
	self->savedPointX = zero;
	self->savedPointY = zero;
	self->horizontalThreshold = zero;
	self->verticalThreshold = zero;
	self->inputLocked = izero;
	self->powerModeEnabled = izero;
	self->directionButtonsEnabled = izero;
	self->extendedSpecialShotsEnabled = izero;
	self->powerGaugeValue = izero;
	self->directionSelection = izero;
	self->savedDirectionSelection = izero;
	self->confirmRequested = izero;
	self->powerMode = izero;
	self->savedPowerMode = izero;
	self->specialShotSelection = izero;
	self->savedSpecialShotSelection = izero;
	self->fadeFrames = izero;
	self->selectionEnabled = izero;
	self->opacity = zero;
	self->selectionResult = izero;
	self->layoutReady = izero;
	AsCStrokePoint2d(self)->selectionChanged = izero;
	self->specialShotButtonsEnabled = izero;
	self->powerModeFeedbackFrames = izero;
	self->powerModeFeedbackActive = izero;
	self->markerStartX = zero;
	self->markerEndX = zero;
	self->message = 0;
	self->selectedIndex = -1;
	return self;
}

// Draw the selector with opacity, the fully visible ball preview, marker and optional message.
int CStrokePoint2d_Draw(StrokePointInterfaceView* self, void* arg) {
	Layout_DrawWithOpacity(self, AsCStrokePoint2d(self)->opacity);
	if (kFloat_1_805E0B38 == AsCStrokePoint2d(self)->opacity)
		ScreenBallPreview_DrawStrokePoint(arg);
	Layout_DrawElementWithOpacity(self, 0x4C, AsCStrokePoint2d(self)->opacity);
	if (AsCStrokePoint2d(self)->message != 0)
		MenuMessage_DrawText(AsCStrokePoint2d(self)->message);
	return 1;
}

// Release the selector message and layout before optional deletion.
StrokePointInterfaceView* CStrokePoint2d_dtor(StrokePointInterfaceView* self, int flags) {
	if (self != 0) {
		self->interfaceVtable = (StrokePointInterfaceTable*)__vt__14CStrokePoint2d;
		if (AsCStrokePoint2d(self)->message != 0) {
			MenuMessage_Dtor(AsCStrokePoint2d(self)->message, 1);
			AsCStrokePoint2d(self)->message = 0;
		}
		AsCStrokePoint2d(self)->message = 0;
		Layout_Destroy(self, 0);
		if (flags > 0)
			__dl__FPv(self);
	}
	return self;
}

struct StrokePointVirtualBase { char pad[224]; };
class StrokePointUpdateHooks : public StrokePointVirtualBase { public: 
virtual void v08(); virtual void v0C(); virtual void v10(); virtual void v14(); virtual void v18(); virtual void v1C(); virtual void v20(); virtual void v24(); virtual void v28(); virtual void v2C(); virtual void v30(); virtual void v34(); virtual void v38(); virtual void v3C(); virtual void v40(); virtual void v44(); virtual void v48(); virtual void v4C(); virtual void v50(); virtual void v54(); virtual void v58(); virtual void v5C(); virtual void v60(); virtual void v64(); virtual void v68(); virtual void v6C(); virtual void v70(); virtual int AdvanceDialogAnimation(); virtual int ConsumeActivatedControls(); virtual int RefreshDirtyControls(); virtual void v80(); virtual void v84(); virtual void UpdateHoveredHelpText();
};
// Update animation, consume input, update hovered help, then refresh dirty controls.
// Vtable slots 74/78/7C/88 map to vfunc_27/28/29/32 in phase1_class_tables.
// Return their bitwise status intersection with Layout_Update bit 0; every hook
// still runs after a zero result. ConsumeActivatedControls returns 0 while closed;
// RefreshDirtyControls returns 0 when unready or clean. This is not an error code.
int CStrokePoint2d_UpdateDialog(StrokePointInterfaceView* self) {
	int result = Layout_Update(self) & 1;
	result &= ((StrokePointUpdateHooks*)self)->AdvanceDialogAnimation();
	result &= ((StrokePointUpdateHooks*)self)->ConsumeActivatedControls();
	if (AsCStrokePoint2d(self)->message != 0)
		((StrokePointUpdateHooks*)self)->UpdateHoveredHelpText();
	result &= ((StrokePointUpdateHooks*)self)->RefreshDirtyControls();
	return result;
}

// Set direction thresholds and clear a left/right selection that no longer reaches its threshold.
int CStrokePoint2d_SetDirectionThresholds(StrokePointInterfaceView* self, float x, float y) {
	int state = (int)self->directionSelection;
	self->horizontalThreshold = x;
	self->verticalThreshold = y;
	if (state == kStrokeDirectionLeft) {
	if (self->pointX > -(float)(double)x)
		self->directionSelection = 0;
	} else if (state == kStrokeDirectionRight) {
		if (self->pointX < (float)(double)x)
			self->directionSelection = 0;
	}
	return 1;
}

// Register voice-manager type information, actor factory and task factory.
void CVoiceManager_RegisterFactory() {
	float f = kFloat_6_2831855;
	s_voiceManagerRuntimeTwoPi = f;
	char* p = voiceManagerStrings;
	IntPair_Set(&s_voiceManagerTypeInfo, p + kVoiceManagerTypeName, &s_actorTypeInfo);
	StringMap_SetByName(ActorFactoryRegistry_GetInstance(), CVoiceManager_Create, s_voiceManagerActorFactoryKey);
	TaskRegistry_AppendFactory(TaskRegistry_GetInstance(), s_voiceManagerTaskFactoryDescriptor);
}

// Release heap-backed small-string storage before optional deletion.
void* dtor_802E6030(StrokePointInterfaceView* self, int flags) {
	if (self != 0) {
		if (self != 0 && (((VoiceStringStorage*)self)->flags >> 31) != 0)
			appFreeUni__FPv(((VoiceStringStorage*)self)->heapData);
		if (flags > 0)
			__dl__FPv(self);
	}
	return self;
}

// Load the special-shot layout resource.
int CStrokePoint2d_LoadSpecialShotLayout(int a, int b) {
	Layout_LoadExtended(a, b, strokePointSpecialShotStrings);
	return 1;
}

#pragma opt_propagation off
#pragma opt_lifetimes off
#pragma opt_common_subs off
// Count down voice delays; play expired names and compact the queue without skipping entries.
// Pending queue entries are 16 bytes: 12-byte small string then delay seconds.
// A negative updated delay expires; zero waits until a later tick. Playback borrows
// the name synchronously. Compaction assigns strings (deep-copy if either is heap),
// then destroys only the final stale string. Retry the same index after removal.
void CVoiceManager_OnProcess(void* ptr, float elapsedSeconds) {
	char* pendingEntry;
	void* self;
	pendingEntry = (*(char**)&AsCVoiceManager(ptr)->pendingVoices);
	float zero = kFloat_0_805E0A78;
	self = ptr;
	while (pendingEntry != (char*)AsCVoiceManager(self)->pendingVoices + AsCVoiceManager(self)->pendingVoiceCount * 0x10) {
		float remainingDelaySeconds = AsPendingVoice802E6030(pendingEntry)->delaySeconds - elapsedSeconds; // +0xC
		AsPendingVoice802E6030(pendingEntry)->delaySeconds = remainingDelaySeconds;
		if (remainingDelaySeconds < zero) {
			unsigned int flag = *(unsigned int*)pendingEntry >> 31;
			void* voiceName;
			if (flag == 0) {
				voiceName = pendingEntry + 1;
			} else {
				voiceName = *(void**)(pendingEntry + 8);
			}
			SoundManager_PlayVoiceByName(g_soundManager, voiceName);
			char* end = (char*)AsCVoiceManager(self)->pendingVoices + AsCVoiceManager(self)->pendingVoiceCount * 0x10;
			// Shift every following entry left; retain queue capacity and string ownership.
			if (((int)(end - pendingEntry) / 16) - 1 != 0) {
				char* sourceEntry;
				char* destinationEntry;
				destinationEntry = pendingEntry;
				sourceEntry = pendingEntry + 0x10;
				while (sourceEntry < end) {
					unsigned int sourceStringFlags;
					unsigned int destinationIsHeap;
					destinationIsHeap = *(unsigned int*)destinationEntry >> 31;
					if (!destinationIsHeap && !((sourceStringFlags = *(unsigned int*)sourceEntry) >> 31)) {
						unsigned int second = *(unsigned int*)(sourceEntry + 4);
						*(unsigned int*)destinationEntry = sourceStringFlags;
						*(unsigned int*)(destinationEntry + 4) = second;
						*(unsigned int*)(destinationEntry + 8) = *(unsigned int*)(sourceEntry + 8);
					} else {
						unsigned int destinationLengthAbiWord = destinationIsHeap ? *(unsigned int*)(destinationEntry + 4) : (unsigned int)(*(unsigned char*)destinationEntry & 0x7f);
						char* sourceText;
						int sourceLength;
						if (!(*(unsigned int*)sourceEntry >> 31)) {
							sourceText = sourceEntry + 1;
							sourceLength = *(unsigned char*)sourceEntry & 0x7f;
						} else {
							sourceText = *(char**)(sourceEntry + 8);
							sourceLength = *(int*)(sourceEntry + 4);
						}
						struct Alloc { char c; } alloc;
						Alloc copy = alloc;
						((void(*)(void*, int, void*, void*, void*, void*))String_ReplaceRange)(destinationEntry, 0, (void*)destinationLengthAbiWord, sourceText, sourceText + sourceLength, &copy);
					}
					AsPendingVoice802E6030(destinationEntry)->delaySeconds = AsPendingVoice802E6030(sourceEntry)->delaySeconds; // +0xC
					destinationEntry += 0x10;
					sourceEntry += 0x10;
				}
			}
			// Destroy the duplicate last name after reducing the logical count.
			int count = AsCVoiceManager(self)->pendingVoiceCount - 1;
			char* base = (char*)AsCVoiceManager(self)->pendingVoices;
			AsCVoiceManager(self)->pendingVoiceCount = count;
			char* last = base + count * 0x10;
			if (last != 0 && last != 0 && (*(unsigned int*)last >> 31)) appFreeUni__FPv(*(void**)(last + 8));
		} else pendingEntry += 0x10;
	}
}
#pragma opt_propagation reset
#pragma opt_lifetimes reset
#pragma opt_common_subs reset

struct Obj802CE2BC {
	char data[0x0C];
	// Initialize the owned stroke-point message object.
	Obj802CE2BC() { MenuMessage_Ctor(this); }
};

// Initialize selector rectangles, marker positions and the lazily created message object.
// Paired IDs are button/companion layers for each named action (stroke_point_2d_dialog.cpp).
// Companion artwork is unverified, but both receive identical interaction states.
// Rectangles/positions are in layout coordinates; the +33 X shifts place gauge
// segments two and three. The remainder supplies gauge origin and segment scale.
int CStrokePoint2d_InitLayout(StrokePointConstructionView* self) {
	void* object = 0;
	// Phase 1: hide transient elements and capture the marker origin.
	Layout_SetElementEnabled(self, kStrokeMarker, 0);
	Layout_SetElementEnabled(self, kStrokeMessage, 0);
	Layout_SetElementEnabled(self, kStrokeMessageBackdrop, 0);
	Layout_GetElementPosition(self, kStrokeMarker, &self->originX);
	gtl_Vec2_Copy(&self->pointX, &self->originX);
	struct PairValue { float x, y, z, w; } value1; // Layout_Read/WriteElementRect move 4 words
	// Phase 2: align each companion with the rectangle of its action button.
	Layout_ReadElementRect(self, kSpinLeftButton, &value1);
	Layout_WriteElementRect(self, kSpinLeftCompanion, &value1);
	Layout_ReadElementRect(self, kSpinRightButton, &value1);
	Layout_WriteElementRect(self, kSpinRightCompanion, &value1);
	Layout_ReadElementRect(self, kSpinTopButton, &value1);
	Layout_WriteElementRect(self, kSpinTopCompanion, &value1);
	Layout_ReadElementRect(self, kSpinBottomButton, &value1);
	Layout_WriteElementRect(self, kSpinBottomCompanion, &value1);
	Layout_ReadElementRect(self, kTomahawkButton, &value1);
	Layout_WriteElementRect(self, kTomahawkCompanion, &value1);
	Layout_ReadElementRect(self, kSpikeButton, &value1);
	Layout_WriteElementRect(self, kSpikeCompanion, &value1);
	Layout_ReadElementRect(self, kCobraButton, &value1);
	Layout_WriteElementRect(self, kCobraCompanion, &value1);
	// Phase 3: enable confirmation interaction and position the gauge segments.
	Layout_SetElementPlayerFlag(self, kConfirmButton, 1, kStrokePlayerFlagSelector);
	float value2[2];
	Layout_GetElementPosition(self, kPowerSegmentTwo, value2);
	value2[0] += kFloat_33_StrokePointGauge;
	Layout_SetElementPosition(self, kPowerSegmentTwo, value2);
	value2[0] += kFloat_33_StrokePointGauge;
	Layout_SetElementPosition(self, kPowerSegmentThree, value2);
	Layout_GetElementPosition(self, kPowerRemainder, value2);
	self->markerStartX = value2[0];
	Layout_GetElementScale(self, kPowerRemainder, value2);
	self->markerEndX = value2[0];
	// Phase 4: allocate the owned help message only once.
	if (self->message == 0) {
		object = (void*)new Obj802CE2BC();
		self->message = (int)object;
		Layout_GetElementPosition(self, kStrokeMessage, value2);
		MenuMessage_SetPosition((void*)self->message, value2);
	}
	self->layoutReady = 1;
	AsCStrokePoint2d(self)->selectionChanged = 1;
	return 1;
}

// Update the stroke point and request a UI refresh only when its coordinates change.
int CStrokePoint2d_SetPointPosition(StrokePointInterfaceView* self, float x, float y) {
	if (self->pointX == x && self->pointY == y)
		return 1;
	self->pointX = x;
	self->pointY = y;
	AsCStrokePoint2d(self)->selectionChanged = 1;
	return 1;
}

// Restore the saved stroke point, direction, special shot and power mode, then request a UI refresh.
int CStrokePoint2d_RestoreSavedSelection(StrokePointInterfaceView* self) {
	gtl_Vec2_Copy(&self->pointX, &self->savedPointStorage);
	self->directionSelection = self->savedDirectionSelection;
	self->specialShotSelection = self->savedSpecialShotSelection;
	self->powerMode = self->savedPowerMode;
	AsCStrokePoint2d(self)->selectionChanged = 1;
	return 1;
}

// Return the selection result word using the existing pointer-shaped ABI.
void* CStrokePoint2d_vfunc_11(StrokePointInterfaceView* self) {
	return self->selectionResult;
}

// Return the confirmation-request word using the existing pointer-shaped ABI.
void* CStrokePoint2d_vfunc_14(StrokePointInterfaceView* self) {
	return self->confirmRequested;
}

// Return the selected direction bits using the existing pointer-shaped ABI.
void* CStrokePoint2d_vfunc_15(StrokePointInterfaceView* self) {
	return self->directionSelection;
}

// Return the selected special-shot bits using the existing pointer-shaped ABI.
void* CStrokePoint2d_vfunc_16(StrokePointInterfaceView* self) {
	return self->specialShotSelection;
}

// Return the selected power mode using the existing pointer-shaped ABI.
void* CStrokePoint2d_vfunc_17(StrokePointInterfaceView* self) {
	return self->powerMode;
}

// Set whether selection input is locked.
int CStrokePoint2d_SetPointerSelectionDisabled(StrokePointInterfaceView* self, int value) {
	self->inputLocked = value;
	return 1;
}

// Set whether power-mode selection is enabled.
int CStrokePoint2d_SetPowerModeEnabled(StrokePointInterfaceView* self, int value) {
	self->powerModeEnabled = value;
	return 1;
}

// Set whether directional selection buttons are enabled.
int CStrokePoint2d_SetDirectionButtonsEnabled(StrokePointInterfaceView* self, int value) {
	self->directionButtonsEnabled = value;
	return 1;
}

// Set gauge fill (33 units per segment; 99 fills all three displayed segments).
int CStrokePoint2d_SetPowerGaugeFill(StrokePointInterfaceView* self, int value) {
	self->powerGaugeValue = value;
	return 1;
}

// Allow Cobra/Spike selection when power is active; Tomahawk uses power alone.
int CStrokePoint2d_SetExtendedSpecialShotsAllowed(StrokePointInterfaceView* self, int value) {
	self->extendedSpecialShotsEnabled = value;
	return 1;
}

// Clear the selected direction bits.
int CStrokePoint2d_ClearDirectionSelection(StrokePointInterfaceView* self) {
	self->directionSelection = 0;
	return 1;
}

// Clear the selected special-shot bits.
int CStrokePoint2d_ClearSpecialShotSelection(StrokePointInterfaceView* self) {
	self->specialShotSelection = 0;
	return 1;
}

// Clear the selected power mode.
int CStrokePoint2d_ClearPowerMode(StrokePointInterfaceView* self) {
	self->powerMode = 0;
	return 1;
}

// Clear direction, confirmation, power mode and special-shot selection.
int CStrokePoint2d_ClearShotSelections(StrokePointInterfaceView* self) {
	self->directionSelection = 0;
	self->confirmRequested = 0;
	self->powerMode = 0;
	self->specialShotSelection = 0;
	return 1;
}

extern const int voiceBaseCharacterNameFormats[];
extern const int voiceAlternateCharacterNameFormats[];
extern char voiceManagerStrings[];
extern "C" void* memcpy(void*, const void*, unsigned int);
extern "C" void* memset(void*, int, unsigned int);
void String_ReserveCapacity(void*, unsigned int);
void* appMallocDummy__FUl(unsigned int);
void* __nwa__FUl(unsigned int);
void __dla__FPv(void*);
extern "C" void __dt__Q23std9exceptionFv(void*, int);
void __dt__Q23std12length_errorFv(void*, int);
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];
struct Alloc802E6250 { char byte; };
typedef void RethrowFn802E6250(const char*, void*, void*) __attribute__((noreturn));

struct FormatTable802E6250 { int v[8]; };
// Local voice-name container specialization, separate from the 16-byte pending
// queue above. Each value is a 12-byte small string: high flag bit selects heap
// ownership (+4 length,+8 pointer), otherwise inline bytes start at +1. Copies
// duplicate heap text; destruction frees only heap text. Names appended below
// are owned by outputNames, never pointers into the temporary format buffer.
struct String802E6250 {
	unsigned int flag;
	unsigned int len;
	char* ptr;
	// Copy formatted text into an owning small string.
	String802E6250(const char* str, const Alloc802E6250& alloc) {
		unsigned int zero = 0;
		flag = zero; len = zero; ptr = (char*)zero;
		unsigned int n = strlen(str);
		String_ReserveCapacity(this, n);
		Alloc802E6250 copy = alloc;
		((void(*)(void*, int, int, void*, void*, void*))String_ReplaceRange)(this, 0, 0, (void*)str, (void*)(str + n), &copy);
	}
	// Copy inline bytes or deep-copy heap-backed voice-name text.
	String802E6250(const String802E6250& other) {
		try {
			unsigned int f = other.flag;
			if (!(f >> 31)) {
				unsigned int n = other.len;
				flag = f; len = n; ptr = other.ptr;
			} else {
				flag = 0; len = 0; ptr = 0;
				String_ReserveCapacity(this, other.len);
				Alloc802E6250 alloc;
				Alloc802E6250 copy = alloc;
				((void(*)(void*, int, int, void*, void*, void*))String_ReplaceRange)(this, 0, 0, other.ptr, other.ptr + other.len, &copy);
			}
		} catch (...) { ((RethrowFn802E6250*)__throw)(0, 0, 0); }
	}
	// Free text only when the string owns a heap allocation.
	~String802E6250() { if (flag >> 31) appFreeUni__FPv(ptr); }
};
struct Vector802E6250 {
	String802E6250* data;
	unsigned int size, capacity;
	void push(const String802E6250&, const Alloc802E6250&);
	// Append one copied voice name with the default allocator tag.
	void push(const String802E6250& local) { push(local, Alloc802E6250()); }
	unsigned int nextCapacity(const unsigned int&) const;
	// Return the 32-bit allocation limit for 12-byte string elements.
	unsigned int maxSize() const {
		Alloc802E6250 alloc = Alloc802E6250();
		return 0x15555555;
	}
};
// Growth guard owns a new allocation and the constructed range [offset,offset+size).
// append constructs the new suffix first; prepend transfers old 12-byte values
// and zeros their sources; install swaps buffers. The guard then frees old storage
// without freeing the transferred strings. Unwinding destroys its live range.
struct Storage802E6250 {
	String802E6250* data;
	unsigned int size, capacity;
	unsigned int* allocator;
	unsigned int offset;
	char reservedTail[4];
	// Initialize an empty growth guard linked to the vector capacity word.
	Storage802E6250(unsigned int* a) {
		data = 0; size = 0; capacity = 0; allocator = a; offset = 0;
	}
	// Construct the new suffix and track its live range for unwind cleanup.
	void append(unsigned int pos, int count, const String802E6250& local) {
		offset = pos;
		String802E6250* item = size + (data + pos);
		Alloc802E6250 alloc = Alloc802E6250();
		while (count != 0) {
			new(item) String802E6250(local);
			--count; ++item; ++size;
		}
	}
	// Transfer old string values before the suffix and clear their previous owners.
	int prepend(String802E6250* last, String802E6250* first) {
		int n = ((char*)last - (char*)first) / 12;
		Alloc802E6250 alloc = Alloc802E6250();
		offset -= n;
		// Transfer ownership before clearing the source entries.
		memcpy(data + offset, first, n * 12);
		memset(first, 0, n * 12);
		return n;
	}
	// Publish replacement storage and retain the old buffer for guard cleanup.
	void install(Vector802E6250* vec, int n) {
		unsigned int oldsize;
		String802E6250* olddata;
		unsigned int oldcap;
		unsigned int newsize;
		unsigned int newcap;
		String802E6250* newdata;
		newsize = size + n;
		oldsize = vec->size;
		size = newsize;
		olddata = vec->data;
		newdata = data;
		oldcap = vec->capacity;
		newcap = capacity;
		String802E6250* start = olddata + offset;
		String802E6250* end = start + oldsize;
		data = olddata;
		Alloc802E6250 alloc = Alloc802E6250();
		vec->capacity = newcap;
		capacity = oldcap;
		vec->data = newdata;
		vec->size = newsize;
		size = oldsize;
		while (end > start) {
			--end;
			if (end != 0 && (end->flag >> 31)) appFreeUni__FPv(end->ptr);
		}
		size = 0;
	}
	// Destroy live heap strings in reverse order while retaining allocation.
	void clear() {
		String802E6250* p;
		unsigned int n = size;
		p = data + n;
		Alloc802E6250 alloc = Alloc802E6250();
		size = size - n;
		while (n != 0) {
			--p;
			if (p != 0 && (p->flag >> 31)) appFreeUni__FPv(p->ptr);
			--n;
		}
	}
	// Destroy any live strings and free the guarded allocation.
	~Storage802E6250() {
		if (data != 0) {
			clear();
			appFreeUni__FPv(data);
		}
	}
};
// Shared RTTI: std::tr1::detail::shared_ptr_deleter<char, std::detail::default_delete<char[]> >.
// Use the common 16-byte ABI view in MSL/error_message.h; keep this local alias for callers.
typedef ::std::detail::__message_control ErrorRef802E6250;
struct Guard802E6250 {
	char* text;
	Alloc802E6250* allocator;
	// Free exception text unless ownership has transferred to its reference block.
	~Guard802E6250() { if (text) __dla__FPv(text); }
};
struct ErrorText802E6250 {
	char* text;
	ErrorRef802E6250* ref;
	// Allocate and share a NUL-terminated copy of the diagnostic text.
	ErrorText802E6250(const char* message) {
		Alloc802E6250 alloc = Alloc802E6250();
		unsigned int length = strlen(message) + 1;
		text = (char*)__nwa__FUl(length * 2 - length);
		{
			Guard802E6250 guard;
			guard.text = text;
			guard.allocator = &alloc;
			char* p = text;
			ref = new ErrorRef802E6250(p);
			guard.text = 0;
		}
		strcpy(text, message);
	}
};
} // extern "C"
namespace std {
struct exception {
	void* vtable;
	// Initialize the standard logic-error base identity.
	exception() : vtable(__vt__Q23std11logic_error) {}
	~exception();
};
struct logic_error : exception {
	ErrorText802E6250 text;
	// Construct copied text for the standard logic-error object.
	logic_error(const char*& message) : text(message) {}

    ~logic_error();
};
struct length_error : logic_error {
	// Set the length-error identity on the constructed logic-error base.
	length_error(const char* message) : logic_error(message) {
		vtable = __vt__Q23std12length_error;
	}
	~length_error();
};
}
extern "C" {

// Throw the voice-name vector length error.
inline void ThrowLength802E6250() {
	throw std::length_error(voiceManagerStrings);
}
// Construction and length errors use explicit calls below; no hidden control flow.

#pragma push
#pragma always_inline on
#pragma inline_max_size(10000)
#pragma inline_max_total_size(100000)
// Recommend capacity for 12-byte sound-name strings with overflow checks.
inline unsigned int Vector802E6250::nextCapacity(const unsigned int& extra) const {
	unsigned int result = capacity;
	if (extra > 0x15555555U - result) { ThrowLength802E6250(); }
	if (result < 0x071C71C7U) {
		unsigned int grow = (3 * (result + 1)) / 5;
		result += *(grow < extra ? &extra : &grow);
	} else if (result < 0x0E38E38EU) {
		unsigned int grow = (result + 1) / 2;
		result += *(grow < extra ? &extra : &grow);
	} else {
		result = 0x15555555;
	}
	return result;
}

// Copy a sound name into the vector, growing storage with exception cleanup when full.
inline void Vector802E6250::push(const String802E6250& local, const Alloc802E6250&) {
	Vector802E6250* vec = this;
	if (vec->size < vec->capacity) {
		String802E6250* item = vec->data + vec->size;
		new(item) String802E6250(local);
		vec->size = vec->size + 1;
	} else {
		Alloc802E6250 growAlloc = Alloc802E6250();
		if (vec->maxSize() - vec->capacity < 1U) { ThrowLength802E6250(); }
		unsigned int extra = vec->size + 1 - vec->capacity;
		Storage802E6250 storage(&vec->capacity);
		unsigned int capacity = vec->nextCapacity(extra);
		Alloc802E6250 reserveAlloc = Alloc802E6250();
		if (capacity > 0x15555555U) { ThrowLength802E6250(); }
		storage.data = (String802E6250*)appMallocDummy__FUl(capacity * 12);
		storage.capacity = capacity;
		// Build suffix before moving old values, so a failed copy leaves vec intact.
		storage.append(vec->size, 1, local);
		int n = storage.prepend(vec->data + vec->size, vec->data);
		storage.install(vec, n);
	}
}
#pragma always_inline reset
#pragma use_lmw_stmw on
// Append a formatted character voice name, selecting the alternate series above voice number 13.
// outputNames must be this unit's 12-byte string vector; character indices must
// select valid eight-entry format tables. Voice zero and missing alternate IDs
// append nothing. Other calls append one owned string; manager is unused.
extern "C" void CVoiceManager_AppendFormattedVoiceName(void* manager, void* outputNames, int voiceNumber, int characterIndex, int alternateCharacterIndex) {
	if (voiceNumber == 0) return;
	FormatTable802E6250 baseNameFormats = *(FormatTable802E6250*)voiceBaseCharacterNameFormats;
	FormatTable802E6250 alternateNameFormats = *(FormatTable802E6250*)voiceAlternateCharacterNameFormats;
	int name, number;
	if (voiceNumber <= kBaseVoiceNumberCount) {
		name = baseNameFormats.v[characterIndex];
		number = voiceNumber;
	} else {
		if (alternateCharacterIndex == 0) return;
		if (alternateCharacterIndex == -1) return;
		name = alternateNameFormats.v[alternateCharacterIndex];
		number = voiceNumber - kBaseVoiceNumberCount;
	}
	char voiceName[kVoiceNameBufferBytes];
	const char* strings = voiceManagerStrings;
	sprintf(voiceName, strings + kVoiceNameFormat, name, number);
	Alloc802E6250 v54;
	String802E6250 local(voiceName, v54);
	Vector802E6250* vec = (Vector802E6250*)outputNames;
	vec->push(local);
	(void)manager;
}
#pragma pop

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462578", _ctors_u802E6030_0, CVoiceManager_RegisterFactory);
#pragma pop
