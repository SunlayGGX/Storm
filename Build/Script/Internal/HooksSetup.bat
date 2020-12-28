echo off

cd /D %~dp0

call BaseStorm.bat

echo Setting up git hooks.

set copyCmd=call :makeCopyCmd

%copyCmd% "%STORM_BUILD_SCRIPT_ROOT%\Hooks" "%STORM_REPO_ROOT%\.git\hooks"


exit /B %errorlevel%


:makeCopyCmd
if not errorlevel 1 (
	if not exist "%~2" (
		echo Creating folder "%~2"
		mkdir "%~2"
	)
	copy "%~1" "%~2"
)