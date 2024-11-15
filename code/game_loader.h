/* date = November 11th 2024 0:16 pm */

#ifndef WINDOWS_GAME_LOADER_H
#define WINDOWS_GAME_LOADER_H

#include "ciel/base.h"

struct
FileContents
{
    Bool allocated;
    Bool contains_proper_data;
    Char *data;
    U64 size;
};

struct GameMemory
{
    Bool game_running;
    Bool opengl_functions_loaded;
    
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
