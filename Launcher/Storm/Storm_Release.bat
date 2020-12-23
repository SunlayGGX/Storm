echo off
cd %~dp0

call "../Base/LauncherSetup.bat" Release

call "Storm.exe"
