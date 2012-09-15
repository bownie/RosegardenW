/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- vi:set ts=8 sts=4 sw=4: */

/*
    Rosegarden
    A MIDI and audio sequencer and musical notation editor.
    Copyright 2000-2010 the Rosegarden development team.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

/*
  RtMidi licence
  --------------

  RtMidi: realtime MIDI i/o C++ classes
  Copyright (c) 2003-2012 Gary P. Scavone

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
  files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,
  modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  Any person wishing to distribute modifications to the Software is asked to send the modifications to the original developer so
  that they can be incorporated into the canonical version. This is, however, not a binding provision of this license.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include "PortableSoundDriver.h"
#include "misc/Debug.h"
#include <windows.h>
#include "MappedEvent.h"


namespace Rosegarden
{

PortableSoundDriver::PortableSoundDriver(MappedStudio *studio):
            SoundDriver(studio, std::string("[PortableSoundDriver]")),
//            m_midiIn(0),
  //          m_midiOut(0),
            m_bufferSize(256),
            m_loopStartTime(0, 0),
            m_loopEndTime(0, 0),
            m_looping(false)
{
    std::cout << "PortableSoundDriver::PortableSoundDriver" << std::endl;

    // Define the temp out buffer
    //
    m_tempOutBuffer = new MappedEvent[m_bufferSize];
}


PortableSoundDriver::~PortableSoundDriver()
{
    //if ( m_midiIn ) delete m_midiIn;
    //if ( m_midiOut ) delete m_midiOut;

    for (unsigned int i = 0; i < m_midiInPorts.size(); i++)
    {
        m_midiInPorts[i]->closePort();
        delete m_midiInPorts[i];
    }

    for(unsigned int i = 0; i < m_midiOutPorts.size(); i++)
    {
        m_midiOutPorts[i]->closePort();
        delete m_midiOutPorts[i];
    }

    if ( m_midiThread ) delete m_midiThread;
}


bool
PortableSoundDriver::initialise()
{
    SEQUENCER_DEBUG << "PortableSoundDriver::initialise" << endl;


    // Initialise at least one port in one directionm_midiInPorts
    //
    try
    {
        m_midiInPorts.push_back(new RtMidiIn());
    }
    catch (RtError &error)
    {
      // Handle the exception here
      SEQUENCER_DEBUG << error.getMessage() << endl;
      return false;
    }

    // RtMidiOut constructor
    //
    try
    {
      m_midiOutPorts.push_back(new RtMidiOut());
    }
    catch ( RtError &error )
    {
      SEQUENCER_DEBUG << error.getMessage() << endl;
      return false;
    }

    // Now create a Midi Thread
    //
    m_midiThread = new MidiThread("rosegarden-portable-midi-thread", this, 44100);

    // Initialise the MIDI thread
    //
    m_midiThread->run();

    if (m_midiThread->running())
    {
        SEQUENCER_DEBUG << "PortableSoundDriver::initialise - MIDI thread is running";
        //m_midiThread->signal();

        // Set MIDI_OK in our status
        //
        m_driverStatus |= MIDI_OK;
    }
    else
    {
        SEQUENCER_DEBUG << "PortableSoundDriver::initialise - MIDI thread did not start";
    }

    //generateFixedInstruments();

    return true;
}

void
PortableSoundDriver::initialisePlayback(const RealTime &position)
{
    m_playStartPosition = position;
    m_systemStartTime = getSystemTime();
}

// Gets the time of the high resolution timer
//
RealTime
PortableSoundDriver::getSystemTime()
{
    _LARGE_INTEGER tick, ticksPerSecond;

    if (QueryPerformanceFrequency(&ticksPerSecond) == false)
    {
        SEQUENCER_DEBUG << "High resolution timer not supported";
        return RealTime(0, 0);
    }

    QueryPerformanceCounter(&tick);

    int seconds = tick.QuadPart/ticksPerSecond.QuadPart;
    int nsecs = (double(tick.QuadPart)/double(ticksPerSecond.QuadPart) * 1000000000.0) - (seconds * 1000000000.0);

    return RealTime(seconds, nsecs);
}

void
PortableSoundDriver::stopPlayback()
{
    SEQUENCER_DEBUG << "PortableSoundDriver::stopPlayback";


    // Clear any pending note ons and stop any playing notes
    //
    m_midiThread->clearBuffersOut();
}

// stop recording, continue playing
void
PortableSoundDriver::punchOut()
{
    SEQUENCER_DEBUG << "PortableSoundDriver::punchOut";

    /*
    snd_seq_event_t event;
    ClientPortPair outputDevice;
    RealTime offTime;

    // drop any pending notes
    snd_seq_drop_output_buffer(m_midiHandle);
    snd_seq_drop_output(m_midiHandle);

    // prepare the event
    snd_seq_ev_clear(&event);
    offTime = getAlsaTime();

    for (NoteOffQueue::iterator it = m_noteOffQueue.begin();
         it != m_noteOffQueue.end(); ++it) {
        // Set destination according to connection for instrument
        //
        outputDevice = getPairForMappedInstrument((*it)->getInstrument());
        if (outputDevice.first < 0 || outputDevice.second < 0)
            continue;

        snd_seq_ev_set_subs(&event);

        // Set source according to port for device
        //
        int src = getOutputPortForMappedInstrument((*it)->getInstrument());
        if (src < 0)
            continue;
        snd_seq_ev_set_source(&event, src);

        snd_seq_ev_set_noteoff(&event,
                               (*it)->getChannel(),
                               (*it)->getPitch(),
                               127);

        delete(*it);
    }

    m_noteOffQueue.erase(m_noteOffQueue.begin(), m_noteOffQueue.end());

    */
}

