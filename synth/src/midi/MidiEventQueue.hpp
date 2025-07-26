#ifndef __MIDI_EVENT_QUEUE_HPP
#define __MIDI_EVENT_QUEUE_HPP

#include <array>
#include <atomic>
#include "midi/MidiNote.hpp"

struct MidiEvent {
    enum class Type { NotePressed, NoteReleased, NoteOff};
    Type type ;
    ActiveNote anote ;
    bool rePressed = false ;
};

using ActiveNoteMap = RTMap<uint8_t, ActiveNote, 128> ;

class MidiEventQueue {
private:
    std::array<MidiEvent, 128> buffer_ ;
    std::atomic<size_t> head_{0};
    std::atomic<size_t> tail_{0};

public:
    bool push(const MidiEvent& event){
        size_t head = head_.load(std::memory_order_relaxed);
        size_t next = ( head + 1 ) % 128 ;
        if ( next == tail_.load(std::memory_order_acquire)) return false ;
        buffer_[head] = event ;
        head_.store(next, std::memory_order_release);
        return true ;
    }

    bool pop( MidiEvent& outEvent ){
        size_t tail = tail_.load(std::memory_order_relaxed);
        if ( tail == head_.load(std::memory_order_acquire)) return false ;
        outEvent = buffer_[tail];
        tail_.store((tail + 1) % 128, std::memory_order_release);
        return true ;
    }
};

#endif // __MIDI_EVENT_QUEUE_HPP