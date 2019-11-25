/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- vi:set ts=8 sts=4 sw=4: */

/*
    Rosegarden
    A MIDI and audio sequencer and musical notation editor.
    Copyright 2000-2018 the Rosegarden development team.
 
    Other copyrights also apply to some parts of this work.  Please
    see the AUTHORS file and individual file headers for details.
 
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#define RG_MODULE_STRING "[AudioPluginOSCGUIManager]"

#include "AudioPluginOSCGUIManager.h"

#include "sound/Midi.h"
#include "misc/Debug.h"
#include "misc/Strings.h"
#include "AudioPluginOSCGUI.h"
#include "base/AudioPluginInstance.h"
#include "base/Exception.h"
#include "base/Instrument.h"
#include "base/MidiProgram.h"
#include "base/RealTime.h"
#include "base/Studio.h"
#include "gui/application/RosegardenMainWindow.h"
#include "OSCMessage.h"
#include "sound/MappedEvent.h"
#include "sound/PluginIdentifier.h"
#include "StudioControl.h"
#include "TimerCallbackAssistant.h"

#include <QString>

//#include <lo/lo.h>
#include <unistd.h>

namespace Rosegarden
{

static void osc_error(int num, const char *msg, const char *path)
{
    std::cerr << "Rosegarden: ERROR: liblo server error " << num
              << " in path " << path << ": " << msg << std::endl;
}
/*
static int osc_message_handler(const char *path, const char *types, lo_arg **argv,
                               int argc, lo_message, void *user_data)
{
    AudioPluginOSCGUIManager *manager = (AudioPluginOSCGUIManager *)user_data;

    InstrumentId instrument;
    int position;
    QString method;

    if (!manager->parseOSCPath(path, instrument, position, method)) {
        return 1;
    }

    OSCMessage *message = new OSCMessage();
    message->setTarget(instrument);
    message->setTargetData(position);
    message->setMethod(qstrtostr(method));

    int arg = 0;
    while (types && arg < argc && types[arg]) {
        message->addArg(types[arg], argv[arg]);
        ++arg;
    }

    manager->postMessage(message);
    return 0;
}
*/

AudioPluginOSCGUIManager::AudioPluginOSCGUIManager(RosegardenMainWindow *mainWindow) :
        m_mainWindow(mainWindow),
        m_studio(nullptr),
        m_haveOSCThread(false),
        m_oscBuffer(1023),
        m_dispatchTimer(nullptr)
{}

AudioPluginOSCGUIManager::~AudioPluginOSCGUIManager()
{
    delete m_dispatchTimer;

    for (TargetGUIMap::iterator i = m_guis.begin(); i != m_guis.end(); ++i) {
        for (IntGUIMap::iterator j = i->second.begin(); j != i->second.end();
             ++j) {
            delete j->second;
        }
    }
    m_guis.clear();

   // if (m_haveOSCThread)
     //   lo_server_thread_stop(m_serverThread);
}

void
AudioPluginOSCGUIManager::checkOSCThread()
{
    if (m_haveOSCThread)
        return ;
/*
    m_serverThread = lo_server_thread_new(nullptr, osc_error);

    lo_server_thread_add_method(m_serverThread, nullptr, nullptr,
                                osc_message_handler, this);

    lo_server_thread_start(m_serverThread);

    RG_DEBUG << "checkOSCThread(): Base OSC URL is " << lo_server_thread_get_url(m_serverThread);

    m_dispatchTimer = new TimerCallbackAssistant(20, timerCallback, this);
*/
    m_haveOSCThread = true;
}

bool
AudioPluginOSCGUIManager::hasGUI(InstrumentId instrument, int position)
{
    PluginContainer *container = nullptr;
    container = m_studio->getContainerById(instrument);
    if (!container) return false;

    AudioPluginInstance *pluginInstance = container->getPlugin(position);
    if (!pluginInstance) return false;

    try {
        QString filePath = AudioPluginOSCGUI::getGUIFilePath
                           (strtoqstr(pluginInstance->getIdentifier()));
        return ( !filePath.isEmpty() );
    } catch (const Exception &e) { // that's OK
        return false;
    }
}

