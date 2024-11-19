//#include <windows.h>

#include "glad/glad.c"
#include "glfw/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "ciel/base.h"
#include "ciel/list.h"
#include "log.h"
#include "diagnostics.h"
#include "game_loader.h"

#include "math.h"
#include "gpu.h"
#include "game.h"

// Later: move to gpu.h
UInt vertex_shader_fallback_id = 0;
UInt fragment_shader_fallback_id = 0;

U64 (*get_file_last_write_time)(const Char *);
FileContents (*read_file_contents)(const Char *);
F64 (*get_time_ms)();
Void (*sleep)(F64);

#include "gpu.cpp"

#define GAME_DATA_DIRECTORY "../data"

#define KEYDOWN(key) (glfwGetKey(game_memory->window, (key)) == GLFW_PRESS)

// Later: get a better solution (sending input from platform layer)
Float d_scroll;
Bool just_scrolled;

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    just_scrolled = true;
    d_scroll = (Float)yoffset;
    //ASSERT(false);
}

void
compile_fallback_shaders()
{
    const Char *vs_source = "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "void main()\n"
        "{\n"
        "gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "}";
    Int vs_source_length = strlen(vs_source);
    
    const Char *fs_source = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "FragColor = vec4(1.0f, 0.0f, 1.0f, 1.0f);\n"
        "}";
    Int fs_source_length = strlen(vs_source);
    
    vertex_shader_fallback_id = glCreateShader(GL_VERTEX_SHADER);
    fragment_shader_fallback_id = glCreateShader(GL_FRAGMENT_SHADER);
    
    
    glShaderSource(vertex_shader_fallback_id, 1, &vs_source, &vs_source_length);
    glCompileShader(vertex_shader_fallback_id);
    
    Int compilation_succeeded;
    glGetShaderiv(vertex_shader_fallback_id, GL_COMPILE_STATUS, &compilation_succeeded);
    ASSERT(compilation_succeeded == GL_TRUE);
    
    
    glShaderSource(fragment_shader_fallback_id, 1, &fs_source, &fs_source_length);
    glCompileShader(fragment_shader_fallback_id);
    
    glGetShaderiv(fragment_shader_fallback_id, GL_COMPILE_STATUS, &compilation_succeeded);
    ASSERT(compilation_succeeded == GL_TRUE);
    
}

#define TERMINATE_GAME_LOOP() {\
game_memory->game_running = false;\
return;\
}

VertexField
create_vertex_field(Int width, Int height, Float left_x, Float left_z, Float coordinate_width)
{
    VertexField result;
    result.width = width;
    result.height = height;
    
    result.vertices = (Float *)alloc(sizeof(Float) * width*3 * height);
    for(Int y = 0; y < height; y++)
    {
        for(Int x = 0; x < width; x++)
        {
            Int stride = 3;
            result.vertices[y*width*stride + x*stride + 0] = ((Float)x) / ((Float)width) * coordinate_width + left_x;
            result.vertices[y*width*stride + x*stride + 1] = 0 + random_float(-0.1, 0.1);
            result.vertices[y*width*stride + x*stride + 2] = ((Float)y) / ((Float)height) *
                coordinate_width * ((Float)height / (Float)width) - left_z;
        }
    }
    
    result.indices = (UInt *)alloc(sizeof(UInt) * (width-1)*6 * (height-1));
    for(Int y = 0; y < height - 1; y++)
    {
        for(Int x = 0; x < width - 1; x++)
        {
            Int stride = 6;
            
            result.indices[y*stride*(width-1) + x*stride + 0] = x + y*width;
            result.indices[y*stride*(width-1) + x*stride + 1] = x + (y+1)*width;
            result.indices[y*stride*(width-1) + x*stride + 2] = x + 1 + (y+1)*width;
            
            result.indices[y*stride*(width-1) + x*stride + 3] = x + y*width;
            result.indices[y*stride*(width-1) + x*stride + 4] = x + 1 + (y+1)*width;
            result.indices[y*stride*(width-1) + x*stride + 5] = x + 1 + y*width;
        }
    }
    
    return result;
}

