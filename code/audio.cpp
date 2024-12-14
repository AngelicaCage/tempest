
Void
write_song_data(GameState *game_state, AudioBuffer *buffer)
{
#if 0
    F64 volume = 0.2;
    F64 hz_scalar = 2.0*3.141592653589793/F64(AUDIO_SAMPLES_PER_SECOND);
    
    SynthSongWaveData *song = &game_state->test_song_wave_data;
    
    // Write sound data for each hand
    for(Int i = 0; i < song->hands.length && i < MAX_HANDS_PER_SONG; i++)
    {
        HandWaveData *hand = &song->hands[i];
        
        // Get playing note index for the hand
        Int note_play_index = -1;
        F64 time_to_note_end = 0;
        
        for(Int j = 0; j < hand->notes.length; j++)
        {
            if(!(j > 0 && hand->notes[j-1].forms_chord_with_next))
                time_to_note_end += hand->notes[j].time;
            
            if(game_state->time_in_song < time_to_note_end)
            {
                note_play_index = j;
                break;
            }
        }
        
        if(note_play_index == -1)
            continue;
        
        //log("%d", note_play_index);
        
        Int notes_playing = 1;
        while(hand->notes[note_play_index+(notes_playing-1)].forms_chord_with_next)
            notes_playing++;
        
        for(Int j = 0; j < notes_playing; j++)
        {
            write_cursor_copy = buffer->write_cursor;
            
            NoteWaveData *note = &hand->notes[note_play_index + j];
            F64 x_scalar = note->frequency * hz_scalar;
            F64 *play_x_to_use = &(hand->play_xs[j]);
            
            for(Int k = 0; k < samples_to_write; k++)
            {
                buffer->data[write_cursor_copy] += sin(*play_x_to_use) * F64(I16_MAX)*volume;
                *play_x_to_use += x_scalar;
                write_cursor_copy = (write_cursor_copy+1) % sample_count;
            }
        }
    }
    
    buffer->write_cursor = write_cursor_copy;
    game_state->time_in_song += F64(game_state->d_time);
#endif
}



#if 0
AudioTrack
load_wav_file(const Char *path)
{
    FileContents file_contents = read_file_contents(path);
    ASSERT(file_contents.file_found);
    ASSERT(file_contents.allocated);
    ASSERT(file_contents.contains_proper_data);
    ASSERT(file_contents.size > 44);
    
    AudioTrack track = {0};
    U64 data_section_size = *((U64 *)(&file_contents.data[40]));
    U8 *data_start = &(file_contents.data[44]);
    
    track.length = data_section_size;
    for(U64 i = 0; i < data_section_size; i++)
    {
    }
    
    mem_free(file_contents.data);
    return track;
}
#endif




Void
write_frame_audio(GameState *game_state, AudioBuffer *buffer)
{
    Int sample_count = sizeof(buffer->data)/2;
    
    Float frame_time = game_state->d_time;
    Int max_samples_to_write = AUDIO_SAMPLES_PER_SECOND * frame_time * 2.5f;
    
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
    
    F64 volume = 0.1;
    F64 hz_scalar = 2.0*3.141592653589793/F64(AUDIO_SAMPLES_PER_SECOND);
    
    for(Int i = 0; i < samples_to_write; i++)
    {
        buffer->data[write_cursor_copy] = 0;
        write_cursor_copy = (write_cursor_copy+1) % sample_count;
    }
    
    AudioTrack *track = &game_state->test_track;
    write_cursor_copy = buffer->write_cursor;
    
    for(Int i = 0; i < samples_to_write; i++)
    {
#if 1
        if(write_cursor_copy % 2 == 0)
        {
            if(track->position >= track->length)
                break;
            buffer->data[write_cursor_copy] = track->data[track->position+0] * volume;
        }
        else
        {
            if(track->position+1 >= track->length)
                break;
            buffer->data[write_cursor_copy] = track->data[track->position+1] * volume;
            track->position += 2;
        }
        write_cursor_copy = (write_cursor_copy+1) % sample_count;
#else
        if(write_cursor_copy % 2 == 0)
        { // Write to left ear
            F64 x_scalar = 130 * hz_scalar;
            buffer->data[write_cursor_copy] += sin(game_state->at_1) * volume * (F64)I16_MAX;
            game_state->at_1 += x_scalar;
        }
        else
        { // Write to right ear
            F64 x_scalar = 466 * hz_scalar;
            buffer->data[write_cursor_copy] += sin(game_state->at_2) * volume * (F64)I16_MAX;
            game_state->at_2 += x_scalar;
        }
        
        write_cursor_copy = (write_cursor_copy+1) % sample_count;
#endif
    }
    
    log("%d", track->position);
    
    buffer->write_cursor = (buffer->write_cursor + samples_to_write) % sample_count;
}