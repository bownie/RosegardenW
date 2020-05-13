## Rosegarden 16.09 "Xerxes" Alpha Release for Windows

![CI](https://github.com/bownie/RosegardenW/workflows/CI/badge.svg)

Rosegarden for Windows is a partial port of the Linux application "Rosegarden" - a notation editor and MIDI and audio sequencer.
This port has reduced functionality compared to the full Linux version.  There is no audio support, limited MIDI support but it should offer a working platform for composition, layout and printing and some MIDI usage.

Please see the 16.09 release notes for list of total functionality but bear in mind limitations above and it is expected that this software is unstable.

As of previous release the MIDI subsystem was as follows:

  - MIDI Playback devices are now assignable (create new Midi Device and assign tracks)

  - Most common MIDI playback events are now supported (no System Exclusives)

  - MIDI Record now works for most events (no System Exclusives)

  - RtMidi updated to latest version (July 2012)

  - Merges from latest trunk release (please see 12.04.00 "Freedom" release notes)


Richard Bown (@bownie29)
Amstelveen, May 2020


## Websites

The main Rosegarden website with details of the latest functionality can be found here:

  http://www.rosegardenmusic.com

## Repository Notes

The trunk repository for Rosegarden (Linux) can be found here:

  http://www.sf.net/projects/rosegarden

Rosegarden for Windows respository is hosted on github for two reasons - I prefer git and the directory structure is different for Rosegarden for Windows:

  https://github.com/bownie/RosegardenW/

