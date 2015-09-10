@echo Run Debug DUMP4...
@set TEMP1=Debug\Dump432.exe
@set TEMP2=Test1.inp
@set TEMP3=tempout.txt
@if NOT EXIST %TEMP1% goto ERR1
@if NOT EXIST %TEMP2% goto ERR2

@echo Running %TEMP1% @%TEMP2%
@echo Running %TEMP1% @%TEMP2% >%TEMP3%
%TEMP1% @%TEMP2% >>%TEMP3%
call np %TEMP3%
@goto END

:ERR1
@echo ERROR: Can NOT locate %TEMP1% file... check name, location...
@goto END

:ERR2
@echo ERROR: Can NOT locate %TEMP2% file... check name, location...
@goto END


:END
