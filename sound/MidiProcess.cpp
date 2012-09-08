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

#include "MidiProcess.h"
#include "misc/Debug.h"
#include "PortableSoundDriver.h"
#include <QDateTime.h>
#include <windows.h>
#include "Midi.h"

namespace Rosegarden
{

MidiThread::MidiThread(std::string name, // for diagnostics
                       SoundDriver *driver,
                       unsigned int sampleRate):
                        AudioThread(name, driver, sampleRate),
                        m_fetchBufferSize(256),
                        m_currentRtOutPort(0)
{
    // Initialise the RingBuffers
    //
    m_outBuffer = new RingBuffer<MappedEvent>(1024);
    m_inBuffer = new RingBuffer<MappedEvent>(1024);

    // Local fetch buffer
    //
    m_fetchBuffer = new MappedEvent[m_fetchBufferSize];

    m_threadLogFile = new QFile("midiThread.txt");
    m_threadLogFile->open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);

    pthread_mutex_t initialisingMutex = PTHREAD_MUTEX_INITIALIZER;
    memcpy(&m_lock, &initialisingMutex, sizeof(pthread_mutex_t));

    pthread_cond_t initialisingCondition = PTHREAD_COND_INITIALIZER;
    memcpy(&m_condition, &initialisingCondition, sizeof(pthread_cond_t));

    logMsg("MidiThread::MidiThead - constructing");

}

MidiThread::~MidiThread()
{
    if (m_outBuffer) delete m_outBuffer;
    if (m_inBuffer) delete m_inBuffer;

    // Tidy up the log file
    //
    if (m_threadLogFile)
    {
        logMsg("MidiThread::MidiThead - destructing");
        m_threadLogFile->close();
        delete m_threadLogFile;
    }

    if (m_fetchBuffer) delete m_fetchBuffer;
}

void
MidiThread::threadRun()
{
    logMsg("MidiThread::threadRun() - starting to run");

    // Initialise MIDI IN from here
    //
    initialiseMidiIn(0);

    // Create a loop for the playing and read of MIDI data
    //
    while (!m_exiting)
    {
        // Wait time for this loop - in 50 microsecond sleeps
        //
        RealTime t = RealTime(0, 50000); // 50us MIDI timing

        // With gettimeofday
        //
        t = t + getTimeOfDay();

        // With getSystemTime
        //
        //t = t + PortableSoundDriver::getSystemTime();

        struct timespec timeout;
        timeout.tv_sec = t.sec;
        timeout.tv_nsec = t.nsec;
        pthread_cond_timedwait(&m_condition, &m_lock, &timeout);
        pthread_testcancel();

        processBuffers();
    }
}

// Initialise MIDI IN to a specific RtMidi port
//
void MidiThread::initialiseMidiIn(unsigned int port)
{
    // Create the MIDI in port
    //
    RtMidiIn *midiIn = ((PortableSoundDriver*)(m_driver))->getRtMidiIn(port);

    try
    {
        // Open the input port
        //
        midiIn->openPort(port);

        // Set our callback function.  This should be done immediately after
        // opening the port to avoid having incoming messages written to the
        // queue.
        midiIn->setCallback(&MidiThread::midiInCallback);

        // Don't ignore sysex, timing, or active sensing messages.
        //
        midiIn->ignoreTypes( false, false, false );
    }
    catch ( RtError &error )
    {
        logMsg(error.getMessage());
    }
}


RealTime MidiThread::getTimeOfDay()
{
    struct timeval now;
    gettimeofday(&now, 0);
    return RealTime(now.tv_sec, now.tv_usec * 1000);;
}

