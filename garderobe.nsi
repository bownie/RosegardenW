;-------------------------------
; Rosegarden NSIS Installer
;
; Richard Bown
; June 2014
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
Name "RG-win32-alpha-4"
Caption "Rosegarden Windows32 Alpha Build 4"

!define icon "icon.ico"
!define COMPANY "Xylgo"
!define SOFTWARE "Rosegarden"

; The file to write
OutFile "rosegarden-win32-alpha-4.exe"

; The default installation directory
;
InstallDir $PROGRAMFILES\${COMPANY}\${SOFTWARE}

; Registry key to check for directory (so if you install again, it will
; overwrite the old one automatically)
;
InstallDirRegKey HKLM "Software\${COMPANY}\${SOFTWARE}" "Install_Dir"

; Application icon
;
Icon "data\pixmaps\icons\rg-rwb-rose3-128x128.ico"

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
    File "release\rosegarden.exe"
    File "..\resources\libgcc_s_dw2-1.dll"
    File "..\resources\mingwm10.dll"
    File "..\resources\QtCore4.dll"
    File "..\resources\QtGui4.dll"
    File "..\resources\QtNetwork4.dll"
    File "..\resources\QtXml4.dll"
    File "..\resources\zlib1.dll"
    File "..\resources\pthreadGC2.dll"

    ; More resources
    ;
    File "data\pixmaps\icons\rg-rwb-rose3-128x128.ico"

    File /r "data"

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

    !insertmacro InstallTTFFont "data\fonts\GNU-LilyPond-feta-design20.ttf"
    !insertmacro InstallTTFFont "data\fonts\GNU-LilyPond-feta-nummer-10.ttf"
    !insertmacro InstallTTFFont "data\fonts\GNU-LilyPond-parmesan-20.ttf"

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
    Delete $INSTDIR\uninstall.exe
    Delete "$INSTDIR\rosegarden.exe"
    Delete "$INSTDIR\libgcc_s_dw2-1.dll"
    Delete "$INSTDIR\mingwm10.dll"
    Delete "$INSTDIR\QtCore4.dll"
    Delete "$INSTDIR\QtGui4.dll"
    Delete "$INSTDIR\QtNetwork4.dll"
    Delete "$INSTDIR\QtXml4.dll"
    Delete "$INSTDIR\zlib1.dll"
    Delete "$INSTDIR\pthreadGC2.dll"

    Delete "$INSTDIR\application.rc"
    Delete "$INSTDIR\rg-rwb-rose3-128x128.ico"

    ; Remove shortcuts, if any
    Delete "$SMPROGRAMS\Rosegarden\*.*"
    Delete "$INSTDIR\Rosegarden\*.*"

    ; Remove the data directory and subdirs
    ;
    RMDir /r "$INSTDIR\data"
    Delete "$INSTDIR\data"

    ; Remove directories used
    ;
    RMDir "$SMPROGRAMS\Rosegarden"
    RMDir "$INSTDIR"

SectionEnd

