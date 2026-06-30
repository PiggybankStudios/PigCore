@echo off
setlocal enabledelayedexpansion

if not exist _build mkdir _build
pushd _build
set physx_folder=../physx
set scripts_folder=../_scripts

python --version > NUL 2> NUL
if errorlevel 1 (
	echo WARNING: Python isn't installed on this computer. Defines cannot be extracted from build_config.h! And build numbers won't be incremented
	exit
)

set extract_define=python %scripts_folder%/extract_define.py ../build_config.h
for /f "delims=" %%i in ('%extract_define% DEBUG_BUILD') do set DEBUG_BUILD=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_WINDOWS') do set BUILD_WINDOWS=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_LINUX') do set BUILD_LINUX=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_PVDRUNTIME') do set BUILD_PVDRUNTIME=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_PHYSX_FOUNDATION') do set BUILD_PHYSX_FOUNDATION=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_PHYSX_COMMON') do set BUILD_PHYSX_COMMON=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_PHYSX_PVD') do set BUILD_PHYSX_PVD=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_SCENEQUERY') do set BUILD_SCENEQUERY=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_LOWLEVEL') do set BUILD_LOWLEVEL=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_LOWLEVELAABB') do set BUILD_LOWLEVELAABB=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_LOWLEVELDYNAMICS') do set BUILD_LOWLEVELDYNAMICS=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_SIMCONTROLLER') do set BUILD_SIMCONTROLLER=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_PHYSX_TASK') do set BUILD_PHYSX_TASK=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_PHYSX_API') do set BUILD_PHYSX_API=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_PHYSX_EXTENSIONS') do set BUILD_PHYSX_EXTENSIONS=%%i
for /f "delims=" %%i in ('%extract_define% BUILD_STATIC_LIB') do set BUILD_STATIC_LIB=%%i

:: set VSCMD_DEBUG=3
:: NOTE: Uncomment or change one of these lines to match your installation of Visual Studio compiler
:: call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64
:: call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" amd64 -no_logo
call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64 > NUL

set win_compiler_flags=/c /nologo /I"%physx_folder%/include" /FC
set win_compiler_flags=%win_compiler_flags% /W4 /wd"4514" /wd"4820" /wd"4127" /wd"4710" /wd"4711" /wd"4577" /wd"4996"
set win_compiler_flags=%win_compiler_flags% /DPX_PHYSX_STATIC_LIB /DPX_DEBUG /DPX_PHYSX_FOUNDATION_EXPORTS /D"PX_PUBLIC_RELEASE=1" /D"PX_NVTX=0" /D"PX_SUPPORT_PVD=1" /D"PX_SUPPORT_OMNI_PVD=1" /DPhysXFoundation_EXPORTS

set linux_common_compiler_flags=-c

if "%DEBUG_BUILD%"=="1" (
	set win_compiler_flags=%win_compiler_flags% /Zi /Od /MDd /D_DEBUG /D"PX_DEBUG=1" /D"PX_CHECKED=1"
	set linux_common_compiler_flags=%linux_common_compiler_flags% -D_DEBUG -D"PX_DEBUG=1" -D"PX_CHECKED=1"
) else (
	set win_compiler_flags=%win_compiler_flags% /O2 /MD /DNDEBUG /D"PX_DEBUG=0" /D"PX_CHECKED=0"
	set linux_common_compiler_flags=%linux_common_compiler_flags% -DNDEBUG -D"PX_DEBUG=0" -D"PX_CHECKED=0"
)

:: +--------------------------------------------------------------+
:: |                       Build PvdRuntime                       |
:: +--------------------------------------------------------------+
set pvdruntime_folder=%physx_folder%/pvdruntime/src
set pvdruntime_cpp_files=%pvdruntime_folder%/OmniPvdFileReadStreamImpl.cpp
set pvdruntime_cpp_files=%pvdruntime_cpp_files%,%pvdruntime_folder%/OmniPvdFileWriteStreamImpl.cpp
set pvdruntime_cpp_files=%pvdruntime_cpp_files%,%pvdruntime_folder%/OmniPvdHelpers.cpp
set pvdruntime_cpp_files=%pvdruntime_cpp_files%,%pvdruntime_folder%/OmniPvdLibraryFunctionsImpl.cpp
set pvdruntime_cpp_files=%pvdruntime_cpp_files%,%pvdruntime_folder%/OmniPvdLog.cpp
set pvdruntime_cpp_files=%pvdruntime_cpp_files%,%pvdruntime_folder%/OmniPvdMemoryReadStreamImpl.cpp
set pvdruntime_cpp_files=%pvdruntime_cpp_files%,%pvdruntime_folder%/OmniPvdMemoryStreamImpl.cpp
set pvdruntime_cpp_files=%pvdruntime_cpp_files%,%pvdruntime_folder%/OmniPvdMemoryWriteStreamImpl.cpp
set pvdruntime_cpp_files=%pvdruntime_cpp_files%,%pvdruntime_folder%/OmniPvdReaderImpl.cpp
set pvdruntime_cpp_files=%pvdruntime_cpp_files%,%pvdruntime_folder%/OmniPvdWriterImpl.cpp

if "%BUILD_PVDRUNTIME%"=="1" (
	echo [Building PvdRuntime .objs...]
)
set win_pvdruntime_object_files=
for %%y in ("%pvdruntime_cpp_files:,=" "%") do (
	set object_path=%%~ny%.obj
	set source_path=%%~y%
	if "%BUILD_PVDRUNTIME%"=="1" (
		if "%BUILD_WINDOWS%"=="1" (
			cl %win_compiler_flags% /I"%physx_folder%/pvdruntime/include" /Fd"PhysX_static_64.pdb" /Fo!object_path! !source_path!
		)
	)
	set win_pvdruntime_object_files=!win_pvdruntime_object_files! !object_path!
)

:: +--------------------------------------------------------------+
:: |                       Build Foundation                       |
:: +--------------------------------------------------------------+
set foundation_folder=%physx_folder%/source/foundation
set foundation_cpp_files=%foundation_folder%/FdFoundation.cpp
set foundation_cpp_files=%foundation_cpp_files%,%foundation_folder%/FdAllocator.cpp
set foundation_cpp_files=%foundation_cpp_files%,%foundation_folder%/FdAssert.cpp
set foundation_cpp_files=%foundation_cpp_files%,%foundation_folder%/FdMathUtils.cpp
set foundation_cpp_files=%foundation_cpp_files%,%foundation_folder%/FdString.cpp
set foundation_cpp_files=%foundation_cpp_files%,%foundation_folder%/FdTempAllocator.cpp

set win_foundation_cpp_files=%foundation_cpp_files%,%foundation_folder%/windows/FdWindowsAtomic.cpp
set win_foundation_cpp_files=%win_foundation_cpp_files%,%foundation_folder%/windows/FdWindowsFPU.cpp
set win_foundation_cpp_files=%win_foundation_cpp_files%,%foundation_folder%/windows/FdWindowsMutex.cpp
set win_foundation_cpp_files=%win_foundation_cpp_files%,%foundation_folder%/windows/FdWindowsPrintString.cpp
set win_foundation_cpp_files=%win_foundation_cpp_files%,%foundation_folder%/windows/FdWindowsSList.cpp
set win_foundation_cpp_files=%win_foundation_cpp_files%,%foundation_folder%/windows/FdWindowsSocket.cpp
set win_foundation_cpp_files=%win_foundation_cpp_files%,%foundation_folder%/windows/FdWindowsSync.cpp
set win_foundation_cpp_files=%win_foundation_cpp_files%,%foundation_folder%/windows/FdWindowsThread.cpp
set win_foundation_cpp_files=%win_foundation_cpp_files%,%foundation_folder%/windows/FdWindowsTime.cpp

