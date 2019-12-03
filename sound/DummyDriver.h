/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- vi:set ts=8 sts=4 sw=4: */

/*
    Rosegarden
    A MIDI and audio sequencer and musical notation editor.
    Copyright 2000-2018 the Rosegarden development team.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "SoundDriver.h"

// An empty sound driver for when we don't want sound support
// but still want to build the sequencer.
//

#ifndef RG_DUMMYDRIVER_H
#define RG_DUMMYDRIVER_H

namespace Rosegarden
{

class DummyDriver : public SoundDriver
{
public:
    DummyDriver(MappedStudio *studio);
    DummyDriver(MappedStudio *studio, QString pastLog);
    ~DummyDriver() override { }

    bool initialise()  override { return true; }
    void initialisePlayback(const RealTime & /*position*/) override { }
    void stopPlayback() override { }
    void punchOut() override { }
    void resetPlayback(const RealTime & /*old position*/,
                               const RealTime & /*position*/) override { }
    void allNotesOff()  override { }
    
    RealTime getSequencerTime() override { return RealTime(0, 0);}

    bool getMappedEventList(MappedEventList &) override { return true; }

    void processEventsOut(const MappedEventList & /*mC*/) override { }

    void processEventsOut(const MappedEventList &,
                                  const RealTime &,
                                  const RealTime &) override { }

    // Activate a recording state
    //
    bool record(RecordStatus /*recordStatus*/,
                        const std::vector<InstrumentId> */*armedInstruments = 0*/,
                        const std::vector<QString> */*audioFileNames = 0*/) override
        { return false; }

    // Process anything that's pending
    //
    void processPending() override { }

    // Sample rate
    //
    unsigned int getSampleRate() const override { return 0; }

    // Return the last recorded audio level
    //
    virtual float getLastRecordedAudioLevel() { return 0.0; }

    // Plugin instance management
    //
    void setPluginInstance(InstrumentId /*id*/,
                                   QString /*pluginIdent*/,
                                   int /*position*/) override { }

    void removePluginInstance(InstrumentId /*id*/,
                                      int /*position*/) override { }

    void removePluginInstances() override { }

    void setPluginInstancePortValue(InstrumentId /*id*/,
                                            int /*position*/,
                                            unsigned long /*portNumber*/,
                                            float /*value*/) override { }

    float getPluginInstancePortValue(InstrumentId ,
                                             int ,
                                             unsigned long ) override { return 0; }

    void setPluginInstanceBypass(InstrumentId /*id*/,
                                         int /*position*/,
                                         bool /*value*/) override { }

    QStringList getPluginInstancePrograms(InstrumentId ,
                                                  int ) override { return QStringList(); }

    QString getPluginInstanceProgram(InstrumentId,
                                             int ) override { return QString(); }

    QString getPluginInstanceProgram(InstrumentId,
                                             int,
                                             int,
                                             int) override { return QString(); }

    unsigned long getPluginInstanceProgram(InstrumentId,
                                                   int ,
                                                   QString) override { return 0; }
    
    void setPluginInstanceProgram(InstrumentId,
                                          int ,
                                          QString ) override { }

    QString configurePlugin(InstrumentId,
                                    int,
                                    QString ,
                                    QString ) override { return QString(); }

    void setAudioBussLevels(int ,
                                    float ,
                                    float ) override { }

    void setAudioInstrumentLevels(InstrumentId,
                                          float,
                                          float) override { }

    bool checkForNewClients() override { return false; }

    void setLoop(const RealTime &/*loopStart*/,
                         const RealTime &/*loopEnd*/) override { }

    QString getStatusLog() override;

    virtual std::vector<PlayableAudioFile*> getPlayingAudioFiles()
        { return std::vector<PlayableAudioFile*>(); }

    void getAudioInstrumentNumbers(InstrumentId &i, int &n) override {
        i = 0; n = 0;
    }
    void getSoftSynthInstrumentNumbers(InstrumentId &i, int &n) override {
        i = 0; n = 0;
    }

    void claimUnwantedPlugin(void */* plugin */) override { }
    void scavengePlugins() override { }

    bool areClocksRunning() const override { return true; }

protected:
    void processMidiOut(const MappedEventList & /*mC*/,
                                const RealTime &, const RealTime &) override { }
    void generateFixedInstruments()  override { }

    QString m_pastLog;
};

}

#endif // RG_DUMMYDRIVER_H

