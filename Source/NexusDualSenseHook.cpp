#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <string>
#include <vector>
#include <algorithm>
#include <math.h>

#if defined(_WIN32)
static_assert(sizeof(void*) == 4, "NexusDualSenseHook must be built for Win32/x86.");
#endif

// SDL3 is deliberately loaded dynamically. This keeps the hook build tiny and
// means Prototype 01 only needs Microsoft's C++ compiler plus the official SDL3.dll.
typedef uint32_t SDL_JoystickID;
struct SDL_Gamepad;

enum SDL_SensorType
{
    SDL_SENSOR_INVALID = -1,
    SDL_SENSOR_UNKNOWN = 0,
    SDL_SENSOR_ACCEL = 1,
    SDL_SENSOR_GYRO = 2
};

enum SDL_GamepadAxis
{
    SDL_GAMEPAD_AXIS_INVALID = -1,
    SDL_GAMEPAD_AXIS_LEFTX = 0,
    SDL_GAMEPAD_AXIS_LEFTY,
    SDL_GAMEPAD_AXIS_RIGHTX,
    SDL_GAMEPAD_AXIS_RIGHTY,
    SDL_GAMEPAD_AXIS_LEFT_TRIGGER,
    SDL_GAMEPAD_AXIS_RIGHT_TRIGGER,
    SDL_GAMEPAD_AXIS_COUNT
};

enum SDL_GamepadButton
{
    SDL_GAMEPAD_BUTTON_INVALID = -1,
    SDL_GAMEPAD_BUTTON_SOUTH = 0,
    SDL_GAMEPAD_BUTTON_EAST,
    SDL_GAMEPAD_BUTTON_WEST,
    SDL_GAMEPAD_BUTTON_NORTH,
    SDL_GAMEPAD_BUTTON_BACK,
    SDL_GAMEPAD_BUTTON_GUIDE,
    SDL_GAMEPAD_BUTTON_START,
    SDL_GAMEPAD_BUTTON_LEFT_STICK,
    SDL_GAMEPAD_BUTTON_RIGHT_STICK,
    SDL_GAMEPAD_BUTTON_LEFT_SHOULDER,
    SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER,
    SDL_GAMEPAD_BUTTON_DPAD_UP,
    SDL_GAMEPAD_BUTTON_DPAD_DOWN,
    SDL_GAMEPAD_BUTTON_DPAD_LEFT,
    SDL_GAMEPAD_BUTTON_DPAD_RIGHT,
    SDL_GAMEPAD_BUTTON_MISC1,
    SDL_GAMEPAD_BUTTON_RIGHT_PADDLE1,
    SDL_GAMEPAD_BUTTON_LEFT_PADDLE1,
    SDL_GAMEPAD_BUTTON_RIGHT_PADDLE2,
    SDL_GAMEPAD_BUTTON_LEFT_PADDLE2,
    SDL_GAMEPAD_BUTTON_TOUCHPAD,
    SDL_GAMEPAD_BUTTON_MISC2,
    SDL_GAMEPAD_BUTTON_MISC3,
    SDL_GAMEPAD_BUTTON_MISC4,
    SDL_GAMEPAD_BUTTON_MISC5,
    SDL_GAMEPAD_BUTTON_MISC6,
    SDL_GAMEPAD_BUTTON_COUNT
};

static const uint32_t SDL_INIT_GAMEPAD = 0x00002000u;
static const uint32_t SDL_INIT_EVENTS  = 0x00004000u;
static const uint32_t SDL_INIT_SENSOR  = 0x00008000u;

typedef bool (__cdecl *PFN_SDL_Init)(uint32_t flags);
typedef void (__cdecl *PFN_SDL_QuitSubSystem)(uint32_t flags);
typedef bool (__cdecl *PFN_SDL_SetHint)(const char* name, const char* value);
typedef const char* (__cdecl *PFN_SDL_GetError)(void);
typedef void (__cdecl *PFN_SDL_PumpEvents)(void);
typedef SDL_JoystickID* (__cdecl *PFN_SDL_GetGamepads)(int* count);
typedef SDL_Gamepad* (__cdecl *PFN_SDL_OpenGamepad)(SDL_JoystickID instance_id);
typedef void (__cdecl *PFN_SDL_CloseGamepad)(SDL_Gamepad* gamepad);
typedef const char* (__cdecl *PFN_SDL_GetGamepadName)(SDL_Gamepad* gamepad);
typedef int16_t (__cdecl *PFN_SDL_GetGamepadAxis)(SDL_Gamepad* gamepad, SDL_GamepadAxis axis);
typedef bool (__cdecl *PFN_SDL_GetGamepadButton)(SDL_Gamepad* gamepad, SDL_GamepadButton button);
typedef bool (__cdecl *PFN_SDL_RumbleGamepad)(SDL_Gamepad* gamepad, uint16_t low_frequency_rumble, uint16_t high_frequency_rumble, uint32_t duration_ms);
typedef bool (__cdecl *PFN_SDL_SetGamepadLED)(SDL_Gamepad* gamepad, uint8_t red, uint8_t green, uint8_t blue);
typedef bool (__cdecl *PFN_SDL_GamepadHasSensor)(SDL_Gamepad* gamepad, SDL_SensorType type);
typedef bool (__cdecl *PFN_SDL_SetGamepadSensorEnabled)(SDL_Gamepad* gamepad, SDL_SensorType type, bool enabled);
typedef bool (__cdecl *PFN_SDL_GetGamepadSensorData)(SDL_Gamepad* gamepad, SDL_SensorType type, float* data, int num_values);
typedef int (__cdecl *PFN_SDL_GetNumGamepadTouchpads)(SDL_Gamepad* gamepad);
typedef int (__cdecl *PFN_SDL_GetNumGamepadTouchpadFingers)(SDL_Gamepad* gamepad, int touchpad);
typedef bool (__cdecl *PFN_SDL_GetGamepadTouchpadFinger)(SDL_Gamepad* gamepad, int touchpad, int finger, bool* down, float* x, float* y, float* pressure);
typedef void (__cdecl *PFN_SDL_free)(void* mem);

struct SDLApi
{
    HMODULE module = nullptr;
    PFN_SDL_Init Init = nullptr;
    PFN_SDL_QuitSubSystem QuitSubSystem = nullptr;
    PFN_SDL_SetHint SetHint = nullptr;
    PFN_SDL_GetError GetError = nullptr;
    PFN_SDL_PumpEvents PumpEvents = nullptr;
    PFN_SDL_GetGamepads GetGamepads = nullptr;
    PFN_SDL_OpenGamepad OpenGamepad = nullptr;
    PFN_SDL_CloseGamepad CloseGamepad = nullptr;
    PFN_SDL_GetGamepadName GetGamepadName = nullptr;
    PFN_SDL_GetGamepadAxis GetGamepadAxis = nullptr;
    PFN_SDL_GetGamepadButton GetGamepadButton = nullptr;
    PFN_SDL_RumbleGamepad RumbleGamepad = nullptr;
    PFN_SDL_SetGamepadLED SetGamepadLED = nullptr;
    PFN_SDL_GamepadHasSensor GamepadHasSensor = nullptr;
    PFN_SDL_SetGamepadSensorEnabled SetGamepadSensorEnabled = nullptr;
    PFN_SDL_GetGamepadSensorData GetGamepadSensorData = nullptr;
    PFN_SDL_GetNumGamepadTouchpads GetNumGamepadTouchpads = nullptr;
    PFN_SDL_GetNumGamepadTouchpadFingers GetNumGamepadTouchpadFingers = nullptr;
    PFN_SDL_GetGamepadTouchpadFinger GetGamepadTouchpadFinger = nullptr;
    PFN_SDL_free Free = nullptr;
};

static HMODULE g_hookModule = nullptr;
static volatile LONG g_stop = 0;
static FILE* g_log = nullptr;

static std::wstring ModuleDirectory(HMODULE module)
{
    wchar_t path[MAX_PATH] = {};
    GetModuleFileNameW(module, path, MAX_PATH);
    wchar_t* slash = wcsrchr(path, L'\\');
    if (slash) *slash = L'\0';
    return path;
}

static std::wstring ParentDirectory(const std::wstring& directory)
{
    std::wstring parent = directory;
    size_t slash = parent.find_last_of(L"\\/");
    if (slash != std::wstring::npos) parent.resize(slash);
    return parent;
}

