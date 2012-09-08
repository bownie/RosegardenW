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


// Specialisation of SoundDriver to support ALSA (http://www.alsa-project.org)
//
//

#include "SoundDriver.h"
#include "MappedStudio.h"
#include "MidiProcess.h"
#include "rtmidi/RtMidi.h"

#ifndef _PORTABLESOUNDDRIVER_H
#define _PORTABLESOUNDDRIVER_H

namespace Rosegarden
{

class PortableSoundDriver : public SoundDriver
{
public:
    PortableSoundDriver(MappedStudio *studio);
    virtual ~PortableSoundDriver();

    virtual bool initialise();
    virtual void initialisePlayback(const RealTime &position);
    virtual void stopPlayback();
    virtual void punchOut(); // stop recording, continue playing
    virtual void resetPlayback(const RealTime &oldPosition, const RealTime &position);
    virtual void allNotesOff();

    virtual RealTime getSequencerTime();

    virtual bool getMappedEventList(MappedEventList &);

    // Process some asynchronous events
    //
    virtual void processEventsOut(const MappedEventList &mC);

    // Process some scheduled events on the output queue.  The
    // slice times are here so that the driver can interleave
    // note-off events as appropriate.
    //
    virtual void processEventsOut(const MappedEventList &mC,
                                  const RealTime &sliceStart,
                                  const RealTime &sliceEnd);

    // Activate a recording state.  armedInstruments and audioFileNames
    // can be NULL if no audio tracks recording.
    //
    virtual bool record(RecordStatus recordStatus,
                        const std::vector<InstrumentId> *armedInstruments ,
                        const std::vector<QString> *audioFileNames );

    // Process anything that's pending
    //
    virtual void processPending();

    // Get the driver's operating sample rate
    //
    virtual unsigned int getSampleRate() const ;

    // Plugin instance management
    //
    virtual void setPluginInstance(InstrumentId id,
                                   QString identifier,
                                   int position);

    virtual void removePluginInstance(InstrumentId id,
                                      int position);

    // Clear down and remove all plugin instances
    //
    virtual void removePluginInstances();

    virtual void setPluginInstancePortValue(InstrumentId id,
                                            int position,
                                            unsigned long portNumber,
                                            float value);

    virtual float getPluginInstancePortValue(InstrumentId id,
                                             int position,
                                             unsigned long portNumber);

    virtual void setPluginInstanceBypass(InstrumentId id,
                                         int position,
                                         bool value);

    virtual QStringList getPluginInstancePrograms(InstrumentId id,
                                                  int position);

    virtual QString getPluginInstanceProgram(InstrumentId id,
                                             int position);

    virtual QString getPluginInstanceProgram(InstrumentId id,
                                             int position,
                                             int bank,
                                             int program);

    virtual unsigned long getPluginInstanceProgram(InstrumentId id,
                                                   int position,
                                                   QString name);

    virtual void setPluginInstanceProgram(InstrumentId id,
                                          int position,
                                          QString program);

    virtual QString configurePlugin(InstrumentId id,
                                    int position,
                                    QString key,
                                    QString value);

    virtual void setAudioBussLevels(int bussId,
                                    float dB,
                                    float pan);

    virtual void setAudioInstrumentLevels(InstrumentId id,
                                          float dB,
                                          float pan);

    // Poll for new clients (for new Devices/Instruments)
    //
    virtual bool checkForNewClients();

    // Set a loop position at the driver (used for transport)
    //
    virtual void setLoop(const RealTime &loopStart, const RealTime &loopEnd);

    // Are we counting?  By default a subclass probably wants to
    // return true, if it doesn't know better.
    //
    virtual bool areClocksRunning() const ;

    virtual void getAudioInstrumentNumbers(InstrumentId &, int &);
    virtual void getSoftSynthInstrumentNumbers(InstrumentId &, int &);

    // Plugin management -- SoundDrivers should maintain a plugin
    // scavenger which the audio process code can use for defunct
    // plugins.  Ownership of plugin is passed to the SoundDriver.
    //
    virtual void claimUnwantedPlugin(void *plugin);

    // This causes all scavenged plugins to be destroyed.  It
    // should only be called in non-RT contexts.
    //
    virtual void scavengePlugins();

    // Ok, some device management overrides - these should be pure virtual I think
    //
    virtual bool addDevice(Device::DeviceType,
                           DeviceId,
                           InstrumentId,
                           MidiDevice::DeviceDirection);
    virtual void removeDevice(DeviceId);
    virtual void removeAllDevices();
    virtual void renameDevice(DeviceId, QString);

    // Some more that are generic from a GUI perspective and should be pure virtual
    //
    virtual unsigned int getConnections(Device::DeviceType type,
                                        MidiDevice::DeviceDirection direction);
    virtual QString getConnection(Device::DeviceType type,
                                  MidiDevice::DeviceDirection direction,
                                  unsigned int connectionNo);
    virtual QString getConnection(DeviceId id);

    // Should be PVs
    //
    virtual void startClocks();
    virtual void stopClocks();

    /*
    virtual void setConnection(DeviceId deviceId, QString connection);
    virtual void setPlausibleConnection(DeviceId deviceId,
                                        QString connection,
                                        bool recordDevice = false);
    virtual void connectSomething();
    */

    //virtual void processNotesOff(const RealTime &time, bool now, bool everything = false);


    // Find the
    int getOutputPortForMappedInstrument(InstrumentId id);

    // Get the current high res system time - also used by the MidiDriver thread so public
    //
    static RealTime getSystemTime();

    // RtMidi handles - get the RtMidiIn/Out port associated with the index
    //
    RtMidiIn* getRtMidiIn(unsigned int portNum);
    RtMidiOut* getRtMidiOut(unsigned int portNum);

    // Check and create a RtMidiOut port if it doesn't exist
    //
    void checkRtMidiOut(unsigned int portNum);

    // Check and create a RtMidiIn port if it doesn't exist
    //
    void checkRtMidiIn(unsigned int portNum);

protected:
    // Helper functions
    //
    virtual void processMidiOut(const MappedEventList &mC,
                                const RealTime &sliceStart,
                                const RealTime &sliceEnd);
    virtual void generateFixedInstruments();

    MappedDevice* createMidiDevice(DeviceId deviceId,
                                   MidiDevice::DeviceDirection reqDirection);

    void addInstrumentsForDevice(MappedDevice *device, InstrumentId base);

    std::vector<RtMidiIn*>    m_midiInPorts;
    std::vector<RtMidiOut*>   m_midiOutPorts;

    RealTime    m_systemStartTime;

    MidiThread  *m_midiThread;

    MappedEvent *m_tempOutBuffer;

    int         m_bufferSize;

    // Also defined in AlsaDriver
    //
    RealTime                     m_loopStartTime;
    RealTime                     m_loopEndTime;
    bool                         m_looping;

    // Also also defined in AlsaDriver
    //
    typedef std::map<DeviceId, int> DeviceIntMap;
    DeviceIntMap                 m_outputPorts;
};

}

#endif // _PORTABLESOUNDDRIVER_H