int
PortableSoundDriver::getOutputPortForMappedInstrument(InstrumentId id)
{
    //SEQUENCER_DEBUG << "NUMBER of output ports = " << m_outputPorts.size() << endl;

    MappedInstrument *instrument = getMappedInstrument(id);
    if (instrument) {
        DeviceId device = instrument->getDevice();

        SEQUENCER_DEBUG << "DEVICE ID = " << device << endl;

        DeviceIntMap::iterator i = m_outputPorts.find(device);

        if (i != m_outputPorts.end()) {
            return i->second;
        }
    }

    return -1;
}

RtMidiIn*
PortableSoundDriver::getRtMidiIn(unsigned int portNum)
{
    return m_midiInPorts[portNum];
}


RtMidiOut*
PortableSoundDriver::getRtMidiOut(unsigned int portNum)
{
    return m_midiOutPorts[portNum];
}

// Assigns a new midi in port as required
//
void
PortableSoundDriver::checkRtMidiIn(unsigned int portNum)
{
    while (portNum >= m_midiInPorts.size() )
    {
        // RtMidiIn constructor
        //
        try
        {
          RtMidiIn *addPort = new RtMidiIn();
          m_midiInPorts.push_back(addPort);
          SEQUENCER_DEBUG << "PortableSoundDriver::getRtMidiIn - adding port " << portNum;
        }
        catch (RtError &error)
        {
          // Handle the exception here
          SEQUENCER_DEBUG << error.getMessage() << endl;
        }
    }
}

// Assigns a new midi out port if required
//
void
PortableSoundDriver::checkRtMidiOut(unsigned int portNum)
{
    while (portNum >= m_midiOutPorts.size() )
    {
        // RtMidiIn constructor
        //
        try
        {
          RtMidiOut *addPort = new RtMidiOut();
          m_midiOutPorts.push_back(addPort);
        }
        catch (RtError &error)
        {
          // Handle the exception here
          SEQUENCER_DEBUG << error.getMessage() << endl;
        }
    }
}

void
PortableSoundDriver::resetPlayback(const RealTime & /*oldPosition*/, const RealTime &position)
{
    SEQUENCER_DEBUG << "PortableSoundDriver::resetPlayback";

    // Clear any pending note ons and stop any playing notes
    //
    m_midiThread->clearBuffersOut();

    m_playStartPosition = position;
    m_systemStartTime = getSystemTime();
}

void
PortableSoundDriver::allNotesOff()
{
    SEQUENCER_DEBUG << "PortableSoundDriver::allNotesOff";

    // Ask the thread to process everything to off
    //
    m_midiThread->processNotesOff(true);
}

