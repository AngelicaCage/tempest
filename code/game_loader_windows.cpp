#include <windows.h>
#include <stdio.h>

#include "ciel/base.h"
#include "game_loader.h"

#define GAME_DLL_PATH "game.dll"

void
print_windows_error(DWORD error_code)
{
    LPSTR message_buffer;
    FormatMessage(
                  FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                  FORMAT_MESSAGE_FROM_SYSTEM |
                  FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL,
                  error_code,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR) &message_buffer,
                  0, NULL );
    printf(message_buffer);
}

Int
get_game_dll_last_write_time(LPFILETIME time)
{
    HANDLE file_handle = CreateFileA(TEXT(GAME_DLL_PATH),
                                     GENERIC_READ,
                                     FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                                     NULL,
                                     OPEN_EXISTING,
                                     FILE_ATTRIBUTE_NORMAL,
                                     NULL);
    
    if(file_handle == INVALID_HANDLE_VALUE)
        return 1;
    
    if(!GetFileTime(file_handle,
                    NULL,
                    NULL,
                    time))
        return 1;
    
    return 0;
}


Int
unload_game_dll(GameCode *game_code, HINSTANCE *module_handle)
{
    int result = 0;
    
    result = FreeLibrary(*module_handle);
    if(result)
    {
        game_code->loaded = false;
    }
    
    return result;
}

Int
load_game_dll(GameCode *game_code, HINSTANCE *module_handle)
{
    *module_handle = LoadLibrary(TEXT(GAME_DLL_PATH));
    
    if(module_handle == NULL)
    {
        print_error("dll wasn't loaded properly");
        return 1;
    }
    
    game_code->update_and_render = (void (*) (GameMemory*)) GetProcAddress(*module_handle, "update_and_render");
    
    if(game_code->update_and_render != NULL)
    {
        game_code->loaded = true;
    }
    else
    {
        print_error("functions didn't load correctly");
        
        print_windows_error(GetLastError());
        
        FreeLibrary(*module_handle);
        return 1;
    }
    
    return 0;
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    Bool running = true;
    
    GameCode game_code = {0};
    HINSTANCE module_handle = 0;
    
    FILETIME dll_last_write_time;
    if(get_game_dll_last_write_time(&dll_last_write_time))
    {
        print_error("couldn't open DLL file for time reading");
        return 1;
    }
    
    Int dll_load_result = load_game_dll(&game_code, &module_handle);
    if(dll_load_result == 1)
    {
        print_error("issue with dll loading");
        return 1;
    }
    GameMemory game_memory = {0};
    game_memory.game_running = true;
    game_memory.size = megabytes(1);
    game_memory.memory = alloc(game_memory.size);
    zero_memory(game_memory.memory, game_memory.size);
    game_memory.allocated = true;
    
    while(game_memory.game_running)
    {
        (game_code.update_and_render)(&game_memory);
    }
    
    return 0;
}