
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
Sound *
load_sound_wav(GameState *game_state, const Char *path)
{
    
    FileContents audio_file_contents = read_file_contents(path);
    
    UInt channels;
    UInt sample_rate;
    U64 frame_count;
    drwav_int16* samples = drwav_open_memory_and_read_pcm_frames_s16(audio_file_contents.data, audio_file_contents.size,
                                                                     &channels, &sample_rate, &frame_count, NULL);
    
    Sound sound;
    sound.frame_count = frame_count;
    sound.samples_per_frame = channels;
    sound.samples = samples;
    
    return game_state->sounds.add(sound);
}
#endif

Sound *
load_sound_mp3(GameState *game_state, const Char *path)
{
    
    FileContents audio_file_contents = read_file_contents(path);
    
    UInt sample_rate;
    U64 frame_count;
    drmp3_config config;
    config.channels = 2;
    config.sampleRate = AUDIO_FRAMES_PER_SECOND;
    I16* samples = drmp3_open_memory_and_read_pcm_frames_s16(audio_file_contents.data, audio_file_contents.size,
                                                             &config, &frame_count, NULL);
    
    Sound sound;
    sound.frame_count = frame_count;
    sound.samples = samples;
    
    return game_state->sounds.add(sound);
}


Void
play_sound(GameState *game_state, Sound *sound, Bool loop)
{
    PlayingSound playing_sound;
    playing_sound.sound = sound;
    playing_sound.current_frame = 0;
    playing_sound.loop = loop;
    game_state->playing_sounds.add(playing_sound);
}


Void
write_frame_audio(GameState *game_state, AudioBuffer *buffer)
{
    Int buffer_sample_count = sizeof(buffer->samples)/sizeof(I16);
    Int buffer_frame_count = buffer_sample_count / 2;
    
    Float d_time = game_state->d_time;
    // Later: use a more reasoned value
    Int max_frames_to_write = AUDIO_FRAMES_PER_SECOND * d_time * 10.5f;
    max_frames_to_write = clamp(max_frames_to_write, 0, AUDIO_BUFFER_FRAME_COUNT);
    
    Int cursor_diff = Int(buffer->write_cursor_absolute - buffer->play_cursor_absolute);
    if(cursor_diff < 0)
    {
        buffer->write_cursor_absolute = buffer->play_cursor_absolute;
    }
    
    Int frames_to_write = max_frames_to_write - cursor_diff;
    if(frames_to_write <= 0)
        return;
    
    U64 write_cursor_copy = buffer->write_cursor_absolute;
    
    F64 volume = 0.2;
    volume = 0;
    
    write_cursor_copy = buffer->write_cursor_absolute;
    
    for(Int i = 0; i < frames_to_write; i++)
    {
        U32 write_cursor_copy_actual = U32(write_cursor_copy % U64(AUDIO_BUFFER_FRAME_COUNT));
        
        buffer->samples[write_cursor_copy_actual*2] = 0;
        buffer->samples[write_cursor_copy_actual*2+1] = 0;
        
        for(Int a = 0; a < game_state->playing_sounds.length; a++)
        {
            PlayingSound *playing_sound = &(game_state->playing_sounds[a]);
            
            if(playing_sound->current_frame >= playing_sound->sound->frame_count)
            {
                if(!playing_sound->loop)
                {
                    game_state->playing_sounds.remove_at(a);
                    continue;
                }
                else
                {
                    playing_sound->current_frame = 0;
                }
            }
            
            buffer->samples[write_cursor_copy_actual*2] += 
                playing_sound->sound->samples[playing_sound->current_frame*2] * volume;
            buffer->samples[write_cursor_copy_actual*2+1] += 
                playing_sound->sound->samples[playing_sound->current_frame*2+1] * volume;
            
            playing_sound->current_frame++;
        }
        
        write_cursor_copy++;
    }
    
    buffer->write_cursor_absolute += U64(frames_to_write);
}