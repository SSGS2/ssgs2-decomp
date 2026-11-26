// Image/texture format dispatch, mip counts and platform layout copies,
// 0x803D4E18..0x803D6468. Pixel kernels: gtl_dxt_decode_and_pixel_convert.cpp; file parsers:
// gtl_image_font_resource_adapters.cpp (TGA/BMP/DDS), gtl_image_decode_and_tiling.cpp (GSR/GIM). Upload consumers:
// gx_render_state_and_texture_upload.cpp and engine_render_cache_and_texture_load.cpp. Preserve ABI names from symbols.txt.
// Mod lookup: mip chain floor -> GetMipLevelCount; one-pixel channel packing ->
// ConvertPixelFormat; engine/platform enum translation -> From/ToPlatformFormat;
// linear/tiled format pairs -> From/ToSwizzled; encoded-file probing/output ->
// GetDescription/GetImageData/GetPaletteData; row/tile layout -> CopyToPlatformLayout.
// Engine format IDs, platform layout IDs and encoded-file probe indices are
// THREE different domains; equal numbers do not imply interchangeable meanings.
#include <types.h>

// Platform selectors proven by ImageGSR_GetDescription / ImageGIM_GetDescription
// (gtl_image_decode_and_tiling.cpp). GSR type numbers describe container variants, not CPU byte order.
// Selectors 0,1,2,9 also occur in pixel dispatch; their physical platform names
// are not established. Keep them numeric rather than guess a console identity.
enum ImagePlatformLayout {
    kImageGsrType4Layout = 3,
    kImageGsrType2Layout = 4,
    kImageGsrType1Layout = 5,
    kImageGsrType3Layout = 6,
    kImageGsrType5Layout = 7,
    kImageGimLayout = 8
};
// Engine IDs used by mip sizing. The 32-bit channel names are LOGICAL packed
// words, not a claim about byte order in every platform layout. Evidence:
// FromPlatformFormat(layout 5) maps D3D A8R8G8B8/X8R8G8B8/A8B8G8R8/X8B8G8R8
// and DXT FourCCs to these IDs; the copy/swap kernels implement byte ordering.
enum ImageMipFormat {
    kImageArgb8 = 1,
    kImageXrgb8 = 2,
    kImageAbgr8 = 14,
    kImageXbgr8 = 15,
    kImageIndexed8 = 30,
    kImageDxt1 = 46,
    kImageDxt3 = 48,
    kImageDxt5 = 50,
    kImageSwizzledArgb8 = 63,
    kImageSwizzledXrgb8 = 64
};
// Descriptor schema is native 32-bit words after format-specific decoding.
// TGA/BMP/DDS write the first four words; GSR additionally writes +10 layout.
// Reserve >=0x14 bytes for description and 8 for palette, since the handler
// determines which words are touched. No generic clearing occurs on entry.
struct ImageDescriptionSchema {
    s32 engineFormat, widthPixels, heightPixels, mipLevelCount, sourceLayout;
};
struct ImagePaletteSchema {
    s32 engineFormat, entryCount;
};

