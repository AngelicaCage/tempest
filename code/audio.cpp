
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
    
    Int write_cursor_copy = buffer->write_cursor;
    
    for(Int k = 0; k < samples_to_write; k++)
    {
        buffer->data[write_cursor_copy] = 0;
        write_cursor_copy = (write_cursor_copy+1) % sample_count;
    }
    
    F64 volume = 0.2;
    F64 hz_scalar = 2.0*3.141592653589793/F64(AUDIO_SAMPLES_PER_SECOND);
    
    SynthSongWaveData *song = &game_state->test_song_wave_data;
    
    // Write sound data for each hand
    for(Int i = 0; i < song->hands.length && i < MAX_HANDS_PER_SONG; i++)
    {
        HandWaveData *hand = &song->hands[i];
        
        // Get playing note index for the hand
        Int note_play_index = 0;
        F64 time_accumulated = 0;
        
        while(time_accumulated < game_state->time_in_song)
        {
            if(note_play_index > hand->notes.length)
                break;
            
            time_accumulated += hand->notes[note_play_index].time;
            note_play_index++;
        }
        
        note_play_index--;
        if(note_play_index >= hand->notes.length)
            continue;
        
        if(note_play_index < 0) note_play_index = 0;
        
        
        Int notes_playing = 1;
        while(hand->notes[note_play_index+(notes_playing-1)].forms_chord_with_next)
            notes_playing++;
        
        for(Int j = 0; j < notes_playing; j++)
        {
            write_cursor_copy = buffer->write_cursor;
            
            NoteWaveData *note = &hand->notes[note_play_index + j];
            F64 x_scalar = note->frequency * hz_scalar;
            F64 *play_x_to_use = &(hand->play_xs[0]);
            
            for(Int k = 0; k < samples_to_write; k++)
            {
                buffer->data[write_cursor_copy] += sin(*play_x_to_use) * F64(I32_MAX)*volume;
                *play_x_to_use += x_scalar;
                write_cursor_copy = (write_cursor_copy+1) % sample_count;
            }
        }
    }
    
    buffer->write_cursor = write_cursor_copy;
    game_state->time_in_song += F64(game_state->d_time);
}