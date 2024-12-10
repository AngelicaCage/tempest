#include "glad/glad.c"
#include "glfw/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
//#define STB_PERLIN_IMPLEMENTATION
//#include "stb/stb_perlin.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

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
Void (*sleep)(F64);

#include "list.h"
#include "math/math.h"
#include "math/vectors.h"
#include "math/rects.h"
#include "shaders.h"
#include "color.h"
#include "timing.h"

#include "input.h"
#include "gpu.h"
#include "game.h"

#include "math/vectors.cpp"
#include "bitmaps.cpp"
#include "drawing.cpp"
#include "window.cpp"
#include "gpu.cpp"
#include "shaders.cpp"
#include "ui_drawing.cpp"
#include "camera.cpp"

#include "timing.cpp"
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
    GameState *game_state = (GameState *)game_memory->memory;
    global_log = game_memory->global_log;
    Camera *camera = &game_state->camera;
    Player *player = &game_state->player;
    Field *field = &game_state->field;
    Input *input = &game_state->input;
    Keys *keys = &input->keys;
    
    if(!game_memory->functions_loaded)
    {
        get_file_last_write_time = game_memory->get_file_last_write_time;
        read_file_contents = game_memory->read_file_contents;
        write_file_contents = game_memory->write_file_contents;
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
        
        game_state->d_time = 1;
        
        fill_key_data(input);
        
        glfwSetScrollCallback(game_memory->window, scroll_callback);
        
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
        
        game_state->fullscreen = false;
        
        
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
                Char *data_pointer = save_file_contents.data;
                
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
    }
    
    player->shot_cooldown_max = 0.15f;
    
    game_state->target_fps = 144;
    
    update_frame_timing_beginning(game_state);
    
    game_state->d_time = game_memory->d_time;
    Float d_time = game_state->d_time;
    
    
    update_input(input, game_memory->window, d_time);
    input->d_scroll = d_scroll;
    d_scroll = 0;
    
    if(!game_state->fullscreen)
    {
        glfwGetWindowPos(game_memory->window,
                         &game_state->windowed_rect.x,
                         &game_state->windowed_rect.y);
        
        glfwGetWindowSize(game_memory->window,
                          &game_state->windowed_rect.w,
                          &game_state->windowed_rect.h);
    }
    
    if(!game_state->fullscreen)
        game_state->windowed_rect = window_get_rect(game_memory->window);
    
    if(keys->f11.just_pressed)
        window_toggle_fullscreen(game_memory->window, game_state->windowed_rect, &game_state->fullscreen);
    
    F64 new_mouse_pos[2];
    glfwGetCursorPos(game_memory->window, &new_mouse_pos[0], &new_mouse_pos[1]);
    input->d_mouse_pos = v2((Float)new_mouse_pos[0] - input->mouse_pos.x,
                            (Float)new_mouse_pos[1] - input->mouse_pos.y);
    input->mouse_pos = v2(new_mouse_pos[0], new_mouse_pos[1]);
    
    RectI window_rect = window_get_rect(game_memory->window);
    game_state->window_dim = v2(window_rect.w, window_rect.h);
    
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
    
    // TODO: optimize
    // make debug time display, with times of chosen parts, total frame time, and max frame time
    // make total frame time red if it exceeds max frame time
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
    
#ifndef TEMPEST_RELEASE
    reload_changed_shaders(game_state);