Void
reload_changed_shaders(GameState *game_state)
{
    for(Int i = 0; i < game_state->shader_programs.length; i++)
    {
        if(i > 0)
            break;
        ShaderProgram *program = &(game_state->shader_programs.data[i]);
        
        U64 vs_last_write_time = get_file_last_write_time(program->vertex_shader.path);
        U64 fs_last_write_time = get_file_last_write_time(program->fragment_shader.path);
        
        if(vs_last_write_time != program->vertex_shader.file_last_write_time ||
           fs_last_write_time != program->fragment_shader.file_last_write_time)
        {
            gpu_delete_shader_program(program);
            *program = gpu_create_shader_program(program->vertex_shader.path, program->fragment_shader.path, program->is_3d);
        }
    }
}

extern "C" __declspec(dllexport) void __cdecl
update_and_render(GameMemory *game_memory)
{
    GameState *game_state = (GameState *)game_memory->memory;
    global_log = game_memory->global_log;
    Camera *camera = &game_state->target_camera;
    
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
        game_state->field = create_vertex_field(400, 200, -30.0f, 18.0f, 60);
        game_state->field_ebos = (UInt *)alloc(sizeof(UInt) * (game_state->field.height-1));
        
        glGenVertexArrays(1, &game_state->field_vao);
        glGenBuffers(1, &game_state->field_vbo);
        for(Int i = 0; i < game_state->field.height-1; i++)
        {
            glGenBuffers(1, &(game_state->field_ebos[i]));
        }
        
        glBindVertexArray(game_state->field_vao);
        
        glBindBuffer(GL_ARRAY_BUFFER, game_state->field_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Float) * game_state->field.width*3 * game_state->field.height, game_state->field.vertices, GL_DYNAMIC_DRAW);
        
        for(Int i = 0; i < game_state->field.height-1; i++)
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, game_state->field_ebos[i]);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(UInt) * (game_state->field.width-1)*6,
                         &(game_state->field.indices[i*(game_state->field.width-1)*6]), GL_STATIC_DRAW);
        }
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Float)*3, (Void *)0);
        glEnableVertexAttribArray(0);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    
    F32 d_time = game_state->d_time;
    
    
    game_state->field = create_vertex_field(400, 200, -30.0f, 18.0f, 60);
    glBindBuffer(GL_ARRAY_BUFFER, game_state->field_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Float) * game_state->field.width*3 * game_state->field.height, game_state->field.vertices, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    
    
    F64 new_mouse_pos[2];
    glfwGetCursorPos(game_memory->window, &new_mouse_pos[0], &new_mouse_pos[1]);
    game_state->d_mouse_pos = v2((Float)new_mouse_pos[0] - game_state->mouse_pos.x,
                                 (Float)new_mouse_pos[1] - game_state->mouse_pos.y);
    game_state->mouse_pos = v2(new_mouse_pos[0], new_mouse_pos[1]);
    
    
    if(camera->orbiting)
    {
        Float camera_orbit_speed = 0.002f;
        if(KEYDOWN(GLFW_KEY_RIGHT))
            camera->orbit_angles.x -= camera_orbit_speed * d_time;
        if(KEYDOWN(GLFW_KEY_LEFT))
            camera->orbit_angles.x += camera_orbit_speed * d_time;
        if(KEYDOWN(GLFW_KEY_UP))
            camera->orbit_angles.y += camera_orbit_speed * d_time;
        if(KEYDOWN(GLFW_KEY_DOWN))
            camera->orbit_angles.y -= camera_orbit_speed * d_time;
        
        // LATER: adjust by window resolution
        if(glfwGetMouseButton(game_memory->window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            Float camera_mouse_pan_orbit_speed = 0.002f;
            camera->orbit_angles.y += game_state->d_mouse_pos.y * camera_mouse_pan_orbit_speed * d_time;
            camera->orbit_angles.x += game_state->d_mouse_pos.x * camera_mouse_pan_orbit_speed * d_time;
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
    
    
    
    
    reload_changed_shaders(game_state);
    
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    
    
    for(Int i = 0; i < game_state->shader_programs.length; i++)
    {
        ShaderProgram *shader_program = &(game_state->shader_programs.data[i]);
        if(!shader_program->is_3d)
            continue;
        
        glUseProgram(shader_program->id);
        
        glm::mat4 model = glm::mat4(1.0f);
        Int model_loc = glGetUniformLocation(shader_program->id, "model");
        glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));
        
        glm::mat4 view = glm::lookAt(game_state->camera.pos.to_glm(),
                                     game_state->camera.target.to_glm(),
                                     game_state->camera.up.to_glm());
        Int view_loc = glGetUniformLocation(shader_program->id, "view");
        glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view));
        
        
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), 1920.0f/1080.0f, 0.1f, 1000.0f);
        Int proj_loc = glGetUniformLocation(shader_program->id, "projection");
        glUniformMatrix4fv(proj_loc, 1, GL_FALSE, glm::value_ptr(proj));
    }
    
    // Draw axes
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    Int positive_line_width = 3;
    Int negative_line_width = 1;
    
    Float line_color[4] = {1.0f, 0.0f, 0.0f, 1.0f};
    Int color_loc = glGetUniformLocation(game_state->shader_programs[1].id, "lineColor");
    glm::mat4 model = glm::mat4(1.0f);
    Int model_loc = glGetUniformLocation(game_state->shader_programs[1].id, "model");
    glUseProgram(game_state->shader_programs[1].id);
    glBindVertexArray(game_state->axis_vao);
    
    { // x axis
        glUniform4fv(color_loc, 1, line_color);
        glLineWidth(positive_line_width);
        glDrawArrays(GL_LINES, 0, 2);
        
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-100, 0, 0));
        glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));
        glLineWidth(negative_line_width);
        glDrawArrays(GL_LINES, 0, 2);
    }
    
    { // z axis
        line_color[0] = 0.0f;
        line_color[2] = 1.0f;
        glUniform4fv(color_loc, 1, line_color);
        model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));
        glLineWidth(positive_line_width);
        glDrawArrays(GL_LINES, 0, 2);
        
        model = glm::translate(model, glm::vec3(-100, 0, 0));
        glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));
        glLineWidth(negative_line_width);
        glDrawArrays(GL_LINES, 0, 2);
    }
    
    { // y axis
        line_color[2] = 0.0f;
        line_color[1] = 1.0f;
        glUniform4fv(color_loc, 1, line_color);
        model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));
        glLineWidth(positive_line_width);
        glDrawArrays(GL_LINES, 0, 2);
        
        model = glm::translate(model, glm::vec3(-100, 0, 0));
        glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));
        glLineWidth(negative_line_width);
        glDrawArrays(GL_LINES, 0, 2);
    }
    
    
    // Draw field
    glLineWidth(positive_line_width);
    glUseProgram(game_state->shader_programs[0].id);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    model_loc = glGetUniformLocation(game_state->shader_programs[0].id, "model");
    glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));
    
    glBindVertexArray(game_state->field_vao);
    for(Int i = 0; i < game_state->field.height-1; i++)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, game_state->field_ebos[i]);
        glDrawElements(GL_TRIANGLES, (game_state->field.width-1)*6, GL_UNSIGNED_INT, (Void *)0);
    }
    
    
    game_state->target_frame_time_ms = ((F64)1000) / ((F64)144);
    F64 frame_end_ms = get_time_ms();
    F64 frame_time_ms = frame_end_ms - frame_start_ms;
    F64 time_to_sleep = game_state->target_frame_time_ms - frame_time_ms;
    game_state->d_time = frame_time_ms + time_to_sleep;
    sleep(time_to_sleep);
}