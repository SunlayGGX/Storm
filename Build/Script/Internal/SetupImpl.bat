cd /D %~dp0

SET STORM_DEPENDENCIES=%STORM_REPO_ROOT%\Dependencies

if exist "%STORM_DEPENDENCIES%" rmdir /s /q "%STORM_DEPENDENCIES%"
mkdir "%STORM_DEPENDENCIES%"

if not exist "%STORM_DEPENDENCIES%" (
	echo Cannot recreate "%STORM_DEPENDENCIES%" correctly. We cannot make any dependencies links!
	exit /B 1
)

set makeFolderLink=call :CreateFolderLink



:: List here the junction to be made

%makeFolderLink% "%STORM_DEPENDENCIES%\Boost" "%BOOST_DEPENDENCIES_PATH%"





exit /B %errorlevel%


:CreateFolderLink
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
exit /B errorlevel

