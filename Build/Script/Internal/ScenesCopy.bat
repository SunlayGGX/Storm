echo off

cd /D %~dp0

call BaseStorm.bat

if not exist "%STORM_REPO_ROOT%\Config\Scenes\Original" (
	mkdir "%STORM_REPO_ROOT%\Config\Scenes\Original"
)

echo We would overwrite the scenes configuration by the default templated one.

copy "%STORM_REPO_ROOT%\Config\Template" "%STORM_REPO_ROOT%\Config\Scenes\Original"
