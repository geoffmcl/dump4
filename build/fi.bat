@setlocal
@if "%~1x" == "x" goto HELP
@set TMPFIND=%~1
@shift
@set TMPCMD=
:RPT
@if "%~1x" == "x" goto GOTCMD
@set TMPCMD=%TMPCMD% %1
@shift
@goto RPT
:GOTCMD

fa4 "%TMPFIND%" ..\* -r -X:build -X::: -b- %TMPCMD%

@goto END

:HELP
@echo Give word or phrase to find in the dump4 source....
@goto END

:END

