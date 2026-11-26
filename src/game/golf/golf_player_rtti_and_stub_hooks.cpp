// Golf player type information: the CGolfPlayer runtime type descriptor and its registration,
// which also sets up two fixed coordinate-basis matrices.

#include <decomp/static_init.h>
#include "classes/CCreature.h"
#include "classes/CDisposable.h"
#include "classes/CGolfPlayer.h"
// RTTI names share the golf-player string pool; retain its original storage symbol.
enum {
    kGolfPlayerTypeNameOffset = 0x6C4
};

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

void* IntPair_Set(void*, void*, void*);
void* ActorFactoryRegistry_GetInstance();
void StringMap_SetByName(void*, void*, const char*);
void CGolfPlayer_Create();
void* TaskRegistry_GetInstance();
void TaskRegistry_AppendFactory(void*, const char*);
void Matrix_SetBasisElements(void*, float, float, float, float, float, float, float, float, float);

// Reserved empty hook; this implementation performs no work.
void CCreature::vfunc_25() {}
// Default virtual hook with no work in this implementation.
void CDisposable::vfunc_24() {}
// Default virtual hook with no work in this implementation.
void CDisposable::vfunc_23() {}
// Default virtual hook with no work in this implementation.
void CDisposable::vfunc_22() {}
// Default virtual hook with no work in this implementation.
void CDisposable::vfunc_21() {}
// Default virtual hook with no work in this implementation.
void CDisposable::vfunc_20() {}
// Default virtual hook with no work in this implementation.
void CGolfPlayer::vfunc_31() {}
// Default virtual hook with no work in this implementation.
void CGolfPlayer::vfunc_25() {}
// Default virtual hook with no work in this implementation.
void CDisposable::vfunc_10() {}
// Default virtual hook with no work in this implementation.
void CDisposable::vfunc_18() {}
// Default virtual hook with no work in this implementation.
void CDisposable::vfunc_17() {}
// Default virtual hook with no work in this implementation.
void CDisposable::vfunc_16() {}
// Default virtual hook with no work in this implementation.
void CDisposable::vfunc_15() {}
// Default virtual hook with no work in this implementation.
void CDisposable::vfunc_14() {}
// Default virtual hook with no work in this implementation.
void CDisposable::vfunc_11() {}
// Reserved empty hook; this implementation performs no work.
void CDisposable::vfunc_9() {}
// Default virtual hook with no work in this implementation.
void CDisposable::vfunc_8() {}
// Default virtual hook with no work in this implementation.
void CDisposable::vfunc_7() {}
// Default virtual hook with no work in this implementation.
void CDisposable::vfunc_6() {}
// Reserved empty hook; this implementation performs no work.
void CCreature::vfunc_4() {}
// Return the shared runtime type descriptor for golf-player actors.
// Borrowed process-lifetime descriptor; caller must not free it.
void* CGolfPlayer::GetRTTI() {
    return &s_golfPlayerTypeInfo;
}

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
    Matrix_SetBasisElements(&golfPlayerInverseBasisMatrix, zeroForInverse, oneForInverse, zeroForInverse,
                            negativeOneForInverse, zeroForInverse, zeroForInverse, zeroForInverse, zeroForInverse,
                            oneForInverse);
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU8046212C", _ctors_u8014B0F4_0, CGolfPlayer_RegisterType);
#pragma pop
