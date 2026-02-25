@echo off
setlocal enabledelayedexpansion

echo ========================================
echo Cultivation World Simulation - Build Script
echo ========================================
echo.

REM 设置编译器路径
set GPP="D:\Program Files (x86)\Dev-Cpp\MinGW64\bin\g++.exe"

REM 设置编译选项
set CXXFLAGS=-std=c++11 -O2
set OUTPUT=cultivation_world.exe

REM 源文件列表
set SOURCES=^
    "Cultivation_World_Simulation(Multi-Process)_V2.cpp" ^
    SimulationWorld_V2.cpp ^
    population_management_V2.cpp ^
    combat_system_V2.cpp ^
    alliance_system_V2.cpp ^
    Generator_Random_V2.cpp ^
    statistics_V2.cpp

REM 显示配置
echo Configuration:
echo   Compiler: %GPP%
echo   Flags: %CXXFLAGS%
echo   Output: %OUTPUT%
echo.
echo Source files:
for %%f in (%SOURCES%) do echo   - %%f
echo.

REM 清理旧文件
echo Cleaning old files...
if exist %OUTPUT% (
    del %OUTPUT%
    echo   Deleted: %OUTPUT%
)
if exist *.o (
    del *.o
    echo   Deleted: *.o
)
echo Done.
echo.

REM 编译
echo Compiling...
%GPP% %CXXFLAGS% %SOURCES% -o %OUTPUT%

if %errorlevel% equ 0 (
    echo.
    echo [SUCCESS] Compilation successful!
    echo.
    echo Output information:
    echo   File: %OUTPUT%
    echo   Size: 
    dir %OUTPUT% | find "cultivation_world.exe"
) else (
    echo.
    echo [ERROR] Compilation failed!
    echo Error code: %errorlevel%
)

echo.
pause
