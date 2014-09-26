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
# /// Walter Gray
# This file is a collection of useful macros used by installer.nsi

Var debugFile
Var un.debugFile

!macro DebugPrint_init
  SetShellVarContext current
  CreateDirectory "$TEMP\LeapInstallerLogs"
  !ifndef __UNINSTALL__
    FileOpen $debugFile "$TEMP\LeapInstallerLogs\installlog.txt" w
  !else
    FileOpen $un.DebugFile "$TEMP\LeapInstallerLogs\uninstalllog.txt" w
  !endif
!macroend

!macro DebugPrint_close
  !ifndef __UNINSTAL__
    FileClose $debugFile
  !else
    FileClose $un.debugFile
  !endif
!macroend

!macro DebugPrint_macro str
  !ifndef __UNINSTALL__
    FileWrite $debugFile '${str}$\r$\n'
  !else
    FileWrite $un.debugFile '${str}$\r$\n'
  !endif
  ClearErrors
!macroend

!macro DebugPrint_file file
  File '${file}'
  ${If} ${Errors}
    ${DebugPrint} 'Error Writing file: ${file}'
	ClearErrors
  ${EndIf}
!macroend

!macro DebugPrint_fileEX ops file
  File ${ops} '${file}'
  ${If} ${Errors}
    ${DebugPrint} 'Error Writing file: ${file}'
	ClearErrors
  ${EndIf}
!macroend

!macro DebugPrint_detail str
  DetailPrint '${str}'
  ${DebugPrint} '${str}'
!macroend


!define DebugPrint `!insertmacro DebugPrint_macro`
!define DebugInit `!insertmacro DebugPrint_init`
!define DebugClose `!insertmacro DebugPrint_close`
!define DebugFile `!insertmacro DebugPrint_file`
!define DebugFileEx `!insertmacro DebugPrint_fileEX`
!define DebugDetail `!insertmacro DebugPrint_detail`


!macro LANG_LOAD LANGLOAD
  !insertmacro MUI_LANGUAGE "${LANGLOAD}"

  !include "Languages\${LANGLOAD}.nsh"
  !insertmacro LANG_INSERT_FILEVERSION #makes the compiler shut up about missing default strings

  !undef LANGFILE_IDNAME
!macroend

!macro LANG_STRING NAME VALUE
  LangString "${NAME}" "${LANG_${LANGFILE_IDNAME}}" "${VALUE}"
!macroend

!macro LANG_VERSIONKEY NAME VALUE
  VIAddVersionKey /LANG=${LANG_${LANGFILE_IDNAME}} "${NAME}" "${VALUE}"
  !if ${LANGFILE_IDNAME} == "English"
    VIAddVersionKey "${NAME}" "${VALUE}"
  !endif
!macroend

!macro LANG_INSERT_FILEVERSION
  !insertmacro LANG_VERSIONKEY FileVersion ${VERSIONMAJOR}.${VERSIONMINOR}.${VERSIONPATCH}
  !insertmacro LANG_VERSIONKEY ProductVersion ${VERSIONMAJOR}.${VERSIONMINOR}.${VERSIONPATCH}.${BUILDNUMBER}
!macroend

!macro LANG_LICENSEFILE NAME
  LicenseLangString license "${LANG_${LANGFILE_IDNAME}}" ${NAME}
!macroend

#traditional chinese languages will fallback to simplified chinese without this, causing
#all manner of badness in the UI.  Insert in .onInit and un.onInit
#This forces HongKong & Macau to use the Traditional chinese localization instead of
#incorrectly falling back to simplified chinese like the normally would.
!macro HandleChineseLanguageFallbacks
  System::Call kernel32::GetUserDefaultUILanguage()i.r0
  ${If} $0 = 3076  #Chinese_Hong_Kong
  ${OrIf} $0 = 5124 #Chinese_Macau
    StrCpy $LANGUAGE 1028   #Chinese_Taiwan 
  ${EndIf}
!macroend

!macro VerifyUserIsAdmin
  UserInfo::GetAccountType
  Pop $0
  ${If} $0 != admin # Require admin rights on NT4+
    ${DebugPrint} "is not admin, $0"
    # For some reason this code gets called both at the start and end now that we use the UAC pugin
    #messageBox mb_iconstop "$(DESC_AdminRequiredErrorMessage)"
    #setErrorLevel 740 # ERROR_ELEVATION_REQUIRED
    Quit
  ${EndIf}
