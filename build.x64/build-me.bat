@setlocal
@REM 20170323 - Switch to msvc140
@set VCVERS=14
@set DOINST=0
@set TMPPRJ=dump4
@set TMPLOG=bldlog-1.txt
@set CONTONERR=0
@set BUILD_RELDBG=0

@REM ###########################################
@REM NOTE: Specific install location
@REM ###########################################
@set TMPINST=C:\MDOS

@REM ############################################
@REM NOTE: MSVC %VCVERS% INSTALL LOCATION
@REM Adjust to suit your environment
@REM ##########################################
@set GENERATOR=Visual Studio %VCVERS% Win64
@set VS_PATH=C:\Program Files (x86)\Microsoft Visual Studio %VCVERS%.0
@set VC_BAT=%VS_PATH%\VC\vcvarsall.bat
@if NOT EXIST "%VS_PATH%" goto NOVS
@if NOT EXIST "%VC_BAT%" goto NOBAT
@set BUILD_BITS=%PROCESSOR_ARCHITECTURE%
@IF /i %BUILD_BITS% EQU x86_amd64 (
    @set "RDPARTY_ARCH=x64"
    @set "RDPARTY_DIR=software.x64"
) ELSE (
    @IF /i %BUILD_BITS% EQU amd64 (
        @set "RDPARTY_ARCH=x64"
        @set "RDPARTY_DIR=software.x64"
    ) ELSE (
        @echo Appears system is NOT 'x86_amd64', nor 'amd64'
        @echo Can NOT build the 64-bit version! Aborting
        @exit /b 1
    )
)

@ECHO Setting environment - CALL "%VC_BAT%" %BUILD_BITS%
@CALL "%VC_BAT%" %BUILD_BITS%
@if ERRORLEVEL 1 goto NOSETUP
@set TMPOPTS=
@set TMPOPTS=%TMPOPTS% -DCMAKE_INSTALL_PREFIX:PATH=%TMPINST%
@set TMPOPTS=%TMPOPTS% -G "%GENERATOR%"

@call chkmsvc %TMPPRJ%

@echo Built project %TMPORJ%... all ouput to %TMPLOG%

@set CMOPTS=%TMPOPTS%
@echo Commence build %DATE% at %TIME% > %TMPLOG%

@echo Doing: 'cmake .. %CMOPTS%
@echo Doing: 'cmake .. %CMOPTS% >> %TMPLOG%
@cmake .. %CMOPTS% >> %TMPLOG% 2>&1
@if ERRORLEVEL 1 goto ERR1

@echo Doing: 'cmake --build . --config Debug'
@echo Doing: 'cmake --build . --config Debug' >> %TMPLOG%
@cmake --build . --config Debug >> %TMPLOG% 2>&1
@if ERRORLEVEL 1 goto ERR2

:DNDEBBLD

@if "%BUILD_RELDBG%x" == "0x" goto DNRELDBG
@echo Doing: 'cmake --build . --config RelWithDebInfo'
@echo Doing: 'cmake --build . --config RelWithDebInfo' >> %TMPLOG%
@cmake --build . --config RelWithDebInfo >> %TMPLOG% 2>&1
@if ERRORLEVEL 1 goto ERR3

:DNRELDBG
@echo Doing: 'cmake --build . --config Release'
@echo Doing: 'cmake --build . --config Release' >> %TMPLOG%
@cmake --build . --config Release >> %TMPLOG% 2>&1
@if ERRORLEVEL 1 goto ERR4

:DNRELBLD

@echo.
@echo Appears successful...
@echo.
@IF "%DOINST%x" == "1x" goto DOINST
@echo NO INSTALL at this time... set DOINST=1
@echo.
@goto END

:DOINST
@echo Continue with Release install to %TMPINST%?
@echo.
@pause
@echo Doing: 'cmake --build . --config Release --target INSTALL'
@echo Doing: 'cmake --build . --config Release --target INSTALL' >> %TMPLOG%
@cmake --build . --config Release --target INSTALL >> %TMPLOG% 2>&1
@if ERRORLEVEL 1 goto ERR5

@fa4 " -- " %TMPLOG%

@echo Done build and install of %TMPPRJ% to %TMPINST%

@goto END

:ERR1
@echo FATAL ERROR: cmake configuration/generation FAILED
@echo FATAL ERROR: cmake configuration/generation FAILED >> %TMPLOG%
@goto ISERR

:ERR2
@echo ERROR: cmake build Debug
@echo ERROR: cmake build Debug >> %TMPLOG%
@if %CONTONERR% EQU 1 goto DNDEBBLD
@goto ISERR

:ERR3
@echo ERROR: cmake build RelWithDebInfo
@echo ERROR: cmake build RelWithDebInfo >> %TMPLOG%
@if %CONTONERR% EQU 1 goto DNRELBLD
@goto ISERR

:ERR4
@echo ERROR: cmake build Release
@echo ERROR: cmake build Release >> %TMPLOG%
@goto ISERR

:NOVS
@echo ERROR: Can NOT locate path %VS_PATH%! *** FIX ME *** to where MSVC is installed
@goto ISERR

:NOBAT
@echo ERROR: Can NOT locate batch %VC_BAT%! *** FIX ME *** to where MSVC is installed
@goto ISERR

:NOSETUP
@echo ERROR: Running the %VC_BAT% caused an ERROR! *** FIX ME ***
@goto ISERR

:ISERR
@endlocal
@exit /b 1

:END
@endlocal
@exit /b 0

@REM eof
