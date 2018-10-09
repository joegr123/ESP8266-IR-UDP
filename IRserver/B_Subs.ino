/************************************************************************************************************************************************
 ************************************************************************************************************************************************
 **                                                          Subroutines                                                                       **
 ************************************************************************************************************************************************
 ************************************************************************************************************************************************
 */


/************************************************************************************************************************************************
 *                                                          SendReceived                                                                        *
 ************************************************************************************************************************************************
 */

void SendReceived(int Type, uint64_t Value, bool Repeat) {

  String Rdata = String(Type) + "," + u64toStr(Value);                      // Send the IR protocol type and value

  if (Repeat) {                                                             // add Repeat or Not flag
    Rdata += ",R";
  } else {
    Rdata += ",N";
  }

  Rdata += ",END";

  char Dbuffer[Rdata.length()+1];
  Rdata.toCharArray(Dbuffer,Rdata.length()+1);

#ifndef TestBuild
  Udp.beginPacket(HomeSeerIP, 8891);
  Udp.write(Dbuffer);
  Udp.endPacket();  
#endif
  
}



/************************************************************************************************************************************************
 *                                                          DisplayReceived                                                                     *
 ************************************************************************************************************************************************
 */
#ifdef Display

void DisplayReceived(int Type, uint64_t Value, bool Repeat) {

  display.setCursor(0,12);

  if (Type > 0) {
    display.print(F("Type = "));
    display.print(Type);
    display.print(F("             "));
    display.setCursor(0,20);
    display.print(F("V= "));
    display.print(u64toStr(Value));
    display.print(F("                 "));
    display.setCursor(114,12);
    if (Repeat) { 
      display.print(F("R"));
    } else {
      display.print(F(" "));
    }
  } else {
    display.print(F("      REC ERROR      "));
    display.setCursor(0,20);
    display.print(F("                     ")); 
  }

  display.display();

}
#endif

/************************************************************************************************************************************************
 *                                                          DisplayIP                                                                           *
 ************************************************************************************************************************************************
 */
#ifdef Display

void DisplayIP (bool Always) {

  static int PConnected = 0;
  if (WiFi.status() != PConnected || Always) {
    display.setCursor(0,0);                                                 
    display.setTextSize(1);       
    if (WiFi.status() != WL_CONNECTED) {
      display.print(F("WiFi Failed!   "));                                     // No WiFi Address
    } else {
      display.print(F("               "));
      display.setCursor(0,0);                                                 
      display.print(WiFi.localIP());                                        // display the ESP IP address
      display.setCursor(17*6,0);
      display.print(WiFi.RSSI());                                           // signal strength 
    }
    PConnected = WiFi.status(); 
  }
  display.display();
}
#endif

#ifndef Display

void DisplayIP (bool Always) {

  if (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, LOW);                        // red LED on if connected
  } else {
    digitalWrite(LED_BUILTIN, HIGH);                       // red LED off if not connected                             
  }
}
#endif

/************************************************************************************************************************************************
 *                                                          StoreLastIR                                                                         *
 *  Store the full data for the last IR received (for debug only)                                                                               *                                                        
 ************************************************************************************************************************************************
 */

void StoreLastIR (void) {

  LastIR = "";
  if (results.overflow) { LastIR += "Overflow warning!<br>"; }
  LastIR += resultToHumanReadableBasic(&results);
  LastIR += "<br>";
  LastIR += resultToTimingInfo(&results);
  LastIR += "<br>";
  LastIR += resultToSourceCode(&results);
  LastIR += "<br>Received ";

}


/************************************************************************************************************************************************
 *                                                          LogIR                                                                               *
 *  Store last LogSize IR commands received (for debug only)                                                                                    *                                                        
 ************************************************************************************************************************************************
 */

void LogIR (int Type, uint64_t Value, bool Repeat, bool Direction) {
  
  if ( Log[0].Type == Type && Log[0].Value == Value && Log[0].Repeat == Repeat && 
      Log[0].Direction == Direction && ((millis() - Log[0].Time) < LogSeconds*1000)) {
        
        Log[0].Count ++;                                               // was repeat, count it
        Log[0].Time      = millis();                                   // update time to last received
        
  } else {
                                                                      // only log it if not multiple repeat       
    for ( int i =LogSize-1; i > 0; i-- ) { Log[i] = Log[i-1]; }       // move it all down one row

    Log[0].Type      = Type;                                           // put the newest IR in
    Log[0].Value     = Value;
    Log[0].Repeat    = Repeat;
    Log[0].Time      = millis();
    Log[0].Direction = Direction;
    Log[0].Count     = 1;
  }
  
}


/************************************************************************************************************************************************
 *                                                          GetUDP                                                                              *
 ************************************************************************************************************************************************
 */

bool GetUDP (void) {

  int Size = Udp.parsePacket();
  if (Size) {
    
    packetSize = Size;    
    Udp.read(packetBuffer, packetSize);
    packetBuffer[packetSize] = 0;                               // terminate the string   

    PacketIP = Udp.remoteIP();
    PacketPort = Udp.remotePort();
    
    LastPacket = String(packetBuffer);
    packetlast = millis();

    return(true);
    
  } else {
    return(false);
  }
}


/************************************************************************************************************************************************
 *                                                 SendIR                                                                                       *
 ************************************************************************************************************************************************
 */

void SendIR (int Type, uint64_t Value) {

      switch (Type) {                                               // what type to send?
        case LampType:
          irsend.sendNEC(Value,NEC_BITS,4);                                                                
          break;
        case SonyType: 
          irsend.sendSony(Value,SONY_20_BITS,4);                                                                      
          break;
        case JVCType:
          irsend.sendJVC(Value,JVC_BITS,4);                                                                      
          break;
        case TVType: 
          irsend.sendSAMSUNG(Value,SAMSUNG_BITS, 4);                                                                      
          break;
        case LGType: 
          irsend.sendLG(Value,LG_BITS,4);                                                                    
          break;
        case DishType: 
          irsend.sendDISH(Value,DISH_BITS,5);                                                                     
          break;
        default:                                                    // can't transmit it because type is invalid
          SendData("Invalid or Unsupported IR Type");
          break; 
      } 
      LogIR(Type,Value, false, TX);                                 // log it
      delay(500);
      SendReceived(Type,Value, false);                              // echo it back
      irrecv.decode(&results);                                      // flush out the received IR that we just sent
      irrecv.resume();

}

