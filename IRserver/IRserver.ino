/**********************************************************************


*/


//#define TestBuild                                     // uncomment for the test build instead of actual unit, also turns on debug output
//#define DebugOut                                      // uncomment to stream data out serial port
//#define Display                                       // uncomment for oled display support

/************************************************************************************************************************************************
                                                              INCLUDES
 ************************************************************************************************************************************************
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>


#ifdef Display
  #include <Wire.h>                                       // I2C
  #include <Adafruit_GFX.h>                             // Display
  #include <Adafruit_SSD1306.h>                         // Specific OLED display
#endif  

#include <WiFiUdp.h>                                    // for OTA updates
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>

#include <IRremoteESP8266.h>                            // for IR
#include <IRrecv.h>                                     // Go into the library and set all the IR protocols that you don't need
#include <IRsend.h>                                     // to disable to save memory and reduce crashes
#include <IRutils.h>

#include <PROGMEM_readAnything.h>                       // to read constants out of FLASH

/************************************************************************************************************************************************
                                                              DEFINES
 ************************************************************************************************************************************************
*/

#define OLED_RESET 4                                  // For I2C OLED display

#define BaudRate  115200                              // baud rate for serial debug data  --- should set as high as possible instead of 115200


#define RECV_PIN2 D5                                  // D5, IR receiver #2 pin
#define RECV_PIN D6                                   // D6, IR receiver pin
#define IR_LED D2                                     // D2, IR emitter pin

#define CAPTURE_BUFFER_SIZE 1024
#define TIMEOUT 10U
#define MIN_UNKNOWN_SIZE 12

#define LogSize 50                                    // Size of IR receive log
#define LogSeconds 5                                  // Number of seconds between repeats to wait to log code again

#define LED_Blue 2                                    // blue LED on GPIO2 (D4)

#define RX true
#define TX false



/************************************************************************************************************************************************
                                                         Global Constants
 ************************************************************************************************************************************************
*/

const char Version[] = "Build 5/31/2018 A";

const char ssid[] = "your ssi";                            // local WiFI
const char password[] = "your password";

#ifdef TestBuild
const char HostName[] = "IRserver-test";                  // test build unit name
#else
const char HostName[] = "IRserver";                       // actual build name
#endif

const char HomeSeerIP[] = "xx.xx.xx.xx";                  // Your HomeSeer PC IP
const unsigned int localPort = 8891;                      // local port to listen on (UDP)

const int      DishType     = 13;
const int      TVType       = 7;
const int      JVCType      = 6;
const int      LampType     = 3;
const int      SonyType     = 4;
const int      LGType       = 10;

const uint64_t JVCOff       = 0xC558ull;
const uint64_t JVCOn        = 0xC5B8ull;
const uint64_t JVCtoDish    = 0xc508ull;
const uint64_t DishOff      = 0x9C00ull;
const uint64_t DishOn       = 0x400ull;
const uint64_t TVOn         = 0xE0E09966ull;
const uint64_t TVOff        = 0xE0E019E6ull;
const uint64_t TVtoDish     = 0xE0E07d82ull;
const uint64_t LampOff      = 0xFF609Full;
const uint64_t LampOn       = 0xFFE01Full;
const uint64_t LampRed      = 0xFF906Full;
const uint64_t LampBlue     = 0xFF50AFull;
const uint64_t LampWhite    = 0xFFD02Full;


struct IRType {
  char* Name;
  byte Type;
};

struct IRCommand {
  char*     Name;
  uint64_t  Value;
};

