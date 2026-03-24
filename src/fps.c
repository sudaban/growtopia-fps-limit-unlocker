#include "fps.h"
#include <windows.h>

set_fps_limit_fn_t g_original_set_fps_limit = NULL;

void set_optimal_fps_limit(void)
{
    DEVMODE display_mode = {.dmSize = sizeof(DEVMODE)};

    if (EnumDisplaySettingsA(NULL, ENUM_CURRENT_SETTINGS, &display_mode))
    {
        g_original_set_fps_limit(NULL, (float)display_mode.dmDisplayFrequency);
        return;
    }

    g_original_set_fps_limit(NULL, 60.0f);
}

void __fastcall set_fps_limit(void* self, float fps)
{
    (void)self;
    (void)fps;
    set_optimal_fps_limit();
}
