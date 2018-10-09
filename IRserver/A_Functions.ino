/************************************************************************************************************************************************
 ************************************************************************************************************************************************
 **                                                           Functions                                                                        **
 ************************************************************************************************************************************************
 ************************************************************************************************************************************************
 */


/************************************************************************************************************************************************
 * timeSince (start)     : Function to return the time since start in days/hours/minutes/seconds                                                *
 * Start = beginning time in milliseconds                                                                                                       *
 * Returns string with days hours minutes and seconds since start                                                                               *
 ************************************************************************************************************************************************
 */

String timeSince(uint32_t const start) {
  if (start == 0)
    return F("Never");
  uint32_t diff = 0;
  uint32_t now = millis();
  if (start < now)
    diff = now - start;
  else
    diff = UINT32_MAX - start + now;
  diff /= 1000;  // Convert to seconds.
  if (diff == 0)  return "0 seconds";

  // Note: millis() can only count up to 45 days, so uint8_t is safe.
  uint8_t days = diff / (60 * 60 * 24);
  uint8_t hours = (diff / (60 * 60)) % 24;
  uint8_t minutes = (diff / 60) % 60;
  uint8_t seconds = diff % 60;

  String result = "";
  if (days)
    result += String(days) + F(" day");
  if (days > 1)  result += F("s");
  if (hours)
    result += " " + String(hours) + F(" hour");
  if (hours > 1)  result += F("s");
  if (minutes)
    result += " " + String(minutes) + F(" minute");
  if (minutes > 1)  result += F("s");
  if (seconds)
    result += " " + String(seconds) + F(" second");
  if (seconds > 1)  result += F("s");
  result.trim();
  return result;
}


/************************************************************************************************************************************************
 * u64toStr (int)    : Function to convert 64 bit integer (unsigned) to hex string                                                              *
 * number = unsigned 64 bit integer                                                                                                             *
 * Returns string from 0ne to eight hex characters, no leading zero                                                                             *
 ************************************************************************************************************************************************
 */

String u64toStr (uint64_t Value) {

  uint32_t ValueL = Value & 0xFFFFFFFF;
  uint32_t ValueH = Value >> 32;
 
  String result = String(ValueL,HEX);
  
  if (ValueH == 0) {
    return (result);                                            // Done if value is less than 0x10000
  } else {
    while (result.length() < 8 ) { result = "0" + result; }     // pad out the string for the lower 32 bits with zeros
    result = String(ValueH,HEX) + result;                       // the whole string if value > 0xFFFF
    return (result);
  }
}



/************************************************************************************************************************************************
 * Strtou64 (string)      : Fuction to convert hex string to 64 bit unsigned integer                                                            *
 * string = string to convert, from one to eight characters                                                                                     *
 * Returns unsigned 64 bit integer value of hex string                                                                                          *
 ************************************************************************************************************************************************
 */

uint64_t Strtou64(String hexString) {
  
  uint64_t decValue = 0;
  uint64_t nextInt;
  
  for (int i = 0; i < hexString.length(); i++) {
    
    nextInt = int(hexString.charAt(i));
    if (nextInt >= 48 && nextInt <= 57) nextInt = map(nextInt, 48, 57, 0, 9);
    if (nextInt >= 65 && nextInt <= 70) nextInt = map(nextInt, 65, 70, 10, 15);
    if (nextInt >= 97 && nextInt <= 102) nextInt = map(nextInt, 97, 102, 10, 15);
    nextInt = constrain(nextInt, 0, 15);
    
    decValue = (decValue * 16) + nextInt;
  }
  
  return decValue;
}
 

/************************************************************************************************************************************************
 *                                                          GetType                                                                             *
 ************************************************************************************************************************************************
 */

byte GetType(void) {

  byte Type = 0;                                 // Type from string, 0 = not valid IR command string

  if (LastPacket.startsWith("T=") && LastPacket.indexOf("V=") > 2) {    // could be valid if it starts with "T=" and has "V=" after

    Type = LastPacket.substring(2,LastPacket.indexOf("V=")).toInt();    // get the type
    if (Type < 1 || Type > 50) { Type = 0;}                             // invalid if not 1 to 50?
  }
  return (Type);
}


/************************************************************************************************************************************************
 *                                                          GetValue                                                                            *
 ************************************************************************************************************************************************
 */

uint64_t GetValue (void) {

  return (Strtou64(LastPacket.substring(LastPacket.indexOf("V=")+2)));      // convert hex characters after "V=" to number  
}


/************************************************************************************************************************************************
 *                                                          ShowCommand                                                                          *
 ************************************************************************************************************************************************
 */

String ShowCommand (uint64_t CCode) {

  for ( int i = 0; i < ArraySize (IRCommands); i++ ) {              // search whole table
    IRCommand ThisCommand;
    PROGMEM_readAnything (&IRCommands[i], ThisCommand);             // pull table record from FLASH
    if (CCode == ThisCommand.Value) { return (ThisCommand.Name); }  // if there's a match
  }
  return (" ");                                                     // no match
}


/************************************************************************************************************************************************
 *                                                          ShowType                                                                            *
 ************************************************************************************************************************************************
 */

String ShowType (int lType) {

  for ( int i = 0; i < ArraySize (IRTypes); i++ ) {                         // search whole table
    IRType ThisType;
    PROGMEM_readAnything (&IRTypes [i], ThisType);                          // get each table record out of FLASH memory
    if (lType == ThisType.Type) { return(ThisType.Name); }                  // if there's a match
  }
  return ("<span style=\"color:red\">" + String(lType) + "</span>");        // no match
}

