/* date = November 30th 2024 8:49 pm */

#ifndef CIEL_RECTS_H
#define CIEL_RECTS_H

// STRUCTS //
struct
Rect
{
    union
    {
        struct
        {
            F32 x, y, w, h;
        };
        struct
        {
            V2 pos, dim;
        };
        struct
        {
            F32 components[4];
        };
    };
    
    inline F32 area()
    {
        return w*h;
    }
};

struct
RectI
{
    union
    {
        struct
        {
            struct
            {
                I32 x, y, w, h;
            };
            struct
            {
                V2I pos, dim;
            };
            struct
            {
                I32 components[4];
            };
        };
    };
    
    inline I32 area()
    {
        return w*h;
    }
};

// CONSTRUCTORS //
inline Rect
rect(F32 x, F32 y, F32 w, F32 h)
{
    return {x, y, w, h};
}
inline Rect
rect(V2 pos, V2 dim)
{
    return {pos.x, pos.y, dim.x, dim.y};
}

inline RectI
recti(I32 x, I32 y, I32 w, I32 h)
{
    return {x, y, w, h};
}
inline RectI
recti(V2I pos, V2I dim)
{
    return {pos.x, pos.y, dim.x, dim.y};
}


#endif //CIEL_RECTS_H
