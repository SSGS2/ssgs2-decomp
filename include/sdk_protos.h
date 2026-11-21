// Real SDK headers first (see include/rvl/README.md); then each function under the CURRENT symbol name with the real
// prototype where the headers have one. The comment above each line gives the SDK name.
#ifndef SDK_PROTOS_H
#define SDK_PROTOS_H

#define AIDSPSampleRate __real_AIDSPSampleRate
#define AIInit __real_AIInit
#define AIInitDMA __real_AIInitDMA
#define AiHwReg __real_AiHwReg
#define C_MTXOrtho __real_C_MTXOrtho
#define DIHwReg __real_DIHwReg
#define DVDAsyncState __real_DVDAsyncState
#define DVDCancel __real_DVDCancel
#define DVDCancelAsync __real_DVDCancelAsync
#define DVDClose __real_DVDClose
#define DVDConvertPathToEntrynum __real_DVDConvertPathToEntrynum
#define DVDCoverState __real_DVDCoverState
#define DVDFastOpen __real_DVDFastOpen
#define DVDGetCommandBlockStatus __real_DVDGetCommandBlockStatus
#define DVDGetDriveStatus __real_DVDGetDriveStatus
#define DVDLowIntType __real_DVDLowIntType
#define DVDOpen __real_DVDOpen
#define DVDQueuePriority __real_DVDQueuePriority
#define DVDReadAsyncPrio __real_DVDReadAsyncPrio
#define DVDReadPrio __real_DVDReadPrio
#define DVDResult __real_DVDResult
#define GXAlphaOp __real_GXAlphaOp
#define GXAnisotropy __real_GXAnisotropy
#define GXAttnFn __real_GXAttnFn
#define GXAttr __real_GXAttr
#define GXAttrType __real_GXAttrType
#define GXBegin __real_GXBegin
#define GXBlendFactor __real_GXBlendFactor
#define GXBlendMode __real_GXBlendMode
#define GXCITexFmt __real_GXCITexFmt
#define GXCallDisplayList __real_GXCallDisplayList
#define GXChannelID __real_GXChannelID
#define GXClearVtxDesc __real_GXClearVtxDesc
#define GXClearZ __real_GXClearZ
#define GXClipMode __real_GXClipMode
#define GXColorSrc __real_GXColorSrc
#define GXCompCnt __real_GXCompCnt
#define GXCompType __real_GXCompType
#define GXCompare __real_GXCompare
#define GXCopyClamp __real_GXCopyClamp
#define GXCopyDisp __real_GXCopyDisp
#define GXCopyTex __real_GXCopyTex
#define GXCullMode __real_GXCullMode
#define GXDiffuseFn __real_GXDiffuseFn
#define GXDirtyFlag __real_GXDirtyFlag
#define GXDisableBreakPt __real_GXDisableBreakPt
#define GXDistAttnFn __real_GXDistAttnFn
#define GXDrawDone __real_GXDrawDone
#define GXEnableBreakPt __real_GXEnableBreakPt
#define GXFifoCmd __real_GXFifoCmd
#define GXFogType __real_GXFogType
#define GXGetCPUFifo __real_GXGetCPUFifo
#define GXGetFifoPtrs __real_GXGetFifoPtrs
#define GXIndTexAlphaSel __real_GXIndTexAlphaSel
#define GXIndTexBiasSel __real_GXIndTexBiasSel
#define GXIndTexFormat __real_GXIndTexFormat
#define GXIndTexMtxID __real_GXIndTexMtxID
#define GXIndTexScale __real_GXIndTexScale
#define GXIndTexStageID __real_GXIndTexStageID
#define GXIndTexWrap __real_GXIndTexWrap
#define GXInit __real_GXInit
#define GXInitLightAttn __real_GXInitLightAttn
#define GXInitLightAttnA __real_GXInitLightAttnA
#define GXInitLightAttnK __real_GXInitLightAttnK
#define GXInitLightColor __real_GXInitLightColor
#define GXInitLightDir __real_GXInitLightDir
#define GXInitLightDistAttn __real_GXInitLightDistAttn
#define GXInitLightPos __real_GXInitLightPos
#define GXInitLightSpot __real_GXInitLightSpot
#define GXInitSpecularDir __real_GXInitSpecularDir
#define GXInitTexObj __real_GXInitTexObj
#define GXInitTexObjCI __real_GXInitTexObjCI
#define GXInitTexObjLOD __real_GXInitTexObjLOD
#define GXInitTlutObj __real_GXInitTlutObj
#define GXInterlaceField __real_GXInterlaceField
#define GXInvalidateTexAll __real_GXInvalidateTexAll
#define GXInvalidateVtxCache __real_GXInvalidateVtxCache
#define GXLightID __real_GXLightID
#define GXLoadLightObjImm __real_GXLoadLightObjImm
#define GXLoadNrmMtxImm __real_GXLoadNrmMtxImm
#define GXLoadPosMtxImm __real_GXLoadPosMtxImm
#define GXLoadTexMtxImm __real_GXLoadTexMtxImm
#define GXLoadTexObj __real_GXLoadTexObj
#define GXLoadTlut __real_GXLoadTlut
#define GXLogicOp __real_GXLogicOp
#define GXMtxType __real_GXMtxType
#define GXPTTexMtx __real_GXPTTexMtx
#define GXPixModeSync __real_GXPixModeSync
#define GXPixelFmt __real_GXPixelFmt
#define GXPosNrmMtx __real_GXPosNrmMtx
#define GXPrimitive __real_GXPrimitive
#define GXProjectionType __real_GXProjectionType
#define GXRasChannelID __real_GXRasChannelID
#define GXSetAlphaCompare __real_GXSetAlphaCompare
#define GXSetAlphaUpdate __real_GXSetAlphaUpdate
#define GXSetArray __real_GXSetArray
#define GXSetBlendMode __real_GXSetBlendMode
#define GXSetCPUFifo __real_GXSetCPUFifo
#define GXSetChanAmbColor __real_GXSetChanAmbColor
#define GXSetChanCtrl __real_GXSetChanCtrl
#define GXSetChanMatColor __real_GXSetChanMatColor
#define GXSetColorUpdate __real_GXSetColorUpdate
#define GXSetCopyClear __real_GXSetCopyClear
#define GXSetCopyFilter __real_GXSetCopyFilter
#define GXSetCullMode __real_GXSetCullMode
#define GXSetCurrentMtx __real_GXSetCurrentMtx
#define GXSetDispCopyDst __real_GXSetDispCopyDst
#define GXSetDispCopyGamma __real_GXSetDispCopyGamma
#define GXSetDispCopySrc __real_GXSetDispCopySrc
#define GXSetDispCopyYScale __real_GXSetDispCopyYScale
#define GXSetDrawSync __real_GXSetDrawSync
#define GXSetFog __real_GXSetFog
#define GXSetIndTexCoordScale __real_GXSetIndTexCoordScale
#define GXSetIndTexMtx __real_GXSetIndTexMtx
#define GXSetIndTexOrder __real_GXSetIndTexOrder
#define GXSetMisc __real_GXSetMisc
#define GXSetNumChans __real_GXSetNumChans
#define GXSetNumIndStages __real_GXSetNumIndStages
#define GXSetNumTevStages __real_GXSetNumTevStages
#define GXSetNumTexGens __real_GXSetNumTexGens
#define GXSetPixelFmt __real_GXSetPixelFmt
#define GXSetPointSize __real_GXSetPointSize
#define GXSetProjection __real_GXSetProjection
#define GXSetScissorBoxOffset __real_GXSetScissorBoxOffset
#define GXSetTevAlphaIn __real_GXSetTevAlphaIn
#define GXSetTevAlphaOp __real_GXSetTevAlphaOp
#define GXSetTevColor __real_GXSetTevColor
#define GXSetTevColorIn __real_GXSetTevColorIn
#define GXSetTevColorOp __real_GXSetTevColorOp
#define GXSetTevColorS10 __real_GXSetTevColorS10
#define GXSetTevDirect __real_GXSetTevDirect
#define GXSetTevKAlphaSel __real_GXSetTevKAlphaSel
#define GXSetTevKColor __real_GXSetTevKColor
#define GXSetTevKColorSel __real_GXSetTevKColorSel
#define GXSetTevOp __real_GXSetTevOp
#define GXSetTevOrder __real_GXSetTevOrder
#define GXSetTevSwapMode __real_GXSetTevSwapMode
#define GXSetTevSwapModeTable __real_GXSetTevSwapModeTable
#define GXSetTexCoordGen2 __real_GXSetTexCoordGen2
#define GXSetTexCopyDst __real_GXSetTexCopyDst
#define GXSetTexCopySrc __real_GXSetTexCopySrc
#define GXSetViewportJitter __real_GXSetViewportJitter
#define GXSetVtxAttrFmt __real_GXSetVtxAttrFmt
#define GXSetVtxDesc __real_GXSetVtxDesc
#define GXSetZCompLoc __real_GXSetZCompLoc
#define GXSetZMode __real_GXSetZMode
#define GXSetZScaleOffset __real_GXSetZScaleOffset
#define GXSetZTexture __real_GXSetZTexture
#define GXSpotFn __real_GXSpotFn
#define GXTevAlphaArg __real_GXTevAlphaArg
#define GXTevBias __real_GXTevBias
#define GXTevColorArg __real_GXTevColorArg
#define GXTevColorChan __real_GXTevColorChan
#define GXTevKAlphaSel __real_GXTevKAlphaSel
#define GXTevKColorID __real_GXTevKColorID
#define GXTevKColorSel __real_GXTevKColorSel
#define GXTevMode __real_GXTevMode
#define GXTevOp __real_GXTevOp
#define GXTevRegID __real_GXTevRegID
#define GXTevRegType __real_GXTevRegType
#define GXTevScale __real_GXTevScale
#define GXTevStageID __real_GXTevStageID
#define GXTevSwapSel __real_GXTevSwapSel
#define GXTexCacheSize __real_GXTexCacheSize
#define GXTexCoordID __real_GXTexCoordID
#define GXTexFilter __real_GXTexFilter
#define GXTexFmt __real_GXTexFmt
#define GXTexGenSrc __real_GXTexGenSrc
#define GXTexGenType __real_GXTexGenType
#define GXTexMapID __real_GXTexMapID
#define GXTexMtx __real_GXTexMtx
#define GXTexWrapMode __real_GXTexWrapMode
#define GXTlut __real_GXTlut
#define GXTlutFmt __real_GXTlutFmt
#define GXTlutSize __real_GXTlutSize
#define GXVtxFmt __real_GXVtxFmt
#define GXXfMem __real_GXXfMem
#define GXXfTexForm __real_GXXfTexForm
#define GXXfTexGen __real_GXXfTexGen
#define GXXfTexProj __real_GXXfTexProj
#define GXZFmt16 __real_GXZFmt16
#define GXZTexOp __real_GXZTexOp
#define GX_BP_REG __real_GX_BP_REG
#define GX_CP_REG __real_GX_CP_REG
#define GX_XF_REG __real_GX_XF_REG
#define LCEnable __real_LCEnable
#define MIHwReg __real_MIHwReg
#define OSAppType __real_OSAppType
#define OSBootMagic __real_OSBootMagic
#define OSCancelAlarm __real_OSCancelAlarm
#define OSConsoleType __real_OSConsoleType
#define OSContextState __real_OSContextState
#define OSCreateThread __real_OSCreateThread
#define OSErrorType __real_OSErrorType
#define OSExceptionType __real_OSExceptionType
#define OSFontEncode __real_OSFontEncode
#define OSGetCurrentThread __real_OSGetCurrentThread
#define OSInitMessageQueue __real_OSInitMessageQueue
#define OSInitMutex __real_OSInitMutex
#define OSInterruptType __real_OSInterruptType
#define OSIsThreadTerminated __real_OSIsThreadTerminated
#define OSJamMessage __real_OSJamMessage
#define OSJoinThread __real_OSJoinThread
#define OSLockMutex __real_OSLockMutex
#define OSMessageFlags __real_OSMessageFlags
#define OSReceiveMessage __real_OSReceiveMessage
#define OSReport __real_OSReport
#define OSResumeThread __real_OSResumeThread
#define OSSendMessage __real_OSSendMessage
#define OSSetAlarm __real_OSSetAlarm
#define OSSetPeriodicAlarm __real_OSSetPeriodicAlarm
#define OSSetPowerCallback __real_OSSetPowerCallback
#define OSSetResetCallback __real_OSSetResetCallback
#define OSSetThreadPriority __real_OSSetThreadPriority
#define OSShutdownEvent __real_OSShutdownEvent
#define OSShutdownSystem __real_OSShutdownSystem
#define OSSleepTicks __real_OSSleepTicks
#define OSSuspendThread __real_OSSuspendThread
#define OSThreadFlag __real_OSThreadFlag
#define OSThreadState __real_OSThreadState
#define OSTicksToCalendarTime __real_OSTicksToCalendarTime
#define OSUnlockMutex __real_OSUnlockMutex
#define OSWakeupThread __real_OSWakeupThread
#define PIHwReg __real_PIHwReg
#define PSMTXConcat __real_PSMTXConcat
#define PSMTXIdentity __real_PSMTXIdentity
#define PSMTXInvXpose __real_PSMTXInvXpose
#define VIClkSpeed __real_VIClkSpeed
#define VIEnableDimming __real_VIEnableDimming
#define VIFlush __real_VIFlush
#define VIGetDTVStatus __real_VIGetDTVStatus
#define VIGetScanMode __real_VIGetScanMode
#define VIGetTvFormat __real_VIGetTvFormat
#define VIHwReg __real_VIHwReg
#define VIInit __real_VIInit
#define VIScanMode __real_VIScanMode
#define VISetBlack __real_VISetBlack
#define VISetNextFrameBuffer __real_VISetNextFrameBuffer
#define VITVFormat __real_VITVFormat
#define VITVMode __real_VITVMode
#define VIWaitForRetrace __real_VIWaitForRetrace
#define VIXFBMode __real_VIXFBMode
// Types for include/sdk_protos.h: the real RVL SDK headers (doldecomp/ogws, CC0-1.0; see include/rvl/README.md),
// then the few names the project still needs that those headers do not declare.
// Enum typedefs from these headers are renamed __real_X while they are read and given back as `typedef int X;`
#include <revolution/OS.h>
#include <revolution/GX.h>
#include <revolution/MTX.h>
#include <revolution/VI.h>
#include <revolution/DVD.h>
#include <revolution/AI.h>

