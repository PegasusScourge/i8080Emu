@echo off
xcopy ..\Debug\i8080.exe i8080.exe /y /q /i
i8080.exe --test
