/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- vi:set ts=8 sts=4 sw=4: */

/*
    Rosegarden
    A sequencer and musical notation editor.
    Copyright 2000-2010 the Rosegarden development team.
    See the AUTHORS file for more details.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "AudioProcess.h"
#include "MappedEventList.h"
#include <string.h>

#ifndef _MIDIPROCESS_H
#define _MIDIPROCESS_H

namespace Rosegarden
{

class MidiThread : public AudioThread
{

public:
    MidiThread(std::string name, // for diagnostics
               SoundDriver *driver,
               unsigned int sampleRate);
    virtual ~MidiThread();

    void bufferMidiOut();

    // These are the two RingBuffers we use to pass MappedEvents in and out
    // of this thread.
    //
    RingBuffer<MappedEvent> *getMidiOutBuffer() { return m_outBuffer; }
    RingBuffer<MappedEvent> *getMidiInBuffer() { return m_inBuffer; }

    void logMsg(const std::string &message);

    RealTime getTimeOfDay();

    // Process note off events as we need to - if we want to force all notes off
    // then pass true to the first argument.
    //
    void processNotesOff(bool everything = false);


    // On jump - clear the out buffers
    //
    void clearBuffersOut();

    // Initialise MIDI IN to a given port and set the callback
    //
    void initialiseMidiIn(unsigned int port);

    // The RTMidi MIDI in callback
    //
    static void midiInCallback(double deltatime, std::vector< unsigned char > *message, void *userData);

protected:
    virtual void threadRun();
    void processBuffers();

    RingBuffer<MappedEvent> *m_outBuffer;
    RingBuffer<MappedEvent> *m_inBuffer;

    RealTime                 m_startTime;

    QFile                   *m_threadLogFile;

    // Locally maintained midi output list
    //
    MappedEventList          m_midiOutList;

    // Fetch buffer
    //
    MappedEvent             *m_fetchBuffer;

    unsigned int            m_fetchBufferSize;

    // MIDI Note-off handling - copy from SoundDriver
    //
    NoteOffQueue            m_noteOffQueue;

    // Keep a track of the current RtMidi output port
    //
    unsigned int            m_currentRtOutPort;
};


}


#endif // MIDIPROCESS_H
