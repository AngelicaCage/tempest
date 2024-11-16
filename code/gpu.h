/* date = November 13th 2024 10:03 am */

#ifndef GPU_H
#define GPU_H

enum class
ShaderType
{
    vertex,
    geometry,
    fragment,
};

struct
Shader
{
    Bool loaded;
    U64 file_last_write_time;
    const Char *path;
    
    ShaderType type;
    UInt id;
};

struct
ShaderProgram
{
    Bool linked;
    Shader vertex_shader;
    Shader fragment_shader;
    UInt id;
};

typedef UInt VBuffer;
typedef UInt VAttributes;


#endif //GPU_H
