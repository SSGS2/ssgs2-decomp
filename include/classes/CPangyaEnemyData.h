// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// The original vtable (__vt__16CPangyaEnemyData) and typeinfo stay data symbols.
#ifndef CLASSES_CPANGYAENEMYDATA_H
#define CLASSES_CPANGYAENEMYDATA_H

// RTTI base class: CEcbCtl<STYPE_CENEMYDATA> (not declared here)
class CPangyaEnemyData {
public:
    void* dataBuffer; // 0x0: CPangyaEnemyData_LoadSharedFile writes the loaded data buffer here
    void* enemyVtable; // 0x4: CPangyaEnemyData_Init/dtor install __vt__16CPangyaEnemyData here
    void* sharedFile; // 0x8: CPangyaEnemyData_LoadSharedFile stores SharedFile_Create result; destructor destroys handle

    // Slots 1-44 all report a constant "valid" answer for this data object.

    // vtable slots (current symbol; implement as a method with its real signature):
    // slot 0 (destructor): CPangyaEnemyData_dtor (medium)  ~CPangyaEnemyData();
    // slot 1: vfunc_1__16CPangyaEnemyDataFv (medium)  void vfunc_1();
    // slot 2: vfunc_2__16CPangyaEnemyDataFv (medium)  void vfunc_2();
    // slot 3: vfunc_3__16CPangyaEnemyDataFv (medium)  void vfunc_3();
    // slot 4: vfunc_4__16CPangyaEnemyDataFv (medium)  void vfunc_4();
    // slot 5: vfunc_5__16CPangyaEnemyDataFv (medium)  void vfunc_5();
    // slot 6: vfunc_6__16CPangyaEnemyDataFv (medium)  void vfunc_6();
    // slot 7: vfunc_7__16CPangyaEnemyDataFv (medium)  void vfunc_7();
    // slot 8: vfunc_8__16CPangyaEnemyDataFv (medium)  void vfunc_8();
    // slot 9: vfunc_9__16CPangyaEnemyDataFv (medium)  void vfunc_9();
    // slot 10: vfunc_10__16CPangyaEnemyDataFv (medium)  void vfunc_10();
    // slot 11: vfunc_11__16CPangyaEnemyDataFv (medium)  void vfunc_11();
    // slot 12: vfunc_12__16CPangyaEnemyDataFv (medium)  void vfunc_12();
    // slot 13: vfunc_13__16CPangyaEnemyDataFv (medium)  void vfunc_13();
    // slot 14: vfunc_14__16CPangyaEnemyDataFv (medium)  void vfunc_14();
    // slot 15: vfunc_15__16CPangyaEnemyDataFv (medium)  void vfunc_15();
    // slot 16: vfunc_16__16CPangyaEnemyDataFv (medium)  void vfunc_16();
    // slot 17: vfunc_17__16CPangyaEnemyDataFv (medium)  void vfunc_17();
    // slot 18: vfunc_18__16CPangyaEnemyDataFv (medium)  void vfunc_18();
    // slot 19: vfunc_19__16CPangyaEnemyDataFv (medium)  void vfunc_19();
    // slot 20: vfunc_20__16CPangyaEnemyDataFv (medium)  void vfunc_20();
    // slot 21: vfunc_21__16CPangyaEnemyDataFv (medium)  void vfunc_21();
    // slot 22: vfunc_22__16CPangyaEnemyDataFv (medium)  void vfunc_22();
    // slot 23: vfunc_23__16CPangyaEnemyDataFv (medium)  void vfunc_23();
    // slot 24: vfunc_24__16CPangyaEnemyDataFv (medium)  void vfunc_24();
    // slot 25: vfunc_25__16CPangyaEnemyDataFv (medium)  void vfunc_25();
    // slot 26: vfunc_26__16CPangyaEnemyDataFv (medium)  void vfunc_26();
    // slot 27: vfunc_27__16CPangyaEnemyDataFv (medium)  void vfunc_27();
    // slot 28: vfunc_28__16CPangyaEnemyDataFv (medium)  void vfunc_28();
    // slot 29: vfunc_29__16CPangyaEnemyDataFv (medium)  void vfunc_29();
    // slot 30: vfunc_30__16CPangyaEnemyDataFv (medium)  void vfunc_30();
    // slot 31: vfunc_31__16CPangyaEnemyDataFv (medium)  void vfunc_31();
    // slot 32: vfunc_32__16CPangyaEnemyDataFv (medium)  void vfunc_32();
    // slot 33: vfunc_33__16CPangyaEnemyDataFv (medium)  void vfunc_33();
    // slot 34: vfunc_34__16CPangyaEnemyDataFv (medium)  void vfunc_34();
    // slot 35: vfunc_35__16CPangyaEnemyDataFv (medium)  void vfunc_35();
    // slot 36: vfunc_36__16CPangyaEnemyDataFv (medium)  void vfunc_36();
    // slot 37: vfunc_37__16CPangyaEnemyDataFv (medium)  void vfunc_37();
    // slot 38: vfunc_38__16CPangyaEnemyDataFv (medium)  void vfunc_38();
    // slot 39: vfunc_39__16CPangyaEnemyDataFv (medium)  void vfunc_39();
    // slot 40: vfunc_40__16CPangyaEnemyDataFv (medium)  void vfunc_40();
    // slot 41: vfunc_41__16CPangyaEnemyDataFv (medium)  void vfunc_41();
    // slot 42: vfunc_42__16CPangyaEnemyDataFv (medium)  void vfunc_42();
    // slot 43: vfunc_43__16CPangyaEnemyDataFv (medium)  void vfunc_43();
    // slot 44: vfunc_44__16CPangyaEnemyDataFv (medium)  void vfunc_44();

    int vfunc_1();
    int vfunc_2();
    int vfunc_3();
    int vfunc_4();
    int vfunc_5();
    int vfunc_6();
    int vfunc_7();
    int vfunc_8();
    int vfunc_9();
    int vfunc_10();
    int vfunc_11();
    int vfunc_12();
    int vfunc_13();
    int vfunc_14();
    int vfunc_15();
    int vfunc_16();
    int vfunc_17();
    int vfunc_18();
    int vfunc_19();
    int vfunc_20();
    int vfunc_21();
    int vfunc_22();
    int vfunc_23();
    int vfunc_24();
    int vfunc_25();
    int vfunc_26();
    int vfunc_27();
    int vfunc_28();
    int vfunc_29();
    int vfunc_30();
    int vfunc_31();
    int vfunc_32();
    int vfunc_33();
    int vfunc_34();
    int vfunc_35();
    int vfunc_36();
    int vfunc_37();
    int vfunc_38();
    int vfunc_39();
    int vfunc_40();
    int vfunc_41();
    int vfunc_42();
    int vfunc_43();
    int vfunc_44();
};

#define CPANGYAENEMYDATA_OFF(f, o) typedef char CPangyaEnemyData_off_##f[((int)&((CPangyaEnemyData*)0)->f == (o)) ? 1 : -1]
CPANGYAENEMYDATA_OFF(dataBuffer, 0x0);
CPANGYAENEMYDATA_OFF(enemyVtable, 0x4);
CPANGYAENEMYDATA_OFF(sharedFile, 0x8);
#undef CPANGYAENEMYDATA_OFF

#endif  // CLASSES_CPANGYAENEMYDATA_H
