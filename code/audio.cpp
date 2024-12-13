
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
    
    F64 hz = game_state->hz;
    hz = 0;
    SynthSongWaveData *song_wave_data = &game_state->test_song_wave_data;
    F64 time_accumualted = 0;
    Int note_index = 0;
    while(time_accumualted < game_state->time_in_song)
    {
        time_accumualted += song_wave_data->notes[note_index].time;
        note_index++;
    }
    if(note_index == 0) note_index = 1;
    if(note_index < song_wave_data->notes.length)
        hz = song_wave_data->notes[note_index - 1].hz;
    
    
    F64 volume = 0.2;
    F64 hz_scalar = 2.0*3.141592653589793/F64(AUDIO_SAMPLES_PER_SECOND);
    
    F64 x_scalar = hz * hz_scalar;
    F64 x = game_state->prev_sin_x;
    
    for(Int i = 0; i < samples_to_write; i++)
    {
        buffer->data[buffer->write_cursor] = sin(x) * F64(I32_MAX)*volume;
        
        buffer->write_cursor = (buffer->write_cursor+1) % sample_count;
        x += x_scalar;
    }
    
    game_state->prev_sin_x = x;
    game_state->time_in_song += F64(game_state->d_time);
}