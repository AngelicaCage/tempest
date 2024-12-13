/* date = December 13th 2024 7:34 am */

#ifndef SYNTH_SONG_H
#define SYNTH_SONG_H

#define MAX_NOTE_COUNT 100
#define MAX_HANDS_PER_SONG 10
#define NOTE_GROUP_MAX_NOTE_COUNT 5

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
    UInt octave;
    Char letter;
    NoteSemitone semitone;
};

struct NoteGroup
{
    Bool is_rest;
    InplaceStack<Note, NOTE_GROUP_MAX_NOTE_COUNT> notes;
    NoteLength length;
};

struct Hand
{
    InplaceStack<NoteGroup, MAX_NOTE_COUNT> note_groups;
};

struct SynthSong
{
    F64 seconds_per_quarter_note;
    InplaceStack<Hand, MAX_HANDS_PER_SONG> hands;
    
    Void push_note_group(Int hand, NoteLength length, Note note1);
    Void push_note(Int hand, NoteLength length, Note note);
    Void push_rest(Int hand, NoteLength length);
};

struct NoteWaveData
{
    F64 frequency;
    
    F64 play_x;
};
struct NoteWaveDataGroup
{
    InplaceStack<NoteWaveData, NOTE_GROUP_MAX_NOTE_COUNT> notes;
    F64 time;
};

struct HandWaveData
{
    InplaceStack<NoteWaveDataGroup, MAX_NOTE_COUNT> note_groups;
    Int play_note_group_index;
};

struct SynthSongWaveData
{
    InplaceStack<HandWaveData, MAX_HANDS_PER_SONG> hands;
};

#endif //SYNTH_SONG_H
