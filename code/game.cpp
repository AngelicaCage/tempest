#include <windows.h>

#include "glad/glad.c"
#include "glfw/glfw3.h"

#include "game_loader.h"

#include "game.h"

void
clean_up_resources(GameState *game_state)
{
    glfwTerminate();
}

#define TERMINATE_GAME_LOOP() {\
game_memory->game_running = false;\
clean_up_resources(game_state);\
return;\
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}  

extern "C" __declspec(dllexport) void __cdecl
update_and_render(GameMemory *game_memory)
{
    GameState *game_state = (GameState *)game_memory->memory;
    if(!game_state->initialized)
    {
        println("Initializing game memory");
        
        // Create GLFW window
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        
        GLFWwindow* window = glfwCreateWindow(800, 600, "Game", NULL, NULL);
        if (window == NULL)
        {
            print_error("failed to create GLFW window");
            glfwTerminate();
            TERMINATE_GAME_LOOP();
        }
        glfwMakeContextCurrent(window);
        
        
        if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            print_error("failed to initialize GLAD");
            TERMINATE_GAME_LOOP();
        }
        
        glViewport(0, 0, 800, 600);
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);  
        
        game_state->window = window;
        
        game_state->initialized = true;
    }
    GLFWwindow *window = game_state->window;
    
    if(glfwWindowShouldClose(window))
        TERMINATE_GAME_LOOP();
    
    glfwSwapBuffers(window);
    glfwPollEvents();
}