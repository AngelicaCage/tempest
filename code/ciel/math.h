/* date = November 18th 2024 8:13 pm */

#ifndef CIEL_MATH_H
#define CIEL_MATH_H


Float pi = 3.141582f;

Float interpolate(Float a, Float b, Float speed)
{
    if(speed > 1) speed = 1;
    if(speed < 0) speed = 0.1f;
    return a + (b - a) * speed;
}

#include "math/vectors.h"
#include "math/rects.h"




F32
random_float(F32 min, F32 max)
{
    return (F32)rand()/(F32)RAND_MAX * (max-min) + min;
}

// inc, exc
Int
random_int(Int min, Int max)
{
    return (F32)rand()/(F32)RAND_MAX * (max-min) + min;
}

inline Float clamp(Float val, Float min, Float max)
{
    if(val < min) return min;
    if(val > max) return max;
    return val;
}
inline Int clamp(Int val, Int min, Int max)
{
    if(val < min) return min;
    if(val > max) return max;
    return val;
}
inline Int clamp_circle(Int val, Int min, Int max)
{
    if(val < min) return max;
    if(val > max) return min;
    return val;
}


inline V2 clamp(V2 val, V2 min, V2 max)
{
    if(val.x < min.x) val.x = min.x;
    if(val.y < min.y) val.y = min.y;
    if(val.x > max.x) val.x = max.x;
    if(val.y > max.y) val.y = max.y;
    
    return val;
}




#endif //CIEL_MATH_H