RealTime
PortableSoundDriver::getSequencerTime()
{
    RealTime t(0, 0);

    t = getSystemTime() + m_playStartPosition - m_systemStartTime;

    return t;
}

// This is where we get incoming events from and generate a MappedEventList from
// the MIDI captured in the MIDIThread.
//
// See AlsaDriver::getMappedEventList(MappedEventList &composition) for clues
//
bool
PortableSoundDriver::getMappedEventList(MappedEventList &mel)
{
    MappedEventList me = MidiThread::getReturnComposition();


    for (MappedEventListIterator it = me.begin(); it != me.end(); it++)
    {
      mel.insert(new MappedEvent(*it));
      SEQUENCER_DEBUG << "PortableSoundDriver::getMappedEventList - Event Type = " << (*it)->getType() << endl;
      SEQUENCER_DEBUG << "PortableSoundDriver::getMappedEventList - Pitch      = " << (*it)->getPitch() << endl;
      SEQUENCER_DEBUG << "PortableSoundDriver::getMappedEventList - Event Vely = " << (*it)->getVelocity() << endl;
      SEQUENCER_DEBUG << "PortableSoundDriver::getMappedEventList - Instrument = " << (*it)->getInstrument() << endl;
      SEQUENCER_DEBUG << "PortableSoundDriver::getMappedEventList - Track Id   = " << (*it)->getTrackId() << endl;

    }

    //SEQUENCER_DEBUG << "GETMAPPEDEVENTLIST GOT " << MidiThread::getReturnComposition().size() << " EVENTS" << endl;
    //SEQUENCER_DEBUG << "MEL SIZE = " << mel.size() << endl;


    return (mel.size() > 0);
}

// Process all outbound events
//
void
PortableSoundDriver::processEventsOut(const MappedEventList &mC)
{
    processEventsOut(mC, RealTime::zeroTime, RealTime::zeroTime);
}

void
PortableSoundDriver::startClocks()
{
    SEQUENCER_DEBUG << "PortableSoundDriver::startClocks()";

    if (m_midiThread->running())
    {
        SEQUENCER_DEBUG << "PortableSoundDriver::startClocks - midi thread is running";

        // Send a synchronisation event to the MIDI thread
        //
        MappedEvent *syncEvent = new MappedEvent(255, MappedEvent::SystemMIDISyncAuto);
        syncEvent->setEventTime(getSystemTime());

        // Send this event to the Midi thread ringbuffer
        //
        m_midiThread->getMidiOutBuffer()->write(syncEvent, 1);

        // Reset this
        //
        MidiThread::setElapsedTime(0);

    }
    else
    {
        SEQUENCER_DEBUG << "PortableSoundDriver::startClocks - midi thread is NOT running";
    }
}

void
PortableSoundDriver::stopClocks()
{
    SEQUENCER_DEBUG << "PortableSoundDriver::stopClocks()";

    //m_alsaPlayStartTime = RealTime::zeroTime;
    m_systemStartTime = RealTime::zeroTime;
}

// Process some scheduled events on the output queue.  The
// slice times are here so that the driver can interleave
// note-off events as appropriate.
//
void
PortableSoundDriver::processEventsOut(const MappedEventList &mC,
                              const RealTime &sliceStart,
                              const RealTime &sliceEnd)
{
    SEQUENCER_DEBUG << "PortableSoundDriver::processEventsOut";

    // You can do some audio crap here if you like
    //
    /*
    for (MappedEventList::const_iterator i = mC.begin(); i != mC.end(); ++i) {

    }
    */

    // Process MIDI
    //
    processMidiOut(mC, sliceStart, sliceEnd);

}

// Activate a recording state.  armedInstruments and audioFileNames
// can be NULL if no audio tracks recording.
//
bool
PortableSoundDriver::record(RecordStatus recordStatus,
                    const std::vector<InstrumentId> *armedInstruments = 0,
                    const std::vector<QString> *audioFileNames = 0)
{
    return true;
}

// Process anything that's pending
//
void
PortableSoundDriver::processPending()
{
    //SEQUENCER_DEBUG << "PortableSoundDriver::processPending";
}

