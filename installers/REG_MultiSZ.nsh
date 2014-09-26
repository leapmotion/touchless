
#--------------------------------------------------------------------------------------------------
!ifndef LOGICLIB
  !include "LogicLib.nsh"
!endif
#--------------------------------------------------------------------------------------------------
#
#       D E F I N E S
#

!define HKEY_CLASSES_ROOT        0x80000000  ; HKCR
!define HKEY_CURRENT_USER        0x80000001  ; HKCU
!define HKEY_LOCAL_MACHINE       0x80000002  ; HKLM
!define HKEY_USERS               0x80000003  ; HKU
!define HKEY_PERFORMANCE_DATA    0x80000004  ; HKPD
!define HKEY_CURRENT_CONFIG      0x80000005  ; HKCC
!define HKEY_DYN_DATA            0x80000006  ; HKDD

!define KEY_QUERY_VALUE          0x0001
!define KEY_SET_VALUE            0x0002
!define KEY_CREATE_SUB_KEY       0x0004
!define KEY_ENUMERATE_SUB_KEYS   0x0008
!define KEY_NOTIFY               0x0010
!define KEY_CREATE_LINK          0x0020

!define REG_NONE                 0
!define REG_SZ                   1
!define REG_EXPAND_SZ            2
!define REG_BINARY               3
!define REG_DWORD                4
!define REG_DWORD_LITTLE_ENDIAN  4
!define REG_DWORD_BIG_ENDIAN     5
!define REG_LINK                 6
!define REG_MULTI_SZ             7

!define RegOpenKeyEx             "Advapi32::RegOpenKeyExA(i, t, i, i, *i) i"
!define RegQueryValueEx          "Advapi32::RegQueryValueExA(i, t, i, *i, i, *i) i"
!define RegCreateKey             "Advapi32::RegCreateKeyA(i, t, *i) i"
!define RegSetValueEx            "Advapi32::RegSetValueExA(i, t, i, i, i, i) i"
!define RegCloseKey              "Advapi32::RegCloseKeyA(i) i"

!define MultiStringDelim         "|"

Var Reg_MULTI_SZ_ErrMsg
#--------------------------------------------------------------------------------------------------

Function SplitFirstStrPart
  Exch $R0                ; save $R0 and get string into $R0
  Exch                    ; move divider char to top of stack
  Exch $R1                ; save $R1 and get divider char into $R1
  Push $R2                ; save $R2 and $R3
  Push $R3
  StrCpy $R3 $R1          ; copy divider char into R3
  StrLen $R1 $R0          ; get length of string into R1
  IntOp $R1 $R1 + 1       ;

  loop:
  IntOp $R1 $R1 - 1
  StrCpy $R2 $R0 1 -$R1
  StrCmp $R1 0 exit0
  StrCmp $R2 $R3 exit1 loop
  exit0:
  StrCpy $R1 ""
  Goto exit2
  exit1:
    IntOp $R1 $R1 - 1
    StrCmp $R1 0 0 +3
     StrCpy $R2 ""
     Goto +2
    StrCpy $R2 $R0 "" -$R1
    IntOp $R1 $R1 + 1
    StrCpy $R0 $R0 -$R1
    StrCpy $R1 $R2
  exit2:
  Pop $R3
  Pop $R2
  Exch $R1 ;rest
  Exch
  Exch $R0 ;first
FunctionEnd

Function GetRegRootValue
  Exch $R1
  ${Switch} $R1
      ${Case} HKCR
         IntOp $R1 0 + ${HKEY_CLASSES_ROOT}
         ${Break}
      ${Case} HKCU
         IntOp $R1 0 + ${HKEY_CURRENT_USER}
         ${Break}
      ${Case} HKLM
         IntOp $R1 0 + ${HKEY_LOCAL_MACHINE}
         ${Break}
      ${Case} HKU
         IntOp $R1 0 + ${HKEY_USERS}
         ${Break}
      ${Case} HKPD
         IntOp $R1 0 + ${HKEY_PERFORMANCE_DATA}
         ${Break}
      ${Case} HKCC
         IntOp $R1 0 + ${HKEY_CURRENT_CONFIG}
         ${Break}
      ${Case} HKDD
         IntOp $R1 0 + ${HKEY_DYN_DATA}
         ${Break}
      ${Default}
         SetErrors
         StrCpy $Reg_MULTI_SZ_ErrMsg "Illegal value $R1 for Registry RootKey"
         ${Break}
  ${EndSwitch}
  Exch $R1
FunctionEnd

#------------------------------------------------------------------------------

