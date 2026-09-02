NEXUS DUALSENSE FOR LUCAS MOD LAUNCHER - TEST 02
================================================

THIS IS THE LUCAS-ONLY VERSION.
Springfield Nexus is NOT required to launch the game.

WHAT TEST 02 FIXES
------------------
Lucas 1.27.1 correctly loaded our custom "Nexus DualSense" hack, but Test 01
made the prompt mod declare it as RequiredHack=NexusDualSense. Lucas rejected
that because this third-party .lmlh is not marked as a Mod-Requirable hack.

Test 02 removes ONLY that dependency. The two pieces are now enabled normally:

    [x] Nexus DualSense
    [x] Nexus DualSense Prompts

The prompt mod still legitimately requires Lucas' built-in CustomFiles hack.

INSTALL / UPDATE
----------------
1. Close Lucas Mod Launcher.

2. Extract the CONTENTS of this ZIP directly into the folder containing:
      Lucas Simpsons Hit & Run Mod Launcher.exe

   Allow Windows to merge/replace the Test 01 files.
   This package does NOT replace DLLs\Hacks.dll.

3. Run:
      INSTALL_NEXUS_DUALSENSE.bat

4. Open Lucas Mod Launcher or click Reload.

5. In the Mods List, tick BOTH:
      Nexus DualSense
      Nexus DualSense Prompts

6. If Lucas warns that Nexus DualSense is an unsigned/untrusted third-party
   hack, allow it if you want to run this test build.

7. Connect the DualSense by Bluetooth or USB and click Launch.

EXPECTED LUCAS LAYOUT
---------------------
Mod Launcher\
  Hacks\
    NexusDualSense.lmlh
  DLLs\
    Hacks.dll                    <- Lucas original, NEVER replace this
    NexusDualSenseHook.dll       <- our Test 66 native SHAR bridge
    SDL3.dll
    NexusDualSense.ini
  Mods\
    SpringfieldNexusDualSensePrompts\
      Meta.ini
      CustomFiles.ini
      Resources\scripts\handlers\ingame_dualsense.lua

DIAGNOSTICS
-----------
After Simpsons.exe starts, look for:
    NexusDualSense_Hook.log

Expected early log lines include:
    Springfield Nexus DualSense Hack - Prototype 10 / Test 66 SHAR event haptics loaded
    SDL3 initialized inside Simpsons.exe.

IMPORTANT
---------
Do NOT put NexusDualSense.lmlh into DLLs.
Do NOT replace DLLs\Hacks.dll.
Do NOT require NexusDualSense from the prompt mod in Test 02; enable the two
checkboxes together in Lucas.
