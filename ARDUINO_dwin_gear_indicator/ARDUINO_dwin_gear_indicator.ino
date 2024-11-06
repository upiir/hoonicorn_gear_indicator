// Gear Indicator with DWIN Intelligent Display using Arduino Leonardo and
// Sparkfun OBDII to UART board to get realtime data from car using OBDII port
// please watch the YouTube tutorial about the details

// created by upir, 2024
// youtube channel: https://www.youtube.com/upir_upir

// YouTube video: https://youtu.be/qqmy5ZxKyq4
// Source files: https://github.com/upiir/hoonicorn_gear_indicator

// Links from the video:
// Do you like this video? You can buy me a coffee ☕: https://www.buymeacoffee.com/upir
// Hoonicorn Mustang video: https://youtu.be/PrqYohBV58o?si=CDgi9yYXTWbcGicc&t=80
// DWIN 4" 480x480 IPS LCD Display: https://s.click.aliexpress.com/e/_DBaVgz7
// DGUS_V7647 editor: https://www.dwin-global.com/tool-page/
// Sparkfun OBDII to UART board: https://www.sparkfun.com/products/9555
// Sparkfun OBDII board on AliExpress: https://s.click.aliexpress.com/e/_Dl1hhvb
// Sparkfun OBDII guide: https://learn.sparkfun.com/tutorials/obd-ii-uart-hookup-guide
// OBDII to DB9 cable: https://s.click.aliexpress.com/e/_DB0w82p
// OBDII simulator: https://s.click.aliexpress.com/e/_DF9q0It
// Arduino Leonardo: https://s.click.aliexpress.com/e/_DnC89Hb
// Breadboard wires: https://s.click.aliexpress.com/e/_Dkbngin
// Nextion Intelligent 5" display: https://s.click.aliexpress.com/e/_DdaDu6B
// Nextion Intelligent 7" display: https://s.click.aliexpress.com/e/_DmikFnz
// Nextion USB to TLL UART board: https://s.click.aliexpress.com/e/_DdLNFEF
// Nextion Editor: https://nextion.tech/nextion-editor/#_section1
// Nextion Instruction set: https://nextion.tech/instruction-set/
// Arduino IDE: https://www.arduino.cc/en/software
// Photopea (online graphics editor like Photoshop): https://www.photopea.com/

// Related videos:
// Ford RS gauges with DWIN display: https://youtu.be/ugS3rKBZVVc
// Digifiz with Nextion display: https://youtu.be/XtSo1BaJ6Bg
// Arduino OBDII Dashboard: https://youtu.be/L3ufJTFX2v0
// Arduino Gear Indicator: https://www.youtube.com/playlist?list=PLjQRaMdk7pBb6r6xglZb92DGyWJTgBVaV
// Temperature gauge: https://youtu.be/A00CvNi1rzQ
// Pitch and roll indicator: https://youtu.be/GosqWcScwC0
// Boost gauge: https://youtu.be/cZTx7T9uwA4
// Pitch and Roll with Nextion display: https://youtu.be/S_ppdbb0poQ
// Parking sensor: https://youtu.be/gg08H-6Z1Lo
// Speedometer: https://youtu.be/dSD02o3M2sw
// Boost gauge with OLED: https://youtu.be/JXmw1xOlBdk




#include <SoftwareSerial.h> // software serial is used for communication with the DWIN display

SoftwareSerial dwin_software_serial(10, 11); // RX, TX - initialize software serial for DWIN display communication

int incomingByte = 0; // for incoming serial data
int jump_to_next_section = 0; // can we jump to the next section (or should we still wait for ">" character)

// this function looks for the received character and does not continue until the ">" character is received
void wait_for_greater_than_sign () {
  jump_to_next_section = 0;
  while (jump_to_next_section == 0) {
    if (Serial1.available() > 0) { // if there is anything being received from the ELM board
      incomingByte = Serial1.read(); // read the incoming byte

      Serial.print((char)incomingByte); // print it back to PC as character
      if (incomingByte == '>') { // if the received character was a greater than sign...
        jump_to_next_section = 1; // jump to the next section
      }
    }
  }
}


