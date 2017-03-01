# DIY Smartwatch (always on)

uses SSD1306 128x64 with 

- mosi 11
- sclk 13
- dc 8
- cs 9
- reset 10

# Features

- clock is analog only
- power level with 3000,3500,4000,4500 mv ticks

send via bluefruit app up to 260 chars - they scroll throuth the clock. send #23:45:59 to set the clock.
Maybe use it with my UART-Smartwatch App (f-droid, github or google-play store (no bluefruit nRF Chip) ).
