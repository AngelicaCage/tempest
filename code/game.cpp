//#include <windows.h>

#include "glad/glad.c"
#include "glfw/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
//#define STB_PERLIN_IMPLEMENTATION
//#include "stb/stb_perlin.h"
//#define STB_IMAGE_IMPLEMENTATION
//#include "stb/stb_image.h"

#include "ciel/base.h"
#include "ciel/list.h"
#include "log.h"
#include "diagnostics.h"
#include "game_loader.h"

#include "math.h"
#include "input.h"
#include "gpu.h"
#include "game.h"

#include "bitmaps.cpp"
#include "drawing.cpp"

// Later: move to shaders.h (or some better-named file)
UInt vertex_shader_fallback_id = 0;
UInt fragment_shader_fallback_id = 0;

U64 (*get_file_last_write_time)(const Char *);
FileContents (*read_file_contents)(const Char *);
F64 (*get_time)();
Void (*sleep)(F64);

#include "gpu.cpp"
#include "shaders.cpp"
#include "field.cpp"

#ifndef TEMPEST_RELEASE
#define GAME_DATA_DIRECTORY "../data"
#else
#define GAME_DATA_DIRECTORY "data"
#endif

#define KEYDOWN(key) (glfwGetKey(game_memory->window, (key)) == GLFW_PRESS)

// Later: integrate these with Input
Float d_scroll;
Bool just_scrolled;

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    just_scrolled = true;
    d_scroll = (Float)yoffset;
    //ASSERT(false);
}

Void
update_main_menu(GameState *game_state)
{
    F32 d_time = game_state->d_time;
    Keys *keys = &game_state->input.keys;
    
    if(keys->down.just_pressed || keys->s.just_pressed)
        game_state->main_menu_selector++;
    if(keys->up.just_pressed || keys->w.just_pressed)
        game_state->main_menu_selector--;
    // Later: write clamp function
    if(game_state->main_menu_selector < 0)
        game_state->main_menu_selector = 1;
    if(game_state->main_menu_selector > 1)
        game_state->main_menu_selector = 0;
    
    if(keys->enter.is_down)
    {
        if(game_state->main_menu_selector == 0)
        {
            game_state->in_game = true;
            game_state->player.pos = v2(0, 0);
            game_state->player.lives = 3;
            game_state->player_bullets.length = 0;
            game_state->enemy_bullets.length = 0;
            game_state->enemies.length = 0;
            game_state->life_lost_explosion_enabled = false;
            game_state->time_in_game = 0;
            game_state->kills = 0;
            game_state->player.powerup = PowerupType::none;
            game_state->last_spawn_time = 0;
        }
        else if(game_state->main_menu_selector == 1)
        {
            game_state->should_quit = true;
        }
        else if(game_state->main_menu_selector == 2)
        {
        }
    }
}

Void
player_subtract_life(GameState *game_state)
{
    game_state->player.lives--;
    // TODO: clear all bullets and enemies on screen
    game_state->life_lost_explosion_enabled = true;
    game_state->life_lost_explosion_radius = 0;
    game_state->life_lost_explosion_center = game_state->player.pos;
    
    if(game_state->player.lives < 0)
    {
        game_state->in_game = false;
    }
}

