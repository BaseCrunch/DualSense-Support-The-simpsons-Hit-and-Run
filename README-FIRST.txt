NEXUS DUALSENSE FOR LUCAS MOD LAUNCHER - TEST 01
================================================

THIS IS THE LUCAS-ONLY VERSION.
Springfield Nexus is NOT required to launch the game.

INSTALL
-------
1. Open your Lucas Mod Launcher folder - the folder containing:
      Lucas Simpsons Hit & Run Mod Launcher.exe
      DLLs\
      Mods\

2. Extract the CONTENTS of this ZIP directly into that folder.
   If Windows asks to merge DLLs / Mods folders, choose Yes.
   This package does NOT replace Lucas' DLLs\Hacks.dll.

3. Run:
      INSTALL_NEXUS_DUALSENSE.bat

   The BAT compiles the current Test 66 native runtime with the same
   Win32/x86 Visual C++ build command used by Springfield Nexus.

4. Reopen Lucas Mod Launcher or click Reload.

5. Enable:
      Nexus DualSense Prompts

   That mod requires the custom NexusDualSense hack, so Lucas should
   enable/load the native hack with it. The custom hack itself is:
      Hacks\NexusDualSense.lmlh

6. If Lucas 1.27.1 warns that NexusDualSense is an unsigned/untrusted
   third-party hack, allow it if you want to run this test build.

7. Launch SHAR with your DualSense connected by Bluetooth or USB.

FILES
-----
Hacks\NexusDualSense.lmlh
    Lucas-compatible x86 wrapper hack. On game load it loads the Test 66
    native runtime from DLLs\NexusDualSenseHook.dll.

DLLs\NexusDualSenseHook.dll
    Built locally by INSTALL_NEXUS_DUALSENSE.bat from the current Test 66
    source. This is the real SHAR native input/haptics bridge.

DLLs\SDL3.dll
    SDL 3.4.14 Win32/x86 runtime used for native DualSense HID/gamepad I/O.

Mods\SpringfieldNexusDualSensePrompts\
    PlayStation-style SHAR prompt companion.

DIAGNOSTICS
-----------
After the game starts, the native hook writes:
    NexusDualSense_Hook.log
next to the Lucas launcher folder.

Expected early log lines include:
    Springfield Nexus DualSense Hack - Prototype 10 / Test 66 SHAR event haptics loaded
    SDL3 initialized inside Simpsons.exe.

IMPORTANT
---------
Do NOT put NexusDualSense.lmlh into DLLs.
Do NOT replace DLLs\Hacks.dll.
The .lmlh belongs in the separate Hacks folder. If your Lucas install does
not currently have a Hacks folder, this package creates/adds it.