void setup() { // run once

  Serial.begin(9600); // start serial communication - USB to PC 
  Serial1.begin(9600); // start serial communication - to Sparkfun OBDII board
  dwin_software_serial.begin(115200); // start serial communication with DWIN display

  delay(4000); // wait 4 seconds - so we can select serial monitor in the meantime

  Serial.println("SEND: ATZ - restart the board"); // send message to PC
  Serial1.println("ATZ"); // send AT command to ELM, ATZ = restart the ELM327, print version

  wait_for_greater_than_sign();

  Serial.println("SEND: ATE0 - turn off the echo"); // send message to PC
  Serial1.println("ATE0"); // send AT command to ELM, ATE0 = echo off

  wait_for_greater_than_sign();

  Serial.println("SEND: ATL1 - add LF (line feed)"); // send message to PC
  Serial1.println("ATL1"); // send AT command to ELM, ATE0 = add LF (line feed)

  wait_for_greater_than_sign();

  Serial.println("SEND: ATS0 - remove spaces in received messages"); // send message to PC
  Serial1.println("ATS0"); // send AT command to ELM, ATS0 = remove spaces in received messages

  wait_for_greater_than_sign();  

  Serial.println("SEND: ATSP0 - automatically detect protocol"); // send message to PC
  Serial1.println("ATSP0"); // send AT command to ELM, ATSP0 = automatically detect protocol

  wait_for_greater_than_sign();   

  Serial.println("SEND: 0100 - get all supported PIDs, it will search for protocol"); // send message to PC
  Serial1.println("0100"); // send AT command to ELM, 0100 = get all supported PIDs, it will search for protocol

  wait_for_greater_than_sign();  

  Serial.println("SEND: ATDP - print protocol name"); // send message to PC
  Serial1.println("ATDP"); // send AT command to ELM, ATDP = print protocol name

  wait_for_greater_than_sign();     


  // setup is complete
}

char received_rpm_string[10]; // received RPM string from ELM
int received_rpm_string_pos; // which character we should replace in the string 
char received_speed_string[10]; // received SPEED string from ELM
int received_speed_string_pos; // which character we should replace in the string 

char rpm_string[5]; // this RPM string only includes the RPM value in HEX stored as string
char speed_string[5]; // this SPEED string only includes the SPEED value in HEX stored as string

unsigned long rpm_value; // string converted to number value
unsigned long speed_value; // string converted to number value

// gauge on Nextion display is made from custom font, those are the characters used in that font (72 characters for full gauges)
char gauge_string[] = "'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmno";

