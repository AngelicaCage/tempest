/* date = November 18th 2024 8:13 pm */

#ifndef MATH_H
#define MATH_H

Float pi = 3.141582f;

Float interpolate(Float a, Float b, Float speed)
{
    return a + (b - a) * speed;
}

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
    Void interpolate_to(V2 target, Float speed)
    {
        x = interpolate(x, target.x, speed);
        y = interpolate(y, target.y, speed);
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
    Void interpolate_to(V3 target, Float speed)
    {
        x = interpolate(x, target.x, speed);
        y = interpolate(y, target.y, speed);
        z = interpolate(z, target.z, speed);
    }
};
V3 v3(Float x, Float y, Float z)
{
    V3 result = {x, y, z};
    return result;
}


F32
random_float(F32 min, F32 max)
{
    return (F32)rand()/(F32)RAND_MAX * (max-min) + min;
}


#endif //MATH_H
