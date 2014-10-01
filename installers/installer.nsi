#===================================================================================================================
#
#    Copyright (c) 2012 Leap Motion. All rights reserved.
#
#  The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
#  protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
#  strictly forbidden unless prior written permission is obtained from Leap Motion.
#
#===================================================================================================================
#
# /// Author(s)
# /// James Donald

SetCompressor /FINAL /SOLID lzma
SetCompressorDictSize 32
RequestExecutionLevel highest

#Include Externals
!include "x64.nsh"
!include "MUI2.nsh"
!include "LogicLib.nsh"
!include "FileFunc.nsh"
!include "WinVer.nsh"
!include "UAC.nsh"
!include "REG_MultiSZ.nsh"

#Leap macros
!include "macros.nsh"

!cd ..

# This will be in the installer/uninstaller's title bar
Name "Touchless For Windows"
OutFile "TouchlessForWindows_LM.exe"
BrandingText "Leap Motion Touchless"
Icon source/TouchlessUI/touchless-icon.ico
!define VersionNumber 9113
!define RegKeyLocation "SOFTWARE\Leap Motion\Touchless"
!define AppIdentifier "Touchless"

!define MultiTouchDriverKey   "SYSTEM\CurrentControlSet\services\HidEmulatorKmdf\Enum"
!define MultiTouchDriverDWORD "Count"

#Sets the default install folder
InstallDir "$PROGRAMFILES\Leap Motion"

#Get the install folder from the registry if available.
InstallDirRegKey HKLM "${RegKeyLocation}" ""

Var /GLOBAL INST_ERR
Var ShouldUnpackDriver
Var ShouldInstallDriver
Var ShouldInstallTouchless

requestExecutionLevel user # required for UAC plugin

#--------------------------------
#Functions
Function .onInit  
  SetSilent silent

  StrCpy $ShouldUnpackDriver    "false"
  StrCpy $ShouldInstallDriver   "false"
  StrCpy $ShouldInstallTouchless "true"

  #The installer needs to detect if multitouch driver is already installed.

  ${If} ${IsWin7}
    StrCpy $ShouldUnpackDriver "true"
    StrCpy $ShouldInstallDriver "true"
    ClearErrors
    ReadRegDWORD $0 HKLM "${MultiTouchDriverKey}" "${MultiTouchDriverDWORD}"
    
    ${IfNot} ${Errors}
      ${If} $0 > 0 #if there is a fake multitouch device enumerated the driver is installed
        StrCpy $ShouldInstallDriver "false"    
      ${EndIf}
    ${EndIf}
    
    ${If} ${FileExists} "$INSTDIR\Touchless For Windows\MultiTouch\InstallerApp.exe" #make sure they didn't just delete the folder.
      StrCpy $ShouldUnpackDriver "false"
    ${Endif}
  ${EndIf}

  #The installer needs to detect if touchless.exe is already installed.

  #If yes - check installed version against installer version.
  #if installer version is newer - install new touchless.
  #if installer version same or older - launch touchless.  
  ClearErrors
  ReadRegDWORD $0 HKLM "${RegKeyLocation}" "Version"
  
  ${IfNot} ${Errors}  
    ${If} ${VersionNumber} > $0 #if the new version is greater than the installed one, wipe the old & install the new
        StrCpy $ShouldInstallTouchless "true"
    ${Else}
        StrCpy $ShouldInstallTouchless "false"
    ${EndIf}
      
    ${IfNot} ${FileExists} "$INSTDIR\Touchless For Windows\Touchless.exe" #make sure they didn't just delete the folder.
        StrCpy $ShouldInstallTouchless "true"
        ${If} ${IsWin7}
          StrCpy $ShouldUnpackDriver "true"    
        ${Endif}
    ${EndIf}
  ${EndIf}
  
  #MessageBox MB_OK "ShouldUnpackDriver=$ShouldUnpackDriver ShouldInstallDriver=$ShouldInstallDriver ShouldInstallTouchless=$ShouldInstallTouchless"
  
  uac_tryagain:
    !insertmacro UAC_RunElevated
    
    ${Switch} $0
    ${Case} 0
      ${If} $1 = 1
        SetErrorLevel $2
        #MessageBox MB_OK "ErrorLevel = $2"
          Quit
      ${EndIf}
            ;we are the outer process, the inner process has done its work, we are done
      ${IfThen} $3 <> 0 ${|} ${Break} ${|} ;we are admin, let the show go on
      ${If} $1 = 3 ;RunAs completed successfully, but with a non-admin user
        MessageBox mb_YesNo|mb_IconExclamation|mb_TopMost|mb_SetForeground "This application requires admin privileges, try again" /SD IDNO IDYES uac_tryagain IDNO 0
      ${EndIf}
      ;fall-through and die
    ${Case} 1223
      #MessageBox mb_IconStop|mb_TopMost|mb_SetForeground "This ${thing} requires admin privileges, aborting!"
      Quit
    ${Case} 1062
      #MessageBox mb_IconStop|mb_TopMost|mb_SetForeground "Logon service not running, aborting!"
      Quit
    ${Default}
      MessageBox mb_IconStop|mb_TopMost|mb_SetForeground "Unable to elevate, error: $0"
      Quit
    ${EndSwitch}
	
	${DebugInit} #this must come AFTER uac elevation or it won't work.