void
MidiThread::processBuffers()
{
    //logMsg("MidiThread::processBuffers");
    memset(m_fetchBuffer, 0, m_fetchBufferSize);
    size_t actual = m_outBuffer->read(m_fetchBuffer, m_fetchBufferSize);

    // Don't do anything if there's no events to process
    //
    if (actual == 0) return;

    // Get lock
    tryLock();

    for(unsigned int i = 0; i < actual; i++)
    {
        m_midiOutList.insert(new MappedEvent(m_fetchBuffer[i]));
    }

    releaseLock();

    // If we've just got one event - test it to see if it's a control event
    //
    if (actual == 1)
    {
        MappedEvent *mE = m_fetchBuffer;

        if ( mE->getType() == MappedEvent::SystemMIDISyncAuto && mE->getInstrument() == 255)
        {
            logMsg("MidiThread::processBuffers - got synchronisation event");

            // Reset the start time of this thread.
            m_startTime = mE->getEventTime();
        }
    }

    QString bufMsg = QString("MidiThread::processBuffers- buffering %1 events").arg(actual);
    logMsg(bufMsg.toStdString());
    bufferMidiOut();

}

// Do some logging to file - keeps it simpler to see what's happening on the MIDI thread.
//
void MidiThread::logMsg(const std::string &message)
{
    // Write to file
    //
    QTextStream out(m_threadLogFile);

    QString dT = QDateTime::currentDateTime().date().toString() + " - " +
                 QDateTime::currentDateTime().time().toString();

    out << dT << " - " << QString(message.c_str()) << endl;
    m_threadLogFile->flush();

    // Write to DEBUG
    //
    SEQUENCER_DEBUG << message;

}

void MidiThread::clearBuffersOut()
{
    logMsg("MidiThread::clearBuffersOut");

    // Get a lock
    //
    getLock();

    // Firstly stop playing everything and clear down the note off queue
    //
    processNotesOff(true);

    // Now empty the output pending buffer - the clear does the delete for us
    //
    m_midiOutList.clear();

    // Release lock
    releaseLock();
}

// The RTMidi in callback
//
void MidiThread::midiInCallback(double deltatime, std::vector< unsigned char > *message, void *userData)
{
    std::cout << "MidiThread::midiInCallback - called" << std::endl;

    unsigned int nBytes = message->size();
    QString msg;

    for ( unsigned int i=0; i<nBytes; i++ )
    {
      msg = QString("GOT MIDI IN Byte %1 = %2").arg(i).arg((int)message->at(i));
      std::cout << msg.toStdString() << std::endl;


    }

    if ( nBytes > 0 )
    {
      msg = QString("GOT MIDI IN stamp = %1").arg(deltatime);
      std::cout << msg.toStdString() << std::endl;
    }
}

// Process any pending note offs
//
void MidiThread::processNotesOff(bool everything)
{
    logMsg("MidiThread::processNotesOff");

    if (m_noteOffQueue.empty()) {
        return;
    }

    RealTime systemTime = getTimeOfDay();
    NoteOffQueue deleteQueue;
    MappedInstrument *instrument;

    for (NoteOffQueue::iterator it = m_noteOffQueue.begin();
         it != m_noteOffQueue.end(); it++)
    {
        if ((*it)->getRealTime() <= systemTime || everything)
        {
            // We can note off everything from here
            //
            try
            {
                // Get the RtMidi port
                //
                int rtPort = ((PortableSoundDriver*)(m_driver))->
                             getOutputPortForMappedInstrument((*it)->getInstrument());
                if (rtPort < 0)
                {
                    QString outMsg = QString("MidiThread::bufferMidiOut - no RtMidi port found");
                    logMsg(outMsg.toStdString());
                    continue;
                }

                instrument = ((PortableSoundDriver*)(m_driver))->getMappedInstrument((*it)->getInstrument());

                std::vector<unsigned char> message;
                message.push_back(MIDI_NOTE_OFF /* + instrument->getChannel() */);
                message.push_back((*it)->getPitch());
                ((PortableSoundDriver*)(m_driver))->getRtMidiOut(rtPort)->sendMessage(&message);

                QString msg = QString("processNotesOff:MidiNoteOff note = %1").arg((int)(*it)->getPitch());

                logMsg(msg.toStdString());

                deleteQueue.insert(*it);
            }
            catch ( RtError &error )
            {
                logMsg(error.getMessage());
            }
        }
    }

    // Remove the ones we've sent
    //
    for (NoteOffQueue::iterator it = deleteQueue.begin();
         it != deleteQueue.end(); it++)
    {
        //delete (*it);
        m_noteOffQueue.erase(*it);
    }
}


