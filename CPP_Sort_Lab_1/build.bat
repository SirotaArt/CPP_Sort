@echo off
rem Build LR1 with MSVC cl.exe. Run from LR1 directory.
setlocal
set "VCVARS=C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
if not exist "%VCVARS%" (
  echo [build] vcvars64.bat not found, edit build.bat
  exit /b 1
)
call "%VCVARS%" >nul

if not exist build mkdir build
pushd build
cl /nologo /EHsc /std:c++17 /O2 /W3 /Fe:lr1.exe ^
   ..\src\main.cpp ..\src\io.cpp
set RC=%ERRORLEVEL%
popd
if "%RC%"=="0" (echo [build] OK) else (echo [build] FAILED rc=%RC%)
exit /b %RC%
