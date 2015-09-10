@setlocal
@set TMPEXE=Release\dump432.exe
@if NOT EXIST %TMPEXE% goto NOEXE

@set TMPFIL1=D:\GTools\Tools\dv32\Inst\tempb211.bmp
@if NOT EXIST %TMPFIL1% goto NOFIL1

%TMPEXE% %TMPFIL1% -bmp > tempd2
call np tempd2

@goto END

:NOEXE
@echo Can NOT locate EXE %TMPEXE%
@goto END

:NOFIL1
@echo Can NOT locate file %TMPFIL1%! *** FIX ME *** point to another BMP file
@goto END

:END
