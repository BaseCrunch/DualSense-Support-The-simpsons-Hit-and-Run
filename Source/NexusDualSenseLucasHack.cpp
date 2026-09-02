typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long DWORD;
typedef int BOOL;
typedef void* HMODULE;
typedef void* FARPROC;

extern "C" void* __cdecl memset(void* dst, int c, unsigned int n) {
    unsigned char* p = (unsigned char*)dst;
    for (unsigned int i = 0; i < n; ++i) p[i] = (unsigned char)c;
    return dst;
}

static HMODULE g_self = 0;
static volatile long g_loaded = 0;

static __forceinline u16 rd16(const void* p) { return *(const volatile u16*)p; }
static __forceinline u32 rd32(const void* p) { return *(const volatile u32*)p; }

extern "C" __declspec(naked) void* __cdecl GetPeb32() {
    __asm {
        mov eax, fs:[0x30]
        ret
    }
}

static int streq(const char* a, const char* b) {
    while (*a && *b && *a == *b) { ++a; ++b; }
    return (unsigned char)*a == (unsigned char)*b;
}

static FARPROC find_export(HMODULE module, const char* target) {
    if (!module) return 0;
    u8* base = (u8*)module;
    if (rd16(base) != 0x5A4D) return 0;
    u32 peoff = rd32(base + 0x3C);
    if (rd32(base + peoff) != 0x00004550) return 0;
    u8* opt = base + peoff + 24;
    if (rd16(opt) != 0x10B) return 0;
    u32 expRva = rd32(opt + 96);
    if (!expRva) return 0;
    u8* exp = base + expRva;
    u32 nNames = rd32(exp + 24);
    u32 funcsRva = rd32(exp + 28);
    u32 namesRva = rd32(exp + 32);
    u32 ordsRva = rd32(exp + 36);
    u32* names = (u32*)(base + namesRva);
    u16* ords = (u16*)(base + ordsRva);
    u32* funcs = (u32*)(base + funcsRva);
    for (u32 i = 0; i < nNames; ++i) {
        const char* name = (const char*)(base + names[i]);
        if (streq(name, target)) {
            u16 ord = ords[i];
            return (FARPROC)(base + funcs[ord]);
        }
    }
    return 0;
}

static FARPROC find_proc_any_module(const char* target) {
    u8* peb = (u8*)GetPeb32();
    if (!peb) return 0;
    u8* ldr = *(u8**)(peb + 0x0C);
    if (!ldr) return 0;
    u8* head = ldr + 0x14; // PEB_LDR_DATA.InMemoryOrderModuleList
    u8* node = *(u8**)head;
    int guard = 0;
    while (node && node != head && guard++ < 128) {
        u8* entry = node - 0x08; // LDR_DATA_TABLE_ENTRY.InMemoryOrderLinks
        HMODULE mod = *(HMODULE*)(entry + 0x18); // DllBase
        FARPROC p = find_export(mod, target);
        if (p) return p;
        node = *(u8**)node;
    }
    return 0;
}

typedef DWORD (__stdcall *PFN_GetModuleFileNameW)(HMODULE, wchar_t*, DWORD);
typedef HMODULE (__stdcall *PFN_GetModuleHandleW)(const wchar_t*);
typedef HMODULE (__stdcall *PFN_LoadLibraryW)(const wchar_t*);

static int ascii_lower_w(wchar_t c) {
    if (c >= L'A' && c <= L'Z') c = (wchar_t)(c + (L'a' - L'A'));
    return (int)c;
}

static bool ends_with_simpsons_exe(const wchar_t* path) {
    const wchar_t* base = path;
    for (const wchar_t* p = path; *p; ++p) if (*p == L'\\' || *p == L'/') base = p + 1;
    static const wchar_t want[] = L"Simpsons.exe";
    int i = 0;
    for (; want[i] && base[i]; ++i) if (ascii_lower_w(want[i]) != ascii_lower_w(base[i])) return false;
    return want[i] == 0 && base[i] == 0;
}

