// CWarningLogoLayout: layout view for CWarningLogo methods in warning_logo_screens.cpp.
// Native construction/EH classes stay in the translation unit; this view emits no methods or vtable.
#ifndef CLASSES_CWARNINGLOGOLAYOUT_H
#define CLASSES_CWARNINGLOGOLAYOUT_H

class CWarningLogoLayout {
public:
    void* vtable; // 0x0: CWarningLogo constructor/destructor install __vt__12CWarningLogo
    char pad_4[0x14];
    int scriptId; // 0x18: CBpCtrl_CreateEntry writes ID; destructor deletes it
    int state; // 0x1C: CWarningLogo_UpdateSequence state-machine switch
    int timelineUpdateCount; // 0x20: CWarningLogo_UpdateSequence increments once per call; timed thresholds
    int delayCounter; // 0x24: state 5 increments until 30
    int finished; // 0x28: terminal cases set to 1
    int unk_2C; // 0x2C: constructor clears; meaning not established
    int unk_30[4]; // 0x30: constructor copies four words of kWarningLogoInitialConfigurationWords
    int unk_40; // 0x40: CWarningLogo constructor initializes configuration word
    int unk_44; // 0x44: CWarningLogo constructor initializes configuration word
    int unk_48; // 0x48: CWarningLogo constructor initializes configuration word
    int unk_4C; // 0x4C: CWarningLogo constructor initializes configuration word
    int unk_50; // 0x50: CWarningLogo constructor initializes configuration word
    int unk_54; // 0x54: CWarningLogo constructor initializes configuration word
    int unk_58; // 0x58: CWarningLogo constructor initializes configuration word
    int unk_5C; // 0x5C: CWarningLogo constructor initializes configuration word
    int unk_60; // 0x60: CWarningLogo constructor initializes configuration word
    int unk_64; // 0x64: CWarningLogo constructor initializes configuration word
    int unk_68; // 0x68: CWarningLogo constructor initializes configuration word
    int unk_6C; // 0x6C: CWarningLogo constructor initializes configuration word
    int unk_70; // 0x70: CWarningLogo constructor initializes configuration word
    int unk_74; // 0x74: CWarningLogo constructor initializes configuration word
    int unk_78; // 0x78: CWarningLogo constructor initializes configuration word
};

#define OFF(f, o) typedef char CWarningLogoLayout_off_##f[((int)&((CWarningLogoLayout*)0)->f == (o)) ? 1 : -1]
OFF(vtable, 0x0);
OFF(scriptId, 0x18);
OFF(state, 0x1C);
OFF(timelineUpdateCount, 0x20);
OFF(delayCounter, 0x24);
OFF(finished, 0x28);
OFF(unk_2C, 0x2C);
OFF(unk_30, 0x30);
OFF(unk_40, 0x40);
OFF(unk_44, 0x44);
OFF(unk_48, 0x48);
OFF(unk_4C, 0x4C);
OFF(unk_50, 0x50);
OFF(unk_54, 0x54);
OFF(unk_58, 0x58);
OFF(unk_5C, 0x5C);
OFF(unk_60, 0x60);
OFF(unk_64, 0x64);
OFF(unk_68, 0x68);
OFF(unk_6C, 0x6C);
OFF(unk_70, 0x70);
OFF(unk_74, 0x74);
OFF(unk_78, 0x78);
#undef OFF

#endif
