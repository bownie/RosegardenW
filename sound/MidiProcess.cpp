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

// Note on map
//
std::map<unsigned int, std::multimap<unsigned int, MappedEvent*> > MidiThread::m_noteOnMap;

// Elapsed time counter
//
double MidiThread::m_elapsedTime = 0;

// Elapsed time
//
//MidiThread::m_elapsedTime = 0;

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

// Returns the captured MID events to the PortableSoundDriver - we do this
// by copying the static list into a local MappedEventList, clearing the
// static list and returning the local one.  Might do this a better way.
//
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

    //QString thing = QString("GetReturnComposition size = %1").arg(mE.size());

    if (mE.size() > 0)
    {
        SEQUENCER_DEBUG << "MidiThread::getReturnComposition() -  returning composition size " << mE.size() << endl;
    }

    // return our local copy
    //
    return mE;
}

// The RTMidi in callback - we use this to identify and push events to the m_returnComposition
// which can then be queried and copied back into the driver on request.
//
// Unlike the AlsaDriver (on which this was originally based) the midiInCallback works on a per
// event basis - so there's no looping required yet for multiple events - this may well change
// once things get sticky with controllers and sysexes.
//
// Partial implementation compared to AlsaDriver - some stuff missing notably including SysExs
// and timing stuff for the moment.  Also some events might be incorrectly wired.
//
void
MidiThread::midiInCallback(double deltaTime, std::vector< unsigned char > *message, void *userData)
{
#ifdef DEBUG_RTMIDI
    SEQUENCER_DEBUG << "MidiThread::midiInCallback - called" << endl;
#endif

    // Always add on the delta time
    //
    MidiThread::m_elapsedTime += deltaTime;

    unsigned int nBytes = message->size();

    // Do something if there is nothing
    //
    if (nBytes == 0){
        return;
    }

    QString msg;

    // Some temporary logging
    //
    for ( unsigned int i=0; i < nBytes; i++ )
    {
      msg = QString("MidiThread::midiInCallback - got MIDI IN Byte %1 = %2").arg(i).arg((int)message->at(i));
      //std::cout << msg.toStdString() << std::endl;
    }

    if ( nBytes > 0 )
    {
      msg = QString("MidiThread::midiInCallback - got MIDI in timestamp = %1").arg(deltaTime);
      //std::cout << msg.toStdString() << std::endl;
    }

    // Channel is the lower byte
    //
    unsigned int channel = ((int)message->at(0)) & 0xF;

    // Bear in mind we might have no note information (no second byte)
    //
    unsigned int chanNoteKey = 0;

    if (message->size() > 1)
    {
        // Set up chanNoteKey
        //
        chanNoteKey = ( channel << 8 ) + (unsigned int) ((int)message->at(1));
    }
    bool fromController = false;

    /* How do we determine controller?

    if (event->dest.client == m_client &&
        event->dest.port == m_controllerPort) {
#ifdef DEBUG_ALSA
        std::cerr << "Received an external controller event" << std::endl;
#endif

        fromController = true;
    }*/


    unsigned int deviceId = Device::NO_DEVICE;
/*
    if (fromController) {
        deviceId = Device::CONTROL_DEVICE;
    } else {
        for (SoundDriver::MappedDeviceList::iterator i = m_devices.begin();
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
*/

    // ----------------------------------------------------
    // Time conversion - deltaTime is in seconds
    //
    int seconds = (int)MidiThread::m_elapsedTime;
    int nanoSeconds = (MidiThread::m_elapsedTime - (double)seconds) * 1000000000;
    RealTime eventTime(seconds, nanoSeconds);

    // Get a lock to insert into the m_returnComposition
    //
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
            mE->setRecordedChannel(0);
            mE->setRecordedDevice(0);
            mE->setType(MappedEvent::MidiNote);

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
            MidiThread::m_returnComposition->insert(new MappedEvent(*mE));
            MidiThread::m_noteOnMap[deviceId].insert(std::pair<unsigned int, MappedEvent*>(chanNoteKey, mE));

//#ifdef DEBUG_RTMIDI
            SEQUENCER_DEBUG << "MidiThread::midiInCallback - added NOTE ON event with pitch " << mE->getPitch()
                            << " and time " << mE->getEventTime() << " (elaspedTime = " << MidiThread::m_elapsedTime << ")"
                            << " and chanNoteKey = " << chanNoteKey << endl;
//#endif
        }

        break;

    case MIDI_NOTE_OFF:
    {

        // Check the note on map for any note on events to close.
        std::multimap<unsigned int, MappedEvent*>::iterator noteOnIt = MidiThread::m_noteOnMap[deviceId].find(chanNoteKey);

        SEQUENCER_DEBUG << "MidiThread::midiInCallback - GOT NOTE OFF - looking for chanNoteKey = " << chanNoteKey << endl;
        if (noteOnIt != MidiThread::m_noteOnMap[deviceId].end()) {

            // Set duration correctly on the NOTE OFF
            //
            MappedEvent *mE = noteOnIt->second;
            RealTime duration = eventTime - mE->getEventTime();

//#ifdef DEBUG_RTMIDI
            SEQUENCER_DEBUG << "MidiThread::midiInCallback - NOTE OFF: found NOTE ON at " << mE->getEventTime() << endl;
//#endif

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
            MidiThread::m_returnComposition->insert(mE);

            // reset the reference
            //
            MidiThread::m_noteOnMap[deviceId].erase(noteOnIt);
        }
    }
    break;

    case MIDI_POLY_AFTERTOUCH:
    {
        if (fromController)
            break;

        // Fix for 632964 by Pedro Lopez-Cabanillas (20030523)
        //
        MappedEvent *mE = new MappedEvent();
        mE->setType(MappedEvent::MidiKeyPressure);
        mE->setEventTime(eventTime);
        mE->setData1(message->at(1));
        mE->setData2(message->at(2));
        mE->setRecordedChannel(channel);
        mE->setRecordedDevice(deviceId);
        MidiThread::m_returnComposition->insert(mE);
    }
    break;

    case MIDI_CTRL_CHANGE:
    {
        MappedEvent *mE = new MappedEvent();
        mE->setType(MappedEvent::MidiController);
        mE->setEventTime(eventTime);
        mE->setData1(message->at(1));
        mE->setData2(message->at(2));
        mE->setRecordedChannel(channel);
        mE->setRecordedDevice(deviceId);
        MidiThread::m_returnComposition->insert(mE);
    }
    break;

    case MIDI_PROG_CHANGE:
    {
        MappedEvent *mE = new MappedEvent();
        mE->setType(MappedEvent::MidiProgramChange);
        mE->setEventTime(eventTime);
        mE->setData1(message->at(1));
        mE->setRecordedChannel(channel);
        mE->setRecordedDevice(deviceId);
        MidiThread::m_returnComposition->insert(mE);
    }
    break;

    case MIDI_PITCH_BEND:
    {
        if (fromController)
            break;

        // Fix for 711889 by Pedro Lopez-Cabanillas (20030523)
        //
        //int s = event->data.control.value + 8192;
        //int d1 = (s >> 7) & 0x7f; // data1 = MSB
        //int d2 = s & 0x7f; // data2 = LSB
        int d1 = message->at(1);
        int d2 = message->at(2);
        MappedEvent *mE = new MappedEvent();
        mE->setType(MappedEvent::MidiPitchBend);
        mE->setEventTime(eventTime);
        mE->setData1(d1);
        mE->setData2(d2);
        mE->setRecordedChannel(channel);
        mE->setRecordedDevice(deviceId);
        MidiThread::m_returnComposition->insert(mE);
    }
        break;

    case MIDI_CHNL_AFTERTOUCH:
    {
        if (fromController)
            break;

        // Fixed by Pedro Lopez-Cabanillas (20030523)
        //
        int s = message->at(1) & 0x7f;
        MappedEvent *mE = new MappedEvent();
        mE->setType(MappedEvent::MidiChannelPressure);
        mE->setEventTime(eventTime);
        mE->setData1(s);
        mE->setRecordedChannel(channel);
        mE->setRecordedDevice(deviceId);
        MidiThread::m_returnComposition->insert(mE);
    }
        break;

    case MIDI_SYSTEM_EXCLUSIVE:

        if (fromController)
            break;

