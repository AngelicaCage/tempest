
Void
update_frame_timing_beginning(GameState *game_state)
{
    F64 this_frame_start_time = get_time();
    game_state->d_time = this_frame_start_time - game_state->last_frame_start_time;
    game_state->last_frame_start_time = this_frame_start_time;
    
    List<F64> *frame_times = &game_state->frame_times;
    frame_times->add(this_frame_start_time);
    if(frame_times->length > 144)
    {
        frame_times->remove_at(0);
    }
    F64 time_diff = frame_times->data[frame_times->length - 1] - frame_times->data[0];
    game_state->fps = (Float)frame_times->length / (Float)time_diff;
    
    
    frame_profile = &(game_state->frame_profile);
    frame_profile->frame_start = this_frame_start_time;
    frame_profile->max_frame_time = 1.0f / game_state->target_fps;
    frame_profile->section_stack.length = 0;
    frame_profile->finished_sections.length = 0;
}

Void update_frame_timing_end(GameState *game_state)
{
    // LATER: improve this
    F64 this_frame_end_time = get_time();
    F64 this_frame_time = this_frame_end_time - frame_profile->frame_start;
    F64 time_to_sleep = (1.0 / game_state->target_fps) - this_frame_time;
    
    frame_profile->frame_end = this_frame_end_time;
    frame_profile->frame_time = frame_profile->frame_end - frame_profile->frame_start;
}

Void start_timing_section(const Char *name)
{
    frame_profile->section_stack.push({name, get_time(), 0, 0});
}

Void end_timing_section()
{
    ASSERT(frame_profile->section_stack.length > 0);
    if(frame_profile->section_stack.length > 0)
    {
        SectionProfile *section = &(frame_profile->section_stack[frame_profile->section_stack.length-1]);
        
        section->end_time = get_time();
        section->elapsed_time = section->end_time - section->start_time;
        
        frame_profile->finished_sections.push(*section);
        frame_profile->section_stack.length--;
    }
}