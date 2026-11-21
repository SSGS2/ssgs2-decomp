// gtl InputEx button sample representation query (virtual slot 4).
// API lookup: CInputExDeviceBase_UpdateButtons (gtl_input_ex_devices_part1.cpp) dispatches this slot
// before constructing a sample. Value 0 copies a raw integer button value;
// value 1 normalizes an analog value using device item parameters. The analog
// implementation returns 1 (gtl_input_ex_button_part2.cpp); the base returns 2 (gtl_input_ex_devices_part1.cpp).
// Digital sample ingestion/update/reset/value conversion lives in gtl_input_ex_button_part2.cpp;
// layouts/constructors in game/gtl_input.h. No gameplay tuning knobs here.
// Translation unit 0x803EB71C-0x803EB724 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

// Select integer sample representation for this digital button. Zero is a type
// discriminator, not failure, "released", or the current press state. The caller
// then passes that integer through IInputExButton_ProcessSample. No memory is read
// or written; the implicit ABI receiver is unused, and no ownership is transferred.
int gtl_CInputExDigitalButton_GetSampleType() {
    return 0;
}

} // extern "C"