#ifdef DEBUG_RTMIDI
        SEQUENCER_DEBUG << "MidiThread::midiInCallback - SYSEX IN not implemented." << endl;
#endif
        break;


    case MIDI_ACTIVE_SENSING:  // MIDI device is still there
        break;

    case MIDI_CUE_POINT: // might not be this message we want here

        if (fromController)
            break;
        //if (getMTCStatus() == TRANSPORT_SLAVE) {
            //handleMTCQFrame(event->data.control.value, eventTime);
        //}
        break;

    case MIDI_TIMING_CLOCK:
#ifdef DEBUG_RTMIDI
        std::cerr << "MidiThread::midiInCallback - got realtime MIDI clock" << std::endl;
#endif
        break;

    case MIDI_START:
        /*
        if ((getMIDISyncStatus() == TRANSPORT_SLAVE) && !isPlaying()) {
            ExternalTransport *transport = getExternalTransportControl();
            if (transport) {
                transport->transportJump(ExternalTransport::TransportStopAtTime,
                                         RealTime::zeroTime);
                transport->transportChange(ExternalTransport::TransportStart);
            }
        }*/
#ifdef DEBUG_RTMIDI
        std::cerr << "MidiThread::midiInCallback - START" << std::endl;
#endif
        break;

    case MIDI_CONTINUE:
        /*
        if ((getMIDISyncStatus() == TRANSPORT_SLAVE) && !isPlaying()) {
            ExternalTransport *transport = getExternalTransportControl();
            if (transport) {
                transport->transportChange(ExternalTransport::TransportPlay);
            }
        }*/
#ifdef DEBUG_RTMIDI
        std::cerr << "MidiThread::midiInCallback - CONTINUE" << std::endl;
#endif
        break;

    case MIDI_STOP:
        /*
        if ((getMIDISyncStatus() == TRANSPORT_SLAVE) && isPlaying()) {
            ExternalTransport *transport = getExternalTransportControl();
            if (transport) {
                transport->transportChange(ExternalTransport::TransportStop);
            }
        }*/
#ifdef DEBUG_RTMIDI
        std::cerr << "MidiThread::midiInCallback - STOP" << std::endl;
#endif
        break;

    case MIDI_SONG_POSITION_PTR:
#ifdef DEBUG_RTMIDI
        std::cerr << "MidiThread::midiInCallback - SONG POSITION" << std::endl;
#endif

        break;

    default:
        break;
    }


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

    // Release lock on shared return structure
    //
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

