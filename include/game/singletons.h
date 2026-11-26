// Singleton instance pointers: the game keeps each one as a function-local static of an inline accessor
// (`inline T* Get() { static T* s; if (!s) { ... } return s; }`); mwcc then loads the pointer before the
// prologue saves r31, which an extern pointer cannot do.
// The static of Sing_<addr>() is the symbol "@LOCAL@Sing_<addr>__Fv@s" (config/R2PE9B/symbols.txt names the
// The accessors must always expand (a unit built with -inline noauto or a large caller would call them).
#ifndef GAME_SINGLETONS_H
#define GAME_SINGLETONS_H
#pragma push
#pragma always_inline on

inline void*& GetInputManagerSlot() {
    static void* s;
    return s;
} // lbl_805DC5C8, guard s_inputManagerInitialized
inline void*& GetSaveDataSlot() {
    static void* s;
    return s;
} // lbl_805DC5E0, guard s_saveDataInitialized
inline void*& Sing_805DC5F0() {
    static void* s;
    return s;
} // lbl_805DC5F0, guard s_bpCtrlInitialized  // role: BpCtrl (name kept: the matching link places its local static by this name)
inline void*& Sing_805DC5F8() {
    static void* s;
    return s;
} // lbl_805DC5F8, guard s_bpCtrlHandleInitialized  // role: BpCtrlHandle (name kept: the matching link places its local static by this name)
inline void*& Sing_805DC608() {
    static void* s;
    return s;
} // lbl_805DC608, guard s_fadeOverlayInitialized  // role: FadeOverlay (name kept: the matching link places its local static by this name)
inline void*& Sing_805DC610() {
    static void* s;
    return s;
} // lbl_805DC610, guard s_nowReloadingInitialized
inline void*& Sing_805DC620() {
    static void* s;
    return s;
} // lbl_805DC620, guard s_resourceTableInitialized
inline void*& Sing_805DC630() {
    static void* s;
    return s;
} // lbl_805DC630, guard s_sharedTexturePointerListInitialized
inline void*& Sing_805DC648() {
    static void* s;
    return s;
} // lbl_805DC648, guard s_renderContextInitialized  // role: RenderContext (name kept: the matching link places its local static by this name)
inline void*& Sing_805DC650() {
    static void* s;
    return s;
} // lbl_805DC650, guard s_pointerPairListInitialized  // role: MessageDrawPairList (name kept: the matching link places its local static by this name)
inline void*& Sing_805DC660() {
    static void* s;
    return s;
} // lbl_805DC660, guard s_effectFadeInitialized
inline void*& GetConteModelSlotsSlot() {
    static void* s;
    return s;
} // lbl_805DC668, guard s_courseSlotTableInitialized
inline void*& Sing_805DC670() {
    static void* s;
    return s;
} // lbl_805DC670, guard s_bpPlayerInitialized  // role: BpPlayer (name kept: the matching link places its local static by this name)
inline void*& Sing_805DC678() {
    static void* s;
    return s;
} // lbl_805DC678, guard s_pangyaEnemyDataInitialized
inline void*& Sing_805DC680() {
    static void* s;
    return s;
} // lbl_805DC680, guard s_sharedFileListInitialized
inline void*& Singleton_TourHintWindow() {
    static void* s;
    return s;
} // lbl_805DC688, guard s_tourHintWindowInitialized
inline void*& Sing_805DC690() {
    static void* s;
    return s;
} // lbl_805DC690, guard s_tourVsWindowInitialized
inline void*& Sing_805DC7B0() {
    static void* s;
    return s;
} // lbl_805DC7B0, guard s_renderPacketInitialized
inline void*& Sing_805DC820() {
    static void* s;
    return s;
} // lbl_805DC820, guard s_homeMenuInitialized  // role: HomeButtonMenu (name kept: the matching link places its local static by this name)
inline void*& Sing_805DC830() {
    static void* s;
    return s;
} // lbl_805DC830, guard s_pointerVectorInitialized  // role: FontNumDrawRegistry (name kept: the matching link places its local static by this name)
inline void*& Sing_805DC930() {
    static void* s;
    return s;
} // lbl_805DC930, guard s_refObjectListInitialized
inline void*& Sing_805DC948() {
    static void* s;
    return s;
} // lbl_805DC948, guard s_systemResetStateInitialized  // role: SystemResetState (name kept: the matching link places its local static by this name)
inline void*& Sing_805DC950() {
    static void* s;
    return s;
} // lbl_805DC950, guard s_movieControllerInitialized
inline void*& Sing_805DC958() {
    static void* s;
    return s;
} // lbl_805DC958, guard s_itemRequestStateInitialized
inline void*& Sing_805DC960() {
    static void* s;
    return s;
} // lbl_805DC960, guard s_videoDimmingStateInitialized
inline void*& Sing_805DCB20() {
    static void* s;
    return s;
} // lbl_805DCB20, guard s_checkDriveStatusInitialized
inline void*& Sing_805DD688() {
    static void* s;
    return s;
} // lbl_805DD688, guard s_gameCubePadDriverRegistered
inline void*& Sing_805DD690() {
    static void* s;
    return s;
} // lbl_805DD690, guard s_freeStylePadDriverRegistered
inline void*& Sing_805DD698() {
    static void* s;
    return s;
} // lbl_805DD698, guard s_classicPadDriverRegistered
inline void*& Sing_805DD6A0() {
    static void* s;
    return s;
} // lbl_805DD6A0, guard s_pointingDriverRegistered

#pragma pop
#endif
