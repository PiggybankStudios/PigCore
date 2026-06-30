/*
File:   tests_vr.c
Author: Taylor Robbins
Date:   01\24\2025
Description: 
	** Holds a little test VR graphical application
*/

//https://github.com/burito/vrtest/blob/master/src/vr.c

#if BUILD_WITH_OPENVR

//TODO: Find a better way to resolve this #define conflict?
#ifdef EXTERN_C
#undef EXTERN_C
#endif
#include "third_party/openvr/openvr_capi.h"

typedef intptr_t    (*VR_InitInternal_f)(EVRInitError *peError, EVRApplicationType eType);
typedef void        (*VR_ShutdownInternal_f)();
typedef bool        (*VR_IsHmdPresent_f)();
typedef intptr_t    (*VR_GetGenericInterface_f)( const char *pchInterfaceVersion, EVRInitError *peError );
typedef bool        (*VR_IsRuntimeInstalled_f)();
typedef const char* (*VR_RuntimePath_f)();
typedef const char* (*VR_GetVRInitErrorAsSymbol_f)( EVRInitError error );
typedef const char* (*VR_GetVRInitErrorAsEnglishDescription_f)( EVRInitError error );

typedef struct VREvent_t VREvent_t;
typedef struct TrackedDevicePose_t TrackedDevicePose_t;
typedef struct Texture_t Texture_t;
typedef struct VRTextureBounds_t VRTextureBounds_t;

typedef struct VR_IVRSystem_FnTable          VR_IVRSystem_FnTable;
typedef struct VR_IVRExtendedDisplay_FnTable VR_IVRExtendedDisplay_FnTable;
typedef struct VR_IVRTrackedCamera_FnTable   VR_IVRTrackedCamera_FnTable;
typedef struct VR_IVRApplications_FnTable    VR_IVRApplications_FnTable;
typedef struct VR_IVRChaperone_FnTable       VR_IVRChaperone_FnTable;
typedef struct VR_IVRChaperoneSetup_FnTable  VR_IVRChaperoneSetup_FnTable;
typedef struct VR_IVRCompositor_FnTable      VR_IVRCompositor_FnTable;
typedef struct VR_IVROverlay_FnTable         VR_IVROverlay_FnTable;
typedef struct VR_IVROverlayView_FnTable     VR_IVROverlayView_FnTable;
typedef struct VR_IVRHeadsetView_FnTable     VR_IVRHeadsetView_FnTable;
typedef struct VR_IVRRenderModels_FnTable    VR_IVRRenderModels_FnTable;
typedef struct VR_IVRNotifications_FnTable   VR_IVRNotifications_FnTable;
typedef struct VR_IVRSettings_FnTable        VR_IVRSettings_FnTable;

OsDll openVrDll = ZEROED;
VR_InitInternal_f                       VR_InitInternal                       = nullptr;
VR_ShutdownInternal_f                   VR_ShutdownInternal                   = nullptr;
VR_IsHmdPresent_f                       VR_IsHmdPresent                       = nullptr;
VR_GetGenericInterface_f                VR_GetGenericInterface                = nullptr;
VR_IsRuntimeInstalled_f                 VR_IsRuntimeInstalled                 = nullptr;
VR_RuntimePath_f                        VR_RuntimePath                        = nullptr;
VR_GetVRInitErrorAsSymbol_f             VR_GetVRInitErrorAsSymbol             = nullptr;
VR_GetVRInitErrorAsEnglishDescription_f VR_GetVRInitErrorAsEnglishDescription = nullptr;

VR_IVRSystem_FnTable*          vrSys        = nullptr;
VR_IVRExtendedDisplay_FnTable* vrDispExt    = nullptr;
VR_IVRTrackedCamera_FnTable*   vrTrkCam     = nullptr;
VR_IVRApplications_FnTable*    vrApp        = nullptr;
VR_IVRChaperone_FnTable*       vrChap       = nullptr;
VR_IVRChaperoneSetup_FnTable*  vrChapSetup  = nullptr;
VR_IVRCompositor_FnTable*      vrComp       = nullptr;
VR_IVROverlay_FnTable*         vrOverlay    = nullptr;
VR_IVROverlayView_FnTable*     vrOvView     = nullptr;
VR_IVRHeadsetView_FnTable*     vrHeadset    = nullptr;
VR_IVRRenderModels_FnTable*    vrRenderMods = nullptr;
VR_IVRNotifications_FnTable*   vrNotif      = nullptr;
VR_IVRSettings_FnTable*        vrSettings   = nullptr;

void* GetVrInterfaceFnTable(const char* interfaceNameAndVersion)
{
	ScratchBegin(scratch);
	//NOTE: GetGenericInterface returns us a class pointer if we just pass
	//      interfaceNameAndVersion (aka something like "IVRSystem_022")
	//      But if we prepend the "FnTable:" then we can get the function table directly
	EVRInitError vrError = ZEROED;
	Str8 fnTableName = PrintInArenaStr(scratch, "FnTable:%s", interfaceNameAndVersion);
	void* result = (void*)VR_GetGenericInterface(fnTableName.chars, &vrError);
	NotNull(result);
	ScratchEnd(scratch);
	return result;
}