Void
update_gameplay(GameState *game_state)
{
    F32 d_time = game_state->d_time;
    Player *player = &game_state->player;
    Keys *keys = &game_state->input.keys;
    V2 playing_area_dim = game_state->field.playing_area_dim;
    
    game_state->time_in_game += d_time;
    
    Float player_speed = 5.0f;
    if(keys->shift_left.is_down)
        player_speed = 2.5f;
    
    if(keys->d.is_down)
        player->pos.x += player_speed * d_time;
    if(keys->a.is_down)
        player->pos.x -= player_speed * d_time;
    if(keys->w.is_down)
        player->pos.y -= player_speed * d_time;
    if(keys->s.is_down)
        player->pos.y += player_speed * d_time;
    
    if(player->shot_cooldown <= 0)
    {
        V2 bullet_dir = v2(0, 0);
        Bool should_shoot = false;
        if(keys->up.is_down)
        {
            bullet_dir += v2(0, -1);
            should_shoot = true;
        }
        if(keys->down.is_down)
        {
            bullet_dir += v2(0, 1);
            should_shoot = true;
        }
        if(keys->left.is_down)
        {
            bullet_dir += v2(-1, 0);
            should_shoot = true;
        }
        if(keys->right.is_down)
        {
            bullet_dir += v2(1, 0);
            should_shoot = true;
        }
        bullet_dir.normalize();
        
        if(should_shoot)
        {
            V2 bullet_vel = bullet_dir * 10.0f;
            player->shot_cooldown = player->shot_cooldown_max;
            game_state->player_bullets.add(bullet(player->pos, bullet_vel,
                                                  0.15f, color(0.88f, 0.42f, 0.88f, 1.0f)));
            
        }
    }
    else
    {
        player->shot_cooldown -= d_time;
    }
    
    V2 play_area_top_left = game_state->field.playing_area_dim / -2;
    V2 play_area_bottom_right = game_state->field.playing_area_dim / 2;
    player->pos = clamp(player->pos, play_area_top_left, play_area_bottom_right);
    
    EnemyType new_enemy_type = EnemyType::none;
    if(keys->number_1.just_pressed)
        new_enemy_type = EnemyType::spread;
    if(keys->number_2.just_pressed)
        new_enemy_type = EnemyType::stream;
    if(keys->number_3.just_pressed)
        new_enemy_type = EnemyType::spin;
    if(keys->number_4.just_pressed)
        new_enemy_type = EnemyType::wall;
    if(keys->number_5.just_pressed)
        new_enemy_type = EnemyType::bomb;
    if(keys->number_6.just_pressed)
        new_enemy_type = EnemyType::suicide;
    
    if(new_enemy_type != EnemyType::none)
    {
        Float enemy_fire_time = 0.3f;
        V2 enemy_pos = v2(random_float(-playing_area_dim.x/2, playing_area_dim.x/2),
                          random_float(-playing_area_dim.y/2, playing_area_dim.y/2));
        
        Enemy new_enemy = create_enemy(enemy_pos, new_enemy_type);
        game_state->enemies.add(new_enemy);
    }
    
    if(keys->k.just_pressed)
    {
        for(Int i = 0; i < game_state->enemies.length; i++)
        {
            game_state->enemies.remove_at(i);
            i--;
        }
        for(Int i = 0; i < game_state->enemy_bullets.length; i++)
        {
            game_state->enemy_bullets.remove_at(i);
            i--;
        }
        
    }
    
    if(game_state->time_in_game - game_state->last_spawn_time >= 1)
    {
        game_state->last_spawn_time = game_state->time_in_game;
        /* costs:
stream: 5
suicide: 10
spin: 30
wall: 50
bomb: 60
*/
        // TODO: initial enemy spawn wave (before 5 seconds)
        // TODO: penalty for not killing enemies. Maybe they get stronger the longer they're alive?
        // TODO: enemy spawn queue?
        // TODO: make sure enemies don't spawn right next to or on top of the player
        
        Int max_enemies = 3;
        if(game_state->time_in_game > 30)
            max_enemies = 5;
        if(game_state->time_in_game > 60)
            max_enemies = 7;
        if(game_state->time_in_game > 120)
            max_enemies = 10;
        if(game_state->time_in_game > 180)
            max_enemies = 12;
        
        Int new_enemies_count = max_enemies - game_state->enemies.length;
        if(new_enemies_count > 0)
        {
            Float spawning_preference = sin(game_state->time_in_game * 0.2f);
            
            Float proportions[5] = {
                random_float(0, 0.4),
                random_float(0, 0.3),
                random_float(0, 0.1),
                random_float(0, 0.1),
                random_float(0, 0.1),
            };
            
            EnemyType types[5] = {
                EnemyType::stream,
                EnemyType::suicide,
                EnemyType::spin,
                EnemyType::wall,
                EnemyType::bomb,
            };
            
            Int type_offset = (Int)((spawning_preference+1)/2*4);
            
            Float *enemy_types = (Float *)alloc(sizeof(Float) * new_enemies_count);
            
            for(Int i = 0; i < new_enemies_count; i++)
            {
                Float cost = 1.0f / (Float)new_enemies_count;
                Int largest_index = 0;
                for(Int a = 1; a < 5; a++)
                {
                    if(proportions[a] > proportions[largest_index])
                        largest_index = a;
                }
                
                proportions[largest_index] -= cost;
                
                Float player_safety_radius = 1.5f;
                V2 enemy_pos = v2(random_float(-playing_area_dim.x/2, playing_area_dim.x/2),
                                  random_float(-playing_area_dim.y/2, playing_area_dim.y/2));
                Float dist_to_player = v2_dist(player->pos, enemy_pos);
                if(dist_to_player <= player_safety_radius)
                {
                    V2 travel_dir = enemy_pos - player->pos;
                    travel_dir.normalize();
                    travel_dir = travel_dir * dist_to_player;
                    enemy_pos += travel_dir;
                    if(enemy_pos.x < -playing_area_dim.x/2 || enemy_pos.y < -playing_area_dim.y/2 ||
                       enemy_pos.x > playing_area_dim.x/2 || enemy_pos.y > playing_area_dim.y/2)
                    {
                        enemy_pos = v2(random_float(-0.5, 0.5), random_float(-0.5, 0.5));
                    }
                }
                
                
                Enemy new_enemy = create_enemy(enemy_pos, types[(largest_index + type_offset)%5]);
                game_state->enemies.add(new_enemy);
            }
            
            free(enemy_types);
        }
    }
    
    for(Int i = 0; i < game_state->enemies.length; i++)
    {
        Enemy *enemy = &(game_state->enemies.data[i]);
        
        Bool enemy_should_be_destroyed = false;
        for(Int a = 0; a < game_state->player_bullets.length; a++)
        {
            Bullet *bullet = &game_state->player_bullets.data[a];
            if(v2_dist(enemy->pos, bullet->pos) <= enemy->radius + bullet->radius)
            {
                game_state->player_bullets.remove_at(a);
                enemy_should_be_destroyed = true;
                game_state->kills++;
                break;
            }
        }
        
        if(game_state->life_lost_explosion_enabled)
        {
            Float dist_to_explosion = v2_dist(game_state->life_lost_explosion_center, enemy->pos);
            if(dist_to_explosion <= game_state->life_lost_explosion_radius + enemy->radius)
            {
                enemy_should_be_destroyed = true;
            }
        }
        
        
        if(enemy_should_be_destroyed)
        {
            game_state->enemy_explosions.add(enemy_explosion(&(game_state->enemies.data[i])));
            game_state->enemies.remove_at(i);
            i--;
            continue;
        }
        
        if(enemy->type == EnemyType::suicide)
        {
            V2 d_pos = player->pos - enemy->pos;
            d_pos.normalize();
            d_pos = d_pos * enemy->suicide_move_speed * d_time;
            enemy->pos += d_pos;
        }
        
        if(!game_state->life_lost_explosion_enabled)
        {
            Float dist_to_player = v2_dist(player->pos, enemy->pos);
            if(dist_to_player <= enemy->radius)
            {
                player_subtract_life(game_state);
            }
        }
        
        enemy->time_to_fire -= d_time;
        if(enemy->time_to_fire <= 0)
        {
            enemy->time_to_fire = enemy->time_between_fires;
            Float bullet_size = 0.2f;
            
            switch(enemy->type)
            {
                case EnemyType::spread:
                {
                    for(Int a = 1; a <= enemy->amount_per_spread; a++)
                    {
                        Float angle = ((Float)a / (Float)enemy->amount_per_spread) * pi*2;
                        V2 bullet_dir = v2(cos(angle), sin(angle));
                        game_state->enemy_bullets.add(bullet(enemy->pos + bullet_dir*enemy->radius, bullet_dir * enemy->bullet_speed,
                                                             bullet_size, color(1.0f, 0.8f, 0.0f, 1.0f)));
                    }
                }; break;
                case EnemyType::stream:
                {
                    V2 bullet_dir = player->pos - enemy->pos;
                    bullet_dir.normalize();
                    game_state->enemy_bullets.add(bullet(enemy->pos + bullet_dir*enemy->radius, bullet_dir * enemy->bullet_speed,
                                                         bullet_size, color(1.0f, 0.8f, 0.0f, 1.0f)));
                    
                }; break;
                case EnemyType::spin:
                {
                    Float initial_angle = get_time() * pi*2 * enemy->spin_speed;
                    for(Int a = 0; a < enemy->spin_arm_count; a++)
                    {
                        Float angle = initial_angle + ((Float)a / (Float)enemy->spin_arm_count) * pi*2;
                        V2 bullet_dir = v2(cos(angle), sin(angle));
                        game_state->enemy_bullets.add(bullet(enemy->pos + bullet_dir*enemy->radius, bullet_dir * enemy->bullet_speed,
                                                             bullet_size, color(1.0f, 0.8f, 0.0f, 1.0f)));
                    }
                }; break;
                case EnemyType::wall:
                {
                    V2 bullet_dir = enemy->wall_dir;
                    game_state->enemy_bullets.add(bullet(enemy->pos + bullet_dir*enemy->radius, bullet_dir * enemy->bullet_speed,
                                                         bullet_size, color(1.0f, 0.8f, 0.0f, 1.0f)));
                    bullet_dir = enemy->wall_dir * -1;
                    game_state->enemy_bullets.add(bullet(enemy->pos + bullet_dir*enemy->radius, bullet_dir * enemy->bullet_speed,
                                                         bullet_size, color(1.0f, 0.8f, 0.0f, 1.0f)));
                }; break;
                case EnemyType::bomb:
                {
                    V2 bullet_dir = player->pos - enemy->pos;
                    Float wobble_amount = 0.2f;
                    V2 wobble = v2(random_float(-wobble_amount, wobble_amount), random_float(-wobble_amount, wobble_amount));
                    bullet_dir.normalize();
                    bullet_dir += wobble;
                    bullet_dir.normalize();
                    game_state->enemy_bullets.add(bullet(enemy->pos + bullet_dir*enemy->radius, bullet_dir * enemy->bullet_speed,
                                                         bullet_size * 4.5f, color(1.0f, 0.8f, 0.0f, 1.0f)));
                }; break;
            }
        }
    }
    
    if(game_state->life_lost_explosion_enabled)
    {
        game_state->life_lost_explosion_radius += d_time * 10;
        if(game_state->life_lost_explosion_radius >= 13)
        {
            game_state->life_lost_explosion_enabled = false;
            game_state->life_lost_explosion_radius = 0;
        }
    }
    
    for(Int i = 0; i < game_state->enemy_bullets.length; i++)
    {
        Bullet *bullet = &game_state->enemy_bullets.data[i];
        
        if(!game_state->life_lost_explosion_enabled)
        {
            Float dist_to_player = v2_dist(player->pos, bullet->pos);
            if(dist_to_player <= bullet->radius)
            {
                player_subtract_life(game_state);
            }
        }
        
        if(game_state->life_lost_explosion_enabled)
        {
            Float dist_to_explosion = v2_dist(game_state->life_lost_explosion_center, bullet->pos);
            if(dist_to_explosion <= game_state->life_lost_explosion_radius + bullet->radius)
            {
                game_state->enemy_bullets.remove_at(i);
                i--;
                continue;
            }
        }
        
        // Later: change this to use play_area_top_left etc.
        // Later: make * function for V2 and Float
        bullet->pos += v2(bullet->vel.x * d_time, bullet->vel.y * d_time);
        if(bullet->pos.x < -playing_area_dim.x/2 - bullet->radius ||
           bullet->pos.x > playing_area_dim.x/2 + bullet->radius || 
           bullet->pos.y < -playing_area_dim.y/2 - bullet->radius ||
           bullet->pos.y > playing_area_dim.y/2 + bullet->radius)
        {
            game_state->enemy_bullets.remove_at(i);
            i--;
            continue;
        }
    }
    
    for(Int i = 0; i < game_state->player_bullets.length; i++)
    {
        Bullet *bullet = &game_state->player_bullets.data[i];
        // Later: make * function for V2 and Float
        bullet->pos += v2(bullet->vel.x * d_time, bullet->vel.y * d_time);
        if(bullet->pos.x < -playing_area_dim.x/2 - bullet->radius ||
           bullet->pos.x > playing_area_dim.x/2 + bullet->radius || 
           bullet->pos.y < -playing_area_dim.y/2 - bullet->radius ||
           bullet->pos.y > playing_area_dim.y/2 + bullet->radius)
        {
            game_state->player_bullets.remove_at(i);
            i--;
        }
    }
    
    for(Int i = 0; i < game_state->enemy_explosions.length; i++)
    {
        EnemyExplosion *explosion = &game_state->enemy_explosions.data[i];
        explosion->time_left -= d_time;
        
        if(explosion->time_left <= 0)
        {
            game_state->enemy_explosions.remove_at(i);
            i--;
        }
    }
}