const IRCommand IRCommands[] PROGMEM =  { {"Dish Exit",0x4800},{"Dish Guide",0x5000},{"Dish down",0x7800},{"Dish Up",0x6800},
                                        {"Dish Right",0x6000},{"Dish Left",0x7000},{"Dish Select", 0x4000},{"Dish Info",0x0},
                                        {"Dish Recall",0x6c00},{"Dish Pause",0x8000},{"Dish Play",0xc10},{"Dish Stop",0x8400},
                                        {"Dish Skip Forward",0xdc10},{"Dish Fast Forward",0xc810},{"Dish Rewind",0xc410},{"Dish Skip Back",0xd810},
                                        {"Dish Record",0x7c00},{"Dish 0",0x4400},{"Dish 1",0x1000},{"Dish 2",0x1400},
                                        {"Dish 3",0x1800},{"Dish 4",0x2000},{"Dish 5",0x2400},{"Dish 6",0x2800},
                                        {"Dish 7",0x3000},{"Dish 8",0x3400},{"Dish 9",0x3800},{"Dish DVR",0xe410},
                                        {"Dish Search",0xb400},{"Dish Red",0x4c00},{"Dish Green",0xd400},{"Dish Yellow",0x8800},
                                        {"Dish Blue",0x8c00},{"Dish On",0x400},{"Dish Off",0x9c00},{"TV Smart Hub",0xe0e09e61},
                                        {"TV Up",0xe0e006f9},{"TV Down",0xe0e08679},{"TV Right",0xe0e046b9},{"TV Left",0xe0e0a659},
                                        {"TV Select",0xe0e016e9},{"TV Exit",0xe0e0b44b},{"TV Info",0xe0e0f807},{"TV Guide",0xe0e0f20d},
                                        {"TV Play",0xe0e0e21d},{"TV Pause",0xe0e052ad},{"TV On",0xE0E09966},{"TV off",0xE0E019E6},
                                        {"TV HDMI 2 (Dish)",0xE0E07d82},{"TV Menu",0xe0e058a7},{"TV Return",0xe0e01ae5},{"Volume Down",0xc5f8},
                                        {"Volume Up",0xc578},{"Mute",0xc538},{"JVC On",0xc5b8},{"JVC Off",0xc558},
                                        {"JVC to TV",0xc5c2},{"JVC to Dish",0xc508},{"Lamp Off",0xFF609F},{"Lamp On",0xFFE01F},
                                        {"Lamp Red",0xFF906F},{"Lamp Blue",0xFF50AF},{"Lamp White",0xFFD02F},{"Room Light Toggle",0xa8b47} };

const IRType IRTypes[6] PROGMEM = { {"Dish",13},{"Samsung",7},{"JVC",6},{"NEC",3},{"Sony",4},{"LG",10} };
  

/************************************************************************************************************************************************
                                                         Global Variables
 ************************************************************************************************************************************************
*/

unsigned long IRlast = 0;                           // last time IR was received (0 = never)
unsigned long packetlast = 0;                       // last time packet was received from HomeSeer

bool  IRtodisplay = false;                          // any new IR received to display?

char packetBuffer[UDP_TX_PACKET_MAX_SIZE];          // buffer to hold incoming packet,

String LastIR;                                      // store the full data on the last IR received

struct LogStruct {
  byte          Type;                               // IR command type
  unsigned long Time;                               // Receive time
  uint64_t      Value;                              // IR value
  bool          Repeat;                             // IR repeat flag
  bool          Direction;                          // True = received, false = transmitted
  int           Count;                              // Number of times same code received with no more than LogSeconds gap between
};

LogStruct Log[LogSize];

String    LastPacket;                               // last packet received
int       packetSize = 0;
IPAddress PacketIP;
int       PacketPort = 0;


/************************************************************************************************************************************************
                                                      Library Stuff
 ************************************************************************************************************************************************
*/

#ifdef Display
  Adafruit_SSD1306 display(OLED_RESET);                 // OLED display
#endif

ESP8266WebServer server ( 80 );                       // Web stuff

WiFiUDP Udp;                                          // for UDP send to and receive from HomeSeer


// Use turn on the save buffer feature for more complete capture coverage.
IRrecv irrecv(RECV_PIN, CAPTURE_BUFFER_SIZE, TIMEOUT, true);                    // set up receiver

IRsend irsend(IR_LED);                                                          // set up emitter

decode_results results;  // Somewhere to store the results

template< typename T, size_t N > size_t ArraySize (T (&) [N]){ return N; }      // to use constants out of FLASH



#ifdef TestBuild
#define DebugOut
#endif