FunctionEnd

Function .onInstSuccess
  ${DebugClose}
FunctionEnd

Function .onInstFailed
  SetErrorLevel 259
  ${DebugClose}
FunctionEnd

Function onAbort
  SetErrorLevel 1602 #HP Required exit codes
  ${DebugClose}
FunctionEnd

#------------------------------
#Pages
!define MUI_ICON source/TouchlessUI/touchless-icon.ico

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

!define MUI_CUSTOMFUNCTION_ABORT onAbort

#--------------------------------
#Sections

Section "Install"

!cd contrib
  #MSVC is required by the driver installer app - install these first thing.
  ${If} ${RunningX64}
      SetRegView 64
      ClearErrors
      ReadRegDWORD $R0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\{1D8E6291-B0D5-35EC-8441-6616F567A0F7}" "Version"
      ${If} ${Errors}
        !insertmacro InstallRedistributable 2010 x64
      ${EndIf}
      SetRegView lastused
  ${Else}
    ClearErrors
    ReadRegDWORD $R0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\{F0C3E5D1-1ADE-321E-8167-68EF0DE699A5}" "Version"
    ${If} ${Errors}
      !insertmacro InstallRedistributable 2010 x86
    ${EndIf}
  ${EndIf}
!cd ..

  ${If} $ShouldInstallTouchless == "true"
    !insertmacro EnsureFileIsUnused "Touchless.exe"
    RMDir /r "$INSTDIR\Touchless For Windows"
  ${EndIf}
  
  #MessageBox MB_OK "installing touchless"
   
  ${If} $ShouldUnpackDriver == "true"
    #MessageBox MB_OK "installing multitouch driver"
    SetOutPath "$INSTDIR\Touchless For Windows"
    ${If} ${RunningX64}
      File /r "drivers\x64\MultiTouch"
    ${Else}
      File /r "drivers\x86\MultiTouch"
    ${EndIf}
  ${EndIf}

  ${If} $ShouldInstallDriver == "true"
    ${DebugDetail} "Installing Windows 7 MultiTouch driver..."
    ClearErrors
    nsExec::execToLog '"$INSTDIR\Touchless For Windows\MultiTouch\InstallerApp.exe"'
    Pop $R0  #return value
    ${DebugDetail} "Multitouch driver install returned code:  $R0"
    ${If} $R0 L= 0x60010000
      ${DebugDetail} "Multitouch driver updated, reboot required for full functionality."
      SetRebootFlag true
    ${ElseIf} ${Errors}
    ${OrIf} $R0 L> 0x80000000  #negative (or >0x80000000 in unsigned notation) are errors.
    ${AndIfNot} ${Silent}
      MessageBox MB_OKCANCEL|MB_ICONSTOP "The Multitouch driver failed to install properly. Press OK to continue, or Cancel to abort the installation. error code:  $R0" IDCANCEL earlyAbort
    ${EndIf}
  ${EndIf}
  
  ${If} $ShouldInstallTouchless == "true"
    SetOutPath "$INSTDIR"

    File /r "Touchless For Windows"
    WriteUninstaller "$INSTDIR\Touchless For Windows\Uninstall Touchless For Windows.exe"
    WriteRegDWORD HKLM "${RegKeyLocation}" "Version" ${VersionNumber}
    
     # Registry information for add/remove programs
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPIDENTIFIER}" "DisplayName" "Touchless For Windows"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPIDENTIFIER}" "UninstallString" "$\"$INSTDIR\Touchless For Windows\Uninstall Touchless For Windows.exe$\""
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPIDENTIFIER}" "QuietUninstallString" "$\"$INSTDIR\Touchless For Windows\Uninstall Touchless For Windows.exe$\" /S"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPIDENTIFIER}" "InstallLocation" "$\"$INSTDIR$\""
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPIDENTIFIER}" "DisplayIcon" "$\"$INSTDIR\Touchless For Windows\Touchless.exe$\""
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPIDENTIFIER}" "Publisher" "Leap Motion"
    #WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPIDENTIFIER}" "HelpLink" "${HELPURL}"
    #WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPIDENTIFIER}" "URLUpdateInfo" "${UPDATEURL}"
    #WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPIDENTIFIER}" "URLInfoAbout" "${ABOUTURL}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPIDENTIFIER}" "DisplayVersion" "${VersionNumber}.0.0"
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPIDENTIFIER}" "VersionMajor" ${VersionNumber}
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPIDENTIFIER}" "VersionMinor" 0
    # There is no option for modifying or repairing the install
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPIDENTIFIER}" "NoModify" 1
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPIDENTIFIER}" "NoRepair" 1
    # Set the INSTALLSIZE constant (!defined at the top of this script) so Add/Remove Programs can accurately report the size
    ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
    IntFmt $0 "0x%08X" $0
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPIDENTIFIER}" "EstimatedSize" "$0"
  ${EndIf}
  
  runTouchless:
  #MessageBox MB_OK "running touchless"
  ExecShell "open" "$INSTDIR\Touchless For Windows\Touchless.exe"
  Goto Done

  earlyAbort:
  Quit
  
