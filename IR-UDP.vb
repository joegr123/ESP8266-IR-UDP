'
' Receive IR Status updates from IRServer
'
Const Fname As String  = "IR-UDP.vb"               ' this file name
Const Lcolor As String = "#7f007f"                     ' log color for normal

Sub Main(parm() as object)

Try

  dim csv
  dim lampvalue
  dim TVref     = hs.GetDeviceRef("r83")
  dim AVRref    = hs.GetDeviceRef("r85")
  dim DISHref   = hs.GetDeviceRef("r84")
  dim LAMPref   = hs.GetDeviceRef("s40")

  if (InStr(parm(2).ToString,",") > 0) then                                         ' is it IR code or start up message?

    csv = split(parm(2).ToString, ",")       ' csv(0) = IR protocol type
                                             ' csv(1) = IR code
                                             ' csv(2) = IR repeat flag
                                             ' csv(3) = "END"

    hs.WriteLogDetail(Fname,"Received: " & csv(0) & "," & csv(1) & "," & csv(2), Lcolor, 9,Fname & ":Main",1)

                                                                                            ' turn on (dish mode) if remote already on
    if ((hs.DeviceValue(TVref) = 0) and (csv(1) = "e0e07d82" or csv(1) = "c508")) then      ' r83 is TV
        hs.WaitSecs (1)                                                                     ' to ignore rest of codes
        hs.RunScriptFunc("IR-UDP-SEND.vb","Main","All On",false,true)              ' turn it all on and set to Dish
    end if

                                                                                            ' turn on (smart tv mode) if remote already on
    if ((hs.DeviceValue(TVref) = 0) and (csv(1) = "e0e0b44b" or csv(1) = "c5c2")) then      ' r83 is TV
        hs.WaitSecs (1)                                                                     ' to ignore rest of codes
        hs.RunScriptFunc("IR-UDP-SEND.vb","Main","Smart TV",false,true)              ' turn it all on and set to smart TV
    end if
                                                                                     '   "Sys Off" to turn off AVR, TV, Dish, but not lamps


    lampvalue = hs.DeviceValue(LAMPref)

    if (csv(1) <> "9c00" and csv(1) <> "e0e019e6" and csv(1) <> "c558" and csv(1) <> "ff609f" and csv(0) <> "3") then     ' if there is activity (other than off), indicate it
       UpdateNlog("r86",100,0)
       hs.SetDeviceString(hs.GetDeviceRef("r86"),"Last IR: " & csv(0) & ":" & csv(1) & ":" & csv(2), true)
    end if

    if (csv(0) = "13" and csv(1) <> "9c00") and (hs.DeviceValue(DISHref) = 0) then
       UpdateNlog("r84",100,0)                                                                ' anything but OFF turns the dish on
    else if (csv(0) = "13" and csv(1) = "9c00") and (hs.DeviceValue(DISHref) > 0) then
       UpdateNlog("r84",0,0)                                                                  ' Dish off

    else if (csv(0) = "7" and csv(1) = "e0e09966") and (hs.DeviceValue(TVref) = 0) then
       UpdateNlog("r83",100,1)                                                                ' TV on
    else if (csv(0) = "7" and csv(1) = "e0e019e6") and (hs.DeviceValue(TVref) > 0) then
       UpdateNlog("r83",0,0)                                                                  ' TV off
    else if (csv(0) = "7" and csv(1) <> "e0e019e6") and (hs.DeviceValue(TVref) = 0) then
       UpdateNlog("r83",100,0)                                                                ' TV must be on if someone is sending commands

    else if (csv(0) = "6" and csv(1) = "c5b8") and (hs.DeviceValue(AVRref) = 0) then
       UpdateNlog("r85",100,0)                                                                ' JVC AVR on
    else if (csv(0) = "6" and csv(1) = "c558") and (hs.DeviceValue(AVRref) > 0) then
       UpdateNlog("r85",0,0)                                                                  ' JVC AVR off
    else if (csv(0) = "6" and csv(1) <> "c558") and (hs.DeviceValue(AVRref) = 0) then
       UpdateNlog("r85",100,0)                                                                ' JVC must be on if someone is sending commands

    else if (csv(0) = "4" and csv(1) = "a8b47" ) then
       if (DateDiff("s",hs.DeviceLastchangeRef(hs.GetDeviceRefByName("Master Bedroom Light")),Now()) > 2) then       ' wait for repeat codes to finish
          if (hs.DeviceValue (hs.GetDeviceRefByName("Master Bedroom Light")) = 0 ) then
             hs.CAPIControlHandler(hs.CAPIGetSingleControl(hs.GetDeviceRefByName("Master Bedroom Light"), True, "On", False, False))
             hs.WaitSecs(2)
          else
             hs.CAPIControlHandler(hs.CAPIGetSingleControl(hs.GetDeviceRefByName("Master Bedroom Light"), True, "Off", False, False))
             hs.WaitSecs(2)
          end if
       end if
                                                                             ' lamps set to red
    else if (csv(0) = "3" and csv(1) = "ff906f" ) then
       UpdateNlog("s40",1,0)                                                 ' lamp is red
    else if (csv(0) = "3" and csv(1) = "ffd02f" ) then
       UpdateNlog("s40",4,0)                                                 ' lamp is white
    else if (csv(0) = "3" and csv(1) = "ff50af" ) then
       UpdateNlog("s40",3,0)                                                 ' lamp is blue
    else if (csv(0) = "3" and csv(1) = "ff609f" ) then                       ' lamps off
       UpdateNlog("s40",0,0)
    else if (csv(0) = "3" and csv(1) = "ffe01f" ) then                       ' lamps on
       UpdateNlog("s40",100,0)                                               ' need to fix this for color?

    end if

  else                                                 ' For previous IRserver code (prior to 04/23/2018)
    hs.WriteLogDetail(Fname,"IR-UDP Error: " & parm(2).ToString, Lcolor, 9,Fname & ":Main",1)

  end if