static void Log(const char* format, ...)
{
    if (!g_log) return;
    SYSTEMTIME st{};
    GetLocalTime(&st);
    fprintf(g_log, "%04u-%02u-%02u %02u:%02u:%02u.%03u  ", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    va_list args;
    va_start(args, format);
    vfprintf(g_log, format, args);
    va_end(args);
    fputc('\n', g_log);
    fflush(g_log);
}

template<typename T>
static bool Resolve(HMODULE module, const char* name, T& out)
{
    out = reinterpret_cast<T>(GetProcAddress(module, name));
    if (!out) Log("Missing SDL export: %s", name);
    return out != nullptr;
}

static bool LoadSDL(SDLApi& sdl)
{
    std::wstring sdlPath = ModuleDirectory(g_hookModule) + L"\\SDL3.dll";
    sdl.module = GetModuleHandleW(L"SDL3.dll");
    if (!sdl.module) sdl.module = LoadLibraryW(sdlPath.c_str());
    if (!sdl.module)
    {
        Log("Could not load SDL3.dll (Win32=%lu) path=%ls", GetLastError(), sdlPath.c_str());
        return false;
    }

    bool ok = true;
    ok &= Resolve(sdl.module, "SDL_Init", sdl.Init);
    ok &= Resolve(sdl.module, "SDL_QuitSubSystem", sdl.QuitSubSystem);
    ok &= Resolve(sdl.module, "SDL_SetHint", sdl.SetHint);
    ok &= Resolve(sdl.module, "SDL_GetError", sdl.GetError);
    ok &= Resolve(sdl.module, "SDL_PumpEvents", sdl.PumpEvents);
    ok &= Resolve(sdl.module, "SDL_GetGamepads", sdl.GetGamepads);
    ok &= Resolve(sdl.module, "SDL_OpenGamepad", sdl.OpenGamepad);
    ok &= Resolve(sdl.module, "SDL_CloseGamepad", sdl.CloseGamepad);
    ok &= Resolve(sdl.module, "SDL_GetGamepadName", sdl.GetGamepadName);
    ok &= Resolve(sdl.module, "SDL_GetGamepadAxis", sdl.GetGamepadAxis);
    ok &= Resolve(sdl.module, "SDL_GetGamepadButton", sdl.GetGamepadButton);
    ok &= Resolve(sdl.module, "SDL_RumbleGamepad", sdl.RumbleGamepad);
    ok &= Resolve(sdl.module, "SDL_SetGamepadLED", sdl.SetGamepadLED);
    ok &= Resolve(sdl.module, "SDL_GamepadHasSensor", sdl.GamepadHasSensor);
    ok &= Resolve(sdl.module, "SDL_SetGamepadSensorEnabled", sdl.SetGamepadSensorEnabled);
    ok &= Resolve(sdl.module, "SDL_GetGamepadSensorData", sdl.GetGamepadSensorData);
    ok &= Resolve(sdl.module, "SDL_GetNumGamepadTouchpads", sdl.GetNumGamepadTouchpads);
    ok &= Resolve(sdl.module, "SDL_GetNumGamepadTouchpadFingers", sdl.GetNumGamepadTouchpadFingers);
    ok &= Resolve(sdl.module, "SDL_GetGamepadTouchpadFinger", sdl.GetGamepadTouchpadFinger);
    ok &= Resolve(sdl.module, "SDL_free", sdl.Free);
    return ok;
}

static bool ContainsNoCase(const char* text, const char* needle)
{
    if (!text || !needle) return false;
    std::string a(text), b(needle);
    std::transform(a.begin(), a.end(), a.begin(), [](unsigned char c) { return (char)tolower(c); });
    std::transform(b.begin(), b.end(), b.begin(), [](unsigned char c) { return (char)tolower(c); });
    return a.find(b) != std::string::npos;
}

static SDL_Gamepad* OpenPreferredGamepad(SDLApi& sdl)
{
    int count = 0;
    SDL_JoystickID* ids = sdl.GetGamepads(&count);
    if (!ids || count <= 0)
    {
        if (ids) sdl.Free(ids);
        return nullptr;
    }

    SDL_Gamepad* fallback = nullptr;
    SDL_Gamepad* preferred = nullptr;
    for (int i = 0; i < count; ++i)
    {
        SDL_Gamepad* pad = sdl.OpenGamepad(ids[i]);
        if (!pad) continue;
        const char* name = sdl.GetGamepadName(pad);
        if (!fallback) fallback = pad;
        else if (!preferred && (ContainsNoCase(name, "dualsense") || ContainsNoCase(name, "dual sense") || ContainsNoCase(name, "wireless controller") || ContainsNoCase(name, "ps5")))
        {
            preferred = pad;
            break;
        }
        else
        {
            sdl.CloseGamepad(pad);
        }
    }
    sdl.Free(ids);

    if (preferred)
    {
        if (fallback && fallback != preferred) sdl.CloseGamepad(fallback);
        return preferred;
    }
    return fallback;
}

static const char* ButtonName(int b)
{
    static const char* names[] = {
        "Cross/South", "Circle/East", "Square/West", "Triangle/North", "Create/Back", "PS/Guide", "Options/Start",
        "L3", "R3", "L1", "R1", "DPad Up", "DPad Down", "DPad Left", "DPad Right", "Misc1/Mic",
        "Right Paddle 1", "Left Paddle 1", "Right Paddle 2", "Left Paddle 2", "Touchpad", "Misc2", "Misc3", "Misc4", "Misc5", "Misc6"
    };
    return (b >= 0 && b < (int)(sizeof(names) / sizeof(names[0]))) ? names[b] : "Unknown";
}


// -----------------------------------------------------------------------------
// Prototype 10 / Test 66: locked Test 62/64 controls + Test 65 rumble + real SHAR EventManager haptics.
//
// Test 61 proved SDL3/injection/DualSense input are healthy, but it also proved
// that writing a guessed retail UserController Button[] layout is unsafe: the
// assumed mNumButtons offset did not match the actual object.  Test 62 removes
// every dependency on mNumButtons/mButtonArray offsets.
//
// sharapi independently confirms that UserController::mMappable begins at +0x3C
// and that the retail Mappable::DispatchOnButton function lives at 0x435C30.
// The original Win32 UserController::Update() broadcasts each virtual input code
// to every registered Mappable with a Button object.  We mirror only that public
// dispatch step here using a LOCAL 8-byte Button; no SHAR object fields are
// written and no replacement controller vtable/XInput layer is involved.
// -----------------------------------------------------------------------------

static const uint32_t SHAR_EXPECTED_FILE_SIZE = 2486272u;
static const uintptr_t SHAR_PREFERRED_BASE = 0x00400000u;
static const char* SHAR_EXPECTED_MD5 = "9009afe5ab6c2daf8605d8b613951902";

// This is the only UserController field offset Test 62 consumes.  sharapi's
// UserController.GetMappable() uses exactly ptr + 0x3C + slot*4.
static const size_t UC_MAPPABLE_OFFSET = 0x3Cu;
static const int UC_MAX_MAPPABLES = 16;
static const int UC_EXPECTED_VIRTUAL_BUTTONS = 46;

static uintptr_t RebaseSharVA(uintptr_t moduleBase, uintptr_t preferredVA)
{
    return moduleBase + (preferredVA - SHAR_PREFERRED_BASE);
}

static bool IsReadableMemory(const void* pointer, size_t bytes = sizeof(void*))
{
    if (!pointer || bytes == 0) return false;
    MEMORY_BASIC_INFORMATION mbi{};
    if (!VirtualQuery(pointer, &mbi, sizeof(mbi))) return false;
    if (mbi.State != MEM_COMMIT) return false;
    if ((mbi.Protect & PAGE_GUARD) || (mbi.Protect & PAGE_NOACCESS)) return false;
    DWORD protection = mbi.Protect & 0xFFu;
    bool readable = protection == PAGE_READONLY || protection == PAGE_READWRITE ||
                    protection == PAGE_WRITECOPY || protection == PAGE_EXECUTE_READ ||
                    protection == PAGE_EXECUTE_READWRITE || protection == PAGE_EXECUTE_WRITECOPY;
    if (!readable) return false;
    uintptr_t start = reinterpret_cast<uintptr_t>(pointer);
    uintptr_t regionEnd = reinterpret_cast<uintptr_t>(mbi.BaseAddress) + mbi.RegionSize;
    return start <= regionEnd && bytes <= (regionEnd - start);
}

static bool IsWritableMemory(const void* pointer, size_t bytes = sizeof(void*))
{
    if (!pointer || bytes == 0) return false;
    MEMORY_BASIC_INFORMATION mbi{};
    if (!VirtualQuery(pointer, &mbi, sizeof(mbi))) return false;
    if (mbi.State != MEM_COMMIT) return false;
    if ((mbi.Protect & PAGE_GUARD) || (mbi.Protect & PAGE_NOACCESS)) return false;
    DWORD protection = mbi.Protect & 0xFFu;
    bool writable = protection == PAGE_READWRITE || protection == PAGE_WRITECOPY ||
                    protection == PAGE_EXECUTE_READWRITE || protection == PAGE_EXECUTE_WRITECOPY;
    if (!writable) return false;
    uintptr_t start = reinterpret_cast<uintptr_t>(pointer);
    uintptr_t regionEnd = reinterpret_cast<uintptr_t>(mbi.BaseAddress) + mbi.RegionSize;
    return start <= regionEnd && bytes <= (regionEnd - start);
}

static bool IsExecutableMemory(const void* pointer)
{
    if (!pointer) return false;
    MEMORY_BASIC_INFORMATION mbi{};
    if (!VirtualQuery(pointer, &mbi, sizeof(mbi))) return false;
    if (mbi.State != MEM_COMMIT) return false;
    if ((mbi.Protect & PAGE_GUARD) || (mbi.Protect & PAGE_NOACCESS)) return false;
    DWORD protection = mbi.Protect & 0xFFu;
    return protection == PAGE_EXECUTE || protection == PAGE_EXECUTE_READ ||
           protection == PAGE_EXECUTE_READWRITE || protection == PAGE_EXECUTE_WRITECOPY;
}

enum class X86Reg : uint8_t
{
    EAX = 0,
    ECX = 1,
    EDX = 2,
    EBX = 3,
    ESP = 4,
    EBP = 5,
    ESI = 6,
    EDI = 7
};

static uint8_t PopOpcode(X86Reg reg)
{
    return static_cast<uint8_t>(0x58u + static_cast<uint8_t>(reg));
}

static void* BuildUserPurgeStub(const X86Reg* convention, int conventionCount, int totalParams, uintptr_t target)
{
    if (!convention || conventionCount < 0 || totalParams < conventionCount || totalParams <= 0)
        return nullptr;
    if (!IsExecutableMemory(reinterpret_cast<const void*>(target)))
        return nullptr;

    uint8_t* code = static_cast<uint8_t*>(VirtualAlloc(nullptr, 96, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
    if (!code) return nullptr;

    uint8_t* out = code;
    *out++ = 0x53; // push ebx
    *out++ = 0x57; // push edi
    *out++ = 0x56; // push esi
    *out++ = 0x55; // push ebp

    const int copyOffset = 4 * 4 + 4 * totalParams;
    if (copyOffset > 0x7F)
    {
        VirtualFree(code, 0, MEM_RELEASE);
        return nullptr;
    }
    for (int i = 0; i < totalParams; ++i)
    {
        *out++ = 0xFF;
        *out++ = 0x74;
        *out++ = 0x24;
        *out++ = static_cast<uint8_t>(copyOffset);
    }

    for (int i = 0; i < conventionCount; ++i)
        *out++ = PopOpcode(convention[i]);

    *out++ = 0xE8;
    uint32_t callFrom = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(out + 4));
    uint32_t callTo = static_cast<uint32_t>(target);
    int32_t rel32 = static_cast<int32_t>(callTo - callFrom);
    memcpy(out, &rel32, sizeof(rel32));
    out += sizeof(rel32);

    *out++ = 0x5D;
    *out++ = 0x5E;
    *out++ = 0x5F;
    *out++ = 0x5B;
    *out++ = 0xC3;

    FlushInstructionCache(GetCurrentProcess(), code, static_cast<SIZE_T>(out - code));
    DWORD oldProtect = 0;
    VirtualProtect(code, 96, PAGE_EXECUTE_READ, &oldProtect);
    return code;
}

// Build a tiny x86 adapter for SHAR EventListener::HandleEvent.  The game calls
// the virtual method as thiscall: ECX=this, [ESP+4]=eventNum, [ESP+8]=param.
// Our normal C callback receives (listener,eventNum,param).  This mirrors the
// independently documented sharapi EventListener wrapper without replacing any
// game vtables.
static void* BuildEventThiscallCallbackStub(void* callback)
{
    if (!callback || !IsExecutableMemory(callback)) return nullptr;

    uint8_t* code = static_cast<uint8_t*>(VirtualAlloc(nullptr, 64, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
    if (!code) return nullptr;

    uint8_t* out = code;
    // push param, then eventNum.  The same ESP+8 displacement works twice
    // because ESP moves by four bytes after the first push.
    *out++ = 0xFF; *out++ = 0x74; *out++ = 0x24; *out++ = 0x08;
    *out++ = 0xFF; *out++ = 0x74; *out++ = 0x24; *out++ = 0x08;
    *out++ = 0x51; // push ecx (listener/this)

    *out++ = 0xE8;
    uint32_t callFrom = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(out + 4));
    uint32_t callTo = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(callback));
    int32_t rel32 = static_cast<int32_t>(callTo - callFrom);
    memcpy(out, &rel32, sizeof(rel32));
    out += sizeof(rel32);

    *out++ = 0x83; *out++ = 0xC4; *out++ = 0x0C; // add esp, 12
    *out++ = 0xC2; *out++ = 0x08; *out++ = 0x00; // ret 8

    FlushInstructionCache(GetCurrentProcess(), code, static_cast<SIZE_T>(out - code));
    DWORD oldProtect = 0;
    VirtualProtect(code, 64, PAGE_EXECUTE_READ, &oldProtect);
    return code;
}

typedef void* (__cdecl *PFN_SHAR_GetInstance)(void);
typedef void* (__cdecl *PFN_SHAR_GetController)(void* inputManager, int controllerIndex);
typedef void  (__cdecl *PFN_SHAR_DispatchOnButton)(void* mappable, int controllerId, int physicalId, const void* button);

struct SharButton
{
    float state;
    uint32_t tickCount;
};
static_assert(sizeof(SharButton) == 8, "SHAR Button layout must remain 8 bytes on Win32.");

enum Win32VirtualInput
{
    VI_MOVE_UP = 0,
    VI_MOVE_DOWN = 1,
    VI_MOVE_LEFT = 2,
    VI_MOVE_RIGHT = 3,
    VI_ATTACK = 4,
    VI_JUMP = 5,
    VI_SPRINT = 6,
    VI_DO_ACTION = 7,
    VI_ACCELERATE = 8,
    VI_REVERSE = 9,
    VI_STEER_LEFT = 10,
    VI_STEER_RIGHT = 11,
    VI_GET_OUT_CAR = 12,
    VI_HANDBRAKE = 13,
    VI_HORN = 14,
    VI_RESET_CAR = 15,
    VI_CAMERA_LEFT = 16,
    VI_CAMERA_RIGHT = 17,
    VI_CAMERA_MOVE_IN = 18,
    VI_CAMERA_MOVE_OUT = 19,
    VI_CAMERA_ZOOM = 20,
    VI_CAMERA_LOOK_UP = 21,
    VI_CAMERA_CAR_LEFT = 22,
    VI_CAMERA_CAR_RIGHT = 23,
    VI_CAMERA_CAR_LOOK_UP = 24,
    VI_CAMERA_CAR_LOOK_BACK = 25,
    VI_CAMERA_TOGGLE = 26,
    VI_FE_BACK = 27,
    VI_FE_MOVE_UP = 28,
    VI_FE_MOVE_DOWN = 29,
    VI_FE_MOVE_LEFT = 30,
    VI_FE_MOVE_RIGHT = 31,
    VI_FE_SELECT = 32,
    VI_FE_FUNCTION1 = 33,
    VI_FE_FUNCTION2 = 34,
    VI_FE_MOUSE_LEFT = 35,
    VI_FE_MOUSE_RIGHT = 36,
    VI_FE_MOUSE_UP = 37,
    VI_FE_MOUSE_DOWN = 38,
    VI_P1_KBD_START = 39,
    VI_P1_KBD_GAS = 40,
    VI_P1_KBD_BRAKE = 41,
    VI_P1_KBD_EBRAKE = 42,
    VI_P1_KBD_NITRO = 43,
    VI_P1_KBD_LEFT = 44,
    VI_P1_KBD_RIGHT = 45
};

struct SharVirtualInputBridge
{
    bool compatible = false;
    bool stubsReady = false;
    bool ready = false;
    bool permanentlyDisabled = false;
    bool loggedReady = false;
    uintptr_t moduleBase = 0;
    uint32_t fileSize = 0;
    uint32_t* tickCount = nullptr;
    void* lastController = nullptr;
    int lastMappableCount = -1;
    ULONGLONG nextProbe = 0;
    ULONGLONG nextStatusLog = 0;
    ULONGLONG nextDispatchLog = 0;
    uint64_t dispatchCallCount = 0;
    float lastVirtual[UC_EXPECTED_VIRTUAL_BUTTONS] = {};

    PFN_SHAR_GetInstance GetInstance = nullptr;
    PFN_SHAR_GetController GetController = nullptr;
    PFN_SHAR_DispatchOnButton DispatchOnButton = nullptr;
};

static SharVirtualInputBridge g_sharBridge;
static ULONGLONG g_forceJumpUntil = 0;

// Test 66 keeps the approved Test 65 haptic foundation. This deliberately does not alter the proven input
// bridge. It mixes short action pulses with a low continuous vehicle-throttle
// texture, using the live SHAR mappable count only as a conservative gameplay
// state hint (observed Test 62: 4=on-foot, 5=vehicle, 3=transition).
struct DualSenseHapticState
{
    bool previousCross = false;
    bool previousCircle = false;
    bool previousSquare = false;
    bool previousTriangle = false;
    ULONGLONG pulseUntil = 0;
    uint16_t pulseLow = 0;
    uint16_t pulseHigh = 0;
    uint16_t lastLow = 0;
    uint16_t lastHigh = 0;
    ULONGLONG nextRefresh = 0;
    int lastGameplayMode = -1; // 0=unknown/menu, 1=on-foot, 2=vehicle
};

static DualSenseHapticState g_haptics;

// Test 66 listens to real SHAR EventManager events.  The numeric IDs below are
// documented SHAR events; no guessed event numbers are used.  The callback only
// updates atomic counters on the game thread.  SDL rumble is still emitted from
// the existing Nexus worker thread, keeping the game callback lightweight.
enum SharHapticEventIndex
{
    HE_VEHICLE_VEHICLE_COLLISION = 0,
    HE_VEHICLE_DESTROYED,
    HE_VEHICLE_DAMAGED,
    HE_SUSPENSION_BOTTOMED_OUT,
    HE_MINOR_CRASH,
    HE_MINOR_VEHICLE_CRASH,
    HE_BIG_CRASH,
    HE_BIG_VEHICLE_CRASH,
    HE_HIT_BREAKABLE,
    HE_HIT_MOVEABLE,
    HE_JUMP_LANDING,
    HE_PEDESTRIAN_SMACKDOWN,
    HE_PLAYER_CAR_HIT_NPC,
    HE_KICK_NPC_SOUND,
    HE_OBJECT_KICKED,
    HE_RUMBLE_COLLISION,
    HE_COUNT
};

struct SharHapticEventDefinition
{
    int eventId;
    const char* name;
    uint16_t low;
    uint16_t high;
    uint16_t durationMs;
    uint16_t cooldownMs;
    int requiredMode; // 0=any, 1=on-foot, 2=vehicle
};

static const SharHapticEventDefinition g_sharHapticEvents[HE_COUNT] =
{
    {  96, "VEHICLE_VEHICLE_COLLISION", 0x5000, 0x7800,  85,  90, 2 },
    {  97, "VEHICLE_DESTROYED",         0xB000, 0xFFFF, 220, 500, 2 },
    { 100, "VEHICLE_DAMAGED",           0x3800, 0x5800,  65, 100, 2 },
    { 101, "SUSPENSION_BOTTOMED_OUT",   0x3000, 0x5000,  55, 100, 2 },
    { 191, "MINOR_CRASH",               0x4800, 0x7000,  75,  90, 2 },
    { 192, "MINOR_VEHICLE_CRASH",       0x5800, 0x8500,  90,  90, 2 },
    { 193, "BIG_CRASH",                 0x9000, 0xD000, 150, 180, 2 },
    { 194, "BIG_VEHICLE_CRASH",         0xA000, 0xF000, 180, 180, 2 },
    { 199, "HIT_BREAKABLE",             0x4A00, 0x7800,  65,  80, 0 },
    { 200, "HIT_MOVEABLE",              0x4400, 0x7000,  60,  80, 0 },
    { 204, "JUMP_LANDING",              0x3A00, 0x6800,  70,  80, 1 },
    { 205, "PEDESTRIAN_SMACKDOWN",      0x6000, 0x9000, 100, 120, 2 },
    { 223, "PLAYER_CAR_HIT_NPC",        0x6200, 0x9400, 105, 120, 2 },
    { 225, "KICK_NPC_SOUND",            0x5800, 0x9000,  80,  80, 1 },
    { 298, "OBJECT_KICKED",             0x5000, 0x8200,  75,  80, 1 },
    { 301, "RUMBLE_COLLISION",          0x6800, 0xA000, 100,  90, 0 }
};

static volatile LONG g_sharEventCounters[HE_COUNT] = {};
static volatile LONG g_sharEventParams[HE_COUNT] = {};
static LONG g_sharEventConsumed[HE_COUNT] = {};
static ULONGLONG g_sharEventLastPulse[HE_COUNT] = {};

typedef void* (__cdecl *PFN_SHAR_EventGetInstance)(void);
typedef void  (__cdecl *PFN_SHAR_EventAddListener)(void* eventManager, int eventNum, void* listener);

struct NexusSharEventListener
{
    void** vtable;
};

struct SharEventBridge
{
    bool symbolsReady = false;
    bool ready = false;
    bool permanentlyDisabled = false;
    bool loggedWaiting = false;
    ULONGLONG nextProbe = 0;
    void* manager = nullptr;
    PFN_SHAR_EventGetInstance GetInstance = nullptr;
    PFN_SHAR_EventAddListener AddListener = nullptr;
    void* callbackStub = nullptr;
    void* vtable[2] = {};
    NexusSharEventListener listener = {};
};

static SharEventBridge g_eventBridge;

static void __cdecl NexusSharEventCallback(void* listener, int eventNum, void* param)
{
    (void)listener;
    for (int i = 0; i < HE_COUNT; ++i)
    {
        if (g_sharHapticEvents[i].eventId != eventNum) continue;
        InterlockedExchange(&g_sharEventParams[i], static_cast<LONG>(reinterpret_cast<uintptr_t>(param)));
        InterlockedIncrement(&g_sharEventCounters[i]);
        break;
    }
}

static void DisableSharEventBridge(const char* stage, DWORD exceptionCode = 0)
{
    if (g_eventBridge.permanentlyDisabled) return;
    g_eventBridge.permanentlyDisabled = true;
    g_eventBridge.ready = false;
    if (exceptionCode)
        Log("SHAR EventManager haptic bridge DISABLED at %s (SEH=0x%08lX). Test 65 input/rumble remain active.", stage, exceptionCode);
    else
        Log("SHAR EventManager haptic bridge DISABLED at %s. Test 65 input/rumble remain active.", stage);
}


static void DisableSharBridge(const char* stage, DWORD exceptionCode = 0)
{
    if (g_sharBridge.permanentlyDisabled) return;
    g_sharBridge.permanentlyDisabled = true;
    g_sharBridge.ready = false;
    if (exceptionCode)
        Log("SDL -> SHAR VIRTUAL INPUT bridge DISABLED at %s (SEH=0x%08lX). SDL/F7 remain active.", stage, exceptionCode);
    else
        Log("SDL -> SHAR VIRTUAL INPUT bridge DISABLED at %s. SDL/F7 remain active.", stage);
}

static bool GetMainExecutableFileSize(uint32_t& sizeOut)
{
    wchar_t exePath[MAX_PATH] = {};
    DWORD chars = GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    if (!chars || chars >= MAX_PATH) return false;
    WIN32_FILE_ATTRIBUTE_DATA fad{};
    if (!GetFileAttributesExW(exePath, GetFileExInfoStandard, &fad)) return false;
    if (fad.nFileSizeHigh != 0) return false;
    sizeOut = fad.nFileSizeLow;
    return true;
}

static bool InitializeSharBridge()
{
    SharVirtualInputBridge& b = g_sharBridge;
    b.moduleBase = reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr));
    if (!b.moduleBase)
    {
        DisableSharBridge("GetModuleHandleW(NULL)");
        return false;
    }

    if (!GetMainExecutableFileSize(b.fileSize))
    {
        DisableSharBridge("read Simpsons.exe file size");
        return false;
    }

    b.compatible = (b.fileSize == SHAR_EXPECTED_FILE_SIZE);
    Log("SHAR retail compatibility: base=0x%08lX fileSize=%lu expectedSize=%lu %s",
        (unsigned long)b.moduleBase, (unsigned long)b.fileSize,
        (unsigned long)SHAR_EXPECTED_FILE_SIZE, b.compatible ? "MATCH" : "MISMATCH");
    Log("sharapi symbol set target MD5: %s", SHAR_EXPECTED_MD5);

    if (!b.compatible)
    {
        DisableSharBridge("retail compatibility guard");
        return false;
    }

    uintptr_t getInstanceVA = RebaseSharVA(b.moduleBase, 0x435210u);
    uintptr_t getControllerVA = RebaseSharVA(b.moduleBase, 0x435450u);
    uintptr_t dispatchVA = RebaseSharVA(b.moduleBase, 0x435C30u);
    uintptr_t tickVA = RebaseSharVA(b.moduleBase, 0x6C900Cu);

    if (!IsExecutableMemory(reinterpret_cast<void*>(getInstanceVA)) ||
        !IsExecutableMemory(reinterpret_cast<void*>(getControllerVA)) ||
        !IsExecutableMemory(reinterpret_cast<void*>(dispatchVA)) ||
        !IsReadableMemory(reinterpret_cast<void*>(tickVA), sizeof(uint32_t)))
    {
        DisableSharBridge("symbol memory validation");
        return false;
    }

    b.GetInstance = reinterpret_cast<PFN_SHAR_GetInstance>(getInstanceVA);
    b.tickCount = reinterpret_cast<uint32_t*>(tickVA);

    const X86Reg getControllerRegs[] = { X86Reg::ECX, X86Reg::EAX };
    const X86Reg dispatchRegs[] = { X86Reg::EAX, X86Reg::EBX };

    b.GetController = reinterpret_cast<PFN_SHAR_GetController>(
        BuildUserPurgeStub(getControllerRegs, 2, 2, getControllerVA));
    b.DispatchOnButton = reinterpret_cast<PFN_SHAR_DispatchOnButton>(
        BuildUserPurgeStub(dispatchRegs, 2, 4, dispatchVA));

    b.stubsReady = b.GetController && b.DispatchOnButton;
    if (!b.stubsReady)
    {
        DisableSharBridge("build __userpurge wrappers");
        return false;
    }

    Log("Test 66 controller wrappers ready. Test 62 Mappable dispatch architecture is preserved.");
    return true;
}

