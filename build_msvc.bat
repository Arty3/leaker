@echo off
REM Build leaker with MSVC (run from a Developer Command Prompt)

if not exist obj\win mkdir obj\win

cl.exe /W4 /WX /O2 /Fe:leaker.exe /Fo:obj\win\ src\leaker.c