Catch Ex As Exception : hs.WriteLogDetail(Fname & " Error",Ex.Message, "#FF0000", 1,Fname & ":Main",9)
End Try

End Sub

Sub UpdateNlog (HDcode As String, Dvalue As Double, Always as Integer) ' this routine updates a device and logs it if
                                                                       ' it changed and if logging is turned on for the device or if always is set
  Try
    dim Dref    = hs.GetDeviceRef(HDcode)                                   ' get the reference to the device based on the house/device code
    dim dv      = hs.GetDeviceByRef(Dref)                                   ' get device settings
    dim Du      = hs.GetDeviceByRef(Dref)                                   ' get device data
    dim Dstring = hs.DeviceVSP_GetStatus(Dref, Dvalue, 1)                   ' get the string
    dim oldVal  = hs.DeviceValueEx(Dref)                                    ' get the prior value

    if (oldVal <> Dvalue) then                                              ' only set it if it changed
       hs.SetDeviceValueByRef (Dref,Dvalue,TRUE)                               ' update the device
       if dv.MISC_Check(hs, Enums.dvMISC.NO_LOG) = 0 then                      ' should it be logged?
          hs.WriteLogDetail(Fname,Du.Location(hs) & " " & Du.name(hs) & " changed to: " & Dstring, Lcolor, 9,Fname & ":UpdateNlog",1)
       end if
    end if

    if (oldVal = Dvalue) and (Always = 1) then                               ' Need to log it even if it didn't change?
       if dv.MISC_Check(hs, Enums.dvMISC.NO_LOG) = 0 then                      ' should it be logged?
          hs.WriteLogDetail(Fname,Du.Location(hs) & " " & Du.name(hs) & " is still: " & Dstring, Lcolor, 9,Fname & ":UpdateNlog",1)
       end if
    end if

Catch Ex As Exception
      hs.WriteLogDetail(Fname & ":UpdateNlog ERROR",Ex.Message, "#FF0000", 1,Fname & ":UpdateNlog",9)
End Try

end sub