set linux_foundation_cpp_files=%foundation_cpp_files%,%foundation_folder%/unix/FdUnixAtomic.cpp
set linux_foundation_cpp_files=%linux_foundation_cpp_files%,%foundation_folder%/unix/FdUnixFPU.cpp
set linux_foundation_cpp_files=%linux_foundation_cpp_files%,%foundation_folder%/unix/FdUnixMutex.cpp
set linux_foundation_cpp_files=%linux_foundation_cpp_files%,%foundation_folder%/unix/FdUnixPrintString.cpp
set linux_foundation_cpp_files=%linux_foundation_cpp_files%,%foundation_folder%/unix/FdUnixSList.cpp
set linux_foundation_cpp_files=%linux_foundation_cpp_files%,%foundation_folder%/unix/FdUnixSocket.cpp
set linux_foundation_cpp_files=%linux_foundation_cpp_files%,%foundation_folder%/unix/FdUnixSync.cpp
set linux_foundation_cpp_files=%linux_foundation_cpp_files%,%foundation_folder%/unix/FdUnixThread.cpp
set linux_foundation_cpp_files=%linux_foundation_cpp_files%,%foundation_folder%/unix/FdUnixTime.cpp

if "%BUILD_PHYSX_FOUNDATION%"=="1" (
	echo [Building PhysXFoundation .objs...]
)
set win_foundation_object_files=
for %%y in ("%win_foundation_cpp_files:,=" "%") do (
	set object_path=%%~ny%.obj
	set source_path=%%~y%
	if "%BUILD_PHYSX_FOUNDATION%"=="1" (
		if "%BUILD_WINDOWS%"=="1" (
			cl %win_compiler_flags% /Fd"PhysX_static_64.pdb" /Fo!object_path! !source_path!
		)
	)
	set win_foundation_object_files=!win_foundation_object_files! !object_path!
)

:: +--------------------------------------------------------------+
:: |                         Build Common                         |
:: +--------------------------------------------------------------+
set common_folder=%physx_folder%/source/common/src
set common_cpp_files=%common_folder%/CmCollection.cpp
set common_cpp_files=%common_cpp_files%,%common_folder%/CmPtrTable.cpp
set common_cpp_files=%common_cpp_files%,%common_folder%/CmRadixSort.cpp
set common_cpp_files=%common_cpp_files%,%common_folder%/CmSerialize.cpp
set common_cpp_files=%common_cpp_files%,%common_folder%/CmVisualization.cpp

:: NOTE: This list got too log for a batch for loop to parse, so we split it and removed the %geomutils_folder%/ portion, adding that to each file name during the loop instead
set geomutils_folder=%physx_folder%/source/geomutils/src
set geomutils_cpp_files=GuSweepTests.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,GuWindingNumber.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,GuAABBPruner.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,GuAABBTree.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,GuAABBTreeUpdateMap.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,GuActorShapeMap.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,GuBounds.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,GuBox.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,GuBucketPruner.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,GuBVH.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,GuCapsule.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,GuCCTSweepTests.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,GuConvexGeometry.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,GuConvexSupport.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,GuExtendedBucketPruner.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,GuFactory.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,GuGeometryQuery.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,GuGjkQuery.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,GuIncrementalAABBPruner.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,GuIncrementalAABBPrunerCore.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,GuIncrementalAABBTree.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,GuInternal.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,GuMaverickNode.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,GuMeshFactory.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,GuMetaData.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,GuMTD.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,GuOverlapTests.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,GuPruningPool.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,GuQuerySystem.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,GuRaycastTests.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,GuSAH.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,GuSDF.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,GuSecondaryPruner.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,GuSqInternal.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,GuSweepMTD.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,GuSweepSharedTests.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,ccd/GuCCDSweepPrimitives.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,ccd/GuCCDSweepConvexMesh.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,common/GuVertexReducer.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,common/GuAdjacencies.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,common/GuBarycentricCoordinates.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,common/GuEdgeList.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,common/GuMeshAnalysis.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,common/GuMeshCleaner.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,common/GuQuantizer.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,common/GuSeparatingAxes.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,contact/GuContactSphereBox.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,contact/GuContactSphereCapsule.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,contact/GuContactSphereMesh.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,contact/GuContactSpherePlane.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,contact/GuContactSphereSphere.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,contact/GuFeatureCode.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,contact/GuContactBoxBox.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,contact/GuContactCapsuleBox.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,contact/GuContactCapsuleCapsule.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,contact/GuContactCapsuleConvex.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,contact/GuContactCapsuleMesh.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,contact/GuContactConvexConvex.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,contact/GuContactConvexCoreConvex.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,contact/GuContactConvexCoreMesh.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,contact/GuContactConvexMesh.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,contact/GuContactCustomGeometry.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,contact/GuContactMeshMesh.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,contact/GuContactPlaneBox.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,contact/GuContactPlaneCapsule.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,contact/GuContactPlaneConvex.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,contact/GuContactPlaneConvexCore.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,contact/GuContactPlaneMesh.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,contact/GuContactPolygonPolygon.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,convex/GuHillClimbing.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,convex/GuShapeConvex.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,convex/GuBigConvexData.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,convex/GuConvexHelper.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,convex/GuConvexMesh.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,convex/GuConvexSupportTable.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,convex/GuConvexUtilsInternal.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,cooking/GuCookingTriangleMesh.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,cooking/GuCookingVolumeIntegration.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,cooking/GuRTreeCooking.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,cooking/GuCookingBigConvexDataBuilder.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,cooking/GuCookingBVH.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,cooking/GuCookingConvexHullBuilder.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,cooking/GuCookingConvexHullLib.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,cooking/GuCookingConvexHullUtils.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,cooking/GuCookingConvexMesh.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,cooking/GuCookingConvexMeshBuilder.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,cooking/GuCookingConvexPolygonsBuilder.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,cooking/GuCookingHF.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,cooking/GuCookingQuickHullConvexHullLib.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,cooking/GuCookingSDF.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,cooking/GuCookingTetrahedronMesh.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,distance/GuDistancePointBox.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,distance/GuDistancePointTetrahedron.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,distance/GuDistancePointTriangle.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,distance/GuDistanceSegmentBox.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,distance/GuDistanceSegmentSegment.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,distance/GuDistanceSegmentTriangle.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,distance/GuDistanceTriangleTriangle.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,gjk/GuEPA.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,gjk/GuGJKSimplex.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,gjk/GuGJKTest.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,hf/GuOverlapTestsHF.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,hf/GuSweepsHF.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,hf/GuHeightField.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,hf/GuHeightFieldUtil.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,intersection/GuIntersectionTetrahedronBox.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,intersection/GuIntersectionTriangleBox.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,intersection/GuIntersectionTriangleTriangle.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,intersection/GuIntersectionBoxBox.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,intersection/GuIntersectionCapsuleTriangle.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,intersection/GuIntersectionEdgeEdge.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,intersection/GuIntersectionRayBox.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,intersection/GuIntersectionRayCapsule.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,intersection/GuIntersectionRaySphere.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,intersection/GuIntersectionSphereBox.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,mesh/GuTriangleMesh.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,mesh/GuTriangleMeshBV4.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,mesh/GuTriangleMeshRTree.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,mesh/GuBV4.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,mesh/GuBV4_AABBSweep.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,mesh/GuBV4_BoxOverlap.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,mesh/GuBV4_CapsuleSweep.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,mesh/GuBV4_CapsuleSweepAA.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,mesh/GuBV4_MeshMeshOverlap.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,mesh/GuBV4_OBBSweep.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,mesh/GuBV4_Raycast.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,mesh/GuBV4_SphereOverlap.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,mesh/GuBV4_SphereSweep.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,mesh/GuBV4Build.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,mesh/GuBV32.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,mesh/GuBV32Build.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,mesh/GuMeshQuery.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,mesh/GuMidphaseBV4.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,mesh/GuMidphaseRTree.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,mesh/GuOverlapTestsMesh.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,mesh/GuRTree.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,mesh/GuRTreeQueries.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,mesh/GuSweepsMesh.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,mesh/GuTetrahedronMesh.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,mesh/GuTetrahedronMeshUtils.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,pcm/GuPCMContactGenBoxConvex.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,pcm/GuPCMContactGenSphereCapsule.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,pcm/GuPCMContactGenUtil.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,pcm/GuPCMContactPlaneBox.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,pcm/GuPCMContactPlaneCapsule.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,pcm/GuPCMContactPlaneConvex.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,pcm/GuPCMContactSphereBox.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,pcm/GuPCMContactSphereCapsule.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,pcm/GuPCMContactSphereConvex.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,pcm/GuPCMContactSphereHeightField.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,pcm/GuPCMContactSphereMesh.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,pcm/GuPCMContactSpherePlane.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,pcm/GuPCMContactSphereSphere.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,pcm/GuPCMShapeConvex.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,pcm/GuPCMTriangleContactGen.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,pcm/GuPersistentContactManifold.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,pcm/GuPCMContactBoxBox.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,pcm/GuPCMContactBoxConvex.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,pcm/GuPCMContactCapsuleBox.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,pcm/GuPCMContactCapsuleCapsule.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,pcm/GuPCMContactCapsuleConvex.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,pcm/GuPCMContactCapsuleHeightField.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,pcm/GuPCMContactCapsuleMesh.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,pcm/GuPCMContactConvexCommon.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,pcm/GuPCMContactConvexConvex.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,pcm/GuPCMContactConvexHeightField.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,pcm/GuPCMContactConvexMesh.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,pcm/GuPCMContactCustomGeometry.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,sweep/GuSweepSphereTriangle.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,sweep/GuSweepTriangleUtils.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,sweep/GuSweepBoxBox.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,sweep/GuSweepBoxSphere.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,sweep/GuSweepBoxTriangle_FeatureBased.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,sweep/GuSweepBoxTriangle_SAT.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,sweep/GuSweepCapsuleBox.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,sweep/GuSweepCapsuleCapsule.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,sweep/GuSweepCapsuleTriangle.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,sweep/GuSweepSphereCapsule.cpp
set geomutils_cpp_files=%geomutils_cpp_files%,sweep/GuSweepSphereSphere.cpp

