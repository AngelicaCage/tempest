#include <windows.h>

#include "ciel/base.h"
#include "ciel/list.h"

#include "log.h"
#include "game_loader.h"

#include "gpu.h"
#include "game.h"

#define TERMINATE_GAME_LOOP() {\
game_memory->game_running = false;\
return;\
}


extern "C" __declspec(dllexport) void __cdecl
update_and_render(GameMemory *game_memory)
{
    GameState *game_state = (GameState *)game_memory->memory;
    global_log = game_memory->global_log;
    
    if(!game_state->initialized)
    {
        // Initialize memory
        game_state->initialized = true;
    }
}