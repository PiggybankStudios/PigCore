/*
File:   tests_vr.c
Author: Taylor Robbins
Date:   01\24\2025
Description: 
	** Holds a little test VR graphical application
*/

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
typedef const char* (*VR_GetVRInitErrorAsSymbol_f)( EVRInitError error );
typedef const char* (*VR_GetVRInitErrorAsEnglishDescription_f)( EVRInitError error );

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
	//NOTE: GetGenericInterface seems to return us a pointer to a C++ class,
	//      and the class has a vtable, the pointer to the vtable is stored right
	//      at the beginning of the class, so we pretend the return value is a
	//      double pointer and dereference it
	EVRInitError vrError = ZEROED;
	void** result = (void**)VR_GetGenericInterface(interfaceNameAndVersion, &vrError);
	NotNull(result);
	return *result;
}

void InitVrTests()
{
	Result loadDllResult = OsLoadDll(FilePathLit("openvr_api.dll"), &openVrDll);
	Assert(loadDllResult == Result_Success);
	
	VR_InitInternal                       =                       (VR_InitInternal_f)OsFindDllFunc(&openVrDll, StrLit("VR_InitInternal"));
	VR_ShutdownInternal                   =                   (VR_ShutdownInternal_f)OsFindDllFunc(&openVrDll, StrLit("VR_ShutdownInternal"));
	VR_IsHmdPresent                       =                       (VR_IsHmdPresent_f)OsFindDllFunc(&openVrDll, StrLit("VR_IsHmdPresent"));
	VR_GetGenericInterface                =                (VR_GetGenericInterface_f)OsFindDllFunc(&openVrDll, StrLit("VR_GetGenericInterface"));
	VR_IsRuntimeInstalled                 =                 (VR_IsRuntimeInstalled_f)OsFindDllFunc(&openVrDll, StrLit("VR_IsRuntimeInstalled"));
	VR_GetVRInitErrorAsSymbol             =             (VR_GetVRInitErrorAsSymbol_f)OsFindDllFunc(&openVrDll, StrLit("VR_GetVRInitErrorAsSymbol"));
	VR_GetVRInitErrorAsEnglishDescription = (VR_GetVRInitErrorAsEnglishDescription_f)OsFindDllFunc(&openVrDll, StrLit("VR_GetVRInitErrorAsEnglishDescription"));
	NotNull(VR_InitInternal);
	NotNull(VR_ShutdownInternal);
	NotNull(VR_IsHmdPresent);
	NotNull(VR_GetGenericInterface);
	NotNull(VR_IsRuntimeInstalled);
	NotNull(VR_GetVRInitErrorAsSymbol);
	NotNull(VR_GetVRInitErrorAsEnglishDescription);
	
	EVRInitError vrInitError = ZEROED;
	intptr_t initResult = VR_InitInternal(&vrInitError, EVRApplicationType_VRApplication_Scene);
	PrintLine_D("initResult: %d (error: %u)", initResult, vrInitError);
	Assert(initResult == 1); //TODO: Is this the only value that is success?
	
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
}

void RunVrTests()
{
	
}

#endif //BUILD_WITH_OPENVR