extern "C" __declspec(dllexport) void __cdecl
update_and_render(GameMemory *game_memory)
{
    GameState *game_state = (GameState *)game_memory->memory;
    global_log = game_memory->global_log;
    Camera *camera = &game_state->target_camera;
    Player *player = &game_state->player;
    Field *field = &game_state->field;
    Input *input = &game_state->input;
    Keys *keys = &input->keys;
    
    if(!game_memory->functions_loaded)
    {
        get_file_last_write_time = game_memory->get_file_last_write_time;
        read_file_contents = game_memory->read_file_contents;
        get_time = game_memory->get_time;
        sleep = game_memory->sleep;
        
        glfwMakeContextCurrent(game_memory->window);
        if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            log_error("failed to initialize GLAD");
            ASSERT(false);
        }
    }
    
    if(!game_state->initialized)
    {
        // Initialize memory
        game_state->initialized = true;
        srand(get_time());
        
        game_state->paused = false;
        game_state->should_quit = false;
        
        game_state->target_fps = 144;
        game_state->d_time = 1;
        game_state->frame_times = create_list<F64>();
        game_state->last_frame_start_time = get_time();
        
        fill_key_data(input);
        
        glfwSetScrollCallback(game_memory->window, scroll_callback);
        
        compile_fallback_shaders();
        
        game_state->field_sp = gpu_create_shader_program(GAME_DATA_DIRECTORY "/shaders/3d_vertex_shader.vs",
                                                         GAME_DATA_DIRECTORY "/shaders/field_fragment_shader.fs", true);
        game_state->line_sp = gpu_create_shader_program(GAME_DATA_DIRECTORY "/shaders/3d_vertex_shader.vs",
                                                        GAME_DATA_DIRECTORY "/shaders/line_fragment_shader.fs", true);
#if 0
        game_state->font_sp = gpu_create_shader_program(GAME_DATA_DIRECTORY "/shaders/font_vertex_shader.vs",
                                                        GAME_DATA_DIRECTORY "/shaders/font_fragment_shader.fs", false);
        
        
        Int width, height, channel_count;
        stbi_set_flip_vertically_on_load(true);
        U8 *font_image_data = stbi_load(GAME_DATA_DIRECTORY "/textures/charmap-oldschool_black.png",
                                        //GAME_DATA_DIRECTORY "/textures/test.jpg",
                                        &width, &height, &channel_count, 0);
        ASSERT(font_image_data);
        glGenTextures(1, &game_state->font_texture);
        glBindTexture(GL_TEXTURE_2D, game_state->font_texture);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, font_image_data);
        //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, font_image_data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(font_image_data);
        
        // TODO: send texture coords as a uniform
        Float tex_coord_width = 0.3f;
        float font_vertices[] = {
            // positions        // texture coords
            0.5f,  0.5f, 0.0f,  tex_coord_width, tex_coord_width,   // top right
            0.5f, -0.5f, 0.0f,  tex_coord_width, 0.0f,   // bottom right
            -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,   // bottom left
            -0.5f,  0.5f, 0.0f, 0.0f, tex_coord_width,    // top left 
        };
        unsigned int indices[] = {
            0, 1, 3, // first triangle
            1, 2, 3  // second triangle
        };
        glGenVertexArrays(1, &game_state->font_vao);
        glGenBuffers(1, &game_state->font_vbo);
        glGenBuffers(1, &game_state->font_ebo);
        
        glBindVertexArray(game_state->font_vao);
        
        glBindBuffer(GL_ARRAY_BUFFER, game_state->font_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(font_vertices), font_vertices, GL_STATIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, game_state->font_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Float)*5, (Void *)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Float)*5, (Void *)(sizeof(Float)*3));
        glEnableVertexAttribArray(1);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
