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

// Allocate the static return composition
//
MappedEventList *MidiThread::m_returnComposition = new MappedEventList();
pthread_mutex_t MidiThread::m_recLock  = PTHREAD_MUTEX_INITIALIZER;
//memcpy(&m_lock, &initialisingMutex, sizeof(pthread_mutex_t));

//pthread_cond_t initialisingCondition = PTHREAD_COND_INITIALIZER;
//memcpy(&m_condition, &initialisingCondition, sizeof(pthread_cond_t));

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


// Get the time of day from the system clock
//
RealTime
MidiThread::getTimeOfDay()
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

MappedEventList
MidiThread::getReturnComposition()
{
    // Might need to get another mutex here
    //
    MappedEventList mE;

    pthread_mutex_lock(&MidiThread::m_recLock);

    for (MappedEventListIterator it = MidiThread::m_returnComposition->begin(); it != MidiThread::m_returnComposition->end(); it++)
    {
        mE.insert(new MappedEvent(*it));
    }

    // clear the local composition
    //
    MidiThread::m_returnComposition->clear();

    pthread_mutex_unlock(&MidiThread::m_recLock);

    QString thing = QString("GetReturnComposition size = %1").arg(mE.size());

    SEQUENCER_DEBUG << thing.toStdString() << endl;

    // return our local copy
    //
    return mE;
}

