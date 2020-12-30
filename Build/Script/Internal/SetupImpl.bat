cd /D %~dp0

SET STORM_DEPENDENCIES=%STORM_REPO_ROOT%\Dependencies
SET STORM_BINARY=%STORM_REPO_ROOT%\bin
SET STORM_DEBUG_BINARY=%STORM_BINARY%\Debug
SET STORM_RELEASE_BINARY=%STORM_BINARY%\Release

if exist "%STORM_DEPENDENCIES%" rmdir /s /q "%STORM_DEPENDENCIES%"
mkdir "%STORM_DEPENDENCIES%"

if not exist "%STORM_DEPENDENCIES%" (
	echo Cannot recreate "%STORM_DEPENDENCIES%" correctly. We cannot make any dependencies links!
	exit /B 1
)

set makeFolderLink=call :CreateFolderLink
set createFolderIfNeeded=call :CreateFolderIfNoExist
set provideDependencyFile=call :ProvideFileDependency


echo Make dependency junctions

:: List here the junction to be made to dependencies

%makeFolderLink% "%STORM_DEPENDENCIES%\Boost" "%BOOST_DEPENDENCIES_PATH%"
%makeFolderLink% "%STORM_DEPENDENCIES%\OIS" "%OIS_DEPENDENCIES_PATH%"
%makeFolderLink% "%STORM_DEPENDENCIES%\Assimp" "%ASSIMP_DEPENDENCIES_PATH%"
%makeFolderLink% "%STORM_DEPENDENCIES%\PhysX" "%PHYSX_DEPENDENCIES_PATH%"
%makeFolderLink% "%STORM_DEPENDENCIES%\Eigen" "%EIGEN_DEPENDENCIES_PATH%"
%makeFolderLink% "%STORM_DEPENDENCIES%\Lua" "%LUA_BASE_LIB_DEPENDENCIES_PATH%"
%makeFolderLink% "%STORM_DEPENDENCIES%\Sol2" "%LUA_WRAPPER_DEPENDENCIES_PATH%"

:: This one is not mandatory...
if exist "%CATCH2_DEPENDENCIES_PATH%" (
	%makeFolderLink% "%STORM_DEPENDENCIES%\Catch2" "%CATCH2_DEPENDENCIES_PATH%"
)


echo Initialize the dependencies structures to the binary folders.

:: Now, create the junction from the exe (where it should be) to Storm Shaders files.
if not errorlevel 1 (
	%createFolderIfNeeded% "%STORM_BINARY%"
	%createFolderIfNeeded% "%STORM_DEBUG_BINARY%"
	%createFolderIfNeeded% "%STORM_RELEASE_BINARY%"

	%makeFolderLink% "%STORM_DEBUG_BINARY%\Shaders" "%STORM_REPO_ROOT%\Source\Shaders"
	%makeFolderLink% "%STORM_RELEASE_BINARY%\Shaders" "%STORM_REPO_ROOT%\Source\Shaders"
)


set OIS_DEPENDENCY_BINARY_PATH=%STORM_DEPENDENCIES%\OIS\bin
%provideDependencyFile% "%STORM_DEBUG_BINARY%\OIS_d.dll" "%OIS_DEPENDENCY_BINARY_PATH%\Debug\OIS_d.dll"
%provideDependencyFile% "%STORM_DEBUG_BINARY%\OIS_d.pdb" "%OIS_DEPENDENCY_BINARY_PATH%\Debug\OIS_d.pdb"
%provideDependencyFile% "%STORM_RELEASE_BINARY%\OIS.dll" "%OIS_DEPENDENCY_BINARY_PATH%\Release\OIS.dll"

set ASSIMP_DEPENDENCY_BINARY_PATH=%STORM_DEPENDENCIES%\Assimp\bin\code
%provideDependencyFile% "%STORM_DEBUG_BINARY%\assimpd.dll" "%ASSIMP_DEPENDENCY_BINARY_PATH%\Debug\assimpd.dll"
%provideDependencyFile% "%STORM_DEBUG_BINARY%\assimpd.pdb" "%ASSIMP_DEPENDENCY_BINARY_PATH%\Debug\assimpd.pdb"
%provideDependencyFile% "%STORM_RELEASE_BINARY%\assimp.dll" "%ASSIMP_DEPENDENCY_BINARY_PATH%\Release\assimp.dll"

