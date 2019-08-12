@echo off
xcopy ..\Debug\i8080.exe i8080.exe /y /q /i
i8080.exe -l CPUTEST.COM 256 --speed 1.0 -va 9216 -vd 256 224
