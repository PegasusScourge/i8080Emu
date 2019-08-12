@echo off
xcopy ..\Debug\i8080.exe i8080.exe /y /q /i
i8080.exe -l invaders.h 0 -l invaders.g 2048 -l invaders.f 4096 -l invaders.e 6144 --speed 0.1
