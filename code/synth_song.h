/* date = December 13th 2024 7:34 am */

#ifndef SYNTH_SONG_H
#define SYNTH_SONG_H

#define MAX_NOTE_COUNT 100
#define MAX_HANDS_PER_SONG 10
#define NOTE_GROUP_MAX_NOTE_COUNT 5
#define MAX_NOTES_PLAYING_PER_HAND 5

enum class NoteSemitone
{
    none,
    natural,
    flat,
    sharp
};

enum class NoteLength
{
    sixteenth,
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
    Bool forms_chord_with_next;
    
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
    Void push_note(Note note);
};

struct NoteWaveData
{
    F64 frequency;
    F64 time;
    Bool forms_chord_with_next;
};

struct HandWaveData
{
    InplaceStack<NoteWaveData, MAX_NOTE_COUNT> notes;
    F64 play_xs[MAX_NOTES_PLAYING_PER_HAND];
};

struct SynthSongWaveData
{
    InplaceStack<HandWaveData, MAX_HANDS_PER_SONG> hands;
};

#endif //SYNTH_SONG_H