static bool InitializeSharEventBridgeSymbols()
{
    SharEventBridge& e = g_eventBridge;
    if (e.symbolsReady) return true;
    if (e.permanentlyDisabled) return false;
    if (!g_sharBridge.compatible || !g_sharBridge.moduleBase) return false;

    uintptr_t getInstanceVA = RebaseSharVA(g_sharBridge.moduleBase, 0x4329A0u);
    uintptr_t addListenerVA = RebaseSharVA(g_sharBridge.moduleBase, 0x4329E0u);
    if (!IsExecutableMemory(reinterpret_cast<void*>(getInstanceVA)) ||
        !IsExecutableMemory(reinterpret_cast<void*>(addListenerVA)))
    {
        DisableSharEventBridge("EventManager symbol memory validation");
        return false;
    }

    e.GetInstance = reinterpret_cast<PFN_SHAR_EventGetInstance>(getInstanceVA);
    const X86Reg addListenerRegs[] = { X86Reg::EBX, X86Reg::EDI };
    e.AddListener = reinterpret_cast<PFN_SHAR_EventAddListener>(
        BuildUserPurgeStub(addListenerRegs, 2, 3, addListenerVA));
    e.callbackStub = BuildEventThiscallCallbackStub(reinterpret_cast<void*>(&NexusSharEventCallback));
    if (!e.AddListener || !e.callbackStub)
    {
        DisableSharEventBridge("build EventManager wrappers");
        return false;
    }

    e.vtable[0] = nullptr;
    e.vtable[1] = e.callbackStub; // EventListener::HandleEvent
    e.listener.vtable = e.vtable;
    e.symbolsReady = true;
    Log("Test 66 EventManager wrappers ready: GetInstance=0x%08lX AddListener=0x%08lX listener=%p HandleEvent=%p",
        (unsigned long)getInstanceVA, (unsigned long)addListenerVA, &e.listener, e.callbackStub);
    return true;
}

