#if 0
Void
gpu_upload_vertices_static(Float v[])
{
}
#endif

Shader
gpu_create_shader(const Char *path, ShaderType type)
{
    Shader result = {0};
    
    result.type = type;
    result.path = path;
    
    result.file_last_write_time = get_file_last_write_time(path);
    FileContents file_read_result = read_file_contents(path);
    
    ASSERT(file_read_result.allocated);
    ASSERT(file_read_result.contains_proper_data);
    
    if(type == ShaderType::fragment)
        result.id = glCreateShader(GL_FRAGMENT_SHADER);
    else if(type == ShaderType::vertex)
        result.id = glCreateShader(GL_VERTEX_SHADER);
    else
        result.id = glCreateShader(GL_GEOMETRY_SHADER);
    
    glShaderSource(result.id, 1, &file_read_result.data, (I32 *)(&file_read_result.size));
    glCompileShader(result.id);
    
    Int compilation_succeeded;
    glGetShaderiv(result.id, GL_COMPILE_STATUS, &compilation_succeeded);
    
    if(compilation_succeeded != GL_TRUE)
    {
        Char *info_log = (Char *)alloc(512);
        glGetShaderInfoLog(result.id, 512, NULL, info_log);
        print_error("shader compilation error: %s", info_log);
        free(info_log);
        ASSERT(false);
        
        return result;
    }
    
    result.loaded = true;
    
    return result;
}