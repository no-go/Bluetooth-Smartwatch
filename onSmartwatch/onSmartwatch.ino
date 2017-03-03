// OLED (11 -> MISO/DIN, 13 ->SCK)
#define PIN_CS     9
#define PIN_RESET 10
#define PIN_DC     8

#define CHAR_TIME_REQUEST     '~'
#define CHAR_TIME_RESPONSE    '#' //#HH:mm:ss
#define CHAR_NOTIFY_HINT      '%' //%[byte]

#define MESSAGEPOS     50

#define MEMOSTR_LIMIT 168

const byte batLength =  60;
char memoStr[MEMOSTR_LIMIT] = {'\0'};
int  memoStrPos   = MESSAGEPOS;
int  page         = 0;
byte COUNT        = 0;

byte hours   = 10;
byte minutes = 10;
byte seconds = 40;
byte tick    = 0;

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class OledWrapper : public Adafruit_SSD1306 {
  public:

    OledWrapper(int dc, int res, int cs) : Adafruit_SSD1306(dc,res,cs) {}

    void begin() {
      Adafruit_SSD1306::begin(SSD1306_SWITCHCAPVCC);
      clearDisplay();
      setTextSize(1); // 8 line with 21 chars
      setTextColor(WHITE);
      setCursor(0,0);    
    }

    void black(const int & num) {
      setTextColor(BLACK);
      print(num);  
      setTextColor(WHITE);  
    }

    void line(const int & x, const int & y, const int & xx, const int & yy) {
      drawLine(x,y,xx,yy, WHITE);
    }
    void pixel(const int & x, const int & y) {
      drawPixel(x,y, WHITE);
    }
    void rect(const int & x, const int & y, const int & w, const int & h) {
      drawRect(x,y,w,h, WHITE);
    }
    void rectFill(const int & x, const int & y, const int & w, const int & h) {
      fillRect(x,y,w,h, WHITE);
    }
    void circle(const int & x, const int & y, const int & radius) {
      drawCircle(x,y,radius, WHITE);
    }
    void setFontType(const int & t) {
      setTextSize(t);
    }
    void on() {
      ssd1306_command(SSD1306_DISPLAYON);
    }
    void off() {
      ssd1306_command(SSD1306_DISPLAYOFF);
    }
    void clear() {
      clearDisplay();
    }
    char umlReplace(char inChar) {
      if (inChar == -97) {
        inChar = 224; // ß
      } else if (inChar == -80) {
        inChar = 248; // °
      } else if (inChar == -67) {
        inChar = 171; // 1/2
      } else if (inChar == -78) {
        inChar = 253; // ²
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
      } else if (inChar == -85) {
        inChar = 0xAE; // <<
      } else if (inChar == -69) {
        inChar = 0xAF; // >>
      }
      return inChar;  
    }
};

OledWrapper * oled = new OledWrapper(PIN_DC, PIN_RESET, PIN_CS);
  
byte powerTick(int mv) {
  if (mv < 2800) return 0;
  return (mv-2800.0)*(batLength-3)/(5100-2800);  
}

int readVcc() {
  int mv;
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(10); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring
  mv = ADCL; 
  mv |= ADCH<<8; 
  mv = 1126400L / mv;
  return powerTick(mv);
}

void anaClock() {
  byte x = 60;
  byte y = 31;
  byte radius = 30;
  oled->circle(x, y, radius);
  int hour = hours;
  if (hour>12) hour-=12;
  oled->line(
    x, y,
    x + radius*cos(PI * ((float)seconds-15.0) / 30),
    y + radius*sin(PI * ((float)seconds-15.0) / 30)
  );
  
  oled->line(
    x, y,
    x + (radius-4)*cos(PI * ((float)minutes-15.0) / 30),
    y + (radius-4)*sin(PI * ((float)minutes-15.0) / 30)
  );
  
  oled->line(
    x, y,
    x + (radius-12)*cos(PI * ((float)hour-3.0) / 6),
    y + (radius-12)*sin(PI * ((float)hour-3.0) / 6)
  );
  oled->line(
    x+1, y,
    x-1 +(radius-14)*cos(PI * ((float)hour-3.0) / 6),
    y +  (radius-14)*sin(PI * ((float)hour-3.0) / 6)
  );
  
  for (byte i=0; i<12; ++i) {
    oled->pixel(x + (radius-3)*cos(PI * ((float)i) / 6), y + (radius-3)*sin(PI * ((float)i) / 6));  
  }

  oled->setCursor(x-5,y-radius+4);
  oled->print(12);
  oled->setCursor(x-2,y+radius-11);
  oled->print(6);
  oled->setCursor(x+radius-9,y-3);
  oled->print(3);
  oled->setCursor(x-radius+6,y-3);
  oled->print(9);
}