static bool SafeGetEventManager(void*& out)
{
    out = nullptr;
    __try
    {
        out = g_eventBridge.GetInstance();
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        DisableSharEventBridge("EventManager::GetInstance", GetExceptionCode());
        return false;
    }
}

static bool SafeAddSharEventListener(void* manager, int eventNum)
{
    __try
    {
        g_eventBridge.AddListener(manager, eventNum, &g_eventBridge.listener);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        DisableSharEventBridge("EventManager::AddListener", GetExceptionCode());
        return false;
    }
}

static bool ProbeSharEventBridge(ULONGLONG now)
{
    SharEventBridge& e = g_eventBridge;
    if (e.ready) return true;
    if (e.permanentlyDisabled || !g_sharBridge.ready) return false;
    if (now < e.nextProbe) return false;
    e.nextProbe = now + 500;

    if (!InitializeSharEventBridgeSymbols()) return false;

    void* manager = nullptr;
    if (!SafeGetEventManager(manager) || e.permanentlyDisabled) return false;
    if (!manager || !IsReadableMemory(manager, sizeof(void*)))
    {
        if (!e.loggedWaiting)
        {
            e.loggedWaiting = true;
            Log("Test 66 EventManager waiting: singleton not ready yet.");
        }
        return false;
    }

    e.loggedWaiting = false;
    for (int i = 0; i < HE_COUNT; ++i)
    {
        if (!SafeAddSharEventListener(manager, g_sharHapticEvents[i].eventId)) return false;
    }

    e.manager = manager;
    e.ready = true;
    Log("SHAR EVENT HAPTIC BRIDGE READY: manager=%p listener=%p registeredEvents=%d", manager, &e.listener, HE_COUNT);
    Log("Real SHAR events now drive additional haptics: collisions, vehicle damage/destruction, suspension impacts, landing, object/NPC kick impacts.");
    return true;
}

