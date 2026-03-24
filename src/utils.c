#include "utils.h"
#include <ctype.h>
#include <psapi.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

HWND get_client_window()
{
    return FindWindowA(NULL, "Growtopia");
}

HMODULE get_client_exe_module()
{
    return GetModuleHandleA(NULL);
}

int32_t parse_pattern(const char* pattern, optional_byte_t* out_bytes, size_t max_count)
{
    int32_t count = 0;
    const char* p = pattern;

    while (*p != '\0')
    {
        if (isspace((unsigned char)*p))
        {
            p++;
            continue;
        }

        if ((size_t)count >= max_count)
        {
            return -1;
        }

        if (*p == '?')
        {
            out_bytes[count++] = WILDCARD_BYTE;
            p++;
            if (*p == '?')
            {
                p++;
            }
        }
        else if (isxdigit((unsigned char)*p))
        {
            char* end;
            unsigned long parsed = strtoul(p, &end, 16);
            if (parsed > 0xFF)
            {
                return -1;
            }

            optional_byte_t value = (optional_byte_t)parsed;
            if (value > 0xFF)
            {
                return -1;
            }
            out_bytes[count++] = value;

            if (p == end)
            {
                p++;
            }
            else
            {
                p = end;
            }
        }
        else
        {
            return -1;
        }
    }
    return count;
}

void* patch_memory(void* dest, const void* src, size_t size)
{
    DWORD old_protection = 0;
    if (!VirtualProtect(dest, size, PAGE_EXECUTE_READWRITE, &old_protection))
    {
        return NULL;
    }
    memcpy(dest, src, size);

    if (!VirtualProtect(dest, size, old_protection, &old_protection))
    {
        return NULL;
    }
    return dest;
}

void* find_memory(const optional_byte_t* pattern, size_t count)
{
    if (pattern == NULL || count == 0)
    {
        return NULL;
    }

    MODULEINFO module_info = {0};
    if (!GetModuleInformation(GetCurrentProcess(), get_client_exe_module(), &module_info, sizeof(module_info)))
    {
        return NULL;
    }

    if (count > module_info.SizeOfImage)
    {
        return NULL;
    }

    uint8_t* begin = module_info.lpBaseOfDll;
    uint8_t* end = begin + module_info.SizeOfImage;
    uint8_t* soft_end = end - count;

    size_t anchor_index = 0;
    while (anchor_index < count && pattern[anchor_index] == WILDCARD_BYTE)
    {
        ++anchor_index;
    }

    if (anchor_index == count)
    {
        return begin;
    }

    uint8_t* p = begin;
    while (p <= soft_end)
    {
        size_t remaining = (size_t)(end - (p + anchor_index));
        uint8_t* anchor = memchr(p + anchor_index, pattern[anchor_index], remaining);
        if (!anchor)
        {
            break;
        }

        uint8_t* candidate = anchor - anchor_index;
        if (candidate > soft_end)
        {
            break;
        }

        int found_match = 1;
        for (size_t i = 0; i < count; i++)
        {
            if (pattern[i] != WILDCARD_BYTE && candidate[i] != pattern[i])
            {
                found_match = 0;
                break;
            }
        }

        if (found_match)
        {
            return candidate;
        }
        p = candidate + 1;
    }
    return NULL;
}

void show_error_message_box(const char* message)
{
    MessageBoxA(NULL, message, "Error", MB_OK | MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND);
}
