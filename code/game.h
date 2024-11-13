/* date = November 12th 2024 2:40 pm */

#ifndef GAME_H
#define GAME_H

enum class ShaderType
{
    vertex,
    geometry,
    fragment,
};

struct Shader
{
    ShaderType type;
    UInt id;
};

struct GameState
{
    Bool initialized;
    
    GLFWwindow *window;
    
    Shader shaders[10];
    
    UInt shader_program;
    
    UInt VAO;
};

#endif //GAME_H