static bool SafeGetInputManager(void*& out)
{
    out = nullptr;
    __try
    {
        out = g_sharBridge.GetInstance();
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        DisableSharBridge("InputManager::GetInstance", GetExceptionCode());
        return false;
    }
}

static bool SafeGetController(void* manager, void*& out)
{
    out = nullptr;
    __try
    {
        out = g_sharBridge.GetController(manager, 0);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        DisableSharBridge("InputManager::GetController", GetExceptionCode());
        return false;
    }
}

static int CountRegisteredMappables(void* controller)
{
    if (!controller) return 0;
    uint8_t* base = reinterpret_cast<uint8_t*>(controller);
    void** slots = reinterpret_cast<void**>(base + UC_MAPPABLE_OFFSET);
    if (!IsReadableMemory(slots, UC_MAX_MAPPABLES * sizeof(void*))) return -1;
    int count = 0;
    for (int i = 0; i < UC_MAX_MAPPABLES; ++i)
        if (slots[i]) ++count;
    return count;
}

static bool SafeSnapshotMappableSlots(void* controller, void** snapshot)
{
    if (!controller || !snapshot) return false;
    __try
    {
        uint8_t* base = reinterpret_cast<uint8_t*>(controller);
        void** slots = reinterpret_cast<void**>(base + UC_MAPPABLE_OFFSET);
        if (!IsReadableMemory(slots, UC_MAX_MAPPABLES * sizeof(void*))) return false;
        for (int i = 0; i < UC_MAX_MAPPABLES; ++i)
            snapshot[i] = slots[i];
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        DisableSharBridge("enumerate UserController mMappable[]", GetExceptionCode());
        return false;
    }
}

static void LogMappableSlots(void* controller)
{
    void* snapshot[UC_MAX_MAPPABLES] = {};
    if (!SafeSnapshotMappableSlots(controller, snapshot)) return;

    // Keep C++ objects outside the SEH function. MSVC rejects __try in a
    // function that requires C++ object unwinding (C2712).
    std::string line = "MAPPABLE SLOTS:";
    char temp[64];
    for (int i = 0; i < UC_MAX_MAPPABLES; ++i)
    {
        if (!snapshot[i]) continue;
        sprintf_s(temp, " [%d]=%p", i, snapshot[i]);
        line += temp;
    }
    if (line == "MAPPABLE SLOTS:") line += " (none)";
    Log("%s", line.c_str());
}

static bool ProbeSharVirtualBridge(ULONGLONG now)
{
    SharVirtualInputBridge& b = g_sharBridge;
    if (!b.compatible || !b.stubsReady || b.permanentlyDisabled) return false;
    if (now < b.nextProbe) return b.ready;
    b.nextProbe = now + 250;

    void* manager = nullptr;
    void* controller = nullptr;
    if (!SafeGetInputManager(manager) || b.permanentlyDisabled) return false;
    if (!manager || !IsReadableMemory(manager))
    {
        if (now >= b.nextStatusLog)
        {
            b.nextStatusLog = now + 2000;
            Log("SHAR dispatch bridge waiting: InputManager not ready.");
        }
        b.ready = false;
        return false;
    }

    if (!SafeGetController(manager, controller) || b.permanentlyDisabled) return false;
    if (!controller || !IsReadableMemory(controller, UC_MAPPABLE_OFFSET + UC_MAX_MAPPABLES * sizeof(void*)))
    {
        if (now >= b.nextStatusLog)
        {
            b.nextStatusLog = now + 2000;
            Log("SHAR dispatch bridge waiting: UserController 0/mMappable[] not ready. manager=%p controller=%p", manager, controller);
        }
        b.ready = false;
        return false;
    }

    void* slots[UC_MAX_MAPPABLES] = {};
    if (!SafeSnapshotMappableSlots(controller, slots) || b.permanentlyDisabled)
    {
        b.ready = false;
        return false;
    }

    int mappableCount = 0;
    for (int i = 0; i < UC_MAX_MAPPABLES; ++i)
    {
        if (slots[i] && IsReadableMemory(slots[i], sizeof(void*)))
            ++mappableCount;
    }

    if (controller != b.lastController)
    {
        b.lastController = controller;
        b.loggedReady = false;
        b.lastMappableCount = -1;
        b.nextDispatchLog = 0;
        b.dispatchCallCount = 0;
        for (int i = 0; i < UC_EXPECTED_VIRTUAL_BUTTONS; ++i) b.lastVirtual[i] = 0.0f;
        Log("Test 66 UserController discovered: InputManager=%p UserController=%p mMappableBase=%p",
            manager, controller, reinterpret_cast<uint8_t*>(controller) + UC_MAPPABLE_OFFSET);
    }

    if (mappableCount != b.lastMappableCount)
    {
        b.lastMappableCount = mappableCount;
        Log("Registered SHAR Mappables: count=%d", mappableCount);
        LogMappableSlots(controller);
    }

    if (mappableCount <= 0)
    {
        b.ready = false;
        if (now >= b.nextStatusLog)
        {
            b.nextStatusLog = now + 2000;
            Log("SHAR dispatch bridge waiting: no readable registered Mappables yet. controller=%p", controller);
        }
        return false;
    }

    b.ready = true;
    if (!b.loggedReady)
    {
        b.loggedReady = true;
        Log("SDL -> SHAR DISPATCH BRIDGE READY: controller=%p mappables=%d", controller, mappableCount);
        Log("Bridge route: SDL3 -> local SharButton -> every UserController mMappable[] -> Mappable::DispatchOnButton.");
        Log("Test 66 preserves Test 62 safety: NO UserController writes and NO guessed mNumButtons/mButtonArray offsets.");
    }
    return true;
}

static bool SafeReadSharTickCount(uint32_t& tickOut)
{
    tickOut = 0;
    if (!g_sharBridge.tickCount || !IsReadableMemory(g_sharBridge.tickCount, sizeof(uint32_t)))
        return false;
    __try
    {
        tickOut = *g_sharBridge.tickCount;
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
    }
}

static bool SafeDispatchToMappable(void* mappable, int virtualId, const SharButton* button, DWORD& exceptionOut)
{
    exceptionOut = 0;
    if (!mappable || !button || !IsReadableMemory(mappable, sizeof(void*))) return false;
    __try
    {
        g_sharBridge.DispatchOnButton(mappable, 0, virtualId, button);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        exceptionOut = GetExceptionCode();
        return false;
    }
}

