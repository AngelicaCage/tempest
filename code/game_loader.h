/* date = November 11th 2024 0:16 pm */

#ifndef WINDOWS_GAME_LOADER_H
#define WINDOWS_GAME_LOADER_H

#include "ciel/base.h"

struct GameMemory
{
    Bool game_running;
    
    Void *memory;
    U64 size;
    Bool allocated;
};

struct GameCode
{
    Bool loaded;
    
    Void (*update_and_render)(GameMemory*);
};

#endif //WINDOWS_GAME_LOADER_H