void loop() { // loop all the time

  //  RPM -------------------------------------

  //Serial.println("SEND: 010C - getting the RPM"); // send message to PC
  Serial1.println("010C"); // send to ELM, 010C = get RPM data, the received RPM will be RPM*4

  jump_to_next_section = 0;
  received_rpm_string_pos = 0;
  while (jump_to_next_section == 0) {
    if (Serial1.available() > 0) { // if there is anything being received from the ELM board
      incomingByte = Serial1.read(); // read the incoming byte

      received_rpm_string[received_rpm_string_pos] = incomingByte; // store the incoming byte in the string
      received_rpm_string_pos++; // increase the current character position

      //Serial.print((char)incomingByte); // print it back to PC as character
      if (incomingByte == '>') { // if we have received greater than symbol
        jump_to_next_section = 1; // jump to the next section
        received_rpm_string[received_rpm_string_pos] = 0; // set the last character to 0=NULL, that sets the length of the string
      }
    }
  }

/*  Serial.println("Received RPM string: ");
  Serial.println(received_rpm_string);
  Serial.println("");*/

  // look at the received string, if the first four characters are "410C", this is the response from the OBDII with RPM value, and the next four charcters are RPM*4
  if ((received_rpm_string[0] == '4') && (received_rpm_string[1] == '1') && (received_rpm_string[2] == '0') && (received_rpm_string[3] == 'C')) {
    // store RPM value
    rpm_string[0] = received_rpm_string[4];
    rpm_string[1] = received_rpm_string[5];
    rpm_string[2] = received_rpm_string[6];
    rpm_string[3] = received_rpm_string[7];
    rpm_string[4] = 0; // set the NULL = end of string

    rpm_value = strtoul(rpm_string, NULL, 16); // convert hex stored as string into number (unsigned long variable)
    rpm_value = rpm_value / 4; // reported RPM is RPM*4 -> divide it by 4

    Serial.print("RPM: "); // print the RPM value back to PC
    Serial.print(rpm_value);
    Serial.print("             ");

  }



  //  SPEED -------------------------------------

  //Serial.println("SEND: 010D - getting the SPEED"); // send message to PC
  Serial1.println("010D"); // send to ELM, 010D = get speed value

  jump_to_next_section = 0;
  received_speed_string_pos = 0;
  while (jump_to_next_section == 0) {
    if (Serial1.available() > 0) { // if there is anything being received from the ELM board
      incomingByte = Serial1.read(); // read the incoming byte

      received_speed_string[received_speed_string_pos] = incomingByte; // store the incoming byte in the string
      received_speed_string_pos++; // increase the current character position

      //Serial.print((char)incomingByte); // print it back to PC as character
      if (incomingByte == '>') { // if we have received greater than symbol
        jump_to_next_section = 1; // jump to the next section
        received_speed_string[received_speed_string_pos] = 0; // set the last character to 0=NULL, that sets the length of the string
      }
    }
  }

/*  Serial.println("Received SPEED string: ");
  Serial.println(received_speed_string);
  Serial.println("");*/

  // look at the received string, if the first four characters are "410D", this is the response from the OBDII with SPEED value, and the next two charcters are speed
  if ((received_speed_string[0] == '4') && (received_speed_string[1] == '1') && (received_speed_string[2] == '0') && (received_speed_string[3] == 'D')) {
    // store SPEED value
    speed_string[0] = received_speed_string[4];
    speed_string[1] = received_speed_string[5];
    speed_string[2] = 0; // set the NULL = end of string

    speed_value = strtoul(speed_string, NULL, 16); // convert hex stored as string into number (unsigned long variable)

    Serial.print("SPEED: "); // print the SPEED value back to PC
    Serial.println(speed_value);

  }




  // calculate current gear and send it to DWIN display
  int current_gear = 0;
  int gear_divider = 0;

  gear_divider = rpm_value / speed_value; // calculate gear divider

  if ((rpm_value <= 850) || (speed_value <= 1)) {current_gear = 0;} // neutral = 0
  else if (gear_divider > 97) {current_gear = 1;} // 1st gear
  else if (gear_divider > 59) {current_gear = 2;} // 2nd gear
  else if (gear_divider > 40) {current_gear = 3;} // 3rd gear
  else if (gear_divider > 29) {current_gear = 4;} // 4th gear
  else if (gear_divider > 22) {current_gear = 5;} // 5th gear
  else                        {current_gear = 6;} // 6th gear

  int current_gear_img = current_gear + 1;

  // send data to DWIN display
  dwin_software_serial.write(0x5a); // header
  dwin_software_serial.write(0xa5); // header
  dwin_software_serial.write(0x05); // number of bytes being send
  dwin_software_serial.write(0x82); // send/set VP  
  dwin_software_serial.write(0x20); // address
  dwin_software_serial.write((byte)0x00); // address
  dwin_software_serial.write(highByte(current_gear_img)); // value
  dwin_software_serial.write(lowByte(current_gear_img)); // value

}