!macroend

; ################################################################
; appends \ to the path if missing
; example: !insertmacro GetCleanDir "c:\blabla"
; Pop $0 => "c:\blabla\"
!macro GetCleanDir INPUTDIR
  ; ATTENTION: USE ON YOUR OWN RISK!
  ; Please report bugs here: http://stefan.bertels.org/
  !define Index_GetCleanDir 'GetCleanDir_Line${__LINE__}'
  Push $R0
  Push $R1
  StrCpy $R0 "${INPUTDIR}"
  StrCmp $R0 "" ${Index_GetCleanDir}-finish
  StrCpy $R1 "$R0" "" -1
  StrCmp "$R1" "\" ${Index_GetCleanDir}-finish
  StrCpy $R0 "$R0\"
${Index_GetCleanDir}-finish:
  Pop $R1
  Exch $R0
  !undef Index_GetCleanDir
!macroend

; ################################################################
; similar to "RMDIR /r DIRECTORY", but does not remove DIRECTORY itself
; example: !insertmacro RemoveFilesAndSubDirs "$INSTDIR"
!macro RemoveFilesAndSubDirs DIRECTORY
  ; ATTENTION: USE ON YOUR OWN RISK!
  ; Please report bugs here: http://stefan.bertels.org/
  !define Index_RemoveFilesAndSubDirs 'RemoveFilesAndSubDirs_${__LINE__}'

  Push $R0
  Push $R1
  Push $R2

  !insertmacro GetCleanDir "${DIRECTORY}"
  Pop $R2
  FindFirst $R0 $R1 "$R2*.*"
${Index_RemoveFilesAndSubDirs}-loop:
  StrCmp $R1 "" ${Index_RemoveFilesAndSubDirs}-done
  StrCmp $R1 "." ${Index_RemoveFilesAndSubDirs}-next
  StrCmp $R1 ".." ${Index_RemoveFilesAndSubDirs}-next
  IfFileExists "$R2$R1\*.*" ${Index_RemoveFilesAndSubDirs}-directory
  ; file
  Delete "$R2$R1"
  goto ${Index_RemoveFilesAndSubDirs}-next
${Index_RemoveFilesAndSubDirs}-directory:
  ; directory
  RMDir /r "$R2$R1"
${Index_RemoveFilesAndSubDirs}-next:
  FindNext $R0 $R1
  Goto ${Index_RemoveFilesAndSubDirs}-loop
${Index_RemoveFilesAndSubDirs}-done:
  FindClose $R0

  Pop $R2
  Pop $R1
  Pop $R0
  !undef Index_RemoveFilesAndSubDirs
!macroend

!define FindProc_NOT_FOUND 1
!define FindProc_FOUND 0
!macro FindProc result processName
  ExecCmd::exec "%SystemRoot%\System32\tasklist /NH /FI $\"IMAGENAME eq ${processName}$\" | %SystemRoot%\System32\find /I $\"${processName}$\""
  Pop $0 ; The handle for the process
  ExecCmd::wait $0
  Pop ${result} ; The exit code
!macroend

Function GetInQuotes
  ; It seems we need this helper routine to call an uninstall.exe
  Exch $R0
  Push $R1
  Push $R2
  Push $R3

   StrCpy $R2 -1
   IntOp $R2 $R2 + 1
    StrCpy $R3 $R0 1 $R2
    StrCmp $R3 "" 0 +3
     StrCpy $R0 ""
     Goto Done
    StrCmp $R3 '"' 0 -5

   IntOp $R2 $R2 + 1
   StrCpy $R0 $R0 "" $R2

   StrCpy $R2 0
   IntOp $R2 $R2 + 1
    StrCpy $R3 $R0 1 $R2
    StrCmp $R3 "" 0 +3
     StrCpy $R0 ""
     Goto Done
    StrCmp $R3 '"' 0 -5

   StrCpy $R0 $R0 $R2
   Done:

  Pop $R3
  Pop $R2
  Pop $R1
  Exch $R0
FunctionEnd

#Macros can have named args, but using labels in them casues compile errors if they're used more than once...
!macro CheckForUninstaller RegKey
  Push "${RegKey}"
  Call CheckForUninstaller