// ogws spells these s64 / u32; the project's prototypes use the SDK's usual aliases.
typedef s64 OSTime;
typedef u32 OSTick;

// A name the project used before the real headers existed (the SDK spells it GXMtxType).
typedef int GXTexMtxType;

// Real functions that some source files declare differently (read as __real_NAME, so both can coexist):
// hide-real: GXInit GXSetCPUFifo GXSetMisc GXSetViewportJitter OSReport
// Library names retain the project's existing parameter types during the SDK includes.
// hide-real: AIInit AIInitDMA C_MTXOrtho DVDCancel DVDCancelAsync DVDClose
// hide-real: DVDConvertPathToEntrynum DVDFastOpen DVDGetCommandBlockStatus DVDGetDriveStatus DVDOpen DVDReadAsyncPrio
// hide-real: DVDReadPrio GXBegin GXCallDisplayList GXClearVtxDesc GXCopyDisp GXCopyTex
// hide-real: GXDisableBreakPt GXDrawDone GXEnableBreakPt GXGetCPUFifo GXGetFifoPtrs GXInitLightAttn
// hide-real: GXInitLightAttnA GXInitLightAttnK GXInitLightColor GXInitLightDir GXInitLightDistAttn GXInitLightPos
// hide-real: GXInitLightSpot GXInitSpecularDir GXInitTexObj GXInitTexObjCI GXInitTexObjLOD GXInitTlutObj
// hide-real: GXInvalidateTexAll GXInvalidateVtxCache GXLoadLightObjImm GXLoadNrmMtxImm GXLoadPosMtxImm GXLoadTexMtxImm
// hide-real: GXLoadTexObj GXLoadTlut GXPixModeSync GXSetAlphaCompare GXSetAlphaUpdate GXSetArray
// hide-real: GXSetBlendMode GXSetChanAmbColor GXSetChanCtrl GXSetChanMatColor GXSetColorUpdate GXSetCopyClear
// hide-real: GXSetCopyFilter GXSetCullMode GXSetCurrentMtx GXSetDispCopyDst GXSetDispCopyGamma GXSetDispCopySrc
// hide-real: GXSetDispCopyYScale GXSetDrawSync GXSetFog GXSetIndTexCoordScale GXSetIndTexMtx GXSetIndTexOrder
// hide-real: GXSetNumChans GXSetNumIndStages GXSetNumTevStages GXSetNumTexGens GXSetPixelFmt GXSetPointSize
// hide-real: GXSetScissorBoxOffset GXSetTevAlphaIn GXSetTevAlphaOp GXSetTevColor GXSetTevColorIn GXSetTevColorOp
// hide-real: GXSetTevColorS10 GXSetTevDirect GXSetTevKAlphaSel GXSetTevKColor GXSetTevKColorSel GXSetTevOp
// hide-real: GXSetTevOrder GXSetTevSwapMode GXSetTevSwapModeTable GXSetTexCoordGen2 GXSetTexCopyDst GXSetTexCopySrc
// hide-real: GXSetVtxAttrFmt GXSetVtxDesc GXSetZCompLoc GXSetZMode GXSetZScaleOffset GXSetZTexture
// hide-real: LCEnable OSCancelAlarm OSCreateThread OSGetCurrentThread OSInitMessageQueue OSInitMutex
// hide-real: OSIsThreadTerminated OSJamMessage OSJoinThread OSLockMutex OSReceiveMessage OSResumeThread
// hide-real: OSSendMessage OSSetAlarm OSSetPeriodicAlarm OSSetPowerCallback OSSetResetCallback OSSetThreadPriority
// hide-real: OSShutdownSystem OSSleepTicks OSSuspendThread OSTicksToCalendarTime OSUnlockMutex OSWakeupThread
// hide-real: PSMTXConcat PSMTXIdentity PSMTXInvXpose VIEnableDimming VIFlush VIGetDTVStatus
// hide-real: VIGetScanMode VIInit VISetBlack VISetNextFrameBuffer VIWaitForRetrace