Done:

SectionEnd

Function un.onInit
  
uac_tryagain:
  !insertmacro UAC_RunElevated
  
  ${Switch} $0
  ${Case} 0
  	${If} $1 = 1
  	  SetErrorLevel $2
  	  #MessageBox MB_OK "ErrorLevel = $2"
        Quit
  	${EndIf}
        	;we are the outer process, the inner process has done its work, we are done
  	${IfThen} $3 <> 0 ${|} ${Break} ${|} ;we are admin, let the show go on
  	${If} $1 = 3 ;RunAs completed successfully, but with a non-admin user
  		MessageBox mb_YesNo|mb_IconExclamation|mb_TopMost|mb_SetForeground "This application requires admin privileges, try again" /SD IDNO IDYES uac_tryagain IDNO 0
  	${EndIf}
  	;fall-through and die
  ${Case} 1223
  	#MessageBox mb_IconStop|mb_TopMost|mb_SetForeground "This ${thing} requires admin privileges, aborting!"
  	Quit
  ${Case} 1062
  	#MessageBox mb_IconStop|mb_TopMost|mb_SetForeground "Logon service not running, aborting!"
  	Quit
  ${Default}
  	MessageBox mb_IconStop|mb_TopMost|mb_SetForeground "Unable to elevate, error: $0"
  	Quit
  ${EndSwitch}
  
  ${DebugInit} # this must come AFTER UAC elevation or it won't work right.
FunctionEnd

Function un.onInstSuccess
  ${DebugClose}
FunctionEnd

Function un.onInstFailed
  ${DebugClose}
FunctionEnd

Section "Uninstall"
  #kills running processes
  !insertmacro EnsureFileIsUnused "Touchless.exe"

  uninstallMultitouch:
    ${If} ${IsWin7}
      ${DebugDetail} "Uninstalling Windows 7 MultiTouch driver..."
      SetOutPath "$INSTDIR\MultiTouch"
      ${If} ${FileExists} "$INSTDIR\MultiTouch\InstallerApp.exe"
        ClearErrors
        nsExec::execToLog '"$INSTDIR\MultiTouch\InstallerApp.exe" uninstall'
        Pop $INST_ERR
        ${IfNot} $INST_ERR = 0
          ${DebugDetail} "Multitouch uninstall returned error code: $INST_ERR"
          ${If} $INST_ERR L= 0x60010000L
            SetRebootFlag true
          ${EndIf}
        ${EndIf}
      ${Else}
        #MessageBox MB_OK|MB_ICONSTOP "$INSTDIR\MultiTouch\InstallerApp.exe does not exist."      
        MessageBox MB_OK|MB_ICONSTOP "The Leap Motion gesture emulation device driver could not be uninstalled. It will need to be removed manually via Device Manager"      
      ${EndIf}
    ${EndIf}

  SetOutPath "$INSTDIR\.."

  RMDir /r "$INSTDIR"
  DeleteRegKey HKLM "${RegKeyLocation}"

SectionEnd
