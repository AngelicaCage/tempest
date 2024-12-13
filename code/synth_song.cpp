

Void
SynthSong::push_note_group(Int hand, NoteLength length, Note note1)
{
    NoteGroup group = {0};
    group.is_rest = false;
    group.notes.push(note1);
    
    hands[hand].note_groups.push(group);
}
Void
SynthSong::push_rest(Int hand, NoteLength length)
{
    NoteGroup group = {0};
    group.is_rest = false;
    
    hands[hand].note_groups.push(group);
}
Void
SynthSong::push_note(Int hand, NoteLength length, Note note)
{
    push_note_group(hand, length, note);
}


NoteGroup create_rest(NoteLength length)
{
    NoteGroup result = {0};
    
    result.is_rest = true;
    result.length = length;
    
    return result;
}
Note create_note(Char letter, UInt octave, NoteSemitone semitone)
{
    Note result = {0};
    
    result.letter = letter;
    result.octave = octave;
    result.semitone = semitone;
    
    return result;
}

// Based on https://www.researchgate.net/publication/324760170/figure/tbl1/AS:631594675081245@1527595313972/Frequencies-of-musical-notes.png

F64 note_frequencies[12][5] = {
    {32, 65, 130, 261, 523},
    {34, 69, 138, 277, 554},
    {36, 73, 146, 293, 587},
    {38, 77, 155, 311, 622},
    {41, 82, 164, 329, 659},
    {43, 87, 174, 349, 698},
    {46, 92, 185, 369, 739},
    {49, 98, 196, 392, 784},
    {52, 104, 208, 415, 830},
    {55, 110, 220, 440, 880},
    {58, 116, 233, 466, 932},
    {61, 123, 246, 493, 987},
};

F64
note_get_frequency(Char letter, NoteSemitone semitone, UInt octave)
{
    if(letter == 'c')
    {
        if(semitone == NoteSemitone::none || semitone == NoteSemitone::natural)
        {
            return note_frequencies[0][octave-1];
        }
        else if (semitone == NoteSemitone::sharp)
        {
            return note_frequencies[1][octave-1];
        }
    }
    else if(letter == 'd')
    {
        if(semitone == NoteSemitone::none || semitone == NoteSemitone::natural)
        {
            return note_frequencies[2][octave-1];
        }
        else if (semitone == NoteSemitone::sharp)
        {
            return note_frequencies[3][octave-1];
        }
    }
    else if(letter == 'e')
    {
        if(semitone == NoteSemitone::none || semitone == NoteSemitone::natural)
        {
            return note_frequencies[4][octave-1];
        }
    }
    else if(letter == 'f')
    {
        if(semitone == NoteSemitone::none || semitone == NoteSemitone::natural)
        {
            return note_frequencies[5][octave-1];
        }
        {
            return note_frequencies[6][octave-1];
        }
    }
    else if(letter == 'g')
    {
        if(semitone == NoteSemitone::none || semitone == NoteSemitone::natural)
        {
            return note_frequencies[7][octave-1];
        }
        else if (semitone == NoteSemitone::sharp)
        {
            return note_frequencies[8][octave-1];
        }
    }
    else if(letter == 'a')
    {
        if(semitone == NoteSemitone::none || semitone == NoteSemitone::natural)
        {
            return note_frequencies[9][octave-1];
        }
        else if (semitone == NoteSemitone::sharp)
        {
            return note_frequencies[10][octave-1];
        }
    }
    else if(letter == 'b')
    {
        if(semitone == NoteSemitone::none || semitone == NoteSemitone::natural)
        {
            return note_frequencies[11][octave-1];
        }
    }
    
    return note_frequencies[0][2];
}

F64
note_get_time(NoteLength length, F64 seconds_per_quarter_note)
{
    if(length == NoteLength::eighth)
    {
        return 0.5*seconds_per_quarter_note;
    }
    if(length == NoteLength::quarter)
    {
        return 1*seconds_per_quarter_note;
    }
    else if(length == NoteLength::three_eighths)
    {
        return 1.5*seconds_per_quarter_note;
    }
    else if(length == NoteLength::half)
    {
        return 2*seconds_per_quarter_note;
    }
    else if(length == NoteLength::three_quarters)
    {
        return 3*seconds_per_quarter_note;
    }
    else if(length == NoteLength::whole)
    {
        return 4*seconds_per_quarter_note;
    }
    return 1*seconds_per_quarter_note;
}

