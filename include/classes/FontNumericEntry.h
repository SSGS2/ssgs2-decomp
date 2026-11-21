// FontNumericEntry: layout view owned by font_num_draw_entries.cpp.
// Evidence: NumericTextTable_ClearEntryText; UITextTable_SetEntryResourceName; UITextTable_ApplyNumericTextFormatFlags: high flags bit controls appFreeUni of text. No virtual declarations or runtime objects.
#ifndef CLASSES_FONTNUMERICENTRY_H
#define CLASSES_FONTNUMERICENTRY_H

class FontNumericEntry {
public:
    char pad_0[0xC];
    int resourceNameWord; // 0xC; NumericTextTable_ClearEntryText
    unsigned int textFlags; // 0x10; NumericTextTable_ClearEntryText
    unsigned int textLength; // 0x14; NumericTextTable_ClearEntryText
    void* text; // 0x18; NumericTextTable_ClearEntryText
    void* resource; // 0x1C; NumericTextTable_ClearEntryText
};

#define OFF(f, o) typedef char FontNumericEntry_off_##f[((int)&((FontNumericEntry*)0)->f == (o)) ? 1 : -1]
OFF(resourceNameWord, 0xC);
OFF(textFlags, 0x10);
OFF(textLength, 0x14);
OFF(text, 0x18);
OFF(resource, 0x1C);
#undef OFF

#endif
