/* date = November 18th 2024 8:13 pm */

#ifndef MATH_H
#define MATH_H

Float pi = 3.141582f;

struct V2
{
    union
    {
        struct
        {
            Float x, y;
        };
        struct
        {
            Float data[2];
        };
    };
    
    glm::vec2 to_glm()
    {
        return glm::vec2(x, y);
    }
};
V2 v2(Float x, Float y)
{
    V2 result = {x, y};
    return result;
}

struct V3
{
    union
    {
        struct
        {
            Float x, y, z;
        };
        struct
        {
            Float data[3];
        };
    };
    
    glm::vec3 to_glm()
    {
        return glm::vec3(x, y, z);
    }
};
V3 v3(Float x, Float y, Float z)
{
    V3 result = {x, y, z};
    return result;
}

#endif //MATH_H
