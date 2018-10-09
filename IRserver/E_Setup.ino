/************************************************************************************************************************************************
 *                                                               Setup                                                                          *
 ************************************************************************************************************************************************                                                               
 */

void setup()
{
  Serial.begin(BaudRate);

#ifdef Display
  Wire.begin(D3, D4);                                                     // I2C  (SDA, SCL)
  Wire.setClock(100000);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);                              // initialize OLED with the I2C addr 0x3C (for the 128x32), turn on display charge pump
  display.clearDisplay();                                                 // Display is 128 x 32 dots, 21 characters x 4 lines
  display.setTextSize(2);                                                 // Size 2 is 10 characters by 2 lines
  display.setTextWrap(false);                                             // Don't jump to the next line
  display.setTextColor(WHITE,BLACK);                                      // Over write the background with each character
  display.setCursor(16,8);                                                // Start kind of centered
  display.println(F("STARTING"));
  display.display();
#endif
  WiFi.mode ( WIFI_STA );
  WiFi.begin(ssid, password);                                             // connect to the WiFi

#ifndef Display
  pinMode(LED_BUILTIN, OUTPUT);                                           // use LEDs if no display
  pinMode(LED_Blue, OUTPUT);
#endif
  
  while ((WiFi.status() != WL_CONNECTED)&&(millis()< 30*1000)) {
    delay(500);                                                           // This needed to give control back to the esp to run its own code!  
  }

#ifdef Display
  display.clearDisplay();                                                 // clear display, display IP address or error
  DisplayIP(true);
#endif  

  server.on ( "/", handleRoot );                                          // root webpage, just identify the device

  server.on ( "/debug",handleDebug );                                     // show info for possible debugging

  server.onNotFound ( handleNotFound );

  server.begin();                                                         // web page server

  Udp.begin(localPort);                                                   // UDP

  irrecv.enableIRIn();                                                    // Start the receiver

  irsend.begin();                                                         // Ready the transmitter
  
                                                                          // OTA stuff
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(HostName);

  // No authentication by default
  // ArduinoOTA.setPassword("admin");
  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else                                                      // U_SPIFFS
      type = "filesystem";
                                                              // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
#ifdef Display
    display.clearDisplay();                                                 
    display.setTextSize(2);
    display.setCursor(0,0);
    display.print("OTA");
    display.setTextSize(1);
    display.setCursor(6*2*3+6,0);
    display.print("Update");
    display.setCursor(6*2*3+6,8);
    display.print(type);
    display.display();
    display.setCursor(0,8*2);
    display.setTextSize(1);
    display.print("Progress: ");
#endif
  });
  ArduinoOTA.onEnd([]() {
#ifdef Display
    display.setCursor(0,8*2);
    display.setTextSize(2);
    display.print("END        ");
    display.display();
    delay(1000);
#endif
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
#ifdef Display
    display.setCursor(60,8*2);
    display.setTextSize(2);
    display.print((progress / (total / 100)));
    display.display();
#endif
  });
  ArduinoOTA.onError([](ota_error_t error) {
#ifdef Display
    display.setTextSize(1);
    display.setCursor(0,24);
    if (error == OTA_AUTH_ERROR) display.print(F("Auth Failed"));
    else if (error == OTA_BEGIN_ERROR) display.print(F("Begin Failed"));
    else if (error == OTA_CONNECT_ERROR) display.print(F("Connect Failed"));
    else if (error == OTA_RECEIVE_ERROR) display.print(F("Receive Failed"));
    else if (error == OTA_END_ERROR) display.print(F("End Failed"));
    display.display();
    delay(5000);
#endif
  });
  ArduinoOTA.begin();

  String Restart = "Reset: ";
  Restart += ESP.getResetReason();
  SendData(Restart);                                // power up or crashed message.

}