static bool append_after_directory(wchar_t* path, int cap, DWORD n, const wchar_t* suffix) {
    if (!n || n >= (DWORD)(cap - 1)) return false;
    int len = (int)n;
    while (len > 0 && path[len-1] != L'\\' && path[len-1] != L'/') --len;
    if (len <= 0) return false;
    int j = 0;
    while (suffix[j]) {
        if (len + j >= cap - 1) return false;
        path[len + j] = suffix[j];
        ++j;
    }
    path[len + j] = 0;
    return true;
}

static bool build_runtime_path(wchar_t* path, int cap) {
    PFN_GetModuleFileNameW GetModuleFileNameW_ = (PFN_GetModuleFileNameW)find_proc_any_module("GetModuleFileNameW");
    PFN_GetModuleHandleW GetModuleHandleW_ = (PFN_GetModuleHandleW)find_proc_any_module("GetModuleHandleW");
    if (!GetModuleFileNameW_) return false;

    // Preferred path: Lucas' built-in DLLs\\Hacks.dll is loaded into the game.
    // Put our runtime beside it so we don't depend on the external .lmlh path.
    if (GetModuleHandleW_) {
        HMODULE lucasHacks = GetModuleHandleW_(L"Hacks.dll");
        if (lucasHacks) {
            DWORD n = GetModuleFileNameW_(lucasHacks, path, (DWORD)cap);
            if (append_after_directory(path, cap, n, L"NexusDualSenseHook.dll")) return true;
        }
    }

    // Fallback: external hack normally lives in <Launcher>\\Hacks. Walk back
    // one directory and then into <Launcher>\\DLLs.
    if (!g_self) return false;
    DWORD n = GetModuleFileNameW_(g_self, path, (DWORD)cap);
    if (!n || n >= (DWORD)(cap - 1)) return false;
    int len = (int)n;
    while (len > 0 && path[len-1] != L'\\' && path[len-1] != L'/') --len; // Hacks\\
    if (len <= 0) return false;
    --len; // remove trailing slash
    while (len > 0 && path[len-1] != L'\\' && path[len-1] != L'/') --len; // launcher root\\
    if (len <= 0) return false;
    static const wchar_t suffix[] = L"DLLs\\NexusDualSenseHook.dll";
    int j = 0;
    while (suffix[j]) {
        if (len + j >= cap - 1) return false;
        path[len + j] = suffix[j];
        ++j;
    }
    path[len + j] = 0;
    return true;
}

static bool running_in_game() {
    PFN_GetModuleFileNameW GetModuleFileNameW_ = (PFN_GetModuleFileNameW)find_proc_any_module("GetModuleFileNameW");
    if (!GetModuleFileNameW_) return false;
    wchar_t path[520] = {};
    DWORD n = GetModuleFileNameW_(0, path, 520);
    if (!n || n >= 519) return false;
    return ends_with_simpsons_exe(path);
}

static bool load_runtime() {
    if (g_loaded) return true;
    if (!running_in_game()) return true; // launcher-side metadata/event calls: no-op
    PFN_LoadLibraryW LoadLibraryW_ = (PFN_LoadLibraryW)find_proc_any_module("LoadLibraryW");
    if (!LoadLibraryW_) return false;
    wchar_t path[520] = {};
    if (!build_runtime_path(path, 520)) return false;
    HMODULE m = LoadLibraryW_(path);
    if (!m) return false;
    g_loaded = 1;
    return true;
}

extern "C" __declspec(dllexport) unsigned int __cdecl HackEntryPoint_NexusDualSense(unsigned int eventId, void* data) {
    (void)eventId;
    (void)data;
    // Lucas can call hack events in both launcher and injected game contexts.
    // On the first event delivered inside Simpsons.exe, load the existing
    // Test 66 runtime. Later events are intentionally ignored by this wrapper.
    bool ok = load_runtime();
    if (eventId == 0) return ok ? 1u : 0u;
    return 0u;
}

extern "C" int __stdcall DllMain(HMODULE hModule, DWORD reason, void* reserved) {
    (void)reserved;
    if (reason == 1) g_self = hModule; // DLL_PROCESS_ATTACH
    return 1;
}
