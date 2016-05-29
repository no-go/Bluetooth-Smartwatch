#include <SPI.h>
#include <SFE_MicroOLED.h>
#include <avr/power.h>
#include <avr/sleep.h>

#define PIN_RESET  9
#define PIN_DC     8
#define PIN_CS    10
#define DC_JUMPER  0
#define BUTTON     3
#define MESSAGEPOS 40

// display 64x48
MicroOLED oled(PIN_RESET, PIN_DC, PIN_CS);

String inStr   = "";
String memoStr = "             no transmission";
int page       = -1;
bool incomming = false;
int minute     = 50;
int hour       = 2;
int vccVal     = 0;

const int xHour[13] = {32,40,47,49,47,40,32,23,17,15,17,24,32};
const int yHour[13] = {6,8,14,23,32,38,40,38,31,23,14,8,6};
const int xMin[60]  = {32,34,36,38,41,42,44,46,48,49,50,51,52,53,53,53,53,53,52,51,50,49,48,46,44,42,41,38,36,34,32,30,28,26,23,21,20,18,16,15,14,13,12,11,11,11,11,11,12,13,14,15,16,18,20,22,23,26,28,30};
const int yMin[60]  = {2,2,2,3,4,5,6,7,9,11,12,14,17,19,21,23,25,27,29,32,34,35,37,39,40,41,42,43,44,44,44,44,44,43,42,41,40,39,37,35,33,32,29,27,25,23,21,19,17,14,12,11,9,7,6,5,4,3,2,2};

int readVcc() {
  int result;
  power_adc_enable();
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif  
  delay(10); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring
  result = ADCL; 
  result |= ADCH<<8; 
  result = 1126400L / result;
  power_adc_disable();
  return (result-2700)/26; // scale: 3310 -> 24, 2710 -> 0
}

void wakeUpNow() {}

void sleepNow() {
  // sleep hint
  oled.clear(PAGE);
  oled.setCursor(20, 20);
  oled.print("tzz..");
  oled.display();
  delay(1000);

  oled.command(DISPLAYOFF);
  power_spi_disable(); 
  power_timer0_disable();

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  attachInterrupt(1, wakeUpNow, HIGH); // INT1 PIN3
  sleep_mode();
       
  // here the device is actually put to sleep!!
  // THE PROGRAM CONTINUES FROM HERE AFTER WAKING UP
  sleep_disable();
  power_spi_enable(); 
  power_timer0_enable();
  oled.command(DISPLAYON);
  incomming = true;
  
  // wakeup Icon
  oled.clear(PAGE);
  oled.circle(32, 23, 10);
  oled.pixel(31, 20);
  oled.pixel(35, 19);
  oled.line (29, 27, 35, 27);
  oled.display();
  delay(800);
  oled.rect(29, 19, 3, 3);
  oled.rect(35, 19, 3, 3);
  oled.pixel(35, 26);
  oled.display();  
  delay(800);
}

void setup() {  
  pinMode(BUTTON, INPUT);
  digitalWrite(BUTTON, HIGH);
  Serial.begin(9600);
  
  power_timer1_disable();
  power_timer2_disable();
  power_adc_disable();
  power_twi_disable();
  
  oled.begin();
  oled.clear(ALL); // Clear the display's internal memory logo
  oled.display();
  oled.scrollRight(3,3);
  /*
  Serial.println("+++");
  delay(250);
  Serial.println("ATE=0");
  delay(250);
  Serial.println("AT+HWMODELED=BLEUART");
  delay(250);
  Serial.println("AT+GAPDEVNAME=UART Notify Watch");
  delay(250);
  Serial.println("AT+BLEPOWERLEVEL=4");
  delay(250);
  Serial.println("ATZ");
  */
  delay(1500);
}

char umlReplace(char inChar) {
  if (inChar == -97) {
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
  return inChar;  
}

void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();    
    if (inChar == -61) continue;
    if (inChar == '\n') {
      // MESSAGEPOS spaces as human read buffer
      memoStr = "                                        " + inStr;
      inStr = "";
      page = 0;
      incomming=false;
      continue;
    }
    incomming = true;
    inStr += umlReplace(inChar);
  }
}

void printClock() {
  oled.circle(32, 23, 23);
  hour = memoStr.substring(MESSAGEPOS+1,MESSAGEPOS+3).toInt();
  if (hour>12) hour-=12;
  minute = memoStr.substring(MESSAGEPOS+4,MESSAGEPOS+6).toInt();
  oled.line(32, 23, xMin[minute], yMin[minute]);
  oled.line(32, 23, xHour[hour], yHour[hour]);
  for (int i=0; i<12; ++i) {
    oled.pixel(xHour[i], yHour[i]);  
  }
  // 12 o'clock
  oled.pixel(30, 3);
  oled.pixel(30, 4);
  oled.pixel(30, 5);
  oled.pixel(30, 6);
  oled.pixel(32, 3);
  oled.pixel(33, 3);
  oled.pixel(33, 4);
  oled.pixel(32, 5);
  oled.pixel(33, 6);
  oled.pixel(34, 6);
  // 3 o'clock
  oled.pixel(49, 21);
  oled.pixel(50, 21);
  oled.pixel(50, 22);
  oled.pixel(49, 23);
  oled.pixel(50, 24);
  oled.pixel(49, 25);
  oled.pixel(50, 25);
  // 6 o'clock
  oled.pixel(32, 41);
  oled.pixel(31, 42);
  oled.pixel(30, 43);
  oled.pixel(31, 43);
  oled.pixel(30, 44);
  oled.pixel(31, 44);
  oled.pixel(32, 44);
  oled.pixel(31, 45);
  // 9 o'clock
  oled.pixel(14, 21);
  oled.pixel(13, 22);
  oled.pixel(15, 22);
  oled.pixel(14, 23);
  oled.pixel(15, 23);
  oled.pixel(14, 24);
  oled.pixel(13, 25);

  oled.display();
}

void loop() {
  delay(80);
  if (page >= 0 && page <= memoStr.length()) {

    // if "first" Char is a #, then
    // print on first scroll iter clock
    // for the next 30 iter
    if (page < 30 && memoStr[MESSAGEPOS] == '#') {
      if (page == 0) {
        oled.clear(PAGE);
        // Battery
        oled.rect(60, 24, 4, 24);
        oled.rectFill(60, 48-vccVal, 4, vccVal);
        printClock();
      }
      // last iter: remove sharp
      if (page == 29) memoStr[MESSAGEPOS] = ' ';
    } else {
      oled.clear(PAGE);
      // Battery
      oled.rect(60, 24, 4, 24);
      oled.rectFill(60, 48-vccVal, 4, vccVal);
      // message
      oled.setCursor(0, 0);
      oled.print(memoStr.substring(page));
      oled.display();
    }
  }

  if (page >= 0) page += 1;
  
  if (page > memoStr.length() && incomming==false) {
    page = -1;
    sleepNow();
    vccVal = readVcc();
    Serial.println( F("~") );
    page = 0;
  }

  if (digitalRead(BUTTON) == LOW) {
    vccVal = readVcc();
    page=0;
  }
}

