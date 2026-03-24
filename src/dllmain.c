#include <minhook.h>
#include <stdint.h>
#include <stdlib.h>
#include <wchar.h>
#include <windows.h>

#include "fps.h"
#include "utils.h"
#include "version.h"

#define PATTERN_BUFFER_MAX_COUNT 1024

typedef HRESULT(WINAPI* direct_input8_create_fn_t)(HINSTANCE, DWORD, REFIID, LPVOID*, LPVOID);
static direct_input8_create_fn_t g_original_direct_input8_create = NULL;

int fix_nt_protect_virtual_memory_if_needed(void)
{
#ifdef FIX_NTPROTECTVIRTUALMEMORY
    HMODULE ntdll = GetModuleHandleA("ntdll");
    void* fn = GetProcAddress(ntdll, "NtProtectVirtualMemory");
    if (!patch_memory(fn, g_nt_protect_virtual_memory_prologue, sizeof(g_nt_protect_virtual_memory_prologue)))
    {
        return 0;
    }
#endif
    return 1;
}

int hook_set_fps_limit(void)
{
    optional_byte_t pattern_bytes[PATTERN_BUFFER_MAX_COUNT] = {0};
    int32_t pattern_length = parse_pattern(g_set_fps_limit_pattern, pattern_bytes, PATTERN_BUFFER_MAX_COUNT);

    if (pattern_length <= 0)
    {
        return 0;
    }
    void* fn = find_memory(pattern_bytes, (size_t)pattern_length);

    if (!fn)
    {
        return 0;
    }
    if (MH_CreateHook(fn, set_fps_limit, (void**)&g_original_set_fps_limit) != MH_OK)
    {
        return 0;
    }
    if (MH_EnableHook(fn) != MH_OK)
    {
        return 0;
    }

    return 1;
}

void setup(void)
{
    SendMessageA(get_client_window(), WM_NULL, 0, 0);

    if (!fix_nt_protect_virtual_memory_if_needed())
    {
        show_error_message_box("Failed to patch NtProtectVirtualMemory. Please report this issue on GitHub.");
        return;
    }

    if (MH_Initialize() != MH_OK)
    {
        show_error_message_box("Failed to initialize MinHook. Please report this issue on GitHub.");
        return;
    }

    if (!hook_set_fps_limit())
    {
        show_error_message_box("Failed to hook SetFPSLimit. Please report this issue on GitHub.");
        return;
    }

    set_optimal_fps_limit();
    SetWindowTextA(get_client_window(), "Growtopia (FPS Limit Unlocked)");
}

DWORD WINAPI setup_thread_proc(LPVOID param)
{
    (void)param;
    setup();
    return 0;
}

void load_original_direct_input8_create(void)
{
    uint32_t sys_path_size = GetSystemDirectoryW(NULL, 0);
    uint32_t module_path_size = sys_path_size + (uint32_t)wcslen(L"\\dinput8.dll") + 1;
    wchar_t* module_path = (wchar_t*)calloc(module_path_size, sizeof(wchar_t));
    if (!module_path)
    {
        return;
    }
    GetSystemDirectoryW(module_path, sys_path_size);
    wcsncat(module_path, L"\\dinput8.dll", module_path_size - wcslen(module_path) - 1);

    HMODULE dinput8 = LoadLibraryW(module_path);
    free(module_path);
    if (!dinput8)
    {
        return;
    }
    g_original_direct_input8_create = (direct_input8_create_fn_t)GetProcAddress(dinput8, "DirectInput8Create");
}

HRESULT WINAPI DirectInput8Create(HINSTANCE hinst, DWORD dwversion, REFIID riidltf, LPVOID* ppvout, LPVOID punkouter)
{
    static long is_initialized = 0;
    if (!InterlockedCompareExchange(&is_initialized, 1, 0))
    {
        load_original_direct_input8_create();
        if (g_original_direct_input8_create)
        {
            HANDLE setup_thread = CreateThread(NULL, 0, setup_thread_proc, NULL, 0, NULL);
            if (setup_thread)
            {
                CloseHandle(setup_thread);
            }
        }
    }

    if (!g_original_direct_input8_create)
    {
        show_error_message_box("Failed to load original DirectInput8Create function. Please report this issue on GitHub.");
        return E_FAIL;
    }
    return g_original_direct_input8_create(hinst, dwversion, riidltf, ppvout, punkouter);
}

BOOL WINAPI DllMain(HINSTANCE hinst_dll, DWORD fdw_reason, LPVOID lpv_reserved)
{
    (void)lpv_reserved;
    switch (fdw_reason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hinst_dll);
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
