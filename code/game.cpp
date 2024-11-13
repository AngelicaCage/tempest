#include <windows.h>

#include "glad/glad.c"
#include "glfw/glfw3.h"

#include "ciel/base.h"

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

void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

extern "C" __declspec(dllexport) void __cdecl
update_and_render(GameMemory *game_memory)
{
    GameState *game_state = (GameState *)game_memory->memory;
    if(!game_state->initialized)
    {
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
        
        game_state->shader_program = shaderProgram;
        
        glDeleteShader(vertex_shader);
        glDeleteShader(fragmentShader);
        
        
        
        Float vertices[] = {
            -0.5f, -0.5f, 0.0f,
            0.5f, -0.5f, 0.0f,
            0.0f,  0.5f, 0.0f
        };
        
        UInt VBO;
        glGenBuffers(1, &VBO);
        
        glGenVertexArrays(1, &(game_state->VAO));
        glBindVertexArray(game_state->VAO);
        
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);  
        
        
        
        
        game_state->initialized = true;
    }
    GLFWwindow *window = game_state->window;
    
    processInput(window);
    
    if(glfwWindowShouldClose(window))
        TERMINATE_GAME_LOOP();
    
    
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    
    glUseProgram(game_state->shader_program);
    glBindVertexArray(game_state->VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    
    
    
    glfwSwapBuffers(window);
    glfwPollEvents();
}