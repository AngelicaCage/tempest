#if 0
Void
gpu_upload_vertices_static(Float v[])
{
}
#endif

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
        
        return;
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
gpu_reload_shader(Shader *shader)
{
    // TODO: re-create any programs that use this
    check(shader);
    check(shader->loaded);
    
    shader->loaded = false;
    glDeleteShader(shader->id);
    
    gpu_compile_shader_from_path(shader);
    
    check(shader->loaded);
}

ShaderProgram
gpu_create_shader_program(Shader *fs, Shader *vs)
{
    check(fs);
    check(vs);
    check(fs->loaded);
    check(vs->loaded);
    
    ShaderProgram result = {0};
    result.fragment_shader = fs;
    result.vertex_shader = vs;
    
    result.id = glCreateProgram();
    glAttachShader(result.id, fs->id);
    glAttachShader(result.id, vs->id);
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
    
    return result;
}