echo off
cd %~dp0

call "../Base/LauncherSetup.bat" Debug

call "Storm_d.exe"