!macroend
Function CheckForUninstaller

  Exch $R0 #RegKey
  Push $R1
  Push $R2

  StrCpy $R1 ""
  StrCpy $R2 ""
  ReadRegStr $R1 HKLM "$R0" "UninstallString"
  ReadRegStr $R2 HKLM "$R0" "InstallLocation"
  StrCmp $R1 "" skipUninstall
  #uninstalling the old version should not be optional.
  #MessageBox MB_YESNO "$(DESC_ExistingInstallMessage)" /SD IDYES IDNO skipUninstall

  Push $R2
  Call GetInQuotes
  Pop $R2
  ClearErrors

  ${If} ${FileExists} $R2
    ${DebugDetail} "$(DESC_Uninstalling)"
	
    ExecWait '$R1 /S /U _?=$R2' #Add the /U trigger to inform the uninstaller this is an upgrade
    ${IfNot} ${FileExists} $R2
      DeleteRegKey HKLM $R0 #Delete the entry since some of our older installers wouldn't clean up after themselves properly.
    ${EndIf}
  ${EndIf}
  
  skipUninstall:
  Pop $R2
  Pop $R1
  Pop $R0

FunctionEnd

!macro StopService ServiceName
  ${DebugDetail} "Stopping service ${ServiceName}..."
  nsExec::Exec 'net stop "Leap Service"'
  Pop $0
  ${If} $0 == "error"
    ${DebugDetail} "Error stopping service ${ServiceName}
  ${EndIf}

!macroend

!macro GrabFirst StringVar
  Push $0
  
  StrLen $0 ${StringVar}
  ${IfNot} $0 = 0
    Push $1
    Push $2
    
    StrCpy $0 "0" #which char to examine
    StrCpy $1 " "#storage of the char
    StrCpy $2 "notdone"
    ${While} $2 != "done" 
      StrCpy $1 "${StringVar}" 1 $0 #grab the next char
      ${If} $1 == " "
      ${OrIf} $1 == "$\n"
      ${OrIf} $1 == "$\r"
      ${OrIf} $1 == "$\t"
	    StrCpy $2 "done"
	  ${Else}
	    IntOp $0 $0 + 1 #i++
      ${EndIf}
    ${EndWhile}
    StrCpy ${StringVar} ${StringVar} $0
    
    Pop $2
    Pop $1
  ${EndIf}
  Pop $0
!macroend

!macro EnsureFileIsUnused ProcName
  Push $R0
  StrCpy $R0 "${ProcName}"
  ${DebugDetail} "$(DESC_CheckingForProcess)"
  StrCpy $R0 "${ProcName}" "" -4
  ${If} $R0 == ".dll"
    nsExec::ExecToStack '"tasklist" /NH /M ${ProcName}'
    Pop $0
    Pop $R0
    ${DebugDetail} Output=$R0
	
  ${ElseIf} $R0 == ".exe" #It's not a dll, so presume it's an exe
    #Check to see if it's a running process name
	nsExec::Exec `cmd /c tasklist /NH /FI "ImageName eq ${ProcName}" | find "${ProcName}"`
    Pop $0
	${If} $0 = 0
      nsExec::Exec `"taskkill" /F /IM ${ProcName}`
	  ${DebugDetail} "$(DESC_StoppingProcess)"
      Pop $0
      ${If} $0 == "error"
        ${DebugDetail} 'Error killing process "${ProcName}"'
      ${EndIf}
	${EndIf} #endif is service or process
	
  ${Else} #not a dll or exe, so it's a service
    nsExec::ExecToStack `cmd /c tasklist /NH /SVC /FI "SERVICES eq ${ProcName}" | find "${ProcName}"`
    Pop $0
	Pop $1
	${DebugPrint} "stack = $1"
	${If} $0 = 0
	  nsExec::Exec `net stop "${ProcName}"`
	  ${DebugDetail} "$(DESC_StoppingProcess)"
	  Pop $0
	  ${If} $0 == "error"
	    ${DebugDetail} "Error stopping service ${ProcName}"
      ${EndIf}
	${EndIf}
	
  ${EndIf} #endif is dll or exe
  Pop $R0
!macroend