// Get the driver's operating sample rate
//
unsigned int
PortableSoundDriver::getSampleRate() const
{
    return 0;
}

// Plugin instance management
//
void
PortableSoundDriver::setPluginInstance(InstrumentId id,
                               QString identifier,
                               int position)
{
}

void
PortableSoundDriver::removePluginInstance(InstrumentId id,
                                  int position)
{
}

// Clear down and remove all plugin instances
//
void
PortableSoundDriver::removePluginInstances()
{
}

void
PortableSoundDriver::setPluginInstancePortValue(InstrumentId id,
                                        int position,
                                        unsigned long portNumber,
                                        float value)
{
}

float
PortableSoundDriver::getPluginInstancePortValue(InstrumentId id,
                                         int position,
                                         unsigned long portNumber)
{
    return 0.0f;
}

void
PortableSoundDriver::setPluginInstanceBypass(InstrumentId id,
                                     int position,
                                     bool value)
{
}

QStringList
PortableSoundDriver::getPluginInstancePrograms(InstrumentId id,
                                              int position)
{
    return QStringList();
}

QString
PortableSoundDriver::getPluginInstanceProgram(InstrumentId id,
                                         int position)
{
    return QString();
}

QString
PortableSoundDriver::getPluginInstanceProgram(InstrumentId id,
                                         int position,
                                         int bank,
                                         int program)
{
    return QString();
}

unsigned long
PortableSoundDriver::getPluginInstanceProgram(InstrumentId id,
                                               int position,
                                               QString name)
{
    return 0;
}

void
PortableSoundDriver::setPluginInstanceProgram(InstrumentId id,
                                      int position,
                                      QString program)
{
}

QString
PortableSoundDriver::configurePlugin(InstrumentId id,
                                int position,
                                QString key,
                                QString value)
{
    return QString();
}

void
PortableSoundDriver::setAudioBussLevels(int bussId,
                                float dB,
                                float pan)
{
}

void
PortableSoundDriver::setAudioInstrumentLevels(InstrumentId id,
                                              float dB,
                                              float pan)
{
}

// Poll for new clients (for new Devices/Instruments)
//
bool
PortableSoundDriver::checkForNewClients()
{
    std::string portName;

    // Always have at least one port available
    //
    if (m_midiInPorts.size() == 0)
    {
        m_midiInPorts.push_back(new RtMidiIn());
    }

    if (m_midiOutPorts.size() == 0)
    {
        m_midiOutPorts.push_back(new RtMidiOut());
    }


    // Check inputs -
    unsigned int nPorts = m_midiInPorts[0]->getPortCount();
    SEQUENCER_DEBUG << "RtMidi: There are " << nPorts << " MIDI input sources available."  << endl;

    for ( unsigned int i = 0; i < nPorts; i++ )
    {
      try
      {
        portName = m_midiInPorts[0]->getPortName(i);
      }

      catch ( RtError &error )
      {
        SEQUENCER_DEBUG << error.getMessage() << endl;
        return false;
      }
      SEQUENCER_DEBUG << "  Input Port #" << i+1 << ": " << portName << endl;
    }

    // Check outputs.
    //
    try
    {
        nPorts = m_midiOutPorts[0]->getPortCount();
    }
    catch ( RtError &error )
    {
        SEQUENCER_DEBUG << error.getMessage() << endl;
        return false;
    }

    SEQUENCER_DEBUG << "There are " << nPorts << " MIDI output ports available." << endl;

    for ( unsigned int i = 0; i < nPorts; i++ )
    {
      try
      {
        portName = m_midiOutPorts[0]->getPortName(i);

    //    device = new MappedDevice(0, Device::Midi, m_midiOut->getPortName(i), m_midiOut->getPortName(i));
      //  m_devices.push_back(device);
        //instr = new MappedInstrument(Instrument::Midi, )
      }
      catch (RtError &error)
      {
        SEQUENCER_DEBUG << error.getMessage() << endl;
        return false;
      }
      SEQUENCER_DEBUG << "  Output Port #" << i+1 << ": " << portName << endl;
    }

    return true;
}

