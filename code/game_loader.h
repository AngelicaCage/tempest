/* date = November 11th 2024 0:16 pm */

#ifndef WINDOWS_GAME_LOADER_H
#define WINDOWS_GAME_LOADER_H

#include "base.h"
#include "log.h"

struct KeyData
{
    Int key_code;
    Bool just_pressed;
    Bool is_down;
    Float press_time;
    Float time_till_next_repeat;
};

struct Keys
{
    union
    {
        struct
        {
            KeyData left;
            KeyData right;
            KeyData up;
            KeyData down;
            KeyData page_up;
            KeyData page_down;
            
            KeyData a;
            KeyData b;
            KeyData c;
            KeyData d;
            KeyData e;
            KeyData f;
            KeyData g;
            KeyData h;
            KeyData i;
            KeyData j;
            KeyData k;
            KeyData l;
            KeyData m;
            KeyData n;
            KeyData o;
            KeyData p;
            KeyData q;
            KeyData r;
            KeyData s;
            KeyData t;
            KeyData u;
            KeyData v;
            KeyData w;
            KeyData x;
            KeyData y;
            KeyData z;
            
            KeyData number_0;
            KeyData number_1;
            KeyData number_2;
            KeyData number_3;
            KeyData number_4;
            KeyData number_5;
            KeyData number_6;
            KeyData number_7;
            KeyData number_8;
            KeyData number_9;
            
            KeyData grave;
            KeyData minus;
            KeyData equal;
            KeyData left_bracket;
            KeyData right_bracket;
            KeyData backslash;
            KeyData semicolon;
            KeyData quote;
            KeyData slash;
            KeyData comma;
            KeyData period;
            
            KeyData space;
            KeyData backspace;
            KeyData del;
            KeyData tab;
            KeyData enter;
            KeyData caps_lock;
            KeyData escape;
            
            // Later: switch the order of these names
            KeyData shift_left;
            KeyData shift_right;
            KeyData control_left;
            KeyData control_right;
            KeyData alt_left;
            KeyData alt_right;
            KeyData function;
            
            KeyData f1;
            KeyData f2;
            KeyData f3;
            KeyData f4;
            KeyData f5;
            KeyData f6;
            KeyData f7;
            KeyData f8;
            KeyData f9;
            KeyData f10;
            KeyData f11;
            KeyData f12;
        };
        struct
        {
            KeyData nav[6];
            KeyData letters[26];
            KeyData numbers[10];
            KeyData symbols[11];
            KeyData special[7];
            KeyData modifiers[7];
            KeyData functions[12];
        };
        struct
        {
            KeyData data[6+26+10+11+7+7+12];
        };
    };
    
};

struct Input
{
    F32 key_first_repeat_time;
    F32 key_repeat_speed;
    
    Float mouse_pos[2];
    Float d_mouse_pos[2];
    Bool left_mouse_down;
    Bool right_mouse_down;
    
    Float d_scroll;
    
    Keys keys;
};

struct
FileContents
{
    Bool file_found;
    Bool allocated;
    Bool contains_proper_data;
    // Later: change this to U8 * ?
    U8 *data;
    U64 size;
};

// Later: make a more reasoned size
#define AUDIO_FRAMES_PER_SECOND (44100)
#define AUDIO_SAMPLES_PER_FRAME (2)
#define AUDIO_BYTES_PER_SAMPLE (2)
#define AUDIO_BUFFER_SECONDS (0.5f)
#define AUDIO_BUFFER_FRAME_COUNT ((Int)((Float)AUDIO_FRAMES_PER_SECOND * AUDIO_BUFFER_SECONDS * AUDIO_SAMPLES_PER_FRAME))
#define AUDIO_BUFFER_SAMPLE_COUNT ((Int)(AUDIO_BUFFER_FRAME_COUNT * AUDIO_SAMPLES_PER_FRAME))

struct AudioBuffer
{
    // All measured in frames
    U64 play_cursor_absolute;
    U64 write_cursor_absolute;
    
    //U32 play_cursor;
    //U32 write_cursor;
    U64 total_frames_played;
    I16 samples[AUDIO_BUFFER_SAMPLE_COUNT];
    
    inline U32 play_cursor_actual()
    {
        return U32(play_cursor_absolute % U64(AUDIO_BUFFER_FRAME_COUNT));
    }
    inline U32 write_cursor_actual()
    {
        return U32(write_cursor_absolute % U64(AUDIO_BUFFER_FRAME_COUNT));
    }
};

struct GameMemory
{
    Bool game_running;
    Bool functions_loaded;
    
    Void *memory;
    U64 size;
    Bool allocated;
    
    AudioBuffer audio_buffer;
    // TODO: have global_log stored here? Inline stack alloc?
    Log *global_log;
    Input input;
    
    // Later: put names in the arguments?
    U64 (*get_file_last_write_time)(const Char *);
    FileContents (*read_file_contents)(const Char *);
    Bool (*write_file_contents)(const Char *, U8 *, U64);
    F64 (*get_time)();
    Void (*sleep)(F64);
    
    F64 d_time;
};

struct GameCode
{
    Bool loaded;
    
    Void (*update_and_render)(GameMemory *);
};



#endif //WINDOWS_GAME_LOADER_H