const char* GetVrEventTypeStr(EVREventType eventType)
{
	switch (eventType)
	{
		case EVREventType_VREvent_None:                                      return "None";
		case EVREventType_VREvent_TrackedDeviceActivated:                    return "TrackedDeviceActivated";
		case EVREventType_VREvent_TrackedDeviceDeactivated:                  return "TrackedDeviceDeactivated";
		case EVREventType_VREvent_TrackedDeviceUpdated:                      return "TrackedDeviceUpdated";
		case EVREventType_VREvent_TrackedDeviceUserInteractionStarted:       return "TrackedDeviceUserInteractionStarted";
		case EVREventType_VREvent_TrackedDeviceUserInteractionEnded:         return "TrackedDeviceUserInteractionEnded";
		case EVREventType_VREvent_IpdChanged:                                return "IpdChanged";
		case EVREventType_VREvent_EnterStandbyMode:                          return "EnterStandbyMode";
		case EVREventType_VREvent_LeaveStandbyMode:                          return "LeaveStandbyMode";
		case EVREventType_VREvent_TrackedDeviceRoleChanged:                  return "TrackedDeviceRoleChanged";
		case EVREventType_VREvent_WatchdogWakeUpRequested:                   return "WatchdogWakeUpRequested";
		case EVREventType_VREvent_LensDistortionChanged:                     return "LensDistortionChanged";
		case EVREventType_VREvent_PropertyChanged:                           return "PropertyChanged";
		case EVREventType_VREvent_WirelessDisconnect:                        return "WirelessDisconnect";
		case EVREventType_VREvent_WirelessReconnect:                         return "WirelessReconnect";
		case EVREventType_VREvent_Reserved_01:                               return "Reserved_01";
		case EVREventType_VREvent_Reserved_02:                               return "Reserved_02";
		case EVREventType_VREvent_ButtonPress:                               return "ButtonPress";
		case EVREventType_VREvent_ButtonUnpress:                             return "ButtonUnpress";
		case EVREventType_VREvent_ButtonTouch:                               return "ButtonTouch";
		case EVREventType_VREvent_ButtonUntouch:                             return "ButtonUntouch";
		case EVREventType_VREvent_Modal_Cancel:                              return "Modal_Cancel";
		case EVREventType_VREvent_MouseMove:                                 return "MouseMove";
		case EVREventType_VREvent_MouseButtonDown:                           return "MouseButtonDown";
		case EVREventType_VREvent_MouseButtonUp:                             return "MouseButtonUp";
		case EVREventType_VREvent_FocusEnter:                                return "FocusEnter";
		case EVREventType_VREvent_FocusLeave:                                return "FocusLeave";
		case EVREventType_VREvent_ScrollDiscrete:                            return "ScrollDiscrete";
		case EVREventType_VREvent_TouchPadMove:                              return "TouchPadMove";
		case EVREventType_VREvent_OverlayFocusChanged:                       return "OverlayFocusChanged";
		case EVREventType_VREvent_ReloadOverlays:                            return "ReloadOverlays";
		case EVREventType_VREvent_ScrollSmooth:                              return "ScrollSmooth";
		case EVREventType_VREvent_LockMousePosition:                         return "LockMousePosition";
		case EVREventType_VREvent_UnlockMousePosition:                       return "UnlockMousePosition";
		case EVREventType_VREvent_InputFocusCaptured:                        return "InputFocusCaptured";
		case EVREventType_VREvent_InputFocusReleased:                        return "InputFocusReleased";
		case EVREventType_VREvent_SceneApplicationChanged:                   return "SceneApplicationChanged";
		case EVREventType_VREvent_InputFocusChanged:                         return "InputFocusChanged";
		case EVREventType_VREvent_SceneApplicationUsingWrongGraphicsAdapter: return "SceneApplicationUsingWrongGraphicsAdapter";
		case EVREventType_VREvent_ActionBindingReloaded:                     return "ActionBindingReloaded";
		case EVREventType_VREvent_HideRenderModels:                          return "HideRenderModels";
		case EVREventType_VREvent_ShowRenderModels:                          return "ShowRenderModels";
		case EVREventType_VREvent_SceneApplicationStateChanged:              return "SceneApplicationStateChanged";
		case EVREventType_VREvent_SceneAppPipeDisconnected:                  return "SceneAppPipeDisconnected";
		case EVREventType_VREvent_ConsoleOpened:                             return "ConsoleOpened";
		case EVREventType_VREvent_ConsoleClosed:                             return "ConsoleClosed";
		case EVREventType_VREvent_OverlayShown:                              return "OverlayShown";
		case EVREventType_VREvent_OverlayHidden:                             return "OverlayHidden";
		case EVREventType_VREvent_DashboardActivated:                        return "DashboardActivated";
		case EVREventType_VREvent_DashboardDeactivated:                      return "DashboardDeactivated";
		case EVREventType_VREvent_DashboardRequested:                        return "DashboardRequested";
		case EVREventType_VREvent_ResetDashboard:                            return "ResetDashboard";
		case EVREventType_VREvent_ImageLoaded:                               return "ImageLoaded";
		case EVREventType_VREvent_ShowKeyboard:                              return "ShowKeyboard";
		case EVREventType_VREvent_HideKeyboard:                              return "HideKeyboard";
		case EVREventType_VREvent_OverlayGamepadFocusGained:                 return "OverlayGamepadFocusGained";
		case EVREventType_VREvent_OverlayGamepadFocusLost:                   return "OverlayGamepadFocusLost";
		case EVREventType_VREvent_OverlaySharedTextureChanged:               return "OverlaySharedTextureChanged";
		case EVREventType_VREvent_ScreenshotTriggered:                       return "ScreenshotTriggered";
		case EVREventType_VREvent_ImageFailed:                               return "ImageFailed";
		case EVREventType_VREvent_DashboardOverlayCreated:                   return "DashboardOverlayCreated";
		case EVREventType_VREvent_SwitchGamepadFocus:                        return "SwitchGamepadFocus";
		case EVREventType_VREvent_RequestScreenshot:                         return "RequestScreenshot";
		case EVREventType_VREvent_ScreenshotTaken:                           return "ScreenshotTaken";
		case EVREventType_VREvent_ScreenshotFailed:                          return "ScreenshotFailed";
		case EVREventType_VREvent_SubmitScreenshotToDashboard:               return "SubmitScreenshotToDashboard";
		case EVREventType_VREvent_ScreenshotProgressToDashboard:             return "ScreenshotProgressToDashboard";
		case EVREventType_VREvent_PrimaryDashboardDeviceChanged:             return "PrimaryDashboardDeviceChanged";
		case EVREventType_VREvent_RoomViewShown:                             return "RoomViewShown";
		case EVREventType_VREvent_RoomViewHidden:                            return "RoomViewHidden";
		case EVREventType_VREvent_ShowUI:                                    return "ShowUI";
		case EVREventType_VREvent_ShowDevTools:                              return "ShowDevTools";
		case EVREventType_VREvent_DesktopViewUpdating:                       return "DesktopViewUpdating";
		case EVREventType_VREvent_DesktopViewReady:                          return "DesktopViewReady";
		case EVREventType_VREvent_StartDashboard:                            return "StartDashboard";
		case EVREventType_VREvent_ElevatePrism:                              return "ElevatePrism";
		case EVREventType_VREvent_OverlayClosed:                             return "OverlayClosed";
		case EVREventType_VREvent_DashboardThumbChanged:                     return "DashboardThumbChanged";
		case EVREventType_VREvent_DesktopMightBeVisible:                     return "DesktopMightBeVisible";
		case EVREventType_VREvent_DesktopMightBeHidden:                      return "DesktopMightBeHidden";
		case EVREventType_VREvent_MutualSteamCapabilitiesChanged:            return "MutualSteamCapabilitiesChanged";
		case EVREventType_VREvent_OverlayCreated:                            return "OverlayCreated";
		case EVREventType_VREvent_OverlayDestroyed:                          return "OverlayDestroyed";
		case EVREventType_VREvent_Notification_Shown:                        return "Notification_Shown";
		case EVREventType_VREvent_Notification_Hidden:                       return "Notification_Hidden";
		case EVREventType_VREvent_Notification_BeginInteraction:             return "Notification_BeginInteraction";
		case EVREventType_VREvent_Notification_Destroyed:                    return "Notification_Destroyed";
		case EVREventType_VREvent_Quit:                                      return "Quit";
		case EVREventType_VREvent_ProcessQuit:                               return "ProcessQuit";
		case EVREventType_VREvent_QuitAcknowledged:                          return "QuitAcknowledged";
		case EVREventType_VREvent_DriverRequestedQuit:                       return "DriverRequestedQuit";
		case EVREventType_VREvent_RestartRequested:                          return "RestartRequested";
		case EVREventType_VREvent_InvalidateSwapTextureSets:                 return "InvalidateSwapTextureSets";
		case EVREventType_VREvent_ChaperoneDataHasChanged:                   return "ChaperoneDataHasChanged";
		case EVREventType_VREvent_ChaperoneUniverseHasChanged:               return "ChaperoneUniverseHasChanged";
		case EVREventType_VREvent_ChaperoneTempDataHasChanged:               return "ChaperoneTempDataHasChanged";
		case EVREventType_VREvent_ChaperoneSettingsHaveChanged:              return "ChaperoneSettingsHaveChanged";
		case EVREventType_VREvent_SeatedZeroPoseReset:                       return "SeatedZeroPoseReset";
		case EVREventType_VREvent_ChaperoneFlushCache:                       return "ChaperoneFlushCache";
		case EVREventType_VREvent_ChaperoneRoomSetupStarting:                return "ChaperoneRoomSetupStarting";
		case EVREventType_VREvent_ChaperoneRoomSetupFinished:                return "ChaperoneRoomSetupFinished";
		case EVREventType_VREvent_StandingZeroPoseReset:                     return "StandingZeroPoseReset";
		case EVREventType_VREvent_AudioSettingsHaveChanged:                  return "AudioSettingsHaveChanged";
		case EVREventType_VREvent_BackgroundSettingHasChanged:               return "BackgroundSettingHasChanged";
		case EVREventType_VREvent_CameraSettingsHaveChanged:                 return "CameraSettingsHaveChanged";
		case EVREventType_VREvent_ReprojectionSettingHasChanged:             return "ReprojectionSettingHasChanged";
		case EVREventType_VREvent_ModelSkinSettingsHaveChanged:              return "ModelSkinSettingsHaveChanged";
		case EVREventType_VREvent_EnvironmentSettingsHaveChanged:            return "EnvironmentSettingsHaveChanged";
		case EVREventType_VREvent_PowerSettingsHaveChanged:                  return "PowerSettingsHaveChanged";
		case EVREventType_VREvent_EnableHomeAppSettingsHaveChanged:          return "EnableHomeAppSettingsHaveChanged";
		case EVREventType_VREvent_SteamVRSectionSettingChanged:              return "SteamVRSectionSettingChanged";
		case EVREventType_VREvent_LighthouseSectionSettingChanged:           return "LighthouseSectionSettingChanged";
		case EVREventType_VREvent_NullSectionSettingChanged:                 return "NullSectionSettingChanged";
		case EVREventType_VREvent_UserInterfaceSectionSettingChanged:        return "UserInterfaceSectionSettingChanged";
		case EVREventType_VREvent_NotificationsSectionSettingChanged:        return "NotificationsSectionSettingChanged";
		case EVREventType_VREvent_KeyboardSectionSettingChanged:             return "KeyboardSectionSettingChanged";
		case EVREventType_VREvent_PerfSectionSettingChanged:                 return "PerfSectionSettingChanged";
		case EVREventType_VREvent_DashboardSectionSettingChanged:            return "DashboardSectionSettingChanged";
		case EVREventType_VREvent_WebInterfaceSectionSettingChanged:         return "WebInterfaceSectionSettingChanged";
		case EVREventType_VREvent_TrackersSectionSettingChanged:             return "TrackersSectionSettingChanged";
		case EVREventType_VREvent_LastKnownSectionSettingChanged:            return "LastKnownSectionSettingChanged";
		case EVREventType_VREvent_DismissedWarningsSectionSettingChanged:    return "DismissedWarningsSectionSettingChanged";
		case EVREventType_VREvent_GpuSpeedSectionSettingChanged:             return "GpuSpeedSectionSettingChanged";
		case EVREventType_VREvent_WindowsMRSectionSettingChanged:            return "WindowsMRSectionSettingChanged";
		case EVREventType_VREvent_OtherSectionSettingChanged:                return "OtherSectionSettingChanged";
		case EVREventType_VREvent_AnyDriverSettingsChanged:                  return "AnyDriverSettingsChanged";
		case EVREventType_VREvent_StatusUpdate:                              return "StatusUpdate";
		case EVREventType_VREvent_WebInterface_InstallDriverCompleted:       return "WebInterface_InstallDriverCompleted";
		case EVREventType_VREvent_MCImageUpdated:                            return "MCImageUpdated";
		case EVREventType_VREvent_FirmwareUpdateStarted:                     return "FirmwareUpdateStarted";
		case EVREventType_VREvent_FirmwareUpdateFinished:                    return "FirmwareUpdateFinished";
		case EVREventType_VREvent_KeyboardClosed:                            return "KeyboardClosed";
		case EVREventType_VREvent_KeyboardCharInput:                         return "KeyboardCharInput";
		case EVREventType_VREvent_KeyboardDone:                              return "KeyboardDone";
		case EVREventType_VREvent_KeyboardOpened_Global:                     return "KeyboardOpened_Global";
		case EVREventType_VREvent_KeyboardClosed_Global:                     return "KeyboardClosed_Global";
		case EVREventType_VREvent_ApplicationListUpdated:                    return "ApplicationListUpdated";
		case EVREventType_VREvent_ApplicationMimeTypeLoad:                   return "ApplicationMimeTypeLoad";
		case EVREventType_VREvent_ProcessConnected:                          return "ProcessConnected";
		case EVREventType_VREvent_ProcessDisconnected:                       return "ProcessDisconnected";
		case EVREventType_VREvent_Compositor_ChaperoneBoundsShown:           return "Compositor_ChaperoneBoundsShown";
		case EVREventType_VREvent_Compositor_ChaperoneBoundsHidden:          return "Compositor_ChaperoneBoundsHidden";
		case EVREventType_VREvent_Compositor_DisplayDisconnected:            return "Compositor_DisplayDisconnected";
		case EVREventType_VREvent_Compositor_DisplayReconnected:             return "Compositor_DisplayReconnected";
		case EVREventType_VREvent_Compositor_HDCPError:                      return "Compositor_HDCPError";
		case EVREventType_VREvent_Compositor_ApplicationNotResponding:       return "Compositor_ApplicationNotResponding";
		case EVREventType_VREvent_Compositor_ApplicationResumed:             return "Compositor_ApplicationResumed";
		case EVREventType_VREvent_Compositor_OutOfVideoMemory:               return "Compositor_OutOfVideoMemory";
		case EVREventType_VREvent_Compositor_DisplayModeNotSupported:        return "Compositor_DisplayModeNotSupported";
		case EVREventType_VREvent_Compositor_StageOverrideReady:             return "Compositor_StageOverrideReady";
		case EVREventType_VREvent_Compositor_RequestDisconnectReconnect:     return "Compositor_RequestDisconnectReconnect";
		case EVREventType_VREvent_TrackedCamera_StartVideoStream:            return "TrackedCamera_StartVideoStream";
		case EVREventType_VREvent_TrackedCamera_StopVideoStream:             return "TrackedCamera_StopVideoStream";
		case EVREventType_VREvent_TrackedCamera_PauseVideoStream:            return "TrackedCamera_PauseVideoStream";
		case EVREventType_VREvent_TrackedCamera_ResumeVideoStream:           return "TrackedCamera_ResumeVideoStream";
		case EVREventType_VREvent_TrackedCamera_EditingSurface:              return "TrackedCamera_EditingSurface";
		case EVREventType_VREvent_PerformanceTest_EnableCapture:             return "PerformanceTest_EnableCapture";
		case EVREventType_VREvent_PerformanceTest_DisableCapture:            return "PerformanceTest_DisableCapture";
		case EVREventType_VREvent_PerformanceTest_FidelityLevel:             return "PerformanceTest_FidelityLevel";
		case EVREventType_VREvent_MessageOverlay_Closed:                     return "MessageOverlay_Closed";
		case EVREventType_VREvent_MessageOverlayCloseRequested:              return "MessageOverlayCloseRequested";
		case EVREventType_VREvent_Input_HapticVibration:                     return "Input_HapticVibration";
		case EVREventType_VREvent_Input_BindingLoadFailed:                   return "Input_BindingLoadFailed";
		case EVREventType_VREvent_Input_BindingLoadSuccessful:               return "Input_BindingLoadSuccessful";
		case EVREventType_VREvent_Input_ActionManifestReloaded:              return "Input_ActionManifestReloaded";
		case EVREventType_VREvent_Input_ActionManifestLoadFailed:            return "Input_ActionManifestLoadFailed";
		case EVREventType_VREvent_Input_ProgressUpdate:                      return "Input_ProgressUpdate";
		case EVREventType_VREvent_Input_TrackerActivated:                    return "Input_TrackerActivated";
		case EVREventType_VREvent_Input_BindingsUpdated:                     return "Input_BindingsUpdated";
		case EVREventType_VREvent_Input_BindingSubscriptionChanged:          return "Input_BindingSubscriptionChanged";
		case EVREventType_VREvent_SpatialAnchors_PoseUpdated:                return "SpatialAnchors_PoseUpdated";
		case EVREventType_VREvent_SpatialAnchors_DescriptorUpdated:          return "SpatialAnchors_DescriptorUpdated";
		case EVREventType_VREvent_SpatialAnchors_RequestPoseUpdate:          return "SpatialAnchors_RequestPoseUpdate";
		case EVREventType_VREvent_SpatialAnchors_RequestDescriptorUpdate:    return "SpatialAnchors_RequestDescriptorUpdate";
		case EVREventType_VREvent_SystemReport_Started:                      return "SystemReport_Started";
		case EVREventType_VREvent_Monitor_ShowHeadsetView:                   return "Monitor_ShowHeadsetView";
		case EVREventType_VREvent_Monitor_HideHeadsetView:                   return "Monitor_HideHeadsetView";
		case EVREventType_VREvent_Audio_SetSpeakersVolume:                   return "Audio_SetSpeakersVolume";
		case EVREventType_VREvent_Audio_SetSpeakersMute:                     return "Audio_SetSpeakersMute";
		case EVREventType_VREvent_Audio_SetMicrophoneVolume:                 return "Audio_SetMicrophoneVolume";
		case EVREventType_VREvent_Audio_SetMicrophoneMute:                   return "Audio_SetMicrophoneMute";
		case EVREventType_VREvent_VendorSpecific_Reserved_Start:             return "VendorSpecific_Reserved_Start";
		case EVREventType_VREvent_VendorSpecific_Reserved_End:               return "VendorSpecific_Reserved_End";
		default: return UNKNOWN_STR;
	}
}
const char* GetVrCompositorErrorStr(EVRCompositorError compositorError)
{
	switch (compositorError)
	{
		case EVRCompositorError_VRCompositorError_None:                         return "None";
		case EVRCompositorError_VRCompositorError_RequestFailed:                return "RequestFailed";
		case EVRCompositorError_VRCompositorError_IncompatibleVersion:          return "IncompatibleVersion";
		case EVRCompositorError_VRCompositorError_DoNotHaveFocus:               return "DoNotHaveFocus";
		case EVRCompositorError_VRCompositorError_InvalidTexture:               return "InvalidTexture";
		case EVRCompositorError_VRCompositorError_IsNotSceneApplication:        return "IsNotSceneApplication";
		case EVRCompositorError_VRCompositorError_TextureIsOnWrongDevice:       return "TextureIsOnWrongDevice";
		case EVRCompositorError_VRCompositorError_TextureUsesUnsupportedFormat: return "TextureUsesUnsupportedFormat";
		case EVRCompositorError_VRCompositorError_SharedTexturesNotSupported:   return "SharedTexturesNotSupported";
		case EVRCompositorError_VRCompositorError_IndexOutOfRange:              return "IndexOutOfRange";
		case EVRCompositorError_VRCompositorError_AlreadySubmitted:             return "AlreadySubmitted";
		case EVRCompositorError_VRCompositorError_InvalidBounds:                return "InvalidBounds";
		case EVRCompositorError_VRCompositorError_AlreadySet:                   return "AlreadySet";
		default: return UNKNOWN_STR;
	}
}

