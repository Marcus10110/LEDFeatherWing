@echo off
setlocal enabledelayedexpansion

:: Note, on windows, espota.exe needs a firewall exclusition.

:: usage:
:: upload.bat 192.168.7.156 192.168.7.161

set REPO_ROOT=%~dp0
set "BIN_PATH=%REPO_ROOT%..\temp\ArduinoBuild\LEDFeatherWing\LEDFeatherWing.ino.bin"
set "TOOLS_DIR=C:\Users\Mark\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.6\tools"

if not exist "%TOOLS_DIR%" ( 
echo ESP Tools directory not found. aborting.
exit /b 1
)

if not exist "%BIN_PATH%" ( 
echo Compiled outout not found. aborting
exit /b 1
)

pushd "%TOOLS_DIR%"

set argCount=0
for %%x in (%*) do (
   set /A argCount+=1
   set "argVec[!argCount!]=%%~x"
)

echo starting upload...

for /L %%i in (1,1,%argCount%) do espota.exe --ip=!argVec[%%i]! --port=3232 --auth=otapass --file="%BIN_PATH%" --progress --debug

popd

echo done
exit /b 0