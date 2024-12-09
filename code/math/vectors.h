/* date = November 30th 2024 7:31 pm */

#ifndef CIEL_VECTORS_H
#define CIEL_VECTORS_H

// STRUCTS //
struct V2
{
    union
    {
        struct
        {
            F32 x, y;
        };
        struct
        {
            F32 components[2];
        };
    };
    
    F32 mag();
    V2 normalized();
};

struct V2I
{
    union
    {
        struct
        {
            I32 x, y;
        };
        struct
        {
            I32 components[2];
        };
    };
};

struct V3
{
    union
    {
        struct
        {
            F32 x, y, z;
        };
        struct
        {
            F32 components[3];
        };
    };
    
#ifdef GLM_VERSION_MAJOR
    glm::vec3 to_glm();
#endif
    
    F32 mag();
    V3 normalized();
    
    static F32 dot(V3 v, V3 w);
    static V3 cross(V3 v, V3 w);
};



#endif //CIEL_VECTORS_H