inline void filler() {
  for (int i=0; i<MESSAGEPOS; ++i) {
    memoStr[i] = ' ';
  }
}

void setup() {
  //Serial.begin(9600); // HC10/nRF
  Serial.begin(115200); // HC05/JDK-08
  oled->begin();
  oled->print('U'); // crazy, but saves dynamic mem
  oled->print('A');
  oled->print('R');
  oled->print('T');
  oled->print('-');
  oled->print('S');
  oled->print('m');
  oled->print('a');
  oled->print('r');
  oled->print('t');
  oled->print('w');
  oled->print('a');
  oled->print('t');
  oled->print('c');
  oled->print('h');
  batteryIcon();
  oled->on();
  oled->display();
  delay(3000);
  oled->clearDisplay();
  filler();
}

void serialEvent() {
  while (Serial.available()) {
    if (memoStrPos >= MEMOSTR_LIMIT) memoStrPos = MEMOSTR_LIMIT;
    char inChar = (char)Serial.read();
    if (inChar == -61) continue; // symbol before utf-8
    if (inChar == -62) continue; // other symbol before utf-8
    if (inChar == '\n') {
      memoStr[memoStrPos] = '\0';
      page = 0;
      continue;
    }
    memoStr[memoStrPos] = oled->umlReplace(inChar);
    memoStrPos++;
    if (memoStrPos >= MEMOSTR_LIMIT) memoStrPos = MEMOSTR_LIMIT;
  }
}

inline void ticking() {
  tick++;
  if (tick > 9) {
    seconds += tick/10;
  }
  
  if (tick > 9) {
    tick = tick % 10;
    oled->clear();
    if (seconds > 59) {
      minutes += seconds / 60;
      seconds  = seconds % 60;
      Serial.println(CHAR_TIME_REQUEST); // ---------------- testing
    }
    if (minutes > 59) {
      hours  += minutes / 60;
      minutes = minutes % 60;
    }
    if (hours > 23) {
      hours = hours % 24;
    }
  }
}

void batteryIcon() {
  byte vccVal = readVcc();

  oled->pixel   (oled->width()-4, oled->height() - batLength);
  oled->pixel   (oled->width()-3, oled->height() - batLength);
  oled->rect    (oled->width()-6, oled->height()  - batLength+1, 6, batLength-1);  
  oled->rectFill(oled->width()-5, oled->height()  - vccVal   -1, 4,      vccVal); 

  oled->pixel(oled->width()-7,  oled->height() - powerTick(3000));
  oled->pixel(oled->width()-8,  oled->height() - powerTick(3000));
  oled->pixel(oled->width()-7,  oled->height() - powerTick(3500));
  oled->pixel(oled->width()-7,  oled->height() - powerTick(4000));
  oled->pixel(oled->width()-8,  oled->height() - powerTick(4000));
  oled->pixel(oled->width()-7,  oled->height() - powerTick(4500));
}

inline byte tob(char c) { return c - '0';}

void loop() {
  anaClock();
  batteryIcon();
  oled->display();
  // 10ms in vcc mesurement (86+10 = in 3h+1min you lost 13min - 1sec was 70ms to slow)
  // 10ms in vcc mesurement (79+10 = in   88min you lost 90sec - 1sec was 13ms to slow)
  delay(77); // 20ms faster
  if (tick == 1) delay(7); // 7ms slower
  

  if (memoStrPos > MESSAGEPOS && page == 0) {

    if (memoStr[MESSAGEPOS] == CHAR_TIME_RESPONSE) {
      
      // extract the time -------------------------
      
      memoStr[MESSAGEPOS] = ' ';
      hours = tob(memoStr[MESSAGEPOS+1])*10 + tob(memoStr[MESSAGEPOS+2]);
      minutes = tob(memoStr[MESSAGEPOS+4])*10 + tob(memoStr[MESSAGEPOS+5]);
      seconds = tob(memoStr[MESSAGEPOS+7])*10 + tob(memoStr[MESSAGEPOS+8]);

    } else if (memoStr[MESSAGEPOS] == CHAR_NOTIFY_HINT) {
      // there is a new message (or a message is deleted)
      
      COUNT = (unsigned char) memoStr[MESSAGEPOS+1];
      page = memoStrPos; // makes a clear and display off        
     }
  }

  // Scrolling message through display
  if (memoStrPos > MESSAGEPOS && page <= memoStrPos) {
    oled->clear();
    oled->setCursor(0, 0);
    oled->print(&(memoStr[page]));
  }

  /// Safe power and switch display off, if message is at the end
  if (page == memoStrPos) {
    // "remove" old chars from buffer
    // print ignores everyting behind \0
    memoStr[MESSAGEPOS] = '\0';
    memoStrPos = MESSAGEPOS;
    page=0;
  }   
  page++;
  ticking();
}

