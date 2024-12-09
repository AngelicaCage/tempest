

// V2 //

F32 V2::mag()
{
    return sqrtf(x*x + y*y);
}
V2 V2::normalized()
{
    F32 m = mag();
    return {x/m, y/m};
}

V2 interpolate(V2 from, V2 to, F32 speed)
{
    return {
        interpolate(from.x, to.x, speed),
        interpolate(from.y, to.y, speed),
    };
}
V2 clamp(V2 val, V2 min, V2 max)
{
    if(val.x < min.x) val.x = min.x;
    if(val.y < min.y) val.y = min.y;
    if(val.x > max.x) val.x = max.x;
    if(val.y > max.y) val.y = max.y;
    
    return val;
}



// V3 //

#ifdef GLM_VERSION_MAJOR
glm::vec3 V3::to_glm()
{
    return glm::vec3(x, y, z);
}
#endif
F32 V3::mag()
{
    return sqrtf(x*x + y*y + z*z);
}
V3 V3::normalized()
{
    V3 result;
    F32 m = mag();
    result.x = x / m;
    result.y = y / m;
    result.z = z / m;
    return result;
}

//
F32 V3::dot(V3 v, V3 w)
{
    return v.x*w.x + v.y*w.y + v.z*w.z;
}

// Returns a V3 perpendicular to the plane formed by v and w.
// Direction is determined by the right-hand rule.
V3 V3::cross(V3 v, V3 w)
{
    return {v.y*w.z-w.y*v.z, v.z*w.x-w.z*v.x, v.x*w.y-w.x*v.y};
}

V3 interpolate(V3 from, V3 to, F32 speed)
{
    return {
        interpolate(from.x, to.x, speed),
        interpolate(from.y, to.y, speed),
        interpolate(from.z, to.z, speed),
    };
}


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
inline V2 &
operator+=(V2 &a, V2 b)
{
    a=a+b;
    return a;
}
inline V2 &
operator*=(V2 &a, Float scalar)
{
    a=a*scalar;
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


