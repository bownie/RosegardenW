Rosegarden 12.04.99 "WinFreedom" Alpha 3 Release for Windows
============================================================

I'm pleased to finally announce the availability of the next Alpha release of Rosegarden for Windows.

This is a working release of Rosegarden for Windows - it's still called an Alpha release because it may
well be buggy and audio file support or audio sequencing is still not supported in any way.  The code base has
been merged with the latest Rosegarden trunk changes ("Freedom") as per 7th July 2012 but also:


  - MIDI Playback devices are now assignable (create new Midi Device and assign tracks)

  - Most common MIDI playback events are now supported (no System Exclusives)

  - MIDI Record now works for most events (no System Exclusives)

  - RtMidi updated to latest version (July 2012)

  - Merges from latest trunk release (please see 12.04.00 "Freedom" release notes)


Rosegarden for Windows is a packaged installation for 32-bit Windows which will work on 32-bit or 64-bit
Windows platforms and has been tested on Windows XP and Windows 7.  Rosegarden for Windows packages Lilypond
printing and print-previewing out of the box.

You can download Rosegarden for Windows from here (*):


  https://bitbucket.org/bownie/rosegarden-for-windows/downloads


Rosegarden for Windows Alpha 2 has had over 5000 downloads in the last nine or so months but so far feedback
has been limited - hopefully that can change a bit with this latest, more functional release.  It would be great
to hear any feedback - good, bad or indifferent - just to work out if I should spend any more time adding features
no matter how slowly!

Please give it a try and provide me or the rosegarden-devel mailing list with any feedback.  Thanks goes of course
to all the core developers on the Rosegarden team who are pumping out the actual features and improvements and
hopefully this version can continue to do them some justice.

Regards,
Richard Bown (@xyglo)
September 2012

(* - Sourceforge file upload didn't want to work for some reason but perhaps the files/links could also be updated there )


Websites
========

The main Rosegarden website is still at:

  http://www.rosegardenmusic.com

There is also a page here with latest Rosegarden for Windows specific information:

  http://www.xyglo.com/rosegarden-for-windows/


Repository Notes
================

The Mercurial repository at BitBucket is now in the lead:

  https://bitbucket.org/bownie/rosegarden-for-windows

The old branch in SourceForge is now trailing and can be deleted if required:

  http://rosegarden.svn.sourceforge.net/viewvc/rosegarden/branches/win32-mingw-rtmidi/
