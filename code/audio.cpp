
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
    
    // get_time() and extrapolate based on AUDIO_SAMPLES_PER_SECOND
    F64 seconds_per_sample = 1.0 / (F64)AUDIO_SAMPLES_PER_SECOND;
    
    U64 total_samples_written = buffer->total_samples_played + cursor_diff;
    
    Float prev_hz = game_state->prev_hz;
    Float hz = game_state->hz;
    Float volume = 0.2f;
    Float hz_scalar = 2.0*3.141592/Float(AUDIO_SAMPLES_PER_SECOND);
    
    // use prev y pos to find an x offset??
    //Float prev_y = sinf(prev_hz*hz_scalar*game_state->prev_frame_end_index);
    //Float wavelength = hz_scalar;
    //Float index = asinf(prev_y) / (hz * hz_scalar);
    //Float index = 0;
    
    Float index = game_state->prev_frame_end_index * prev_hz / hz;
    
    for(Int i = 0; i < samples_to_write; i++)
    {
        Float x_scalar = hz * hz_scalar;
        //log("%lf", x_scalar);
        
        // Treat our start position as 0, construct sin functions around that
        Float x = x_scalar*index;
        
        buffer->data[buffer->write_cursor] = sinf(x) * Float(I32_MAX)*volume;
        
        total_samples_written++;
        buffer->write_cursor = (buffer->write_cursor+1) % sample_count;
        index++;
    }
    game_state->prev_frame_end_index = index;
}