set win_common_cpp_files=%common_cpp_files%,%common_folder%/windows/CmWindowsDelayLoadHook.cpp
set win_common_cpp_files=%win_common_cpp_files%,%common_folder%/windows/CmWindowsModuleUpdateLoader.cpp

set linux_common_cpp_files=%common_cpp_files%

if "%BUILD_PHYSX_COMMON%"=="1" (
	echo [Building PhysXCommon .objs...]
)
set win_common_includes=/I"%physx_folder%/source/common/include"
set win_common_includes=%win_common_includes% /I"%physx_folder%/source/common/src"
set win_common_includes=%win_common_includes% /I"%physx_folder%/source/geomutils/include"
set win_common_includes=%win_common_includes% /I"%physx_folder%/source/geomutils/src"
set win_common_includes=%win_common_includes% /I"%physx_folder%/source/geomutils/src/common"
set win_common_includes=%win_common_includes% /I"%physx_folder%/source/geomutils/src/contact"
set win_common_includes=%win_common_includes% /I"%physx_folder%/source/geomutils/src/pcm"
set win_common_includes=%win_common_includes% /I"%physx_folder%/source/geomutils/src/mesh"
set win_common_includes=%win_common_includes% /I"%physx_folder%/source/geomutils/src/hf"
set win_common_includes=%win_common_includes% /I"%physx_folder%/source/geomutils/src/intersection"
set win_common_includes=%win_common_includes% /I"%physx_folder%/source/geomutils/src/gjk"
set win_common_includes=%win_common_includes% /I"%physx_folder%/source/geomutils/src/distance"
set win_common_includes=%win_common_includes% /I"%physx_folder%/source/geomutils/src/convex"
set win_common_includes=%win_common_includes% /I"%physx_folder%/source/geomutils/src/sweep"
set win_common_object_files=
for %%y in ("%win_common_cpp_files:,=" "%") do (
	set object_path=%%~ny%.obj
	set source_path=%%~y%
	if "%BUILD_PHYSX_COMMON%"=="1" (
		if "%BUILD_WINDOWS%"=="1" (
			cl %win_compiler_flags% %win_common_includes% /Fd"PhysX_static_64.pdb" /Fo!object_path! !source_path!
		)
)
	set win_common_object_files=!win_common_object_files! !object_path!
)
for %%y in ("%geomutils_cpp_files:,=" "%") do (
	set object_path=%%~ny%.obj
	set source_path=%geomutils_folder%/%%~y%
	if "%BUILD_PHYSX_COMMON%"=="1" (
		if "%BUILD_WINDOWS%"=="1" (
			cl %win_compiler_flags% %win_common_includes% /Fd"PhysX_static_64.pdb" /Fo!object_path! !source_path!
		)
	)
	set win_common_object_files=!win_common_object_files! !object_path!
)

:: +--------------------------------------------------------------+
:: |                     Build Physx Pvd SDK                      |
:: +--------------------------------------------------------------+
set pvd_folder=%physx_folder%/source/pvd/src
set pvd_cpp_files=%pvd_folder%/PxProfileEventImpl.cpp
set pvd_cpp_files=%pvd_cpp_files%,%pvd_folder%/PxPvd.cpp
set pvd_cpp_files=%pvd_cpp_files%,%pvd_folder%/PxPvdDataStream.cpp
set pvd_cpp_files=%pvd_cpp_files%,%pvd_folder%/PxPvdDefaultFileTransport.cpp
set pvd_cpp_files=%pvd_cpp_files%,%pvd_folder%/PxPvdDefaultSocketTransport.cpp
set pvd_cpp_files=%pvd_cpp_files%,%pvd_folder%/PxPvdImpl.cpp
set pvd_cpp_files=%pvd_cpp_files%,%pvd_folder%/PxPvdMemClient.cpp
set pvd_cpp_files=%pvd_cpp_files%,%pvd_folder%/PxPvdObjectModelMetaData.cpp
set pvd_cpp_files=%pvd_cpp_files%,%pvd_folder%/PxPvdObjectRegistrar.cpp
set pvd_cpp_files=%pvd_cpp_files%,%pvd_folder%/PxPvdProfileZoneClient.cpp
set pvd_cpp_files=%pvd_cpp_files%,%pvd_folder%/PxPvdUserRenderer.cpp

