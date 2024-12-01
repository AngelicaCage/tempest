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
            F32 data[2];
        };
    };
    
    F32 mag()
    {
        return sqrtf(x*x + y*y);
    }
    Void normalize()
    {
        F32 m = mag();
        x = x / m;
        y = y / m;
    }
    
#ifdef GLM_VERSION_MAJOR
    glm::vec2 to_glm()
    {
        return glm::vec2(x, y);
    }
#endif
    Void interpolate_to(V2 target, F32 speed)
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
            I32 x, y;
        };
        struct
        {
            I32 data[2];
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
            F32 data[3];
        };
    };
    
#ifdef GLM_VERSION_MAJOR
    glm::vec3 to_glm()
    {
        return glm::vec3(x, y, z);
    }
#endif
    
    F32 mag()
    {
        return sqrtf(x*x + y*y + z*z);
    }
    V3 normalized()
    {
        V3 result;
        F32 m = mag();
        result.x = x / m;
        result.y = y / m;
        result.z = z / m;
        return result;
    }
    // TODO: remove in-place normalize
    Void normalize()
    {
        F32 m = mag();
        x = x / m;
        y = y / m;
        z = z / m;
    }
    
    static F32 dot(V3 a, V3 b)
    {
        return a.x*b.x + a.y*b.y + a.z*b.z;
    }
#if 0
    static V3 cross(V3 a, V3 b)
    {
        F32 angle_between = acos(dot(a, b)/(a.mag()*b.mag()));
        return a.mag()*b.mag()*
    }
#endif
};


// CONSTRUCTORS //
inline V2
v2(F32 x, F32 y)
{
    return {x, y};
}
inline V2
v2(F32 *f)
{
    return {f[0], f[1]};
}
inline V2
v2(V2I v)
{
    return {(F32)v.x, (F32)v.y};
}
inline V2
v2(V3 v)
{
    return {(F32)v.x, (F32)v.y};
}

inline V2I
v2i(I32 x, I32 y)
{
    return {x, y};
}
inline V2I
v2i(I32 *f)
{
    return {f[0], f[1]};
}
inline V2I
v2i(V2 vec)
{
    return {(I32)vec.x, (I32)vec.y};
}

inline V3
v3(F32 x, F32 y, F32 z)
{
    return {x, y, z};
}
inline V3
v3(F32 *f)
{
    return {f[0], f[1], f[2]};
}
inline V3
v3(glm::vec3 v)
{
    return {v.x, v.y, v.z};
}


// FUNCTIONS //
F32 v2_dist(V2 a, V2 b)
{
    return sqrtf(pow(b.x-a.x, 2) + pow(b.y-a.y, 2));
}
F32 v2i_dist(V2I a, V2I b)
{
    return sqrtf((F32)pow(b.x-a.x, 2) + (F32)pow(b.y-a.y, 2));
}

V2 interpolate(V2 from, V2 to, F32 speed)
{
    return {
        interpolate(from.x, to.x, speed),
        interpolate(from.y, to.y, speed),
    };
}
V3 interpolate(V3 from, V3 to, F32 speed)
{
    return {
        interpolate(from.x, to.x, speed),
        interpolate(from.y, to.y, speed),
        interpolate(from.z, to.z, speed),
    };
}


// OPERATORS //
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
inline V2
operator*(V2 a, Float scalar)
{
    return {a.x * scalar, a.y *scalar};
}
inline V2
operator/(V2 a, Float scalar)
{
    return {a.x / scalar, a.y / scalar};
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

#endif //CIEL_VECTORS_H