
Void
update_frame_timing_beginning(GameState *game_state)
{
    frame_profile = &(game_state->frame_profile);
    game_state->d_time = frame_profile->elapsed_time;
    if(game_state->d_time == 0)
        game_state->d_time = 0.06f;
    
    frame_profile->start_time = get_time();
    frame_profile->target_max_time = 1.0f / game_state->target_fps;
    frame_profile->section_stack.length = 0;
    frame_profile->finished_sections.length = 0;
}

Void update_frame_timing_end(GameState *game_state)
{
    frame_profile->end_time = get_time();
    frame_profile->elapsed_time = frame_profile->end_time - frame_profile->start_time;
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