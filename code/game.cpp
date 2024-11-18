//#include <windows.h>

#include "glad/glad.c"
#include "glfw/glfw3.h"

#include "ciel/base.h"
#include "ciel/list.h"
#include "log.h"
#include "diagnostics.h"
#include "game_loader.h"

#include "gpu.h"
#include "game.h"

// TODO: move to gpu.h
UInt vertex_shader_fallback_id = 0;
UInt fragment_shader_fallback_id = 0;

U64 (*get_file_last_write_time)(const Char *);
FileContents (*read_file_contents)(const Char *);

#include "gpu.cpp"

#define GAME_DATA_DIRECTORY "../data"

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
create_vertex_field(Int width, Int height)
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
            result.vertices[y*width*stride + x*stride + 0] = ((Float)x) / ((Float)width) - 0.5f;
            result.vertices[y*width*stride + x*stride + 1] = ((Float)y) / ((Float)height) - 0.5f;
            result.vertices[y*width*stride + x*stride + 2] = 0;
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


extern "C" __declspec(dllexport) void __cdecl
update_and_render(GameMemory *game_memory)
{
    GameState *game_state = (GameState *)game_memory->memory;
    global_log = game_memory->global_log;
    
    
    
    if(!game_state->initialized)
    {
        // Initialize memory
        get_file_last_write_time = game_memory->get_file_last_write_time;
        read_file_contents = game_memory->read_file_contents;
        
        game_state->initialized = true;
        game_state->shader_programs = create_list<ShaderProgram>();
        
#if 1
        glfwMakeContextCurrent(game_memory->window);
        if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            log_error("failed to initialize GLAD");
            ASSERT(false);
        }
#endif
        
        compile_fallback_shaders();
        
        game_state->shader_programs.add(gpu_create_shader_program(GAME_DATA_DIRECTORY "/shaders/basic_vertex_shader_1.vs",
                                                                  GAME_DATA_DIRECTORY "/shaders/basic_fragment_shader_2.fs"));
        
        
        game_state->field = create_vertex_field(6, 5);
        game_state->field_ebos = (UInt *)alloc(sizeof(UInt) * (game_state->field.height-1));
        
        glGenVertexArrays(1, &game_state->field_vao);
        glGenBuffers(1, &game_state->field_vbo);
        for(Int i = 0; i < game_state->field.height-1; i++)
        {
            glGenBuffers(1, &(game_state->field_ebos[i]));
        }
        
        glBindVertexArray(game_state->field_vao);
        
        glBindBuffer(GL_ARRAY_BUFFER, game_state->field_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Float) * game_state->field.width*3 * game_state->field.height, game_state->field.vertices, GL_STATIC_DRAW);
        
        for(Int i = 0; i < game_state->field.height-1; i++)
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, game_state->field_ebos[i]);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(UInt) * (game_state->field.width-1)*6,
                         &(game_state->field.indices[i*(game_state->field.width-1)*6]), GL_STATIC_DRAW);
        }
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Float)*3, (Void *)0);
        glEnableVertexAttribArray(0);
        
        glBindVertexArray(0); 
        glBindBuffer(GL_ARRAY_BUFFER, 0); 
        
    }
    
    for(int i = 0; i < game_state->shader_programs.length; i++)
    {
        ShaderProgram *program = &(game_state->shader_programs.data[i]);
        
        U64 vs_last_write_time = get_file_last_write_time(program->vertex_shader.path);
        U64 fs_last_write_time = get_file_last_write_time(program->fragment_shader.path);
        
        if(vs_last_write_time != program->vertex_shader.file_last_write_time ||
           fs_last_write_time != program->fragment_shader.file_last_write_time)
        {
            gpu_delete_shader_program(program);
            *program = gpu_create_shader_program(program->vertex_shader.path, program->fragment_shader.path);
        }
    }
    
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glUseProgram(game_state->shader_programs[0].id);
    glBindVertexArray(game_state->field_vao);
    for(Int i = 0; i < game_state->field.height-1; i++)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, game_state->field_ebos[i]);
        glDrawElements(GL_TRIANGLES, (game_state->field.width-1)*6, GL_UNSIGNED_INT, (Void *)0);
    }
    
    
    
}