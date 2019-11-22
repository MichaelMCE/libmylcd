@echo off 

call gccpath.bat

del mylcd.def /y

rem ren libmylcd.def mylcd.def
rem lib /machine:IX86 /def:mylcd.def

dlltool -l libmylcd.a -D mylcd.dll -d libmylcd.def