SynthSongWaveData
convert_song_to_wave_data(SynthSong *song)
{
    SynthSongWaveData song_wave_data = {0};
    
    F64 time_so_far = 0;
    for(Int a = 0; a < song->hands.length && a < MAX_HANDS_PER_SONG; a++)
    {
        Hand *hand = &song->hands[a];
        
        HandWaveData new_hand_wave_data = {0};
        
        for(Int i = 0; i < hand->note_groups.length; i++)
        {
            NoteGroup *group = &hand->note_groups[i];
            NoteWaveDataGroup new_group = {0};
            
            new_group.time = note_get_time(group->length, song->seconds_per_quarter_note);
            
            
            for(Int j = 0; j < group->notes.length; j++)
            {
                Note note = group->notes[j];
                NoteWaveData new_note_wave_data = {0};
                
                new_note_wave_data.frequency = note_get_frequency(note.letter, note.semitone, note.octave);
                
                //hand_wave_data->notes.push(wave_data);
                new_group.notes.push(new_note_wave_data);
            }
            new_hand_wave_data.note_groups.push(new_group);
        }
        song_wave_data.hands.push(new_hand_wave_data);
    }
    
    return song_wave_data;
}


Void
song_convert_flats_to_sharps(SynthSong *song)
{
    for(Int i = 0; i < song->hands.length && i < MAX_HANDS_PER_SONG; i++)
    {
        Hand *hand = &song->hands[i];
        
        for(Int j = 0; j < hand->note_groups.length; j++)
        {
            NoteGroup *group = &hand->note_groups[j];
            
            for(Int k = 0; k < group->notes.length; k++)
            {
                Note *note = &group->notes[k];
                
                if(note->letter == 'd' && note->semitone == NoteSemitone::flat)
                {
                    note->letter = 'c';
                    note->semitone = NoteSemitone::sharp;
                }
                else if(note->letter == 'e' && note->semitone == NoteSemitone::flat)
                {
                    note->letter = 'e';
                    note->semitone = NoteSemitone::sharp;
                }
                else if(note->letter == 'g' && note->semitone == NoteSemitone::flat)
                {
                    note->letter = 'f';
                    note->semitone = NoteSemitone::sharp;
                }
                else if(note->letter == 'a' && note->semitone == NoteSemitone::flat)
                {
                    note->letter = 'g';
                    note->semitone = NoteSemitone::sharp;
                }
                else if(note->letter == 'b' && note->semitone == NoteSemitone::flat)
                {
                    note->letter = 'a';
                    note->semitone = NoteSemitone::sharp;
                }
            }
        }
    }
}

