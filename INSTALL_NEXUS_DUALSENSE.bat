@echo off
setlocal EnableExtensions EnableDelayedExpansion
cd /d "%~dp0"

echo ============================================================
echo Nexus DualSense for Lucas Mod Launcher - Test 02
echo Builds current Test 66 x86 runtime and verifies Lucas install
echo ============================================================
echo.

if not exist "Lucas Simpsons Hit & Run Mod Launcher.exe" (
  echo ERROR: This BAT is not in the Lucas Mod Launcher folder.
  echo.
  echo Extract the CONTENTS of this ZIP into the folder containing:
  echo   Lucas Simpsons Hit ^& Run Mod Launcher.exe
  echo Then run this BAT again.
  echo.
  pause
  exit /b 1
)

if not exist "Hacks" mkdir "Hacks"
if not exist "DLLs" mkdir "DLLs"
if not exist "Mods" mkdir "Mods"

if not exist "Hacks\NexusDualSense.lmlh" (
  echo ERROR: Hacks\NexusDualSense.lmlh is missing.
  goto :failed
)
if not exist "DLLs\SDL3.dll" (
  echo ERROR: DLLs\SDL3.dll is missing.
  goto :failed
)
if not exist "Source\NexusDualSenseHook.cpp" (
  echo ERROR: Source\NexusDualSenseHook.cpp is missing.
  goto :failed
)
if not exist "Mods\SpringfieldNexusDualSensePrompts\Meta.ini" (
  echo ERROR: Nexus DualSense prompt mod is missing.
  goto :failed
)

echo [1/3] Locating Visual Studio C++ x86 build tools...
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
  echo ERROR: Visual Studio C++ build tools were not found.
  echo Open Visual Studio Installer and add "Desktop development with C++".
  goto :failed
)
set "VSROOT="
for /f "usebackq tokens=*" %%I in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSROOT=%%I"
if not defined VSROOT (
  echo ERROR: Visual C++ x86/x64 tools are not installed.
  echo Open Visual Studio Installer and add "Desktop development with C++".
  goto :failed
)
call "%VSROOT%\VC\Auxiliary\Build\vcvars32.bat" >nul
if errorlevel 1 goto :failed

echo [2/3] Building current Test 66 NexusDualSenseHook.dll for Win32/x86...
cl /nologo /std:c++17 /O2 /EHsc /LD /DWIN32 /D_WINDOWS ^
  "%~dp0Source\NexusDualSenseHook.cpp" ^
  /link /OUT:"%~dp0DLLs\NexusDualSenseHook.dll" user32.lib
if errorlevel 1 goto :failed

if exist "%~dp0NexusDualSenseHook.obj" del /q "%~dp0NexusDualSenseHook.obj" >nul 2>nul
if exist "%~dp0Source\NexusDualSenseHook.obj" del /q "%~dp0Source\NexusDualSenseHook.obj" >nul 2>nul
if exist "%~dp0DLLs\NexusDualSenseHook.exp" del /q "%~dp0DLLs\NexusDualSenseHook.exp" >nul 2>nul
if exist "%~dp0DLLs\NexusDualSenseHook.lib" del /q "%~dp0DLLs\NexusDualSenseHook.lib" >nul 2>nul

echo [3/3] Verifying Lucas files...
if not exist "DLLs\NexusDualSenseHook.dll" (
  echo ERROR: Native runtime did not build.
  goto :failed
)
if not exist "Hacks\NexusDualSense.lmlh" goto :failed
if not exist "DLLs\SDL3.dll" goto :failed
if not exist "Mods\SpringfieldNexusDualSensePrompts\CustomFiles.ini" goto :failed
if not exist "Mods\SpringfieldNexusDualSensePrompts\Resources\scripts\handlers\ingame_dualsense.lua" goto :failed

powershell -NoProfile -ExecutionPolicy Bypass -Command ^
 "$p='DLLs\NexusDualSenseHook.dll'; $b=[IO.File]::ReadAllBytes($p); if($b.Length -lt 512 -or $b[0]-ne 0x4D -or $b[1]-ne 0x5A){exit 2}; $e=[BitConverter]::ToInt32($b,0x3C); $m=[BitConverter]::ToUInt16($b,$e+4); if($m-ne 0x14C){Write-Host 'ERROR: Runtime is not Win32/x86.'; exit 3}; Write-Host '  NexusDualSenseHook.dll: Win32/x86 OK'"
if errorlevel 1 goto :failed

echo.
echo ============================================================
echo NEXUS DUALSENSE LUCAS INSTALL READY
echo ============================================================
echo.
echo Files are now in the correct Lucas folders:
echo   Hacks\NexusDualSense.lmlh
echo   DLLs\NexusDualSenseHook.dll
echo   DLLs\SDL3.dll
echo   Mods\SpringfieldNexusDualSensePrompts\
echo.
echo NEXT:
echo   1. Close and reopen Lucas Mod Launcher, or click Reload.
echo   2. Enable BOTH "Nexus DualSense" and "Nexus DualSense Prompts".
echo      Test 02 intentionally keeps these as two separate checkboxes.
echo   3. If Lucas shows an unsigned/untrusted hack warning, allow this hack.
echo   4. Launch the game with your DualSense connected.
echo.
echo Runtime log after launching:
echo   NexusDualSense_Hook.log
 echo.
pause
exit /b 0

:failed
echo.
echo NEXUS DUALSENSE LUCAS INSTALL FAILED.
echo Copy the full window output back to ChatGPT.
echo.
pause
exit /b 1
