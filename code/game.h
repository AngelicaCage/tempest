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
    
    Float shot_cooldown_max;
    Float shot_cooldown;
    
    Int bomb_count; // TODO: reflective bomb, can store up to 2 or maybe 1
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

enum class EnemyType
{
    spread,
    stream,
    spin,
    wall,
    bomb, // maybe?
    suicide,
};

struct Enemy
{
    V2 pos;
    Float radius;
    Color color;
    
    Float time_between_fires; // in seconds
    Float time_to_fire;
    
    EnemyType type;
    union
    {
        struct
        { // spread
        };
        struct
        { // stream
        };
        struct
        { // wall
            V2 wall_dir;
        };
        struct
        { // bomb
        };
        struct
        { // suicide
        };
    };
};

// TODO: implement these
struct EnemyExplosion
{
    V2 pos;
    Float initial_radius;
    Float initial_height;
    Color initial_color;
    
    Float time_left_max;
    Float time_left;
};
EnemyExplosion enemy_explosion(Enemy *enemy)
{
    EnemyExplosion result;
    result.pos = enemy->pos;
    result.initial_radius = enemy->radius;
    result.initial_height = 0.3f;
    result.initial_color = color(1, 0, 0, 1);
    result.time_left_max = 0.25f;
    result.time_left = result.time_left_max;
    return result;
}

struct GameState
{
    Bool initialized;
    
    Bool paused;
    Bool should_quit;
    
    // all timing is in seconds
    List<F64> frame_times; // max 144
    Float fps;
    Float target_fps;
    F64 last_frame_start_time;
    F32 d_time;
    
    Input input;
    
    union
    {
        struct
        {
            ShaderProgram field_sp;
            ShaderProgram line_sp;
        };
        struct
        {
            ShaderProgram shader_programs[2];
        };
    };
    
    UInt font_texture;
    UInt font_vbo;
    UInt font_vao;
    UInt font_ebo;
    
    UInt axis_vbo;
    UInt axis_vao;
    
    SmallFieldBitmap text_bitmaps[36];
    
    Camera target_camera;
    Camera camera;
    
    Bool in_game;
    F64 time_in_game;
    
    Int main_menu_selector;
    
    Field field;
    Player player;
    List<Bullet> player_bullets;
    List<Enemy> enemies;
    List<Bullet> enemy_bullets;
    List<EnemyExplosion> enemy_explosions;
};

#endif //GAME_H
