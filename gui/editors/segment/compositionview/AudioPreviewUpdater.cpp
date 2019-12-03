/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- vi:set ts=8 sts=4 sw=4: */

/*
    Rosegarden
    A MIDI and audio sequencer and musical notation editor.
    Copyright 2000-2015 the Rosegarden development team.
 
    Other copyrights also apply to some parts of this work.  Please
    see the AUTHORS file and individual file headers for details.
 
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#define RG_MODULE_STRING "[AudioPreviewUpdater]"

#include "AudioPreviewUpdater.h"
#include "AudioPreviewThread.h"
#include "AudioPreviewReadyEvent.h"
#include "CompositionModelImpl.h"

#include "misc/Debug.h"
#include "base/Composition.h"
#include "base/RealTime.h"
#include "base/Segment.h"

#include <QEvent>
#include <QObject>
#include <QRect>


namespace Rosegarden
{

static int apuExtantCount = 0;

AudioPreviewUpdater::AudioPreviewUpdater(AudioPreviewThread &thread,
        const Composition& c, const Segment* s,
        const QRect& r,
        CompositionModelImpl* parent)
        : QObject(parent),
        m_thread(thread),
        m_composition(c),
        m_segment(s),
        m_rect(r),
        m_showMinima(false),
        m_channels(0),
        m_previewToken( -1)
{
    ++apuExtantCount;
    RG_DEBUG << "AudioPreviewUpdater::AudioPreviewUpdater " << this << " (now " << apuExtantCount << " extant)" << endl;
}

AudioPreviewUpdater::~AudioPreviewUpdater()
{
    --apuExtantCount;
    RG_DEBUG << "AudioPreviewUpdater::~AudioPreviewUpdater on " << this << " ( token " << m_previewToken << ") (now " << apuExtantCount << " extant)" << endl;
    if (m_previewToken >= 0)
        m_thread.cancelPreview(m_previewToken);
}

void AudioPreviewUpdater::update()
{
    // Get sample start and end times and work out duration
    //
    RealTime audioStartTime = m_segment->getAudioStartTime();
    RealTime audioEndTime = audioStartTime +
                            m_composition.getElapsedRealTime(m_segment->getEndMarkerTime()) -
                            m_composition.getElapsedRealTime(m_segment->getStartTime()) ;

    RG_DEBUG << "AudioPreviewUpdater(" << this << ")::update() - for file id "
    << m_segment->getAudioFileId() << " requesting values - thread running : "
    << m_thread.isRunning() << " - thread finished : " << m_thread.isFinished() << endl;

    AudioPreviewThread::Request request;
    request.audioFileId = m_segment->getAudioFileId();
    request.audioStartTime = audioStartTime;
    request.audioEndTime = audioEndTime;
    request.width = m_rect.width();
    request.showMinima = m_showMinima;
    request.notify = this;

    if (m_previewToken >= 0) m_thread.cancelPreview(m_previewToken);
    m_previewToken = m_thread.requestPreview(request);

    if (!m_thread.isRunning()) m_thread.start();
}

void AudioPreviewUpdater::cancel()
{
    if (m_previewToken >= 0) m_thread.cancelPreview(m_previewToken);
    m_previewToken = -1;
}

bool AudioPreviewUpdater::event(QEvent *e)
{
    
    RG_DEBUG << "AudioPreviewUpdater(" << this << ")::event (" << e << ")" << endl;

    if (e->type() == AudioPreviewThread::AudioPreviewReady) {
        AudioPreviewReadyEvent *ev = dynamic_cast<AudioPreviewReadyEvent *>(e);
        if (ev) {
            int token = (int)ev->data();
            m_channels = 0; // to be filled as getPreview return value

            RG_DEBUG << "AudioPreviewUpdater::token " << token << ", my token " << m_previewToken << endl;

            if (m_previewToken >= 0 && token >= m_previewToken) {

                m_previewToken = -1;
                m_thread.getPreview(token, m_channels, m_values);

                if (m_channels == 0) {
                    RG_DEBUG << "AudioPreviewUpdater: failed to find preview!\n";
                } else {

                    RG_DEBUG << "AudioPreviewUpdater: got correct preview (" << m_channels
                    << " channels, " << m_values.size() << " samples)\n";
                }

                emit audioPreviewComplete(this);

            } else {

                // this one is out of date already
                std::vector<float> tmp;
                unsigned int tmpChannels;
                m_thread.getPreview(token, tmpChannels, tmp);

                RG_DEBUG << "AudioPreviewUpdater: got obsolete preview (" << tmpChannels
                << " channels, " << tmp.size() << " samples)\n";
            }

            return true;
        }
    }

    return QObject::event(e);

}

}
#include "moc_AudioPreviewUpdater.cpp"
