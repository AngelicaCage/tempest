
 Enum glCheckError_(const char *file, int line)
{
    Enum errorCode;
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



Void
gpu_compile_shader_from_path(Shader *shader)
{
    shader->file_last_write_time = get_file_last_write_time(shader->path);
    FileContents file_read_result = read_file_contents(shader->path);
    
    if(!file_read_result.allocated || !file_read_result.contains_proper_data)
        return;
    
    if(shader->type == ShaderType::fragment)
        shader->id = glCreateShader(GL_FRAGMENT_SHADER);
    else if(shader->type == ShaderType::vertex)
        shader->id = glCreateShader(GL_VERTEX_SHADER);
    else
        shader->id = glCreateShader(GL_GEOMETRY_SHADER);
    
    glShaderSource(shader->id, 1, (const Char **)(&file_read_result.data), (I32 *)(&file_read_result.size));
    glCompileShader(shader->id);
    
    free(file_read_result.data);
    
    Int compilation_succeeded;
    glGetShaderiv(shader->id, GL_COMPILE_STATUS, &compilation_succeeded);
    
    if(compilation_succeeded != GL_TRUE)
    {
        Char *info_log = (Char *)mem_alloc(512);
        glGetShaderInfoLog(shader->id, 512, NULL, info_log);
        log_warning("shader compilation error: %s", info_log);
        free(info_log);
    }
    
    shader->loaded = true;
    return;
}

Shader
gpu_create_shader(const Char *path, ShaderType type)
{
    Shader result = {0};
    
    result.type = type;
    result.path = path;
    
    gpu_compile_shader_from_path(&result);
    
    // Later: weird issue with hot reloading
    return result;
}

Void
gpu_delete_shader(UInt program_id, Shader *shader)
{
    glDetachShader(program_id, shader->id);
    glDeleteShader(shader->id);
    shader->loaded = false;
}



Void
gpu_shader_program_set_uniform_name_location_pairs(ShaderProgram *program)
{
    program->uniforms = allocate_list<SPUniformData>();
    
    Int uniform_count;
    glGetProgramiv(program->id, GL_ACTIVE_UNIFORMS, &uniform_count);
    
    for(UInt i = 0; i < uniform_count; i++)
    {
        UInt name_buffer_size = 64;
        Char name_buffer[64];
        
        Int uniform_size;
        Enum uniform_type;
        
        glGetActiveUniform(program->id, i, name_buffer_size, NULL,
                           &uniform_size, &uniform_type, name_buffer);
        Int uniform_location = glGetUniformLocation(program->id, name_buffer);
        
        program->uniforms.add({create_string(name_buffer), uniform_location, uniform_type, uniform_size});
    }
}

ShaderProgram
gpu_create_shader_program(const Char *vs_path, const Char *fs_path, Bool is_3d)
{
    ShaderProgram result = {0};
    result.is_3d = is_3d;
    
    result.vertex_shader = gpu_create_shader(vs_path, ShaderType::vertex);
    result.fragment_shader = gpu_create_shader(fs_path, ShaderType::fragment);
    
    result.id = glCreateProgram();
    glAttachShader(result.id, result.vertex_shader.id);
    glAttachShader(result.id, result.fragment_shader.id);
    glLinkProgram(result.id);
    
    Int linking_succeeded;
    glGetProgramiv(result.id, GL_LINK_STATUS, &linking_succeeded);
    if(!linking_succeeded) {
        Char *info_log = (Char *)mem_alloc(512);
        glGetProgramInfoLog(result.id, 512, NULL, info_log);
        log_warning("shader program linking error: %s", info_log);
        free(info_log);
        
        //ASSERT(false);
        
        return result;
    }
    
    result.linked = true;
    
    gpu_shader_program_set_uniform_name_location_pairs(&result);
    
    if(result.vertex_shader.loaded)
        gpu_delete_shader(result.id, &result.vertex_shader);
    if(result.fragment_shader.loaded)
        gpu_delete_shader(result.id, &result.fragment_shader);
    
    return result;
}

Void
gpu_delete_shader_program(ShaderProgram *program)
{
    glDeleteProgram(program->id);
    free(program->uniforms.data);
    program->linked = false;
}

Void
gpu_use_shader_program(ShaderProgram *program)
{
    glUseProgram(program->id);
}

Void
gpu_update_camera_in_shaders(GameState *game_state)
{
    for(Int i = 0; i < sizeof(game_state->shader_programs) / sizeof(ShaderProgram); i++)
    {
        ShaderProgram *shader_program = &(game_state->shader_programs[i]);
        if(!shader_program->is_3d)
            continue;
        
        glUseProgram(shader_program->id);
        
        glm::mat model = glm::mat4(1.0f);
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
}

Void
gpu_set_uniform_1i(ShaderProgram *program, const Char *name, Int value)
{
    glUniform1i(program->get_uniform_location(name), value);
}

Void
gpu_set_uniform_1f(ShaderProgram *program, const Char *name, Float value)
{
    glUniform1f(program->get_uniform_location(name), value);
}
Void
gpu_set_uniform_2f(ShaderProgram *program, const Char *name, Float *value)
{
    glUniform2fv(program->get_uniform_location(name), 1, value);
}
Void
gpu_set_uniform_4f(ShaderProgram *program, const Char *name, Float *value)
{
    glUniform4fv(program->get_uniform_location(name), 1, value);
}

Void
gpu_set_uniform_mat4x4(ShaderProgram *program, const Char *name, Float *value)
{
    glUniformMatrix4fv(program->get_uniform_location(name), 1, GL_FALSE, value);
}


// GENERATING //
UInt
gpu_gen_vao()
{
    UInt result;
    glGenVertexArrays(1, &result);
    return result;
}
UInt
gpu_gen_vbo()
{
    UInt result;
    glGenBuffers(1, &result);
    return result;
}
UInt
gpu_gen_ebo()
{
    UInt result;
    glGenBuffers(1, &result);
    return result;
}
UInt gpu_gen_texture()
{
    UInt result;
    glGenTextures(1, &result);
    return result;
}

// BINDING //
Void
gpu_bind_vao(UInt id)
{
    glBindVertexArray(id);
}
Void
gpu_bind_vbo(UInt id)
{
    glBindBuffer(GL_ARRAY_BUFFER, id);
}
Void
gpu_bind_ebo(UInt id)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
}
Void
gpu_bind_texture(UInt id)
{
    glBindTexture(GL_TEXTURE_2D, id);
}

Void
gpu_clear_bindings()
{
    gpu_bind_vao(0);
    gpu_bind_vbo(0);
    gpu_bind_ebo(0);
    gpu_bind_texture(0);
}


// DATA UPLOADING //
Void
gpu_upload_vertices(UInt vbo_id, Float *vertices, U64 size)
{
    gpu_bind_vbo(vbo_id);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
    gpu_bind_vbo(0);
}
Void
gpu_upload_vertices_stream(UInt vbo_id, Float *vertices, U64 size)
{
    gpu_bind_vbo(vbo_id);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
    gpu_bind_vbo(0);
}

Void
gpu_upload_indices(UInt ebo_id, UInt *indices, U64 size)
{
    gpu_bind_ebo(ebo_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, GL_STATIC_DRAW);
    gpu_bind_ebo(0);
}

Void
gpu_upload_image(UInt texture_id, U8 *image_data, Int width, Int height)
{
    gpu_bind_texture(texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    // Later: should we do this for images above a certain size
    //glGenerateMipmap(GL_TEXTURE_2D);
    gpu_bind_texture(0);
}


// PARAMETER SETTING //
// Later: only clear bindings for debug build?
Void
gpu_vao_attach_ebo(UInt vao_id, UInt ebo_id)
{
    gpu_bind_vao(vao_id);
    gpu_bind_ebo(ebo_id);
    gpu_bind_vao(0);
    gpu_bind_ebo(0);
}
Void
gpu_vao_attach_vbo(UInt vao_id, UInt vbo_id)
{
    gpu_bind_vao(vao_id);
    gpu_bind_vbo(vbo_id);
    gpu_bind_vao(0);
    gpu_bind_vbo(0);
}

Void
gpu_vao_attach_vbo_attribute(UInt vao_id, UInt vbo_id, UInt index, UInt attribute_element_count, UInt vertex_size, U64 offset)
{
    gpu_bind_vao(vao_id);
    gpu_bind_vbo(vbo_id);
    glVertexAttribPointer(index, attribute_element_count, GL_FLOAT, GL_FALSE, vertex_size, (Void *)offset);
    glEnableVertexAttribArray(index);
    gpu_bind_vao(0);
    gpu_bind_vbo(0);
}

Void
gpu_set_texture_parameter_int(UInt texture_id, Enum parameter, Int value)
{
    gpu_bind_texture(texture_id);
    glTexParameteri(GL_TEXTURE_2D, parameter, value);
    gpu_bind_texture(0);
}
Void
gpu_set_texture_wrapping_repeat(UInt texture_id)
{
    gpu_set_texture_parameter_int(texture_id, GL_TEXTURE_WRAP_S, GL_REPEAT);
    gpu_set_texture_parameter_int(texture_id, GL_TEXTURE_WRAP_T, GL_REPEAT);
}
Void
gpu_set_texture_filter_nearest(UInt texture_id)
{
    gpu_set_texture_parameter_int(texture_id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gpu_set_texture_parameter_int(texture_id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

Void
gpu_set_texture_unit(Int unit_index, UInt texture_id)
{
    glActiveTexture(GL_TEXTURE0 + unit_index);
    gpu_bind_texture(texture_id);
}


// DRAWING //

Void
gpu_draw_indices(UInt vao_id, Enum type, UInt vertex_count)
{
    glBindVertexArray(vao_id);
    glDrawElements(type, vertex_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

Void
gpu_draw_vertices(UInt vao_id, Enum type, UInt vertex_count)
{
    glBindVertexArray(vao_id);
    glDrawArrays(type, 0, vertex_count);
    glBindVertexArray(0);
}