/* date = November 12th 2024 2:40 pm */

#ifndef GAME_H
#define GAME_H

struct Camera
{
    V3 target_pos, target_target;
    V3 pos, target, up;
    
    Bool orbiting;
    Float orbit_distance;
    V2 orbit_angles; // rotate x, then y
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
    
    SplitRenderMesh mesh;
};

struct SmallFieldBitmap
{
    Bool data[5][5];
};


enum class PowerupType
{
    none,
    pierce,
    caliber,
    machinegun,
    extra_life,
};
struct Powerup
{
    V2 pos;
    PowerupType type;
    F64 time_left;
};

struct Player
{
    V2 pos;
    V2 vel;
    Float max_speed;
    
    Float shot_cooldown_max;
    Float shot_cooldown;
    
    Int lives;
    
    PowerupType powerup;
    F64 powerup_time_left;
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
    none,
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
    //Color color;
    
    Float time_between_fires; // in seconds
    Float time_to_fire;
    Float bullet_speed;
    
    EnemyType type;
    union
    {
        struct
        { // spread
            Int amount_per_spread;
        };
        struct
        { // stream
        };
        struct
        { // spin
            Float spin_speed;
            Int spin_arm_count;
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
            Float suicide_move_speed;
        };
    };
};

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

struct SaveData
{
    B32 has_seen_tutorial;
    
    B32 has_finished_a_game;
    F64 highest_time;
    I32 highest_kills;
};

struct GameState
{
    Bool initialized;
    
    Bool paused;
    Bool should_quit;
    
    // all timing is in seconds
    InplaceCircularArray<FrameProfile, 128> frame_profiles;
    Float fps;
    Float target_fps;
    Float d_time;
    
    Bool show_debug_interface;
    
    Bool fullscreen;
    RectI windowed_rect;
    V2 window_dim;
    
    Input input;
    
    union
    {
        struct
        {
            ShaderProgram shape_sp;
            ShaderProgram field_sp;
            ShaderProgram line_sp;
            ShaderProgram debug_font_sp;
        };
        struct
        {
            ShaderProgram shader_programs[4];
        };
    };
    
    UInt font_texture;
    UInt font_vbo;
    UInt font_vao;
    UInt font_ebo;
    
    Mesh axis_mesh;
    
    SmallFieldBitmap text_bitmaps[36];
    
    Camera camera;
    
    Bool in_game;
    F64 time_in_game;
    
    Int main_menu_selector;
    Int pause_menu_selector;
    
    Field field;
    List<Bullet> player_bullets;
    List<Enemy> enemies;
    List<Bullet> enemy_bullets;
    List<EnemyExplosion> enemy_explosions;
    
    Player player;
    Int kills;
    Bool life_lost_explosion_enabled;
    Float life_lost_explosion_radius;
    V2 life_lost_explosion_center;
    
    F64 last_spawn_time;
    
    Bool in_tutorial;
    Int tutorial_phase;
    
    Bool player_dead;
    
    SaveData save;
    
    Mesh rect_mesh;
    Mesh circle_mesh;
};

#endif //GAME_H