static bool DispatchVirtualInput(int virtualId, float state)
{
    SharVirtualInputBridge& b = g_sharBridge;
    if (!b.ready || !b.lastController || virtualId < 0 || virtualId >= UC_EXPECTED_VIRTUAL_BUTTONS)
        return false;

    if (state < 0.0f) state = 0.0f;
    if (state > 1.0f) state = 1.0f;

    float previous = b.lastVirtual[virtualId];
    bool wasDown = previous > 0.0001f;
    bool isDown = state > 0.0001f;
    float delta = state - previous;
    if (delta < 0.0f) delta = -delta;
    bool changed = delta > 0.0025f;

    // Match UserController::Update semantics: held/non-zero values are rebroadcast
    // continuously; zero is sent once when releasing a value owned by this bridge.
    if (!isDown && !wasDown)
        return true;

    void* slots[UC_MAX_MAPPABLES] = {};
    if (!SafeSnapshotMappableSlots(b.lastController, slots))
    {
        b.ready = false;
        b.nextProbe = 0;
        return false;
    }

    uint32_t tick = 0;
    SafeReadSharTickCount(tick);
    SharButton localButton = { state, tick };

    int attempted = 0;
    int succeeded = 0;
    DWORD firstException = 0;
    int firstExceptionSlot = -1;
    for (int i = 0; i < UC_MAX_MAPPABLES; ++i)
    {
        void* mappable = slots[i];
        if (!mappable || !IsReadableMemory(mappable, sizeof(void*))) continue;
        ++attempted;
        DWORD seh = 0;
        if (SafeDispatchToMappable(mappable, virtualId, &localButton, seh))
        {
            ++succeeded;
            ++b.dispatchCallCount;
        }
        else if (seh && !firstException)
        {
            firstException = seh;
            firstExceptionSlot = i;
        }
    }

    if (firstException)
    {
        Log("Mappable::DispatchOnButton SEH: slot=%d code=%d state=%.3f exception=0x%08lX; bridge will re-probe.",
            firstExceptionSlot, virtualId, state, firstException);
        b.ready = false;
        b.nextProbe = 0;
        return false;
    }

    if (attempted <= 0 || succeeded <= 0)
    {
        b.ready = false;
        b.nextProbe = 0;
        return false;
    }

    b.lastVirtual[virtualId] = state;

    ULONGLONG now = GetTickCount64();
    if ((isDown || wasDown || changed) && now >= b.nextDispatchLog)
    {
        b.nextDispatchLog = now + 1000;
        Log("INPUT DISPATCH code=%d state=%.3f previous=%.3f mappables=%d/%d totalCalls=%llu",
            virtualId, state, previous, succeeded, attempted,
            static_cast<unsigned long long>(b.dispatchCallCount));
    }
    return true;
}

static float ClampUnit(float value)
{
    if (value < -1.0f) return -1.0f;
    if (value > 1.0f) return 1.0f;
    return value;
}

static float RawSignedAxis(int16_t raw)
{
    return raw < 0 ? static_cast<float>(raw) / 32768.0f : static_cast<float>(raw) / 32767.0f;
}

static void ApplyRadialDeadzone(float& x, float& y, float innerDeadzone)
{
    x = ClampUnit(x);
    y = ClampUnit(y);

    float magnitude = sqrtf(x * x + y * y);
    if (magnitude <= innerDeadzone)
    {
        x = 0.0f;
        y = 0.0f;
        return;
    }

    if (magnitude > 1.0f) magnitude = 1.0f;
    float scaledMagnitude = (magnitude - innerDeadzone) / (1.0f - innerDeadzone);
    if (scaledMagnitude > 1.0f) scaledMagnitude = 1.0f;

    float sourceMagnitude = sqrtf(x * x + y * y);
    if (sourceMagnitude <= 0.00001f)
    {
        x = 0.0f;
        y = 0.0f;
        return;
    }

    float scale = scaledMagnitude / sourceMagnitude;
    x = ClampUnit(x * scale);
    y = ClampUnit(y * scale);
}

static float NormalizeTriggerAxis(int16_t raw)
{
    // SDL3 exposes the DualSense triggers as 0..32767.  Keep a small deadzone
    // at the physical rest point, then rescale so the usable range is still 0..1.
    if (raw <= 0) return 0.0f;
    float value = static_cast<float>(raw) / 32767.0f;
    const float deadzone = 0.05f;
    if (value <= deadzone) return 0.0f;
    value = (value - deadzone) / (1.0f - deadzone);
    if (value > 1.0f) value = 1.0f;
    return value;
}

static float MaxF(float a, float b)
{
    return a > b ? a : b;
}

static float Digitalize(float value, float threshold = 0.55f)
{
    return value >= threshold ? 1.0f : 0.0f;
}

static void ServiceSharVirtualInput(SDLApi& sdl, SDL_Gamepad* pad, ULONGLONG now)
{
    if (!pad || !ProbeSharVirtualBridge(now) || g_sharBridge.permanentlyDisabled) return;

    // Test 63 preserves Test 62's proven Mappable::DispatchOnButton bridge.
    // Only input conditioning/mappings below this point are changed.
    float lx = RawSignedAxis(sdl.GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_LEFTX));
    float ly = RawSignedAxis(sdl.GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_LEFTY));
    float rx = RawSignedAxis(sdl.GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_RIGHTX));
    float ry = RawSignedAxis(sdl.GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_RIGHTY));

    // Radial deadzones avoid the square/axis-biased feel of Test 62 while also
    // suppressing the tiny idle values seen in the successful Test 62 log.
    ApplyRadialDeadzone(lx, ly, 0.12f);
    ApplyRadialDeadzone(rx, ry, 0.14f);

    float l2 = NormalizeTriggerAxis(sdl.GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER));
    float r2 = NormalizeTriggerAxis(sdl.GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER));

    bool dpadUp = sdl.GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_DPAD_UP);
    bool dpadDown = sdl.GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_DPAD_DOWN);
    bool dpadLeft = sdl.GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_DPAD_LEFT);
    bool dpadRight = sdl.GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_DPAD_RIGHT);

    float up = MaxF(ly < 0.0f ? -ly : 0.0f, dpadUp ? 1.0f : 0.0f);
    float down = MaxF(ly > 0.0f ? ly : 0.0f, dpadDown ? 1.0f : 0.0f);
    float left = MaxF(lx < 0.0f ? -lx : 0.0f, dpadLeft ? 1.0f : 0.0f);
    float right = MaxF(lx > 0.0f ? lx : 0.0f, dpadRight ? 1.0f : 0.0f);

    // Character movement. On Win32 these virtual inputs feed the Character
    // Mappable's DPad values. Fractional values preserve the DualSense stick
    // magnitude, giving us analog walking/running without inventing IDs 200/201.
    DispatchVirtualInput(VI_MOVE_UP, up);
    DispatchVirtualInput(VI_MOVE_DOWN, down);
    DispatchVirtualInput(VI_MOVE_LEFT, left);
    DispatchVirtualInput(VI_MOVE_RIGHT, right);

    // Frontend navigation gets a deliberate digital threshold.  This prevents
    // slight stick movement from accidentally stepping through menus while the
    // physical D-pad remains immediate and fully digital.
    float feUp = dpadUp ? 1.0f : Digitalize(ly < 0.0f ? -ly : 0.0f);
    float feDown = dpadDown ? 1.0f : Digitalize(ly > 0.0f ? ly : 0.0f);
    float feLeft = dpadLeft ? 1.0f : Digitalize(lx < 0.0f ? -lx : 0.0f);
    float feRight = dpadRight ? 1.0f : Digitalize(lx > 0.0f ? lx : 0.0f);
    DispatchVirtualInput(VI_FE_MOVE_UP, feUp);
    DispatchVirtualInput(VI_FE_MOVE_DOWN, feDown);
    DispatchVirtualInput(VI_FE_MOVE_LEFT, feLeft);
    DispatchVirtualInput(VI_FE_MOVE_RIGHT, feRight);

    // Vehicle steering uses the same conditioned left-stick values.  The KBD
    // aliases are retained because some SHAR frontend/minigame paths consume them.
    DispatchVirtualInput(VI_STEER_LEFT, left);
    DispatchVirtualInput(VI_STEER_RIGHT, right);
    DispatchVirtualInput(VI_P1_KBD_LEFT, left);
    DispatchVirtualInput(VI_P1_KBD_RIGHT, right);

    // Preserve the proven Test 62 right-stick camera route and Test 63 deadzone.
    // Test 64 correction: R3 is reserved for Camera Change and must not also
    // trigger Look Back. Right-stick-down keeps the existing look-back route.
    bool r3CameraToggle = sdl.GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_RIGHT_STICK);
    float camLeft = rx < 0.0f ? -rx : 0.0f;
    float camRight = rx > 0.0f ? rx : 0.0f;
    float camUp = ry < 0.0f ? -ry : 0.0f;
    float camDown = ry > 0.0f ? ry : 0.0f;
    float carLookBack = camDown;
    DispatchVirtualInput(VI_CAMERA_LEFT, camLeft);
    DispatchVirtualInput(VI_CAMERA_RIGHT, camRight);
    DispatchVirtualInput(VI_CAMERA_MOVE_IN, camUp);
    DispatchVirtualInput(VI_CAMERA_MOVE_OUT, camDown);
    DispatchVirtualInput(VI_CAMERA_CAR_LEFT, camLeft);
    DispatchVirtualInput(VI_CAMERA_CAR_RIGHT, camRight);
    DispatchVirtualInput(VI_CAMERA_CAR_LOOK_UP, camUp);
    DispatchVirtualInput(VI_CAMERA_CAR_LOOK_BACK, carLookBack);

    bool cross = sdl.GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_SOUTH);
    bool circle = sdl.GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_EAST);
    bool square = sdl.GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_WEST);
    bool triangle = sdl.GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_NORTH);
    bool create = sdl.GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_BACK);
    bool options = sdl.GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_START);
    bool l3 = sdl.GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_LEFT_STICK);
    bool l1 = sdl.GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER);
    bool r1 = sdl.GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER);

    float crossValue = cross ? 1.0f : 0.0f;
    float circleValue = circle ? 1.0f : 0.0f;
    float squareValue = square ? 1.0f : 0.0f;
    float triangleValue = triangle ? 1.0f : 0.0f;

    // On-foot PlayStation layout.
    bool forcedJump = now < g_forceJumpUntil;
    DispatchVirtualInput(VI_JUMP, (cross || forcedJump) ? 1.0f : 0.0f);
    DispatchVirtualInput(VI_ATTACK, squareValue);
    DispatchVirtualInput(VI_SPRINT, circleValue);
    DispatchVirtualInput(VI_DO_ACTION, triangleValue);
    DispatchVirtualInput(VI_GET_OUT_CAR, triangleValue);

    // Frontend/pause-menu controls.  Options keeps both aliases used by the PC
    // game; Create remains Back while also serving Reset Car during gameplay.
    DispatchVirtualInput(VI_FE_SELECT, (cross || options) ? 1.0f : 0.0f);
    DispatchVirtualInput(VI_FE_BACK, (circle || create) ? 1.0f : 0.0f);
    DispatchVirtualInput(VI_FE_FUNCTION1, squareValue);
    DispatchVirtualInput(VI_FE_FUNCTION2, triangleValue);
    DispatchVirtualInput(VI_P1_KBD_START, options ? 1.0f : 0.0f);

    // Modern DualSense driving layout with PS2-style face-button fallbacks:
    //   R2 / Cross   = accelerate
    //   L2 / Circle  = brake/reverse
    //   Square       = handbrake
    //   Triangle     = get out/action
    //   R3           = camera toggle
    //   R1 or L3     = horn
    //   Create       = reset car
    float accelerate = MaxF(r2, crossValue);
    float reverse = MaxF(l2, circleValue);
    DispatchVirtualInput(VI_ACCELERATE, accelerate);
    DispatchVirtualInput(VI_REVERSE, reverse);
    DispatchVirtualInput(VI_P1_KBD_GAS, accelerate);
    DispatchVirtualInput(VI_P1_KBD_BRAKE, reverse);
    DispatchVirtualInput(VI_HANDBRAKE, squareValue);
    DispatchVirtualInput(VI_P1_KBD_EBRAKE, squareValue);
    DispatchVirtualInput(VI_HORN, (r1 || l3) ? 1.0f : 0.0f);
    DispatchVirtualInput(VI_RESET_CAR, create ? 1.0f : 0.0f);
    DispatchVirtualInput(VI_CAMERA_TOGGLE, r3CameraToggle ? 1.0f : 0.0f);


    // On-foot trigger camera helpers.  Vehicle mappables ignore these camera
    // virtual inputs when not relevant, while vehicle acceleration/braking keep
    // the same analog trigger values above.
    DispatchVirtualInput(VI_CAMERA_ZOOM, l2);
    DispatchVirtualInput(VI_CAMERA_LOOK_UP, r2);
}

