;-------------------------------
; Rosegarden NSIS Installer
;
; Richard Bown
; November 2019
;-------------------------------
!include FontReg.nsh
!include FontName.nsh
!include WinMessages.nsh

; Request application privileges for Windows Vista/7 etc
;
RequestExecutionLevel admin

; We're using the modern UI
;
!include "MUI.nsh"

; The name of the installer
Name "RG-win32-alpha-1906"
Caption "Rosegarden Windows32 Alpha Build 1906"

!define icon "icon.ico"
!define COMPANY "Rosegarden"
!define SOFTWARE "Rosegarden"

; The file to write
OutFile "rosegarden-win32-alpha-1906.exe"

; The default installation directory
;
InstallDir $PROGRAMFILES\${COMPANY}\${SOFTWARE}

; Registry key to check for directory (so if you install again, it will
; overwrite the old one automatically)
;
InstallDirRegKey HKLM "Software\${COMPANY}\${SOFTWARE}" "Install_Dir"

; Application icon
;
Icon "rg-rwb-rose3-128x128.ico"

; MUI stuff
;
!insertmacro MUI_PAGE_LICENSE "COPYING.txt"
!insertmacro MUI_LANGUAGE "English"

;--------------------------------

; Pages

Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

; The stuff to install
Section "Rosegarden"

    SectionIn RO

    ; Set output path to the installation directory.
    SetOutPath $INSTDIR

    File "COPYING.txt"
    File "README.txt"
    File "README-linux.txt"
    File "AUTHORS.txt"

    ; The files we are building into the package
    ;
    File "rosegarden.exe"
    ;File "icudt52.dll"
    ;File "icuin52.dll"
    ;File "icuuc52.dll"
    File "libgcc_s_dw2-1.dll"
    File "libstdc++-6.dll"
    ;File "libwinpthread-1.dll"
    File "Qt5Core.dll"
    File "Qt5Gui.dll"
    File "Qt5Network.dll"
    File "Qt5PrintSupport.dll"
    File "Qt5Svg.dll"
    File "Qt5Widgets.dll"
    File "Qt5Xml.dll"
    File "ca.qm"
	File "cs.qm"
	File "cy.qm"
    File "de.qm"
	File "en.qm"
	File "en_GB.qm"
	File "en_US.qm"
	File "es.qm"
	File "et.qm"
	File "eu.qm"
    File "fi.qm"
	File "fr.qm"
	File "id.qm"
    File "it.qm"
	File "ja.qm"
	File "nl.qm"
	File "pl.qm"
	File "pt_BR.qm"
    File "ru.qm"
    File "sv.qm"
    File "zh_CN.qm"
	File "rosegarden.qm"
    ;File "zlib1.dll"

    ;File /r "accessible"
    ;File /r "bearer"
    ;File /r "iconengines"
    ;File /r "imageformats"
    ;File /r "platforms"
    ;File /r "printsupport"

    ; More resources
    ;
    File "rg-rwb-rose3-128x128.ico"

    ; Write the installation path into the registry
    WriteRegStr HKLM "Software\${COMPANY}\${SOFTWARE}" "Install_Dir" "$INSTDIR"
    WriteRegStr HKCR "Rosegarden\DefaultIcon" "" "$INSTDIR\rg-rwb-rose3-128x128.ico"

    ; Write the uninstall keys for Windows
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Rosegarden" "DisplayName" "Rosegarden"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Rosegarden" "UninstallString" '"$INSTDIR\uninstall.exe"'

    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Rosegarden" "NoModify" 1
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Rosegarden" "NoRepair" 1
    WriteUninstaller "uninstall.exe"

SectionEnd

Section "Fonts"
    ; Using the FontName package this is very easy - works out the install path for us and
    ; we just need to specify the file name of the fonts.
    ;

    ; Copy the FONTS variable into FONT_DIR
    ;
    StrCpy $FONT_DIR $FONTS

    ; Remove and then install fonts
    ;
    !insertmacro RemoveTTFFont "GNU-LilyPond-feta-design20.ttf"
    !insertmacro RemoveTTFFont "GNU-LilyPond-feta-nummer-10.ttf"
    !insertmacro RemoveTTFFont "GNU-LilyPond-parmesan-20.ttf"

    !insertmacro InstallTTFFont "fonts\GNU-LilyPond-feta-design20.ttf"
    !insertmacro InstallTTFFont "fonts\GNU-LilyPond-feta-nummer-10.ttf"
    !insertmacro InstallTTFFont "fonts\GNU-LilyPond-parmesan-20.ttf"

    ; Complete font registration without reboot
    ;
    SendMessage ${HWND_BROADCAST} ${WM_FONTCHANGE} 0 0 /TIMEOUT=5000

