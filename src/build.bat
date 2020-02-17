@echo off
setlocal
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat" > nul

set SOURCES=main.c
set PROGRAM=reader

if not exist ..\bin mkdir ..\bin

cl -analyze -nologo -W4 -Od -Zi -DMAIN -D_CRT_SECURE_NO_WARNINGS -o ..\bin\%PROGRAM%.exe %SOURCES%
