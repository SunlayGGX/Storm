echo off

cd /D %~dp0

set STORM_REPO_ROOT=%~dp0..\..\..
set STORM_BUILD_SCRIPT_ROOT=%STORM_REPO_ROOT%\Build\Script
set STORM_DEPENDENCIES=%STORM_REPO_ROOT%\Dependencies
set STORM_EXECUTABLE_FOLDER=%STORM_REPO_ROOT%\bin

set DevPropertiesSetterBaseFileName=UserSettings

if not exist "%STORM_BUILD_SCRIPT_ROOT%\%DevPropertiesSetterBaseFileName%.bat" (
	echo Copying %STORM_BUILD_SCRIPT_ROOT%\Internal\%DevPropertiesSetterBaseFileName%.bat.tmpl
	copy /y "%STORM_BUILD_SCRIPT_ROOT%\Internal\%DevPropertiesSetterBaseFileName%.bat.tmpl" "%STORM_BUILD_SCRIPT_ROOT%\%DevPropertiesSetterBaseFileName%.bat"
	if errorlevel 1 (
		color 40
		echo Failed to create %STORM_BUILD_SCRIPT_ROOT%\%DevPropertiesSetterBaseFileName%.bat and it doesn't exist. We cannot setup STORM dev root.
	) else (
		color 60
		echo We created user settings file %STORM_BUILD_SCRIPT_ROOT%\%DevPropertiesSetterBaseFileName%.bat with default value. You can customize it as you wish then rerun the Setup.bat.
	)
	pause
	exit %errorlevel%
)

call "%STORM_BUILD_SCRIPT_ROOT%\%DevPropertiesSetterBaseFileName%.bat"