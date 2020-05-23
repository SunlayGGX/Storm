echo off

cd /D %~dp0

call BaseStorm.bat

echo We would overwrite the scenes configuration by the default templated one.

copy "%STORM_REPO_ROOT%\Config\Template" "%STORM_REPO_ROOT%\Config\Scenes\Original"
