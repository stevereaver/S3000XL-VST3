@echo off
setlocal enabledelayedexpansion

REM ------------------------------------------------------------------------------
REM Build the MAME static libraries for the s3000xl subtarget.
REM
REM This mirrors the old run_genie_s3000xl.bat + run_msbuild_mame_s3000xl.bat
REM but uses repo-relative paths.
REM
REM Override locations via environment variables:
REM   MSYS2_ROOT    default: C:\msys64
REM   VS_VCVARSALL  default: C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat
REM ------------------------------------------------------------------------------

set "REPO_ROOT=%~dp0.."
set "MAME_ROOT=%REPO_ROOT%\mame"

if not defined MSYS2_ROOT set "MSYS2_ROOT=C:\msys64"
if not defined VS_VCVARSALL set "VS_VCVARSALL=C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat"

set "MSYSTEM=MINGW64"
set "OS=Windows_NT"
set "MINGW_PREFIX=/mingw64"

REM Generate VS2022 solution with GENie under MSYS2/MinGW64.
"%MSYS2_ROOT%\usr\bin\bash.exe" -lc "export PATH=/mingw64/bin:$PATH; export OS=Windows_NT; export MINGW_PREFIX=/mingw64; cd '$(cygpath -u '%MAME_ROOT%')' && make vs2022 SUBTARGET=s3000xl SOURCES=src/mame/akai/s3000.cpp NOWERROR=1" > "%MAME_ROOT%\genie_s3000xl.log" 2>&1
if %errorlevel% neq 0 (
    echo GENie step failed. See %MAME_ROOT%\genie_s3000xl.log
    exit /b %errorlevel%
)
echo GENie OK. Log: %MAME_ROOT%\genie_s3000xl.log

REM Build the static libraries with MSVC.
call "%VS_VCVARSALL%" x64
set "_CL_=/U_WIN32_WINNT /UNTDDI_VERSION /D_WIN32_WINNT=0x0A00 /DNTDDI_VERSION=0xA000000"

msbuild "%MAME_ROOT%\build\projects\windows\mames3000xl\vs2022\mames3000xl.sln" ^
    /p:Configuration=Release /p:Platform=x64 /p:PreferredToolArchitecture=x64 ^
    /m /v:minimal /nologo > "%MAME_ROOT%\msbuild_mame_s3000xl.log" 2>&1
if %errorlevel% neq 0 (
    echo MSBuild step failed. See %MAME_ROOT%\msbuild_mame_s3000xl.log
    exit /b %errorlevel%
)
echo MAME libs built. Log: %MAME_ROOT%\msbuild_mame_s3000xl.log