if "%BUILD_PHYSX_PVD%"=="1" (
	echo [Building PhysXPvdSDK .objs...]
)
set win_pvd_includes=/I"%physx_folder%/source/pvd/include"
set win_pvd_includes=%win_pvd_includes% /I"%physx_folder%/source/filebuf/include"
set win_pvd_object_files=
for %%y in ("%pvd_cpp_files:,=" "%") do (
	set object_path=%%~ny%.obj
	set source_path=%%~y%
	if "%BUILD_PHYSX_PVD%"=="1" (
		if "%BUILD_WINDOWS%"=="1" (
			cl %win_compiler_flags% %win_pvd_includes% /Fd"PhysX_static_64.pdb" /Fo!object_path! !source_path!
		)
	)
	set win_pvd_object_files=!win_pvd_object_files! !object_path!
)

:: +--------------------------------------------------------------+
:: |                       Build SceneQuery                       |
:: +--------------------------------------------------------------+
set scene_query_folder=%physx_folder%/source/scenequery/src
set scene_query_cpp_files=%scene_query_folder%/SqQuery.cpp
set scene_query_cpp_files=%scene_query_cpp_files%,%scene_query_folder%/SqCompoundPruner.cpp
set scene_query_cpp_files=%scene_query_cpp_files%,%scene_query_folder%/SqCompoundPruningPool.cpp
set scene_query_cpp_files=%scene_query_cpp_files%,%scene_query_folder%/SqFactory.cpp
set scene_query_cpp_files=%scene_query_cpp_files%,%scene_query_folder%/SqManager.cpp

if "%BUILD_SCENEQUERY%"=="1" (
	echo [Building SceneQuery .objs...]
)
set win_scene_query_includes=/I"%physx_folder%/source/scenequery/include"
set win_scene_query_includes=%win_scene_query_includes% /I"%physx_folder%/source/common/src"
set win_scene_query_includes=%win_scene_query_includes% /I"%physx_folder%/source/geomutils/include"
set win_scene_query_includes=%win_scene_query_includes% /I"%physx_folder%/source/geomutils/src"
set win_scene_query_includes=%win_scene_query_includes% /I"%physx_folder%/source/geomutils/src/intersection"
set win_scene_query_object_files=
for %%y in ("%scene_query_cpp_files:,=" "%") do (
	set object_path=%%~ny%.obj
	set source_path=%%~y%
	if "%BUILD_SCENEQUERY%"=="1" (
		if "%BUILD_WINDOWS%"=="1" (
			cl %win_compiler_flags% %win_scene_query_includes% /Fd"PhysX_static_64.pdb" /Fo!object_path! !source_path!
		)
	)
	set win_scene_query_object_files=!win_scene_query_object_files! !object_path!
)

:: +--------------------------------------------------------------+
:: |                        Build LowLevel                        |
:: +--------------------------------------------------------------+
set low_level_folder=%physx_folder%/source/lowlevel
set low_level_cpp_files=%low_level_folder%/api/src/px_globals.cpp
set low_level_cpp_files=%low_level_cpp_files%,%low_level_folder%/common/src/pipeline/PxcNpMemBlockPool.cpp
set low_level_cpp_files=%low_level_cpp_files%,%low_level_folder%/common/src/pipeline/PxcNpThreadContext.cpp
set low_level_cpp_files=%low_level_cpp_files%,%low_level_folder%/common/src/pipeline/PxcContactCache.cpp
set low_level_cpp_files=%low_level_cpp_files%,%low_level_folder%/common/src/pipeline/PxcContactMethodImpl.cpp
set low_level_cpp_files=%low_level_cpp_files%,%low_level_folder%/common/src/pipeline/PxcMaterialMethodImpl.cpp
set low_level_cpp_files=%low_level_cpp_files%,%low_level_folder%/common/src/pipeline/PxcNpBatch.cpp
set low_level_cpp_files=%low_level_cpp_files%,%low_level_folder%/common/src/pipeline/PxcNpCacheStreamPair.cpp
set low_level_cpp_files=%low_level_cpp_files%,%low_level_folder%/common/src/pipeline/PxcNpContactPrepShared.cpp
set low_level_cpp_files=%low_level_cpp_files%,%low_level_folder%/software/src/PxsNphaseImplementationContext.cpp
set low_level_cpp_files=%low_level_cpp_files%,%low_level_folder%/software/src/PxsSimpleIslandManager.cpp
set low_level_cpp_files=%low_level_cpp_files%,%low_level_folder%/software/src/PxsCCD.cpp
set low_level_cpp_files=%low_level_cpp_files%,%low_level_folder%/software/src/PxsContactManager.cpp
set low_level_cpp_files=%low_level_cpp_files%,%low_level_folder%/software/src/PxsContext.cpp
set low_level_cpp_files=%low_level_cpp_files%,%low_level_folder%/software/src/PxsDefaultMemoryManager.cpp
set low_level_cpp_files=%low_level_cpp_files%,%low_level_folder%/software/src/PxsIslandSim.cpp

if "%BUILD_LOWLEVEL%"=="1" (
	echo [Building LowLevel .objs...]
)
set win_low_level_includes=/I"%physx_folder%/source/lowlevel/api/include"
set win_low_level_includes=%win_low_level_includes% /I"%physx_folder%/source/lowlevel/common/include/collision"
set win_low_level_includes=%win_low_level_includes% /I"%physx_folder%/source/lowlevel/common/include/pipeline"
set win_low_level_includes=%win_low_level_includes% /I"%physx_folder%/source/lowlevel/common/include/utils"
set win_low_level_includes=%win_low_level_includes% /I"%physx_folder%/source/lowlevel/software/include"
set win_low_level_includes=%win_low_level_includes% /I"%physx_folder%/source/lowleveldynamics/include"
set win_low_level_includes=%win_low_level_includes% /I"%physx_folder%/source/common/src"
set win_low_level_includes=%win_low_level_includes% /I"%physx_folder%/source/geomutils/include"
set win_low_level_includes=%win_low_level_includes% /I"%physx_folder%/source/geomutils/src"
set win_low_level_includes=%win_low_level_includes% /I"%physx_folder%/source/geomutils/src/ccd"
set win_low_level_includes=%win_low_level_includes% /I"%physx_folder%/source/geomutils/src/contact"
set win_low_level_includes=%win_low_level_includes% /I"%physx_folder%/source/geomutils/src/convex"
set win_low_level_includes=%win_low_level_includes% /I"%physx_folder%/source/geomutils/src/hf"
set win_low_level_includes=%win_low_level_includes% /I"%physx_folder%/source/geomutils/src/mesh"
set win_low_level_includes=%win_low_level_includes% /I"%physx_folder%/source/geomutils/src/pcm"
set win_low_level_includes=%win_low_level_includes% /I"%physx_folder%/source/physxgpu/include"
set win_low_level_object_files=
for %%y in ("%low_level_cpp_files:,=" "%") do (
	set object_path=%%~ny%.obj
	set source_path=%%~y%
	if "%BUILD_LOWLEVEL%"=="1" (
		if "%BUILD_WINDOWS%"=="1" (
			cl %win_compiler_flags% %win_low_level_includes% /Fd"PhysX_static_64.pdb" /Fo!object_path! !source_path!
		)
	)
	set win_low_level_object_files=!win_low_level_object_files! !object_path!
)

