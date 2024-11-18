/* date = November 12th 2024 2:40 pm */

#ifndef GAME_H
#define GAME_H

struct VertexField
{
    Int width, height;
    // array of rows
    Float *vertices;
    UInt *indices;
};

struct GameState
{
    Bool initialized;
    
    //GLFWwindow *window;
    
    List<ShaderProgram> shader_programs;
    
    VertexField field;
    UInt field_vbo;
    UInt field_vao;
    UInt *field_ebos;
    
    UInt axis_vbo;
    UInt axis_vao;
};

#endif //GAME_H
