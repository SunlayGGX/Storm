echo off
cd %~dp0

call "../Base/LauncherSetup.bat" Profile

call "Storm.exe"
