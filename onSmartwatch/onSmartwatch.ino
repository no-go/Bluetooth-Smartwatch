// OLED (11 -> MISO/DIN, 13 ->SCK)
#define PIN_CS     9
#define PIN_RESET 10
#define PIN_DC     8

#define BUTTON1    3
#define BUTTON2    4

const byte batLength =  60;

byte hours   = 10;
byte minutes = 10;
byte seconds = 15;
byte tick    = 0;

bool showText = false;
void game();

/* 0 = press  300ms to start game
 * 1 = press  300ms to make hour +1
 * 2 = press  300ms to make minute +1
 */
int modus = 0;

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
    void black(const int & x, const int & y, const int & radius) {
      drawCircle(x,y,radius, BLACK);
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
  return (mv-2800.0)*(batLength-3)/(3400-2800);  
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
  if (modus>0 && tick%2) {
    oled->black(x, y, radius);
  } else {
    oled->circle(x, y, radius);    
  }
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

void setup() {
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  Serial.begin(115200);
  oled->begin();
  batteryIcon();
  oled->on();
  oled->display();
  delay(3000);
  oled->clear();
}

void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == -61) continue; // symbol before utf-8
    if (inChar == -62) continue; // other symbol before utf-8
    if (showText == false) {
      showText = true;
      oled->clear();
      oled->setCursor(0,0);
    }
  
    if (inChar == '\n' || inChar == '\r') {
      oled->clear();
      oled->setCursor(0,0);
      showText = false;
      continue;
    }
    oled->print(oled->umlReplace(inChar));
  }
}

