/* date = December 9th 2024 6:27 pm */

#ifndef TIMING_H
#define TIMING_H

// TODO: move this to be shared, like Log
// TODO: have two frame profiles, one for previous frame and one for current frame
// TODO: come up with better names

struct SectionProfile
{
    const Char *name;
    
    F64 start_time;
    F64 end_time;
    F64 elapsed_time;
};

struct FrameProfile
{
    F64 start_time;
    F64 end_time;
    F64 elapsed_time;
    
    F64 target_max_time;
    
    InplaceStack(SectionProfile, 10) section_stack;
    InplaceStack(SectionProfile, 10) finished_sections;
};
FrameProfile *frame_profile;

#endif //TIMING_H