// The RTMidi in callback - we use this to identify and push events to the m_returnComposition
// which can then be queried and copied back into the driver on request.
//
void
MidiThread::midiInCallback(double deltatime, std::vector< unsigned char > *message, void *userData)
{
    std::cout << "MidiThread::midiInCallback - called" << std::endl;

    unsigned int nBytes = message->size();

    // Do something if there is nothing
    //
    if (nBytes == 0){
        return;
    }

    QString msg;

    for ( unsigned int i=0; i < nBytes; i++ )
    {
      msg = QString("GOT MIDI IN Byte %1 = %2").arg(i).arg((int)message->at(i));
      std::cout << msg.toStdString() << std::endl;
    }

    if ( nBytes > 0 )
    {
      msg = QString("GOT MIDI IN stamp = %1").arg(deltatime);
      std::cout << msg.toStdString() << std::endl;
    }

    //m_returnComposition

    // ----------------------------------------------------
    //
    RealTime eventTime(0, 0);

    // Get a lock for this method
    //
    //getLock();
    pthread_mutex_lock(&MidiThread::m_recLock);

    switch (message->at(0))
    {
    case MIDI_NOTE_ON:
        if (message->at(2) > 0)
        {
        MappedEvent *mE = new MappedEvent();
        mE->setPitch(message->at(1));
        mE->setVelocity(message->at(2));
        mE->setEventTime(eventTime);
        //mE->setRecordedChannel(channel);
        //mE->setRecordedDevice(deviceId);

        // Negative duration - we need to hear the NOTE ON
        // so we must insert it now with a negative duration
        // and pick and mix against the following NOTE OFF
        // when we create the recorded segment.
        //
        mE->setDuration(RealTime( -1, 0));

        // Create a copy of this when we insert the NOTE ON -
        // keeping a copy alive on the m_noteOnMap.
        //
        // We shake out the two NOTE Ons after we've recorded
        // them.
        //
        MidiThread::m_returnComposition->insert(mE);
//        m_noteOnMap[deviceId].insert(std::pair<unsigned int, MappedEvent*>(chanNoteKey, mE));
        }

        break;

    default:
        break;
    }


    //    std::cerr << "AlsaDriver::getMappedEventList: looking for events" << std::endl;

    //snd_seq_event_t *event;
/*
    while (snd_seq_event_input(m_midiHandle, &event) > 0) {
        //        std::cerr << "AlsaDriver::getMappedEventList: found something" << std::endl;

        unsigned int channel = (unsigned int)event->data.note.channel;
        unsigned int chanNoteKey = ( channel << 8 ) +
            (unsigned int) event->data.note.note;

        bool fromController = false;

        if (event->dest.client == m_client &&
            event->dest.port == m_controllerPort) {
#ifdef DEBUG_ALSA
            std::cerr << "Received an external controller event" << std::endl;
#endif

            fromController = true;
        }

        unsigned int deviceId = Device::NO_DEVICE;

        if (fromController) {
            deviceId = Device::CONTROL_DEVICE;
        } else {
            for (MappedDeviceList::iterator i = m_devices.begin();
                 i != m_devices.end(); ++i) {
                ClientPortPair pair(m_devicePortMap[(*i)->getId()]);
                if (((*i)->getDirection() == MidiDevice::Record) &&
                    ( pair.first == event->source.client ) &&
                    ( pair.second == event->source.port )) {
                    deviceId = (*i)->getId();
                    break;
                }
            }
        }

        eventTime.sec = event->time.time.tv_sec;
        eventTime.nsec = event->time.time.tv_nsec;
        eventTime = eventTime - m_alsaRecordStartTime + m_playStartPosition;

#ifdef DEBUG_ALSA
        if (!fromController) {
            std::cerr << "Received normal event: type " << int(event->type) << ", chan " << channel << ", note " << int(event->data.note.note) << ", time " << eventTime << std::endl;
        }
#endif
*/

    /*
    switch (message->at(0)) {
        case MIDI_NOTE_ON:
            //if (fromController)
                //continue;
            if (event->data.note.velocity > 0) {
                MappedEvent *mE = new MappedEvent();
                mE->setPitch(event->data.note.note);
                mE->setVelocity(event->data.note.velocity);
                mE->setEventTime(eventTime);
                mE->setRecordedChannel(channel);
                mE->setRecordedDevice(deviceId);

                // Negative duration - we need to hear the NOTE ON
                // so we must insert it now with a negative duration
                // and pick and mix against the following NOTE OFF
                // when we create the recorded segment.
                //
                mE->setDuration(RealTime( -1, 0));

                // Create a copy of this when we insert the NOTE ON -
                // keeping a copy alive on the m_noteOnMap.
                //
                // We shake out the two NOTE Ons after we've recorded
                // them.
                //
                composition.insert(new MappedEvent(mE));
                m_noteOnMap[deviceId].insert(std::pair<unsigned int, MappedEvent*>(chanNoteKey, mE));

                break;
            }

        case SND_SEQ_EVENT_NOTEOFF: {
            if (fromController)
                continue;

            // Check the note on map for any note on events to close.
//            std::map<unsigned int, std::multimap<unsigned int, MappedEvent*> >::iterator noteOnMapIt = m_noteOnMap.find(deviceId);
            std::multimap<unsigned int, MappedEvent*>::iterator noteOnIt = m_noteOnMap[deviceId].find(chanNoteKey);

            if (noteOnIt != m_noteOnMap[deviceId].end()) {

                // Set duration correctly on the NOTE OFF
                //
                MappedEvent *mE = noteOnIt->second;
                RealTime duration = eventTime - mE->getEventTime();

#ifdef DEBUG_ALSA
                std::cerr << "NOTE OFF: found NOTE ON at " << mE->getEventTime() << std::endl;
#endif

                if (duration <= RealTime::zeroTime) {
                    duration = RealTime::fromMilliseconds(1); // Fix zero duration record bug.
                    mE->setEventTime(eventTime);
                }

                // Velocity 0 - NOTE OFF.  Set duration correctly
                // for recovery later.
                //
                mE->setVelocity(0);
                mE->setDuration(duration);

                // force shut off of note
                composition.insert(mE);

                // reset the reference
                //
                m_noteOnMap[deviceId].erase(noteOnIt);

            }
        }
            break;

        case SND_SEQ_EVENT_KEYPRESS: {
            if (fromController)
                continue;

            // Fix for 632964 by Pedro Lopez-Cabanillas (20030523)
            //
            MappedEvent *mE = new MappedEvent();
            mE->setType(MappedEvent::MidiKeyPressure);
            mE->setEventTime(eventTime);
            mE->setData1(event->data.note.note);
            mE->setData2(event->data.note.velocity);
            mE->setRecordedChannel(channel);
            mE->setRecordedDevice(deviceId);
            composition.insert(mE);
        }
            break;

        case SND_SEQ_EVENT_CONTROLLER: {
            MappedEvent *mE = new MappedEvent();
            mE->setType(MappedEvent::MidiController);
            mE->setEventTime(eventTime);
            mE->setData1(event->data.control.param);
            mE->setData2(event->data.control.value);
            mE->setRecordedChannel(channel);
            mE->setRecordedDevice(deviceId);
            composition.insert(mE);
        }
            break;

        case SND_SEQ_EVENT_PGMCHANGE: {
            MappedEvent *mE = new MappedEvent();
            mE->setType(MappedEvent::MidiProgramChange);
            mE->setEventTime(eventTime);
            mE->setData1(event->data.control.value);
            mE->setRecordedChannel(channel);
            mE->setRecordedDevice(deviceId);
            composition.insert(mE);

        }
            break;

        case SND_SEQ_EVENT_PITCHBEND: {
            if (fromController)
                continue;

            // Fix for 711889 by Pedro Lopez-Cabanillas (20030523)
            //
            int s = event->data.control.value + 8192;
            int d1 = (s >> 7) & 0x7f; // data1 = MSB
            int d2 = s & 0x7f; // data2 = LSB
            MappedEvent *mE = new MappedEvent();
            mE->setType(MappedEvent::MidiPitchBend);
            mE->setEventTime(eventTime);
            mE->setData1(d1);
            mE->setData2(d2);
            mE->setRecordedChannel(channel);
            mE->setRecordedDevice(deviceId);
            composition.insert(mE);
        }
            break;

        case SND_SEQ_EVENT_CHANPRESS: {
            if (fromController)
                continue;

            // Fixed by Pedro Lopez-Cabanillas (20030523)
            //
            int s = event->data.control.value & 0x7f;
            MappedEvent *mE = new MappedEvent();
            mE->setType(MappedEvent::MidiChannelPressure);
            mE->setEventTime(eventTime);
            mE->setData1(s);
            mE->setRecordedChannel(channel);
            mE->setRecordedDevice(deviceId);
            composition.insert(mE);
        }
            break;

        case SND_SEQ_EVENT_SYSEX:

            if (fromController)
                continue;

            if (!testForMTCSysex(event) &&
                !testForMMCSysex(event)) {

                // Bundle up the data into a block on the MappedEvent
                //
                std::string data;
                char *ptr = (char*)(event->data.ext.ptr);
                for (unsigned int i = 0; i < event->data.ext.len; ++i)
                    data += *(ptr++);

#ifdef DEBUG_ALSA

                if ((MidiByte)(data[1]) == MIDI_SYSEX_RT) {
                    std::cerr << "REALTIME SYSEX" << endl;
                    for (unsigned int ii = 0; ii < event->data.ext.len; ++ii) {
                        printf("B %d = %02x\n", ii, ((char*)(event->data.ext.ptr))[ii]);
                    }
                } else {
                    std::cerr << "NON-REALTIME SYSEX" << endl;
                    for (unsigned int ii = 0; ii < event->data.ext.len; ++ii) {
                        printf("B %d = %02x\n", ii, ((char*)(event->data.ext.ptr))[ii]);
                    }
                }
#endif

                // Thank you to Christoph Eckert for pointing out via
                // Pedro Lopez-Cabanillas aseqmm code that we need to pool
                // alsa system execlusive messages since they may be broken
                // across several ALSA mesages.

                // Unfortunately, pooling these messages get very complicated
                // since it creates many corner cases during this realtime
                // activity that may involve possible bad data transmissions.

                bool beginNewMessage = false;
                if (data.length() > 0) {
                    // Check if at start of MIDI message
                    if (MidiByte(data.at(0)) == MIDI_SYSTEM_EXCLUSIVE) {
                        data.erase(0,1); // Skip (SYX). RG doesn't use it.
                        beginNewMessage = true;
                    }
                }

                std::string sysExcData;
                MappedEvent *sysExcEvent = 0;

                // Check to see if there are any pending System Exclusive Messages
                if (!m_pendSysExcMap->empty()) {
                    // Check our map to see if we have a pending operations for
                    // the current deviceId.
                    DeviceEventMap::iterator pendIt = m_pendSysExcMap->find(deviceId);

                    if (pendIt != m_pendSysExcMap->end()) {
                        sysExcEvent = pendIt->second.first;
                        sysExcData = pendIt->second.second;

                        // Be optimistic that we won't have to re-add this afterwards.
                        // Also makes keeping track of this easier.
                        m_pendSysExcMap->erase(pendIt);
                    }
                }

                bool createNewEvent = false;
                if (!sysExcEvent) {
                    // Did not find a pending (unfinished) System Exclusive message.
                    // Create a new event.
                    createNewEvent = true;

                    if (!beginNewMessage) {
                        std::cerr << "AlsaDriver::getMappedEventList - "
                                  << "New ALSA message arrived with incorrect MIDI System "
                                  << "Exclusive start byte" << std::endl
                                  << "This is probably a bad transmission" << std::endl;
                    }
                } else {
                    // We found a pending (unfinished) System Exclusive message.

                    // Check if at start of MIDI message
                    if (!beginNewMessage) {
                        // Prepend pooled events to the current message data

                        if (sysExcData.size() > 0) {
                            data.insert(0, sysExcData);
                        }
                    } else {
                        // This is the start of a new message but have
                        // pending (incomplete) messages already.
                        createNewEvent = true;

                        // Decide how to handle previous (incomplete) message
                        if (sysExcData.size() > 0) {
                            std::cerr << "AlsaDriver::getMappedEventList - "
                                      << "Sending an incomplete ALSA message to the composition"
                                      << std::endl  << "This is probably a bad transmission"
                                      << std::endl;

                            // Push previous (incomplete) message to composition
                            DataBlockRepository::setDataBlockForEvent(sysExcEvent, sysExcData);
                            composition.insert(sysExcEvent);
                        } else {
                            // Previous message has no meaningful data.
                            std::cerr << "AlsaDriver::getMappedEventList - "
                                      << "Discarding meaningless incomplete ALSA message"
                                      << std::endl;

                            delete sysExcEvent;
                        }
                    }
                }

                if (createNewEvent) {
                    // Still need a current event to work with.  Create it.
                    sysExcEvent = new MappedEvent();
                    sysExcEvent->setType(MappedEvent::MidiSystemMessage);
                    sysExcEvent->setData1(MIDI_SYSTEM_EXCLUSIVE);
                    sysExcEvent->setRecordedDevice(deviceId);
                    sysExcEvent->setEventTime(eventTime);
                }

                // We need to check to see if this event completes the
                // System Exclusive event.

                bool pushOnMap = false;
                if (!data.empty()) {
                    int lastChar = data.size() - 1;

                    // Check to see if we are at the end of a message.
                    if (MidiByte(data.at(lastChar)) == MIDI_END_OF_EXCLUSIVE) {
                        // Remove (EOX). RG doesn't use it.
                        data.erase(lastChar);

                        // Push message to composition
                        DataBlockRepository::setDataBlockForEvent(sysExcEvent, data);
                        composition.insert(sysExcEvent);
                    } else {

                        pushOnMap = true;
                    }
                } else {
                    // Data is empty.  Anyway we got here we need to put it back
                    // in the pending map.  This will resolve itself elsewhere.
                    // But if we are here, this is probably and error.

                    std::cerr << "AlsaDriver::getMappedEventList - "
                              << " ALSA message arrived with no useful System Exclusive"
                              << "data bytes" << std::endl
                              << "This is probably a bad transmission" << std::endl;

                    pushOnMap = true;
                }

                if (pushOnMap) {
                    // Put the unfinished event back in the pending map.
                    m_pendSysExcMap->insert(std::make_pair(deviceId,
                                                           std::make_pair(sysExcEvent, data)));

                    if (beginNewMessage) {
                        // Let user know about pooling on first recieved event.

                        // Yes, standard output.
                        // It is used elswhere in this file as well.
                        std::cout << "AlsaDriver::getMappedEventList - "
                                  << "Encountered long System Exclusive Message "
                                  << "(pooling message until transmission complete)"
                                  << std::endl;
                    }
                }
            }
            break;


        case SND_SEQ_EVENT_SENSING:  // MIDI device is still there
            break;

        case SND_SEQ_EVENT_QFRAME:
            if (fromController)
                continue;
            if (getMTCStatus() == TRANSPORT_SLAVE) {
                handleMTCQFrame(event->data.control.value, eventTime);
            }
            break;

        case SND_SEQ_EVENT_CLOCK:
#ifdef DEBUG_ALSA
            std::cerr << "AlsaDriver::getMappedEventList - "
                      << "got realtime MIDI clock" << std::endl;
#endif
            break;

        case SND_SEQ_EVENT_START:
            if ((getMIDISyncStatus() == TRANSPORT_SLAVE) && !isPlaying()) {
                ExternalTransport *transport = getExternalTransportControl();
                if (transport) {
                    transport->transportJump(ExternalTransport::TransportStopAtTime,
                                             RealTime::zeroTime);
                    transport->transportChange(ExternalTransport::TransportStart);
                }
            }
#ifdef DEBUG_ALSA
            std::cerr << "AlsaDriver::getMappedEventList - "
                      << "START" << std::endl;
#endif
            break;

        case SND_SEQ_EVENT_CONTINUE:
            if ((getMIDISyncStatus() == TRANSPORT_SLAVE) && !isPlaying()) {
                ExternalTransport *transport = getExternalTransportControl();
                if (transport) {
                    transport->transportChange(ExternalTransport::TransportPlay);
                }
            }
#ifdef DEBUG_ALSA
            std::cerr << "AlsaDriver::getMappedEventList - "
                      << "CONTINUE" << std::endl;
#endif
            break;

        case SND_SEQ_EVENT_STOP:
            if ((getMIDISyncStatus() == TRANSPORT_SLAVE) && isPlaying()) {
                ExternalTransport *transport = getExternalTransportControl();
                if (transport) {
                    transport->transportChange(ExternalTransport::TransportStop);
                }
            }
#ifdef DEBUG_ALSA
            std::cerr << "AlsaDriver::getMappedEventList - "
                      << "STOP" << std::endl;
#endif
            break;

        case SND_SEQ_EVENT_SONGPOS:
#ifdef DEBUG_ALSA
            std::cerr << "AlsaDriver::getMappedEventList - "
                      << "SONG POSITION" << std::endl;
#endif

            break;

            // these cases are handled by checkForNewClients
            //
        case SND_SEQ_EVENT_CLIENT_START:
        case SND_SEQ_EVENT_CLIENT_EXIT:
        case SND_SEQ_EVENT_CLIENT_CHANGE:
        case SND_SEQ_EVENT_PORT_START:
        case SND_SEQ_EVENT_PORT_EXIT:
        case SND_SEQ_EVENT_PORT_CHANGE:
        case SND_SEQ_EVENT_PORT_SUBSCRIBED:
        case SND_SEQ_EVENT_PORT_UNSUBSCRIBED:
            m_portCheckNeeded = true;
#ifdef DEBUG_ALSA
            std::cerr << "AlsaDriver::getMappedEventList - "
                      << "got announce event ("
                      << int(event->type) << ")" << std::endl;
#endif

            break;
        case SND_SEQ_EVENT_TICK:
        default:
#ifdef DEBUG_ALSA
            std::cerr << "AlsaDriver::getMappedEventList - "
                      << "got unhandled MIDI event type from ALSA sequencer"
                      << "(" << int(event->type) << ")" << std::endl;
#endif

            break;


        }
        */


    // ------------------ end of midi in loop

    //}
/*
    if (getMTCStatus() == TRANSPORT_SLAVE && isPlaying()) {
#ifdef MTC_DEBUG
        std::cerr << "seq time is " << getSequencerTime() << ", last MTC receive "
                  << m_mtcLastReceive << ", first time " << m_mtcFirstTime << std::endl;
#endif

        if (m_mtcFirstTime == 0) { // have received _some_ MTC quarter-frame info
            RealTime seqTime = getSequencerTime();
            if (m_mtcLastReceive < seqTime &&
                seqTime - m_mtcLastReceive > RealTime(0, 500000000L)) {
                ExternalTransport *transport = getExternalTransportControl();
                if (transport) {
                    transport->transportJump(ExternalTransport::TransportStopAtTime,
                                             m_mtcLastEncoded);
                }
            }
        }
    }
    */

    // Release lock
    //
    //releaseLock();
    pthread_mutex_unlock(&MidiThread::m_recLock);
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
    //MappedInstrument *instrument;

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

                //instrument = ((PortableSoundDriver*)(m_driver))->getMappedInstrument((*it)->getInstrument());

                std::vector<unsigned char> message;
                message.push_back(MIDI_NOTE_OFF + (*it)->getChannel());
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

