#ifndef FPS_H
#define FPS_H

typedef void(__fastcall* set_fps_limit_fn_t)(void*, float);
extern set_fps_limit_fn_t g_original_set_fps_limit;

void set_optimal_fps_limit(void);
void __fastcall set_fps_limit(void* self, float fps);


#endif // FPS_H