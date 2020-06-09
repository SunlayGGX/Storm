cd /D %~dp0

SET STORM_DEPENDENCIES=%STORM_REPO_ROOT%\Dependencies
SET STORM_BINARY=%STORM_REPO_ROOT%\bin

if exist "%STORM_DEPENDENCIES%" rmdir /s /q "%STORM_DEPENDENCIES%"
mkdir "%STORM_DEPENDENCIES%"

if not exist "%STORM_DEPENDENCIES%" (
	echo Cannot recreate "%STORM_DEPENDENCIES%" correctly. We cannot make any dependencies links!
	exit /B 1
)

set makeFolderLink=call :CreateFolderLink



:: List here the junction to be made to dependencies

%makeFolderLink% "%STORM_DEPENDENCIES%\Boost" "%BOOST_DEPENDENCIES_PATH%"
%makeFolderLink% "%STORM_DEPENDENCIES%\OIS" "%OIS_DEPENDENCIES_PATH%"
%makeFolderLink% "%STORM_DEPENDENCIES%\Assimp" "%ASSIMP_DEPENDENCIES_PATH%"
%makeFolderLink% "%STORM_DEPENDENCIES%\PhysX" "%PHYSX_DEPENDENCIES_PATH%"
%makeFolderLink% "%STORM_DEPENDENCIES%\Eigen" "%EIGEN_DEPENDENCIES_PATH%"

:: This one is not mandatory...
if exist "%CATCH2_DEPENDENCIES_PATH%" (
	%makeFolderLink% "%STORM_DEPENDENCIES%\Catch2" "%CATCH2_DEPENDENCIES_PATH%"
)



:: Now, create the junction from the exe (where it should be) to Storm Shaders files.
if not errorlevel 1 (
	if not exist "%STORM_BINARY%" (
		echo Creating "%STORM_BINARY%"
		mkdir "%STORM_BINARY%"
	)

	%makeFolderLink% "%STORM_BINARY%\Shaders" "%STORM_REPO_ROOT%\Source\Shaders"
)


exit /B %errorlevel%


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

