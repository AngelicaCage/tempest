/* date = November 13th 2024 10:03 am */

#ifndef GPU_H
#define GPU_H

enum class ShaderType
{
    vertex,
    geometry,
    fragment,
};

struct Shader
{
    ShaderType type;
    UInt id;
    
    const Char *path;
};

typedef UInt VBufferID;
typedef UInt GPU;


#endif //GPU_H
