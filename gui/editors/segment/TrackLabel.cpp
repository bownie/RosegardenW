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


#include "TrackLabel.h"

#include "misc/Debug.h"
#include "gui/dialogs/TrackLabelDialog.h"

#include <QFont>
#include <QFrame>
#include <QString>
#include <QTimer>
#include <QWidget>
#include <QMouseEvent>

namespace Rosegarden
{

TrackLabel::TrackLabel(TrackId id,
                       int position,
                       QWidget *parent) :
        QLabel(parent),
        m_mode(ShowTrack),
        m_forcePresentationName(false),
        m_id(id),
        m_position(position)
{
    setObjectName("TrackLabel");
    
    QFont font;
    font.setPointSize(font.pointSize() * 95 / 100);
    if (font.pixelSize() > 14)
        font.setPixelSize(14);
    font.setBold(false);
    setFont(font);

    setFrameShape(QFrame::NoFrame);

    m_pressTimer = new QTimer(this);

    connect(m_pressTimer, &QTimer::timeout,
            this, &TrackLabel::changeToInstrumentList);

    setToolTip(tr("<qt>"
                  "<p>Click to select all the segments on this track.</p>"
                  "<p>Shift+click to add to or to remove from the"
                  " selection all the segments on this track.</p>"
                  "<p>Click and hold with either mouse button to assign"
                  " this track to an instrument.</p>"
                  "</qt>"));
    setSelected(false);
}

void
TrackLabel::updateLabel()
{
    if (m_forcePresentationName) {
        setText(m_presentationName);
        return;
    }

    if (m_mode == ShowTrack) {
        setText(m_trackName);
    } else if (m_mode == ShowInstrument) {
        if (m_programChangeName != "") {
            setText(m_programChangeName);
        } else {
            setText(m_presentationName);
        }
    }
}

void
TrackLabel::setSelected(bool selected)
{
    m_selected = selected;

    QPalette pal = palette();
    if (m_selected) {
        setAutoFillBackground(true);
        pal.setColor(QPalette::Window, QColor(0xAA, 0xAA, 0xAA));
        pal.setColor(QPalette::WindowText, Qt::white);
    } else {
        setAutoFillBackground(false);
        pal.setColor(QPalette::WindowText, Qt::black);
    }
    setPalette(pal);
}

void
TrackLabel::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::RightButton) {

        emit clicked();
        emit changeToInstrumentList();

    } else if (e->button() == Qt::LeftButton) {

        // start a timer on this hold
        m_pressTimer->setSingleShot(true);
        m_pressTimer->start(200); // 200ms, single shot
    }
}

void
TrackLabel::mouseReleaseEvent(QMouseEvent *e)
{
    // stop the timer if running
    if (m_pressTimer->isActive())
        m_pressTimer->stop();

    if (e->button() == Qt::LeftButton) {
        emit clicked();
    }
}

void
TrackLabel::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton)
        return ;

    // Highlight this label alone and cheat using
    // the clicked signal
    //
    emit clicked();

    // Just in case we've got our timing wrong - reapply
    // this label highlight
    //
    setSelected(true);


    TrackLabelDialog dlg(this,
                         tr("Change track name"),
                         tr("Enter new track name"),
                         m_trackName,
                         tr("<qt>The track name is also the notation staff name, eg. &quot;Trumpet.&quot;</qt>"),
                         tr("Enter short name"),
                         m_shortName,
                         tr("<qt>The short name is an alternate name that appears each time the staff system wraps, eg. &quot;Tr.&quot;</qt>")
                         );

    if (dlg.exec() == QDialog::Accepted) {

        QString longLabel = dlg.getPrimaryText();
        QString shortLabel = dlg.getSecondaryText();

        emit renameTrack(longLabel, shortLabel, m_id);
    }

}

}
