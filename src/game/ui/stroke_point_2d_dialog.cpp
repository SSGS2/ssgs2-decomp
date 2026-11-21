typedef unsigned long new_size_t;
// Byte offsets preserve the pooled-string storage and original relocation addends.
enum StrokePointSpecialShotStringsOffset {
    kStrokePointSpecialShotStrings_close_position_dialog = 0xB, // close_position_dialog
    kStrokePointSpecialShotStrings_select_special_shot = 0x76, // select_special_shot
    kStrokePointSpecialShotStrings_ui_icon_click = 0x8A, // ui_icon_click
    kStrokePointSpecialShotStrings_ui_error = 0x98, // ui_error
    kStrokePointSpecialShotStrings_range_error = 0x21, // bounds-error message
};

// UI values are logical actions and layout IDs, not Wii Remote hardware buttons.
enum StrokeDialogState { kStrokeClosed = 0, kStrokeOpening = 1, kStrokeOpen = 2, kStrokeClosing = 3 };
enum StrokeSpinMask { kSpinTop = 1, kSpinBottom = 2, kSpinLeft = 4, kSpinRight = 8 };
enum StrokeSpecialShotMask { kShotTomahawk = 0x10, kShotCobra = 0x20, kShotSpike = 0x40 };
enum StrokeTiming { kStrokeAnimationFrames = 10, kPowerClickFrames = 3, kSpecialFadeStep = 0x40,
    kPowerSegment = 0x21, kTwoPowerSegments = 0x42, kFullPowerGauge = 0x63 };
enum StrokeElement { kTomahawkButton = 0x2D, kSpikeButton = 0x2F, kCobraButton = 0x31,
    kPowerSegmentOne = 0x39, kPowerSegmentTwo = 0x3A, kPowerSegmentThree = 0x3B, kPowerRemainder = 0x3C,
    kSpinLeftButton = 0x3D, kSpinBottomButton = 0x3F, kSpinRightButton = 0x41, kSpinTopButton = 0x43,
    kPowerIconOne = 0x45, kPowerIconTwo = 0x46, kCharacterIcon = 0x47, kPowerButton = 0x48,
    kConfirmButton = 0x49, kStrikeMarker = 0x4C, kStrokeElementCount = 0x4D };
enum StrokeAlpha { kTransparent = 0, kOpaque = 0xFF, kPackedAlphaBias = 0x100 };
enum StrokeCompanionElement { kTomahawkCompanion = 0x2E, kSpikeCompanion = 0x30,
    kCobraCompanion = 0x32, kSpinLeftCompanion = 0x3E, kSpinBottomCompanion = 0x40,
    kSpinRightCompanion = 0x42, kSpinTopCompanion = 0x44 };
enum StrokeValueState { kStrokeValueUnavailable = 0, kStrokeValueAvailable = 2,
    kStrokeValueSelected = 3 };
enum StrokeFeedbackState { kStrokeFeedbackIdle = 1, kStrokeFeedbackSelected = 3 };
enum { kStrokePlayerFlagSelector = 5, kSpecialColorFirstElement = 0x24,
    kSpecialColorLastElement = 0x32 };
// ID pairs: Tomahawk 2D/2E, Spike 2F/30, Cobra 31/32; Left 3D/3E,
// Bottom 3F/40, Right 41/42, Top 43/44. Each second ID receives the same
// value/feedback/player-flag update; its precise visual asset is unresolved.
// Value states here: 0 unavailable, 2 available, 3 selected/click feedback.
// Feedback states: 1 normal, 3 selected. Player flag 5 gates pointer interaction;
// its boolean value is separate from both state domains. Do not interchange them.

// Translation unit 0x802E72B0-0x802E8A04 (10 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

// Placement construction uses caller-provided storage without allocating it.
inline void* operator new(new_size_t, void* p) { return p; }

