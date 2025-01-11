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
    Bool using_fallback;
    U64 file_last_write_time;
    const Char *path;
    
    ShaderType type;
    UInt id;
};

struct
SPUniformData
{
    String name;
    //const Char *name;
    Int location;
    Enum type;
    Int size;
};

struct
ShaderProgram
{
    Bool linked;
    Shader vertex_shader;
    Shader fragment_shader;
    UInt id;
    Bool is_3d;
    
    List<SPUniformData> uniforms;
    
    Int get_uniform_location(const Char *name)
    {
        Int name_length = strlen(name);
        Int result = 0;
        for(Int i = 0; i < uniforms.length; i++)
        {
            Bool is_equal = true;
            for(Int a = 0; a < name_length && a < uniforms[i].name.length; a++)
            {
                if(name[a] != uniforms[i].name[a])
                {
                    is_equal = false;
                    break;
                }
            }
            if(is_equal)
            {
                result = uniforms[i].location;
                break;
            }
        }
        
        return result;
    }
};

struct
Mesh
{
    Bool has_ebo;
    
    UInt vao;
    UInt vbo;
    UInt ebo;
};

struct
SplitRenderMesh
{
    UInt vao;
    UInt vbo;
    UInt *ebos;
};


#endif //GPU_H
