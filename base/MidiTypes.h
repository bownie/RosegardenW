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

#ifndef RG_MIDI_TYPES_H
#define RG_MIDI_TYPES_H

#include <list>

#include "Event.h"
#include "Instrument.h"

// Internal representation of some very MIDI specific event types
// that fall clearly outside of NotationTypes and still require
// representation.
//


namespace Rosegarden 
{

class MIDIValueOutOfRange : public Exception {
public:
    MIDIValueOutOfRange(std::string name) :
        Exception("Value of " + name + " out of byte range") { }
    MIDIValueOutOfRange(std::string name, std::string file, int line) :
        Exception("Value of " + name + " out of byte range", file, line) { }
};


// Rosegarden's internal represetation of MIDI PitchBend
//
class PitchBend
{
public:
    static const std::string EventType;
    static const int EventSubOrdering;

    static const PropertyName MSB;
    static const PropertyName LSB;

    PitchBend(MidiByte msb, MidiByte lsb);
    PitchBend(const Event &);
    ~PitchBend();

    MidiByte getMSB() const { return m_msb; }
    MidiByte getLSB() const { return m_lsb; }
    
    /// Returned event is on heap; caller takes responsibility for ownership
    Event *getAsEvent(timeT absoluteTime) const;

private:
    MidiByte m_msb;
    MidiByte m_lsb;
};


// Controller
//

class Controller
{
public:
    static const std::string EventType;
    static const int EventSubOrdering;

    static const PropertyName NUMBER;  // controller number
    static const PropertyName VALUE;   // and value

    Controller(MidiByte number,
               MidiByte value);

    Controller(const Event &);
    ~Controller();

    MidiByte getNumber() const { return m_number; }
    MidiByte getValue() const { return m_value; }

    /// Returned event is on heap; caller takes responsibility for ownership
    Event *getAsEvent(timeT absoluteTime) const;

private:
    MidiByte    m_number;
    MidiByte    m_value;

};


// Key pressure
//

class KeyPressure
{
public:
    static const std::string EventType;
    static const int EventSubOrdering;

    static const PropertyName PITCH;
    static const PropertyName PRESSURE;

    KeyPressure(MidiByte pitch, MidiByte pressure);
    KeyPressure(const Event &event);
    ~KeyPressure();

    MidiByte getPitch() const { return m_pitch; }
    MidiByte getPressure() const { return m_pressure; }

    /// Returned event is on heap; caller takes responsibility for ownership
    Event *getAsEvent(timeT absoluteTime) const;

private:
    MidiByte m_pitch;
    MidiByte m_pressure;
};


// Channel pressure
//

class ChannelPressure
{
public:
    static const std::string EventType;
    static const int EventSubOrdering;

    static const PropertyName PRESSURE;

    ChannelPressure(MidiByte pressure);
    ChannelPressure(const Event &event);
    ~ChannelPressure();

    MidiByte getPressure() const { return m_pressure; }

    /// Returned event is on heap; caller takes responsibility for ownership
    Event *getAsEvent(timeT absoluteTime) const;

private:
    MidiByte m_pressure;
};


// Program Change
//

class ProgramChange
{
public:
    static const std::string EventType;
    static const int EventSubOrdering;

    static const PropertyName PROGRAM;

    ProgramChange(MidiByte program);
    ProgramChange(const Event &event);
    ~ProgramChange();

    MidiByte getProgram() const { return m_program; }

    /// Returned event is on heap; caller takes responsibility for ownership
    Event *getAsEvent(timeT absoluteTime) const;

private:
    MidiByte m_program;
};


// System exclusive
//

class SystemExclusive
{
public:
    static const std::string EventType;
    static const int EventSubOrdering;

    struct BadEncoding : public Exception {
        BadEncoding() : Exception("Bad SysEx encoding") { }
    };

    static const PropertyName DATABLOCK;

    SystemExclusive(std::string rawData);
    SystemExclusive(const Event &event);
    ~SystemExclusive();

    std::string getRawData() const { return m_rawData; }
    std::string getHexData() const { return toHex(m_rawData); }

    /// Returned event is on heap; caller takes responsibility for ownership
    Event *getAsEvent(timeT absoluteTime) const;

    static std::string toHex(std::string rawData);
    static std::string toRaw(std::string hexData);
    static bool isHex(std::string data);

private:
    std::string m_rawData;
    static unsigned char toRawNibble(char);
};



}


#endif
