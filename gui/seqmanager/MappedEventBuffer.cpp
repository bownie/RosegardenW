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

#define RG_MODULE_STRING "[MappedEventBuffer]"

#include "MappedEventBuffer.h"

#include "document/RosegardenDocument.h"
#include "misc/Debug.h"
#include "sound/MappedEvent.h"
#include "sound/MappedInserterBase.h"

// #define DEBUG_MAPPED_EVENT_BUFFER 1

namespace Rosegarden
{

MappedEventBuffer::MappedEventBuffer(RosegardenDocument *doc) :
    m_buffer(nullptr),
    m_capacity(0),
    m_size(0),
    m_doc(doc),
    m_start(RealTime::zeroTime),
    m_end(RealTime::beforeMaxTime),
    m_refCount(0)
{
}

MappedEventBuffer::~MappedEventBuffer()
{
    // Safe even if nullptr.
    delete[] m_buffer;
}

void
MappedEventBuffer::init()
{
    int size = calculateSize();

    if (size > 0) {
        reserve(size);

        RG_DEBUG << "init() : size = " << size;

        initSpecial();
        fillBuffer();
    } else {
        RG_DEBUG << "init() : mmap size = 0 - skipping mmapping for now";
    }
}

bool
MappedEventBuffer::refresh()
{
    bool resized = false;

    int newFill = calculateSize();
    int oldSize = capacity();

#ifdef DEBUG_MAPPED_EVENT_BUFFER    
    RG_DEBUG << "refresh() - " << this
                 << " - old size = " << oldSize
                 << " - old fill = " << size()
                 << " - new fill = " << newFill;
#endif

    // If we need to expand the buffer to hold the events
    if (newFill > oldSize) {
        resized = true;
        reserve(newFill);
    }

    // Ask the deriver to fill the buffer from the document
    fillBuffer();

    return resized;
}

int
MappedEventBuffer::capacity() const
{
    return m_capacity.fetchAndAddRelaxed(0);
}

int
MappedEventBuffer::size() const
{
    return m_size.fetchAndAddRelaxed(0);
}

void
MappedEventBuffer::reserve(int newSize)
{
    if (newSize <= capacity())  return;

    MappedEvent *oldBuffer = m_buffer;
    MappedEvent *newBuffer = new MappedEvent[newSize];

    if (oldBuffer) {
        for (int i = 0; i < m_size.fetchAndAddRelaxed(0); ++i) {
            newBuffer[i] = m_buffer[i];
        }
    }

    {
        QWriteLocker locker(&m_lock);
        m_buffer = newBuffer;
        m_capacity.fetchAndStoreRelease(newSize);
    }

#ifdef DEBUG_MAPPED_EVENT_BUFFER
    SEQUENCER_DEBUG << "MappedEventBuffer::reserve: Resized to " << newSize << " events";
#endif

    delete[] oldBuffer;
}

void
MappedEventBuffer::resize(int newFill)
{
    m_size.fetchAndStoreRelaxed(newFill);
}

void
MappedEventBuffer::
mapAnEvent(MappedEvent *e)
{
    if (size() >= capacity()) {
        // We need a bigger buffer.  We scale by 1.5, a compromise
        // between allocating too often and wasting too much space.
        // We also add 1 in case the space didn't increase due to
        // rounding.
        int newSize = 1 + float(capacity()) * 1.5;
        reserve(newSize);
    }

    getBuffer()[size()] = e;
    // Some mappers need this to be done now because they may resize
    // the buffer later, which will only copy the filled part.
    resize(size() + 1);
}

void
MappedEventBuffer::
doInsert(MappedInserterBase &inserter, MappedEvent &evt,
         RealTime /*start*/, bool /*firstOutput*/)
{
    inserter.insertCopy(evt);
}

// The default doesn't have to do anything to get ready.
void
MappedEventBuffer::
makeReady(MappedInserterBase &/*inserter*/, RealTime /*time*/) {}

    /*** MappedEventBuffer::iterator ***/

MappedEventBuffer::iterator::iterator(QSharedPointer<MappedEventBuffer> s) :
    m_s(s),
    m_index(0),
    m_ready(false),
    m_active(false)
    //m_currentTime
{
    //RG_DEBUG << "iterator ctor";
}

// ++prefix
MappedEventBuffer::iterator &
MappedEventBuffer::iterator::operator++()
{
    int fill = m_s->size();
    if (m_index < fill)  ++m_index;
    return *this;
}

// postfix++
MappedEventBuffer::iterator
MappedEventBuffer::iterator::operator++(int)
{
    // This line is the main reason we need a copy ctor.
    iterator r = *this;
    int fill = m_s->size();
    if (m_index < fill)  ++m_index;
    return r;
}

MappedEventBuffer::iterator &
MappedEventBuffer::iterator::operator+=(int offset)
{
    int fill = m_s->size();
    if (m_index + offset <= fill) {
        m_index += offset;
    } else {
        m_index = fill;
    }
    return *this;
}

MappedEventBuffer::iterator &
MappedEventBuffer::iterator::operator-=(int offset)
{
    if (m_index > offset)
        m_index -= offset;
    else
        m_index = 0;

    return *this;
}

bool MappedEventBuffer::iterator::operator==(const iterator& it)
{
    return (m_index == it.m_index) && (m_s == it.m_s);
}

void MappedEventBuffer::iterator::reset()
{
    m_index = 0;
}

MappedEvent
MappedEventBuffer::iterator::operator*()
{
    const MappedEvent *e = peek();

    if (e)
        return *e;
    else
        return MappedEvent();
}

MappedEvent *
MappedEventBuffer::iterator::peek() const
{
    // The lock formerly here has moved out to callers.

    // If we're at the end, return nullptr
    if (m_index >= m_s->size())
        return nullptr;

    // Otherwise return a pointer into the buffer.
    return &m_s->m_buffer[m_index];
}

bool
MappedEventBuffer::iterator::atEnd() const
{
    int size = m_s->size();
    return (m_index >= size);
}

void
MappedEventBuffer::iterator::
doInsert(MappedInserterBase &inserter, MappedEvent &evt)
{
    // Get time when note starts sounding, eg for finding the correct
    // controllers.  It can't simply be event time, because if we
    // jumped into the middle of a long note, we'd wrongly find the
    // controller values as they are at the time the note starts.
    if (evt.getEventTime() > m_currentTime)
        m_currentTime = evt.getEventTime();

    // Mapper does the actual insertion.
    getSegment()->doInsert(inserter, evt, m_currentTime, !isReady());
    setReady(true);
}

}
