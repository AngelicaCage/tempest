
Void
write_frame_audio(GameState *game_state, AudioBuffer *buffer)
{
    Int sample_count = sizeof(buffer->data)/2;
    
    Float frame_time = game_state->d_time;
    Int max_samples_to_write = AUDIO_SAMPLES_PER_SECOND * frame_time * 5.5f;
    
    Int cursor_diff = buffer->write_cursor - buffer->play_cursor;
    if(cursor_diff < 0)
    {
        // Assume that the write cursor has wrapped around, not that the play cursor has gone past the write cursor
        // In the future we should account for the latter case though
        if(-cursor_diff < sample_count / 2)
        {
            // The play cursor probably overran the write cursor
            buffer->write_cursor = buffer->play_cursor;
            cursor_diff = 0;
        }
        else
        {
            cursor_diff += sample_count;
        }
        
    }
    
    Int samples_to_write = max_samples_to_write - cursor_diff;
    if(samples_to_write <= 0)
        return;
    
    // Write audio starting at write cursor
    
    {
        Int write_cursor_copy = buffer->write_cursor;
        for(Int k = 0; k < samples_to_write; k++)
        {
            buffer->data[write_cursor_copy] = 0;
            write_cursor_copy = (write_cursor_copy+1) % sample_count;
        }
    }
    
    F64 volume = 0.2;
    F64 hz_scalar = 2.0*3.141592653589793/F64(AUDIO_SAMPLES_PER_SECOND);
    
    SynthSongWaveData *song = &game_state->test_song_wave_data;
    
    for(Int i = 0; i < song->hands.length && i < MAX_HANDS_PER_SONG; i++)
    {
        HandWaveData *hand = &song->hands[i];
        hand->play_note_group_index = 0;
        
        F64 time_accumualted = 0;
        while(time_accumualted < game_state->time_in_song)
        {
            if(hand->play_note_group_index >= hand->note_groups.length)
            {
                hand->play_note_group_index = -1;
                break;
            }
            time_accumualted += hand->note_groups[hand->play_note_group_index].time;
            hand->play_note_group_index++;
        }
        
        if(hand->play_note_group_index == 0) hand->play_note_group_index = 1;
        hand->play_note_group_index -= 1;
        
        if(hand->play_note_group_index < 0)
            continue;
        NoteWaveDataGroup *group = &hand->note_groups[hand->play_note_group_index];
        
        Int write_cursor_copy = buffer->write_cursor;
        for(Int i = 0; i < group->notes.length; i++)
        {
            write_cursor_copy = buffer->write_cursor;
            NoteWaveData *note = &group->notes[i];
            F64 x_scalar = note->frequency * hz_scalar;
            
            for(Int k = 0; k < samples_to_write; k++)
            {
                buffer->data[write_cursor_copy] += sin(note->play_x) * F64(I32_MAX)*volume;
                note->play_x += x_scalar;
                write_cursor_copy = (write_cursor_copy+1) % sample_count;
            }
        }
        
        buffer->write_cursor = write_cursor_copy;
    }
    game_state->time_in_song += F64(game_state->d_time);
    
#if 0
    SynthSongWaveData *song_wave_data = &game_state->test_song_wave_data;
    for(Int a = 0; a < song_wave_data->hands.length; a++)
    {
        HandWaveData *hand = &song_wave_data->hands[a];
        hand->hz = 0;
        F64 time_accumualted = 0;
        Int note_index = 0;
        while(time_accumualted < game_state->time_in_song)
        {
            if(note_index >= hand->notes.length)
            {
                note_index = -1;
                break;
            }
            time_accumualted += hand->notes[note_index].time;
            note_index++;
        }
        if(note_index < 0)
            continue;
        if(note_index == 0) note_index = 1;
        note_index -= 1;
        if(note_index < song_wave_data->hands[a].notes.length)
            hand->hz = song_wave_data->hands[a].notes[note_index].hz;
    }
    
    
    F64 volume = 0.2;
    F64 hz_scalar = 2.0*3.141592653589793/F64(AUDIO_SAMPLES_PER_SECOND);
    
    
    for(Int i = 0; i < samples_to_write; i++)
    {
        buffer->data[buffer->write_cursor] = 0;
        
        for(Int a = 0; a < song_wave_data->hands.length; a++)
        {
            HandWaveData *hand = &song_wave_data->hands[a];
            F64 x_scalar = hand->hz * hz_scalar;
            
            buffer->data[buffer->write_cursor] += sin(hand->x) * F64(I32_MAX)*volume;
            
            hand->x += x_scalar;
        }
        
        buffer->write_cursor = (buffer->write_cursor+1) % sample_count;
    }
    
    game_state->time_in_song += F64(game_state->d_time);
#endif
}