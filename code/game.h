/* date = November 12th 2024 2:40 pm */

#ifndef GAME_H
#define GAME_H

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
Color color(Float r, Float g, Float b, Float a)
{
    return {r, g, b, a};
};

struct FieldPoint
{
    Float height;
    Color color;
};

struct Field
{
    union
    {
        struct
        {
            V2I dim;
        };
        struct
        {
            Int width, height;
        };
    };
    
    // array of rows
    FieldPoint **points;
    
    V2 center_world;
    V2 dim_world;
    
    Bool render_data_allocated;
    Float *vertices;
    UInt *indices;
    UInt vbo;
    UInt vao;
    UInt *ebos;
};

struct FieldBitmap
{
    union
    {
        struct
        {
            V2I dim;
        };
        struct
        {
            UInt width, height;
        };
    };
    Bool **data;
};

struct Player
{
    V2 pos;
    V2 vel;
    Float max_speed;
    Color color;
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
    
    Field field;
    
    UInt axis_vbo;
    UInt axis_vao;
    
    Player player;
    
    FieldBitmap test_bitmap;
};

#endif //GAME_H