:: +--------------------------------------------------------------+
:: |                      Build LowLevelAABB                      |
:: +--------------------------------------------------------------+
set low_level_aabb_folder=%physx_folder%/source/lowlevelaabb/src
set low_level_aabb_cpp_files=%low_level_aabb_folder%/BpBroadPhaseABP.cpp
set low_level_aabb_cpp_files=%low_level_aabb_cpp_files%,%low_level_aabb_folder%/BpBroadPhaseMBP.cpp
set low_level_aabb_cpp_files=%low_level_aabb_cpp_files%,%low_level_aabb_folder%/BpBroadPhaseSap.cpp
set low_level_aabb_cpp_files=%low_level_aabb_cpp_files%,%low_level_aabb_folder%/BpBroadPhaseSapAux.cpp
set low_level_aabb_cpp_files=%low_level_aabb_cpp_files%,%low_level_aabb_folder%/BpBroadPhaseShared.cpp
set low_level_aabb_cpp_files=%low_level_aabb_cpp_files%,%low_level_aabb_folder%/BpBroadPhaseUpdate.cpp
set low_level_aabb_cpp_files=%low_level_aabb_cpp_files%,%low_level_aabb_folder%/BpFiltering.cpp
set low_level_aabb_cpp_files=%low_level_aabb_cpp_files%,%low_level_aabb_folder%/BpAABBManager.cpp
set low_level_aabb_cpp_files=%low_level_aabb_cpp_files%,%low_level_aabb_folder%/BpAABBManagerBase.cpp
set low_level_aabb_cpp_files=%low_level_aabb_cpp_files%,%low_level_aabb_folder%/BpBroadPhase.cpp

if "%BUILD_LOWLEVELAABB%"=="1" (
	echo [Building LowLevelAABB .objs...]
)
set win_low_level_aabb_includes=/I"%physx_folder%/source/lowlevelaabb/include"
set win_low_level_aabb_includes=%win_low_level_aabb_includes% /I"%physx_folder%/source/common/src"
set win_low_level_aabb_includes=%win_low_level_aabb_includes% /I"%physx_folder%/source/geomutils/include"
set win_low_level_aabb_includes=%win_low_level_aabb_includes% /I"%physx_folder%/source/geomutils/src"
set win_low_level_aabb_includes=%win_low_level_aabb_includes% /I"%physx_folder%/source/lowlevel/api/include"
set win_low_level_aabb_includes=%win_low_level_aabb_includes% /I"%physx_folder%/source/lowlevel/common/include/utils"
set win_low_level_aabb_object_files=
for %%y in ("%low_level_aabb_cpp_files:,=" "%") do (
	set object_path=%%~ny%.obj
	set source_path=%%~y%
	if "%BUILD_LOWLEVELAABB%"=="1" (
		if "%BUILD_WINDOWS%"=="1" (
			cl %win_compiler_flags% %win_low_level_aabb_includes% /Fd"PhysX_static_64.pdb" /Fo!object_path! !source_path!
		)
	)
	set win_low_level_aabb_object_files=!win_low_level_aabb_object_files! !object_path!
)

:: +--------------------------------------------------------------+
:: |                    Build LowLevelDynamics                    |
:: +--------------------------------------------------------------+
set low_level_dyn_folder=%physx_folder%/source/lowleveldynamics
set low_level_dyn_cpp_files=%low_level_dyn_folder%/src/DyThresholdTable.cpp
set low_level_dyn_cpp_files=%low_level_dyn_cpp_files%,%low_level_dyn_folder%/src/DyAllocator.cpp
set low_level_dyn_cpp_files=%low_level_dyn_cpp_files%,%low_level_dyn_folder%/src/DyArticulationContactPrep.cpp
set low_level_dyn_cpp_files=%low_level_dyn_cpp_files%,%low_level_dyn_folder%/src/DyArticulationContactPrepPF.cpp
set low_level_dyn_cpp_files=%low_level_dyn_cpp_files%,%low_level_dyn_folder%/src/DyArticulationMimicJoint.cpp
set low_level_dyn_cpp_files=%low_level_dyn_cpp_files%,%low_level_dyn_folder%/src/DyConstraintPartition.cpp
set low_level_dyn_cpp_files=%low_level_dyn_cpp_files%,%low_level_dyn_folder%/src/DyConstraintSetup.cpp
set low_level_dyn_cpp_files=%low_level_dyn_cpp_files%,%low_level_dyn_folder%/src/DyConstraintSetupBlock.cpp
set low_level_dyn_cpp_files=%low_level_dyn_cpp_files%,%low_level_dyn_folder%/src/DyContactPrep.cpp
set low_level_dyn_cpp_files=%low_level_dyn_cpp_files%,%low_level_dyn_folder%/src/DyContactPrep4.cpp
set low_level_dyn_cpp_files=%low_level_dyn_cpp_files%,%low_level_dyn_folder%/src/DyContactPrep4PF.cpp
set low_level_dyn_cpp_files=%low_level_dyn_cpp_files%,%low_level_dyn_folder%/src/DyContactPrepPF.cpp
set low_level_dyn_cpp_files=%low_level_dyn_cpp_files%,%low_level_dyn_folder%/src/DyDynamics.cpp
set low_level_dyn_cpp_files=%low_level_dyn_cpp_files%,%low_level_dyn_folder%/src/DyFeatherstoneArticulation.cpp
set low_level_dyn_cpp_files=%low_level_dyn_cpp_files%,%low_level_dyn_folder%/src/DyFeatherstoneForwardDynamic.cpp
set low_level_dyn_cpp_files=%low_level_dyn_cpp_files%,%low_level_dyn_folder%/src/DyFeatherstoneInverseDynamic.cpp
set low_level_dyn_cpp_files=%low_level_dyn_cpp_files%,%low_level_dyn_folder%/src/DyFrictionCorrelation.cpp
set low_level_dyn_cpp_files=%low_level_dyn_cpp_files%,%low_level_dyn_folder%/src/DyRigidBodyToSolverBody.cpp
set low_level_dyn_cpp_files=%low_level_dyn_cpp_files%,%low_level_dyn_folder%/src/DySleep.cpp
set low_level_dyn_cpp_files=%low_level_dyn_cpp_files%,%low_level_dyn_folder%/src/DySolverConstraints.cpp
set low_level_dyn_cpp_files=%low_level_dyn_cpp_files%,%low_level_dyn_folder%/src/DySolverConstraintsBlock.cpp
set low_level_dyn_cpp_files=%low_level_dyn_cpp_files%,%low_level_dyn_folder%/src/DySolverControl.cpp
set low_level_dyn_cpp_files=%low_level_dyn_cpp_files%,%low_level_dyn_folder%/src/DySolverControlPF.cpp
set low_level_dyn_cpp_files=%low_level_dyn_cpp_files%,%low_level_dyn_folder%/src/DySolverCore.cpp
set low_level_dyn_cpp_files=%low_level_dyn_cpp_files%,%low_level_dyn_folder%/src/DySolverPFConstraints.cpp
set low_level_dyn_cpp_files=%low_level_dyn_cpp_files%,%low_level_dyn_folder%/src/DySolverPFConstraintsBlock.cpp
set low_level_dyn_cpp_files=%low_level_dyn_cpp_files%,%low_level_dyn_folder%/src/DyTGSContactPrep.cpp
set low_level_dyn_cpp_files=%low_level_dyn_cpp_files%,%low_level_dyn_folder%/src/DyTGSContactPrepBlock.cpp
set low_level_dyn_cpp_files=%low_level_dyn_cpp_files%,%low_level_dyn_folder%/src/DyTGSDynamics.cpp
set low_level_dyn_cpp_files=%low_level_dyn_cpp_files%,%low_level_dyn_folder%/src/DyThreadContext.cpp

