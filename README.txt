Rosegarden 11.11.42 Alpha 2 Release for Windows
===============================================

This is the second package of a Windows build of Rosegarden.  Improvments you'll see this time are:

- Recent merge from main branch so most of the 11.11.42 features should be available.
- Localised language support is working.
- All support files from the regular rosegarden distrubtion as well as all necessary DLLs are now packaged.
- Fonts are installed automatically.
- If you also install Lilypond then Print Previewing/Printing is working.
- Path problems should be fixed.

Notes:

- Installation goes to the $PROGRAMFILE\Xyglo\Rosegarden directory.
- rosegarden.exe executable rather than garderobe.exe as in previous version.
- When Rosegarden runs it creates an audio folder under AppData profile (Roaming) called 'rosegarden'.
- Under the HOMEPATH directory a .local/share/rosegarden folder is created also
- Under registry key HKEY_USERS\YOUR_USER_ID\Software\rosegardenmusic you get the configuration information.
- Temporary directory for example:  C:\Users\bownr\AppData\Local\Temp


Limitations

- Lilypond export creates a few temporary files which cannot currently be removed by the software so these will build up.
- No improvements or changes to MIDI code at this release.

Cheers,
Richard Bown
December 2012