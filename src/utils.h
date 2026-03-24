#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <stdint.h>
#include <wtypesbase.h>

#define WILDCARD_BYTE ((uint16_t)-1)
typedef uint16_t optional_byte_t;

HWND get_client_window();
HMODULE get_client_exe_module();

int32_t parse_pattern(const char* pattern, optional_byte_t* out_bytes, size_t max_count);

void* patch_memory(void* dest, const void* src, size_t size);
void* find_memory(const optional_byte_t* pattern, size_t count);
void show_error_message_box(const char* message);

#endif // UTILS_H