SynthSong
song_ode_to_joy()
{
    SynthSong song = {0};
    
    song.seconds_per_quarter_note = 0.5;
    
    // Hand 1
    song.hands.push({0});
    UInt octave = 4;
    
    song.push_rest(0, NoteLength::quarter);
    
    song.push_note(0, NoteLength::quarter, create_note('f', octave, NoteSemitone::none));
    song.push_note(0, NoteLength::quarter, create_note('f', octave, NoteSemitone::none));
    song.push_note(0, NoteLength::quarter, create_note('g', octave, NoteSemitone::none));
    song.push_note(0, NoteLength::quarter, create_note('a', octave, NoteSemitone::none));
    song.push_note(0, NoteLength::quarter, create_note('a', octave, NoteSemitone::none));
    song.push_note(0, NoteLength::quarter, create_note('g', octave, NoteSemitone::none));
    song.push_note(0, NoteLength::quarter, create_note('f', octave, NoteSemitone::none));
    song.push_note(0, NoteLength::quarter, create_note('e', octave, NoteSemitone::none));
    song.push_note(0, NoteLength::quarter, create_note('d', octave, NoteSemitone::none));
    song.push_note(0, NoteLength::quarter, create_note('d', octave, NoteSemitone::none));
    song.push_note(0, NoteLength::quarter, create_note('e', octave, NoteSemitone::none));
    song.push_note(0, NoteLength::quarter, create_note('f', octave, NoteSemitone::none));
    song.push_note(0, NoteLength::three_eighths, create_note('f', octave, NoteSemitone::none));
    song.push_note(0, NoteLength::eighth, create_note('e', octave, NoteSemitone::none));
    song.push_note(0, NoteLength::half, create_note('e', octave, NoteSemitone::none));
#if 0
    
    song.push_note(0, create_note('f', octave, NoteSemitone::none, NoteLength::quarter));
    song.push_note(0, create_note('f', octave, NoteSemitone::none, NoteLength::quarter));
    song.push_note(0, create_note('g', octave, NoteSemitone::none, NoteLength::quarter));
    song.push_note(0, create_note('a', octave, NoteSemitone::none, NoteLength::quarter));
    song.push_note(0, create_note('a', octave, NoteSemitone::none, NoteLength::quarter));
    song.push_note(0, create_note('g', octave, NoteSemitone::none, NoteLength::quarter));
    song.push_note(0, create_note('f', octave, NoteSemitone::none, NoteLength::quarter));
    song.push_note(0, create_note('e', octave, NoteSemitone::none, NoteLength::quarter));
    song.push_note(0, create_note('d', octave, NoteSemitone::none, NoteLength::quarter));
    song.push_note(0, create_note('d', octave, NoteSemitone::none, NoteLength::quarter));
    song.push_note(0, create_note('e', octave, NoteSemitone::none, NoteLength::quarter));
    song.push_note(0, create_note('f', octave, NoteSemitone::none, NoteLength::quarter));
    song.push_note(0, create_note('e', octave, NoteSemitone::none, NoteLength::three_eighths));
    song.push_note(0, create_note('d', octave, NoteSemitone::none, NoteLength::eighth));
    song.push_note(0, create_note('d', octave, NoteSemitone::none, NoteLength::half));
    
    song.push_note(0, create_note('e', octave, NoteSemitone::none, NoteLength::quarter));
    song.push_note(0, create_note('e', octave, NoteSemitone::none, NoteLength::quarter));
    song.push_note(0, create_note('f', octave, NoteSemitone::none, NoteLength::quarter));
    song.push_note(0, create_note('d', octave, NoteSemitone::none, NoteLength::quarter));
    song.push_note(0, create_note('e', octave, NoteSemitone::none, NoteLength::quarter));
    song.push_note(0, create_note('f', octave, NoteSemitone::none, NoteLength::eighth));
    song.push_note(0, create_note('g', octave, NoteSemitone::none, NoteLength::eighth));
    song.push_note(0, create_note('g', octave, NoteSemitone::none, NoteLength::quarter));
    song.push_note(0, create_note('d', octave, NoteSemitone::none, NoteLength::quarter));
    song.push_note(0, create_note('e', octave, NoteSemitone::none, NoteLength::quarter));
    song.push_note(0, create_note('f', octave, NoteSemitone::none, NoteLength::eighth));
    song.push_note(0, create_note('g', octave, NoteSemitone::none, NoteLength::eighth));
    song.push_note(0, create_note('f', octave, NoteSemitone::none, NoteLength::quarter));
    song.push_note(0, create_note('e', octave, NoteSemitone::none, NoteLength::quarter));
    song.push_note(0, create_note('d', octave, NoteSemitone::none, NoteLength::quarter));
    song.push_note(0, create_note('e', octave, NoteSemitone::none, NoteLength::quarter));
    song.push_note(0, create_note('a', octave-1, NoteSemitone::none, NoteLength::quarter));
    song.push_note(0, create_note('f', octave, NoteSemitone::none, NoteLength::quarter));
    
    song.push_note(0, create_note('f', octave, NoteSemitone::none, NoteLength::quarter));
    song.push_note(0, create_note('f', octave, NoteSemitone::none, NoteLength::quarter));
    song.push_note(0, create_note('g', octave, NoteSemitone::none, NoteLength::quarter));
    song.push_note(0, create_note('a', octave, NoteSemitone::none, NoteLength::quarter));
    song.push_note(0, create_note('a', octave, NoteSemitone::none, NoteLength::quarter));
    song.push_note(0, create_note('g', octave, NoteSemitone::none, NoteLength::quarter));
    song.push_note(0, create_note('f', octave, NoteSemitone::none, NoteLength::quarter));
    song.push_note(0, create_note('e', octave, NoteSemitone::none, NoteLength::quarter));
    song.push_note(0, create_note('d', octave, NoteSemitone::none, NoteLength::quarter));
    song.push_note(0, create_note('d', octave, NoteSemitone::none, NoteLength::quarter));
    song.push_note(0, create_note('e', octave, NoteSemitone::none, NoteLength::quarter));
    song.push_note(0, create_note('f', octave, NoteSemitone::none, NoteLength::quarter));
    song.push_note(0, create_note('e', octave, NoteSemitone::none, NoteLength::three_eighths));
    song.push_note(0, create_note('d', octave, NoteSemitone::none, NoteLength::eighth));
    song.push_note(0, create_note('d', octave, NoteSemitone::none, NoteLength::half));
    
    // Hand 2
    song.hands.push({0});
    octave = octave - 1;
    
    song.push_note(1, create_rest(NoteLength::quarter));
    
    song.push_note(1, create_note('d', octave, NoteSemitone::none, NoteLength::whole));
    song.push_note(1, create_note('a', octave, NoteSemitone::none, NoteLength::three_quarters));
    song.push_note(1, create_note('a', octave, NoteSemitone::none, NoteLength::quarter));
    song.push_note(1, create_note('d', octave, NoteSemitone::none, NoteLength::half));
    song.push_note(1, create_note('a', octave, NoteSemitone::none, NoteLength::quarter));
    song.push_note(1, create_note('d', octave+1, NoteSemitone::none, NoteLength::quarter));
    song.push_note(1, create_note('a', octave, NoteSemitone::none, NoteLength::half));
    song.push_note(1, create_note('a', octave, NoteSemitone::none, NoteLength::half));
    
    song.push_note(1, create_note('d', octave, NoteSemitone::none, NoteLength::whole));
    song.push_note(1, create_note('a', octave, NoteSemitone::none, NoteLength::three_quarters));
    song.push_note(1, create_note('a', octave, NoteSemitone::none, NoteLength::quarter));
    song.push_note(1, create_note('d', octave, NoteSemitone::none, NoteLength::half));
    song.push_note(1, create_note('a', octave, NoteSemitone::none, NoteLength::quarter));
    song.push_note(1, create_note('d', octave+1, NoteSemitone::none, NoteLength::quarter));
    song.push_note(1, create_note('a', octave, NoteSemitone::none, NoteLength::half));
    song.push_note(1, create_note('d', octave, NoteSemitone::none, NoteLength::half));
    
    song.push_note(1, create_note('a', octave, NoteSemitone::none, NoteLength::half));
    song.push_note(1, create_note('a', octave, NoteSemitone::none, NoteLength::half));
    song.push_note(1, create_note('a', octave, NoteSemitone::none, NoteLength::half));
    song.push_note(1, create_note('a', octave, NoteSemitone::none, NoteLength::half));
    song.push_note(1, create_note('a', octave, NoteSemitone::none, NoteLength::half));
    song.push_note(1, create_note('a', octave, NoteSemitone::sharp, NoteLength::half));
    song.push_note(1, create_note('b', octave, NoteSemitone::none, NoteLength::quarter));
    song.push_note(1, create_note('g', octave, NoteSemitone::sharp, NoteLength::quarter));
    song.push_note(1, create_note('a', octave, NoteSemitone::none, NoteLength::half));
    
    song.push_note(1, create_note('d', octave+1, NoteSemitone::none, NoteLength::half));
    song.push_note(1, create_note('c', octave+1, NoteSemitone::natural, NoteLength::half));
    song.push_note(1, create_note('b', octave, NoteSemitone::none, NoteLength::half));
    song.push_note(1, create_note('g', octave, NoteSemitone::none, NoteLength::half));
    song.push_note(1, create_note('f', octave, NoteSemitone::none, NoteLength::quarter));
    song.push_note(1, create_note('f', octave, NoteSemitone::none, NoteLength::quarter));
    song.push_note(1, create_note('a', octave, NoteSemitone::none, NoteLength::quarter));
    song.push_note(1, create_note('d', octave+1, NoteSemitone::none, NoteLength::quarter));
    song.push_note(1, create_note('a', octave, NoteSemitone::none, NoteLength::half));
    song.push_note(1, create_note('d', octave, NoteSemitone::none, NoteLength::half));
#endif
    
    //song.push_note(1, create_note('', octave, NoteSemitone::none, NoteLength::quarter));
    
    
    for(Int i = 0; i < song.hands.length && i < MAX_HANDS_PER_SONG; i++)
    {
        Hand *hand = &song.hands[i];
        
        for(Int j = 0; j < hand->note_groups.length; j++)
        {
            NoteGroup *group = &hand->note_groups[j];
            
            for(Int k = 0; k < group->notes.length; k++)
            {
                Note *note = &group->notes[k];
                if((note->letter == 'c' || note->letter == 'f') && note->semitone != NoteSemitone::natural)
                    note->semitone = NoteSemitone::sharp;
            }
        }
    }
    
#if 0
    for(Int a = 0; a < song.hands.length && a < MAX_HANDS_PER_SONG; a++)
    {
        for(Int i = 0; i < song.hands[a].notes.length; i++)
        {
            Note *note = &song.hands[a].notes[i];
            if((note->letter == 'c' || note->letter == 'f') && note->semitone != NoteSemitone::natural)
                note->semitone = NoteSemitone::sharp;
            
            //log("%c%c", note->letter, note->semitone == NoteSemitone::sharp ? '#' : ' ');
        }
    }
#endif
    
    return song;
}
