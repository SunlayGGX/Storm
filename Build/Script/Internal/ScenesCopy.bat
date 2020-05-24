echo off

cd /D %~dp0

call BaseStorm.bat

echo We would overwrite the scenes configuration by the default templated one.

set copyCmd=call :makeCopyCmd

%copyCmd% "%STORM_REPO_ROOT%\Config\Template\Scenes" "%STORM_REPO_ROOT%\Config\Custom\Scenes\Original"
%copyCmd% "%STORM_REPO_ROOT%\Config\Template\General" "%STORM_REPO_ROOT%\Config\Custom\General\Original"


exit /B %errorlevel%


:makeCopyCmd
if not errorlevel 1 (
	if not exist "%~2" (
		echo Creating folder "%~2"
		mkdir "%~2"
	)
	copy "%~1" "%~2"
)