Function Reg_MULTI_SZ_Delete

  Pop $R3    ; SUB_KEY
  Pop $R2    ; KEY
  Pop $R1    ; ROOT (HLKM|HKCU....)

  StrCpy $Reg_MULTI_SZ_ErrMsg ""
  ClearErrors

  ${Switch} $R1
      ${Case} HKCR
         DeleteRegValue HKCR $R2 $R3
         ${Break}
      ${Case} HKCU
         DeleteRegValue HKCU $R2 $R3
         ${Break}
      ${Case} HKLM
         DeleteRegValue HKLM $R2 $R3
         ${Break}
      ${Case} HKU
         DeleteRegValue HKU $R2 $R3
         ${Break}
      ${Case} HKPD
         DeleteRegValue HKPD $R2 $R3
         ${Break}
      ${Case} HKCC
         DeleteRegValue HKCC $R2 $R3
         ${Break}
      ${Case} HKDD
         DeleteRegValue HKDD $R2 $R3
         ${Break}
      ${Default}
         SetErrors
         StrCpy $Reg_MULTI_SZ_ErrMsg "Illegal value $R1 for Registry RootKey"
         ${Break}
  ${EndSwitch}

  ${If} ${Errors}
      ${If} $Reg_MULTI_SZ_ErrMsg == ""
            StrCpy $Reg_MULTI_SZ_ErrMsg "Can't delete registry key!$\r$\n($R1\$R2\$R3)"
      ${EndIf}
  ${EndIf}
FunctionEnd

#------------------------------------------------------------------------------

Function Reg_MULTI_SZ_Write
  Pop $R4    ; VALUE string(s)
  Pop $R3    ; SUB_KEY
  Pop $R2    ; KEY
  Pop $R1    ; ROOT (HLKM|HKCU....)

  Push $R0
  Push $0
  Push $1
  Push $2
  Push $3
  Push $4
  Push $5

  StrCpy $0 ""
  StrCpy $1 ""   ; Buffer
  StrCpy $2 ""
  StrCpy $3 ""   ; AdvApi result value
  StrCpy $4 ""   ; String part to write
  StrCpy $5 "0"  ; counter

  SetPluginUnload alwaysoff
  ClearErrors
  StrCpy $Reg_MULTI_SZ_ErrMsg ""

  Push $R1
  Call GetRegRootValue
  Pop $R0

  ${Unless} ${Errors}
          ; Create a buffer for the multi_sz value
          System::Call "*(&t${NSIS_MAX_STRLEN}) i.r1"
          ; Open/create the registry key
          System::Call "${RegCreateKey}($R0, '$R2', .r0) .r3"
          ; Failed?

          ${Unless} $3 = 0
              SetErrors
              StrCpy $Reg_MULTI_SZ_ErrMsg "Can't create registry key!$\r$\n($R1\$R2\$R3)"
          ${Else}
              ; Fill in the buffer with our strings
              StrCpy $2 $1                            ; Initial position
              StrCpy $4 $R4                           ; copy VALUE into $4 for splitting by | char

              ${Do}
                      Push ${MultiStringDelim}
                      Push $R4
                      Call SplitFirstStrPart
                      Pop $4
                      Pop $R4
                      IntOp $5 $5 + 1
                      DetailPrint  "Value part $5: $4"

                      StrLen $3 '$4'                           ; Length of first string
                      IntOp $3 $3 + 1                         ; Plus null
                      System::Call "*$2(&t$3 '$4')"           ; Place the string
                      IntOp $2 $2 + $3                        ; Advance to the next position
               ${LoopUntil} $R4 == ""

               System::Call "*$2(&t1 '')"              ; Place the terminating null
               IntOp $2 $2 + 1                         ; Advance to the next position

              ; Create/write the value
               IntOp $2 $2 - $1                        ; Total length
               System::Call "${RegSetValueEx}(r0, '$R3', 0, ${REG_MULTI_SZ}, r1, r2) .r3"
               ; Failed?
               ${Unless} $3 = 0
                    SetErrors
                    StrCpy $Reg_MULTI_SZ_ErrMsg "Can't set key value! $\r$\n($R1\$R2\$R3 => $R4)"
               ${EndUnless}

              ; Close the registry key
              System::Call "${RegCloseKey}(r0)"
          ${EndUnless}

          ; Clear the buffer
          SetPluginUnload manual
          System::Free $1
  ${EndUnless}

  Pop $5
  Pop $4
  Pop $3
  Pop $2
  Pop $1
  Pop $0
  Pop $R0
  
FunctionEnd
#------------------------------------------------------------------------------