if "%BUILD_LOWLEVELDYNAMICS%"=="1" (
	echo [Building LowLevelDynamics .objs...]
)
set win_low_level_dyn_includes=/I"%physx_folder%/source/lowleveldynamics/include"
set win_low_level_dyn_includes=%win_low_level_dyn_includes% /I"%physx_folder%/source/common/src"
set win_low_level_dyn_includes=%win_low_level_dyn_includes% /I"%physx_folder%/source/geomutils/include"
set win_low_level_dyn_includes=%win_low_level_dyn_includes% /I"%physx_folder%/source/geomutils/src"
set win_low_level_dyn_includes=%win_low_level_dyn_includes% /I"%physx_folder%/source/geomutils/src/pcm"
set win_low_level_dyn_includes=%win_low_level_dyn_includes% /I"%physx_folder%/source/lowlevel/api/include"
set win_low_level_dyn_includes=%win_low_level_dyn_includes% /I"%physx_folder%/source/lowlevel/common/include/pipeline"
set win_low_level_dyn_includes=%win_low_level_dyn_includes% /I"%physx_folder%/source/lowlevel/common/include/utils"
set win_low_level_dyn_includes=%win_low_level_dyn_includes% /I"%physx_folder%/source/lowlevel/software/include"
set win_low_level_dyn_includes=%win_low_level_dyn_includes% /I"%physx_folder%/source/lowleveldynamics/shared"
set win_low_level_dyn_object_files=
for %%y in ("%low_level_dyn_cpp_files:,=" "%") do (
	set object_path=%%~ny%.obj
	set source_path=%%~y%
	if "%BUILD_LOWLEVELDYNAMICS%"=="1" (
		if "%BUILD_WINDOWS%"=="1" (
			cl %win_compiler_flags% %win_low_level_dyn_includes% /Fd"PhysX_static_64.pdb" /Fo!object_path! !source_path!
		)
	)
	set win_low_level_dyn_object_files=!win_low_level_dyn_object_files! !object_path!
)

:: +--------------------------------------------------------------+
:: |                  Build SimulationController                  |
:: +--------------------------------------------------------------+
set sim_cont_folder=%physx_folder%/source/simulationcontroller/src
set sim_cont_cpp_files=%sim_cont_folder%/ScStaticCore.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScTriggerInteraction.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScVisualize.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScActorCore.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScActorSim.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScArticulationCore.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScArticulationJointCore.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScArticulationJointSim.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScArticulationMimicJointSim.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScArticulationSim.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScArticulationTendonCore.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScArticulationTendonJointCore.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScArticulationTendonSim.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScBodyCore.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScBodySim.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScBroadphase.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScCCD.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScConstraintBreakage.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScConstraintCore.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScConstraintInteraction.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScConstraintSim.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScDeformableSurfaceCore.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScDeformableSurfaceShapeSim.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScDeformableSurfaceSim.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScDeformableVolumeCore.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScDeformableVolumeShapeSim.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScDeformableVolumeSim.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScElementInteractionMarker.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScElementSim.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScFiltering.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScInteraction.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScIterators.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScKinematics.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScMetaData.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScNPhaseCore.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScParticleSystemCore.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScParticleSystemShapeCore.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScParticleSystemShapeSim.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScParticleSystemSim.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScPhysics.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScPipeline.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScRigidCore.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScRigidSim.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScScene.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScShapeCore.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScShapeInteraction.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScShapeSim.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScShapeSimBase.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScSimStats.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScSimulationController.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScSleep.cpp
set sim_cont_cpp_files=%sim_cont_cpp_files%,%sim_cont_folder%/ScSqBoundsManager.cpp

if "%BUILD_SIMCONTROLLER%"=="1" (
	echo [Building SimulationController .objs...]
)
set win_sim_cont_includes=/I"%physx_folder%/source/simulationcontroller/include"
set win_sim_cont_includes=%win_sim_cont_includes% /I"%physx_folder%/source/lowlevel/api/include"
set win_sim_cont_includes=%win_sim_cont_includes% /I"%physx_folder%/source/lowlevel/software/include"
set win_sim_cont_includes=%win_sim_cont_includes% /I"%physx_folder%/source/lowlevel/common/include/pipeline"
set win_sim_cont_includes=%win_sim_cont_includes% /I"%physx_folder%/source/lowlevel/common/include/utils"
set win_sim_cont_includes=%win_sim_cont_includes% /I"%physx_folder%/source/lowlevelaabb/include"
set win_sim_cont_includes=%win_sim_cont_includes% /I"%physx_folder%/source/lowleveldynamics/include"
set win_sim_cont_includes=%win_sim_cont_includes% /I"%physx_folder%/source/lowleveldynamics/shared"
set win_sim_cont_includes=%win_sim_cont_includes% /I"%physx_folder%/source/common/src"
set win_sim_cont_includes=%win_sim_cont_includes% /I"%physx_folder%/source/geomutils/include"
set win_sim_cont_includes=%win_sim_cont_includes% /I"%physx_folder%/source/geomutils/src"
set win_sim_cont_includes=%win_sim_cont_includes% /I"%physx_folder%/source/geomutils/src/ccd"
set win_sim_cont_includes=%win_sim_cont_includes% /I"%physx_folder%/source/geomutils/src/convex"
set win_sim_cont_includes=%win_sim_cont_includes% /I"%physx_folder%/source/geomutils/src/hf"
set win_sim_cont_includes=%win_sim_cont_includes% /I"%physx_folder%/source/geomutils/src/mesh"
set win_sim_cont_includes=%win_sim_cont_includes% /I"%physx_folder%/source/geomutils/src/pcm"
set win_sim_cont_includes=%win_sim_cont_includes% /I"%physx_folder%/source/physxgpu/include"
set win_sim_cont_object_files=
for %%y in ("%sim_cont_cpp_files:,=" "%") do (
	set object_path=%%~ny%.obj
	set source_path=%%~y%
	if "%BUILD_SIMCONTROLLER%"=="1" (
		if "%BUILD_WINDOWS%"=="1" (
			cl %win_compiler_flags% %win_sim_cont_includes% /Fd"PhysX_static_64.pdb" /Fo!object_path! !source_path!
		)
	)
	set win_sim_cont_object_files=!win_sim_cont_object_files! !object_path!
)

:: +--------------------------------------------------------------+
:: |                       Build PhysXTask                        |
:: +--------------------------------------------------------------+
set physx_task_folder=%physx_folder%/source/task/src
set physx_task_cpp_files=%physx_task_folder%/TaskManager.cpp

