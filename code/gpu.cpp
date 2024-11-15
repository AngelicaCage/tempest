Void
gpu_upload_vertices_static(Float v[])
{
}

#if 1
Shader
gpu_create_shader(const Char *path, ShaderType type)
{
    Shader result;
    
    result.type = type;
    
    result.id = glCreateShader(GL_VERTEX_SHADER);
    //glShaderSource(result.id, 1, &source, NULL);
    // TODO
    glCompileShader(result.id);
    
    return result;
}
#endif