Rosegarden 15.08 "Nostalgia" Alpha 5 Release for Windows
========================================================

Work in progress.  Merge not completed.

Rosegarden for Windows is a port of the Linux application "Rosegarden" - a notation editor and MIDI and audio sequencer.   This port has slightly more limited functionality that the Linux version - no audio support, limited MIDI support - but should offer a working platform for composition, layout and printing and some MIDI usage.

This release is a long overdue catch up of features plus a port to Qt5 and offers no new working MIDI or audio functionality above the previous Rosegarden for Windows release.  However you will benefit from all the latest Rosegarden trunk features.

As of previous release the MIDI subsystem was as follows:

  - MIDI Playback devices are now assignable (create new Midi Device and assign tracks)

  - Most common MIDI playback events are now supported (no System Exclusives)

  - MIDI Record now works for most events (no System Exclusives)

  - RtMidi updated to latest version (July 2012)

  - Merges from latest trunk release (please see 12.04.00 "Freedom" release notes)


Rosegarden for Windows is a packaged installation for 32-bit Windows which will work on 32-bit or 64-bit Windows platforms however the build is *** not completely working yet **

The windows binary will be delivered as soon as I've worked out the problems with 32-bit builds on Windows 8.1.  In the meantime this git repository holds the merged code which should build on Windows with a little patience!


Regards,
Richard Bown (@xyglo)
August 2015


Websites
========

The main Rosegarden website with details of the latest functionality can be found here:

  http://www.rosegardenmusic.com

There is also a page here with latest Rosegarden for Windows specific information here:

  http://www.xyglo.com/rosegarden-for-windows/

Repository Notes
================

The trunk repository for Rosegarden (Linux) can be found here:

  http://www.sf.net/projects/rosegarden

Rosegarden for Windows respository is hosted on github for two reasons - I prefer git and the directory structure is different for Rosegarden for Windows:

  https://github.com/bownie/RosegardenW/

