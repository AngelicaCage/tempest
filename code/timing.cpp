
Void
update_frame_timing_beginning(GameState *game_state, Float d_time)
{
    FrameProfile *old_profile = &(game_state->frame_profiles.last());
    old_profile->d_time = d_time;
    
#if 0
    game_state->d_time = old_profile->elapsed_time;
    if(game_state->d_time < 1.0f / game_state->target_fps)
        game_state->d_time = 1.0f / game_state->target_fps;
#endif
    
    game_state->frame_profiles.add({0});
    FrameProfile *profile = &(game_state->frame_profiles.last());
    
    profile->start_time = get_time();
    profile->target_max_time = 1.0f / game_state->target_fps;
    profile->section_stack.length = 0;
    profile->finished_sections.length = 0;
}

Void update_frame_timing_end(GameState *game_state)
{
    FrameProfile *profile = &(game_state->frame_profiles.last());
    
    profile->end_time = get_time();
    profile->elapsed_time = profile->end_time - profile->start_time;
}

Void start_timing_section(FrameProfile *profile, const Char *name)
{
    profile->section_stack.push({name, get_time(), 0, 0});
}

Void end_timing_section(FrameProfile *profile)
{
    ASSERT(profile->section_stack.length > 0);
    if(profile->section_stack.length > 0)
    {
        SectionProfile *section = &(profile->section_stack[profile->section_stack.length-1]);
        
        section->end_time = get_time();
        section->elapsed_time = section->end_time - section->start_time;
        
        profile->finished_sections.push(*section);
        profile->section_stack.length--;
    }
}