/*
 * The MIT License (MIT)

Copyright (c) 2016 Jochen Peters (JotPe, Krefeld)

Permission is hereby granted, free of charge, to any person obtaining 
a copy of this software and associated documentation files (the "Software"), 
to deal in the Software without restriction, including without limitation 
the rights to use, copy, modify, merge, publish, distribute, sublicense, 
and/or sell copies of the Software, and to permit persons to whom the 
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included 
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS 
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
DEALINGS IN THE SOFTWARE.
*/
#include <SPI.h>
#include <SFE_MicroOLED.h>

#define PIN_RESET  9 // Connect RST to pin 9
#define PIN_DC     8 // Connect DC to pin 8
#define PIN_CS    10 // Connect CS to pin 10
#define DC_JUMPER  0

// SPI declaration
MicroOLED oled(PIN_RESET, PIN_DC, PIN_CS);

// not in serial event, because a message is max 
// 20 bytes long and splitted in more then 1 serial
// event
String inStr        = "";
const int buttonPin = A4;
int buttonState     =  0;
int newMessage      =  0;
int delayer         =  0;
String memoStr      = "";
bool freshMsg       = false;
bool isTime         = false;
int hour;
int minute;

// save power, because sin/cos is to "expensive"
const int xHour[] = {29,34,38,39,38,34,29,24,20,19,20,24,29};
const int yHour[] = {21,22,26,31,36,40,41,40,36,31,26,22,21};
const int xMin[]  = {29,30,32,33,34,35,37,38,39,40,40,41,41,42,42,42,42,42,41,41,40,40,39,38,37,35,34,33,32,30,29,28,26,25,24,22,21,20,19,18,18,17,17,16,16,16,16,16,17,17,18,18,19,20,21,23,24,25,26,28};
const int yMin[]  = {18,18,18,19,19,20,20,21,22,23,24,26,27,28,30,31,32,34,35,36,38,39,40,41,42,42,43,43,44,44,44,44,44,43,43,42,42,41,40,39,37,36,35,34,32,31,30,28,27,26,24,23,22,21,20,20,19,19,18,18};

int readVcc() {
  int result; // Read 1.1V reference against AVcc 
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1); 
  delay(10); 
  // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); 
  // Convert 
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL; 
  result |= ADCH<<8; 
  result = 1126400L / result;
  // Back-calculate AVcc in mV 
  //return (result-2700)/13; // scale: 3310 -> 48, 2710 -> 0
  return (result-2700)/26; // scale: 3310 -> 24, 2710 -> 0
}

void setup() {
  pinMode(buttonPin, INPUT);
  digitalWrite(buttonPin, HIGH);
  
  Serial.begin(9600);
  // -- the module stores it !! ----------------
  if (false) {
    Serial.println("+++");
    delay(100);
    Serial.println("ATE=0");
    delay(100);
    Serial.println("AT+HWMODELED=BLEUART");
    delay(100);
    Serial.println("AT+GAPDEVNAME=UART Notify Watch");
    delay(100);
    /* power level lower for better battery life
     -40 : limit  60cm air
     -20
     -16 : limit 100cm throgh a person (Not save near other 2.5GHz devices!)
     -12 ok (8h with 250ms delay in main loop)
      -8 : limit 300cm throgh a person (Not save near other 2.5GHz devices!) 11h with 500ms delay in main loop
      -4 , 0 , 4 // 4 is still not save next to the reciever
    */ 
    Serial.println("AT+BLEPOWERLEVEL=-16");
    delay(300);
    Serial.println("ATZ");
    delay(100);
  }
    
  oled.begin();    // Initialize the OLED
  oled.clear(ALL); // Clear the display's internal memory
  oled.clear(PAGE); // Clear the buffer.
  oled.setFontType(0);
  oled.setCursor(0, 0);
  oled.print("   UART   ----------  Notify  ----------   Watch");
  oled.display();
  delay(1500);
  oled.clear(PAGE);
  oled.display();
  
  Serial.setTimeout(500);
  Serial.readString();
}

void printClock(int dx=0, int dy=0) {
    oled.circle(29+dx, 31+dy, 13);
    hour   = memoStr.substring(11,13).toInt();
    if (hour>12) hour-=12;
    minute = memoStr.substring(14,16).toInt();
    oled.line(29+dx, 31+dy, xMin[minute]+dx, yMin[minute]+dy);
    oled.line(29+dx, 31+dy, xHour[hour]+dx, yHour[hour]+dy);  
}

void loop() {

  if (freshMsg == true && memoStr[0] != '#') {
    freshMsg = false;
    oled.clear(PAGE);
    oled.setFontType(1);
    oled.setCursor(0, 20);
    oled.print("NEW: " + String(newMessage));
    oled.display();
    oled.setFontType(0);
    delayer = 1;
  }
    
  buttonState = digitalRead(buttonPin);
  
  if (buttonState == LOW) {
    int vccVal = readVcc();
    newMessage = 0;
    delayer    = 1;
    
    // show last message
    oled.clear(PAGE);
    oled.setCursor(0, 0);
    oled.print(memoStr);
    oled.rect(60, 24, 4, 24);
    oled.rectFill(60, 48-vccVal, 4, vccVal);

    if (isTime == true) printClock();
    oled.display();

    delay(1000);
    // still down? -> get Time
    buttonState = digitalRead(buttonPin);
    if (buttonState == LOW) {
      delay(200);
      Serial.println( F("t") );
    }

    if (memoStr.length() > 60) {
      delay(2000);
      oled.clear(PAGE);
      oled.setCursor(0, 0);
      oled.print(
        memoStr.substring(60, memoStr.length())
      );
      oled.display();    
    }
  }
  
  // wait 16 = 8 sec, then Disapear message
  if (delayer > 16) {
    delayer = 0;
    oled.clear(PAGE);
    // display 64x48
    int i;
    for (i=0; i<newMessage; i++) {
      oled.pixel(62, i*2);
    }
    oled.display();
  }
  
  if (delayer != 0) {
    delayer++;
    delay(500);
  }
}

void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    
    if (inChar == -61) {
      continue;
    } else if (inChar == -97) {
      inChar = 224; // ß
    } else if (inChar == -92) {
      inChar = 132; // ä
    } else if (inChar == -74) {
      inChar = 148; // ö
    } else if (inChar == -68) {
      inChar = 129; // ü
    } else if (inChar == -124) {
      inChar = 142; // Ä
    } else if (inChar == -106) {
      inChar = 153; // Ö
    } else if (inChar == -100) {
      inChar = 154; // Ü
    } 
    
    if (inChar == '\n') {
      inStr  = "";
      
      if (memoStr[0] == '#') {
        isTime = true;
        // remove hashes
        memoStr[0] = ' ';
        memoStr[9] = ' ';
        oled.clear(PAGE);
        printClock(3, -6);
        oled.display();
        delayer = 1;
      } else {
        newMessage++;
        freshMsg = true;
        isTime   = false;
        delayer = 16;
      }
    } else {
      inStr += inChar;
      memoStr = inStr;
    }
  }
}

