@echo off
SETLOCAL

SET VS_VERSION=Visual Studio 18 2026
SET VS_IDE_PATH=C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE
SET SOLUTION_NAME=Spectrumizer

SET BUILD_DIR=build

IF EXIST %BUILD_DIR% rmdir /s /q %BUILD_DIR%

mkdir %BUILD_DIR%
cd %BUILD_DIR%
SET VS_CMAKE="%VS_IDE_PATH%\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"

%VS_CMAKE% .. -G "%VS_VERSION%" -A x64 -DCMAKE_CONFIGURATION_TYPES="Debug;Release"

timeout /t 1 >nul

start "" "%VS_IDE_PATH%\devenv.exe" "%~dp0%BUILD_DIR%\%SOLUTION_NAME%.sln"
