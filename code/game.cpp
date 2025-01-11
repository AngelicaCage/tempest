#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define DR_MP3_IMPLEMENTATION
#include "dr_libs/dr_mp3.h"


#include "base.h"
#include "log.h"
#include "game_loader.h"

#ifndef TEMPEST_RELEASE
#define GAME_DATA_DIRECTORY "../data"
#else
#define GAME_DATA_DIRECTORY "data"
#endif

U64 (*get_file_last_write_time)(const Char *);
FileContents (*read_file_contents)(const Char *);
Bool (*write_file_contents)(const Char *, U8 *, U64);
F64 (*get_time)();

#include "list.h"
#include "math/math.h"
#include "math/vectors.h"
#include "math/rects.h"
#include "shaders.h"
#include "color.h"
#include "timing.h"
#include "audio.h"

#include "input.h"
#include "gpu.h"
#include "game.h"

#include "opengl_functions_and_enums.h"


#include "math/vectors.cpp"
#include "bitmaps.cpp"
#include "drawing.cpp"
#include "window.cpp"
#include "gpu.cpp"
#include "shaders.cpp"
#include "ui_drawing.cpp"
#include "camera.cpp"

#include "timing.cpp"
#include "audio.cpp"
#include "field.cpp"

#define KEYDOWN(key) (glfwGetKey(game_memory->window, (key)) == GLFW_PRESS)

Void
write_to_save_file(GameState *game_state)
{
    U64 size = sizeof(Bool)*2 + sizeof(F64) + sizeof(Int);
    U8 *save_file_buffer = (U8 *)mem_alloc(size);
    
    // Later: make a generic way to do this
    // SEE OTHER Later when we read
    
    U8 *data_pointer = save_file_buffer;
    
    *(B32 *)data_pointer = game_state->save.has_seen_tutorial;
    data_pointer += sizeof(B32);
    
    *(B32 *)data_pointer = game_state->save.has_finished_a_game;
    data_pointer += sizeof(B32);
    
    *(F64 *)data_pointer = game_state->save.highest_time;
    data_pointer += sizeof(F64);
    
    *(I32 *)data_pointer = game_state->save.highest_kills;
    
    Bool success = write_file_contents("save.tempest_save", save_file_buffer, size);
}

#include "gameplay.cpp"