if "%BUILD_PHYSX_TASK%"=="1" (
	echo [Building PhysXTask .objs...]
)
set win_physx_task_object_files=
for %%y in ("%physx_task_cpp_files:,=" "%") do (
	set object_path=%%~ny%.obj
	set source_path=%%~y%
	if "%BUILD_PHYSX_TASK%"=="1" (
		if "%BUILD_WINDOWS%"=="1" (
			cl %win_compiler_flags% /Fd"PhysX_static_64.pdb" /Fo!object_path! !source_path!
		)
	)
	set win_physx_task_object_files=!win_physx_task_object_files! !object_path!
)

:: +--------------------------------------------------------------+
:: |        Build PhysX API (what would be the main .dll)         |
:: +--------------------------------------------------------------+
set physx_api_folder=%physx_folder%/source/physx/src
set physx_metadata_folder=%physx_folder%/source/physxmetadata
set immediatemode_folder=%physx_folder%/source/immediatemode/src
set physx_api_cpp_files=%physx_api_folder%/NpShapeManager.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/PvdMetaDataPvdBinding.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/PvdPhysicsClient.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/NpActor.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/NpAggregate.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/NpArticulationJointReducedCoordinate.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/NpArticulationLink.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/NpArticulationMimicJoint.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/NpArticulationReducedCoordinate.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/NpArticulationTendon.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/NpBounds.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/NpCheck.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/NpConstraint.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/NpDebugViz.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/NpDeformableAttachment.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/NpDeformableElementFilter.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/NpDeformableSurface.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/NpDeformableSurfaceMaterial.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/NpDeformableVolume.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/NpDeformableVolumeMaterial.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/NpDirectGPUAPI.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/NpFactory.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/NpMaterial.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/NpMetaData.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/NpParticleBuffer.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/NpPBDMaterial.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/NpPBDParticleSystem.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/NpPhysics.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/NpPruningStructure.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/NpPvdSceneClient.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/NpPvdSceneQueryCollector.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/NpRigidDynamic.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/NpRigidStatic.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/NpScene.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/NpSceneFetchResults.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/NpSceneQueries.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/NpSerializerAdapter.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/NpShape.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/gpu/PxPhysXGpuModuleLoader.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/gpu/PxGpu.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/omnipvd/NpOmniPvd.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/omnipvd/NpOmniPvdMetaData.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/omnipvd/NpOmniPvdRegistrationData.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/omnipvd/NpOmniPvdSimulationControllerCallbacks.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/omnipvd/OmniPvdChunkAlloc.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/omnipvd/OmniPvdPxSampler.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_metadata_folder%/core/src/PxAutoGeneratedMetaDataObjects.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_metadata_folder%/core/src/PxMetaDataObjects.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%physx_metadata_folder%/extensions/src/PxExtensionAutoGeneratedMetaDataObjects.cpp
set physx_api_cpp_files=%physx_api_cpp_files%,%immediatemode_folder%/NpImmediateMode.cpp

set win_physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/windows/NpWindowsDelayLoadHook.cpp
set win_physx_api_cpp_files=%win_physx_api_cpp_files%,%physx_api_folder%/device/windows/PhysXIndicatorWindows.cpp

set linux_physx_api_cpp_files=%physx_api_cpp_files%,%physx_api_folder%/device/linux/PhysXIndicatorLinux.cpp

if "%BUILD_PHYSX_API%"=="1" (
	echo [Building PhysX .objs...]
)
set win_physx_api_includes=/I"%physx_folder%/source/common/src"
set win_physx_api_includes=%win_physx_api_includes% /I"%physx_folder%/source/common/include"
set win_physx_api_includes=%win_physx_api_includes% /I"%physx_folder%/source/pvd/include"
set win_physx_api_includes=%win_physx_api_includes% /I"%physx_folder%/pvdruntime/include"
set win_physx_api_includes=%win_physx_api_includes% /I"%physx_folder%/source/scenequery/include"
set win_physx_api_includes=%win_physx_api_includes% /I"%physx_folder%/source/geomutils/include"
set win_physx_api_includes=%win_physx_api_includes% /I"%physx_folder%/source/geomutils/src"
set win_physx_api_includes=%win_physx_api_includes% /I"%physx_folder%/source/geomutils/src/common"
set win_physx_api_includes=%win_physx_api_includes% /I"%physx_folder%/source/geomutils/src/contact"
set win_physx_api_includes=%win_physx_api_includes% /I"%physx_folder%/source/geomutils/src/pcm"
set win_physx_api_includes=%win_physx_api_includes% /I"%physx_folder%/source/geomutils/src/mesh"
set win_physx_api_includes=%win_physx_api_includes% /I"%physx_folder%/source/geomutils/src/hf"
set win_physx_api_includes=%win_physx_api_includes% /I"%physx_folder%/source/geomutils/src/intersection"
set win_physx_api_includes=%win_physx_api_includes% /I"%physx_folder%/source/geomutils/src/gjk"
set win_physx_api_includes=%win_physx_api_includes% /I"%physx_folder%/source/geomutils/src/distance"
set win_physx_api_includes=%win_physx_api_includes% /I"%physx_folder%/source/geomutils/src/convex"
set win_physx_api_includes=%win_physx_api_includes% /I"%physx_folder%/source/geomutils/src/sweep"
set win_physx_api_includes=%win_physx_api_includes% /I"%physx_folder%/source/simulationcontroller/include"
set win_physx_api_includes=%win_physx_api_includes% /I"%physx_folder%/source/simulationcontroller/src"
set win_physx_api_includes=%win_physx_api_includes% /I"%physx_folder%/source/lowlevel/api/include"
set win_physx_api_includes=%win_physx_api_includes% /I"%physx_folder%/source/lowlevel/software/include"
set win_physx_api_includes=%win_physx_api_includes% /I"%physx_folder%/source/lowlevel/common/include/pipeline"
set win_physx_api_includes=%win_physx_api_includes% /I"%physx_folder%/source/lowlevel/common/include/utils"
set win_physx_api_includes=%win_physx_api_includes% /I"%physx_folder%/source/lowleveldynamics/include"
set win_physx_api_includes=%win_physx_api_includes% /I"%physx_folder%/source/lowleveldynamics/shared"
set win_physx_api_includes=%win_physx_api_includes% /I"%physx_folder%/source/lowlevelaabb/include"
set win_physx_api_includes=%win_physx_api_includes% /I"%physx_folder%/source/physxgpu/include"
set win_physx_api_includes=%win_physx_api_includes% /I"%physx_folder%/source/physxmetadata/core/include"
set win_physx_api_includes=%win_physx_api_includes% /I"%physx_folder%/source/physxmetadata/extensions/include"
set win_physx_api_includes=%win_physx_api_includes% /I"%physx_folder%/source/physx/src"
set win_physx_api_includes=%win_physx_api_includes% /I"%physx_folder%/source/physx/src/device"
set win_physx_api_object_files=
for %%y in ("%win_physx_api_cpp_files:,=" "%") do (
	set object_path=%%~ny%.obj
	set source_path=%%~y%
	if "%BUILD_PHYSX_API%"=="1" (
		if "%BUILD_WINDOWS%"=="1" (
			cl %win_compiler_flags% %win_physx_api_includes% /Fd"PhysX_static_64.pdb" /Fo!object_path! !source_path!
		)
	)
	set win_physx_api_object_files=!win_physx_api_object_files! !object_path!
)

