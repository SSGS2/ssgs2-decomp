// Translation unit 0x80267C20-0x80268730 (8 functions; boundaries inferred by decomp-toolkit from extab).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
// Each buffer owns one allocation; size counts records, not bytes. Member-address
// guards below come from container destructors. ReleaseSampleStorage delegates
// to V80267: its MSL/vector.h cleanup subtracts the count to clear size before
// releasing the allocation; this is not a reference-count decrement.
struct SampleBuffer {
    void* data;
    int size;
};

struct SampleBufferStorage {
    SampleBuffer samples;
};

struct SampleBufferOwner {
    int pad[3];
    SampleBuffer samples;
};

#include "MSL/vector.h"
struct E80267 { unsigned int w[1]; };
typedef std::vector<E80267, app::app_allocator<E80267> > V80267;
enum { kSampleHistoryCapacity = 120, kImpulseWindowSamples = 30, kGestureImpulse = 0, kGestureHorizontalReversal = 1, kGestureVerticalReversal = 2, kDirectionNegative = 1, kDirectionPositive = 2, kDirectionBoth = 3 };

extern "C" {

void appFreeUni__FPv(void*);
void __dl__FPv(void*);

// Clear the constructed-record count and release the owned backing allocation.
static inline void ReleaseSampleStorage(SampleBuffer* buffer) {
    ((V80267*)buffer)->~V80267();
}

// Mark both party-item selection ids as absent.
void PartyItem_ResetIds(int* p) {
    p[2] = -1;
    p[3] = -1;
}

// Destroy a sample buffer and optionally release its owner storage.
SampleBufferStorage* SampleBufferStorage_dtor(SampleBufferStorage* self, int flag) {
    if (self) {
        ReleaseSampleStorage(&self->samples);
        if (flag > 0) __dl__FPv(self);
    }
    return self;
}

// Destroy the embedded sample buffer and optionally release its wrapper.
SampleBufferOwner* SampleBufferOwner_dtor(SampleBufferOwner* self, int flag) {
    if (self) {
        ReleaseSampleStorage(&self->samples);
        if (flag > 0) __dl__FPv(self);
    }
    return self;
}

struct InputManagerStorage { char x[0x100]; };
extern InputManagerStorage s_gameInputManager;
extern char s_inputManagerInitialized;
extern char s_inputManagerDtorRegistration_MotionHistory[]; // Input-manager global destructor registration storage.
void InputManager_Init(void*);
void InputManager_Dtor(void*);
void InputManager_SetSelectedPlayer(InputManagerStorage*, int);
int InputManager_GetSelectedPad(InputManagerStorage*);
void gtl_CScreenPointer_SetSamplingParameter(int, int, void*);
void* __register_global_object(void*, void*, void*);
extern int __vt__9CAccState[];

// Lazily construct the shared input manager and register its shutdown cleanup.
static inline InputManagerStorage* GetInputManager() {
    if (!((InputManagerStorage *&)Sing_805DC5C8())) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(&s_gameInputManager);
            __register_global_object(&s_gameInputManager, (void*)InputManager_Dtor, s_inputManagerDtorRegistration_MotionHistory);
            s_inputManagerInitialized = 1;
        }
        ((InputManagerStorage *&)Sing_805DC5C8()) = &s_gameInputManager;
    }
    return ((InputManagerStorage *&)Sing_805DC5C8());
}

struct AccState {
    int* vtable;
    SampleBufferOwner primaryHistory;
    char gap[4];
    SampleBufferOwner secondaryHistory;

    char gap2[0xc];
    int playerIndex;
    int savedSamplingParameters;
    int savedSamplingParameter2;
    int samplingOverridden;
};

// Destroy a second sample-buffer wrapper with the original nested cleanup guards.
SampleBufferOwner* PadMotionHistoryStorage_Dtor(SampleBufferOwner* self, int flag) {
    if (self) {
        if (&self->samples) ReleaseSampleStorage(&self->samples);
        if (flag > 0) __dl__FPv(self);
    }
    return self;
}

} // extern "C"

// Saves and restores the manager's selected handle. The original class name is unknown.
struct Holder {
    int previousPlayer;
    // Remember the selected player so the frame sampler can switch temporarily.
    Holder(int player) : previousPlayer(player) {}
    ~Holder();
};

// Restore the manager's previous selected player on normal or exceptional exit.
Holder::~Holder() {
    InputManagerStorage* m = GetInputManager();
    InputManager_SetSelectedPlayer(m, previousPlayer);
}

