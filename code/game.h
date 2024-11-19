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

struct Camera
{
    V3 pos, target, up;
    Bool orbiting;
    Float orbit_distance;
    V2 orbit_angles; // rotate x, then y
};

struct Color
{
    union
    {
        struct
        {
            Float r, g, b, a;
        };
        struct
        {
            Float data[4];
        };
    };
};

struct FieldPoint
{
    Float height;
    Color color;
};

struct Field
{
    FieldPoint **points;
};

struct GameState
{
    Bool initialized;
    
    F64 target_frame_time_ms;
    F32 d_time; // ms
    
    V2 mouse_pos;
    V2 d_mouse_pos;
    
    List<ShaderProgram> shader_programs;
    
    Camera target_camera;
    Camera camera;
    
    
    VertexField field;
    UInt field_vbo;
    UInt field_vao;
    UInt *field_ebos;
    
    UInt axis_vbo;
    UInt axis_vao;
    
};

#endif //GAME_H
