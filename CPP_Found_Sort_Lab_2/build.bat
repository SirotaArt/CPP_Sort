@echo off
setlocal
set "VCVARS=C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
call "%VCVARS%" >nul
if not exist build mkdir build
pushd build
cl /nologo /EHsc /std:c++17 /O2 /W3 /Fe:lr2.exe ^
   ..\src\main.cpp ..\src\io.cpp
set RC=%ERRORLEVEL%
popd
if "%RC%"=="0" (echo [build] OK) else (echo [build] FAILED rc=%RC%)
exit /b %RC%
