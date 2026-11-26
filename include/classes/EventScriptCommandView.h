// EventScriptCommandView: layout view; methods in event_conte_script_interpreter.cpp (CEventConteProc::ProcessSealPointCommand and other bytecode handlers; operands are command-specific).
// No virtual methods, inheritance, or emitted runtime objects.
#ifndef CLASSES_SCRIPTCOMMAND80092804_H
#define CLASSES_SCRIPTCOMMAND80092804_H

class EventScriptCommandView {
public:
    unsigned char opcode;      // 0x0
    unsigned char operandByte; // 0x1
    short operandShort;        // 0x2
    unsigned int operandWord0; // 0x4
    unsigned int operandWord1; // 0x8
};

#define OFF(f, o) typedef char ScriptCommand80092804_off_##f[((int)&((EventScriptCommandView*)0)->f == (o)) ? 1 : -1]
OFF(opcode, 0x0);
OFF(operandByte, 0x1);
OFF(operandShort, 0x2);
OFF(operandWord0, 0x4);
OFF(operandWord1, 0x8);
#undef OFF

#define AsEventScriptCommandView(p) ((EventScriptCommandView*)(p))

#endif
