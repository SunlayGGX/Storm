echo off
cd %~dp0

call "../Base/LauncherSetup.bat"

call "Storm-LogViewer.exe" NoInitialRead
