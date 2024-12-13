/* date = December 13th 2024 7:34 am */

#ifndef SYNTH_SONG_H
#define SYNTH_SONG_H

#define MAX_NOTE_COUNT 100
#define MAX_HANDS_PER_SONG 10

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

struct Hand
{
    InplaceStack<Note, MAX_NOTE_COUNT> notes;
};

struct SynthSong
{
    F64 seconds_per_quarter_note;
    Int hand_count;
    Hand hands[MAX_HANDS_PER_SONG];
    
    Void push_note(Int hand, Note note);
};

struct NoteWaveData
{
    F64 hz;
    F64 time;
};

struct HandWaveData
{
    InplaceStack<NoteWaveData, MAX_NOTE_COUNT> notes;
};


struct SynthSongWaveData
{
    HandWaveData hands[MAX_HANDS_PER_SONG];
};

#endif //SYNTH_SONG_H
