/* date = December 1st 2024 1:54 pm */

#ifndef COLOR_H
#define COLOR_H

struct Color
{
    union
    {
        struct
        {
            Float r, g, b, a;
        };
        struct
        {
            Float components[4];
        };
    };
    
    Color inverted()
    {
        return {1-r, 1-g, 1-b, a};
    }
    
    // TODO: make these more tolerable colors?
    inline static Color red() { return {1, 0, 0, 1}; }
    inline static Color blue() { return {0, 0, 1, 1}; }
    inline static Color yellow() { return {1, 1, 0, 1}; }
    
    inline static Color purple() { return {1, 0, 1, 1}; }
    inline static Color green() { return {0, 1, 0, 1}; }
    inline static Color orange() { return {1, 0.5, 0, 1}; }
};

inline Color
color(Float r, Float g, Float b, Float a)
{
    return {r, g, b, a};
};
inline Color
interpolate(Color from, Color to, Float speed)
{
    return {
        interpolate(from.r, to.r, speed),
        interpolate(from.g, to.g, speed),
        interpolate(from.b, to.b, speed),
        interpolate(from.a, to.a, speed),
    };
}

#endif //COLOR_H
