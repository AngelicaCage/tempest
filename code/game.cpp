//#include <windows.h>

#include "glad/glad.c"
#include "glfw/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
//#define STB_PERLIN_IMPLEMENTATION
//#include "stb/stb_perlin.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

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
    
    Float player_speed = 5.0f;
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
        Float enemy_fire_time = 0.3f;
        Enemy new_enemy = {
            v2(random_float(-playing_area_dim.x/2, playing_area_dim.x/2),
               random_float(-playing_area_dim.y/2, playing_area_dim.y/2)),
            0.3f,
            color(1, 0, 0, 1),
            enemy_fire_time,
            enemy_fire_time
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
            Float bullet_speed = 5.0f;
            V2 bullet_dir = v2(random_float(-1, 1), random_float(-1, 1));
            if(bullet_dir.x == 0 && bullet_dir.y == 0)
                bullet_dir.x = 1;
            bullet_dir.normalize();
            game_state->enemy_bullets.add(bullet(enemy->pos, bullet_dir,
                                                 0.2f, color(1.0f, 0.8f, 0.0f, 1.0f)));
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



GLenum glCheckError_(const char *file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        const Char *error;
        switch (errorCode)
        {
            case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
            default: error = "Unknown error";
        }
        ASSERT(false);
    }
    return errorCode;
}
#define gl_check_error() glCheckError_(__FUNCTION__, __LINE__) 


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
        
        game_state->target_fps = 144;
        game_state->d_time = 1;
        game_state->frame_times = create_list<F64>();
        game_state->last_frame_start_time = get_time();
        
        game_state->paused = false;
        
        fill_key_data(input);
        
        glfwSetScrollCallback(game_memory->window, scroll_callback);
        
        compile_fallback_shaders();
        
        game_state->field_sp = gpu_create_shader_program(GAME_DATA_DIRECTORY "/shaders/3d_vertex_shader.vs",
                                                         GAME_DATA_DIRECTORY "/shaders/field_fragment_shader.fs", true);
        game_state->line_sp = gpu_create_shader_program(GAME_DATA_DIRECTORY "/shaders/3d_vertex_shader.vs",
                                                        GAME_DATA_DIRECTORY "/shaders/line_fragment_shader.fs", true);
        game_state->font_sp = gpu_create_shader_program(GAME_DATA_DIRECTORY "/shaders/font_vertex_shader.vs",
                                                        GAME_DATA_DIRECTORY "/shaders/font_fragment_shader.fs", false);
        
        
        Int width, height, channel_count;
        U8 *font_image_data = stbi_load(GAME_DATA_DIRECTORY "/textures/charmap-oldschool_black.png",
                                        &width, &height, &channel_count, 0);
        ASSERT(font_image_data);
        glGenTextures(1, &game_state->font_texture);
        glBindTexture(GL_TEXTURE_2D, game_state->font_texture);
        gl_check_error();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, font_image_data);
        gl_check_error();
        glGenerateMipmap(GL_TEXTURE_2D);
        gl_check_error();
        stbi_image_free(font_image_data);
        
        float font_vertices[] = {
            // positions        // texture coords
            -0.5f,  0.5f, 0.0f, 0.0f, 1.0f,    // top left 
            -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,   // bottom left
            0.5f, -0.5f, 0.0f,  1.0f, 0.0f,   // bottom right
            -0.5f,  0.5f, 0.0f, 0.0f, 1.0f,   // top left 
            0.5f, -0.5f, 0.0f,  1.0f, 0.0f,   // bottom right
            0.5f,  0.5f, 0.0f,  1.0f, 1.0f,   // top right
        };
        glGenVertexArrays(1, &game_state->font_vao);
        glGenBuffers(1, &game_state->font_vbo);
        
        glBindVertexArray(game_state->font_vao);
        glBindBuffer(GL_ARRAY_BUFFER, game_state->font_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(font_vertices), font_vertices, GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Float)*5, (Void *)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Float)*5, (Void *)(sizeof(Float)*3));
        glEnableVertexAttribArray(1);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        
        
        
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
        player->max_speed = 100;
        player->color = color(1, 1, 1, 1);
        
        game_state->enemy_bullets = create_list<Bullet>();
        game_state->enemies = create_list<Enemy>();
        
        generate_text_bitmaps(game_state);
        update_field_data(game_state, &(game_state->field));
        fill_field_render_data(&(game_state->field));
        
        game_state->d_time = 0.06f;
    }
    
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
        
        if(camera->orbiting)
        {
            Float camera_orbit_speed = 2.0f;
            //if(KEYDOWN(GLFW_KEY_RIGHT))
#if 1
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
    }
    
    update_field_data(game_state, &(game_state->field));
    fill_field_render_data(&(game_state->field));
    
    
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
    
    
    glUseProgram(game_state->font_sp.id);
    
    glBindTexture(GL_TEXTURE_2D, game_state->font_texture);
    glUniform1i(glGetUniformLocation(game_state->font_sp.id, "texture1"), game_state->font_texture);
    
    glBindVertexArray(game_state->font_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    
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