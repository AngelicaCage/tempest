/* date = December 14th 2024 10:39 am */

#ifndef AUDIO_H
#define AUDIO_H

// Frame refers to all the audio data at an instance in time (a frame). If there are 2 channels
// then it will be 2 samples, 1 -> 1, etc.
// So samples array length == samples_per_frame*frame_count
struct Sound
{
    // We always use 2-channel audio, so 2 samples per frame
    U64 frame_count;
    
    I16 *samples;
};

struct PlayingSound
{
    Sound *sound;
    
    Float volume;
    
    Bool loop;
    U64 current_frame;
};

#endif //AUDIO_H

