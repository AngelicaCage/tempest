
Void start_timing_section(const Char *name)
{
    frame_profile->section_stack.add({name, get_time(), 0, 0});
}

Void end_timing_section()
{
    ASSERT(frame_profile->section_stack.length > 0);
    if(frame_profile->section_stack.length > 0)
    {
        SectionProfile *section = &(frame_profile->section_stack.data[frame_profile->section_stack.length-1]);
        
        section->end_time = get_time();
        section->elapsed_time = section->end_time - section->start_time;
        
        frame_profile->finished_sections.add(*section);
        frame_profile->section_stack.length--;
    }
}

