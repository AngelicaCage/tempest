#include <windows.h>
#include <stdio.h>

#include "glad/glad.c"
#include "glfw/glfw3.h"

#include "ciel/base.h"
#include "game_loader.h"

#include "gpu.h"
#include "gpu.cpp"

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
    
#if 0
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
#endif
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


void error_callback( Int error, const Char *msg ) {
    print_error("%d: %s", error, msg);
    ASSERT(false);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
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
    
    //game_state->shaders = create_list<Shader>();
    
    //game_state->shaders.add(gpu_create_shader("data/basic_fragment_shader.fs", ShaderType::fragment));
    
    const Char *vertex_shader_source = "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "}\0";
    
    UInt vertex_shader;
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
    glCompileShader(vertex_shader);
    
    int  success;
    char infoLog[512];
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(vertex_shader, 512, NULL, infoLog);
        ASSERT(false);
    }
    
    
    const Char *fragmentShaderSource = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
        "}\n";
    UInt fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        ASSERT(false);
    }
    
    
    const Char *fragmentShaderSource2 = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "FragColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);\n"
        "}\n";
    UInt fragmentShader2;
    fragmentShader2 = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader2, 1, &fragmentShaderSource2, NULL);
    glCompileShader(fragmentShader2);
    
    glGetShaderiv(fragmentShader2, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(fragmentShader2, 512, NULL, infoLog);
        ASSERT(false);
    }
    
    
    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertex_shader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        ASSERT(false);
    }
    
    UInt shader_program1 = shaderProgram;
    
    
    unsigned int shaderProgram2;
    shaderProgram2 = glCreateProgram();
    glAttachShader(shaderProgram2, vertex_shader);
    glAttachShader(shaderProgram2, fragmentShader2);
    glLinkProgram(shaderProgram2);
    glGetProgramiv(shaderProgram2, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(shaderProgram2, 512, NULL, infoLog);
        ASSERT(false);
    }
    
    UInt shader_program2 = shaderProgram2;
    
    
    glDeleteShader(vertex_shader);
    glDeleteShader(fragmentShader);
    
    
    
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
        if(dll_last_write_time.dwLowDateTime != old_dll_last_write_time.dwLowDateTime ||
           dll_last_write_time.dwHighDateTime != old_dll_last_write_time.dwHighDateTime)
        {
            unload_game_dll(&game_code, &module_handle);
            
            Sleep(500);
            
            Int dll_load_result = load_game_dll(&game_code, &module_handle);
            if(dll_load_result == 1)
            {
                print_error("issue with dll loading");
                return 1;
            }
            
            game_memory.opengl_functions_loaded = false;
        }
        
        processInput(window);
        if(glfwWindowShouldClose(window))
            break;
        
        (game_code.update_and_render)(&game_memory);
        
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        
        glUseProgram(shader_program1);
        glBindVertexArray(VAO1);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        
        glUseProgram(shader_program2);
        glBindVertexArray(VAO2);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwTerminate();
    return 0;
}