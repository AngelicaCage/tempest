#include <windows.h>
#include <stdio.h>

#include "glad/glad.c"
#include "glfw/glfw3.h"

#include "ciel/base.h"
#include "log.h"
#include "diagnostics.h"
#include "game_loader.h"
#include "gpu.h"

#define GAME_DLL_PATH "game.dll"
#define GAME_DLL_COPY_PATH "game_temp_copy.dll"


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
    CopyFile(GAME_DLL_PATH, GAME_DLL_COPY_PATH, false);
    
    *module_handle = LoadLibrary(TEXT(GAME_DLL_COPY_PATH));
    
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

Void framebuffer_size_callback(GLFWwindow* window, Int width, Int height)
{
    glViewport(0, 0, width, height);
}

Void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

UInt vertex_shader_fallback_id = 0;
UInt fragment_shader_fallback_id = 0;

void
compile_fallback_shaders()
{
    const Char *vs_source = "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "void main()\n"
        "{\n"
        "gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "}";
    Int vs_source_length = strlen(vs_source);
    
    const Char *fs_source = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "FragColor = vec4(1.0f, 0.0f, 1.0f, 1.0f);\n"
        "}";
    Int fs_source_length = strlen(vs_source);
    
    vertex_shader_fallback_id = glCreateShader(GL_VERTEX_SHADER);
    fragment_shader_fallback_id = glCreateShader(GL_FRAGMENT_SHADER);
    
    
    glShaderSource(vertex_shader_fallback_id, 1, &vs_source, &vs_source_length);
    glCompileShader(vertex_shader_fallback_id);
    
    Int compilation_succeeded;
    glGetShaderiv(vertex_shader_fallback_id, GL_COMPILE_STATUS, &compilation_succeeded);
    ASSERT(compilation_succeeded == GL_TRUE);
    
    
    glShaderSource(fragment_shader_fallback_id, 1, &fs_source, &fs_source_length);
    glCompileShader(fragment_shader_fallback_id);
    
    glGetShaderiv(fragment_shader_fallback_id, GL_COMPILE_STATUS, &compilation_succeeded);
    ASSERT(compilation_succeeded == GL_TRUE);
    
}

#include "gpu.cpp"


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
    game_memory.size = megabytes(1);
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
    
    GLFWwindow* window = glfwCreateWindow(800, 600, "Game", NULL, NULL);
    if (window == NULL)
    {
        print_error("failed to create GLFW window");
        glfwTerminate();
        ASSERT(false);
    }
    glfwMakeContextCurrent(window);
    
    
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        print_error("failed to initialize GLAD");
        ASSERT(false);
    }
    
    glViewport(0, 0, 800, 600);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    compile_fallback_shaders();
    
    ShaderProgram shader_programs[10];
    
    shader_programs[0] = gpu_create_shader_program("data/shaders/basic_vertex_shader_1.vs",
                                                   "data/shaders/basic_fragment_shader_1.fs");
    shader_programs[1] = gpu_create_shader_program("data/shaders/basic_vertex_shader_1.vs",
                                                   "data/shaders/basic_fragment_shader_2.fs");
    
    float vertices1[] = {
        -1.0f, -0.5f, 0.0f,
        -0.5f, 0.5f, 0.0f,
        0.0f, -0.5f, 0.0f,
    };
    
    float vertices2[] = {
        1.0f, -0.5f, 0.0f,
        0.5f, 0.5f, 0.0f,
        0.0f, -0.5f, 0.0f,
    };
    
    
    UInt EBO;
    glGenBuffers(1, &EBO);
    UInt VBO1, VBO2;
    glGenBuffers(1, &VBO1);
    glGenBuffers(1, &VBO2);
    UInt VAO1;
    UInt VAO2;
    glGenVertexArrays(1, &VAO1);
    glGenVertexArrays(1, &VAO2);
    
    
    glBindVertexArray(VAO1);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO1);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices1), vertices1, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
    glEnableVertexAttribArray(0);
    
    
    glBindVertexArray(VAO2);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
    glEnableVertexAttribArray(0);
    
    
    
    while(game_memory.game_running)
    {
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
        }
        
        for(int i = 0; i < 2; i++)
        {
            ShaderProgram *program = &(shader_programs[i]);
            
            U64 vs_last_write_time = get_file_last_write_time(program->vertex_shader.path);
            U64 fs_last_write_time = get_file_last_write_time(program->fragment_shader.path);
            
            if(vs_last_write_time != program->vertex_shader.file_last_write_time ||
               fs_last_write_time != program->fragment_shader.file_last_write_time)
            {
                gpu_delete_shader_program(program);
                *program = gpu_create_shader_program(program->vertex_shader.path, program->fragment_shader.path);
            }
        }
        
        processInput(window);
        if(glfwWindowShouldClose(window))
            break;
        
        (game_code.update_and_render)(&game_memory);
        
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        
        glUseProgram(shader_programs[0].id);
        glBindVertexArray(VAO1);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        
        glUseProgram(shader_programs[1].id);
        glBindVertexArray(VAO2);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwTerminate();
    return 0;
}