#if BUILD_WITH_RAYLIB
RenderTexture2D vrRenderTexture = ZEROED;
VrStereoConfig vrStereoConfig = ZEROED;
Camera3D vrCamera = ZEROED;
#endif //BUILD_WITH_RAYLIB

bool InitVrTests()
{
	Result loadDllResult = OsLoadDll(FilePathLit("openvr_api.dll"), &openVrDll);
	Assert(loadDllResult == Result_Success);
	
	VR_InitInternal                       =                       (VR_InitInternal_f)OsFindDllFunc(&openVrDll, StrLit("VR_InitInternal"));
	VR_ShutdownInternal                   =                   (VR_ShutdownInternal_f)OsFindDllFunc(&openVrDll, StrLit("VR_ShutdownInternal"));
	VR_IsHmdPresent                       =                       (VR_IsHmdPresent_f)OsFindDllFunc(&openVrDll, StrLit("VR_IsHmdPresent"));
	VR_GetGenericInterface                =                (VR_GetGenericInterface_f)OsFindDllFunc(&openVrDll, StrLit("VR_GetGenericInterface"));
	VR_IsRuntimeInstalled                 =                 (VR_IsRuntimeInstalled_f)OsFindDllFunc(&openVrDll, StrLit("VR_IsRuntimeInstalled"));
	VR_RuntimePath                        =                        (VR_RuntimePath_f)OsFindDllFunc(&openVrDll, StrLit("VR_RuntimePath"));
	VR_GetVRInitErrorAsSymbol             =             (VR_GetVRInitErrorAsSymbol_f)OsFindDllFunc(&openVrDll, StrLit("VR_GetVRInitErrorAsSymbol"));
	VR_GetVRInitErrorAsEnglishDescription = (VR_GetVRInitErrorAsEnglishDescription_f)OsFindDllFunc(&openVrDll, StrLit("VR_GetVRInitErrorAsEnglishDescription"));
	NotNull(VR_InitInternal);
	NotNull(VR_ShutdownInternal);
	NotNull(VR_IsHmdPresent);
	NotNull(VR_GetGenericInterface);
	NotNull(VR_IsRuntimeInstalled);
	NotNull(VR_RuntimePath);
	NotNull(VR_GetVRInitErrorAsSymbol);
	NotNull(VR_GetVRInitErrorAsEnglishDescription);
	
	if (!VR_IsHmdPresent()) { WriteLine_E("VR Headset is not present!"); return false; }
	if (!VR_IsRuntimeInstalled()) { WriteLine_E("VR Runtime is not installed!"); return false; }
	
	// PrintLine_D("OpenVR Runtime Path: \"%s\"", VR_RuntimePath()); //"D:\SteamLibrary\steamapps\common\SteamVR"
	
	EVRInitError vrInitError = ZEROED;
	intptr_t initResult = VR_InitInternal(&vrInitError, EVRApplicationType_VRApplication_Scene);
	UNUSED(initResult); //Seems to always be 1
	// PrintLine_D("initResult: %d (error: %u)", initResult, vrInitError);
	if (vrInitError != EVRInitError_VRInitError_None) { PrintLine_E("Failed to Init OpenVR SDK: %s", VR_GetVRInitErrorAsEnglishDescription(vrInitError)); return false; }
	
	vrSys        =          (VR_IVRSystem_FnTable*)GetVrInterfaceFnTable(IVRSystem_Version);          NotNull(vrSys);
	vrDispExt    = (VR_IVRExtendedDisplay_FnTable*)GetVrInterfaceFnTable(IVRExtendedDisplay_Version); NotNull(vrDispExt);
	vrTrkCam     =   (VR_IVRTrackedCamera_FnTable*)GetVrInterfaceFnTable(IVRTrackedCamera_Version);   NotNull(vrTrkCam);
	vrApp        =    (VR_IVRApplications_FnTable*)GetVrInterfaceFnTable(IVRApplications_Version);    NotNull(vrApp);
	vrChap       =       (VR_IVRChaperone_FnTable*)GetVrInterfaceFnTable(IVRChaperone_Version);       NotNull(vrChap);
	vrChapSetup  =  (VR_IVRChaperoneSetup_FnTable*)GetVrInterfaceFnTable(IVRChaperoneSetup_Version);  NotNull(vrChapSetup);
	vrComp       =      (VR_IVRCompositor_FnTable*)GetVrInterfaceFnTable(IVRCompositor_Version);      NotNull(vrComp);
	vrOverlay    =         (VR_IVROverlay_FnTable*)GetVrInterfaceFnTable(IVROverlay_Version);         NotNull(vrOverlay);
	vrOvView     =     (VR_IVROverlayView_FnTable*)GetVrInterfaceFnTable(IVROverlayView_Version);     NotNull(vrOvView);
	vrHeadset    =     (VR_IVRHeadsetView_FnTable*)GetVrInterfaceFnTable(IVRHeadsetView_Version);     NotNull(vrHeadset);
	vrRenderMods =    (VR_IVRRenderModels_FnTable*)GetVrInterfaceFnTable(IVRRenderModels_Version);    NotNull(vrRenderMods);
	vrNotif      =   (VR_IVRNotifications_FnTable*)GetVrInterfaceFnTable(IVRNotifications_Version);   NotNull(vrNotif);
	vrSettings   =        (VR_IVRSettings_FnTable*)GetVrInterfaceFnTable(IVRSettings_Version);        NotNull(vrSettings);
	
	u32 recommendedWidth, recommendedHeight;
	vrSys->GetRecommendedRenderTargetSize(&recommendedWidth, &recommendedHeight);
	PrintLine_D("recommendedWidth: %u recommendedHeight: %u", recommendedWidth, recommendedHeight);
	
	TrackedDeviceIndex_t rightHandIndex = vrSys->GetTrackedDeviceIndexForControllerRole(ETrackedControllerRole_TrackedControllerRole_RightHand); //returns 0xFFFFFFFFFFFFFFFF
	PrintLine_D("rightHandIndex: %u", rightHandIndex);
	
	#if BUILD_WITH_RAYLIB
	VrDeviceInfo vrDeviceInfo = {
		// Oculus Rift CV1 parameters for simulator
		.hResolution = recommendedWidth*2,     // Horizontal resolution in pixels
		.vResolution = recommendedHeight,    // Vertical resolution in pixels
		.hScreenSize = 0.133793f,            // Horizontal size in meters
		.vScreenSize = 0.0669f,              // Vertical size in meters
		.eyeToScreenDistance = 0.041f,       // Distance between eye and display in meters
		.lensSeparationDistance = 0.07f,     // Lens separation distance in meters
		.interpupillaryDistance = 0.07f,     // IPD (distance between pupils) in meters

		// NOTE: CV1 uses fresnel-hybrid-asymmetric lenses with specific compute shaders
		// Following parameters are just an approximation to CV1 distortion stereo rendering
		.lensDistortionValues[0] = 1.0f,     // Lens distortion constant parameter 0
		.lensDistortionValues[1] = 0.22f,    // Lens distortion constant parameter 1
		.lensDistortionValues[2] = 0.24f,    // Lens distortion constant parameter 2
		.lensDistortionValues[3] = 0.0f,     // Lens distortion constant parameter 3
		.chromaAbCorrection[0] = 0.996f,     // Chromatic aberration correction parameter 0
		.chromaAbCorrection[1] = -0.004f,    // Chromatic aberration correction parameter 1
		.chromaAbCorrection[2] = 1.014f,     // Chromatic aberration correction parameter 2
		.chromaAbCorrection[3] = 0.0f,       // Chromatic aberration correction parameter 3
	};
	vrStereoConfig = LoadVrStereoConfig(vrDeviceInfo);
	vrRenderTexture = LoadRenderTexture(vrDeviceInfo.hResolution, vrDeviceInfo.vResolution);
	vrCamera.position = NewVector3(1, 2, -10);
	vrCamera.target = NewVector3(0, 0, 0);
	vrCamera.up = Vector3FromV3(V3_Up);
	vrCamera.fovy = 60; //ToDegrees32(QuarterPi32);
	vrCamera.projection = CAMERA_PERSPECTIVE;
	
	SetTargetFPS(90);
	#endif
	
	return true;
}

