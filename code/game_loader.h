/* date = November 11th 2024 0:16 pm */

#ifndef WINDOWS_GAME_LOADER_H
#define WINDOWS_GAME_LOADER_H

#include "glfw/glfw3.h"

#include "ciel/base.h"
#include "log.h"

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
    
    Void *memory;
    U64 size;
    Bool allocated;
    
    Log *global_log;
    GLFWwindow *window;
    
    U64 (*get_file_last_write_time)(const Char *);
    FileContents (*read_file_contents)(const Char *);
};

struct GameCode
{
    Bool loaded;
    
    Void (*update_and_render)(GameMemory *);
};


#endif //WINDOWS_GAME_LOADER_H
