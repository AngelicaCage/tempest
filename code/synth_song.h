/* date = December 13th 2024 7:34 am */

#ifndef SYNTH_SONG_H
#define SYNTH_SONG_H

#define MAX_NOTE_COUNT 100

enum class NoteSemitone
{
    none,
    flat,
    sharp
};

enum class NoteLength
{
    eighth,
    quarter,
    three_eighths,
    half,
    whole,
};

struct Note
{
    Bool is_rest;
    NoteLength length;
    
    UInt octave;
    Char letter;
    NoteSemitone semitone;
};

struct SynthSong
{
    F64 seconds_per_quarter_note;
    InplaceStack<Note, MAX_NOTE_COUNT> notes;
    
    Void push_note(Note note);
};

struct NoteWaveData
{
    F64 hz;
    F64 time;
};

struct SynthSongWaveData
{
    InplaceStack<NoteWaveData, MAX_NOTE_COUNT> notes;
};

#endif //SYNTH_SONG_H
