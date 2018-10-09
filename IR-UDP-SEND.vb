Imports System.Net.Sockets
Imports System.Text

Const Fname As String  = "IR-UDP-SEND.vb"               ' this file name
Const Lcolor As String = "#7f007f"                     ' log color for normal

Sub Main(ByVal Parm As Object)

    Dim udpClient As New UdpClient("192.168.1.23", 8891)
    Dim sendBytes As [Byte]() = Encoding.ASCII.GetBytes(Parm)
    hs.WriteLogDetail(Fname ,parm + " Sent",Lcolor,9,Fname & ":Main",0)

    Try
        udpClient.Send(sendBytes, sendBytes.Length)
    Catch Ex As Exception : hs.WriteLogDetail(Fname & " Error",Ex.Message, "#FF0000", 1,Fname & ":Main",9)
    End Try

End Sub