Function Reg_MULTI_SZ_Read

  Pop $R3    ; SUB_KEY
  Pop $R2    ; KEY
  Pop $R1    ; ROOT (HLKM|HKCU....)

  Push $R0
  Push $0
  Push $1
  Push $2
  Push $3
  Push $4
  Push $5
  Push $6
  Push $7
  Push $8

  StrCpy $0 ""
  StrCpy $1 ""   ; Buffer
  StrCpy $2 ""
  StrCpy $3 ""   ; AdvApi result value
  IntOp  $4 $4 & 0
  IntOp  $5 $5 & 0
  IntOp  $6 $6 & 0


  StrCpy $Reg_MULTI_SZ_ErrMsg ""
  ClearErrors
  
  Push $R1
  Call GetRegRootValue
  Pop $R0

  ${Unless} ${Errors}
          SetPluginUnload alwaysoff
          System::Call "${RegOpenKeyEx}($R0, '$R2', 0, ${KEY_QUERY_VALUE}|${KEY_ENUMERATE_SUB_KEYS}, .r0) .r3"
          ${If} $3 = 0
                 System::Call "${RegQueryValueEx}(r0, '$R3', 0, .r1, 0, .r2) .r3"
                 ${If} $3 = 0
                        ${If} $1 == ${REG_MULTI_SZ}
                                ${If} $2 <> 0
                                        System::Alloc $2
                                        Pop $1          ; buffer
                                        ${If} $1 <> 0
                                              System::Call "${RegQueryValueEx}(r0, '$R3', 0, n, r1, r2) .r3"
                                              ; $1 = buffer, $2 = length of value $3 = result
                                              ${If} $3 = 0
                                                  StrCpy $4 $1           ; copy ptr to buffer
                                                  IntOp $6 $4 + $2       ; calc offset by adding length of copied data
                                                  IntOp $6 $6 - 1        ; subtract the 0 at the end

                                                  ${While} $4 < $6
                                                       ; copy first zero terminated string starting at $4 into $3
                                                       System::Call "*$4(&t${NSIS_MAX_STRLEN} .r3)"
                                                       ; print result  (SHOULD BE REMOVED FOR PRODUCTION CODE)
                                                       DetailPrint $3
                                                       ; append MultiStringDelim if not first one
                                                       ${If} $R4 != ""
                                                             StrCpy $R4 "$R4${MultiStringDelim}"
                                                       ${EndIf}
                                                       ; check if read value will fit into (max length ${NSIS_MAX_STRLEN} )
                                                       StrLen $7 $R4
                                                       StrLen $8 $3
                                                       IntOp $8 $8 + $7
                                                       ${If} $8 > ${NSIS_MAX_STRLEN}
                                                             StrCpy $Reg_MULTI_SZ_ErrMsg "MULTI_SZ data too big (>${NSIS_MAX_STRLEN} bytes)"
                                                             ${ExitWhile}
                                                       ${EndIf}
                                                       ; if yes : append value
                                                       StrCpy $R4 "$R4$3"
                                                       ; increment pointer in the buffer by length of $3 + 1 (for the zero)
                                                       StrLen $5 $3
                                                       IntOp $4 $4 + $5
                                                       IntOp $4 $4 + 1
                                                       ; check if end of buffer reached
                                                       ; if not loop
                                                  ${EndWhile}
                                              ${Else}
                                                     StrCpy $Reg_MULTI_SZ_ErrMsg "Can't query registry value data!"
                                              ${EndIf}
                                        ${Else}
                                               StrCpy $Reg_MULTI_SZ_ErrMsg "Can't allocate enough memory!"
                                        ${EndIf}
                                ${Else}
                                       StrCpy $Reg_MULTI_SZ_ErrMsg "Registry value empty!"
                                ${EndIf}
                        ${Else}
                               StrCpy $Reg_MULTI_SZ_ErrMsg "Registry value no REG_MULTI_SZ!"
                        ${EndIf}
                 ${Else}
                        StrCpy $Reg_MULTI_SZ_ErrMsg "Can't query registry value size!"
                 ${EndIf}
          ${Else}
                StrCpy $Reg_MULTI_SZ_ErrMsg "Can't open registry key!"
          ${EndIf}

          System::Free $1

          ${If} $0 <> 0
              System::Call "${RegCloseKey}(r0)"
          ${EndIf}

          SetPluginUnload manual
  ${EndUnless}

  Pop $8
  Pop $7
  Pop $6
  Pop $5
  Pop $4
  Pop $3
  Pop $2
  Pop $1
  Pop $0
  Pop $R0

  ${If} $Reg_MULTI_SZ_ErrMsg != ""
      SetErrors
      StrCpy $Reg_MULTI_SZ_ErrMsg "$\r$\n($R1\$R2\$R3)"
  ${EndIf}
  ${If} ${Errors}
      StrCpy $R4 ""
  ${Endif}
  Push $R4
  
FunctionEnd
#------------------------------------------------------------------------------