// Set a loop position at the driver (used for transport)
//
void
PortableSoundDriver::setLoop(const RealTime &loopStart, const RealTime &loopEnd)
{
    m_loopStartTime = loopStart;
    m_loopEndTime = loopEnd;

    // currently we use this simple test for looping - it might need
    // to get more sophisticated in the future.
    //
    if (m_loopStartTime != m_loopEndTime)
        m_looping = true;
    else
        m_looping = false;
}

// Are we counting?  By default a subclass probably wants to
// return true, if it doesn't know better.
//
bool
PortableSoundDriver::areClocksRunning() const
{
    return true;
}

void
PortableSoundDriver::getAudioInstrumentNumbers(InstrumentId &, int &)
{
}

void
PortableSoundDriver::getSoftSynthInstrumentNumbers(InstrumentId &ssInstrumentBase, int &ssInstrumentCount)
{
    ssInstrumentBase = SoftSynthInstrumentBase;
    ssInstrumentCount = SoftSynthInstrumentCount;
}

// Plugin management -- SoundDrivers should maintain a plugin
// scavenger which the audio process code can use for defunct
// plugins.  Ownership of plugin is passed to the SoundDriver.
//
void
PortableSoundDriver::claimUnwantedPlugin(void *plugin)
{
}

// This causes all scavenged plugins to be destroyed.  It
// should only be called in non-RT contexts.
//
void
PortableSoundDriver::scavengePlugins()
{
}

void
PortableSoundDriver::processMidiOut(const MappedEventList &mC,
                                    const RealTime &sliceStart,
                                    const RealTime &sliceEnd)
{
    RealTime outputTime;
    RealTime outputStopTime;
    //MappedInstrument *instrument;
    //ClientPortPair outputDevice;
    //MidiByte channel;

    // special case for unqueued events
    //
    bool now = (sliceStart == RealTime::zeroTime && sliceEnd == RealTime::zeroTime);

    if (!now) {
        // This 0.5 sec is arbitrary, but it must be larger than the
        // sequencer's read-ahead
        RealTime diff = RealTime::fromSeconds(0.5);
        RealTime cutoff = sliceStart - diff;
        //cropRecentNoteOffs(cutoff - m_playStartPosition + m_alsaPlayStartTime);
    }

    //this->m_midiThread->bufferMidiOut(mC);

    RingBuffer<MappedEvent> *rb = this->m_midiThread->getMidiOutBuffer();

    // reset the buffer size
    //
    delete [] this->m_tempOutBuffer;
    m_tempOutBuffer = new MappedEvent[mC.size()];
    memset(m_tempOutBuffer, 0, mC.size());

    //Write out events singly
    //
    MappedEventList::const_iterator it = mC.begin();
    for (unsigned int i = 0; i < mC.size(); ++i)
    {
        m_tempOutBuffer[i] = **(it++);
        //rb->write(*it, 1);
    }

    SEQUENCER_DEBUG << "PortableSoundDriver::processMidiOut - writing " << mC.size() << "events";

    // Write out temporary buffer
    //
    rb->write(m_tempOutBuffer, mC.size());

    //processNotesOff(sliceEnd - m_playStartPosition + m_systemStartTime, now);

}

/*
void
PortableSoundDriver::processNotesOff(const RealTime &time, bool now, bool everything)
{
    SEQUENCER_DEBUG << "PortableSoundDriver::processNotesOff";
    //this->m_midiThread->processNotesOff(everything);
}
*/