void
AudioPluginOSCGUIManager::startGUI(InstrumentId instrument, int position)
{
    RG_DEBUG << "startGUI(): " << instrument << "," << position;

    checkOSCThread();

    if (m_guis.find(instrument) != m_guis.end() &&
            m_guis[instrument].find(position) != m_guis[instrument].end()) {
        RG_DEBUG << "startGUI(): stopping GUI first";
        stopGUI(instrument, position);
    }

    // check the label
    PluginContainer *container = nullptr;
    container = m_studio->getContainerById(instrument);
    if (!container) {
        RG_WARNING << "startGUI(): no such instrument or buss as " << instrument;
        return;
    }

    AudioPluginInstance *pluginInstance = container->getPlugin(position);
    if (!pluginInstance) {
        RG_WARNING << "startGUI(): no plugin at position " << position << " for instrument " << instrument;
        return ;
    }

    try {
        AudioPluginOSCGUI *gui =
            new AudioPluginOSCGUI(pluginInstance,
                                  getOSCUrl(instrument,
                                            position,
                                            strtoqstr(pluginInstance->getIdentifier())),
                                  getFriendlyName(instrument,
                                                  position,
                                                  strtoqstr(pluginInstance->getIdentifier())));
        m_guis[instrument][position] = gui;

    } catch (const Exception &e) {

        RG_WARNING << "startGUI(): failed to start GUI: " << e.getMessage();
    }
}

void
AudioPluginOSCGUIManager::showGUI(InstrumentId instrument, int position)
{
    RG_DEBUG << "showGUI(): " << instrument << "," << position;

    if (m_guis.find(instrument) != m_guis.end() &&
        m_guis[instrument].find(position) != m_guis[instrument].end()) {
        m_guis[instrument][position]->show();
    } else {
        startGUI(instrument, position);
    }
}

void
AudioPluginOSCGUIManager::stopGUI(InstrumentId instrument, int position)
{
    if (m_guis.find(instrument) != m_guis.end() &&
        m_guis[instrument].find(position) != m_guis[instrument].end()) {
        delete m_guis[instrument][position];
        m_guis[instrument].erase(position);
        if (m_guis[instrument].empty())
            m_guis.erase(instrument);
    }
}

void
AudioPluginOSCGUIManager::stopAllGUIs()
{
    while (!m_guis.empty()) {
        while (!m_guis.begin()->second.empty()) {
            delete (m_guis.begin()->second.begin()->second);
            m_guis.begin()->second.erase(m_guis.begin()->second.begin());
        }
        m_guis.erase(m_guis.begin());
    }
}

void
AudioPluginOSCGUIManager::postMessage(OSCMessage *message)
{
    RG_DEBUG << "postMessage()";

    m_oscBuffer.write(&message, 1);
}

void
AudioPluginOSCGUIManager::updateProgram(InstrumentId instrument, int position)
{
    RG_DEBUG << "updateProgram(" << instrument << "," << position << ")";

    if (m_guis.find(instrument) == m_guis.end() ||
        m_guis[instrument].find(position) == m_guis[instrument].end())
        return ;

    PluginContainer *container = nullptr;
    container = m_studio->getContainerById(instrument);
    if (!container) return;

    AudioPluginInstance *pluginInstance = container->getPlugin(position);
    if (!pluginInstance) return;

    unsigned long rv = StudioControl::getPluginProgram
                       (pluginInstance->getMappedId(),
                        strtoqstr(pluginInstance->getProgram()));

    int bank = rv >> 16;
    int program = rv - (bank << 16);

    RG_DEBUG << "updateProgram(" << instrument << "," << position << "): rv " << rv << ", bank " << bank << ", program " << program;

    m_guis[instrument][position]->sendProgram(bank, program);
}

void
AudioPluginOSCGUIManager::updatePort(InstrumentId instrument, int position,
                                     int port)
{
    RG_DEBUG << "updatePort(" << instrument << "," << position << "," << port << ")";

    if (m_guis.find(instrument) == m_guis.end() ||
        m_guis[instrument].find(position) == m_guis[instrument].end())
        return ;

    PluginContainer *container = nullptr;
    container = m_studio->getContainerById(instrument);
    if (!container) return;

    AudioPluginInstance *pluginInstance = container->getPlugin(position);
    if (!pluginInstance)
        return ;

    PluginPortInstance *porti = pluginInstance->getPort(port);
    if (!porti)
        return ;

    RG_DEBUG << "updatePort(" << instrument << "," << position << "," << port << "): value " << porti->value;

    m_guis[instrument][position]->sendPortValue(port, porti->value);
}

