@echo off

k:
cd K:\code\Teensy\USBHost
call apath.bat

cd %APPATH%
arduino.exe --pref build.path=M:\temp\teensy --board teensy:avr:teensy36:speed=180 --preserve-temp-files --port COM31 --upload K:\code\Teensy\USBHost\USBHost.ino


rem pause --verbose-build 
