/* date = December 14th 2024 10:39 am */

#ifndef AUDIO_H
#define AUDIO_H

// Frame refers to all the audio data at an instance in time (a frame). If there are 2 channels
// then it will be 2 samples, 1 -> 1, etc.
// So samples array length == samples_per_frame*frame_count
struct AudioTrack
{
    U32 samples_per_frame;
    U64 frame_count;
    
    I16 *samples;
};

struct PlayingAudioTrack
{
    AudioTrack *audio;
    U64 current_frame;
    // Later: add looping
};

#endif //AUDIO_H

