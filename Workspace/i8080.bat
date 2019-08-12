@echo off
xcopy ..\Debug\i8080.exe i8080.exe /y /q /i
i8080.exe -l invaders.h 0 -l invaders.g 2048 -l invaders.f 4096 -l invaders.e 6144 --speed 4.0 -va 9216 -vd 256 224
