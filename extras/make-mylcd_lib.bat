@echo off 

cd %1

ren mylcd mylcd.def
"../lib.exe" /machine:IX86 /def:mylcd.def

