@echo off
xcopy ..\Debug\i8080.exe i8080.exe /y /q /i
i8080.exe -l CPUTEST.COM 256 --speed 4.0 -va 0 -vd 256 224 --loglevel 2