static uint16_t ScaleRumble(float value, uint16_t maximum)
{
    if (value <= 0.0f) return 0;
    if (value >= 1.0f) return maximum;
    return static_cast<uint16_t>(value * static_cast<float>(maximum));
}

static void QueueHapticPulse(uint16_t low, uint16_t high, unsigned int durationMs, const char* label, ULONGLONG now)
{
    if (low > g_haptics.pulseLow) g_haptics.pulseLow = low;
    if (high > g_haptics.pulseHigh) g_haptics.pulseHigh = high;
    ULONGLONG until = now + durationMs;
    if (until > g_haptics.pulseUntil) g_haptics.pulseUntil = until;
    Log("HAPTIC PULSE %-12s low=%u high=%u duration=%ums", label ? label : "event", low, high, durationMs);
}

static void ServiceQueuedSharEventHaptics(ULONGLONG now, int gameplayMode)
{
    if (!g_eventBridge.ready) return;

    for (int i = 0; i < HE_COUNT; ++i)
    {
        LONG current = InterlockedCompareExchange(&g_sharEventCounters[i], 0, 0);
        if (current == g_sharEventConsumed[i]) continue;
        g_sharEventConsumed[i] = current;

        const SharHapticEventDefinition& def = g_sharHapticEvents[i];
        LONG rawParam = InterlockedCompareExchange(&g_sharEventParams[i], 0, 0);

        // Keep player-state-sensitive events from buzzing while their relevant
        // mappable is not active. Generic collision/breakable events are mode 0.
        if (def.requiredMode != 0 && def.requiredMode != gameplayMode)
        {
            Log("SHAR EVENT observed id=%d %-28s count=%ld param=0x%08lX ignoredMode=%d currentMode=%d",
                def.eventId, def.name, current, (unsigned long)rawParam, def.requiredMode, gameplayMode);
            continue;
        }

        if (now < g_sharEventLastPulse[i] + def.cooldownMs) continue;
        g_sharEventLastPulse[i] = now;
        QueueHapticPulse(def.low, def.high, def.durationMs, def.name, now);
        Log("SHAR EVENT HAPTIC id=%d %-28s count=%ld param=0x%08lX mode=%d",
            def.eventId, def.name, current, (unsigned long)rawParam, gameplayMode);
    }
}

static void ServiceDualSenseHaptics(SDLApi& sdl, SDL_Gamepad* pad, ULONGLONG now)
{
    if (!pad || !sdl.RumbleGamepad) return;

    int mappables = g_sharBridge.ready ? g_sharBridge.lastMappableCount : 0;
    int gameplayMode = (mappables == 4) ? 1 : ((mappables >= 5) ? 2 : 0);
    if (gameplayMode != g_haptics.lastGameplayMode)
    {
        g_haptics.lastGameplayMode = gameplayMode;
        Log("HAPTIC MODE: %s (registered SHAR mappables=%d)",
            gameplayMode == 1 ? "ON-FOOT" : (gameplayMode == 2 ? "VEHICLE" : "MENU/TRANSITION"), mappables);
    }

    ServiceQueuedSharEventHaptics(now, gameplayMode);

    bool cross = sdl.GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_SOUTH);
    bool circle = sdl.GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_EAST);
    bool square = sdl.GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_WEST);
    bool triangle = sdl.GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_NORTH);

    // Short tactile signatures. They are intentionally modest: this test proves
    // the mixer/state gating before we wire collisions, mission events and SHAR's
    // own RumbleEffect requests into the same output path.
    if (gameplayMode == 1)
    {
        if (cross && !g_haptics.previousCross) QueueHapticPulse(0x2400, 0x4800, 55, "jump", now);
        if (square && !g_haptics.previousSquare) QueueHapticPulse(0x5800, 0x9000, 75, "attack", now);
        if (triangle && !g_haptics.previousTriangle) QueueHapticPulse(0x3000, 0x6000, 60, "action", now);
    }
    else if (gameplayMode == 2)
    {
        if (square && !g_haptics.previousSquare) QueueHapticPulse(0x5000, 0x7800, 70, "handbrake", now);
        if (triangle && !g_haptics.previousTriangle) QueueHapticPulse(0x2800, 0x5000, 55, "get-out", now);
        if (circle && !g_haptics.previousCircle) QueueHapticPulse(0x3000, 0x4800, 50, "reverse", now);
    }

    g_haptics.previousCross = cross;
    g_haptics.previousCircle = circle;
    g_haptics.previousSquare = square;
    g_haptics.previousTriangle = triangle;

    uint16_t baseLow = 0;
    uint16_t baseHigh = 0;

    // Vehicle proof texture: throttle creates a low-frequency engine-like rumble.
    // This is input/state-derived, not yet real RPM. It exists so we can validate
    // sustained SDL rumble, mixing and clean stop behavior before deeper SHAR hooks.
    if (gameplayMode == 2)
    {
        float throttle = NormalizeTriggerAxis(sdl.GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER));
        if (throttle > 0.02f)
        {
            baseLow = static_cast<uint16_t>(0x0800 + ScaleRumble(throttle, 0x2800));
            baseHigh = ScaleRumble(throttle, 0x0900);
        }
    }

    uint16_t low = baseLow;
    uint16_t high = baseHigh;
    if (now < g_haptics.pulseUntil)
    {
        if (g_haptics.pulseLow > low) low = g_haptics.pulseLow;
        if (g_haptics.pulseHigh > high) high = g_haptics.pulseHigh;
    }
    else
    {
        g_haptics.pulseLow = 0;
        g_haptics.pulseHigh = 0;
    }

    bool changed = low != g_haptics.lastLow || high != g_haptics.lastHigh;
    bool refresh = (low || high) && now >= g_haptics.nextRefresh;
    if (changed || refresh)
    {
        uint32_t duration = (low || high) ? 120u : 0u;
        bool ok = sdl.RumbleGamepad(pad, low, high, duration);
        if (!ok)
            Log("HAPTIC OUTPUT FAILED low=%u high=%u error=%s", low, high, sdl.GetError ? sdl.GetError() : "unknown");
        g_haptics.lastLow = low;
        g_haptics.lastHigh = high;
        g_haptics.nextRefresh = now + 80;
    }
}

