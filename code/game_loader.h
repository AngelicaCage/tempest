/* date = November 11th 2024 0:16 pm */

#ifndef WINDOWS_GAME_LOADER_H
#define WINDOWS_GAME_LOADER_H

#include "glfw/glfw3.h"

#include "base.h"
#include "log.h"

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
    
    Log *global_log;
    GLFWwindow *window;
    
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