extern "C" {

// Restore overridden pointer sampling and release both motion histories.
AccState* CAccState_dtor(AccState* self, int flag) {
    if (self) {
        self->vtable = __vt__9CAccState;
        if (self->samplingOverridden) {
            InputManagerStorage* m = GetInputManager();
            InputManager_SetSelectedPlayer(m, self->playerIndex);
            if (int r = InputManager_GetSelectedPad(m)) gtl_CScreenPointer_SetSamplingParameter(r, 0, &self->savedSamplingParameters);
            self->samplingOverridden = 0;
        }
        if (&self->secondaryHistory) if (&self->secondaryHistory.samples)
            ReleaseSampleStorage(&self->secondaryHistory.samples);
        if (&self->primaryHistory)
            ReleaseSampleStorage(&self->primaryHistory.samples);
        if (flag > 0) __dl__FPv(self);
    }
    return self;
}

int InputManager_GetSelectedPlayer(InputManagerStorage*);
void gtl_CScreenPointer_GetSamplingParameter(int, int, void*);
void gtl_CInputExDeviceBase_GetDeviceResult(int, int*);
void gtl_CScreenPointer_GetSample(int, int, void*);

// Raw 36-byte input record; acceleration occupies the first three float words.
struct PadMotionSample { int accelerationBits[3]; int deviceWords[4]; float deviceFloat; int deviceWord; };
struct MotionAxes { float x; float y; };
extern const float kFloat_0_UiAnimation;
extern const float kMotionHistorySamplingParameter_0_1; // 0.1: second parameter of the gesture sampling override.
extern const float kMotionImpulseMagnitudeThreshold;
extern const float kMotionAccumulatedMagnitudeMaximum;
extern const float kFloat_50_U80267C20;
extern const float kFloat_neg50_SwingInput;
extern const float kMotionHorizontalNegativeThreshold; // -0.2: negative horizontal direction threshold.
extern const float kFloat_0_2_SwingSamples;
extern const float kMotionVerticalReversalThreshold; // -0.5: latest-sample vertical reversal threshold.
extern const int kMotionHorizontalReversalLookback; // 2: maximum horizontal reversal lookback.

float Vec3_Length(PadMotionSample*);

struct PadSampleHistory {
    char p0[8];
    int writeIndex;
    int lastValidIndex;
    PadMotionSample* samples;
    char p1[0x20];
    float horizontalImpulse;
    float totalMagnitude;
    int playerIndex;
    int savedSamplingParameters;
    int savedSamplingParameter2;
    int samplingOverridden;
};

// Wrap an unsigned index into the 120-slot history; return a borrowed record or null.
// lastValidIndex is the highest written ring slot, not a sample count. A negative
// writeIndex marks empty history. Before the first wrap, callers must not request
// more past records than have been written; after wrap all 120 slots are valid.
// The gesture routine relies on that invariant and does not check this result.
// This helper does not recognize the -1 empty sentinel after its unsigned cast;
// the gesture mode's history-length guard must exclude empty-history reads.
static inline PadMotionSample* SampleAt(PadSampleHistory* self, unsigned int i) {
    unsigned int k = i % kSampleHistoryCapacity;
    if (k > *(volatile unsigned int*)&self->lastValidIndex) return 0;
    return &self->samples[k];
}

// Detect a gesture and return 1 on recognition, otherwise 0; mode is kGesture*.
// History consumption by mode (lookbacks are sample counts, not milliseconds):
// impulse: require a quiet newest sample, sum up to 30 prior slots, keep history;
// horizontal: inspect at most two slots, reseed slot 0 with the first directional
// sample only after both signs were seen; unsuccessful attempts keep history;
// vertical: require newest Y below -0.5, scan older Y for >0.2, then empty history
// even if none passed. Early threshold/short-history exits keep history.
// Impulse fields are raw input sums; physical acceleration units are unresolved.
int PartyItemMotion_DetectGesture(PadSampleHistory* self, int mode) {
    int sampleLookbackLimit = self->lastValidIndex; // Exclusive lookback bound; do not add one to this index.
    int directionsSeen = 0;
    int detected = 0;
    do {
        // Original 0x8026812C: the three modes retain their existing body order.
        switch (mode) {
        case kGestureImpulse:
            if (sampleLookbackLimit < 1) break;
            if (Vec3_Length(SampleAt(self, self->writeIndex)) > kMotionImpulseMagnitudeThreshold) break;
            self->horizontalImpulse = kFloat_0_UiAnimation;
            self->totalMagnitude = kFloat_0_UiAnimation;
            if (sampleLookbackLimit > kImpulseWindowSamples) sampleLookbackLimit = kImpulseWindowSamples;
            for (int sampleOffset = 0; sampleOffset < sampleLookbackLimit; sampleOffset++) {
                PadMotionSample* sample = SampleAt(self, self->writeIndex - sampleOffset);
                float magnitude = Vec3_Length(sample);
                if (magnitude <= kMotionImpulseMagnitudeThreshold) continue;
                detected = 1;
                self->totalMagnitude += magnitude;
                self->horizontalImpulse += ((MotionAxes*)sample)->x;
            }
            if (self->totalMagnitude > kMotionAccumulatedMagnitudeMaximum) self->totalMagnitude = kMotionAccumulatedMagnitudeMaximum;
            if (self->horizontalImpulse > kFloat_50_U80267C20) self->horizontalImpulse = kFloat_50_U80267C20;
            if (self->horizontalImpulse < kFloat_neg50_SwingInput) self->horizontalImpulse = kFloat_neg50_SwingInput;
            break;
        case kGestureHorizontalReversal: {
            int windowSamples = *(sampleLookbackLimit > 2 ? &kMotionHorizontalReversalLookback : &sampleLookbackLimit);
            PadMotionSample firstDirectionalSample;
            for (int sampleOffset = 0; sampleOffset < windowSamples; sampleOffset++) {
                PadMotionSample* sample = SampleAt(self, self->writeIndex - sampleOffset);
                int directionBits = 0;
                if (((MotionAxes*)sample)->x <= kMotionHorizontalNegativeThreshold) directionBits = kDirectionNegative;
                if (((MotionAxes*)sample)->x >= kFloat_0_2_SwingSamples) directionBits = kDirectionPositive;
                if (!directionBits) continue;
                if (!directionsSeen) firstDirectionalSample = *sample;
                directionsSeen |= directionBits;
                if (directionsSeen != kDirectionBoth) continue;
                self->writeIndex = -1;
                self->lastValidIndex = -1;
                if (self->writeIndex < 0) self->writeIndex = 0;
                else self->writeIndex = (self->writeIndex + 1) % kSampleHistoryCapacity;
                if (self->writeIndex > self->lastValidIndex) self->lastValidIndex = self->writeIndex;
                self->samples[self->writeIndex] = firstDirectionalSample;
                detected = 1;
                break;
            }
            break;
        }
        case kGestureVerticalReversal: {
            if (sampleLookbackLimit <= 1) break;
            if (((MotionAxes*)SampleAt(self, self->writeIndex))->y >= kMotionVerticalReversalThreshold) break;
            int positiveRun = 0;
            for (int sampleOffset = 1; sampleOffset < sampleLookbackLimit; sampleOffset++) {
                PadMotionSample* sample = SampleAt(self, *(volatile unsigned int*)&self->writeIndex - sampleOffset);
                if (((MotionAxes*)sample)->y > kFloat_0_2_SwingSamples) {
                    positiveRun++;
                    if (positiveRun >= 1) {
                        detected = 1;
                        self->writeIndex = -1;
                        self->lastValidIndex = -1;
                        break;
                    }
                } else {
                    positiveRun = 0;
                }
            }
            self->writeIndex = -1;
            self->lastValidIndex = -1;
            break;
        }
        }
    } while (0);
    return detected;
}

// Append one input record and return 1, including when no pad is selected.
// Requires allocated storage for 120 records. With a pad but zero reported
// samples, the existing code still copies the uninitialized local record; no
// validity guarantee is added here. Gesture users must arrange valid input.
// First use retains the pad's two sampling parameters until CAccState_dtor.
int PadSampleHistory_RecordFrame(PadSampleHistory* self) {
    Holder restorePlayer(InputManager_GetSelectedPlayer(GetInputManager()));
    InputManagerStorage* manager = GetInputManager();
    InputManager_SetSelectedPlayer(manager, self->playerIndex);
    int pad = InputManager_GetSelectedPad(manager);
    if (!pad) return 1;
    if (!self->samplingOverridden) {
        gtl_CScreenPointer_GetSamplingParameter(pad, 0, &self->savedSamplingParameters);
        float parameters[2];
        parameters[0] = kFloat_0_UiAnimation;
        parameters[1] = kMotionHistorySamplingParameter_0_1;
        gtl_CScreenPointer_SetSamplingParameter(pad, 0, parameters);
        self->samplingOverridden = 1;
    }
    int sampleCount = 0;
    gtl_CInputExDeviceBase_GetDeviceResult(pad, &sampleCount);
    PadMotionSample sample;
// With no new sample the original copies its stale stack record, i.e. the previous call's sample.
    if (sampleCount > 0) gtl_CScreenPointer_GetSample(pad, 0, &sample);
    if (self->writeIndex < 0) self->writeIndex = 0;
    else self->writeIndex = (self->writeIndex + 1) % kSampleHistoryCapacity;
    if (self->writeIndex > self->lastValidIndex) self->lastValidIndex = self->writeIndex;
    PadMotionSample* destination = &self->samples[self->writeIndex];
    *destination = sample;
    return 1;
}

} // extern "C"
