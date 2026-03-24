#ifndef VERSION_H
#define VERSION_H

#include <cstdint>

// Enable NtProtectVirtualMemory hook fix for v4.00+ clients
#define FIX_NTPROTECTVIRTUALMEMORY 1

#ifdef _WIN64
// Standalone/legacy client prologue
static const uint8_t g_nt_protect_virtual_memory_prologue[] = {0x4c, 0x8b, 0xd1, 0xb8, 0x50};
static const char* g_set_fps_limit_pattern =
    "4C 8B DC 48 81 EC C8 00 00 00 48 8B ? ? ? ? ? 48 33 C4 48 89 84 24 B0 00 00 00 0F 57 C0 0F 2F C8";

#else
// Steam/Ubiconnect client prologue
static const uint8_t g_nt_protect_virtual_memory_prologue[] = {0xb8, 0x50, 0x00, 0x00, 0x00};


static const char* g_set_fps_limit_pattern = "55 8B EC 6A FF 68 ? ? ? ? 64 A1 00 00 00 00 50 83 EC 6C A1 ? ? ? ? 33 C5 89 "
                                             "45 F0 50 8D 45 F4 64 A3 00 00 00 00 F3 0F 10 4D 08";
#endif

#endif // VERSION_H
