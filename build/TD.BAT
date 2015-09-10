@setlocal
@set TMPFIL1=c:\mssdk\lib\user32.lib
@set TMPFIL2=E:\WINFAX\MACROS\WINFAX.BMP
@set TMPFIL3=F:\GTOOLS32\CPP\Rnb\*.BMP
@set TMPFIL4=test1.inp

@set TMPEXE=Release\dump432.exe
@if NOT EXIST %TMPEXE% goto NOEXE
@echo	Test Dump4 ...

@if "%1." == "b." goto Dobmp
@if "%1." == "B." goto Dobmp
@if "%1." == "1." goto DoTest1
@REM =======================
@rem COMMANDS
@REM Dump4 32-Bits - HEX DUMP UTILITY - 4th July, 2000
@REM Usage   : Dump4 [@]InputFile[s] [Switches]
@REM Switches: Each preceeded by - or / space separated.
@REM  -? or H This brief help.
@REM  -bmp    Dump as a BITMAP file.
@REM  -Bnnn   Begin at offset nnn into file.
@REM  -Ennn   End at offset nnn into file.
@REM  -i[@]Nm Input Name (alternative method).
@REM  -p      Remove parity bit before display.
@REM  -v[n]   Verbosity. -v=-v1 -v0=Silent -vn=Level up to 9.
@REM  -X[oha] Exclude [Offset|Hex|Ascii] display.
@REM Notes  1: Up to 200 Input files can be given.
@REM       2: Input File Names can contains wildcards.
@REM       3: Preceeded with @ means further commands in file.
@REM                                                  Happy Dumping!
@goto Test2

:Dobmp
@REM The BMP functions
@if NOT EXIST %TMPFIL2% goto NOFIL2
@if NOT EXIST %TMPFIL3% goto NOFIL3
%TMPEXE% -bmp %TMPFIL2% > tempdb
%TMPEXE% -v9 -bmp %TMPFIL3% > tempb2
call np tempb2
@goto End

:Test2
@if NOT EXIST %TMPFIL1% goto NOFIL1
%TMPEXE% %TMPFIL1% -xoh > tempd42
call np tempd42
@goto End

:DoTest1
@if NOT EXIST %TMPFIL4% goto NOFIL4
%TMPEXE% @%TMPFIL4% %2 %3 %4 %5 > tempt1.txt
call np tempt1.txt
@goto end

:NOEXE
@echo Error: Can NOT locate EXE %TMPEXE%
@goto END

:NOFIL1
@echo Can NOT locate %TMPFIL1%! *** FIX ME *** Point to another library sample
@goto END
:NOFIL2
@echo Can NOT locate %TMPFIL2%! *** FIX ME *** Point to another bmp sample
@goto END
:NOFIL3
@echo Can NOT locate %TMPFIL3%! *** FIX ME *** Point to another bmp sample
@goto END
:NOFIL4
@echo Can NOT locate %TMPFIL4%! *** FIX ME *** Point to another input file sample
@goto END

:End