extern "C" __declspec(dllexport) void __cdecl
update_and_render(GameMemory *game_memory)
{
    if(!game_memory->functions_loaded)
    {
        get_file_last_write_time = game_memory->get_file_last_write_time;
        read_file_contents = game_memory->read_file_contents;
        write_file_contents = game_memory->write_file_contents;
        get_time = game_memory->get_time;
        
        copy_opengl_functions(game_memory);
    }
    
    GameState *game_state = (GameState *)game_memory->memory;
    game_state->input = game_memory->input;
    
    global_log = &game_memory->global_log;
    Camera *camera = &game_state->camera;
    Player *player = &game_state->player;
    Field *field = &game_state->field;
    Input *input = &game_state->input;
    Keys *keys = &input->keys;
    
    if(!game_state->initialized)
    {
        game_memory->d_time = 1.f/144.f;
        // Initialize memory
        game_state->initialized = true;
        srand(get_time());
        
        game_state->paused = false;
        game_state->should_quit = false;
        
        compile_fallback_shaders();
        
        game_state->shape_sp = gpu_create_shader_program(GAME_DATA_DIRECTORY "/shaders/2d_shape.vs",
                                                         GAME_DATA_DIRECTORY "/shaders/2d_shape.fs", false);
        game_state->field_sp = gpu_create_shader_program(GAME_DATA_DIRECTORY "/shaders/3d_vertex_shader.vs",
                                                         GAME_DATA_DIRECTORY "/shaders/field_fragment_shader.fs", true);
        game_state->line_sp = gpu_create_shader_program(GAME_DATA_DIRECTORY "/shaders/3d_vertex_shader.vs",
                                                        GAME_DATA_DIRECTORY "/shaders/line_fragment_shader.fs", true);
        game_state->debug_font_sp = gpu_create_shader_program(GAME_DATA_DIRECTORY "/shaders/font_vertex_shader.vs",
                                                              GAME_DATA_DIRECTORY "/shaders/font_fragment_shader.fs", false);
        load_debug_font(game_state);
        setup_rect_mesh(&game_state->rect_mesh);
        
        
        // Setup Axes
        {
            game_state->axis_mesh.vao = gpu_gen_vao();
            game_state->axis_mesh.vbo = gpu_gen_vbo();
            
            Float axis_vertices[] = {
                0, 0, 0,
                100, 0, 0,
            };
            gpu_upload_vertices(game_state->axis_mesh.vbo,
                                axis_vertices, sizeof(axis_vertices));
            
            gpu_vao_attach_vbo_attribute(game_state->axis_mesh.vao, game_state->axis_mesh.vbo,
                                         0, 3, sizeof(Float)*3, 0);
        }
        
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
        game_state->player_bullets = allocate_list<Bullet>();
        game_state->life_lost_explosion_enabled = false;
        game_state->life_lost_explosion_radius = 0;
        
        game_state->enemy_bullets = allocate_list<Bullet>();
        game_state->enemies = allocate_list<Enemy>();
        game_state->enemy_explosions = allocate_list<EnemyExplosion>();
        
        generate_text_bitmaps(game_state);
        update_field_data(game_state, &(game_state->field));
        fill_field_render_data(&(game_state->field));
        
        game_state->main_menu_selector = 0;
        
        game_state->d_time = 0.06f;
        game_state->in_game = false;
        game_state->time_in_game = 0;
        
        game_state->last_spawn_time = game_state->time_in_game;
        
        FileContents save_file_contents = read_file_contents("save.tempest_save");
        game_state->save = {0};
        if(save_file_contents.file_found &&
           save_file_contents.allocated &&
           save_file_contents.contains_proper_data)
        {
            if(save_file_contents.size >= sizeof(Bool)*2 + sizeof(F64) + sizeof(Int))
            {
                // Later: make a generic way to do this
                // ex: file_contents.read_bool();
                // and the same the other way around, with writing
                // find a good way to write structs, piece by piece so no weird packing issues
                U8 *data_pointer = save_file_contents.data;
                
                game_state->save.has_seen_tutorial = *(B32 *)data_pointer;
                data_pointer += sizeof(B32);
                
                game_state->save.has_finished_a_game = *(B32 *)data_pointer;
                data_pointer += sizeof(B32);
                
                game_state->save.highest_time = *(F64 *)data_pointer;
                data_pointer += sizeof(F64);
                
                game_state->save.highest_kills = *(I32 *)data_pointer;
            }
            else
            {
                ASSERT(false);
            }
        }
        
        if(save_file_contents.allocated)
            free(save_file_contents.data);
        
        
        camera->pos = v3(1, 1, 3);
        camera->target = v3(0, 0, 0);
        camera->up = v3(0, 1, 0);
        camera->orbiting = true;
        camera->orbit_angles = v2(pi / 2.0f, pi/3.0f);
        camera->orbit_distance = 10.0f;
        
        game_state->show_debug_interface = true;
        
        Sound *new_sound_ptr = load_sound_mp3(game_state,
                                              GAME_DATA_DIRECTORY "/audio/test/bunker_1.mp3");
        play_sound(game_state, new_sound_ptr, true);
        
        load_sound_mp3(game_state, GAME_DATA_DIRECTORY "/audio/test/bell.mp3");
    }
    
    game_state->window_info = game_memory->window_info;
    
    game_state->target_fps = 144;
    game_state->fps = (sizeof(game_state->frame_profiles)/sizeof(FrameProfile)) /
    (game_state->frame_profiles.last().start_time - game_state->frame_profiles[0].start_time);
    
    update_frame_timing_beginning(game_state, (Float)game_memory->d_time);
    
    {
        Float average = 0;
        for(Int a = 0; a < game_state->frame_profiles.length; a++)
        {
            average += (Float)(game_state->frame_profiles[a].d_time);
        }
        average /= game_state->frame_profiles.length;
        game_state->d_time = average;
        
        if(game_state->frame_profiles.length < 10)
            game_state->d_time = 1.0f/game_state->target_fps;
    }
    
    Float d_time = game_state->d_time;
    
    player->shot_cooldown_max = 0.15f;
    
    if(keys->f1.just_pressed)
        game_state->show_debug_interface = !game_state->show_debug_interface;
    
    if(game_state->in_game)
    {
        if(keys->escape.just_pressed)
        {
            game_state->paused = !game_state->paused;
            game_state->pause_menu_selector = 0;
        }
        
        if(game_state->paused)
        {
            if(keys->down.just_pressed || keys->s.just_pressed)
                game_state->pause_menu_selector++;
            if(keys->up.just_pressed || keys->w.just_pressed)
                game_state->pause_menu_selector--;
            
            game_state->pause_menu_selector = clamp_circle(game_state->pause_menu_selector, 0, 1);
            
            if(keys->enter.just_pressed)
            {
                if(game_state->pause_menu_selector == 0)
                    game_state->paused = false;
                else if(game_state->pause_menu_selector == 1)
                {
                    exit_to_main_menu(game_state);
                }
            }
        }
        else
        {
            update_gameplay(game_state);
        }
    }
    else
    {
        update_main_menu(game_state);
    }
    
    if(keys->j.just_pressed)
        play_sound(game_state, &game_state->sounds[1], false);
    
    start_timing_section(&game_state->frame_profiles.last(), "update_field_data");
    update_field_data(game_state, &(game_state->field));
    end_timing_section(&game_state->frame_profiles.last());
    
    start_timing_section(&game_state->frame_profiles.last(), "fill_field_render_data");
    fill_field_render_data(&(game_state->field));
    end_timing_section(&game_state->frame_profiles.last());
    
    
    update_camera(game_state);
    
#ifndef TEMPEST_RELEASE
    if(keys->q.is_down)
    {
        game_state->should_quit = true;
    }
#endif
    
    if(game_state->should_quit)
    {
        game_memory->game_running = false;
        return;
    }
    
    write_frame_audio(game_state, &game_memory->audio_buffer);
    
#ifndef TEMPEST_RELEASE
    reload_changed_shaders(game_state);
#endif
    
    gpu_update_camera_in_shaders(game_state);
    
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    //draw_axes(game_state);
    
    draw_field(game_state);
    
    Float text_scale = 2.0f;
    
    glDisable(GL_DEPTH_TEST);
    
    if(keys->j.just_pressed)
        log("omg");
    
    ui_draw_log(game_state, global_log, text_scale);
    
    if(game_state->show_debug_interface)
        ui_draw_debug_interface(game_state, game_memory, text_scale);
    
    update_frame_timing_end(game_state);
    
}