:: +--------------------------------------------------------------+
:: |                    Build PhysXExtensions                     |
:: +--------------------------------------------------------------+
set extensions_folder=%physx_folder%/source/physxextensions/src
set extensions_cpp_files=%extensions_folder%/ExtSqManager.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/ExtSqQuery.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/ExtTetMakerExt.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/ExtTetrahedronMeshExt.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/ExtTriangleMeshExt.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/ExtBroadPhase.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/ExtCollection.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/ExtContactJoint.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/ExtConvexCoreExt.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/ExtConvexMeshExt.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/ExtCpuWorkerThread.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/ExtCustomGeometryExt.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/ExtCustomSceneQuerySystem.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/ExtD6Joint.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/ExtD6JointCreate.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/ExtDefaultCpuDispatcher.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/ExtDefaultErrorCallback.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/ExtDefaultSimulationFilterShader.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/ExtDefaultStreams.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/ExtDeformableSkinning.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/ExtDeformableSurfaceExt.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/ExtDeformableVolumeExt.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/ExtDistanceJoint.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/ExtExtensions.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/ExtFixedJoint.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/ExtGearJoint.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/ExtGjkQueryExt.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/ExtJoint.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/ExtMetaData.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/ExtParticleClothCooker.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/ExtParticleExt.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/ExtPrismaticJoint.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/ExtPvd.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/ExtPxStringTable.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/ExtRackAndPinionJoint.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/ExtRaycastCCD.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/ExtRemeshingExt.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/ExtRevoluteJoint.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/ExtRigidActorExt.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/ExtRigidBodyExt.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/ExtSampling.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/ExtSceneQueryExt.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/ExtSceneQuerySystem.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/ExtSimpleFactory.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/ExtSmoothNormals.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/ExtSphericalJoint.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/omnipvd/ExtOmniPvdRegistrationData.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/omnipvd/OmniPvdPxExtensionsSampler.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/serialization/SnSerialization.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/serialization/SnSerializationRegistry.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/serialization/SnSerialUtils.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/serialization/Binary/SnBinaryDeserialization.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/serialization/Binary/SnBinarySerialization.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/serialization/Binary/SnConvX.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/serialization/Binary/SnConvX_Align.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/serialization/Binary/SnConvX_Convert.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/serialization/Binary/SnConvX_Error.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/serialization/Binary/SnConvX_MetaData.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/serialization/Binary/SnConvX_Output.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/serialization/Binary/SnConvX_Union.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/serialization/Binary/SnSerializationContext.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/serialization/Xml/PsFastXml.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/serialization/Xml/SnJointRepXSerializer.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/serialization/Xml/SnRepXCoreSerializer.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/serialization/Xml/SnRepXUpgrader.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/serialization/Xml/SnXmlSerialization.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/tet/ExtRemesher.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/tet/ExtTetSplitting.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/tet/ExtTetUnionFind.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/tet/ExtUtilities.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/tet/ExtVoxelTetrahedralizer.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/tet/ExtBVH.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/tet/ExtDelaunayBoundaryInserter.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/tet/ExtDelaunayTetrahedralizer.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/tet/ExtFastWindingNumber.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/tet/ExtInsideTester.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/tet/ExtMeshSimplificator.cpp
set extensions_cpp_files=%extensions_cpp_files%,%extensions_folder%/tet/ExtOctreeTetrahedralizer.cpp

if "%BUILD_PHYSX_EXTENSIONS%"=="1" (
	echo [Building PhysXExtensions .objs...]
)
set win_extensions_includes=/I"%physx_folder%/source/geomutils/include"
set win_extensions_includes=%win_extensions_includes% /I"%physx_folder%/source/geomutils/src"
set win_extensions_includes=%win_extensions_includes% /I"%physx_folder%/source/geomutils/src/intersection"
set win_extensions_includes=%win_extensions_includes% /I"%physx_folder%/source/geomutils/src/mesh"
set win_extensions_includes=%win_extensions_includes% /I"%physx_folder%/source/common/include"
set win_extensions_includes=%win_extensions_includes% /I"%physx_folder%/source/common/src"
set win_extensions_includes=%win_extensions_includes% /I"%physx_folder%/source/pvd/include"
set win_extensions_includes=%win_extensions_includes% /I"%physx_folder%/source/scenequery/include"
set win_extensions_includes=%win_extensions_includes% /I"%physx_folder%/source/physx/src"
set win_extensions_includes=%win_extensions_includes% /I"%physx_folder%/source/physxmetadata/core/include"
set win_extensions_includes=%win_extensions_includes% /I"%physx_folder%/source/physxmetadata/extensions/include"
set win_extensions_includes=%win_extensions_includes% /I"%physx_folder%/source/physxextensions/src"
set win_extensions_includes=%win_extensions_includes% /I"%physx_folder%/source/physxextensions/src/serialization/File"
set win_extensions_includes=%win_extensions_includes% /I"%physx_folder%/source/physxextensions/src/serialization/Binary"
set win_extensions_includes=%win_extensions_includes% /I"%physx_folder%/source/physxextensions/src/serialization/Xml"
set win_extensions_object_files=
for %%y in ("%extensions_cpp_files:,=" "%") do (
	set object_path=%%~ny%.obj
	set source_path=%%~y%
	if "%BUILD_PHYSX_EXTENSIONS%"=="1" (
		if "%BUILD_WINDOWS%"=="1" (
			cl %win_compiler_flags% %win_extensions_includes% /Fd"PhysX_static_64.pdb" /Fo!object_path! !source_path!
		)
	)
	set win_extensions_object_files=!win_extensions_object_files! !object_path!
)

REM TODO: physxcharacterkinematic?
REM TODO: physxcooking?
REM TODO: physxvehicle?
REM TODO: physxvehicle2?

:: +--------------------------------------------------------------+
:: |                       Build Static Lib                       |
:: +--------------------------------------------------------------+
echo [Building PhysX_static_64.lib...]
set win_all_object_files1=%win_pvdruntime_object_files%
set win_all_object_files1=%win_all_object_files1% %win_foundation_object_files%
set win_all_object_files1=%win_all_object_files1% %win_pvd_object_files%
set win_all_object_files1=%win_all_object_files1% %win_scene_query_object_files%
set win_all_object_files1=%win_all_object_files1% %win_low_level_object_files%
set win_all_object_files1=%win_all_object_files1% %win_low_level_aabb_object_files%
set win_all_object_files1=%win_all_object_files1% %win_low_level_dyn_object_files%
set win_all_object_files1=%win_all_object_files1% %win_sim_cont_object_files%
set win_all_object_files1=%win_all_object_files1% %win_physx_task_object_files%
set win_all_object_files1=%win_all_object_files1% %win_physx_api_object_files%
set win_all_object_files1=%win_all_object_files1% %win_extensions_object_files%
set win_all_object_files2=%win_common_object_files%

:: NOTE: We again ran into line length limits, so we had to split the obj files into two variables and do two LIB calls
LIB %win_all_object_files1% /NOLOGO /MACHINE:X64 /OUT:PhysX_partial_64.lib
LIB PhysX_partial_64.lib %win_all_object_files2% /NOLOGO /MACHINE:X64 /OUT:PhysX_static_64.lib
DEL PhysX_partial_64.lib
