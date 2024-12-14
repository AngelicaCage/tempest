/* date = December 14th 2024 10:39 am */

#ifndef AUDIO_H
#define AUDIO_H

struct AudioTrack
{
    U64 length;
    I16 *data;
    Int position;
};

#endif //AUDIO_H
