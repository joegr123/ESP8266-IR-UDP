/************************************************************************************************************************************************
 *                                                             MAIN LOOP                                                                        *
 ************************************************************************************************************************************************                                                               
 */

/*
   Main Loop
   Wait for received to be true, meaning a sync stream plus
   all of the data bit edges have been found.
   Convert all of the pulse timings to bits and calculate
   the results.
*/
void loop() {

  ArduinoOTA.handle();                                                                                // for over the air updates
  
  server.handleClient();                                                                              // webserver

  DisplayIP(false);                                                                                   // in case of wifi trouble

  if (irrecv.decode(&results)) {                                                                      // did we receive anything?

    if (results.decode_type > 0) {SendReceived(results.decode_type, results.value, results.repeat);}  // Report what we received if good
    LogIR(results.decode_type, results.value, results.repeat, RX);                                    // Log of last LogSize IR received or transmitted, for debug
    irrecv.resume();                                                                                  // Ready for next one.
    yield();
    
    IRlast = millis();
    IRtodisplay = true;
    
  }

#ifndef Display
  if (IRtodisplay) {
    digitalWrite(LED_Blue, LOW);                        // blue LED on if valid IR received  
  } else {
    digitalWrite(LED_Blue, HIGH);                       // blue LED off if not or after 1 second                             
  }
#endif



  if (IRtodisplay && ((millis() - IRlast) > 1000)) {                          // update display if something new to show and IR is idle now

#ifdef Display
    DisplayReceived(Log[0].Type, Log[0].Value, Log[0].Repeat);
#endif

    IRtodisplay = false; 
    StoreLastIR();                                                            // for detailed debug
    yield();
  }

  
  if (GetUDP()) {                                                             // Get UDP packet, if there is one

#ifdef Display
    display.setCursor(0,12);
    display.print(LastPacket);
    display.print(F("                     "));
    display.setCursor(0,20);
    display.print(F("                     "));
    display.display();
#endif
#ifndef Display
    digitalWrite(LED_Blue, LOW);                                        // blue LED on
#endif

    if (LastPacket == "All Off") {                                       // command to turn it all off?
      SendIR(TVType, TVOff);
      SendIR(DishType, DishOff);
      SendIR(JVCType, JVCOff);
      SendIR(LampType, LampOff);

    } else if (LastPacket == "All On") {                                // command to turn all on, set to Dish
      SendIR(JVCType, JVCOn);
      SendIR(TVType, TVOn);
      SendIR(DishType, DishOn);
      SendIR(JVCType, JVCtoDish);
      SendIR(TVType, TVtoDish);
      
                               
    } else if (LastPacket == "Red On") {                                // command to turn the red lamps on
      SendIR(LampType, LampOn);
      SendIR(LampType, LampRed);
      SendIR(LampType, LampRed);
      SendIR(LampType, LampRed);
                               
    } else if (LastPacket == "Blue On") {                               // command to turn the blue lamps on
      SendIR(LampType, LampOn);
      SendIR(LampType, LampBlue);

    } else if (LastPacket == "White On") {                              // command to turn the white lamps on
      SendIR(LampType, LampOn);
      SendIR(LampType, LampWhite);

    } else if (LastPacket == "Lamp Off") {                              // command to turn the red lamps off
      SendIR(LampType, LampOff);
                               
    } else if (GetType() > 0)  {                                        // is this a discrete IR command?
      SendIR(GetType(),GetValue());                                     // transmit it

    } else {
      SendData("Invalid Command");
    }
    digitalWrite(LED_Blue, HIGH);                                       // blue LED off
  }
}