extern "C" {

// Probe records are 8 bytes: handler index and int probe(imageAddress,0).
// Parallel function tables below contain 11 Wii code-address words, not data.
struct GtlDescEntry {
    s32 index;
    s32 (*test)(s32, s32);
};

struct GtlDescTable {
    GtlDescEntry entries[11];
};

struct GtlFnTable {
    s32 fn[11];
};

typedef s32 (*GtlDescFn3)(s32, s32, s32);
typedef s32 (*GtlImageFn2)(s32, s32);

// Return the FIRST successful probe's handler index, or -1. encodedImage is a
// trusted readable file buffer; probes have no usable byte bound (second arg=0).
// Does not validate the selected operation's function pointer or modify outputs.
static inline s32 GtlFindIndex(GtlDescTable* t, s32 encodedImage) {
    GtlDescEntry* e = t->entries;
    u32 i;
    for (i = 0; i < 11; i++, e++) {
        if (e->test != 0 && e->test(encodedImage, 0) != 0) {
            return e->index;
        }
    }
    return -1;
}

// External table definitions (original .data); ordered index/probe/operations:
// index format      description     pixels          palette
// 0 TGA             TGA handler     TGA handler     TGA handler
// 1 BMP             BMP handler     BMP handler     NULL
// 2 TIM2            NULL            NULL            NULL
// 3 GSR type4       GSR handler     GSR handler     NULL
// 4 GSR type2       GSR handler     GSR handler     GSR handler
// 5 GSR type1       GSR handler     GSR handler     GSR handler
// 6 GSR type3       GSR handler     GSR handler     NULL
// 7 GSR type5       GSR handler     GSR handler     GSR handler
// 8 GIM             GIM handler     NULL            NULL
// 9 PICT            PICT handler    NULL            NULL
// 10 DDS            DDS handler     DDS handler     NULL
// Probes: ImageTGA/BMP/TIM2_IsValid, ImageGSR_IsType4/2/1/3/5,
// ImageGIM_IsValid, ImagePICT_Probe, ImageDDS_Probe, all prefixed gtl_.
// IMPORTANT: wrappers do not check NULL operation entries. Only request an
// operation supported above; an unsupported recognized file can call address 0.
extern GtlDescTable s_imageFormatProbeTable;  // {index, probe} order above.
extern GtlFnTable s_imageDescriptionHandlers; // int description(out,paletteOut,image).
extern GtlFnTable s_imagePixelDataHandlers;   // int pixels(pointerWordOut,image).
extern GtlFnTable s_imagePaletteDataHandlers; // int palette(pointerWordOut,image).

extern void* memcpy(void*, const void*, unsigned long);
// The original tail call forwards the live argument registers (r3..r10).
void gtl_Image_LinearToTiled(void);
void gtl_Image_ConvertMortonLayout(s32, s32, s32, s32, s32, s32, s32, s32, s32);
s32 gtl_ImageFormat_GetBytesPerElement(s32);

void gtl_PixelConvert_RGBA8ToPacked4444(s32, s32, s32, s32, s32, s32);
void gtl_PixelConvert_NoOp_0(s32, s32, s32, s32, s32, s32);
void gtl_PixelConvert_NoOp_1(s32, s32, s32, s32, s32, s32);
void gtl_PixelCopy_Copy4Bytes_0(s32, s32, s32, s32, s32, s32);
void gtl_PixelCopy_Copy4Bytes_1(s32, s32, s32, s32, s32, s32);
void gtl_PixelCopy_Copy4Bytes_2(s32, s32, s32, s32, s32, s32);
void gtl_PixelCopy_Copy1Byte(s32, s32, s32, s32, s32, s32);
void gtl_PixelConvert_SwapRB32_0(s32, s32, s32, s32, s32, s32);
void gtl_PixelConvert_SwapRB32_1(s32, s32, s32, s32, s32, s32);
void gtl_PixelConvert_SwapRB32_2(s32, s32, s32, s32, s32, s32);
void gtl_PixelConvert_SwapRB32_3(s32, s32, s32, s32, s32, s32);
void gtl_PixelConvert_RGBX8ToRGBA8(s32, s32, s32, s32, s32, s32);
void gtl_PixelConvert_RGBX8ToARGB8_0(s32, s32, s32, s32, s32, s32);
void gtl_PixelConvert_RGBX8ToARGB8_1(s32, s32, s32, s32, s32, s32);
void gtl_PixelConvert_RGBX8ToARGB8_2(s32, s32, s32, s32, s32, s32);
void gtl_PixelConvert_BGRX8ToRGBA8(s32, s32, s32, s32, s32, s32);
void gtl_PixelConvert_CopyRGB8(s32, s32, s32, s32, s32, s32);
void gtl_PixelConvert_CopyAlphaByte(s32, s32, s32, s32, s32, s32);
void gtl_PixelConvert_RGBA8ToARGB8_0(s32, s32, s32, s32, s32, s32);
void gtl_PixelConvert_RGBA8ToARGB8_1(s32, s32, s32, s32, s32, s32);
void gtl_PixelConvert_Unpack4444(s32, s32, s32, s32, s32, s32);
void gtl_PixelConvert_SwapRB24(s32, s32, s32, s32, s32, s32);
void gtl_PixelConvert_RGB565ToRGBA8(s32, s32, s32, s32, s32, s32);
void gtl_PixelConvert_CopyRGBA8(s32, s32, s32, s32, s32, s32);
void gtl_PixelConvert_CopyByte0(s32, s32, s32, s32, s32, s32);

s32 gtl_Image_GetMipLevelCount(u32 width, u32 height, s32 format, s32 platform);
void gtl_Image_ConvertPixelFormat(s32 destinationPixel, s32 sourcePixel, s32 destinationFormat, s32 sourceFormat,
                                  s32 sourceLayout, s32 destinationLayout);
s32 gtl_ImageFormat_FromPlatformFormat(s32 platformFormat, s32 platformLayout);
s32 gtl_ImageFormat_ToPlatformFormat(s32 engineFormat, s32 platformLayout);
s32 gtl_ImageFormat_IsBlockCompressed(s32 engineFormat);
s32 gtl_ImageFormat_ToSwizzled(s32 engineFormat);
s32 gtl_ImageFormat_FromSwizzled(s32 engineFormat);
s32 gtl_ImageFormat_IsPaletted(s32 engineFormat);
s32 gtl_Image_GetDescription(s32 descriptionOut, s32 paletteDescriptionOut, s32 encodedImage);
s32 gtl_Image_GetImageData(s32 pixelAddressOut, s32 encodedImage);
s32 gtl_Image_GetPaletteData(s32 paletteAddressOut, s32 encodedImage);
void gtl_Image_CopyToPlatformLayout(s32 sourcePixels, s32 unusedSourceWord1, s32 unusedSourceWord2,
                                    s32 destinationPixels, s32 copyWidth, s32 copyHeight, s32 unusedDestinationWord,
                                    s32 engineFormat, s32 destinationLayout);

// Count levels INCLUDING level zero by halving both pixel dimensions until
// EITHER falls below its platform/format floor. Inputs must be positive.
// Supported domain ONLY: layout 3, any format (floor 1x1); layouts 5/7 with
// formats 1/2/14/15 (4x4), 30 (8x4), 46/48/50 (8x8); layout 6 with formats
// 1/2/63/64 (4x4). Every other pair leaves minW/minH uninitialized; callers
// must validate before calling. A sub-floor input still returns one. Odd sizes
// truncate on shift; no power-of-two rounding. No allocation or state changes.
s32 gtl_Image_GetMipLevelCount(u32 width, u32 height, s32 format, s32 platform) {
    u32 minW;
    u32 minH;
    s32 count;

    switch (platform) {
    case kImageGsrType1Layout:
    case kImageGsrType5Layout:
        switch (format) {
        case kImageArgb8:
        case kImageXrgb8:
        case kImageAbgr8:
        case kImageXbgr8:
            minW = 4;
            minH = 4;
            break;
        case kImageIndexed8:
            minW = 8;
            minH = 4;
            break;
        case kImageDxt1:
        case kImageDxt3:
        case kImageDxt5:
            minW = 8;
            minH = 8;
            break;
        }
        break;
    case kImageGsrType4Layout:
        minW = 1;
        minH = 1;
        break;
    case kImageGsrType3Layout:
        switch (format) {
        case kImageArgb8:
        case kImageXrgb8:
        case kImageSwizzledArgb8:
        case kImageSwizzledXrgb8:
            minW = 4;
            minH = 4;
            break;
        }
        break;
    }

    count = 1;
    for (;;) {
        width >>= 1;
        height >>= 1;
        if (width < minW || height < minH) {
            break;
        }
        count++;
    }
    return count;
}

// Convert ONE pixel from borrowed source to writable destination (32-bit ABI
// addresses). Require enough bytes for both formats and distinct buffers for
// reorder kernels; this is not an image-size loop or an allocation API.
// Decisions: destination layout -> source layout/format -> destination format
// -> named kernel. Unsupported combinations and the named NoOp kernels return
// without writing; there is no error/status return. Layout 2 routes sources
// 3/5/6/7 through the layout-3 policy; layout-4/8/9 entry points choose their
// format policy without inspecting sourceLayout. Preserve those shared joins.
// Engine formats 30/88 are linear/swizzled indexed8; 1/63 and 2/64 are
// linear/swizzled packed 32-bit pairs; 14/76,15/77,16/78,17/79 are further
// linear/swizzled pairs. From/ToSwizzled below is the authoritative pair map.
// Read the gtl_dxt_decode_and_pixel_convert.cpp kernel body before a channel-order mod: format ID alone
// does not describe memory byte order. Semantic joins below retain the exact
// original compare/branch layout; all 12 functions in this unit are exact.
void gtl_Image_ConvertPixelFormat(s32 destinationPixel, s32 sourcePixel, s32 destinationFormat, s32 sourceFormat,
                                  s32 sourceLayout, s32 destinationLayout) {
    switch ((u32)destinationLayout) {
        // Original jump table jumptable_804D9DF8 (the Wii labels follow code order, not the table values).
    case 0:
    destinationLayout0:
        if (sourceLayout == 1)
            goto destinationLayout1;
        if (sourceLayout >= 1)
            return;
        if (sourceLayout < 0)
            return;
        if (sourceFormat != 1)
            return;
        if (destinationFormat == 8)
            goto pack4444ForLayout0;
        if (destinationFormat >= 8)
            return;
        if (destinationFormat >= 7)
            goto keepFormat7ForLayout0;
    conversionDone:
        return;
    pack4444ForLayout0:
        gtl_PixelConvert_RGBA8ToPacked4444(destinationPixel, sourcePixel, destinationFormat, sourceFormat, sourceLayout,
                                           destinationLayout);
        return;
    keepFormat7ForLayout0:
        gtl_PixelConvert_NoOp_0(destinationPixel, sourcePixel, destinationFormat, sourceFormat, sourceLayout,
                                destinationLayout);
        return;
    case 1:
    destinationLayout1:
        if (sourceFormat != 1)
            return;
        switch (destinationFormat) {
        default:
            return;
        case 1:
            gtl_PixelCopy_Copy4Bytes_0(destinationPixel, sourcePixel, destinationFormat, sourceFormat, sourceLayout,
                                       destinationLayout);
            return;
        }
    case 2:
    destinationLayout2:
        switch (sourceLayout) {
        case 3:
        case 5:
        case 6:
        case 7:
            goto destinationLayout3;
        case 4:
        layout2FromLayout4:
            if (sourceFormat == 30)
                goto layout2CopyIndexed8;
            if (sourceFormat >= 30)
                goto layout2HighSourceFormat;
            if (sourceFormat == 1)
                goto layout2SourceFormat1;
            return;
        layout2HighSourceFormat:
            if (sourceFormat == 64)
                goto layout2SourceFormat64;
            if (sourceFormat >= 64)
                return;
            if (sourceFormat >= 63)
                goto layout2SourceFormat63;
            return;
        layout2SourceFormat64:
            if (destinationFormat == 77)
                goto layout2KeepFormat15Or77;
            if (destinationFormat >= 77)
                return;
            switch (destinationFormat) {
            default:
                return;
            case 15:
                goto layout2KeepFormat15Or77;
            }
        layout2KeepFormat15Or77:
            gtl_PixelConvert_NoOp_1(destinationPixel, sourcePixel, destinationFormat, sourceFormat, sourceLayout,
                                    destinationLayout);
            return;
        layout2SourceFormat63:
            if (destinationFormat == 76)
                goto layout2SwapFormat14Or76;
            if (destinationFormat >= 76)
                return;
            switch (destinationFormat) {
            default:
                return;
            case 14:
                goto layout2SwapFormat14Or76;
            }
        layout2SwapFormat14Or76:
            gtl_PixelConvert_SwapRB32_1(destinationPixel, sourcePixel, destinationFormat, sourceFormat, sourceLayout,
                                        destinationLayout);
            return;
        layout2SourceFormat1:
            if (destinationFormat == 76)
                goto layout2SwapFormat76;
            if (destinationFormat >= 76)
                return;
            switch (destinationFormat) {
            default:
                return;
            case 1:
                goto layout2CopyFormat1;
            }
        layout2CopyFormat1:
            gtl_PixelCopy_Copy4Bytes_0(destinationPixel, sourcePixel, destinationFormat, sourceFormat, sourceLayout,
                                       destinationLayout);
            return;
        layout2SwapFormat76:
            gtl_PixelConvert_SwapRB32_0(destinationPixel, sourcePixel, destinationFormat, sourceFormat, sourceLayout,
                                        destinationLayout);
            return;
        layout2CopyIndexed8:
            switch (destinationFormat) {
            default:
                return;
            case 30:
                gtl_PixelCopy_Copy1Byte(destinationPixel, sourcePixel, destinationFormat, sourceFormat, sourceLayout,
                                        destinationLayout);
                return;
            }
        default:
            goto conversionDone;
        }
    case 3:
    destinationLayout3:
        switch (sourceFormat) {
        case 0:
            goto layout3SourceFormat0;
        case 1:
            goto layout3SourceFormat1;
        case 5:
            goto layout3SourceRgb565;
        case 62:
            goto layout3SourceFormat62;
        case 63:
            goto destinationLayout4OrSource63;
        case 64:
            goto destinationLayout8OrSource64;
        case 70:
            goto destinationLayout9OrSource70;
        case 88:
            goto layout3SourceIndexed8;
        default:
            return;
        }
    layout3SourceFormat62:
        if (destinationFormat == 17)
            goto layout3RgbxToArgbVariant1;
        if (destinationFormat >= 17)
            goto layout3HighDestinationFormat;
        if (destinationFormat == 14)
            goto layout3BgrxToRgba;
        if (destinationFormat >= 14)
            goto layout3Format15Or16;
        if (destinationFormat == 2)
            goto layout3RgbxToRgba;
        return;
    layout3Format15Or16:
        if (destinationFormat >= 16)
            goto layout3RgbxToArgbVariant0;
        return;
    layout3HighDestinationFormat:
        if (destinationFormat == 76)
            goto layout3BgrxToRgba;
        if (destinationFormat >= 76)
            goto layout3Format78;
        if (destinationFormat == 64)
            goto layout3RgbxToRgba;
        return;
    layout3Format78:
        if (destinationFormat == 78)
            goto layout3RgbxToArgbVariant0;
        return;
    layout3RgbxToRgba:
        gtl_PixelConvert_RGBX8ToRGBA8(destinationPixel, sourcePixel, destinationFormat, sourceFormat, sourceLayout,
                                      destinationLayout);
        return;
    layout3RgbxToArgbVariant1:
        gtl_PixelConvert_RGBX8ToARGB8_1(destinationPixel, sourcePixel, destinationFormat, sourceFormat, sourceLayout,
                                        destinationLayout);
        return;
    layout3RgbxToArgbVariant0:
        gtl_PixelConvert_RGBX8ToARGB8_0(destinationPixel, sourcePixel, destinationFormat, sourceFormat, sourceLayout,
                                        destinationLayout);
        return;
    layout3BgrxToRgba:
        gtl_PixelConvert_BGRX8ToRGBA8(destinationPixel, sourcePixel, destinationFormat, sourceFormat, sourceLayout,
                                      destinationLayout);
        return;
    layout3SourceFormat1:
        if (destinationFormat == 28)
            goto layout3CopyAlpha;
        if (destinationFormat >= 28)
            goto layout3DestinationFormat76Or62;
        if (destinationFormat == 1)
            goto layout3CopyFourBytes;
        if (destinationFormat >= 1)
            goto layout3DestinationFormat14;
        if (destinationFormat >= 0)
            goto layout3CopyRgb;
        return;
    layout3DestinationFormat14:
        if (destinationFormat == 14)
            goto layout3SwapRedBlue;
        return;
    layout3DestinationFormat76Or62:
        if (destinationFormat == 76)
            goto layout3SwapRedBlue;
        if (destinationFormat >= 76)
            return;
        if (destinationFormat == 62)
            goto layout3CopyRgb;
        return;
    layout3CopyFourBytes:
        gtl_PixelCopy_Copy4Bytes_0(destinationPixel, sourcePixel, destinationFormat, sourceFormat, sourceLayout,
                                   destinationLayout);
        return;
    layout3SwapRedBlue:
        gtl_PixelConvert_SwapRB32_0(destinationPixel, sourcePixel, destinationFormat, sourceFormat, sourceLayout,
                                    destinationLayout);
        return;
    layout3CopyRgb:
        gtl_PixelConvert_CopyRGB8(destinationPixel, sourcePixel, destinationFormat, sourceFormat, sourceLayout,
                                  destinationLayout);
        return;
    layout3CopyAlpha:
        gtl_PixelConvert_CopyAlphaByte(destinationPixel, sourcePixel, destinationFormat, sourceFormat, sourceLayout,
                                       destinationLayout);
        return;
    case 4:
    destinationLayout4OrSource63:
        switch (destinationFormat) {
        case 1:
        case 63:
        layout4CopyFourBytes:
            gtl_PixelCopy_Copy4Bytes_0(destinationPixel, sourcePixel, destinationFormat, sourceFormat, sourceLayout,
                                       destinationLayout);
            return;
        case 14:
        case 76:
        layout4SwapRedBlue:
            gtl_PixelConvert_SwapRB32_0(destinationPixel, sourcePixel, destinationFormat, sourceFormat, sourceLayout,
                                        destinationLayout);
            return;
        case 16:
        case 78:
        layout4RgbaToArgb:
            gtl_PixelConvert_RGBA8ToARGB8_0(destinationPixel, sourcePixel, destinationFormat, sourceFormat,
                                            sourceLayout, destinationLayout);
            return;
        default:
            goto conversionDone;
        }
    case 9:
    destinationLayout9OrSource70:
        switch (destinationFormat) {
        default:
            return;
        case 16:
            gtl_PixelConvert_Unpack4444(destinationPixel, sourcePixel, destinationFormat, sourceFormat, sourceLayout,
                                        destinationLayout);
            return;
        }
    layout3SourceIndexed8:
        if (destinationFormat == 88)
            goto layout3CopyIndexed8;
        if (destinationFormat >= 88)
            return;
        switch (destinationFormat) {
        default:
            return;
        case 30:
            goto layout3CopyIndexed8;
        }
    layout3CopyIndexed8:
        gtl_PixelCopy_Copy1Byte(destinationPixel, sourcePixel, destinationFormat, sourceFormat, sourceLayout,
                                destinationLayout);
        return;
    layout3SourceFormat0:
        switch (destinationFormat) {
        default:
            return;
        case 13:
            gtl_PixelConvert_SwapRB24(destinationPixel, sourcePixel, destinationFormat, sourceFormat, sourceLayout,
                                      destinationLayout);
            return;
        }
    case 8:
    destinationLayout8OrSource64:
        switch (destinationFormat) {
        case 2:
        case 64:
        layout8CopyFourBytes:
            gtl_PixelCopy_Copy4Bytes_2(destinationPixel, sourcePixel, destinationFormat, sourceFormat, sourceLayout,
                                       destinationLayout);
            return;
        case 77:
        layout8SwapRedBlue:
            gtl_PixelConvert_SwapRB32_3(destinationPixel, sourcePixel, destinationFormat, sourceFormat, sourceLayout,
                                        destinationLayout);
            return;
        case 17:
        case 79:
        layout8RgbaToArgb:
            gtl_PixelConvert_RGBA8ToARGB8_1(destinationPixel, sourcePixel, destinationFormat, sourceFormat,
                                            sourceLayout, destinationLayout);
            return;
        case 16:
        layout8RgbxToArgb:
            gtl_PixelConvert_RGBX8ToARGB8_2(destinationPixel, sourcePixel, destinationFormat, sourceFormat,
                                            sourceLayout, destinationLayout);
            return;
        default:
            goto conversionDone;
        }
    layout3SourceRgb565:
        switch (destinationFormat) {
        default:
            return;
        case 1:
            gtl_PixelConvert_RGB565ToRGBA8(destinationPixel, sourcePixel, destinationFormat, sourceFormat, sourceLayout,
                                           destinationLayout);
            return;
        }
    case 5:
    destinationLayout5:
        if (sourceLayout == 5)
            goto layout5FromLayout5;
        if (sourceLayout >= 5)
            return;
        if (sourceLayout < 4)
            return;
        if (sourceFormat != 30)
            return;
        switch (destinationFormat) {
        default:
            return;
        case 30:
            gtl_PixelCopy_Copy1Byte(destinationPixel, sourcePixel, destinationFormat, sourceFormat, sourceLayout,
                                    destinationLayout);
            return;
        }
    layout5FromLayout5:
        if (sourceFormat != 1)
            return;
        switch (destinationFormat) {
        default:
            return;
        case 1:
            gtl_PixelCopy_Copy4Bytes_0(destinationPixel, sourcePixel, destinationFormat, sourceFormat, sourceLayout,
                                       destinationLayout);
            return;
        }
    case 6:
    destinationLayout6:
        if (sourceLayout != 1)
            return;
        if (sourceFormat == 63)
            goto layout6SourceFormat63;
        if (sourceFormat >= 63)
            goto layout6SourceFormat76;
        if (sourceFormat == 30)
            goto layout6CopyIndexed8;
        if (sourceFormat >= 30)
            return;
        if (sourceFormat == 14)
            goto layout6SourceFormat14Or76;
        return;
    layout6SourceFormat76:
        if (sourceFormat != 76)
            return;
    layout6SourceFormat14Or76:
        if (destinationFormat == 63)
            goto layout6SwapRedBlue;
        if (destinationFormat >= 63)
            goto layout6DestinationFormat76;
        if (destinationFormat == 14)
            goto layout6CopyFourBytes;
        if (destinationFormat >= 14)
            return;
        if (destinationFormat == 1)
            goto layout6SwapRedBlue;
        return;
    layout6DestinationFormat76:
        switch (destinationFormat) {
        default:
            return;
        case 76:
            goto layout6CopyFourBytes;
        }
    layout6CopyFourBytes:
        gtl_PixelCopy_Copy4Bytes_1(destinationPixel, sourcePixel, destinationFormat, sourceFormat, sourceLayout,
                                   destinationLayout);
        return;
    layout6SwapRedBlue:
        gtl_PixelConvert_SwapRB32_2(destinationPixel, sourcePixel, destinationFormat, sourceFormat, sourceLayout,
                                    destinationLayout);
        return;
    layout6SourceFormat63:
        if (destinationFormat == 63)
            goto layout6CopyRgba;
        if (destinationFormat >= 63)
            return;
        switch (destinationFormat) {
        default:
            return;
        case 2:
            goto layout6CopyRgba;
        }
    layout6CopyRgba:
        gtl_PixelConvert_CopyRGBA8(destinationPixel, sourcePixel, destinationFormat, sourceFormat, sourceLayout,
                                   destinationLayout);
        return;
    layout6CopyIndexed8:
        switch (destinationFormat) {
        default:
            return;
        case 30:
            gtl_PixelConvert_CopyByte0(destinationPixel, sourcePixel, destinationFormat, sourceFormat, sourceLayout,
                                       destinationLayout);
            return;
        }
    case 7:
    destinationLayout7:
        if (sourceLayout != 1)
            return;
        if (sourceFormat == 76)
            goto layout7SourceFormat14Or76;
        if (sourceFormat >= 76)
            return;
        if (sourceFormat != 14)
            return;
    layout7SourceFormat14Or76:
        if (destinationFormat == 63)
            goto layout7SwapRedBlue;
        if (destinationFormat >= 63)
            return;
        switch (destinationFormat) {
        default:
            return;
        case 1:
            goto layout7SwapRedBlue;
        }
    layout7SwapRedBlue:
        gtl_PixelConvert_SwapRB32_2(destinationPixel, sourcePixel, destinationFormat, sourceFormat, sourceLayout,
                                    destinationLayout);
        return;
    default:
        return;
    }
}

// Translate a platform's stored format to an engine ID; unknown pairs return -1.
// Layout 3 passes the word through. High-bit modifiers in layouts 4/7/8 are
// significant; these are integer codes, not pointers, signed sizes or flags to strip.
s32 gtl_ImageFormat_FromPlatformFormat(s32 platformFormat, s32 platformLayout) {
    switch (platformLayout) {
    case 8:
        switch (platformFormat) {
        case 0:
            return 0x50;
        case 1:
            return 0x52;
        case 2:
            return 0x53;
        case 3:
            return 0x4C;
        case 4:
            return 0x59;
        case 5:
            return 0x58;
        case (s32)0x80000005:
            return 0x1E;
        case (s32)0x80000001:
            return 0x14;
        default:
            return -1;
        }
    case 6:
        switch (platformFormat) {
        case 0x18280086:
            return 0x3F;
        case 0x28280086:
            return 0x40;
        case 0x28280186:
            return 2;
        case 0x18280186:
            return 1;
        default:
            return -1;
        }
    case 4:
        switch (platformFormat) {
        case (s32)0x80000003:
            return 0xE;
        case 3:
            return 0x4C;
        case 5:
            return 0x58;
        case 0:
            return 0x50;
        case 1:
            return 0x52;
        case (s32)0x80000002:
            return 0x15;
        case 2:
            return 0x53;
        case 4:
            return 0x59;
        case (s32)0x80000005:
            return 0x1E;
        case (s32)0x80000001:
            return 0x14;
        case (s32)0x80000004:
            return 0x1F;
        default:
            return -1;
        }
    case 3:
        return platformFormat;
    case 7:
        switch (platformFormat) {
        case 0:
            return 0x5B;
        case 6:
            return 0x4C;
        case 0xE:
            return 0x62;
        case (s32)0x80000006:
            return 0xE;
        case (s32)0xC0000006:
            return 0xF;
        case 0x40000006:
            return 0x4D;
        case (s32)0x80000000:
            return 0x21;
        case (s32)0x80000008:
            return 0x1F;
        case (s32)0x80000009:
            return 0x1E;
        case (s32)0x8000000E:
            return 0x2E;
        case (s32)0xC0000030:
            return 0x30;
        case (s32)0xC0000032:
            return 0x32;
        case 0x16:
            return 0x6C;
        case (s32)0x80000016:
            return 0x38;
        case (s32)0x80000002:
            return 0x25;
        case (s32)0x80000001:
            return 0x22;
        case (s32)0x80000003:
            return 0x24;
        case 0x2B:
            return 0x60;
        default:
            return -1;
        }
    case 5:
        switch (platformFormat) {
        case 0x15:
            return 1;
        case 0x20:
            return 0xE;
        case 0x16:
            return 2;
        case 0x21:
            return 0xF;
        case 0x17:
            return 5;
        case 0x31545844:
            return 0x2E;
        case 0x33545844:
            return 0x30;
        case 0x35545844:
            return 0x32;
        case 0x29:
            return 0x1E;
        case 0x4B:
            return 0x36;
        case 0x50:
            return 0x37;
        case 0x71:
            return 0x19;
        default:
            return -1;
        }
    default:
        return -1;
    }
}

// Translate engine ID to a stored platform format, or -1. This is not a perfect
// inverse: aliases coalesce, and engine -1 maps to 0 on layouts 4/5/8. No I/O.
s32 gtl_ImageFormat_ToPlatformFormat(s32 engineFormat, s32 platformLayout) {
    switch (platformLayout) {
    case 6:
        switch (engineFormat) {
        case 63:
            return 0x18280086;
        case 64:
            return 0x28280086;
        case 2:
            return 0x28280086;
        case 1:
            return 0x18280186;
        case -1:
            return -1;
        default:
            return -1;
        }
    case 8:
    case 4:
        switch (engineFormat) {
        case 14:
            return 0x80000003;
        case 76:
            return 3;
        case 83:
            return 2;
        case 80:
            return 0;
        case 30:
            return 0x80000005;
        case 88:
            return 5;
        case 31:
            return 0x80000004;
        case 82:
            return 1;
        case -1:
            return 0;
        default:
            return -1;
        }
    case 7:
        switch (engineFormat) {
        case 76:
        case 13:
            return 6;
        case 77:
            return 0x40000006;
        case 14:
            return 0x80000006;
        case -1:
            return -1;
        case 30:
            return 0x80000009;
        case 80:
            return 4;
        case 15:
            return 0xC0000006;
        case 46:
            return 0x8000000E;
        case 98:
            return 0xE;
        case 48:
            return 0xC0000030;
        case 50:
            return 0xC0000032;
        default:
            return -1;
        }
    case 5:
        switch (engineFormat) {
        case 1:
        case 63:
            return 0x15;
        case 14:
        case 76:
            return 0x20;
        case 2:
            return 0x16;
        case 15:
        case 77:
            return 0x21;
        case 62:
            return 0x14;
        case 30:
        case 88:
            return 0x29;
        case 67:
            return 0x17;
        case 46:
        case 98:
            return 0x31545844;
        case 48:
        case 100:
            return 0x33545844;
        case 50:
        case 102:
            return 0x35545844;
        case 25:
            return 0x71;
        case -1:
            return 0;
        }
        break;
    case 3:
        return engineFormat;
    }
    return -1;
}

// Classify the ten explicit compressed IDs; all others, including -1, return 0.
s32 gtl_ImageFormat_IsBlockCompressed(s32 engineFormat) {
    switch (engineFormat) {
    case 0x2E:
    case 0x2F:
    case 0x30:
    case 0x31:
    case 0x32:
    case 0x62:
    case 0x63:
    case 0x64:
    case 0x65:
    case 0x66:
        return 1;
    }
    return 0;
}

// Map a supported linear ID to its tiled/swizzled counterpart. Already-swizzled
// IDs pass through; unsupported linear IDs return -1. Changes a code, not pixels.
s32 gtl_ImageFormat_ToSwizzled(s32 engineFormat) {
    s32 isLinearFormat;

    if (engineFormat != 0x4F) {
        if (engineFormat < 0x4F) {
            if (engineFormat < 0x41) {
                if (engineFormat < 0x3E) {
                    goto block_12;
                }
                goto block_11;
            }
            if (engineFormat < 0x43) {
                goto block_12;
            }
            goto block_11;
        }
        if (engineFormat < 0x62) {
            if (engineFormat < 0x5A) {
                goto block_11;
            }
            goto block_12;
        }
        if (engineFormat < 0x67) {
        block_11:
            isLinearFormat = 0;
        } else {
            goto block_12;
        }
    } else {
    block_12:
        isLinearFormat = 1;
    }

    if (isLinearFormat == 0) {
        return engineFormat;
    }
    switch (engineFormat) {
    case 0:
        return 0x3E;
    case 1:
        return 0x3F;
    case 16:
        return 0x4E;
    case 2:
        return 0x40;
    case 5:
        return 0x43;
    case 6:
        return 0x44;
    case 7:
        return 0x45;
    case 8:
        return 0x46;
    case 9:
        return 0x47;
    case 10:
        return 0x49;
    case 11:
        return 0x4A;
    case 13:
        return 0x4B;
    case 14:
        return 0x4C;
    case 15:
        return 0x4D;
    case 18:
        return 0x50;
    case 19:
        return 0x51;
    case 20:
        return 0x52;
    case 21:
        return 0x53;
    case 22:
        return 0x54;
    case 23:
        return 0x55;
    case 24:
        return 0x56;
    case 46:
        return 0x62;
    case 47:
        return 0x63;
    case 48:
        return 0x64;
    case 49:
        return 0x65;
    case 50:
        return 0x66;
    case 30:
        return 0x58;
    case 17:
        return 0x4F;
    }
    return -1;
}

// Reverse supported swizzled IDs to linear. Linear IDs pass through; unhandled
// codes in the swizzled family return -1. Pair with actual layout copying below.
s32 gtl_ImageFormat_FromSwizzled(s32 engineFormat) {
    s32 isLinearFormat;

    if (engineFormat != 0x4F) {
        if (engineFormat < 0x4F) {
            if (engineFormat < 0x41) {
                if (engineFormat < 0x3E) {
                    goto block_12;
                }
                goto block_11;
            }
            if (engineFormat < 0x43) {
                goto block_12;
            }
            goto block_11;
        }
        if (engineFormat < 0x62) {
            if (engineFormat < 0x5A) {
                goto block_11;
            }
            goto block_12;
        }
        if (engineFormat < 0x67) {
        block_11:
            isLinearFormat = 0;
        } else {
            goto block_12;
        }
    } else {
    block_12:
        isLinearFormat = 1;
    }

    if (isLinearFormat == 0) {
        switch (engineFormat) {
        case 0x3E:
            return 0;
        case 0x3F:
            return 1;
        case 0x4E:
            return 0x10;
        case 0x40:
            return 2;
        case 0x43:
            return 5;
        case 0x44:
            return 6;
        case 0x45:
            return 7;
        case 0x46:
            return 8;
        case 0x47:
            return 9;
        case 0x49:
            return 0xA;
        case 0x4A:
            return 0xB;
        case 0x4B:
            return 0xD;
        case 0x4C:
            return 0xE;
        case 0x4D:
            return 0xF;
        case 0x50:
            return 0x12;
        case 0x51:
            return 0x13;
        case 0x52:
            return 0x14;
        case 0x53:
            return 0x15;
        case 0x54:
            return 0x16;
        case 0x55:
            return 0x17;
        case 0x56:
            return 0x18;
        case 0x58:
            return 0x1E;
        case 0x59:
            return 0x1F;
        case 0x62:
            return 0x2E;
        case 0x63:
            return 0x2F;
        case 0x64:
            return 0x30;
        case 0x65:
            return 0x31;
        case 0x66:
            return 0x32;
        }
        return -1;
    }
    return engineFormat;
}

// Recognize indexed8/indexed4 in linear and swizzled layouts (30/31/88/89).
s32 gtl_ImageFormat_IsPaletted(s32 engineFormat) {
    switch (engineFormat) {
    case 0x1E:
    case 0x1F:
    case 0x58:
    case 0x59:
        return 1;
    default:
        return 0;
    }
}

#pragma push
#pragma use_lmw_stmw on
// Dispatch a trusted encoded file to int handler(description,palette,image).
// Copies tables to preserve the original stack/loads; never takes ownership.
static inline s32 DescribeImage(s32 descriptionOut, s32 encodedImage, s32 paletteDescriptionOut) {
    GtlFnTable table;
    GtlDescTable entries;
    s32 handlerIndex;
    entries = s_imageFormatProbeTable;
    handlerIndex = GtlFindIndex(&entries, encodedImage);

    if (handlerIndex == -1) {
        return 0;
    }
    table = s_imageDescriptionHandlers;
    return ((GtlDescFn3)table.fn[handlerIndex])(descriptionOut, paletteDescriptionOut, encodedImage);
}

// Write ImageDescriptionSchema and optional-handler-dependent ImagePaletteSchema;
// both outputs should be valid (GSR requires palette output). Return handler
// status; if no probe matches, return 0 with outputs untouched. Parser failure
// may partially write: inspect the named handler. Callers often supply a FOURTH
// byte-count ABI word; this implementation does not forward/use it. Therefore
// it provides no bounds validation and requires a complete trusted image.
s32 gtl_Image_GetDescription(s32 descriptionOut, s32 paletteDescriptionOut, s32 encodedImage) {
    return DescribeImage(descriptionOut, encodedImage, paletteDescriptionOut);
}
#pragma pop

// Write one Wii pointer word to BORROWED pixel storage inside encodedImage;
// keep the file alive through upload/use. Return handler status, not the pointer.
// No probe -> 0 and untouched output. Require a supported nonnull table entry;
// see the table map above. No size bound or payload validation is added here.
s32 gtl_Image_GetImageData(s32 pixelAddressOut, s32 encodedImage) {
    GtlDescTable entries = s_imageFormatProbeTable;
    GtlFnTable table;
    s32 handlerIndex = GtlFindIndex(&entries, encodedImage);
    GtlImageFn2 getPixels;

    if (handlerIndex == -1) {
        return 0;
    }
    table = s_imagePixelDataHandlers;
    getPixels = (GtlImageFn2)table.fn[handlerIndex];
    return getPixels(pixelAddressOut, encodedImage);
}

// Write one borrowed palette-address word; a supported parser may write null
// and return 1 for a file with no palette. No probe returns 0/untouched output.
// Only TGA and GSR types 1/2/5 have handlers; other recognized types call null.
s32 gtl_Image_GetPaletteData(s32 paletteAddressOut, s32 encodedImage) {
    GtlFnTable table;
    GtlDescTable entries = s_imageFormatProbeTable;
    s32 handlerIndex = GtlFindIndex(&entries, encodedImage);

    if (handlerIndex == -1) {
        return 0;
    }
    table = s_imagePaletteDataHandlers;
    {
        GtlImageFn2 getPalette = (GtlImageFn2)table.fn[handlerIndex];
        return getPalette(paletteAddressOut, encodedImage);
    }
}

// Copy trusted non-overlapping image storage; width/height are pixels and all
// addresses/unused words retain their 32-bit ABI. Layouts 5/6 memcpy a packed
// byte count (width*height*bitsPerPixel >>3). Layout 3 uses CopyLinearRows
// (gtl_image_layout_and_sizing.cpp) with direction=1: linear-to-Morton pixel ordering for ordinary
// formats, packed memcpy for compressed formats; it does NOT flip image rows.
// Layout 7 tail-calls LinearToTiled (gtl_image_decode_and_tiling.cpp).
// Other layouts do nothing. For tiled dimensions/capacity, obey that callee's
// format-specific rules, including padded single-row/column compressed blocks.
// No capacity/error return. Require dimensions/products to fit 32-bit arithmetic.
// Words 1/2/6 are unused by the known callees; do not give them pitch semantics.
// The zero-argument LinearToTiled declaration above is deliberately retained:
// the exact tail branch forwards live ABI registers, without C++ argument moves.
void gtl_Image_CopyToPlatformLayout(s32 sourcePixels, s32 unusedSourceWord1, s32 unusedSourceWord2,
                                    s32 destinationPixels, s32 copyWidth, s32 copyHeight, s32 unusedDestinationWord,
                                    s32 engineFormat, s32 destinationLayout) {
    u32 bitsPerPixel;
    s32 pixelCount;

    switch (destinationLayout) {
    case 7:
        gtl_Image_LinearToTiled();
        return;
    case 5:
        if (engineFormat >= 0x33) {
            goto upper_a;
        }
        if (engineFormat == 0x20) {
            goto dflt_a;
        }
        if (engineFormat >= 0x20) {
            goto mid_a;
        }
        if (engineFormat == -1) {
            goto zero_a;
        }
        if (engineFormat < -1) {
            goto dflt_a;
        }
        if (engineFormat < 0x1F) {
            goto dflt_a;
        }
        goto four_a;
    mid_a:
        if (engineFormat == 0x2E) {
            goto four_b;
        }
        if (engineFormat >= 0x2E) {
            goto eight_a;
        }
        if (engineFormat < 0x22) {
            goto four_a;
        }
        goto dflt_a;
    upper_a:
        if (engineFormat == 0x5B) {
            goto four_a;
        }
        if (engineFormat >= 0x5B) {
            goto upper2_a;
        }
        if (engineFormat != 0x59) {
            goto dflt_a;
        }
        goto four_a;
    upper2_a:
        if (engineFormat == 0x62) {
            goto four_b;
        }
        if (engineFormat < 0x62) {
            goto dflt_a;
        }
        if (engineFormat < 0x67) {
            goto eight_a;
        }
        goto dflt_a;
    four_a:
        bitsPerPixel = 4;
        goto mul_a;
    four_b:
        bitsPerPixel = 4;
        goto mul_a;
    eight_a:
        bitsPerPixel = 8;
        goto mul_a;
    dflt_a:
        bitsPerPixel = gtl_ImageFormat_GetBytesPerElement(engineFormat) * 8;
        goto mul_a;
    zero_a:
        bitsPerPixel = 0;
    mul_a:
        pixelCount = copyHeight * copyWidth;
        memcpy((void*)destinationPixels, (void*)sourcePixels, (u32)(pixelCount * bitsPerPixel) >> 3U);
        return;
    case 3:
        gtl_Image_ConvertMortonLayout(sourcePixels, unusedSourceWord1, unusedSourceWord2, destinationPixels, copyWidth,
                                      copyHeight, unusedDestinationWord, engineFormat, 1);
        return;
    case 6:
        if (engineFormat >= 0x33) {
            goto upper_c;
        }
        if (engineFormat == 0x20) {
            goto dflt_c;
        }
        if (engineFormat >= 0x20) {
            goto mid_c;
        }
        if (engineFormat == -1) {
            goto zero_c;
        }
        if (engineFormat < -1) {
            goto dflt_c;
        }
        if (engineFormat < 0x1F) {
            goto dflt_c;
        }
        goto four_c;
    mid_c:
        if (engineFormat == 0x2E) {
            goto four_d;
        }
        if (engineFormat >= 0x2E) {
            goto eight_c;
        }
        if (engineFormat < 0x22) {
            goto four_c;
        }
        goto dflt_c;
    upper_c:
        if (engineFormat == 0x5B) {
            goto four_c;
        }
        if (engineFormat >= 0x5B) {
            goto upper2_c;
        }
        if (engineFormat != 0x59) {
            goto dflt_c;
        }
        goto four_c;
    upper2_c:
        if (engineFormat == 0x62) {
            goto four_d;
        }
        if (engineFormat < 0x62) {
            goto dflt_c;
        }
        if (engineFormat < 0x67) {
            goto eight_c;
        }
        goto dflt_c;
    four_c:
        bitsPerPixel = 4;
        goto mul_c;
    four_d:
        bitsPerPixel = 4;
        goto mul_c;
    eight_c:
        bitsPerPixel = 8;
        goto mul_c;
    dflt_c:
        bitsPerPixel = gtl_ImageFormat_GetBytesPerElement(engineFormat) * 8;
        goto mul_c;
    zero_c:
        bitsPerPixel = 0;
    mul_c:
        pixelCount = copyHeight * copyWidth;
        memcpy((void*)destinationPixels, (void*)sourcePixels, (u32)(pixelCount * bitsPerPixel) >> 3U);
        return;
    }
}

} // extern "C"
