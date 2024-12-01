/* date = November 30th 2024 7:31 pm */

#ifndef V2_H
#define V2_H

// STRUCTS //
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
    
#ifdef GLM_VERSION_MAJOR
    glm::vec2 to_glm()
    {
        return glm::vec2(x, y);
    }
#endif
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
    
#ifdef GLM_VERSION_MAJOR
    glm::vec3 to_glm()
    {
        return glm::vec3(x, y, z);
    }
#endif
    
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
    // TODO: remove in-place normalize
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


// CONSTRUCTORS //
inline V2
v2(Float x, Float y)
{
    return {x, y};
}
inline V2
v2(Float *f)
{
    return {f[0], f[1]};
}
inline V2
v2(V2I v)
{
    return {(Float)v.x, (Float)v.y};
}
inline V2
v2(V3 v)
{
    return {(Float)v.x, (Float)v.y};
}

inline V2I
v2i(Int x, Int y)
{
    return {x, y};
}
inline V2I
v2i(Int *f)
{
    return {f[0], f[1]};
}
inline V2I
v2i(V2 vec)
{
    return {(Int)vec.x, (Int)vec.y};
}

inline V3
v3(Float x, Float y, Float z)
{
    return {x, y, z};
}
inline V3
v3(Float *f)
{
    return {f[0], f[1], f[2]};
}
inline V3
v3(glm::vec3 v)
{
    return {v.x, v.y, v.z};
}


// FUNCTIONS //
Float v2_dist(V2 a, V2 b)
{
    return sqrtf(pow(b.x-a.x, 2) + pow(b.y-a.y, 2));
}
Float v2i_dist(V2I a, V2I b)
{
    return sqrtf((Float)pow(b.x-a.x, 2) + (Float)pow(b.y-a.y, 2));
}

V2 interpolate(V2 from, V2 to, Float speed)
{
    return {
        interpolate(from.x, to.x, speed),
        interpolate(from.y, to.y, speed),
    };
}
V3 interpolate(V3 from, V3 to, Float speed)
{
    return {
        interpolate(from.x, to.x, speed),
        interpolate(from.y, to.y, speed),
        interpolate(from.z, to.z, speed),
    };
}

#endif //V2_H