/* date = December 13th 2024 7:34 am */

#ifndef SYNTH_SONG_H
#define SYNTH_SONG_H

#define MAX_NOTE_COUNT 100
#define MAX_HANDS_PER_SONG 10

enum class NoteSemitone
{
    none,
    natural,
    flat,
    sharp
};

enum class NoteLength
{
    eighth,
    quarter,
    three_eighths,
    half,
    three_quarters,
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

struct Hand
{
    InplaceStack<Note, MAX_NOTE_COUNT> notes;
};

struct SynthSong
{
    F64 seconds_per_quarter_note;
    InplaceStack<Hand, MAX_HANDS_PER_SONG> hands;
    
    Void push_note(Int hand, Note note);
};

struct NoteWaveData
{
    F64 hz;
    F64 time;
};

struct HandWaveData
{
    F64 x;
    F64 hz;
    InplaceStack<NoteWaveData, MAX_NOTE_COUNT> notes;
};

struct SynthSongWaveData
{
    InplaceStack<HandWaveData, MAX_HANDS_PER_SONG> hands;
};

#endif //SYNTH_SONG_H