set PHYSX_DEPENDENCY_BINARY_PATH=%STORM_DEPENDENCIES%\PhysX\physx\lib
%provideDependencyFile% "%STORM_DEBUG_BINARY%\PhysXFoundation_64.dll" "%PHYSX_DEPENDENCY_BINARY_PATH%\Debug\PhysXFoundation_64.dll"
%provideDependencyFile% "%STORM_DEBUG_BINARY%\PhysXFoundation_64.pdb" "%PHYSX_DEPENDENCY_BINARY_PATH%\Debug\PhysXFoundation_64.pdb"
%provideDependencyFile% "%STORM_DEBUG_BINARY%\PhysX_64.dll" "%PHYSX_DEPENDENCY_BINARY_PATH%\Debug\PhysX_64.dll"
%provideDependencyFile% "%STORM_DEBUG_BINARY%\PhysX_64.pdb" "%PHYSX_DEPENDENCY_BINARY_PATH%\Debug\PhysX_64.pdb"
%provideDependencyFile% "%STORM_DEBUG_BINARY%\PhysXCommon_64.dll" "%PHYSX_DEPENDENCY_BINARY_PATH%\Debug\PhysXCommon_64.dll"
%provideDependencyFile% "%STORM_DEBUG_BINARY%\PhysXCommon_64.pdb" "%PHYSX_DEPENDENCY_BINARY_PATH%\Debug\PhysXCommon_64.pdb"
%provideDependencyFile% "%STORM_DEBUG_BINARY%\PhysXCooking_64.dll" "%PHYSX_DEPENDENCY_BINARY_PATH%\Debug\PhysXCooking_64.dll"
%provideDependencyFile% "%STORM_DEBUG_BINARY%\PhysXCooking_64.pdb" "%PHYSX_DEPENDENCY_BINARY_PATH%\Debug\PhysXCooking_64.pdb"

%provideDependencyFile% "%STORM_RELEASE_BINARY%\PhysXFoundation_64.dll" "%PHYSX_DEPENDENCY_BINARY_PATH%\Release\PhysXFoundation_64.dll"
%provideDependencyFile% "%STORM_RELEASE_BINARY%\PhysXFoundation_64.pdb" "%PHYSX_DEPENDENCY_BINARY_PATH%\Release\PhysXFoundation_64.pdb"
%provideDependencyFile% "%STORM_RELEASE_BINARY%\PhysX_64.dll" "%PHYSX_DEPENDENCY_BINARY_PATH%\Release\PhysX_64.dll"
%provideDependencyFile% "%STORM_RELEASE_BINARY%\PhysX_64.pdb" "%PHYSX_DEPENDENCY_BINARY_PATH%\Release\PhysX_64.pdb"
%provideDependencyFile% "%STORM_RELEASE_BINARY%\PhysXCommon_64.dll" "%PHYSX_DEPENDENCY_BINARY_PATH%\Release\PhysXCommon_64.dll"
%provideDependencyFile% "%STORM_RELEASE_BINARY%\PhysXCommon_64.pdb" "%PHYSX_DEPENDENCY_BINARY_PATH%\Release\PhysXCommon_64.pdb"
%provideDependencyFile% "%STORM_RELEASE_BINARY%\PhysXCooking_64.dll" "%PHYSX_DEPENDENCY_BINARY_PATH%\Release\PhysXCooking_64.dll"
%provideDependencyFile% "%STORM_RELEASE_BINARY%\PhysXCooking_64.pdb" "%PHYSX_DEPENDENCY_BINARY_PATH%\Release\PhysXCooking_64.pdb"


exit /B %errorlevel%

:CreateFolderIfNoExist
if not errorlevel 1 (
	if not exist "%~1" (
		echo Creating "%~1"
		mkdir "%~1"
	)
)
exit /B errorlevel

:CreateFolderLink
if not errorlevel 1 (
	echo "%~1" to "%~2"
	if not exist "%~2" (
		echo Cannot create "%~1" because the target "%~2" doesn't exist. Aborting!
		set errorlevel=90009
	)
	if exist "%~1" rmdir "%~1"
	mklink /J "%~1" "%~2"
	if errorlevel 1 (
		echo Failed to create junction from "%~1" to "%~2" !
		echo.
	)
	echo.
)
exit /B errorlevel

:: The name is like this because all we must do is provide those dependency file to a specific location.
:: As long as the file are provided, it is unspecified if this is a copy or a junction we make under the hood.
:ProvideFileDependency
if not errorlevel 1 (
	echo "%~1" to "%~2"
	if not exist "%~2" (
		echo Cannot create "%~1" because the target "%~2" doesn't exist. Aborting!
		set errorlevel=90009
	)
	if exist "%~1" del /Q "%~1"
	copy "%~2" "%~1"
	if errorlevel 1 (
		echo Failed to create junction from "%~1" to "%~2" !
		echo.
	)
	echo.
)
exit /B errorlevel
