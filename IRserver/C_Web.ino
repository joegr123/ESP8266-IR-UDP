/************************************************************************************************************************************************
 ************************************************************************************************************************************************
 **                                                         Webpages                                                                           **
 ************************************************************************************************************************************************
 ************************************************************************************************************************************************
 */


/************************************************************************************************************************************************
 *                                                          Root                                                                                *
 * Just identify ourselves                                                                                                                      *                                                         
 ************************************************************************************************************************************************
 */

void handleRoot() {

  server.send(200, "text/html",
              "<html>" \
                "<head><title>MB RM IR Server</title></head>" \
                "<body>" \
                  "<h1>MB RM IR Server</h1>" \
                  "<p><a href=\"debug\">Debug Page</a></p>" \
                "</body>" \
              "</html>");
}


/************************************************************************************************************************************************
 *                                                          debug                                                                               *
 * Provide debug data                                                                                                                           *                                                         
 ************************************************************************************************************************************************
 */

void handleDebug() {

  String debugstr = "<html><head><title>MB RM IR Server Debug Page</title>";
  debugstr += "<style> p.normal {border: 2px solid black;} table, th, td {border: 1px solid black;border-collapse: collapse;} th, td {padding: 5px;}</style></head><body>";

  debugstr += "<h1>MB RM IR Server Debug Page</h1><h2>System</h2><p class=\"normal\">";

  debugstr += "Version: ";              // system
  debugstr += Version;
#ifdef TestBuild
  debugstr += " - test build<br>";                                        // test build 
#else
  debugstr += " - live build<br>";                                        // actual build 
#endif

  debugstr += "WiFi signal level = " + String(WiFi.RSSI()) + " dBm<br>";  // WiFi
  debugstr += timeSince(1);                                               // up time
  debugstr += " uptime<br>";                  
  debugstr += "Last Reset: ";
  debugstr += ESP.getResetReason();    

  debugstr += "</p><h2>Last Packet received</h2><p class=\"normal\">";    // last packet received from HomeSeer, if any
  if (packetlast == 0) {
    debugstr += "No packets received since last reset.";
  } else {
    debugstr += "Packet size = " + String(packetSize) + "<br>";
    debugstr += "Sender IP Address: ";
    for (int i = 0; i < 4; i++) {
      debugstr += String(PacketIP[i]);
      if (i < 3) {
        debugstr += ".";
      }
    }
    debugstr += " : port " + String(PacketPort) + "<br>";
    debugstr += "Packet = \"" + LastPacket + "\"<br>Received ";
    debugstr += timeSince(packetlast);
    debugstr += " ago"; 
  }
  
  debugstr += "</p><h2>Last IR received</h2><p class=\"normal\">";        // last IR received, if any
  if (IRlast == 0) {
    debugstr += "No IR received/decoded since last reset.";
  } else {
    debugstr += LastIR;
    debugstr += timeSince(IRlast);
    debugstr += " ago"; 
  }

  debugstr += "</p><h2>IR RX/TX Log</h2><table style=\"width:75%\">";
  debugstr += "<tr><th>Dir</th><th>Type</th><th>Value</th><th>Description</th><th>Repeat</th><th>Count</th><th>Time</th></tr>";

  for ( int i = 0; i < LogSize - 1; i++ ) {                                   // display table of last LogSize IR commands received, last first

    if (Log[i].Time != 0) {

      if (Log[i].Direction) {                                                  // received or transmitted ?
        debugstr += "<tr><td><span style=\"color:blue\">RX</span></td><td>";
      } else {
        debugstr += "<tr><td><span style=\"color:red\">TX</span></td><td>"; 
      }
      
      debugstr += ShowType(Log[i].Type) + "</td><td>";       
      debugstr += u64toStr (Log[i].Value) + "</td><td>";
      debugstr += ShowCommand(Log[i].Value) + "</td><td>";
      if (Log[i].Repeat) {
        debugstr += "R</td><td>";
      } else {
        debugstr += " </td><td>";
      }
      debugstr += String(Log[i].Count) + "</td><td>";
      debugstr += timeSince(Log[i].Time) + " ago</td></tr>";
    }
  }

  debugstr += "</table></body></html>";

  server.send ( 200, "text/html", debugstr );

}





/************************************************************************************************************************************************
 *                                                    Requested page doesn't exist                                                              *
  ************************************************************************************************************************************************
 */

void handleNotFound() {

  server.send(404, "text/html",
              "<html>" \
                "<head><title>MB RM IR Server 404</title></head>" \
                "<body>" \
                  "<h1><center><span style=\"color:red\">ERROR 404</span></center></h1>" \
                  "<p><center>No such page!</center></p>" \
                "</body>" \
              "</html>");

}


/************************************************************************************************************************************************
 *                                                          SendData                                                                            *
 ************************************************************************************************************************************************
 */

void SendData(String data) {

  char Dbuffer[data.length()+1];
  data.toCharArray(Dbuffer,data.length()+1);

#ifndef TestBuild
  Udp.beginPacket(HomeSeerIP, 8891);
  Udp.write(Dbuffer);
  Udp.endPacket();  
#endif
  
}




