/* date = December 9th 2024 6:27 pm */

#ifndef TIMING_H
#define TIMING_H

struct SectionProfile
{
    const Char *name;
    F64 time;
};

struct FrameProfile
{
    F64 frame_start;
    F64 frame_end;
    F64 frame_time;
    
    F64 max_frame_time;
    
    List<SectionProfile> sections;
};
FrameProfile frame_profile;


#endif //TIMING_H