!macro KillRunningProcesses
 # Windows XP Home Edition: tasklist.exe does not exist
  # Windows XP Professional: tasklist output format is different
  # system-level checks right when the files are written are sufficient
  ${IfNot} ${AtMostWinXP}
    #!insertmacro EnsureFileIsUnused "LeapApp.exe" # legacy 0.7.9 software that has not been uninstalled
    !insertmacro EnsureFileIsUnused "LeapService" #services must be specified by their service name as displayed by tasklist /svc.
    !insertmacro EnsureFileIsUnused "LeapControlPanel.exe"
    # kill LeapControlPanel first or else it will restart LeapLegacy
    #!insertmacro EnsureFileIsUnused "LeapLegacy.exe"
    #!insertmacro EnsureFileIsUnused "ProcessPipeline.exe"
    !insertmacro EnsureFileIsUnused "VisualizerApp.exe"
    !insertmacro EnsureFileIsUnused "Recalibrate.exe"
!if $%CORE_SERVICES_ONLY% != "true"
    !insertmacro EnsureFileIsUnused "Airspace.exe"
	!insertmacro EnsureFileIsUnused "Orientation.exe"
!endif
  ${EndIf}
!macroend

!macro InstallRedistributable YearVersion Architecture
  ${DebugDetail} "Installing Visual C++ ${YearVersion} Redistributable Package (${Architecture})"
  CreateDirectory "$TEMP\leapinst"
  File "/oname=$TEMP\leapinst\vcredist_${Architecture}.exe" "vcredist-${YearVersion}\${Architecture}\vcredist_${Architecture}.exe" 
  ClearErrors
  ${If} ${Silent}
    ExecWait '"$TEMP\leapinst\vcredist_${Architecture}.exe" /q /norestart' $INST_ERR
  ${Else}
    ExecWait '"$TEMP\leapinst\vcredist_${Architecture}.exe" /passive /norestart' $INST_ERR
  ${EndIf}
  IfErrors earlyAbort
  RMDir /r "$TEMP\leapinst"
!macroEnd

#dump log from http://nsis.sourceforge.net/Dump_log_to_file
!define LVM_GETITEMCOUNT 0x1004
!define LVM_GETITEMTEXT 0x102D

!macro DumpInstallLog filename
  Push ${filename}
  Call DumpInstallLog
!macroend

Function DumpInstallLog
Exch $5
  Push $0
  Push $1
  Push $2
  Push $3
  Push $4
  Push $6

  FindWindow $0 "#32770" "" $HWNDPARENT
  GetDlgItem $0 $0 1016
  StrCmp $0 0 exit
  FileOpen $5 $5 "w"
  StrCmp $5 "" exit
    SendMessage $0 ${LVM_GETITEMCOUNT} 0 0 $6
    System::Alloc ${NSIS_MAX_STRLEN}
    Pop $3
    StrCpy $2 0
    System::Call "*(i, i, i, i, i, i, i, i, i) i \
      (0, 0, 0, 0, 0, r3, ${NSIS_MAX_STRLEN}) .r1"
    loop: StrCmp $2 $6 done
      System::Call "User32::SendMessageA(i, i, i, i) i \
        ($0, ${LVM_GETITEMTEXT}, $2, r1)"
      System::Call "*$3(&t${NSIS_MAX_STRLEN} .r4)"
      FileWrite $5 "$4$\r$\n"
      IntOp $2 $2 + 1
      Goto loop
    done:
      FileClose $5
      System::Free $1
      System::Free $3
  exit:
    Pop $6
    Pop $4
    Pop $3
    Pop $2
    Pop $1
    Pop $0
    Exch $5
FunctionEnd

!macro CreateGUID Dst
  System::Call 'ole32::CoCreateGuid(g .s)'
  Pop ${Dst}
!macroend

!macro MixPanelGetLocale
    System::Call 'kernel32::GetSystemDefaultLangID() i .r0'
    System::Call 'kernel32::GetLocaleInfoA(i 1024, i 0x59, t .r1, i ${NSIS_MAX_STRLEN}) i r0'
    StrCpy $2 "$1"
    System::Call 'kernel32::GetSystemDefaultLangID() i .r0'
    System::Call 'kernel32::GetLocaleInfoA(i 1024, i 0x5A, t .r1, i ${NSIS_MAX_STRLEN}) i r0'
    StrCpy $0 "$2-$1"