#endif
    
    gpu_update_camera_in_shaders(game_state);
    
    glShadeModel(GL_SMOOTH);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    
    draw_axes(game_state);
    
    draw_field(game_state);
    
    Float text_scale = 2.0f;
    {
        glDisable(GL_DEPTH_TEST);
        // Later: something's up with the depth buffer
        String fps_string = create_string("%d fps", (Int)game_state->fps);
        
        Rect bg_rect = rect(0, 0, fps_string.length*7*text_scale, 8*text_scale);
        //ui_draw_rect(game_state, bg_rect, color(1, 1, 1, 0.3));
        
        ui_draw_debug_text(game_state, v2(0, 0), text_scale, fps_string.data, fps_string.length, Color::white(), color(0, 0, 0, 0.2f));
        free_string(&fps_string);
        
        ui_draw_rect(game_state, 1920/2, 1080/2, 100, 100, Color::white());
    }
    
    
    ui_draw_log(game_state, global_log, text_scale);
    
    update_frame_timing_end(game_state);
    
    {
        Char profile_text_buffer[100];
        V2 draw_pos = v2(0, 40);
        Float vertical_spacing = 16;
        Int buffer_length = 0;
        
        FrameProfile *profile = &(game_state->frame_profiles.last());
        
        for(Int i = 0; i < profile->finished_sections.length; i++)
        {
            Float average = 0;
            for(Int a = 0; a < game_state->frame_profiles.length; a++)
            {
                average += game_state->frame_profiles[a].finished_sections[i].elapsed_time;
            }
            average /= game_state->frame_profiles.length;
            
            SectionProfile *section = &(profile->finished_sections[i]);
            
            ui_draw_timing_pair(game_state, draw_pos, text_scale, section->name, average, Color::white());
            draw_pos.y += vertical_spacing;
        }
        
        {
            Float average = 0;
            for(Int a = 0; a < game_state->frame_profiles.length; a++)
            {
                average += game_state->frame_profiles[a].elapsed_time;
            }
            average /= game_state->frame_profiles.length;
            
            Color ft_draw_color = (profile->elapsed_time < profile->target_max_time) ? Color::white() : color(1, 0.5, 0.5, 1);
            ui_draw_timing_pair(game_state, draw_pos, text_scale, "Frame Time", average, ft_draw_color);
            draw_pos.y += vertical_spacing;
        }
        
        
        {
            Float profile_width = 3;
            Float horizontal_spacing = 1;
            Float max_height = 60;
            Float extra_height = 60;
            
            Float top = draw_pos.y + 30;
            Float bottom = top + max_height + extra_height;
            
            Int max_len = sizeof(game_state->frame_profiles.data) / sizeof(FrameProfile);
            ui_draw_rect(game_state, 0, top, max_len*(profile_width+horizontal_spacing), max_height+extra_height, color(0, 0, 0, 0.4f));
            
            Float max_frame_time = 0;
            for(Int i = 0; i < game_state->frame_profiles.length; i++)
            {
                if(game_state->frame_profiles[i].elapsed_time > max_frame_time)
                    max_frame_time = game_state->frame_profiles[i].elapsed_time;
            }
            
            for(Int i = 0; i < game_state->frame_profiles.length; i++)
            {
                FrameProfile *profile = &(game_state->frame_profiles.data[i]);
                Float fraction = profile->elapsed_time / profile->target_max_time;
                
                Float profile_height = fraction * max_height;
                Float profile_top = bottom - profile_height;
                Float shade = 0.8f;
                ui_draw_rect(game_state, i*(profile_width+horizontal_spacing), profile_top, profile_width, profile_height, color(shade, shade, shade, 1.0f));
                
                Color section_colors[4] = {
                    Color::green(),
                    Color::orange(),
                    Color::blue(),
                    Color::yellow(),
                };
                Float last_section_end = 0;
                for(Int a = 0; a < profile->finished_sections.length; a++)
                {
                    fraction = profile->finished_sections[a].elapsed_time / profile->elapsed_time;
                    Float section_height = profile_height * fraction;
                    ui_draw_rect(game_state, i*(profile_width+horizontal_spacing), bottom - last_section_end - section_height,
                                 profile_width, section_height, section_colors[a]);
                    last_section_end += section_height;
                }
            }
            ui_draw_rect(game_state, game_state->frame_profiles.start*(profile_width+horizontal_spacing) - 1, top, 2, max_height + extra_height, Color::red());
            ui_draw_rect(game_state, 0, bottom - max_height, game_state->frame_profiles.length*(profile_width+horizontal_spacing), 2, Color::blue());
        }
        
        ui_draw_timing_pair(game_state, draw_pos, text_scale, "Max Frame Time", profile->target_max_time, Color::white());
    }
    
}