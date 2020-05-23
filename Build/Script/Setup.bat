echo off

cd /D %~dp0/Internal


echo.
echo ***************************************************************
echo *************** Welcome to Storm project setup ***************
echo ***************************************************************
echo.



call :CallBatch BaseStorm.bat
call :CallBatch SetupImpl.bat
call :CallBatch ScenesCopy.bat



goto Success


:: Function
:CallBatch
echo Call to "%~1"
if not exist "%~1" (
	echo "%1" doesn't exists!
	goto Failure
)
call %~1
if errorlevel 1 (
	echo "%1" run exited with error level %ERRORLEVEL%
	goto Failure
)
exit /B 0



:: Return statement

:Failure
color 40
echo.
echo Error level got : %errorlevel%
echo ****************************************************************
echo ****************************************************************
echo *************************** FAILURE ****************************
echo ********************** See message above ***********************
echo ****************************************************************
echo ****************************************************************
echo.
pause
exit %errorlevel%


:Success
color 20
echo.
echo ****************************************************************
echo ****************************************************************
echo *************************** SUCCESS ****************************
echo ****************************************************************
echo ****************************************************************
echo.
timeout 5
exit 0

