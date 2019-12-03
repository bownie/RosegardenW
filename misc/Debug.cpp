/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- vi:set ts=8 sts=4 sw=4: */

/*
    Rosegarden
    A sequencer and musical notation editor.
    Copyright 2000-2018 the Rosegarden development team.
    See the AUTHORS file for more details.
 
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "Debug.h"

#include "Strings.h"

#include "base/Event.h"
#include "base/Segment.h"
#include "base/RealTime.h"
#include "base/Colour.h"
#include "gui/editors/guitar/Chord.h"
#include "gui/editors/guitar/Fingering.h"

namespace Rosegarden
{


ROSEGARDENPRIVATE_EXPORT QDebug &operator<<(QDebug &dbg, const std::string &s)
{
    dbg << strtoqstr(s);

    return dbg;
}

ROSEGARDENPRIVATE_EXPORT QDebug &operator<<(QDebug &dbg, const Rosegarden::Event &e)
{
    dbg << "Event type : " << e.getType() << '\n';
    dbg << "\tDuration : " << e.getDuration() << '\n';
    dbg << "\tAbsolute Time : " << e.getAbsoluteTime() << '\n';

    //     for (Event::PropertyMap::const_iterator i = e.properties().begin();
    //          i != e.properties().end(); ++i) {
    //         dbg << "\t\t" << (*i).first << " : "
    //             << ((*i).second)->unparse() << '\n';
    //     }

    //     e.dump(std::cerr);

    return dbg;
}

ROSEGARDENPRIVATE_EXPORT QDebug &operator<<(QDebug &dbg, const Rosegarden::Segment &t)
{
//    dbg << "Segment for instrument " << t.getTrack()
//        << " starting at " << t.getStartTime() << '\n';

    dbg << "Segment Object\n";
    dbg << "  Label: " << t.getLabel() << '\n';
    dbg << "  Track: " << t.getTrack() << '\n';
    // Assume 4/4 time and provide a potentially helpful bar number.
    dbg << "  Start Time: " << t.getStartTime() << 
        "(4/4 bar" << t.getStartTime() / (960.0*4.0) + 1 << ")\n";
    dbg << "  End Time: " << t.getEndTime() << 
        "(4/4 bar" << t.getEndTime() / (960.0*4.0) + 1 << ")\n";
    dbg << "  End Marker Time: " << t.getEndMarkerTime() << 
        "(4/4 bar" << t.getEndMarkerTime() / (960.0*4.0) + 1 << ")\n";

    dbg << "Events:\n";

    for (Rosegarden::Segment::const_iterator i = t.begin();
            i != t.end(); ++i) {
        if (!(*i)) {
            dbg << "WARNING : skipping null event ptr\n";
            continue;
        }

        dbg << *(*i) << endl;
    }

    return dbg;
}

ROSEGARDENPRIVATE_EXPORT QDebug &operator<<(QDebug &dbg, const Rosegarden::RealTime &t)
{
    dbg << t.toString();
    return dbg;
}

ROSEGARDENPRIVATE_EXPORT QDebug &operator<<(QDebug &dbg, const Rosegarden::Colour &c)
{
    dbg << "Colour : rgb = " << c.getRed() << "," << c.getGreen() << "," << c.getBlue();
    return dbg;
}

ROSEGARDENPRIVATE_EXPORT QDebug &operator<<(QDebug &dbg, const Rosegarden::Guitar::Chord &c)
{
    dbg << "Chord root = " << c.getRoot() << ", ext = '" << c.getExt() << "'";

//    for(unsigned int i = 0; i < c.getNbFingerings(); ++i) {
//        dbg << "\nFingering " << i << " : " << c.getFingering(i).toString().c_str();
//    }
    
     Rosegarden::Guitar::Fingering f = c.getFingering();

     dbg << ", fingering : ";

     for(unsigned int j = 0; j < 6; ++j) {
         int pos = f[j];
         if (pos >= 0)
             dbg << pos << ' ';
         else
             dbg << "x ";
    }        
    return dbg;
}

}