void
AudioPluginOSCGUIManager::updateConfiguration(InstrumentId instrument, int position,
        QString key)
{
    RG_DEBUG << "updateConfiguration(" << instrument << "," << position << "," << key << ")";

    if (m_guis.find(instrument) == m_guis.end() ||
            m_guis[instrument].find(position) == m_guis[instrument].end())
        return ;

    PluginContainer *container = m_studio->getContainerById(instrument);
    if (!container) return;

    AudioPluginInstance *pluginInstance = container->getPlugin(position);
    if (!pluginInstance) return;

    QString value = strtoqstr(pluginInstance->getConfigurationValue(qstrtostr(key)));

    RG_DEBUG << "updateConfiguration(" << instrument << "," << position << "," << key << "): value " << value;

    m_guis[instrument][position]->sendConfiguration(key, value);
}

QString
AudioPluginOSCGUIManager::getOSCUrl(InstrumentId instrument, int position,
                                    QString identifier)
{
    // OSC URL will be of the form
    //   osc.udp://localhost:54343/plugin/dssi/<instrument>/<position>/<label>
    // where <position> will be "synth" for synth plugins

    QString type, soName, label;
    PluginIdentifier::parseIdentifier(identifier, type, soName, label);

    /*
    QString baseUrl = lo_server_thread_get_url(m_serverThread);
    if (!baseUrl.endsWith("/"))
        baseUrl += '/';
*/
    QString url = QString("%1%2/%3/%4/%5/%6")
                  .arg("baseUrl")
                  .arg("plugin")
                  .arg(type)
                  .arg(instrument);

    if (position == int(Instrument::SYNTH_PLUGIN_POSITION)) {
        url = url.arg("synth");
    } else {
        url = url.arg(position);
    }

    url = url.arg(label);

    return url;
}

bool
AudioPluginOSCGUIManager::parseOSCPath(QString path, InstrumentId &instrument,
                                       int &position, QString &method)
{
    RG_DEBUG << "parseOSCPath(" << path << ")";

    if (!m_studio)
        return false;

    QString pluginStr("/plugin/");

    if (path.startsWith("//")) {
        path = path.right(path.length() - 1);
    }

    if (!path.startsWith(pluginStr)) {
        RG_WARNING << "parseOSCPath(): malformed path " << path;
        return false;
    }

    path = path.right(path.length() - pluginStr.length());

    QString type = path.section('/', 0, 0);
    QString instrumentStr = path.section('/', 1, 1);
    QString positionStr = path.section('/', 2, 2);
    QString label = path.section('/', 3, -2);
    method = path.section('/', -1, -1);

    if (instrumentStr.isEmpty() || positionStr.isEmpty() ) {
        RG_WARNING << "parseOSCPath(): no instrument or position in " << path;
        return false;
    }

    instrument = instrumentStr.toUInt();

    if (positionStr == "synth") {
        position = Instrument::SYNTH_PLUGIN_POSITION;
    } else {
        position = positionStr.toInt();
    }

    // check the label
    PluginContainer *container = m_studio->getContainerById(instrument);
    if (!container) {
        RG_WARNING << "parseOSCPath(): no such instrument or buss as " << instrument << " in path " << path;
        return false;
    }

    AudioPluginInstance *pluginInstance = container->getPlugin(position);
    if (!pluginInstance) {
        RG_WARNING << "parseOSCPath(): no plugin at position " << position << " for instrument " << instrument << " in path " << path;
        return false;
    }

    QString identifier = strtoqstr(pluginInstance->getIdentifier());
    QString iType, iSoName, iLabel;
    PluginIdentifier::parseIdentifier(identifier, iType, iSoName, iLabel);
    if (iLabel != label) {
        RG_WARNING << "parseOSCPath(): wrong label for plugin at position " << position << " for instrument " << instrument << " in path " << path << " (actual label is " << iLabel << ")";
        return false;
    }

    RG_DEBUG << "parseOSCPath(): good path " << path << ", got mapped id " << pluginInstance->getMappedId();

    return true;
}

QString
AudioPluginOSCGUIManager::getFriendlyName(InstrumentId instrument, int position,
        QString)
{
    PluginContainer *container = m_studio->getContainerById(instrument);
    if (!container)
        return tr("Rosegarden Plugin");
    else {
        if (position == int(Instrument::SYNTH_PLUGIN_POSITION)) {
            return tr("Rosegarden: %1").arg(strtoqstr(container->getAlias()));
        } else {
            return tr("Rosegarden: %1: %2").arg(strtoqstr(container->getAlias()))
                    .arg(tr("Plugin slot %1").arg(position + 1));
        }
    }
}

void
AudioPluginOSCGUIManager::timerCallback(void *data)
{
    AudioPluginOSCGUIManager *manager = (AudioPluginOSCGUIManager *)data;
    manager->dispatch();
}

void
AudioPluginOSCGUIManager::dispatch()
{
    if (!m_studio)
        return ;

}

}