void
PortableSoundDriver::generateFixedInstruments()
{
    // Create a number of soft synth Instruments
    //
    MappedInstrument *instr;
    char number[100];
    InstrumentId first;
    int count;
    getSoftSynthInstrumentNumbers(first, count);

    // soft-synth device takes id to match first soft-synth instrument
    // number, for easy identification & consistency with GUI
    DeviceId ssiDeviceId = first;

    for (int i = 0; i < count; ++i) {
        sprintf(number, " #%d", i + 1);
        std::string name = QObject::tr("Synth plugin").toStdString() + std::string(number);
        instr = new MappedInstrument(Instrument::SoftSynth,
                                     i,
                                     first + i,
                                     name,
                                     ssiDeviceId);
        m_instruments.push_back(instr);

        m_studio->createObject(MappedObject::AudioFader,
                               first + i);
    }

    MappedDevice *device =
        new MappedDevice(ssiDeviceId,
                         Device::SoftSynth,
                         "Synth plugin",
                         "Soft synth connection");
    m_devices.push_back(device);

    // Create a number of audio Instruments - these are just
    // logical Instruments anyway and so we can create as
    // many as we like and then use them for Tracks.
    //
    // Note that unlike in earlier versions of Rosegarden, we always
    // have exactly one soft synth device and one audio device (even
    // if audio output is not actually working, the device is still
    // present).
    //
    /*
    std::string audioName;
    getAudioInstrumentNumbers(first, count);
*/

    // audio device takes id to match first audio instrument
    // number, for easy identification & consistency with GUI
    /*
    DeviceId audioDeviceId = first;

    for (int i = 0; i < count; ++i) {
        sprintf(number, " #%d", i + 1);
        audioName = QObject::tr("Audio").toStdString() + std::string(number);
        instr = new MappedInstrument(Instrument::Audio,
                                     i,
                                     first + i,
                                     audioName,
                                     audioDeviceId);
        m_instruments.push_back(instr);

        // Create a fader with a matching id - this is the starting
        // point for all audio faders.
        //
        m_studio->createObject(MappedObject::AudioFader, first + i);
    }
        */

    // Create audio device
    //
/*
    device =
        new MappedDevice(audioDeviceId,
                         Device::Audio,
                         "Audio",
                         "Audio connection");
    m_devices.push_back(device);
    */
}

bool
PortableSoundDriver::addDevice(Device::DeviceType type,
                               DeviceId deviceId,
                               InstrumentId baseInstrumentId,
                               MidiDevice::DeviceDirection direction)
{
    SEQUENCER_DEBUG << "PortableSoundDriver::addDevice" << endl;

    if (type == Device::Midi) {

        MappedDevice *device = createMidiDevice(deviceId, direction);
        if (!device) {
            SEQUENCER_DEBUG << "WARNING: Device creation failed" << endl;

        } else {
            addInstrumentsForDevice(device, baseInstrumentId);
            m_devices.push_back(device);

            //if (direction == MidiDevice::Record) {
//                setRecordDevice(device->getId(), true);
//            }

            return true;
        }
    }

    return false;
}

MappedDevice *
PortableSoundDriver::createMidiDevice(DeviceId deviceId,
                                      MidiDevice::DeviceDirection reqDirection)
{
    std::string connectionName = "";
    std::string deviceName = "";
    unsigned int rtMidiPort = -1;
    //std::string portName;

    if (reqDirection == MidiDevice::Play)
    {
        // Check outputs.
        //
        unsigned int nPorts = 0;
        try
        {
            nPorts = m_midiOutPorts[0]->getPortCount();
        }
        catch ( RtError &error )
        {
            SEQUENCER_DEBUG << error.getMessage() << endl;
            return false;
        }

        if (m_outputPorts.size() < m_midiOutPorts[0]->getPortCount())
        {
            rtMidiPort = m_outputPorts.size();

            try
            {
                deviceName = m_midiOutPorts[0]->getPortName(rtMidiPort);
                connectionName = deviceName;
            }
            catch (RtError &error)
            {
                SEQUENCER_DEBUG << error.getMessage() << endl;
            }
        }
    }
    /*
    else
    {
        unsigned int nPorts = 0;

        try
        {
            nPorts = m_midiIn->getPortCount();
        }
        catch ( RtError &error )
        {
            SEQUENCER_DEBUG << error.getMessage() << endl;
            return false;
        }

        for ( unsigned int i=0; i<nPorts; i++ )
        {
          try
          {
            portName = m_midiIn->getPortName(i);
          }

          catch ( RtError &error )
          {
            SEQUENCER_DEBUG << error.getMessage() << endl;
          }
        }
    }*/

    // Create device once we've filled the fields
    //
    if ( deviceName != "" )
    {
        MappedDevice *device = new MappedDevice(deviceId,
                                                Device::Midi,
                                                deviceName,
                                                connectionName);

        m_outputPorts[deviceId] = rtMidiPort;

        SEQUENCER_DEBUG << "Setting m_outputPorts deviceId = " << deviceId
                        << " to " << rtMidiPort;

        device->setDirection(reqDirection);
        return device;
    }
    else
        return 0;

}

