
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

#ifndef RG_DECOUNTERPOINTCOMMAND_H
#define RG_DECOUNTERPOINTCOMMAND_H

#include "document/BasicSelectionCommand.h"
#include <QString>
#include <QCoreApplication>


class Overlapping;


namespace Rosegarden
{

class Segment;
class EventSelection;
class CommandRegistry;


class DeCounterpointCommand : public BasicSelectionCommand
{
    Q_DECLARE_TR_FUNCTIONS(Rosegarden::DeCounterpointCommand)

public:
    DeCounterpointCommand(EventSelection &selection) :
        BasicSelectionCommand(getGlobalName(), selection, true),
        m_selection(&selection) { }

    DeCounterpointCommand(Segment &segment) :
        BasicSelectionCommand(getGlobalName(), segment, true),
        m_selection(nullptr) { }

    static QString getGlobalName() { return tr("Split-and-Tie Overlapping &Chords"); }

    static void registerCommand(CommandRegistry *r);

protected:
    void modifySegment() override;

private:
    EventSelection *m_selection;// only used on 1st execute (cf bruteForceRedo)
};
  


}

#endif
