// defines associated specifically with Nokia 5110 LCD ScrnFuncts
#define PIN_SCE   7
#define PIN_RESET 6
#define PIN_DC    5
#define PIN_SDIN  4
#define PIN_SCLK  3

#define LCD_C     LOW
#define LCD_D     HIGH
#define LCD_X     84
#define LCD_Y     48
#define LCD_NLINE 6
#define LCD_NCOL  12

/*  ************************************************ Notes & Changes*********************************************
NOTES:
MiniPRO Pins:
#__      Function_________________________________

RESET    Reset (not used)
Tx  0    n/a (dedicated Serial Input)
Rx  1    Diag Output RS232-TTL 9600 BAUD 
PIN 2    n/a
PIN 3-7  Nokia Display  (specifics below)
PIN 8    Activate changes to NOKIA contrast B0 --> BF GND and press RESET

Raw: 3.3 to 16 VDC Max

Nokia 5110 Graphic LCD Pinout:
_______ Mini Pro____   _______ Nokia GLCD___      
#define PIN_SCE   7    LCD CE ....  Pin 2         
#define PIN_RESET 6    LCD RST .... Pin 1        
#define PIN_DC    5    LCD Dat/Com. Pin 3  (DC)  
#define PIN_SDIN  4    LCD SPIDat . Pin 4  (DIN)  
#define PIN_SCLK  3    LCD SPIClk . Pin 5         

//                     LCD Gnd .... Pin 8          
//                     LCD Vcc .... Pin 6   3.3 volts  from the minipro VCC     
//                     LCD Vled ... Pin 7   (100 to 300 Ohms to Gnd)

Cable wiring between nodeMcu and Mini Pro
-----------------------------------------
--NodeMcu--   --Mini Pro--
Rx            Tx (not used here but useful for touch screen)
Tx            Rx
Gnd           Gnd
VU            Raw

Extra features:
---------------
A Jumper between Nokia GLCD pin 7 and  100 ohm + Gnd to switch on or off the lcd light
A7 ---| |--- 100 ohms --- GND
A push button between PIN8 of mini pro and GND to ajust the contrast after a reset. 
- Press the switch, reset and wait for the right contrast (16 steps) .
- Release the switch. Done.
The contrast is saved in eprom.

Warning:
The webradio serial must be set at 28800 b/s



*/