inline void ticking() {
  tick++;
  if (tick > 9) {
    seconds += tick/10;
  }
  
  if (tick > 9) {
    tick = tick % 10;
    if (showText == false) oled->clear();
    if (seconds > 59) {
      minutes += seconds / 60;
      seconds  = seconds % 60;
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
}

void loop() {
  if(showText == false) {
    anaClock();
  }
  batteryIcon();
  oled->display();
  // 10ms in vcc mesurement (86+10 = in 3h+1min you lost 13min - 1sec was 70ms to slow)
  // 10ms in vcc mesurement (79+10 = in   88min you lost 90sec - 1sec was 13ms to slow)
  delay(77); // 20ms faster
  if (tick == 1) delay(7); // 7ms slower

  if (digitalRead(BUTTON1) == LOW) {
    delay(300);
    tick += 3;
    if (digitalRead(BUTTON1) == LOW) {

      oled->setCursor(0,0);
      showText = false;
      
      switch(modus) {
        case 1:
          hours++;
          seconds=0;
          tick=0;
          break;
        case 2:
          minutes++;
          seconds=0;
          tick=0;
          break;
        default:
          game();
          break;
      }
    }
  }

  if (digitalRead(BUTTON2) == LOW) {
    delay(300);
    tick += 3;
    if (digitalRead(BUTTON2) == LOW) {
      oled->setCursor(0,0);
      showText = false;
      modus++;
      if(modus>2) modus=0;
    }
  }

  ticking();
}















#include <EEPROM.h>
int eeAddress = 0;
int score     = 0;
int highscore = 0;

void setByte(byte b, int x, int y) {
  int tmp;
  if (x<0 || x>63 || y<0 || y>47) return;
  for (byte bitNr=0; bitNr<8; ++bitNr) {
    if (((b >> bitNr) & 0x01)) {
      tmp = 7-bitNr+x;
      if (tmp<0 || tmp>63) continue;
      oled->pixel(tmp, y);
    }
  }
}

void setByte90(byte b, int x, int y) {
  int tmp;
  if (x<0 || x>63 || y<0 || y>47) return;
  for (byte bitNr=0; bitNr<8; ++bitNr) {
    if (((b >> bitNr) & 0x01)) {
      tmp = 7-bitNr+y;
      if (tmp<0 || tmp>47) continue;
      oled->pixel(x, tmp);
    }
  }
}

void dino(byte y) {
  byte i;
  static const byte a[] ={
    0b00111000,
    0b00101100,
    0b00111110,
    0b00111110,
    0b00111000,
    0b00111110,
    0b10011000,
    0b01111000,
    0b00111100,
    0b00011000
  };
  for (i=0; i<sizeof(a); ++i) {
    setByte(a[i], 2, i+y);
  }
}

void died() {
  byte i;
  static const byte a[] ={
    0b00011100,
    0b01111100,
    0b01010100,
    0b01101100,
    0b01010100,
    0b01111100,
    0b00111100,
    0b00001100,
    0b00011100,
    0b00011100,
    0b00001100,
    0b00111100
  };
  for (i=0; i<sizeof(a); ++i) {
    setByte90(a[i], 26-i, 38);
  }
}

void feet1(const byte & y) {
  byte i;
  static const byte a[] ={ 
    0b00111000,
    0b00100100
  };
  for (i=0; i<sizeof(a); ++i) {
    setByte(a[i], 2, i+y);
  }
}

void feet2(const byte & y) {
  byte i;
  static const byte a[] ={
    0b01111000,
    0b00001000
  };
  for (i=0; i<sizeof(a); ++i) {
    setByte(a[i], 2, i+y);
  }
}

void feet3(const byte & y) {
  byte i;
  static const byte a[] ={
    0b00110000,
    0b00101000
  };
  for (i=0; i<sizeof(a); ++i) {
    setByte(a[i], 2, i+y);
  }
}

void printCactus(const int & x) {
  byte i;
  int tmp;
  static const byte a[] = {
    0b00100000,
    0b01100000,
    0b00101000,
    0b10101000,
    0b10101000,
    0b11111000,
    0b01111000,
    0b00111000,
    0b00110000,
    0b00110000,
    0b00100000,
    0b00100000
  };
  for (i=0; i<sizeof(a); ++i) {
    setByte(a[i], x, i+33);
  }
}

void gameStart() {
  oled->clear();
  EEPROM.get(eeAddress, highscore);
  oled->setCursor(0, 0);
  oled->print('H');
  oled->print('i');
  oled->print('g');
  oled->print('h');
  oled->print('s');
  oled->print('c');
  oled->print('o');
  oled->print('r');
  oled->print('e');
  oled->print(':');
  oled->print('\n');
  oled->print(highscore);
  oled->display();
  delay(3000); // to see the highscore!
  tick += 30;
}

void gameOver() {
  if (score > highscore) {
    // store it in a persistent flash ROM
    highscore = score;
    EEPROM.put(eeAddress, highscore);
  }
  oled->setCursor(0, 0);
  oled->black(1);
  oled->setCursor(0, 0);
  oled->print(0);
  oled->setCursor(8, 22);
  oled->print('G');
  oled->print('a');
  oled->print('m');
  oled->print('e');
  oled->print(' ');
  oled->print('O');
  oled->print('v');
  oled->print('e');
  oled->print('r');
  oled->display();
  delay(3500); // read your score
  tick += 35;
}

void game() {
  score = 0;
  byte gamespeed;
  byte lives = 3;
  byte jumpY = 0;
  byte cloud = 56;
  short cactus1 = 70;
  int subTick = 0;

  gameStart();
    
  while(lives > 0) {
    // score is time :-D
    score++;
    
    oled->clear();
    oled->setCursor(0, 0);
    oled->print(lives);
    oled->setCursor(20, 0);
    oled->print(score);

    // cloud
    if (cloud == 0) cloud=56;
    oled->setCursor(cloud, 10);
    oled->print('*');
    if (score%2 == 0) cloud--;
      
    // ground
    oled->line(0, 45, 63, 45);

    // cactus
    if (cactus1 < -7) cactus1=70;
    printCactus(cactus1);
    cactus1-=2;


    // collision and die ---------------
    if (cactus1 == 6 && jumpY<13) {
      lives--;
      died();
      
      oled->display();
      delay(1000);
      tick += 10;
      
    } else {
      // a good jump?
      if (jumpY>=13 && cactus1 == 6) score += jumpY;
      
      // alive -------------------------      
      dino(32-jumpY);
      // print the trippling feeds
      if (score%3     == 0) feet1(42-jumpY);
      if ((score+1)%3 == 0) feet2(42-jumpY);
      if ((score+2)%3 == 0) feet3(42-jumpY);
      oled->display();
    }

    // speedup ?!
    if (score > 4000) {
      gamespeed = 10;
    } else if (score > 3000) {
      gamespeed = 20;
    } else if (score > 1500) {
      gamespeed = 30;
    } else if (score > 1000) {
      gamespeed = 40;
    } else if (score > 500) {
      gamespeed = 50;
    } else {
      gamespeed = 70;
    }
    
    subTick += gamespeed;
    delay(gamespeed);
    if (subTick > 100) {
      subTick = 0; ticking();
    }

    // jump animation + sound
    switch(jumpY) {          
      case 1: jumpY = 7; break;
      case 7: jumpY = 11;  break;
      case 11: jumpY = 13; break;
      case 13: jumpY = 15; break;
      case 15: jumpY = 14; break;
      case 14: jumpY = 12; break;
      case 12: jumpY = 10; break;
      case 10: jumpY = 9;  break;
      case 9:  jumpY = 8; break;
      case 8:  jumpY = 6; break;
      case 6: jumpY = 4;  break;
      case 4: jumpY = 2;  break;
      case 2: jumpY = 0; break;
    }

    // jump button
    if (jumpY==0 && digitalRead(BUTTON1) == LOW) jumpY = 1;
  }
  gameOver();
}