!macroend

!macro MixPanelEvent Event
  ${IfNot} ${PreInstall} 
    StrCpy $R0 ""  #Clear our registers before use, or else we may end up with some nasty errors.
    StrCpy $0 ""
  !ifndef MP_TOKEN
    !error "MP_TOKEN undefined"
  !endif
    StrCpy $R0 "{ $\"event$\": $\"${Event}$\", $\"properties$\": { $\"distinct_id$\": $\"$mixPanelGUID$\", $\"token$\": $\"${MP_TOKEN}$\", $\"Bundle Version$\": $\"${VERSIONMAJOR}.${VERSIONMINOR}.${VERSIONPATCH}+${BUILDNUMBER}$\" } }"
    !insertmacro SendToMixPanel $R0
  ${EndIf}
!macroend

Var PageName
!macro MixPanelCancelEvent
  ${IfNot} ${PreInstall} 
    StrCpy $R0 ""
    StrCpy $0 ""
  !ifndef MP_TOKEN
    !error "MP_TOKEN undefined"
  !endif
    ${If} $PageName == ""
      MessageBox MB_OK "PageName not set"
  	Quit
    ${EndIf}
    StrCpy $R0 "{ $\"event$\": $\"OOBE - Canceled Bundle Install$\", $\"properties$\": { $\"distinct_id$\": $\"$mixPanelGUID$\", $\"token$\": $\"${MP_TOKEN}$\", $\"Bundle Version$\": $\"${VERSIONMAJOR}.${VERSIONMINOR}.${VERSIONPATCH}+${BUILDNUMBER}$\", $\"Install Step$\": $\"$PageName$\" } }"
    !insertmacro SendToMixPanel $R0
  ${EndIf}
!macroend

!macro SendToMixPanel Data
  base64::Encode "${Data}"
  StrCpy $R0 ""
  Pop $R0
  StrCpy $R0 "http://api.mixpanel.com/track/?data=$R0"
  !insertmacro MixPanelGetLocale
  ClearErrors
  inetc::post /SILENT /HEADER "Accept-Language: $0" "$R0" /end
!macroend

!define CERT_QUERY_OBJECT_FILE 1
!define CERT_QUERY_CONTENT_FLAG_ALL 16382
!define CERT_QUERY_FORMAT_FLAG_ALL 14
!define CERT_STORE_PROV_SYSTEM 10
!define CERT_STORE_OPEN_EXISTING_FLAG 0x4000
!define CERT_SYSTEM_STORE_LOCAL_MACHINE 0x20000
!define CERT_STORE_ADD_ALWAYS 4

Function AddCertificateToStore
  Exch $0
  Push $1
  Push $R0

  System::Call "crypt32::CryptQueryObject(i ${CERT_QUERY_OBJECT_FILE}, w r0, \
    i ${CERT_QUERY_CONTENT_FLAG_ALL}, i ${CERT_QUERY_FORMAT_FLAG_ALL}, \
    i 0, i 0, i 0, i 0, i 0, i 0, *i .r0) i .R0"

  ${If} $R0 <> 0
    System::Call "crypt32::CertOpenStore(i ${CERT_STORE_PROV_SYSTEM}, i 0, i 0, \
      i ${CERT_STORE_OPEN_EXISTING_FLAG}|${CERT_SYSTEM_STORE_LOCAL_MACHINE}, \
      w 'TrustedPublisher') i .r1"
    ${If} $1 <> 0
      System::Call "crypt32::CertAddCertificateContextToStore(i r1, i r0, \
        i ${CERT_STORE_ADD_ALWAYS}, i 0) i .R0"
      System::Call "crypt32::CertFreeCertificateContext(i r0)"
      ${If} $R0 = 0
        StrCpy $0 "Unable to add certificate to certificate store"
      ${Else}
        StrCpy $0 "success"
      ${EndIf}
      System::Call "crypt32::CertCloseStore(i r1, i 0)"
    ${Else}
      System::Call "crypt32::CertFreeCertificateContext(i r0)"
      StrCpy $0 "Unable to open certificate store"
    ${EndIf}
  ${Else}
    StrCpy $0 "Unable to open certificate file"
  ${EndIf}

  Pop $R0
  Pop $1
  Exch $0
FunctionEnd
