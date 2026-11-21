#include <decomp/static_init.h>
// RTTI names share the golf-player string pool; retain its original storage symbol.
enum { kGolfPlayerTypeNameOffset = 0x6C4 };

// Translation unit 0x8014B0F4-0x8014B210 (22 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

// Static RTTI descriptor: (type-name pointer, actor-base descriptor pointer).
extern char s_golfPlayerTypeInfo;
extern char s_actorTypeInfo;
extern const char golfPlayerStrings[];
// Process-lifetime strings; registration borrows their storage.
extern char golfPlayerFactoryName[];
extern char golfPlayerTaskName[];
// Affine matrices used by the puppet/bone transforms in golf_player_actor.cpp.
extern char golfPlayerBasisMatrix[];
extern char golfPlayerInverseBasisMatrix[];
// Full turn in radians, copied to the module startup cache below.
extern const float kFloat_6_2831855;
extern float s_golfPlayerFullTurnRadians;
extern const float kFloat_0_GolfPlayer;
extern const float kFloat_1_GolfPlayer;
// Legacy shared name; this use supplies -1 to a fixed basis, not a slope calculation.
extern const float kGroundSlopeSign_GolfPlayer;

void IntPair_Set(void*, void*, void*);
void* ActorFactoryRegistry_GetInstance();
void StringMap_SetByName(void*, void*, const char*);
void CGolfPlayer_Create();
void* TaskRegistry_GetInstance();
void TaskRegistry_AppendFactory(void*, const char*);
void Matrix_SetBasisElements(void*, float, float, float, float, float, float, float, float, float);

// Reserved empty hook; this implementation performs no work.
void CCreature_vfunc_25() {}
// Default virtual hook with no work in this implementation.
void CDisposable_vfunc_24() {}
// Default virtual hook with no work in this implementation.
void CDisposable_vfunc_23() {}
// Default virtual hook with no work in this implementation.
void CDisposable_vfunc_22() {}
// Default virtual hook with no work in this implementation.
void CDisposable_vfunc_21() {}
// Default virtual hook with no work in this implementation.
void CDisposable_vfunc_20() {}
// Default virtual hook with no work in this implementation.
void CGolfPlayer_vfunc_31() {}
// Default virtual hook with no work in this implementation.
void CGolfPlayer_vfunc_25() {}
// Default virtual hook with no work in this implementation.
void CDisposable_vfunc_10() {}
// Default virtual hook with no work in this implementation.
void CDisposable_vfunc_18() {}
// Default virtual hook with no work in this implementation.
void CDisposable_vfunc_17() {}
// Default virtual hook with no work in this implementation.
void CDisposable_vfunc_16() {}
// Default virtual hook with no work in this implementation.
void CDisposable_vfunc_15() {}
// Default virtual hook with no work in this implementation.
void CDisposable_vfunc_14() {}
// Default virtual hook with no work in this implementation.
void CDisposable_vfunc_11() {}
// Reserved empty hook; this implementation performs no work.
void CDisposable_vfunc_9() {}
// Default virtual hook with no work in this implementation.
void CDisposable_vfunc_8() {}
// Default virtual hook with no work in this implementation.
void CDisposable_vfunc_7() {}
// Default virtual hook with no work in this implementation.
void CDisposable_vfunc_6() {}
// Reserved empty hook; this implementation performs no work.
void CCreature_vfunc_4() {}
// Return the shared runtime type descriptor for golf-player actors.
// Borrowed process-lifetime descriptor; caller must not free it.
void* CGolfPlayer_GetRTTI() { return &s_golfPlayerTypeInfo; }

// Register the golf-player RTTI/factory and initialize the two fixed coordinate-basis matrices.
void CGolfPlayer_RegisterType() {
    // Publish static RTTI, actor creation callback and task name before use by registries.
    s_golfPlayerFullTurnRadians = kFloat_6_2831855;
    IntPair_Set(&s_golfPlayerTypeInfo, (char*)&golfPlayerStrings + kGolfPlayerTypeNameOffset, &s_actorTypeInfo);
    StringMap_SetByName(ActorFactoryRegistry_GetInstance(), (void*)CGolfPlayer_Create, golfPlayerFactoryName);
    TaskRegistry_AppendFactory(TaskRegistry_GetInstance(), golfPlayerTaskName);
    // Stored basis rows are (0,-1,0), (1,0,0), (0,0,1); translation is zero.
    // The second matrix is its inverse/transpose. Which game space each side
    // represents is unresolved; preserve the paired use in the puppet transforms.
    float zero = kFloat_0_GolfPlayer;
    float one = kFloat_1_GolfPlayer;
    float negativeOne = kGroundSlopeSign_GolfPlayer;
    Matrix_SetBasisElements(&golfPlayerBasisMatrix, zero, negativeOne, zero, one, zero, zero, zero, zero, one);
    float zeroForInverse = kFloat_0_GolfPlayer;
    float oneForInverse = kFloat_1_GolfPlayer;
    float negativeOneForInverse = kGroundSlopeSign_GolfPlayer;
    Matrix_SetBasisElements(&golfPlayerInverseBasisMatrix, zeroForInverse, oneForInverse, zeroForInverse, negativeOneForInverse, zeroForInverse, zeroForInverse, zeroForInverse, zeroForInverse, oneForInverse);
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU8046212C", _ctors_u8014B0F4_0, CGolfPlayer_RegisterType);
#pragma pop
