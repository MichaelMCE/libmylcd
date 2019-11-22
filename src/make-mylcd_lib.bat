@echo off 

rem call gccpath.bat

del mylcd.def /y

ren libmylcddll.def mylcd.def
lib /machine:IX86 /def:mylcd.def

