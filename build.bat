@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

cd /d "%~dp0"

REM ====== 配置 ======
REM 如果 MinGW 不在 PATH 中，取消下面一行注释并修改路径
REM set "PATH=D:\mingw64\bin;%PATH%"

set "CFLAGS=-O2 -Wall -finput-charset=UTF-8 -fexec-charset=UTF-8"

echo === WASH Build Script (v0.2.3) ===
echo.

where gcc >nul 2>&1
if errorlevel 1 (
    echo [ERROR] gcc not found in PATH.
    echo Please install MinGW-w64 or add it to PATH.
    pause
    exit /b 1
)

REM ====== 编译 Lua 5.1 静态库 ======
echo [1/3] Building Lua 5.1 static library...
if exist lua-src\*.o del /q lua-src\*.o

for %%f in (lapi lcode ldebug ldo ldump lfunc lgc llex lmem loadlib lobject lopcodes lparser lstate lstring ltable ltm lundump lvm lzio lauxlib lbaselib ldblib liolib lmathlib loslib ltablib lstrlib linit) do (
    gcc -O2 -c lua-src\%%f.c -o lua-src\%%f.o -Ilua-src
    if errorlevel 1 (
        echo [ERROR] Failed to compile %%f.c
        pause
        exit /b 1
    )
)

ar rcs lua-src\liblua.a lua-src\*.o
echo Lua library built.

REM ====== 编译 Wash ======
echo.
echo [2/3] Compiling WASH kernel...
if exist src\*.o del /q src\*.o

gcc %CFLAGS% -c src\shell.c -o src\shell.o -Ilua-src
gcc %CFLAGS% -c src\builtin.c -o src\builtin.o -Ilua-src
gcc %CFLAGS% -c src\lua_kernel.c -o src\lua_kernel.o -Ilua-src
gcc %CFLAGS% -c src\main.c -o src\main.o -Ilua-src

gcc %CFLAGS% -o wash.exe src\main.o src\shell.o src\builtin.o src\lua_kernel.o lua-src\liblua.a -lm

if errorlevel 1 (
    echo [ERROR] Link failed.
    pause
    exit /b 1
)

REM ====== 清理临时文件 ======
echo.
echo [3/3] Cleaning up...
del /q src\*.o lua-src\*.o 2>nul

echo.
echo === BUILD SUCCESS ===
echo Output: wash.exe
echo.
echo Put wash.exe in the same directory as wash-modules\ and run it.
pause
endlocal
