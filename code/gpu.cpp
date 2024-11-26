Void
gpu_compile_shader_from_path(Shader *shader)
{
    shader->file_last_write_time = get_file_last_write_time(shader->path);
    FileContents file_read_result = read_file_contents(shader->path);
    
    check(file_read_result.allocated);
    check(file_read_result.contains_proper_data);
    
    if(shader->type == ShaderType::fragment)
        shader->id = glCreateShader(GL_FRAGMENT_SHADER);
    else if(shader->type == ShaderType::vertex)
        shader->id = glCreateShader(GL_VERTEX_SHADER);
    else
        shader->id = glCreateShader(GL_GEOMETRY_SHADER);
    
    glShaderSource(shader->id, 1, &file_read_result.data, (I32 *)(&file_read_result.size));
    glCompileShader(shader->id);
    
    free(file_read_result.data);
    
    Int compilation_succeeded;
    glGetShaderiv(shader->id, GL_COMPILE_STATUS, &compilation_succeeded);
    
    if(compilation_succeeded != GL_TRUE)
    {
        Char *info_log = (Char *)alloc(512);
        glGetShaderInfoLog(shader->id, 512, NULL, info_log);
        log_warning("shader compilation error: %s", info_log);
        free(info_log);
        
        ASSERT(false);
        
        if(shader->type == ShaderType::fragment)
            shader->id = fragment_shader_fallback_id;
        else if(shader->type == ShaderType::vertex)
            shader->id = vertex_shader_fallback_id;
        
        shader->using_fallback = true;
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
    
    check(result.loaded);
    
    return result;
}

Void
gpu_delete_shader(UInt program_id, Shader *shader)
{
    check(shader);
    
    glDetachShader(program_id, shader->id);
    glDeleteShader(shader->id);
    shader->loaded = false;
}

ShaderProgram
gpu_create_shader_program(const Char *vs_path, const Char *fs_path, Bool is_3d)
{
    check(fs_path);
    check(vs_path);
    
    ShaderProgram result = {0};
    result.is_3d = is_3d;
    
    result.vertex_shader = gpu_create_shader(vs_path, ShaderType::vertex);
    result.fragment_shader = gpu_create_shader(fs_path, ShaderType::fragment);
    
    check(result.vertex_shader.loaded);
    check(result.fragment_shader.loaded);
    
    result.id = glCreateProgram();
    glAttachShader(result.id, result.vertex_shader.id);
    glAttachShader(result.id, result.fragment_shader.id);
    glLinkProgram(result.id);
    
    Int linking_succeeded;
    glGetProgramiv(result.id, GL_LINK_STATUS, &linking_succeeded);
    if(!linking_succeeded) {
        Char *info_log = (Char *)alloc(512);
        glGetProgramInfoLog(result.id, 512, NULL, info_log);
        log_warning("shader program linking error: %s", info_log);
        free(info_log);
        
        ASSERT(false);
        
        return result;
    }
    
    result.linked = true;
    
    if(!result.vertex_shader.using_fallback)
        gpu_delete_shader(result.id, &result.vertex_shader);
    if(!result.fragment_shader.using_fallback)
        gpu_delete_shader(result.id, &result.fragment_shader);
    
    return result;
}

Void
gpu_delete_shader_program(ShaderProgram *program)
{
    glDeleteProgram(program->id);
    program->linked = false;
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

#if 0
Void
gpu_upload_vertices_static(Float v[])
{
}
#endif


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