#undef AIDSPSampleRate
#undef AIInit
#undef AIInitDMA
#undef AiHwReg
#undef C_MTXOrtho
#undef DIHwReg
#undef DVDAsyncState
#undef DVDCancel
#undef DVDCancelAsync
#undef DVDClose
#undef DVDConvertPathToEntrynum
#undef DVDCoverState
#undef DVDFastOpen
#undef DVDGetCommandBlockStatus
#undef DVDGetDriveStatus
#undef DVDLowIntType
#undef DVDOpen
#undef DVDQueuePriority
#undef DVDReadAsyncPrio
#undef DVDReadPrio
#undef DVDResult
#undef GXAlphaOp
#undef GXAnisotropy
#undef GXAttnFn
#undef GXAttr
#undef GXAttrType
#undef GXBegin
#undef GXBlendFactor
#undef GXBlendMode
#undef GXCITexFmt
#undef GXCallDisplayList
#undef GXChannelID
#undef GXClearVtxDesc
#undef GXClearZ
#undef GXClipMode
#undef GXColorSrc
#undef GXCompCnt
#undef GXCompType
#undef GXCompare
#undef GXCopyClamp
#undef GXCopyDisp
#undef GXCopyTex
#undef GXCullMode
#undef GXDiffuseFn
#undef GXDirtyFlag
#undef GXDisableBreakPt
#undef GXDistAttnFn
#undef GXDrawDone
#undef GXEnableBreakPt
#undef GXFifoCmd
#undef GXFogType
#undef GXGetCPUFifo
#undef GXGetFifoPtrs
#undef GXIndTexAlphaSel
#undef GXIndTexBiasSel
#undef GXIndTexFormat
#undef GXIndTexMtxID
#undef GXIndTexScale
#undef GXIndTexStageID
#undef GXIndTexWrap
#undef GXInit
#undef GXInitLightAttn
#undef GXInitLightAttnA
#undef GXInitLightAttnK
#undef GXInitLightColor
#undef GXInitLightDir
#undef GXInitLightDistAttn
#undef GXInitLightPos
#undef GXInitLightSpot
#undef GXInitSpecularDir
#undef GXInitTexObj
#undef GXInitTexObjCI
#undef GXInitTexObjLOD
#undef GXInitTlutObj
#undef GXInterlaceField
#undef GXInvalidateTexAll
#undef GXInvalidateVtxCache
#undef GXLightID
#undef GXLoadLightObjImm
#undef GXLoadNrmMtxImm
#undef GXLoadPosMtxImm
#undef GXLoadTexMtxImm
#undef GXLoadTexObj
#undef GXLoadTlut
#undef GXLogicOp
#undef GXMtxType
#undef GXPTTexMtx
#undef GXPixModeSync
#undef GXPixelFmt
#undef GXPosNrmMtx
#undef GXPrimitive
#undef GXProjectionType
#undef GXRasChannelID
#undef GXSetAlphaCompare
#undef GXSetAlphaUpdate
#undef GXSetArray
#undef GXSetBlendMode
#undef GXSetCPUFifo
#undef GXSetChanAmbColor
#undef GXSetChanCtrl
#undef GXSetChanMatColor
#undef GXSetColorUpdate
#undef GXSetCopyClear
#undef GXSetCopyFilter
#undef GXSetCullMode
#undef GXSetCurrentMtx
#undef GXSetDispCopyDst
#undef GXSetDispCopyGamma
#undef GXSetDispCopySrc
#undef GXSetDispCopyYScale
#undef GXSetDrawSync
#undef GXSetFog
#undef GXSetIndTexCoordScale
#undef GXSetIndTexMtx
#undef GXSetIndTexOrder
#undef GXSetMisc
#undef GXSetNumChans
#undef GXSetNumIndStages
#undef GXSetNumTevStages
#undef GXSetNumTexGens
#undef GXSetPixelFmt
#undef GXSetPointSize
#undef GXSetProjection
#undef GXSetScissorBoxOffset
#undef GXSetTevAlphaIn
#undef GXSetTevAlphaOp
#undef GXSetTevColor
#undef GXSetTevColorIn
#undef GXSetTevColorOp
#undef GXSetTevColorS10
#undef GXSetTevDirect
#undef GXSetTevKAlphaSel
#undef GXSetTevKColor
#undef GXSetTevKColorSel
#undef GXSetTevOp
#undef GXSetTevOrder
#undef GXSetTevSwapMode
#undef GXSetTevSwapModeTable
#undef GXSetTexCoordGen2
#undef GXSetTexCopyDst
#undef GXSetTexCopySrc
#undef GXSetViewportJitter
#undef GXSetVtxAttrFmt
#undef GXSetVtxDesc
#undef GXSetZCompLoc
#undef GXSetZMode
#undef GXSetZScaleOffset
#undef GXSetZTexture
#undef GXSpotFn
#undef GXTevAlphaArg
#undef GXTevBias
#undef GXTevColorArg
#undef GXTevColorChan
#undef GXTevKAlphaSel
#undef GXTevKColorID
#undef GXTevKColorSel
#undef GXTevMode
#undef GXTevOp
#undef GXTevRegID
#undef GXTevRegType
#undef GXTevScale
#undef GXTevStageID
#undef GXTevSwapSel
#undef GXTexCacheSize
#undef GXTexCoordID
#undef GXTexFilter
#undef GXTexFmt
#undef GXTexGenSrc
#undef GXTexGenType
#undef GXTexMapID
#undef GXTexMtx
#undef GXTexWrapMode
#undef GXTlut
#undef GXTlutFmt
#undef GXTlutSize
#undef GXVtxFmt
#undef GXXfMem
#undef GXXfTexForm
#undef GXXfTexGen
#undef GXXfTexProj
#undef GXZFmt16
#undef GXZTexOp
#undef GX_BP_REG
#undef GX_CP_REG
#undef GX_XF_REG
#undef LCEnable
#undef MIHwReg
#undef OSAppType
#undef OSBootMagic
#undef OSCancelAlarm
#undef OSConsoleType
#undef OSContextState
#undef OSCreateThread
#undef OSErrorType
#undef OSExceptionType
#undef OSFontEncode
#undef OSGetCurrentThread
#undef OSInitMessageQueue
#undef OSInitMutex
#undef OSInterruptType
#undef OSIsThreadTerminated
#undef OSJamMessage
#undef OSJoinThread
#undef OSLockMutex
#undef OSMessageFlags
#undef OSReceiveMessage
#undef OSReport
#undef OSResumeThread
#undef OSSendMessage
#undef OSSetAlarm
#undef OSSetPeriodicAlarm
#undef OSSetPowerCallback
#undef OSSetResetCallback
#undef OSSetThreadPriority
#undef OSShutdownEvent
#undef OSShutdownSystem
#undef OSSleepTicks
#undef OSSuspendThread
#undef OSThreadFlag
#undef OSThreadState
#undef OSTicksToCalendarTime
#undef OSUnlockMutex
#undef OSWakeupThread
#undef PIHwReg
#undef PSMTXConcat
#undef PSMTXIdentity
#undef PSMTXInvXpose
#undef VIClkSpeed
#undef VIEnableDimming
#undef VIFlush
#undef VIGetDTVStatus
#undef VIGetScanMode
#undef VIGetTvFormat
#undef VIHwReg
#undef VIInit
#undef VIScanMode
#undef VISetBlack
#undef VISetNextFrameBuffer
#undef VITVFormat
#undef VITVMode
#undef VIWaitForRetrace
#undef VIXFBMode
typedef int AIDSPSampleRate;
typedef int AiHwReg;
typedef int DIHwReg;
typedef int DVDAsyncState;
typedef int DVDCoverState;
typedef int DVDLowIntType;
typedef int DVDQueuePriority;
typedef int DVDResult;
typedef int GXAlphaOp;
typedef int GXAnisotropy;
typedef int GXAttnFn;
typedef int GXAttr;
typedef int GXAttrType;
typedef int GXBlendFactor;
typedef int GXBlendMode;
typedef int GXCITexFmt;
typedef int GXChannelID;
typedef int GXClearZ;
typedef int GXClipMode;
typedef int GXColorSrc;
typedef int GXCompCnt;
typedef int GXCompType;
typedef int GXCompare;
typedef int GXCopyClamp;
typedef int GXCullMode;
typedef int GXDiffuseFn;
typedef int GXDirtyFlag;
typedef int GXDistAttnFn;
typedef int GXFifoCmd;
typedef int GXFogType;
typedef int GXIndTexAlphaSel;
typedef int GXIndTexBiasSel;
typedef int GXIndTexFormat;
typedef int GXIndTexMtxID;
typedef int GXIndTexScale;
typedef int GXIndTexStageID;
typedef int GXIndTexWrap;
typedef int GXInterlaceField;
typedef int GXLightID;
typedef int GXLogicOp;
typedef int GXMtxType;
typedef int GXPTTexMtx;
typedef int GXPixelFmt;
typedef int GXPosNrmMtx;
typedef int GXPrimitive;
typedef int GXProjectionType;
typedef int GXRasChannelID;
typedef int GXSpotFn;
typedef int GXTevAlphaArg;
typedef int GXTevBias;
typedef int GXTevColorArg;
typedef int GXTevColorChan;
typedef int GXTevKAlphaSel;
typedef int GXTevKColorID;
typedef int GXTevKColorSel;
typedef int GXTevMode;
typedef int GXTevOp;
typedef int GXTevRegID;
typedef int GXTevRegType;
typedef int GXTevScale;
typedef int GXTevStageID;
typedef int GXTevSwapSel;
typedef int GXTexCacheSize;
typedef int GXTexCoordID;
typedef int GXTexFilter;
typedef int GXTexFmt;
typedef int GXTexGenSrc;
typedef int GXTexGenType;
typedef int GXTexMapID;
typedef int GXTexMtx;
typedef int GXTexWrapMode;
typedef int GXTlut;
typedef int GXTlutFmt;
typedef int GXTlutSize;
typedef int GXVtxFmt;
typedef int GXXfMem;
typedef int GXXfTexForm;
typedef int GXXfTexGen;
typedef int GXXfTexProj;
typedef int GXZFmt16;
typedef int GXZTexOp;
typedef int GX_BP_REG;
typedef int GX_CP_REG;
typedef int GX_XF_REG;
typedef int MIHwReg;
typedef int OSAppType;
typedef int OSBootMagic;
typedef int OSConsoleType;
typedef int OSContextState;
typedef int OSErrorType;
typedef int OSExceptionType;
typedef int OSFontEncode;
typedef int OSInterruptType;
typedef int OSMessageFlags;
typedef int OSShutdownEvent;
typedef int OSThreadFlag;
typedef int OSThreadState;
typedef int PIHwReg;
typedef int VIClkSpeed;
typedef int VIHwReg;
typedef int VIScanMode;
typedef int VITVFormat;
typedef int VITVMode;
typedef int VIXFBMode;

