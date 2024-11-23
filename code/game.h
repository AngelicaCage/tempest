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
    Void interpolate_to(Color target, Float speed)
    {
        r = interpolate(r, target.r, speed);
        g = interpolate(g, target.g, speed);
        b = interpolate(b, target.b, speed);
        a = interpolate(a, target.a, speed);
    }
    
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
    FieldPoint **target_points;
    
    V2 center_world;
    V2 dim_world;
    V2 playing_area_dim;
    
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

struct SmallFieldBitmap
{
    Bool data[5][5];
};


struct Player
{
    V2 pos;
    V2 vel;
    Float max_speed;
    Color color;
};

struct Bullet
{
    V2 pos;
    V2 vel;
    Float radius;
    Color color;
};
Bullet bullet(V2 pos, V2 vel, Float radius, Color color)
{
    Bullet result;
    result.pos = pos;
    result.vel = vel;
    result.radius = radius;
    result.color = color;
    return result;
}

struct Enemy
{
    V2 pos;
    Float radius;
    Color color;
    
    Float time_between_fires; // in seconds
    Float time_to_fire;
};

struct GameState
{
    Bool initialized;
    
    Bool paused;
    
    F64 target_frame_time_ms;
    F32 d_time; // ms
    
    Input input;
    
    List<ShaderProgram> shader_programs;
    
    Camera target_camera;
    Camera camera;
    
    Field field;
    
    UInt axis_vbo;
    UInt axis_vao;
    
    Player player;
    
    List<Enemy> enemies;
    List<Bullet> enemy_bullets;
    
    SmallFieldBitmap text_bitmaps[26];
};

#endif //GAME_H