extern "C" {

struct StrokePointAnimationState {
    char baseStorage[0xAC];
    int animationFramesRemaining;
    int dialogState;
    float opacity;
};

struct StrokePointLayout {
    char baseStorage[0x5C];
    float markerOriginX;
    float markerOriginY;
    float strikeX;
    float strikeY;
    float committedStrikeX;
    float committedStrikeY;
    float horizontalSpinThreshold;
    char reserved078[0x4];
    int pointerSelectionDisabled;
    int powerSelectionAllowed;
    int spinSelectionAllowed;
    int specialShotSelectionAllowed;
    int powerGauge;
    int spinMask;
    int previousSpinMask;
    int confirmPressed;
    int powerCount;
    int previousPowerCount;
    int specialShotMask;
    int previousSpecialShotMask;
    int animationFramesRemaining;
    int dialogState;
    float opacity;
    int visible;
    int resourcesReady;
    int controlsDirty;
    int specialShotOpacity;
    int powerClickFramesRemaining;
    int powerClickFeedback;
    float gaugeOriginX;
    float gaugeSegmentWidth;
    int message;
    int characterIndex;
    void** selectionVtable;
};

extern const float kFloat_1_805E0B38;
extern const float kFloat_1_5_StrokePointMarkerScale; // kFloat_1_5_StrokePointMarkerScale
extern const float kFloat_neg1_StrokePointVerticalLimit; // kFloat_neg1_StrokePointVerticalLimit
extern const float kFloat_0_StrokePointControls; // kFloat_0_StrokePointControls
extern const float kFloat_10_StrokePointOpenFrames; // kFloat_10_StrokePointOpenFrames
extern const float kFloat_5_StrokePointCloseFadeFrames; // kFloat_5_StrokePointCloseFadeFrames
extern const float kFloat_0_5_Menu802E72B0;
extern const float kFloat_200_StrokePointSound;
extern const double kSignedIntToDoubleBias_StrokePointControls; // kSignedIntToDoubleBias_StrokePointControls
extern const float kFloat_33_StrokePointGauge; // kFloat_33_StrokePointGauge
extern const float kFloat_66_StrokePointGauge; // kFloat_66_StrokePointGauge
extern int strokePointCharacterIconRows[]; // strokePointCharacterIconRows
extern int g_soundManager;
extern const char strokePointSpecialShotStrings[];
extern void* s_sharedCharDeleterVtable[];
extern void* __vt__Q23std11logic_error[];
extern void* __vt__Q23std12out_of_range[];
extern void* __nw__FUl(unsigned int size);
extern void __dl__FPv(void*);
extern void __dla__FPv(void*);
extern void* __nwa__FUl(unsigned int size);
extern "C" void __dt__Q23std9exceptionFv(void*);
extern void __dt__Q33std6detail16char_array_guardFv(void*, int);
extern void __dt__Q23std12out_of_rangeFv();
extern unsigned int strlen(const char*);
extern char* strcpy(char*, const char*);
extern void __throw(const char*, void*, void*) __attribute__((noreturn));
#pragma does_not_return __throw
void gtl_Vec2_Copy(float* dst, float* src);
void Sound_PlayByName(int, const char*, int, int, int, int, float, float, float);
int Layout_SetElementEnabled(StrokePointLayout*, int, int);
void Layout_SetElementPlayerFlag(StrokePointLayout*, int, int, int);
void Layout_SetElementValueState(StrokePointLayout*, int, int);
void Layout_SetElementFeedbackState(StrokePointLayout*, int, int);
void Layout_SetElementPosition(StrokePointLayout*, int, float*);
void Layout_GetElementPosition(StrokePointLayout*, int, float*);
void Layout_GetElementScale(StrokePointLayout*, int, float*);
void Layout_SetElementScale(StrokePointLayout*, int, float*);
void Layout_SetElementColor(StrokePointLayout*, int, int);
StrokePointLayout* Layout_GetElementSprite(StrokePointLayout*, int);
void Sprite_SetTextureRegionSize2D(StrokePointLayout*, float*);
void Layout_CollectActivated(void*, int*, int*);
void appFreeUni__FPv(void*);

// Return the committed strike-point coordinates when both output pointers are valid.
int CStrokePoint2d_GetCommittedStrikePoint(StrokePointLayout* self, float* x, float* y) {
    if (!x || !y)
        return 0;
    *x = self->committedStrikeX;
    *y = self->committedStrikeY;
    return 1;
}

// Open the strike-point dialog, snapshot selections and reset transient button states.
int CStrokePoint2d_OpenAndSaveSelection(StrokePointLayout* self) {
    self->dialogState = kStrokeOpening;
    self->animationFramesRemaining = kStrokeAnimationFrames;
    self->opacity = kFloat_0_StrokePointControls; // kFloat_0_StrokePointControls
    self->controlsDirty = 1;
    self->confirmPressed = 0;
    self->powerClickFeedback = 0;
    self->powerClickFramesRemaining = 0;
    if (self->powerCount == 0)
        self->specialShotOpacity = 0;
    else
        self->specialShotOpacity = kOpaque;
    gtl_Vec2_Copy(&self->committedStrikeX, &self->strikeX);
    self->previousSpinMask = self->spinMask;
    self->previousSpecialShotMask = self->specialShotMask;
    self->previousPowerCount = self->powerCount;
    return 1;
}

// Advance open/close animation and fade the special-shot controls toward their target opacity.
int CStrokePoint2d_AdvanceDialogAnimation(StrokePointLayout* self) {
    if (self->animationFramesRemaining) {
        int count = self->animationFramesRemaining - 1;
        self->animationFramesRemaining = count;
        if (self->dialogState == kStrokeOpening) {
            self->opacity = (kStrokeAnimationFrames - count) / kFloat_10_StrokePointOpenFrames; // kFloat_10_StrokePointOpenFrames
            if (!count)
                self->dialogState = kStrokeOpen;
        } else if (self->dialogState == kStrokeClosing) {
            if (count < 5)
                self->opacity = count / kFloat_5_StrokePointCloseFadeFrames; // kFloat_5_StrokePointCloseFadeFrames
            if (self->animationFramesRemaining == 5)
                Sound_PlayByName(g_soundManager, strokePointSpecialShotStrings + kStrokePointSpecialShotStrings_close_position_dialog, 0,
                    0, 0, 0, kFloat_0_5_Menu802E72B0, kFloat_200_StrokePointSound, kFloat_1_805E0B38);
            if (!self->animationFramesRemaining) {
                self->dialogState = kStrokeClosed;
                self->controlsDirty = 1;
            }
        }
    }
    if (kFloat_0_StrokePointControls == self->opacity)
        self->visible = 0;
    else
        self->visible = 1;
    if (!self->powerCount) {
        if (self->specialShotOpacity > 0) {
            self->specialShotOpacity -= kSpecialFadeStep;
            if (self->specialShotOpacity < 0)
                self->specialShotOpacity = 0;
            self->controlsDirty = 1;
        }
    } else if (self->specialShotOpacity < kOpaque) {
        self->specialShotOpacity += kSpecialFadeStep;
        if (self->specialShotOpacity > kOpaque)
            self->specialShotOpacity = kOpaque;
        self->controlsDirty = 1;
    }
    return 1;
}

// Return whether the strike-point dialog is fully open.
int CStrokePoint2d_vfunc_26(StrokePointAnimationState* self) {
    return self->dialogState == kStrokeOpen;
}

// Begin the ten-frame closing animation with opacity reset to the shared initial value.
int CStrokePoint2d_StartClosingDialog(StrokePointAnimationState* self) {
    float value = kFloat_1_805E0B38;
    self->dialogState = kStrokeClosing;
    self->animationFramesRemaining = kStrokeAnimationFrames;
    self->opacity = value;
    return 1;
}

// Report stable state only while neither opening nor closing.
int CStrokePoint2d_IsDialogIdle(StrokePointAnimationState* self) {
    // 0x802E72B0 unit: the two excluded states share the final return.
    if (self->dialogState != kStrokeOpening && self->dialogState != kStrokeClosing)
        return 1;
    return 0;
}

// Refresh visible controls, selected action states and the character icon when dirty.
int CStrokePoint2d_RefreshDirtyControls(StrokePointLayout* self) {
    if (!self->resourcesReady)
        return 0;
    if (!self->controlsDirty)
        return 0;
    self->controlsDirty = 0;
    int enabled = 1;
    int enabled2 = 1;
    if (self->dialogState == kStrokeClosed) {
        enabled = 0;
        enabled2 = 0;
    }
    if (self->specialShotOpacity == 0)
        enabled2 = 0;
    Layout_SetElementEnabled(self, kSpinLeftButton, enabled);
    Layout_SetElementEnabled(self, kSpinBottomButton, enabled);
    Layout_SetElementEnabled(self, kSpinRightButton, enabled);
    Layout_SetElementEnabled(self, kSpinTopButton, enabled);
    Layout_SetElementEnabled(self, kPowerButton, enabled);
    Layout_SetElementEnabled(self, kConfirmButton, enabled);
    Layout_SetElementEnabled(self, kTomahawkButton, enabled2);
    Layout_SetElementEnabled(self, kSpikeButton, enabled2);
    Layout_SetElementEnabled(self, kCobraButton, enabled2);
    /* original 0x802E7EDC-0x802E7F00: lwz r12,0xe0(self); lwz r12,0x80/0x84(r12); mtctr; bctrl (vtable at +0xE0; ours loaded one word too many) */
    (*((void (**)(StrokePointLayout*))(self->selectionVtable + 32)))(self);
    (*((void (**)(StrokePointLayout*))(self->selectionVtable + 33)))(self);
    if (self->confirmPressed) {
        Layout_SetElementPlayerFlag(self, kConfirmButton, 0, kStrokePlayerFlagSelector);
        Layout_SetElementValueState(self, kConfirmButton, kStrokeValueAvailable);
    } else {
        Layout_SetElementPlayerFlag(self, kConfirmButton, 1, kStrokePlayerFlagSelector);
        Layout_SetElementValueState(self, kConfirmButton, kStrokeValueUnavailable);
    }
    if (!self->dialogState && self->resourcesReady) {
        for (int i = 0; i < kStrokeElementCount; ++i)
            Layout_SetElementPlayerFlag(self, i, 0, kStrokePlayerFlagSelector);
    }
    if (self->characterIndex < 0) {
        Layout_SetElementEnabled(self, kCharacterIcon, 0);
    } else {
        Layout_SetElementEnabled(self, kCharacterIcon, 1);
        int idx = 0;
        if (self->characterIndex < 8)
            idx = strokePointCharacterIconRows[self->characterIndex]; // strokePointCharacterIconRows
        StrokePointLayout* elem = Layout_GetElementSprite(self, kCharacterIcon);
        float pos[2] = { kFloat_0_StrokePointControls, (float)(idx * 0x18) };
        Sprite_SetTextureRegionSize2D(elem, pos);
    }
    return 1;
}

// Position the strike marker and update four directional spin-control indicators.
// Marker position = origin + 1.5*strike in layout coordinates. Spin eligibility
// depends on horizontal threshold or vertical +/-1 and spinSelectionAllowed.
// Ineligible directions clear a currently selected matching spin. These four
// branches deliberately reuse two compiler temporaries with opposite roles in
// the RIGHT branch; the names below state that exception. feedbackState carries
// forward if an ineligible branch does not assign it (existing behavior).
int CStrokePoint2d_UpdateStrikeMarkerAndSpinButtons(StrokePointLayout* self) {
    float pos[2];
    float k = kFloat_1_5_StrokePointMarkerScale; // kFloat_1_5_StrokePointMarkerScale
    pos[0] = self->markerOriginX + k * self->strikeX;
    pos[1] = self->markerOriginY + k * self->strikeY;
    Layout_SetElementPosition(self, kStrikeMarker, pos);

    int feedbackState = 1;
    int valueStateOrRightFlag;
    int playerFlagOrRightValue;
    // Right only: first temporary = player flag, second = visual value state.
    if (self->strikeX >= self->horizontalSpinThreshold && self->spinSelectionAllowed) {
        if (self->pointerSelectionDisabled) {
            valueStateOrRightFlag = 0;
            playerFlagOrRightValue = 2;
        } else {
            valueStateOrRightFlag = 1;
            if (self->spinMask == kSpinRight) {
                playerFlagOrRightValue = 3;
                feedbackState = 3;
            } else {
                playerFlagOrRightValue = 2;
                feedbackState = 1;
            }
        }
    } else {
        valueStateOrRightFlag = 0;
        playerFlagOrRightValue = 0;
        if (self->spinMask == kSpinRight)
            self->spinMask = 0;
    }
    Layout_SetElementPlayerFlag(self, kSpinRightButton, valueStateOrRightFlag, kStrokePlayerFlagSelector);
    Layout_SetElementPlayerFlag(self, kSpinRightCompanion, valueStateOrRightFlag, kStrokePlayerFlagSelector);
    Layout_SetElementValueState(self, kSpinRightButton, playerFlagOrRightValue);
    Layout_SetElementFeedbackState(self, kSpinRightButton, feedbackState);
    Layout_SetElementValueState(self, kSpinRightCompanion, playerFlagOrRightValue);
    Layout_SetElementFeedbackState(self, kSpinRightCompanion, feedbackState);

    // Remaining directions: first temporary = value state, second = player flag.
    if (self->strikeX <= -self->horizontalSpinThreshold && self->spinSelectionAllowed) {
        if (self->pointerSelectionDisabled) {
            valueStateOrRightFlag = 2;
            playerFlagOrRightValue = 0;
        } else {
            playerFlagOrRightValue = 1;
            if (self->spinMask == kSpinLeft) {
                valueStateOrRightFlag = 3;
                feedbackState = 3;
            } else {
                valueStateOrRightFlag = 2;
                feedbackState = 1;
            }
        }
    } else {
        playerFlagOrRightValue = 0;
        valueStateOrRightFlag = 0;
        if (self->spinMask == kSpinLeft)
            self->spinMask = 0;
    }
    Layout_SetElementPlayerFlag(self, kSpinLeftButton, playerFlagOrRightValue, kStrokePlayerFlagSelector);
    Layout_SetElementPlayerFlag(self, kSpinLeftCompanion, playerFlagOrRightValue, kStrokePlayerFlagSelector);
    Layout_SetElementValueState(self, kSpinLeftButton, valueStateOrRightFlag);
    Layout_SetElementFeedbackState(self, kSpinLeftButton, feedbackState);
    Layout_SetElementValueState(self, kSpinLeftCompanion, valueStateOrRightFlag);
    Layout_SetElementFeedbackState(self, kSpinLeftCompanion, feedbackState);

    if (self->strikeY <= kFloat_neg1_StrokePointVerticalLimit && self->spinSelectionAllowed) {
        if (self->pointerSelectionDisabled) {
            valueStateOrRightFlag = 2;
            playerFlagOrRightValue = 0;
        } else {
            playerFlagOrRightValue = 1;
            if (self->spinMask == kSpinTop) {
                valueStateOrRightFlag = 3;
                feedbackState = 3;
            } else {
                valueStateOrRightFlag = 2;
                feedbackState = 1;
            }
        }
    } else {
        playerFlagOrRightValue = 0;
        valueStateOrRightFlag = 0;
        if (self->spinMask == kSpinTop)
            self->spinMask = 0;
    }
    Layout_SetElementPlayerFlag(self, kSpinTopButton, playerFlagOrRightValue, kStrokePlayerFlagSelector);
    Layout_SetElementPlayerFlag(self, kSpinTopCompanion, playerFlagOrRightValue, kStrokePlayerFlagSelector);
    Layout_SetElementValueState(self, kSpinTopButton, valueStateOrRightFlag);
    Layout_SetElementFeedbackState(self, kSpinTopButton, feedbackState);
    Layout_SetElementValueState(self, kSpinTopCompanion, valueStateOrRightFlag);
    Layout_SetElementFeedbackState(self, kSpinTopCompanion, feedbackState);

    if (self->strikeY >= kFloat_1_805E0B38 && self->spinSelectionAllowed) {
        if (self->pointerSelectionDisabled) {
            valueStateOrRightFlag = 2;
            playerFlagOrRightValue = 0;
        } else {
            playerFlagOrRightValue = 1;
            if (self->spinMask == kSpinBottom) {
                valueStateOrRightFlag = 3;
                feedbackState = 3;
            } else {
                valueStateOrRightFlag = 2;
                feedbackState = 1;
            }
        }
    } else {
        playerFlagOrRightValue = 0;
        valueStateOrRightFlag = 0;
        if (self->spinMask == kSpinBottom)
            self->spinMask = 0;
    }
    Layout_SetElementPlayerFlag(self, kSpinBottomButton, playerFlagOrRightValue, kStrokePlayerFlagSelector);
    Layout_SetElementPlayerFlag(self, kSpinBottomCompanion, playerFlagOrRightValue, kStrokePlayerFlagSelector);
    Layout_SetElementValueState(self, kSpinBottomButton, valueStateOrRightFlag);
    Layout_SetElementFeedbackState(self, kSpinBottomButton, feedbackState);
    Layout_SetElementValueState(self, kSpinBottomCompanion, valueStateOrRightFlag);
    Layout_SetElementFeedbackState(self, kSpinBottomCompanion, feedbackState);
    return 1;
}

// Draw up to three power segments and refresh power/special-shot button states.
// Gauge domain is 0..99 in three 33-unit segments. Full sprites represent each
// complete segment; the remainder sprite uses gaugeSegmentWidth*remaining units
// and shifts right by 33 or 66 layout units. Values >=99 hide the remainder.
// powerCount is a separate 0/1/2 selection. Tomahawk needs any selected power;
// Cobra/Spike also need specialShotSelectionAllowed. pointerSelectionDisabled
// suppresses interaction-flag writes here, while visual state still updates.
int CStrokePoint2d_UpdatePowerAndSpecialShots(StrokePointLayout* self) {
    // Phase 1: power-button eligibility and its three-tick click feedback.
    if (self->powerGauge >= kPowerSegment && self->powerSelectionAllowed) {
        if (self->powerClickFeedback) {
            Layout_SetElementPlayerFlag(self, kPowerButton, 0, kStrokePlayerFlagSelector);
            Layout_SetElementValueState(self, kPowerButton, kStrokeValueSelected);
        } else {
            Layout_SetElementPlayerFlag(self, kPowerButton, 1, kStrokePlayerFlagSelector);
            Layout_SetElementValueState(self, kPowerButton, kStrokeValueAvailable);
        }
    } else {
        Layout_SetElementPlayerFlag(self, kPowerButton, 0, kStrokePlayerFlagSelector);
        Layout_SetElementValueState(self, kPowerButton, kStrokeValueUnavailable);
    }

    // Phase 2: selected power-count icons, independent of the gauge fill itself.
    switch (self->powerCount) {
    case 0:
        Layout_SetElementValueState(self, kPowerIconOne, kStrokeValueUnavailable);
        Layout_SetElementValueState(self, kPowerIconTwo, kStrokeValueUnavailable);
        break;
    case 1:
        Layout_SetElementValueState(self, kPowerIconOne, kStrokeValueAvailable);
        Layout_SetElementValueState(self, kPowerIconTwo, kStrokeValueAvailable);
        break;
    case 2:
        Layout_SetElementValueState(self, kPowerIconOne, kStrokeValueSelected);
        Layout_SetElementValueState(self, kPowerIconTwo, kStrokeValueSelected);
        break;
    }

    // Phase 3: full segments plus a horizontally scaled remainder sprite.
    float y[2];
    float x[2];
    Layout_GetElementPosition(self, kPowerRemainder, x);
    Layout_GetElementScale(self, kPowerRemainder, y);
    y[0] = self->gaugeSegmentWidth;
    x[0] = self->gaugeOriginX;
    if (self->powerGauge < kPowerSegment) {
        Layout_SetElementEnabled(self, kPowerSegmentOne, 0);
        Layout_SetElementEnabled(self, kPowerSegmentTwo, 0);
        Layout_SetElementEnabled(self, kPowerSegmentThree, 0);
        Layout_SetElementPosition(self, kPowerRemainder, x);
        y[0] = y[0] * (float)self->powerGauge;
        Layout_SetElementScale(self, kPowerRemainder, y);
        Layout_SetElementEnabled(self, kPowerRemainder, 1);
    } else if (self->powerGauge < kTwoPowerSegments) {
        Layout_SetElementEnabled(self, kPowerSegmentOne, 1);
        Layout_SetElementEnabled(self, kPowerSegmentTwo, 0);
        Layout_SetElementEnabled(self, kPowerSegmentThree, 0);
        x[0] += kFloat_33_StrokePointGauge; // kFloat_33_StrokePointGauge
        Layout_SetElementPosition(self, kPowerRemainder, x);
        y[0] = y[0] * (float)(self->powerGauge - kPowerSegment);
        Layout_SetElementScale(self, kPowerRemainder, y);
        Layout_SetElementEnabled(self, kPowerRemainder, 1);
    } else if (self->powerGauge < kFullPowerGauge) {
        Layout_SetElementEnabled(self, kPowerSegmentOne, 1);
        Layout_SetElementEnabled(self, kPowerSegmentTwo, 1);
        Layout_SetElementEnabled(self, kPowerSegmentThree, 0);
        x[0] += kFloat_66_StrokePointGauge; // kFloat_66_StrokePointGauge
        Layout_SetElementPosition(self, kPowerRemainder, x);
        y[0] = y[0] * (float)(self->powerGauge - kTwoPowerSegments);
        Layout_SetElementScale(self, kPowerRemainder, y);
        Layout_SetElementEnabled(self, kPowerRemainder, 1);
    } else {
        Layout_SetElementEnabled(self, kPowerSegmentOne, 1);
        Layout_SetElementEnabled(self, kPowerSegmentTwo, 1);
        Layout_SetElementEnabled(self, kPowerSegmentThree, 1);
        Layout_SetElementEnabled(self, kPowerRemainder, 0);
    }

    // Phase 4: derive all special-shot states before updating each paired element.
    int powerEnabled = 0;
    int specialEnabled = 0;
    int tomahawkState = 0;
    int tomahawkFeedback = 1;
    int cobraState = 0;
    int cobraFeedback = 1;
    int spikeState = 0;
    int spikeFeedback = 1;
    if (self->powerCount) {
        int mode = self->specialShotMask;
        powerEnabled = 1;
        if (mode == kShotTomahawk) {
            tomahawkState = 3;
            tomahawkFeedback = 3;
        } else {
            tomahawkState = 2;
        }
        if (self->specialShotSelectionAllowed) {
            specialEnabled = 1;
            if (mode == kShotCobra) {
                cobraState = 3;
                cobraFeedback = 3;
            } else {
                cobraState = 2;
            }
            if (mode == kShotSpike) {
                spikeState = 3;
                spikeFeedback = 3;
            } else {
                spikeState = 2;
            }
        }
    }
    if (!self->pointerSelectionDisabled) {
        Layout_SetElementPlayerFlag(self, kTomahawkButton, powerEnabled, kStrokePlayerFlagSelector);
        Layout_SetElementPlayerFlag(self, kTomahawkCompanion, powerEnabled, kStrokePlayerFlagSelector);
        Layout_SetElementPlayerFlag(self, kSpikeButton, specialEnabled, kStrokePlayerFlagSelector);
        Layout_SetElementPlayerFlag(self, kSpikeCompanion, specialEnabled, kStrokePlayerFlagSelector);
        Layout_SetElementPlayerFlag(self, kCobraButton, specialEnabled, kStrokePlayerFlagSelector);
        Layout_SetElementPlayerFlag(self, kCobraCompanion, specialEnabled, kStrokePlayerFlagSelector);
    }
    Layout_SetElementValueState(self, kTomahawkButton, tomahawkState); Layout_SetElementFeedbackState(self, kTomahawkButton, tomahawkFeedback);
    Layout_SetElementValueState(self, kTomahawkCompanion, tomahawkState); Layout_SetElementFeedbackState(self, kTomahawkCompanion, tomahawkFeedback);
    Layout_SetElementValueState(self, kCobraButton, cobraState); Layout_SetElementFeedbackState(self, kCobraButton, cobraFeedback);
    Layout_SetElementValueState(self, kCobraCompanion, cobraState); Layout_SetElementFeedbackState(self, kCobraCompanion, cobraFeedback);
    Layout_SetElementValueState(self, kSpikeButton, spikeState); Layout_SetElementFeedbackState(self, kSpikeButton, spikeFeedback);
    Layout_SetElementValueState(self, kSpikeCompanion, spikeState); Layout_SetElementFeedbackState(self, kSpikeCompanion, spikeFeedback);
    // Phase 5: the color API consumes this existing alpha-minus-256 encoding.
    int value = self->specialShotOpacity - kPackedAlphaBias;
    for (int id = kSpecialColorFirstElement; id <= kSpecialColorLastElement; ++id)
        Layout_SetElementColor(self, id, value);
    return 1;
}

struct RangeOwner28 {
    void* table;
    int a;
    int b;
    char* text;
    // Create the one-reference control block that owns an exception-message array.
    RangeOwner28(char* p) {
        a = 1;
        b = 1;
        table = s_sharedCharDeleterVtable;
        text = p;
    }
};

struct RangeGuard28 {
    char* text;
    char* allocator;
    // Free an exception-message array unless ownership was transferred to its control block.
    ~RangeGuard28() { if (text) __dla__FPv(text); }
};

namespace std {
struct exception {
    void* vtable;
};
struct logic_error : exception {
    char* message;
    void* control;
    // Copy the bounds-error message into shared-deleter-owned exception storage.
    logic_error(const char* s) {
        vtable = (void*)__vt__Q23std11logic_error;
        char allocator __attribute__((aligned(4))) = 0;
        unsigned int n = strlen(s) + 1;
        message = (char*)__nwa__FUl((n << 1) - n);
        {
            char* buffer = message;
            RangeGuard28 guard = { buffer, &allocator };
            control = new RangeOwner28(buffer);
            guard.text = 0;
        }
        strcpy(message, s);
    }
    ~logic_error();
};
struct out_of_range : logic_error {
    // Construct the bounds exception and install its final interface.
    out_of_range(const char* s) : logic_error(s) { vtable = (void*)__vt__Q23std12out_of_range; }
    ~out_of_range();
};
}

struct IntVector28 {
    int* data;
    unsigned int size;
    unsigned int capacity;

    // Return an activated element ID by reference, throwing if index is out of bounds.
    int& at(unsigned int index) {
        if (index >= size) {
            const char* message = strokePointSpecialShotStrings + kStrokePointSpecialShotStrings_range_error;
            throw std::out_of_range(message);
        }
        return data[index];
    }

    // Clear the activation count and free the vector's backing allocation.
    ~IntVector28() {
        if (data) {
            size -= size;
            appFreeUni__FPv(data);
        }
    }
};

struct VirtualBase28 { char pad[0xE0]; };
struct Virtual28 : VirtualBase28 {
    virtual void slot00(); virtual void slot01(); virtual void slot02();
    virtual void slot03(); virtual void slot04(); virtual void slot05();
    virtual void slot06(); virtual void slot07(); virtual void slot08();
    virtual void slot09(); virtual void slot10(); virtual void slot11();
    virtual void slot12(); virtual void slot13(); virtual void slot14();
    virtual void slot15(); virtual void slot16(); virtual void slot17();
    virtual void slot18(); virtual void slot19(); virtual void slot20();
    virtual void slot21(); virtual void slot22(); virtual void slot23();
    virtual void slot24(); virtual void slot25(); virtual void slot26();
    virtual void slot27(); virtual void slot28(); virtual void slot29();
    virtual void slot30(); virtual void slot31(); virtual void slot32();
    virtual int poll(int);
};

// Consume spin, power-count, special-shot and confirm clicks while the dialog is open.
// Return 0 unless fully open; otherwise consume the borrowed layout's activation
// list and return 1. Processing order is spin toggles -> power cycling -> special
// toggles -> click timer -> confirm, so simultaneous actions follow that order.
// Any spin click clears an existing spin (even a different direction); choosing
// a new direction requires a later activation. Special shots switch directly.
// Eligibility is enforced by the element interaction flags set by render/update;
// this handler does not repeat every spin/power/special eligibility test.
int CStrokePoint2d_ConsumeControlActivations(StrokePointLayout* self) {
    int oldSpinMask;
    int oldSpecialShotMask;
    if (self->dialogState != kStrokeOpen)
        return 0;

    oldSpinMask = self->spinMask;
    oldSpecialShotMask = self->specialShotMask;
    IntVector28 activated;
    activated.data = 0;
    activated.size = 0;
    activated.capacity = 0;
    Layout_CollectActivated(self, (int*)&activated, 0);

    if (!self->pointerSelectionDisabled) {
        // Phase 1: pointer-enabled spin actions toggle the selection as described above.
        for (unsigned int i = 0; i < activated.size; ++i) {
            if (self->spinMask == 0) {
                switch (activated.at(i)) {
                case kSpinLeftButton: self->spinMask = kSpinLeft; break;
                case kSpinBottomButton: self->spinMask = kSpinBottom; break;
                case kSpinRightButton: self->spinMask = kSpinRight; break;
                case kSpinTopButton: self->spinMask = kSpinTop; break;
                }
            } else {
                switch (activated.at(i)) {
                case kSpinLeftButton:
                case kSpinBottomButton:
                case kSpinRightButton:
                case kSpinTopButton:
                    self->spinMask = 0;
                    break;
                }
            }
        }
    }

    if (oldSpinMask != self->spinMask)
        self->controlsDirty = 1;

    // Phase 2: power cycles through 0/1/2 for gauge >=66, or 0/1 otherwise;
    // wrapping to zero clears the special shot. Every click starts three-tick feedback.
    for (unsigned int i = 0; i < activated.size; ++i) {
        switch (activated.at(i)) {
        case kPowerButton:
            ++self->powerCount;
            if (self->powerGauge >= kTwoPowerSegments) {
                if (self->powerCount > 2) {
                    self->powerCount = 0;
                    self->specialShotMask = 0;
                }
            } else if (self->powerCount > 1) {
                self->powerCount = 0;
                self->specialShotMask = 0;
            }
            self->powerClickFeedback = 1;
            self->powerClickFramesRemaining = kPowerClickFrames;
            self->controlsDirty = 1;
            break;
        }
    }

    if (!self->pointerSelectionDisabled) {
        for (unsigned int i = 0; i < activated.size; ++i) {
            switch (activated.at(i)) {
            case kCobraButton:
                if (self->specialShotMask != kShotCobra) {
                    Sound_PlayByName(g_soundManager, strokePointSpecialShotStrings + kStrokePointSpecialShotStrings_select_special_shot, 0,
                        0, 0, 0, kFloat_0_5_Menu802E72B0, kFloat_200_StrokePointSound, kFloat_1_805E0B38);
                    self->specialShotMask = kShotCobra;
                } else {
                    Sound_PlayByName(g_soundManager, strokePointSpecialShotStrings + kStrokePointSpecialShotStrings_ui_icon_click, 0,
                        0, 0, 0, kFloat_0_5_Menu802E72B0, kFloat_200_StrokePointSound, kFloat_1_805E0B38);
                    self->specialShotMask = 0;
                }
                break;
            case kSpikeButton:
                if (self->specialShotMask != kShotSpike) {
                    Sound_PlayByName(g_soundManager, strokePointSpecialShotStrings + kStrokePointSpecialShotStrings_select_special_shot, 0,
                        0, 0, 0, kFloat_0_5_Menu802E72B0, kFloat_200_StrokePointSound, kFloat_1_805E0B38);
                    self->specialShotMask = kShotSpike;
                } else {
                    Sound_PlayByName(g_soundManager, strokePointSpecialShotStrings + kStrokePointSpecialShotStrings_ui_icon_click, 0,
                        0, 0, 0, kFloat_0_5_Menu802E72B0, kFloat_200_StrokePointSound, kFloat_1_805E0B38);
                    self->specialShotMask = 0;
                }
                break;
            case kTomahawkButton:
                if (self->specialShotMask != kShotTomahawk) {
                    Sound_PlayByName(g_soundManager, strokePointSpecialShotStrings + kStrokePointSpecialShotStrings_select_special_shot, 0,
                        0, 0, 0, kFloat_0_5_Menu802E72B0, kFloat_200_StrokePointSound, kFloat_1_805E0B38);
                    self->specialShotMask = kShotTomahawk;
                } else {
                    Sound_PlayByName(g_soundManager, strokePointSpecialShotStrings + kStrokePointSpecialShotStrings_ui_icon_click, 0,
                        0, 0, 0, kFloat_0_5_Menu802E72B0, kFloat_200_StrokePointSound, kFloat_1_805E0B38);
                    self->specialShotMask = 0;
                }
                break;
            }
        }
    }

    // Phase 3 ends: mark the dialog dirty only when the special selection changed.
    if (oldSpecialShotMask != self->specialShotMask)
        self->controlsDirty = 1;

    // Phase 4: advance click feedback in handler ticks; newly set 3 decrements now.
    if (self->powerClickFramesRemaining > 0) {
        --self->powerClickFramesRemaining;
        if (!self->powerClickFramesRemaining) {
            self->powerClickFeedback = 0;
            self->controlsDirty = 1;
        }
    }

    for (unsigned int i = 0; i < activated.size; ++i) {
        if (activated.at(i) == kConfirmButton) {
            self->confirmPressed = 1;
            self->controlsDirty = 1;
        }
    }

    if (((Virtual28*)self)->poll(kCobraButton) ||
        ((Virtual28*)self)->poll(kSpikeButton) ||
        ((Virtual28*)self)->poll(kTomahawkButton))
        Sound_PlayByName(g_soundManager, strokePointSpecialShotStrings + kStrokePointSpecialShotStrings_ui_error, 0,
            0, 0, 0, kFloat_0_5_Menu802E72B0, kFloat_200_StrokePointSound, kFloat_1_805E0B38);

    return 1;
}

} // extern "C"