#ifdef __cplusplus
extern "C" {
#endif

// OSInit @ 0x8033D7B8: declared by <revolution/OS/OS.h>
// OSSetAlarm @ 0x8033E314: prototype from <revolution/OS/OSAlarm.h>
void OSSetAlarm(OSAlarm* alarm, s64 tick, OSAlarmHandler handler);
// DCInvalidateRange @ 0x8033EFB8: declared by <revolution/OS/OSCache.h>
// DCFlushRange @ 0x8033EFE4: declared by <revolution/OS/OSCache.h>
// OSDisableInterrupts @ 0x80343090: declared by <revolution/OS/OSInterrupt.h>
// OSEnableInterrupts @ 0x803430A4: declared by <revolution/OS/OSInterrupt.h>
// OSRestoreInterrupts @ 0x803430B8: declared by <revolution/OS/OSInterrupt.h>
// OSInitMessageQueue @ 0x8034382C: prototype from <revolution/OS/OSMessage.h>
void OSInitMessageQueue(OSMessageQueue* queue, OSMessage* buffer, s32 capacity);
// OSSendMessage @ 0x8034388C: prototype from <revolution/OS/OSMessage.h>
BOOL OSSendMessage(OSMessageQueue* queue, OSMessage mesg, u32 flags);
// OSReceiveMessage @ 0x80343954: prototype from <revolution/OS/OSMessage.h>
BOOL OSReceiveMessage(OSMessageQueue* queue, OSMessage* mesg, u32 flags);
// OSInitMutex @ 0x80344314: prototype from <revolution/OS/OSMutex.h>
void OSInitMutex(OSMutex* mutex);
// OSLockMutex @ 0x8034434C: prototype from <revolution/OS/OSMutex.h>
void OSLockMutex(OSMutex* mutex);
// OSUnlockMutex @ 0x80344428: prototype from <revolution/OS/OSMutex.h>
void OSUnlockMutex(OSMutex* mutex);
// OSReturnToMenu @ 0x80344C90: declared by <revolution/OS/OSReset.h>
// OSResetSystem @ 0x80344E00: declared by <revolution/OS/OSReset.h>
// OSGetCurrentThread @ 0x80345BC0: prototype from <revolution/OS/OSThread.h>
OSThread*OSGetCurrentThread(void);
// OSCreateThread @ 0x80346158: prototype from <revolution/OS/OSThread.h>
BOOL OSCreateThread(OSThread* thread, OSThreadFunc func, void* funcArg, void* stackBegin, u32 stackSize, s32 prio, u16 flags);
// OSSleepThread @ 0x80346BEC: declared by <revolution/OS/OSThread.h>
// OSGetTime @ 0x80347788: declared by <revolution/OS/OSTime.h>
// OSGetTick @ 0x803477A0: declared by <revolution/OS/OSTime.h>
// PSMTXIdentity @ 0x803496C8: prototype from <revolution/MTX/mtx.h>
void PSMTXIdentity(Mtx);
// PSMTXConcat @ 0x80349728: prototype from <revolution/MTX/mtx.h>
void PSMTXConcat(const Mtx, const Mtx, Mtx);
// DVDGetDriveStatus @ 0x8034E654: prototype from <revolution/DVD/dvd.h>
s32 DVDGetDriveStatus(void);
// DVDSetAutoFatalMessaging @ 0x8034F720: declared by <revolution/DVD/dvdfatal.h>
// VIWaitForRetrace @ 0x80352420: prototype from <revolution/VI/vi.h>
void VIWaitForRetrace(void);
// VIConfigure @ 0x803528BC: declared by <revolution/VI/vi.h>
// VIGetTvFormat @ 0x803536D0: prototype from <revolution/VI/vi.h>
VITVFormat VIGetTvFormat(void);
// GXSetVtxDesc @ 0x8036097C: prototype from <revolution/GX/GXAttr.h>
void GXSetVtxDesc(GXAttr name, GXAttrType type);
// GXSetVtxAttrFmt @ 0x80360DF0: prototype from <revolution/GX/GXAttr.h>
void GXSetVtxAttrFmt(GXVtxFmt fmt, GXAttr attr, GXCompCnt compCnt, GXCompType compType, u8 shift);
// GXSetTexCoordGen2 @ 0x80361224: prototype from <revolution/GX/GXAttr.h>
void GXSetTexCoordGen2(GXTexCoordID id, GXTexGenType type, GXTexGenSrc src, u32 texMtxIdx, GXBool normalize, u32 dualTexMtxIdx);
// GXSetNumTexGens @ 0x8036144C: prototype from <revolution/GX/GXAttr.h>
void GXSetNumTexGens(u8 num);
// GXSetNumChans @ 0x80363140: prototype from <revolution/GX/GXLight.h>
void GXSetNumChans(u8 num);
// GXSetChanCtrl @ 0x80363164: prototype from <revolution/GX/GXLight.h>
void GXSetChanCtrl(GXChannelID chan, GXBool enable, GXColorSrc ambSrc, GXColorSrc matSrc, GXLightID lightMask, GXDiffuseFn diffFn, GXAttnFn attnFn);
// GXInitTexObj @ 0x803632C8: prototype from <revolution/GX/GXTexture.h>
void GXInitTexObj(GXTexObj* obj, void* image, u16 w, u16 h, GXTexFmt fmt, GXTexWrapMode wrap_s, GXTexWrapMode wrap_t, GXBool mipmap);
// GXLoadTexObj @ 0x80363804: prototype from <revolution/GX/GXTexture.h>
void GXLoadTexObj(const GXTexObj*, GXTexMapID);
// GXSetTevColorIn @ 0x80364590: prototype from <revolution/GX/GXTev.h>
void GXSetTevColorIn(GXTevStageID, GXTevColorArg, GXTevColorArg, GXTevColorArg, GXTevColorArg);
// GXSetTevAlphaIn @ 0x803645D0: prototype from <revolution/GX/GXTev.h>
void GXSetTevAlphaIn(GXTevStageID, GXTevAlphaArg, GXTevAlphaArg, GXTevAlphaArg, GXTevAlphaArg);
// GXSetTevColorOp @ 0x80364610: prototype from <revolution/GX/GXTev.h>
void GXSetTevColorOp(GXTevStageID, GXTevOp, GXTevBias, GXTevScale, GXBool, GXTevRegID);
// GXSetTevAlphaOp @ 0x80364668: prototype from <revolution/GX/GXTev.h>
void GXSetTevAlphaOp(GXTevStageID, GXTevOp, GXTevBias, GXTevScale, GXBool, GXTevRegID);
// GXSetTevSwapModeTable @ 0x803648BC: prototype from <revolution/GX/GXTev.h>
void GXSetTevSwapModeTable(GXTevSwapSel, GXTevColorChan, GXTevColorChan, GXTevColorChan, GXTevColorChan);
// GXSetAlphaCompare @ 0x80364938: prototype from <revolution/GX/GXTev.h>
void GXSetAlphaCompare(GXCompare, u8, GXAlphaOp, GXCompare, u8);
// GXSetTevOrder @ 0x803649FC: prototype from <revolution/GX/GXTev.h>
void GXSetTevOrder(GXTevStageID, GXTexCoordID, GXTexMapID, GXChannelID);
// GXSetNumTevStages @ 0x80364B58: prototype from <revolution/GX/GXTev.h>
void GXSetNumTevStages(u8);
// GXSetBlendMode @ 0x80364ECC: prototype from <revolution/GX/GXPixel.h>
void GXSetBlendMode(GXBlendMode mode, GXBlendFactor src, GXBlendFactor dst, GXLogicOp op);
// GXSetZMode @ 0x80364F74: prototype from <revolution/GX/GXPixel.h>
void GXSetZMode(GXBool enableTest, GXCompare func, GXBool enableUpdate);
// GXSetProjection @ 0x80365238: prototype from <revolution/GX/GXTransform.h>
void GXSetProjection(const Mtx44 proj, GXProjectionType type);
// GXLoadPosMtxImm @ 0x803652E8: prototype from <revolution/GX/GXTransform.h>
void GXLoadPosMtxImm(const Mtx mtx, u32 id);
// GXLoadNrmMtxImm @ 0x80365338: prototype from <revolution/GX/GXTransform.h>
void GXLoadNrmMtxImm(const Mtx mtx, u32 id);
// GXSetCurrentMtx @ 0x80365390: prototype from <revolution/GX/GXTransform.h>
void GXSetCurrentMtx(u32 id);
// GXLoadTexMtxImm @ 0x803653B0: prototype from <revolution/GX/GXTransform.h>
void GXLoadTexMtxImm(const Mtx mtx, u32 id, GXMtxType type);
// GXSetViewport @ 0x80365530: declared by <revolution/GX/GXTransform.h>
// GXSetScissor @ 0x8036558C: declared by <revolution/GX/GXTransform.h>
// GXSetScissorBoxOffset @ 0x803655F4: prototype from <revolution/GX/GXTransform.h>
void GXSetScissorBoxOffset(u32 ox, u32 oy);
// strlen @ 0x80365F1C
size_t strlen(const char* s);
// __register_global_object @ 0x80366000
void* __register_global_object(void* object, void* destructor, void* regmem);
// __construct_new_array @ 0x80366308
void* __construct_new_array(void* block, void* ctor, void* dtor, size_t size, size_t n);
// __construct_array @ 0x803664C8
void fn_803664C8(void* ptr, void* ctor, void* dtor, size_t size, size_t n);
// __ptmf_scall @ 0x803666E8
void __ptmf_scall(...);
// __cvt_fp2unsigned @ 0x803669DC
unsigned long Runtime_ConvertDoubleToUnsigned(double x);
// __unexpected @ 0x8036803C
void __unexpected(...);
// __throw @ 0x80368738
void __throw(const char* typeinfo, void* object, void* dtor);
// abs @ 0x8036B3EC
int abs(int x);
// memmove @ 0x8036CB50
void* memmove(void* dst, const void* src, size_t n);
// memcmp @ 0x8036CC74
int memcmp(const void* a, const void* b, size_t n);
// __fpclassifyf @ 0x8036CF90
int __fpclassifyf(float x);
// __fpclassifyd @ 0x8036D00C
int __fpclassifyd(double x);
// sprintf @ 0x8036F3E4
int sprintf(char* s, const char* fmt, ...);
// qsort @ 0x8036F4B4
void qsort(void* base, size_t n, size_t size, int (*cmp)(const void*, const void*));
// rand @ 0x8036F624
int rand(void);
// sscanf @ 0x80370AF0
int sscanf(const char* s, const char* fmt, ...);
// strcpy @ 0x80370C68
char* strcpy(char* dst, const char* src);
// strncpy @ 0x80370D28
char* strncpy(char* dst, const char* src, size_t n);
// strcat @ 0x80370D6C
char* strcat(char* dst, const char* src);
// strncat @ 0x80370D98
char* strncat(char* dst, const char* src, size_t n);
// strcmp @ 0x80370DE4
int strcmp(const char* a, const char* b);
// strncmp @ 0x80370F00
int strncmp(const char* a, const char* b, size_t n);
// strchr @ 0x80370F40
char* strchr(const char* s, int c);
// strrchr @ 0x80370F70
char* strrchr(const char* s, int c);
// strstr @ 0x80370FB8
char* strstr(const char* s, const char* needle);
// wmemset @ 0x80373EB4
wchar_t* wmemset(wchar_t* d, wchar_t c, size_t n);
// wmemcmp @ 0x80373F14
int wmemcmp(const wchar_t* a, const wchar_t* b, size_t n);
// wcslen @ 0x803753DC
size_t wcslen(const wchar_t* s);
// wcscpy @ 0x803753F8
wchar_t* wcscpy(wchar_t* d, const wchar_t* s);
// wcsncpy @ 0x80375414
wchar_t* wcsncpy(wchar_t* d, const wchar_t* s, size_t n);
// wcscmp @ 0x80375458
int wcscmp(const wchar_t* a, const wchar_t* b);
// __ieee754_atan2 @ 0x80375F40
double __ieee754_atan2(double y, double x);
// __kernel_cos @ 0x80377158
double __kernel_cos(double x, double y);
// __kernel_sin @ 0x80378974
double __kernel_sin(double x, double y, int iy);
// atan @ 0x80378C8C
double atan(double x);
// ceil @ 0x80378ECC
double ceil(double x);
// cos @ 0x80379038
double cos(double x);
// floor @ 0x8037910C
double floor(double x);
// sin @ 0x80379444
double sin(double x);
// tan @ 0x8037951C
double tan(double x);
// __ieee754_sqrt @ 0x803795A8
double __ieee754_sqrt(double x);
// Engine_memcpy @ 0x8042104C
void* Memory_Copy(void* dst, const void* src, size_t n);
// Engine_ZeroMemory @ 0x80421064
void* Memory_Clear(void* p, size_t n);

#ifdef __cplusplus
}
#endif

#endif  // SDK_PROTOS_H
