/* date = November 12th 2024 2:40 pm */

#ifndef GAME_H
#define GAME_H

struct GameState
{
    Bool initialized;
    
    //GLFWwindow *window;
    
    List<Shader> shaders;
    
    UInt shader_program1;
    UInt shader_program2;
    
    UInt VAO1, VAO2;
};

#endif //GAME_H
