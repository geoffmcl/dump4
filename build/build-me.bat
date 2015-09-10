@setlocal
@set DOINST=0
@set TMPSRC=..
@set TMPDST=C:\MDOS\dump432.exe
@set TMPESRC=Release\dump432.exe
@set TMPCMF=%TMPSRC%\CMakeLists.txt
@if NOT EXIST %TMPCMF% goto ERR1
@set TMPLOG=bldlog-1.txt

@set TMPOPTS=-DCMAKE_INSTALL_PREFIX=C:\MDOS

:RPT
@if "%~1x" == "x" goto GOTCMD
@set TMPOPTS=%TMPOPTS% %1
@shift
@goto RPT
:GOTCMD

@echo Begin dump4 build %DATE% %TIME% > %TMPLOG%

@echo Doing: 'cmake %TMPSRC% %TMPOPTS%'
@echo Doing: 'cmake %TMPSRC% %TMPOPTS%' >> %TMPLOG%
@cmake %TMPSRC% %TMPOPTS% >> %TMPLOG% 2>&1
@if ERRORLEVEL 1 goto CMERR1

@echo Doing: 'cmake --build . --config Debug'
@echo Doing: 'cmake --build . --config Debug' >> %TMPLOG%
@cmake --build . --config Debug >> %TMPLOG% 2>&1
@if ERRORLEVEL 1 goto CMERR2

@echo Doing: 'cmake --build . --config Release'
@echo Doing: 'cmake --build . --config Release' >> %TMPLOG%
@cmake --build . --config Release >> %TMPLOG% 2>&1
@if ERRORLEVEL 1 goto CMERR3
@echo.
@echo Look like a successful build
@echo.
@if "%DOINST%x" == "1x" goto DOINST
@echo No install at this time... set DOINST=1
@echo.
@goto END

:DOINST
@if NOT EXIST %TMPDST% goto DNCHK
@if NOT EXIST %TMPESRC% goto DNCHK
@echo Current
@call dirmin %TMPDST%
@echo New
@call dirmin %TMPESRC%
@fc4 -v0 -q %TMPESRC% %TMPDST% >nul
@if ERRORLEVEL 2 got DNCHK
@if ERRORLEVEL 1 (
@echo Files are different
) else (
@echo.
@echo Files are EXACTLY the SAME... no install needed
@echo.
@goto END
)
:DNCHK
@echo.
@echo Proceed to install to C:\MDOS?
@echo.
@pause

cmake --build . --config Release --target INSTALL

@goto END

:ERR1
@echo ERROR: Can NOT locate file %TMPCMF%! Check name, location, and FIX ME %0
@goto ISERR

:CMERR1
@echo Cmake configuration or generation error...
@goto ISERR

:CMERR2
@echo Cmake build debug error...
@goto ISERR

:CMERR3
@echo Cmake build release error...
@goto ISERR

:ISERR
@endlocal
@exit /b 1

:END
@endlocal
@exit /b 0

@REM eof