static bool TestSharJump(ULONGLONG now)
{
    if (!ProbeSharVirtualBridge(now) || g_sharBridge.permanentlyDisabled)
    {
        Log("F8 virtual-input Jump: bridge not ready.");
        return false;
    }
    g_forceJumpUntil = now + 180;
    bool sent = DispatchVirtualInput(VI_JUMP, 1.0f);
    Log("F8 direct-dispatch Jump TEST ARMED for 180ms (virtual input Jump=%d, initialDispatch=%s).",
        VI_JUMP, sent ? "SUCCESS" : "FAILED");
    return sent;
}

static DWORD WINAPI ControllerThread(LPVOID)
{
    std::wstring moduleDir = ModuleDirectory(g_hookModule);
    std::wstring nexusDir = ParentDirectory(moduleDir);
    std::wstring logPath = nexusDir + L"\\NexusDualSense_Hook.log";
    _wfopen_s(&g_log, logPath.c_str(), L"a");
    if (!g_log)
    {
        logPath = moduleDir + L"\\NexusDualSense_Hook.log";
        _wfopen_s(&g_log, logPath.c_str(), L"a");
    }
    Log("============================================================");
    Log("Springfield Nexus DualSense Hack - Prototype 10 / Test 66 SHAR event haptics loaded in PID %lu", GetCurrentProcessId());
    Log("Hook module directory: %ls", moduleDir.c_str());
    Log("Diagnostics log path: %ls", logPath.c_str());

    SDLApi sdl;
    if (!LoadSDL(sdl))
    {
        Log("SDL binding failed; hook thread exiting.");
        return 1;
    }

    sdl.SetHint("SDL_JOYSTICK_HIDAPI_PS5", "1");
    sdl.SetHint("SDL_JOYSTICK_ENHANCED_REPORTS", "1");

    if (!sdl.Init(SDL_INIT_GAMEPAD | SDL_INIT_EVENTS | SDL_INIT_SENSOR))
    {
        Log("SDL_Init failed: %s", sdl.GetError ? sdl.GetError() : "unknown");
        return 2;
    }
    Log("SDL3 initialized inside Simpsons.exe.");
    InitializeSharBridge();

    SDL_Gamepad* pad = nullptr;
    bool lastButtons[SDL_GAMEPAD_BUTTON_COUNT] = {};
    bool lastTouchDown = false;
    bool lastF6 = false;
    bool lastF7 = false;
    bool lastF8 = false;
    ULONGLONG nextGameplayService = 0;
    int ledIndex = 0;
    ULONGLONG nextOpenAttempt = 0;
    ULONGLONG nextStateLog = 0;

    while (InterlockedCompareExchange(&g_stop, 0, 0) == 0)
    {
        sdl.PumpEvents();
        ULONGLONG now = GetTickCount64();

        if (!pad && now >= nextOpenAttempt)
        {
            nextOpenAttempt = now + 1000;
            pad = OpenPreferredGamepad(sdl);
            if (pad)
            {
                const char* name = sdl.GetGamepadName(pad);
                Log("Controller opened: %s", name ? name : "(unknown)");
                if (sdl.GamepadHasSensor(pad, SDL_SENSOR_GYRO))
                {
                    sdl.SetGamepadSensorEnabled(pad, SDL_SENSOR_GYRO, true);
                    Log("Gyroscope available and enabled.");
                }
                if (sdl.GamepadHasSensor(pad, SDL_SENSOR_ACCEL))
                {
                    sdl.SetGamepadSensorEnabled(pad, SDL_SENSOR_ACCEL, true);
                    Log("Accelerometer available and enabled.");
                }
                int touchpads = sdl.GetNumGamepadTouchpads(pad);
                Log("Touchpads reported: %d", touchpads);
                Log("Hotkeys: F6 = manual rumble, F7 = lightbar, F8 = direct Jump proof. Test 66 adds real SHAR EventManager haptics without changing Test 64 controls.");
                Log("Test 66 controls LOCKED from Test 64: Cross jump/select/accel; Circle sprint/back/brake; Square attack/handbrake; Triangle action/get-out; R3 camera change; R1/L3 horn; Create reset; R2/L2 analog driving.");
            }
        }

        if (pad)
        {
            for (int b = 0; b < SDL_GAMEPAD_BUTTON_COUNT; ++b)
            {
                bool down = sdl.GetGamepadButton(pad, (SDL_GamepadButton)b);
                if (down != lastButtons[b])
                {
                    Log("BUTTON %s: %s", ButtonName(b), down ? "DOWN" : "UP");
                    lastButtons[b] = down;
                }
            }

            bool touchDown = false;
            float tx = 0.0f, ty = 0.0f, pressure = 0.0f;
            if (sdl.GetNumGamepadTouchpads(pad) > 0 && sdl.GetNumGamepadTouchpadFingers(pad, 0) > 0 &&
                sdl.GetGamepadTouchpadFinger(pad, 0, 0, &touchDown, &tx, &ty, &pressure))
            {
                if (touchDown != lastTouchDown)
                {
                    Log("TOUCHPAD finger0: %s x=%.3f y=%.3f pressure=%.3f", touchDown ? "DOWN" : "UP", tx, ty, pressure);
                    lastTouchDown = touchDown;
                }
            }

            if (now >= nextStateLog)
            {
                nextStateLog = now + 2000;
                int16_t lx = sdl.GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_LEFTX);
                int16_t ly = sdl.GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_LEFTY);
                int16_t rx = sdl.GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_RIGHTX);
                int16_t ry = sdl.GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_RIGHTY);
                int16_t l2 = sdl.GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER);
                int16_t r2 = sdl.GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER);
                float gyro[3] = {}, accel[3] = {};
                bool hasGyro = sdl.GetGamepadSensorData(pad, SDL_SENSOR_GYRO, gyro, 3);
                bool hasAccel = sdl.GetGamepadSensorData(pad, SDL_SENSOR_ACCEL, accel, 3);
                Log("STATE LX=%d LY=%d RX=%d RY=%d L2=%d R2=%d%s%s",
                    (int)lx, (int)ly, (int)rx, (int)ry, (int)l2, (int)r2,
                    hasGyro ? " gyro=OK" : " gyro=N/A", hasAccel ? " accel=OK" : " accel=N/A");
            }

            if (now >= nextGameplayService)
            {
                nextGameplayService = now + 16;
                ServiceSharVirtualInput(sdl, pad, now);
                ServiceDualSenseHaptics(sdl, pad, now);
            }

            bool f6 = (GetAsyncKeyState(VK_F6) & 0x8000) != 0;
            if (f6 && !lastF6)
            {
                bool ok = sdl.RumbleGamepad(pad, 0xB000, 0xFFFF, 500);
                Log("F6 rumble test: %s%s", ok ? "SUCCESS" : "FAILED", ok ? "" : (sdl.GetError ? sdl.GetError() : ""));
            }
            lastF6 = f6;

            bool f7 = (GetAsyncKeyState(VK_F7) & 0x8000) != 0;
            if (f7 && !lastF7)
            {
                static const uint8_t colors[][3] = { {255, 48, 48}, {48, 255, 96}, {48, 112, 255}, {180, 64, 255}, {255, 190, 48} };
                const uint8_t* c = colors[ledIndex++ % 5];
                bool ok = sdl.SetGamepadLED(pad, c[0], c[1], c[2]);
                Log("F7 lightbar test RGB(%u,%u,%u): %s", c[0], c[1], c[2], ok ? "SUCCESS" : "FAILED");
            }
            lastF7 = f7;
        }

        // Keep probing the real SHAR InputManager even before a pad is opened, so
        // diagnostics can report mappable registration as soon as it exists.
        ProbeSharVirtualBridge(now);
        ProbeSharEventBridge(now);

        bool f8 = (GetAsyncKeyState(VK_F8) & 0x8000) != 0;
        if (f8 && !lastF8)
        {
            TestSharJump(now);
        }
        lastF8 = f8;

        Sleep(10);
    }

    if (pad) sdl.CloseGamepad(pad);
    sdl.QuitSubSystem(SDL_INIT_GAMEPAD | SDL_INIT_EVENTS | SDL_INIT_SENSOR);
    Log("Hook thread stopped.");
    if (g_log) { fclose(g_log); g_log = nullptr; }
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        g_hookModule = hModule;
        DisableThreadLibraryCalls(hModule);
        HANDLE thread = CreateThread(nullptr, 0, ControllerThread, nullptr, 0, nullptr);
        if (thread) CloseHandle(thread);
    }
    else if (reason == DLL_PROCESS_DETACH)
    {
        InterlockedExchange(&g_stop, 1);
    }
    return TRUE;
}