#endif
        
        
        camera->pos = v3(1, 1, 3);
        camera->target = v3(0, 0, 0);
        camera->up = v3(0, 1, 0);
        camera->orbiting = true;
        camera->orbit_angles = v2(pi / 2.0f, pi/3.0f);
        camera->orbit_distance = 10.0f;
        
        
        // Axes
        glGenVertexArrays(1, &game_state->axis_vao);
        glGenBuffers(1, &game_state->axis_vbo);
        
        glBindVertexArray(game_state->axis_vao);
        
        Float axis_vertices[] = {
            0, 0, 0,
            100, 0, 0,
        };
        glBindBuffer(GL_ARRAY_BUFFER, game_state->axis_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(axis_vertices), axis_vertices, GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Float)*3, (Void *)0);
        glEnableVertexAttribArray(0);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        
        
        // Field
        //*field = create_field(400, 300);
        *field = create_field(220, 150);
        field->render_data_allocated = false;
        field->center_world = v2(0, 0);
        Float field_phys_width = 22;
        field->dim_world = v2(field_phys_width, field_phys_width * ((Float)field->height / (Float)field->width));
        
        player->pos = v2(0, 0);
        player->vel = v2(0, 0);
        player->max_speed = 100;
        player->shot_cooldown_max = 0.5f;
        player->shot_cooldown = 0;
        player->lives = 3;
        player->powerup = PowerupType::none;
        game_state->kills = 0;
        game_state->player_bullets = create_list<Bullet>();
        game_state->life_lost_explosion_enabled = false;
        game_state->life_lost_explosion_radius = 0;
        
        game_state->enemy_bullets = create_list<Bullet>();
        game_state->enemies = create_list<Enemy>();
        game_state->enemy_explosions = create_list<EnemyExplosion>();
        
        generate_text_bitmaps(game_state);
        update_field_data(game_state, &(game_state->field));
        fill_field_render_data(&(game_state->field));
        
        game_state->main_menu_selector = 0;
        
        game_state->d_time = 0.06f;
        game_state->in_game = false;
        game_state->time_in_game = 0;
        
        game_state->last_spawn_time = game_state->time_in_game;
    }
    player->shot_cooldown_max = 0.15f;
    
    game_state->target_fps = 144;
    
    F64 this_frame_start_time = get_time();
    game_state->d_time = this_frame_start_time - game_state->last_frame_start_time;
    game_state->last_frame_start_time = this_frame_start_time;
    
    List<F64> *frame_times = &game_state->frame_times;
    frame_times->add(this_frame_start_time);
    if(frame_times->length > 144)
    {
        frame_times->remove_at(0);
    }
    F64 time_diff = frame_times->data[frame_times->length - 1] - frame_times->data[0];
    game_state->fps = (Float)frame_times->length / (Float)time_diff;
    
    Float d_time = game_state->d_time;
    
    
    
    
    
    update_key_input(input, game_memory->window, d_time);
    F64 new_mouse_pos[2];
    glfwGetCursorPos(game_memory->window, &new_mouse_pos[0], &new_mouse_pos[1]);
    input->d_mouse_pos = v2((Float)new_mouse_pos[0] - input->mouse_pos.x,
                            (Float)new_mouse_pos[1] - input->mouse_pos.y);
    input->mouse_pos = v2(new_mouse_pos[0], new_mouse_pos[1]);
    
    if(game_state->in_game)
    {
        if(keys->escape.just_pressed)
            game_state->paused = !game_state->paused;
        
        if(!game_state->paused)
        {
            update_gameplay(game_state);
        }
    }
    else
    {
        update_main_menu(game_state);
    }
    
    update_field_data(game_state, &(game_state->field));
    fill_field_render_data(&(game_state->field));
    
    
    if(camera->orbiting)
    {
        Float camera_orbit_speed = 2.0f;
        //if(KEYDOWN(GLFW_KEY_RIGHT))
#if 0
        if(keys->right.is_down)
            camera->orbit_angles.x -= camera_orbit_speed * d_time;
        if(keys->left.is_down)
            camera->orbit_angles.x += camera_orbit_speed * d_time;
        if(keys->up.is_down)
            camera->orbit_angles.y += camera_orbit_speed * d_time;
        if(keys->down.is_down)
            camera->orbit_angles.y -= camera_orbit_speed * d_time;
#endif
        
#if 0
        // LATER: adjust by window resolution
        if(glfwGetMouseButton(game_memory->window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            Float camera_mouse_pan_orbit_speed = 2.0f;
            camera->orbit_angles.y += input->d_mouse_pos.y * camera_mouse_pan_orbit_speed * d_time;
            camera->orbit_angles.x += input->d_mouse_pos.x * camera_mouse_pan_orbit_speed * d_time;
        }
        
        if(just_scrolled)
        {
            just_scrolled = false;
            camera->orbit_distance -= d_scroll * 0.5 * (camera->orbit_distance);
        }
#endif
        
        Float angle_y_min = -1*pi/2.2f;
        if(camera->orbit_angles.y < angle_y_min)
            camera->orbit_angles.y = angle_y_min;
        if(camera->orbit_angles.y > -angle_y_min)
            camera->orbit_angles.y = -angle_y_min;
        
        
        camera->pos.x = camera->orbit_distance * cos(camera->orbit_angles.y) * cos(camera->orbit_angles.x);
        camera->pos.y = camera->orbit_distance * sin(camera->orbit_angles.y);
        camera->pos.z = camera->orbit_distance * cos(camera->orbit_angles.y) * sin(camera->orbit_angles.x);
    }
    
    {
        Camera *real_camera = &game_state->camera;
        Float interp_speed = 50.0f;
        real_camera->pos.interpolate_to(camera->pos, interp_speed * d_time);
        real_camera->target.interpolate_to(camera->target, interp_speed * d_time);
        real_camera->up.interpolate_to(camera->up, interp_speed * d_time);
        real_camera->orbit_angles.interpolate_to(camera->orbit_angles, interp_speed * d_time);
        real_camera->orbit_distance = interpolate(real_camera->orbit_distance,
                                                  camera->orbit_distance,
                                                  interp_speed * d_time);
    }
    
    if(keys->q.is_down)
    {
        game_state->should_quit = true;
    }
    
    if(game_state->should_quit)
    {
        game_memory->game_running = false;
        return;
    }
    
    reload_changed_shaders(game_state);
    
    gpu_update_camera_in_shaders(game_state);
    
    glShadeModel(GL_SMOOTH);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    
    //glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    
    //draw_axes(game_state);
    
    draw_field(game_state);
    
    
#if 0
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glUseProgram(game_state->font_sp.id);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, game_state->font_texture);
    glUniform1i(glGetUniformLocation(game_state->font_sp.id, "texture1"), game_state->font_texture);
    
    glBindVertexArray(game_state->font_vao);
    //glDrawArrays(GL_TRIANGLES, 0, 6);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
#endif
    
    
    // LATER: improve this
    F64 this_frame_end_time = get_time();
    F64 this_frame_time = this_frame_end_time - this_frame_start_time;
    F64 time_to_sleep = (1.0 / game_state->target_fps) - this_frame_time;
    
    if(time_to_sleep < 0)
        time_to_sleep = 0;
    
#if 0
    if(time_to_sleep > 0)
        sleep(time_to_sleep);
#endif
}