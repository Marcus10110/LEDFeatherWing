
pushd ..\..\..\temp\emsdk\
::emsdk activate latest
call emsdk_env.bat
popd
set FEATHER_WING_DIR=C:\Users\Mark\Software\LEDFeatherWing\src
call em++ --bind -std=c++17 led.cpp %FEATHER_WING_DIR%\animations.cpp -I%FEATHER_WING_DIR% -o led.js -s EXTRA_EXPORTED_RUNTIME_METHODS="['ccall', 'cwrap', 'addOnPostRun']"
if not exist "..\dist" mkdir "..\dist"
copy /Y led.js ..\dist
copy /Y led.wasm ..\dist
echo "build and deploy complete."