void RunVrTests()
{
	VREvent_t event = ZEROED;
	while (vrSys->PollNextEvent(&event, sizeof(event)))
	{
		if (event.eventType != EVREventType_VREvent_Reserved_01 &&
			event.eventType != EVREventType_VREvent_Reserved_02)
		{
			PrintLine_D("VR Event: %s", GetVrEventTypeStr((EVREventType)event.eventType));
		}
	}
	
	v3 headsetPosition = V3_Zero;
	quat headsetRotation = Quat_Identity;
	
	EVRCompositorError compError;
	TrackedDevicePose_t renderPoses[64]; //k_unMaxTrackedDeviceCount doesn't work? But it's 64
	// TrackedDevicePose_t gamePoses[k_unMaxTrackedDeviceCount] = ZEROED;
	compError = vrComp->WaitGetPoses(&renderPoses[0], ArrayCount(renderPoses), nullptr, 0);
	if (compError != EVRCompositorError_VRCompositorError_None) { PrintLine_W("WaitGetPoses Error: %s", GetVrCompositorErrorStr(compError)); }
	for (u64 pIndex = 0; pIndex < ArrayCount(renderPoses); pIndex++)
	{
		if (renderPoses[pIndex].bPoseIsValid)
		{
			ETrackedDeviceClass deviceClass = vrSys->GetTrackedDeviceClass((TrackedDeviceIndex_t)pIndex);
			switch (deviceClass)
			{
				case ETrackedDeviceClass_TrackedDeviceClass_HMD:
				{
					// PrintLine_D("Headset is index %llu", pIndex);
					//NOTE: OpenVR gives us a +Y Up / Right-Handed transformation matrix
					//      We work in a +Y Up / Left-Handed coordinate space so we
					//      need to do a change of basis transformation which looks like
					//      A * M * Inverse(A) where A is a matrix that has the OpenVR
					//      unit vectors described in terms of our coordinate space used
					//      as columns. In this case ovr_i = -i, ovr_j = j, ovr_k = -k (X and Z axis are flipped)
					//      Also note that this matrix is symmetric so inverse(A) = A
					mat3 pigToOpenVROrBack = NewMat3(
						-1, 0, 0,
						0, 1, 0,
						0, 0,-1
					);
					HmdMatrix34_t hmdMatrix = renderPoses[pIndex].mDeviceToAbsoluteTracking;
					mat3 matrix = NewMat3(
						hmdMatrix.m[0][0], hmdMatrix.m[0][1], hmdMatrix.m[0][2],
						hmdMatrix.m[1][0], hmdMatrix.m[1][1], hmdMatrix.m[1][2],
						hmdMatrix.m[2][0], hmdMatrix.m[2][1], hmdMatrix.m[2][2]
					);
					matrix = Mul(Mul(pigToOpenVROrBack, matrix), pigToOpenVROrBack);
					headsetRotation = QuatFromMat3(matrix);
					
					// NOTE: Also flip X and Z axis when grabbing the headset position in space
					headsetPosition = NewV3(-hmdMatrix.m[0][3], hmdMatrix.m[1][3], -hmdMatrix.m[2][3]);
				} break;
			}
		}
	}
	
	#if BUILD_WITH_RAYLIB
	
	mat4 headsetRotationMat = ToMat4FromQuat(headsetRotation);
	v3 headsetForwardVec = MulMat4AndV3(headsetRotationMat, V3_Forward, false);
	v3 headsetUpVec = MulMat4AndV3(headsetRotationMat, V3_Up, false);
	v3 cameraOrigin = V3FromVector3(vrCamera.position);
	v3 cameraPosition = Add(cameraOrigin, headsetPosition);
	v3 cameraLookAt = Add(cameraPosition, Mul(headsetForwardVec, 5.0f));
	vrCamera.position = Vector3FromV3(cameraPosition);
	vrCamera.target = Vector3FromV3(cameraLookAt);
	vrCamera.up = Vector3FromV3(headsetUpVec);
	
	BeginTextureMode(vrRenderTexture);
	{
		ClearBackground(RAYWHITE);
		BeginVrStereoMode(vrStereoConfig);
		BeginMode3D(vrCamera);
		{
			DrawCube(Vector3_Zero, 2.0f, 2.0f, 2.0f, RED);
			DrawCubeWires(Vector3_Zero, 2.0f, 2.0f, 2.0f, MAROON);
			DrawGrid(40, 1.0f);
		}
		EndMode3D();
		EndVrStereoMode();
	}
	EndTextureMode();
	
	Texture_t leftEyeTexture = ZEROED;
	leftEyeTexture.handle = (void*)((uintptr_t)vrRenderTexture.texture.id);
	leftEyeTexture.eType = ETextureType_TextureType_OpenGL;
	leftEyeTexture.eColorSpace = EColorSpace_ColorSpace_Gamma;
	VRTextureBounds_t leftEyeBounds = ZEROED;
	leftEyeBounds.uMin = 0;
	leftEyeBounds.vMin = 0;
	leftEyeBounds.uMax = 0.5f;
	leftEyeBounds.vMax = 1;
	compError = vrComp->Submit(EVREye_Eye_Left, &leftEyeTexture, &leftEyeBounds, EVRSubmitFlags_Submit_Default);
	if (compError != EVRCompositorError_VRCompositorError_None) { PrintLine_W("Submit(Left) Error: %s", GetVrCompositorErrorStr(compError)); }
	
	Texture_t rightEyeTexture = ZEROED;
	rightEyeTexture.handle = (void*)((uintptr_t)vrRenderTexture.texture.id);
	rightEyeTexture.eType = ETextureType_TextureType_OpenGL;
	rightEyeTexture.eColorSpace = EColorSpace_ColorSpace_Gamma;
	VRTextureBounds_t rightEyeBounds = ZEROED;
	rightEyeBounds.uMin = 0.5f;
	rightEyeBounds.vMin = 0;
	rightEyeBounds.uMax = 1;
	rightEyeBounds.vMax = 1;
	compError = vrComp->Submit(EVREye_Eye_Right, &rightEyeTexture, &rightEyeBounds, EVRSubmitFlags_Submit_Default);
	if (compError != EVRCompositorError_VRCompositorError_None) { PrintLine_W("Submit(Right) Error: %s", GetVrCompositorErrorStr(compError)); }
	
	BeginDrawing();
	{
		ClearBackground(RAYWHITE);
		Rectangle sourceRec = (Rectangle){ 0, 0, (r32)vrRenderTexture.texture.width, -(r32)vrRenderTexture.texture.height };
		Rectangle destRec = (Rectangle){ 0, 0, (r32)GetScreenWidth(), -(r32)GetScreenHeight() };
		DrawTexturePro(vrRenderTexture.texture, sourceRec, destRec, NewVector2(0, 0), 0.0f, WHITE);
		DrawFPS(10, 10);
	}
	EndDrawing();
	
	vrCamera.position = Vector3FromV3(cameraOrigin);
	#endif //BUILD_WITH_RAYLIB
}

#endif //BUILD_WITH_OPENVR
