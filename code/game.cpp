
#include "game_loader.h"

#include "game.h"


extern "C" __declspec(dllexport) void __cdecl update_and_render(GameMemory *game_memory)
{
    GameState *game_state = (GameState *)game_memory->memory;
    
    if(!game_state->initialized)
    {
        // TODO
        println("Initializing game memory");
        game_state->initialized = true;
    }
}