/* date = November 11th 2024 0:16 pm */

#ifndef GAME_LOADER_H
#define GAME_LOADER_H

#include "base.h"
#include "log.h"

struct OpenGLFunctions
{
    UInt (*glCreateShader)(Enum shaderType);
    Void (*glShaderSource)(UInt shader, UInt count, const Char **string, const Int *length);
    Void (*glCompileShader)(UInt shader);
    Void (*glGetShaderiv)(UInt shader, Enum pname, Int *params);
    Void (*glGetShaderInfoLog)(UInt shader, Int maxLength, Int *length, Char *infoLog);
    UInt (*glCreateProgram)();
    Void (*glAttachShader)(UInt program, UInt shader);
    Void (*glDetachShader)(UInt program, UInt shader);
    Void (*glLinkProgram)(UInt program);
    Void (*glValidateProgram)(UInt program);
    Void (*glGetProgramiv)(UInt program, Enum pname, Int *params);
    Void (*glGetProgramInfoLog)(UInt program, Int maxLength, Int *length, Char *infoLog);
    Void (*glGetActiveUniform)(UInt program, UInt index, Int bufSize, Int *length, Int *size, Enum *type, Char *name);
    Void (*glGenBuffers)(Int n, UInt *buffers);
    Void (*glGenVertexArrays)(Int n, UInt *arrays);
    Void (*glGetAttribLocation)(UInt program, const Char *name);
    Int (*glGetUniformLocation)(UInt program, const Char *name);
    Void (*glBindVertexArray)(UInt array);
    Void (*glEnableVertexAttribArray)(UInt index);
    Void (*glVertexAttribPointer)(UInt index, Int size, Enum type, Bool normalized, Int stride, const Void *pointer);
    Void (*glBindBuffer)(Enum target, UInt buffer);
    Void (*glBufferData)(Enum target, U64 size, const Void *data, Enum usage);
    Void (*glUseProgram)(UInt program);
    Void (*glDeleteVertexArrays)(Int n, const UInt *arrays);
    Void (*glDeleteBuffers)(Int n, const UInt *buffers);
    Void (*glDeleteProgram)(UInt program);
    Void (*glDeleteShader)(UInt shader);
    
    Void (*glUniform1i)(Int location, Int v0);
    Void (*glUniform1f)(Int location, Float v0);
    Void (*glUniform3f)(Int location, Float v0, Float v1, Float v2);
    Void (*glUniform2fv)(Int location, Int count, const Float *value);
    Void (*glUniform3fv)(Int location, Int count, const Float *value);
    Void (*glUniform4fv)(Int location, Int count, const Float *value);
    Void (*glUniformMatrix3fv)(Int location, Int count, Bool transpose, const Float *value);
    Void (*glUniformMatrix4fv)(Int location, Int count, Bool transpose, const Float *value);
    
    Void (*glPolygonMode)(Enum face, Enum mode);
    Void (*glLineWidth)(Float width);
    Void (*glDrawArrays)(Enum mode, Int first, Int count);
    Void (*glDrawElements)(Enum mode, Int count, Enum type, const Void *indices);
    
    Enum (*glGetError)();
    
    Void (*glGenTextures)(Int n, UInt *textures);
    Void (*glBindTexture)(Enum target, UInt texture);
    Void (*glTexImage2D)(Enum target, Int level, Int internalformat, Int width, Int height, Int border, Enum format, Enum type, const Void * data);
    Void (*glTexParameterf)(Enum target, Enum pname, Float param);
    Void (*glTexParameteri)(Enum target, Enum pname, Int param);
    Void (*glActiveTexture)(Enum texture);
    
    Void (*glBlendEquationSeparate)(Enum modeRGB, Enum modeAlpha);
    Void (*glClear)(UInt mask);
    Void (*glClearColor)(Float red, Float green, Float blue, Float alpha);
    Void (*glEnable)(Enum cap);
    Void (*glDisable)(Enum cap);
    Void (*glBlendFunc)(Enum sfactor, Enum dfactor);
};

struct KeyData
{
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
            KeyData mouse_left;
            KeyData mouse_middle;
            KeyData mouse_right;
            
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
            
            KeyData shift;
            KeyData control;
            KeyData alt;
            
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
            KeyData f13;
            KeyData f14;
            KeyData f15;
            KeyData f16;
            KeyData f17;
            KeyData f18;
            KeyData f19;
            KeyData f20;
            KeyData f21;
            KeyData f22;
            KeyData f23;
            KeyData f24;
        };
        struct
        {
            KeyData mouse[3];
            KeyData navigation[6];
            KeyData letters[26];
            KeyData numbers[10];
            KeyData symbols[11];
            KeyData special[7];
            KeyData modifiers[7];
            KeyData functions[12];
        };
        struct
        {
            KeyData data[3+6+26+10+11+7+7+12];
        };
    };
};

struct Input
{
    Float mouse_pos[2];
    Float prev_mouse_pos[2];
    Float d_scroll;
    Keys keys;
};

struct
FileContents
{
    Bool file_found;
    Bool allocated;
    Bool contains_proper_data;
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
    // TODO: have global_log stored here? Inline circular array?
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



#endif //GAME_LOADER_H