void MidiThread::bufferMidiOut()
{
    MappedInstrument *instrument;
    std::vector<MappedEvent*> removeList;
    RealTime outputStopTime;
    MidiByte channel;

    // Get a lock on this before doing anything
    //
    getLock();

    QString outMsg = QString("MidiThread::bufferMidiOut - currently queued %1 events").arg(m_midiOutList.size());
    logMsg(outMsg.toStdString());

    /*
    if ((mC.begin() != mC.end())) {
        SequencerDataBlock::getInstance()->setVisual(*mC.begin());
    }
    */

    // Process note offs
    //
    processNotesOff();

    // NB the MappedEventList is implicitly ordered by time (std::multiset)
    //
    for (MappedEventList::const_iterator i = m_midiOutList.begin(); i != m_midiOutList.end(); ++i) {

        if ((*i)->getType() >= MappedEvent::Audio)
            continue;

        bool isControllerOut = ((*i)->getRecordedDevice() ==
                                Device::CONTROL_DEVICE);

        //bool isSoftSynth = (!isControllerOut &&
        //                    ((*i)->getInstrument() >= SoftSynthInstrumentBase));

        // Now add the event time, take away the start pointer position and add the
        // system starting time.  This will tell us how from the startTime (system time)
        // we have to output this event.  If our current time is after then

        RealTime outputTime = (*i)->getEventTime() - m_driver->getStartPosition() +  m_startTime;

        instrument = ((PortableSoundDriver*)(m_driver))->getMappedInstrument((*i)->getInstrument());

        bool needNoteOff = false;

        if (isControllerOut) {
            channel = (*i)->getRecordedChannel();
        } else if (instrument != 0) {
            channel = (*i)->getRecordedChannel();
            //instrument->getChannel();
            //channel = 0;
        } else {
            channel = 0;
        }

        outMsg = QString("MidiThread::bufferMidiOut - looking for instrument %1").arg((*i)->getInstrument());
        logMsg(outMsg.toStdString());

        int rtPort = ((PortableSoundDriver*)(m_driver))->
                     getOutputPortForMappedInstrument((*i)->getInstrument());
        if (rtPort < 0)
        {
            outMsg = QString("MidiThread::bufferMidiOut - no RtMidi port found");
            logMsg(outMsg.toStdString());
            continue;
        }

        // Check to see if the port exists
        //
        ((PortableSoundDriver*)(m_driver))->checkRtMidiOut(rtPort);


        outMsg = QString("MidiThread::bufferMidiOut - RtMidi port USING - %1 - %2").arg(rtPort).arg(QString(((PortableSoundDriver*)(m_driver))->getRtMidiOut(rtPort)->getPortName(rtPort).c_str()));
        logMsg(outMsg.toStdString());

        // Set the output port here
        //
        ((PortableSoundDriver*)(m_driver))->getRtMidiOut(rtPort)->openPort(rtPort);

        if (getTimeOfDay() >= outputTime)
        {
            QString outMsg = QString("Writing out MIDI event type = %1, data = %2").
                             arg((*i)->getType()).arg((int)(*i)->getData1());
            logMsg(outMsg.toStdString());

            switch ((*i)->getType())
            {
                case MappedEvent::MidiNoteOneShot:
                    {
                        std::vector<unsigned char> message;
                        message.push_back(MIDI_NOTE_ON + channel);
                        message.push_back((*i)->getPitch());
                        message.push_back((*i)->getVelocity());

                        ((PortableSoundDriver*)(m_driver))->getRtMidiOut(rtPort)->sendMessage(&message);

                        QString msg = QString("MidiNoteOneShot note = %1, vely = %2").arg((int)(*i)->getPitch())
                                      .arg((int)(*i)->getVelocity());
                        logMsg(msg.toStdString());

                        needNoteOff = true;
                        outputStopTime = outputTime + (*i)->getDuration();
/*
                        if (!isSoftSynth) {
                            LevelInfo info;
                            info.level = (*i)->getVelocity();
                            info.levelRight = 0;
                            SequencerDataBlock::getInstance()->setInstrumentLevel
                                ((*i)->getInstrument(), info);
                        }

                        weedRecentNoteOffs((*i)->getPitch(), channel, (*i)->getInstrument());
                        */
                    }
                    break;

            case MappedEvent::MidiNote:
                // We always use plain NOTE ON here, not ALSA
                // time+duration notes, because we have our own NOTE
                // OFF stack (which will be augmented at the bottom of
                // this function) and we want to ensure it gets used
                // for the purposes of e.g. soft synths
                //
                if ((*i)->getVelocity() > 0) {
                    std::vector<unsigned char> message;
                    message.push_back(MIDI_NOTE_ON + channel);
                    message.push_back((*i)->getPitch());
                    message.push_back((*i)->getVelocity());

                    ((PortableSoundDriver*)(m_driver))->getRtMidiOut(rtPort)->sendMessage(&message);

                    QString msg = QString("MidiNote note = %1, vely = %2").arg((int)(*i)->getPitch())
                                  .arg((int)(*i)->getVelocity());
                    logMsg(msg.toStdString());

                } else {
                    std::vector<unsigned char> message;
                    message.push_back(MIDI_NOTE_OFF + channel);
                    message.push_back((*i)->getPitch());
                    message.push_back((*i)->getVelocity());

                    ((PortableSoundDriver*)(m_driver))->getRtMidiOut(rtPort)->sendMessage(&message);

                    QString msg = QString("MidiNoteOff note = %1, vely = %2").arg((int)(*i)->getPitch())
                                  .arg((int)(*i)->getVelocity());
                    logMsg(msg.toStdString());
                }

                break;

            case MappedEvent::MidiProgramChange:
                {
                    std::vector<unsigned char> message;
                    message.push_back(MIDI_PROG_CHANGE + channel);
                    message.push_back((*i)->getData1());
                    ((PortableSoundDriver*)(m_driver))->getRtMidiOut(rtPort)->sendMessage(&message);

                    QString msg = QString("MidiProgramChange PC = %1")
                                  .arg((int)(*i)->getData1());
                    logMsg(msg.toStdString());
                }
                break;


            case MappedEvent::MidiKeyPressure: // Also called MIDI_POLY_AFTERTOUCH
                {
                    std::vector<unsigned char> message;
                    message.push_back(MIDI_POLY_AFTERTOUCH + channel);
                    message.push_back((*i)->getData1());
                    message.push_back((*i)->getData2());
                    ((PortableSoundDriver*)(m_driver))->getRtMidiOut(rtPort)->sendMessage(&message);

                    QString msg = QString("MidiKeyPressure Data1 = %1, Data2 = %2")
                                  .arg((int)(*i)->getData1()).arg((int)(*i)->getData2());
                    logMsg(msg.toStdString());
                }
                break;


            case MappedEvent::MidiChannelPressure: // Also called MIDI_CHNL_AFTERTOUCH
                {
                    std::vector<unsigned char> message;
                    message.push_back(MIDI_CHNL_AFTERTOUCH + channel);
                    message.push_back((*i)->getData1());
                    ((PortableSoundDriver*)(m_driver))->getRtMidiOut(rtPort)->sendMessage(&message);

                    QString msg = QString("MidiChannelPressure Data1 = %1")
                                  .arg((int)(*i)->getData1());
                    logMsg(msg.toStdString());
                }
                break;

            case MappedEvent::MidiPitchBend:
                {
                int d1 = (int)((*i)->getData1());
                int d2 = (int)((*i)->getData2());
                //int value = ((d1 << 7) | d2) - 8192;

                // keep within -8192 to +8192
                //
                // if (value & 0x4000)
                //    value -= 0x8000;
                std::vector<unsigned char> message;
                message.push_back(MIDI_PITCH_BEND + channel);
                message.push_back(d1);
                message.push_back(d2);
                ((PortableSoundDriver*)(m_driver))->getRtMidiOut(rtPort)->sendMessage(&message);

                QString msg = QString("MidiPitchBend d1 = %1, d2 = %2")
                              .arg(d1).arg(d2);
                logMsg(msg.toStdString());

                }
                break;

            case MappedEvent::MidiSystemMessage: {
/*
                switch ((*i)->getData1()) {
                case MIDI_SYSTEM_EXCLUSIVE: {
                    char out[2];
                    sprintf(out, "%c", MIDI_SYSTEM_EXCLUSIVE);
                    std::string data = out;

                    data += DataBlockRepository::getDataBlockForEvent((*i));

                    sprintf(out, "%c", MIDI_END_OF_EXCLUSIVE);
                    data += out;

                    snd_seq_ev_set_sysex(&event,
                                         data.length(),
                                         (char*)(data.c_str()));
                }
                    break;

                case MIDI_TIMING_CLOCK: {
                    RealTime rt =
                        RealTime(time.tv_sec, time.tv_nsec);


                    sendSystemQueued(SND_SEQ_EVENT_CLOCK, "", rt);

                    continue;

                }
                    break;

                default:
                    logMsg("AlsaDriver::processMidiOut - unrecognised system message");
                    break;
                }*/
            }
                break;

            case MappedEvent::MidiController:
                {
                    std::vector<unsigned char> message;
                    message.push_back(MIDI_CTRL_CHANGE + channel);
                    message.push_back((*i)->getData1());
                    message.push_back((*i)->getData2());
                    ((PortableSoundDriver*)(m_driver))->getRtMidiOut(rtPort)->sendMessage(&message);

                    QString msg = QString("MidiController data1 = %1, data2 = %2").arg((int)(*i)->getData1())
                                  .arg((int)(*i)->getData2());
                    logMsg(msg.toStdString());

                }
                break;

            case MappedEvent::Audio:
            case MappedEvent::AudioCancel:
            case MappedEvent::AudioLevel:
            case MappedEvent::AudioStopped:
            case MappedEvent::SystemUpdateInstruments:
            case MappedEvent::SystemJackTransport:  //???
            case MappedEvent::SystemMMCTransport:
            case MappedEvent::SystemMIDIClock:
            case MappedEvent::SystemMIDISyncAuto:
                break;

            default:
            case MappedEvent::InvalidMappedEvent:
                logMsg("MappedEvent::InvalidMappedEvent");
                continue;
            }


            // now need to remove this event from the list
            //
            removeList.push_back(*i);

            // Add note to note off stack
            //
            if (needNoteOff)
            {
                NoteOffEvent *noteOffEvent =
                    new NoteOffEvent(outputStopTime,  // already calculated
                                     (*i)->getPitch(),
                                     channel,
                                     (*i)->getInstrument());

                m_noteOffQueue.insert(noteOffEvent);
            }
        }
    }

    for (std::vector<MappedEvent*>::iterator it = removeList.begin(); it < removeList.end(); it++)
    {
        for (MappedEventList::const_iterator i = m_midiOutList.begin(); i != m_midiOutList.end(); ++i)
        {
            if ((*i) == (*it))
            {
                logMsg("MidiThread::bufferMidiOut - deleting event");
                m_midiOutList.erase(i);
                break;
            }
        }

    }

    releaseLock();
}


}