SectionEnd


; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

    CreateDirectory "$SMPROGRAMS\Rosegarden"
    CreateShortCut "$SMPROGRAMS\Rosegarden\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
    ;CreateShortCut "$SMPROGRAMS\Rosegarden\Rosegarden.lnk" "$INSTDIR\rosegarden.exe" "" "$INSTDIR\garderobe.nsi" 0
    CreateShortCut "$SMPROGRAMS\Rosegarden\Rosegarden.lnk" "$INSTDIR\rosegarden.exe" "" "$INSTDIR\rg-rwb-rose3-128x128.ico"

SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"

    ; Remove registry keys
    ;
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Rosegarden"
    DeleteRegKey HKLM "Software\${COMPANY}\${SOFTWARE}"

    Delete "$INSTDIR\COPYING.txt"
    Delete "$INSTDIR\README.txt"
    Delete "$INSTDIR\README-linux.txt"
    Delete "$INSTDIR\AUTHORS.txt"

    ; Remove files and uninstaller
    ;
    Delete $INSTDIR\*uninstall.exe
    Delete "$INSTDIR\rosegarden.exe"
    ;Delete "$INSTDIR\icudt52.dll"
    ;Delete "$INSTDIR\icuin52.dll"
    ;Delete "$INSTDIR\icuuc52.dll"
    Delete "$INSTDIR\libgcc_s_dw2-1.dll"
    Delete "$INSTDIR\libstdc++-6.dll"
    ;Delete "$INSTDIR\libwinpthread-1.dll"
    Delete "$INSTDIR\Qt5Core.dll"
    Delete "$INSTDIR\Qt5Gui.dll"
    Delete "$INSTDIR\Qt5Network.dll"
    Delete "$INSTDIR\Qt5PrintSupport.dll"
    Delete "$INSTDIR\Qt5Svg.dll"
    Delete "$INSTDIR\Qt5Widgets.dll"
    Delete "$INSTDIR\Qt5Xml.dll"
	Delete "$INSTDIR\ca.qm"
    Delete "$INSTDIR\cs.qm"
	Delete "$INSTDIR\cy.qm"
    Delete "$INSTDIR\de.qm"
	Delete "$INSTDIR\en.qm"
	Delete "$INSTDIR\en_GB.qm"
	Delete "$INSTDIR\en_US.qm"
	Delete "$INSTDIR\es.qm"
	Delete "$INSTDIR\et.qm"
	Delete "$INSTDIR\eu.qm"
    Delete "$INSTDIR\fi.qm"
    Delete "$INSTDIR\fr.qm"
	Delete "$INSTDIR\id.qm"
    Delete "$INSTDIR\it.qm"
    Delete "$INSTDIR\ja.qm"
	Delete "$INSTDIR\nl.qm"
	Delete "$INSTDIR\pl.qm"
	Delete "$INSTDIR\pt_BR.qm"
    Delete "$INSTDIR\ru.qm"
    Delete "$INSTDIR\sv.qm"
    Delete "$INSTDIR\zh_CN.qm"
	Delete "$INSTDIR\rosegarden.qm"
    ;Delete "$INSTDIR\zlib1.dll"

    Delete "$INSTDIR\application.rc"
    Delete "$INSTDIR\rg-rwb-rose3-128x128.ico"

    ; Remove shortcuts, if any
    Delete "$SMPROGRAMS\Rosegarden\*.*"
    Delete "$INSTDIR\Rosegarden\*.*"

    ; Remove the data directory and subdirs
    ;
    RMDir /r "$INSTDIR\accessible"
    RMDir /r "$INSTDIR\bearer"
    RMDir /r "$INSTDIR\iconengines"
    RMDir /r "$INSTDIR\imageformats"
    RMDir /r "$INSTDIR\platforms"
    RMDir /r "$INSTDIR\printsupport"

    Delete "$INSTDIR\accessible"
    Delete "$INSTDIR\bearer"
    Delete "$INSTDIR\iconengines"
    Delete "$INSTDIR\imageformats"
    Delete "$INSTDIR\platforms"
    Delete "$INSTDIR\printsupport"

    ; Remove directories used
    ;
    RMDir "$SMPROGRAMS\Rosegarden"
    RMDir "$INSTDIR"

SectionEnd

