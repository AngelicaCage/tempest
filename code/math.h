/* date = November 18th 2024 8:13 pm */

#ifndef MATH_H
#define MATH_H

Float pi = 3.141582f;

Float interpolate(Float a, Float b, Float speed)
{
    if(speed > 1) speed = 1;
    if(speed < 0) speed = 0.1f;
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
    
    Float mag()
    {
        return sqrtf(x*x + y*y);
    }
    Void normalize()
    {
        Float m = mag();
        x = x / m;
        y = y / m;
    }
    
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
struct V2I
{
    union
    {
        struct
        {
            Int x, y;
        };
        struct
        {
            Int data[2];
        };
    };
};

V2 v2(Float x, Float y)
{
    V2 result = {x, y};
    return result;
}
V2 v2(V2I v)
{
    return {(Float)v.x, (Float)v.y};
}
Float v2_dist(V2 a, V2 b)
{
    return sqrtf(pow(b.x-a.x, 2) + pow(b.y-a.y, 2));
}

V2I v2i(Int x, Int y)
{
    V2I result = {x, y};
    return result;
}
V2I v2i(V2 vec)
{
    V2I result = {(Int)vec.x, (Int)vec.y};
    return result;
}
Float v2i_dist(V2I a, V2I b)
{
    return sqrtf((Float)pow(b.x-a.x, 2) + (Float)pow(b.y-a.y, 2));
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
    Float mag()
    {
        return sqrtf(x*x + y*y + z*z);
    }
    V3 normalized()
    {
        V3 result;
        Float m = mag();
        result.x = x / m;
        result.y = y / m;
        result.z = z / m;
        return result;
    }
    Void normalize()
    {
        Float m = mag();
        x = x / m;
        y = y / m;
        z = z / m;
    }
    
    static Float dot(V3 a, V3 b)
    {
        return a.x*b.x + a.y*b.y + a.z*b.z;
    }
#if 0
    static V3 cross(V3 a, V3 b)
    {
        Float angle_between = acos(dot(a, b)/(a.mag()*b.mag()));
        return a.mag()*b.mag()*
    }
#endif
};
V3 v3(Float x, Float y, Float z)
{
    V3 result = {x, y, z};
    return result;
}
V3 v3_from_floats(Float *f)
{
    V3 result = {f[0], f[1], f[2]};
    return result;
}
V3 v3_from_glm(glm::vec3 v)
{
    V3 result = {v.x, v.y, v.z};
    return result;
}

F32
random_float(F32 min, F32 max)
{
    return (F32)rand()/(F32)RAND_MAX * (max-min) + min;
}


inline V3
operator+(V3 a, V3 b)
{
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}
inline V3 &
operator+=(V3 &a, V3 b)
{
    a=a+b;
    return a;
}
inline V3
operator-(V3 a, V3 b)
{
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}
inline V3 &
operator-=(V3 &a, V3 b)
{
    a=a-b;
    return a;
}

inline V2
operator+(V2 a, V2 b)
{
    return {a.x + b.x, a.y + b.y};
}
inline V2 &
operator+=(V2 &a, V2 b)
{
    a=a+b;
    return a;
}
inline V2
operator-(V2 a, V2 b)
{
    return {a.x - b.x, a.y - b.y};
}
inline V2 &
operator-=(V2 &a, V2 b)
{
    a=a-b;
    return a;
}

inline V2I
operator+(V2I a, V2I b)
{
    return {a.x + b.x, a.y + b.y};
}
inline V2I &
operator+=(V2I &a, V2I b)
{
    a=a+b;
    return a;
}
inline V2I
operator-(V2I a, V2I b)
{
    return {a.x - b.x, a.y - b.y};
}
inline V2I &
operator-=(V2I &a, V2I b)
{
    a=a-b;
    return a;
}



#endif //MATH_H
