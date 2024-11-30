#include <windows.h>
#include <stdio.h>
#include <math.h>

#include "glad/glad.c"
#define GLFW_DLL
#include "glfw/glfw3.h"

#include "ciel/base.h"
#include "log.h"
#include "diagnostics.h"
#include "game_loader.h"
#include "gpu.h"

#ifndef TEMPEST_RELEASE
#define GAME_DLL_PATH "game.dll"
#define GAME_DLL_COPY_PATH "game_temp_copy.dll"
#else
#define GAME_DLL_PATH "tempest.dll"
#endif



void
log_windows_error(DWORD error_code)
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
    log(message_buffer);
}

F64
get_time()
{
    LARGE_INTEGER ticks;
    LARGE_INTEGER freq;
    if (!QueryPerformanceCounter(&ticks))
    {
        log_windows_error(GetLastError());
        ASSERT(false);
    }
    if (!QueryPerformanceFrequency(&freq))
    {
        log_windows_error(GetLastError());
        ASSERT(false);
    }
    //return (F64)(ticks.QuadPart) / (F64)(freq.QuadPart * 1000);
    return ((F64)(ticks.QuadPart)) / ((F64)(freq.QuadPart));
}

Void
sleep(F64 seconds)
{
    //ASSERT(timeBeginPeriod(1) == TIMERR_NOERROR);
    //ASSERT(timeEndPeriod(1) == TIMERR_NOERROR);
    
    //Sleep((DWORD)(seconds * 1000.0));
    //Sleep((DWORD)16);
    // NOTE: EVIL sleep
    // Later: make this not devour cpu cycles
    
#if 1
    F64 start_time = get_time();
    F64 current_time;
    F64 time_diff;
    do
    {
        current_time = get_time();
        time_diff = current_time - start_time;
    }
    while(time_diff < seconds);
#endif
}

U64 get_file_last_write_time(const Char *path)
{
    WIN32_FILE_ATTRIBUTE_DATA Data;
    if(GetFileAttributesEx(path, GetFileExInfoStandard, &Data))
    {
        U64 result = Data.ftLastWriteTime.dwLowDateTime;
        result = result | ((U64)(Data.ftLastWriteTime.dwHighDateTime) << 32);
        return result;
    }
    
    return 0;
}

Int
get_game_dll_last_write_time(LPFILETIME time)
{
    WIN32_FILE_ATTRIBUTE_DATA Data;
    if(GetFileAttributesEx(GAME_DLL_PATH, GetFileExInfoStandard, &Data))
    {
        *time = Data.ftLastWriteTime;
    }
    else
    {
        return 1;
    }
    
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
#ifndef TEMPEST_RELEASE
    CopyFile(GAME_DLL_PATH, GAME_DLL_COPY_PATH, false);
    
    *module_handle = LoadLibrary(TEXT(GAME_DLL_COPY_PATH));
#else
    *module_handle = LoadLibrary(TEXT(GAME_DLL_PATH));
#endif
    
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
        
        log_windows_error(GetLastError());
        
        FreeLibrary(*module_handle);
        return 1;
    }
    
    return 0;
}

FileContents
read_file_contents(const Char *path)
{
    FileContents result = {0};
    
    HANDLE file_handle = CreateFileA(path,
                                     GENERIC_READ,
                                     FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                                     NULL,
                                     OPEN_EXISTING,
                                     FILE_ATTRIBUTE_NORMAL,
                                     NULL);
    
    if(file_handle == INVALID_HANDLE_VALUE)
    {
        print_error("couldn't open file");
        return result;
    }
    
    LARGE_INTEGER file_size;
    Bool get_file_size_result = GetFileSizeEx(file_handle,
                                              &file_size);
    if(!get_file_size_result)
    {
        print_error("couldn't get file size");
        return result;
    }
    
    result.size = file_size.QuadPart;
    result.data = (Char *)alloc(result.size);
    result.allocated = true;
    
    DWORD bytes_read;
    Bool read_file_result = ReadFile(file_handle,
                                     (Void *)result.data,
                                     result.size,
                                     &bytes_read,
                                     NULL);
    
    if(!read_file_result)
    {
        print_error("couldn't read file");
        return result;
    }
    
    result.contains_proper_data = true;
    return result;
};


Void error_callback( Int error, const Char *msg ) {
    print_error("%d: %s", error, msg);
    ASSERT(false);
}

Float width_over_height = 1000.0f/1000.0f;

Void framebuffer_size_callback(GLFWwindow* window, Int width, Int height)
{
    glViewport(0, 0, width, height * width_over_height);
}


Int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    Log _global_log;
    global_log = &_global_log;
    
    GameCode game_code = {0};
    HINSTANCE module_handle = 0;
    
    FILETIME dll_last_write_time;
    if(get_game_dll_last_write_time(&dll_last_write_time))
    {
        print_error("couldn't open DLL file for time reading");
        ASSERT(false);
        return 1;
    }
    
    Int dll_load_result = load_game_dll(&game_code, &module_handle);
    if(dll_load_result == 1)
    {
        print_error("issue with dll loading");
        ASSERT(false);
        return 1;
    }
    GameMemory game_memory = {0};
    game_memory.game_running = true;
    game_memory.functions_loaded = false;
    game_memory.size = megabytes(10);
    game_memory.memory = alloc(game_memory.size);
    zero_memory(game_memory.memory, game_memory.size);
    game_memory.global_log = global_log;
    game_memory.allocated = true;
    
    
    // Set up window and GLFW context
    
    glfwSetErrorCallback( error_callback );
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* window = glfwCreateWindow(1920, 1080, "Game", NULL, NULL);
    if (window == NULL)
    {
        print_error("failed to create GLFW window");
        glfwTerminate();
        ASSERT(false);
    }
    glfwMakeContextCurrent(window);
    
    game_memory.window = window;
    game_memory.get_file_last_write_time = get_file_last_write_time;
    game_memory.read_file_contents = read_file_contents;
    game_memory.get_time = get_time;
    game_memory.sleep = sleep;
    
    
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        print_error("failed to initialize GLAD");
        ASSERT(false);
    }
    
    framebuffer_size_callback(window, 1920, 1080);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    // Set schedular granularity to 1ms
    
    while(game_memory.game_running)
    {
#ifndef TEMPEST_RELEASE
        FILETIME old_dll_last_write_time = dll_last_write_time;
        get_game_dll_last_write_time(&dll_last_write_time);
        if(CompareFileTime(&dll_last_write_time, &old_dll_last_write_time) != 0)
        {
            unload_game_dll(&game_code, &module_handle);
            
            // TODO: use a better system, like a lock file
            Sleep(500);
            
            Int dll_load_result = load_game_dll(&game_code, &module_handle);
            if(dll_load_result == 1)
            {
                print_error("issue with dll loading");
                return 1;
            }
            
            game_memory.functions_loaded = false;
        }
#endif
        if(glfwWindowShouldClose(window))
            break;
        
        (game_code.update_and_render)(&game_memory);
        
        glfwSwapInterval(1);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwTerminate();
    return 0;
}
