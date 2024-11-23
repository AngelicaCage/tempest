//#include <windows.h>

#include "glad/glad.c"
#include "glfw/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
//#define STB_PERLIN_IMPLEMENTATION
//#include "stb/stb_perlin.h"

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
F64 (*get_time_ms)();
Void (*sleep)(F64);

#include "gpu.cpp"
#include "shaders.cpp"
#include "field.cpp"

#define GAME_DATA_DIRECTORY "../data"

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
update_gameplay(GameState *game_state)
{
    F32 d_time = game_state->d_time;
    Player *player = &game_state->player;
    Keys *keys = &game_state->input.keys;
    V2 playing_area_dim = game_state->field.playing_area_dim;
    
    Float player_speed = 0.01f;
    if(keys->d.is_down)
        player->pos.x += player_speed * d_time;
    if(keys->a.is_down)
        player->pos.x -= player_speed * d_time;
    if(keys->w.is_down)
        player->pos.y -= player_speed * d_time;
    if(keys->s.is_down)
        player->pos.y += player_speed * d_time;
    
    if(keys->e.just_pressed)
    {
        Enemy new_enemy = {
            v2(random_float(-playing_area_dim.x/2, playing_area_dim.x/2),
               random_float(-playing_area_dim.y/2, playing_area_dim.y/2)),
            0.3f,
            color(1, 1, 1, 1),
            10,
            10
        };
        game_state->enemies.add(new_enemy);
    }
    
    for(Int i = 0; i < game_state->enemies.length; i++)
    {
        Enemy *enemy = &(game_state->enemies.data[i]);
        enemy->time_to_fire -= d_time;
        if(enemy->time_to_fire <= 0)
        {
            enemy->time_to_fire = enemy->time_between_fires;
            game_state->enemy_bullets.add(bullet(enemy->pos, v2(random_float(-0.01f, 0.01f), random_float(-0.01f, 0.01f)),
                                                 0.2f, color(1.0f, 0.0f, 1.0f, 1.0f)));
        }
    }
    
    for(Int i = 0; i < game_state->enemy_bullets.length; i++)
    {
        Bullet *bullet = &game_state->enemy_bullets.data[i];
        // Later: make * function for V2 and Float
        bullet->pos += v2(bullet->vel.x * d_time, bullet->vel.y * d_time);
        if(bullet->pos.x < -playing_area_dim.x/2 - bullet->radius ||
           bullet->pos.x > playing_area_dim.x/2 + bullet->radius || 
           bullet->pos.y < -playing_area_dim.y/2 - bullet->radius ||
           bullet->pos.y > playing_area_dim.y/2 + bullet->radius)
        {
            game_state->enemy_bullets.remove_at(i);
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
        get_time_ms = game_memory->get_time_ms;
        sleep = game_memory->sleep;
        
        glfwMakeContextCurrent(game_memory->window);
        if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            log_error("failed to initialize GLAD");
            ASSERT(false);
        }
    }
    
    F64 frame_start_ms = get_time_ms();
    
    if(!game_state->initialized)
    {
        // Initialize memory
        game_state->initialized = true;
        game_state->target_frame_time_ms = ((F64)1000) / ((F64)144);
        game_state->d_time = 1;
        
        game_state->paused = false;
        
        fill_key_data(input);
        
        glfwSetScrollCallback(game_memory->window, scroll_callback);
        
        game_state->shader_programs = create_list<ShaderProgram>();
        
        compile_fallback_shaders();
        
        game_state->shader_programs.add(gpu_create_shader_program(GAME_DATA_DIRECTORY "/shaders/3d_vertex_shader.vs",
                                                                  GAME_DATA_DIRECTORY "/shaders/field_fragment_shader.fs", true));
        game_state->shader_programs.add(gpu_create_shader_program(GAME_DATA_DIRECTORY "/shaders/3d_vertex_shader.vs",
                                                                  GAME_DATA_DIRECTORY "/shaders/line_fragment_shader.fs", true));
        
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
        *field = create_field(200, 150);
        field->render_data_allocated = false;
        field->center_world = v2(0, 0);
        Float field_phys_width = 20;
        field->dim_world = v2(field_phys_width, field_phys_width * ((Float)field->height / (Float)field->width));
        
        player->pos = v2(0, 0);
        player->vel = v2(0, 0);
        player->max_speed = 1;
        player->color = color(1, 1, 1, 1);
        
        game_state->enemy_bullets = create_list<Bullet>();
        game_state->enemies = create_list<Enemy>();
        
        generate_text_bitmaps(game_state);
        update_field_data(game_state, &(game_state->field));
        fill_field_render_data(&(game_state->field));
    }
    
    F32 d_time = game_state->d_time;
    
    update_key_input(input, game_memory->window, d_time);
    F64 new_mouse_pos[2];
    glfwGetCursorPos(game_memory->window, &new_mouse_pos[0], &new_mouse_pos[1]);
    input->d_mouse_pos = v2((Float)new_mouse_pos[0] - input->mouse_pos.x,
                            (Float)new_mouse_pos[1] - input->mouse_pos.y);
    input->mouse_pos = v2(new_mouse_pos[0], new_mouse_pos[1]);
    
    if(keys->q.is_down)
    {
        game_memory->game_running = false;
        return;
    }
    
    if(keys->escape.just_pressed)
        game_state->paused = !game_state->paused;
    
    if(!game_state->paused)
    {
        update_gameplay(game_state);
        update_field_data(game_state, &(game_state->field));
        fill_field_render_data(&(game_state->field));
        
        if(camera->orbiting)
        {
            Float camera_orbit_speed = 0.002f;
            //if(KEYDOWN(GLFW_KEY_RIGHT))
            if(keys->right.is_down)
                camera->orbit_angles.x -= camera_orbit_speed * d_time;
            if(keys->left.is_down)
                camera->orbit_angles.x += camera_orbit_speed * d_time;
            if(keys->up.is_down)
                camera->orbit_angles.y += camera_orbit_speed * d_time;
            if(keys->down.is_down)
                camera->orbit_angles.y -= camera_orbit_speed * d_time;
            
            // LATER: adjust by window resolution
            if(glfwGetMouseButton(game_memory->window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
            {
                Float camera_mouse_pan_orbit_speed = 0.002f;
                camera->orbit_angles.y += input->d_mouse_pos.y * camera_mouse_pan_orbit_speed * d_time;
                camera->orbit_angles.x += input->d_mouse_pos.x * camera_mouse_pan_orbit_speed * d_time;
            }
            
            if(just_scrolled)
            {
                just_scrolled = false;
                camera->orbit_distance -= d_scroll * 0.5 * (camera->orbit_distance);
            }
            
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
            Float interp_speed = 0.015f;
            real_camera->pos.interpolate_to(camera->pos, interp_speed * d_time);
            real_camera->target.interpolate_to(camera->target, interp_speed * d_time);
            real_camera->up.interpolate_to(camera->up, interp_speed * d_time);
            real_camera->orbit_angles.interpolate_to(camera->orbit_angles, interp_speed * d_time);
            real_camera->orbit_distance = interpolate(real_camera->orbit_distance,
                                                      camera->orbit_distance,
                                                      interp_speed * d_time);
        }
    }
    
    
    reload_changed_shaders(game_state);
    
    gpu_update_camera_in_shaders(game_state);
    
    glShadeModel(GL_SMOOTH);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    //draw_axes(game_state);
    
    draw_field(game_state);
    
    
    
    // TODO: change d_time to seconds
    // LATER: improve this
    game_state->target_frame_time_ms = ((F64)1000) / ((F64)144);
    F64 frame_end_ms = get_time_ms();
    F64 frame_time_ms = frame_end_ms - frame_start_ms;
    F64 time_to_sleep = game_state->target_frame_time_ms - frame_time_ms;
    if(time_to_sleep < 0)
        time_to_sleep = 0;
    game_state->d_time = (frame_time_ms + time_to_sleep);
    if(time_to_sleep > 0)
        sleep(time_to_sleep);
}