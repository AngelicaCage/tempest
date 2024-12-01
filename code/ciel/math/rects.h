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
            F32 x1, y1, x2, y2;
        };
        struct
        {
            V2 pos, dim;
        };
        struct
        {
            V2 top_left, bottom_right;
        };
        struct
        {
            F32 data[4];
        };
    };
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
                I32 x1, y1, x2, y2;
            };
            struct
            {
                V2I pos, dim;
            };
            struct
            {
                V2I top_left, bottom_right;
            };
            struct
            {
                I32 data[4];
            };
            
        };
    };
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