void
PortableSoundDriver::addInstrumentsForDevice(MappedDevice *device, InstrumentId base)
{
    std::string channelName;
    char number[100];

    for (int channel = 0; channel < 16; ++channel) {

        // name is just number, derive rest from device at gui
        sprintf(number, "#%d", channel + 1);
        channelName = std::string(number);

        if (channel == 9) channelName = std::string("#10[D]");

        MappedInstrument *instr = new MappedInstrument
            (Instrument::Midi, channel, base++, channelName, device->getId());
        m_instruments.push_back(instr);
    }
}

void
PortableSoundDriver::removeDevice(DeviceId id)
{
    SEQUENCER_DEBUG << "PortableSoundDriver::removeDevice" << endl;

    for (MappedDeviceList::iterator i = m_devices.end();
         i != m_devices.begin(); ) {

        --i;

        if ((*i)->getId() == id) {
            delete *i;
            m_devices.erase(i);
        }
    }

    for (MappedInstrumentList::iterator i = m_instruments.end();
         i != m_instruments.begin(); ) {

        --i;

        if ((*i)->getDevice() == id) {
            delete *i;
            m_instruments.erase(i);
        }
    }
}


void
PortableSoundDriver::removeAllDevices()
{
    SEQUENCER_DEBUG << "PortableSoundDriver::removeAllDevices" << endl;

    while (!m_outputPorts.empty()) {
        m_outputPorts.erase(m_outputPorts.begin());
    }

    //clearDevices();
}

void
PortableSoundDriver::renameDevice(DeviceId, QString)
{
    SEQUENCER_DEBUG << "PortableSoundDriver::renameDevice" << endl;
}

unsigned int
PortableSoundDriver::getConnections(Device::DeviceType type,
                                    MidiDevice::DeviceDirection direction)
{
    SEQUENCER_DEBUG << "getConnections"<< endl;

    if ( type != Device::Midi )
    {
        return 0;
    }

    switch (direction)
    {
        case MidiDevice::Play:
            return m_midiOutPorts[0]->getPortCount();
            break;

        case MidiDevice::Record:
            return m_midiInPorts[0]->getPortCount();
            break;

        default:
            SEQUENCER_DEBUG << "getConnections - invalid direction" << endl;
            break;
    }

    return 0;
}

QString
PortableSoundDriver::getConnection(Device::DeviceType type,
                                   MidiDevice::DeviceDirection direction,
                                   unsigned int connectionNo)
{
    SEQUENCER_DEBUG << "getConnection" << endl;

    if ( direction == MidiDevice::Record )
    {
        try
        {
            return QString(m_midiInPorts[0]->getPortName(connectionNo).c_str());
        }

        catch ( RtError &error )
        {
            SEQUENCER_DEBUG << error.getMessage() << endl;
        }
    }

    if ( direction == MidiDevice::Play )
    {
        try
        {
            return QString(m_midiOutPorts[0]->getPortName(connectionNo).c_str());
        }
        catch (RtError &error)
        {
            SEQUENCER_DEBUG << error.getMessage() << endl;
        }
    }

    return QString("<invalid device number passed>");
}

// For this method - to avoid having to map play and record devices
// we always assume that record devices start at +1 over the play devices
//
QString
PortableSoundDriver::getConnection(DeviceId id)
{
    SEQUENCER_DEBUG << "getConnection" << endl;

    if (id < m_midiOutPorts[0]->getPortCount())
    {
        return QString(m_midiOutPorts[0]->getPortName(id).c_str());
    }

    if ( id < (m_midiOutPorts[0]->getPortCount() + m_midiInPorts[0]->getPortCount()))
    {
        return QString(m_midiInPorts[0]->getPortName(id - m_midiOutPorts[0]->getPortCount()).c_str());
    }

    return QString("<deviceId is out of scope>");
}


}
