/*

  Karadioucglib.pde
  
*/

//-------------------------------------------------------
// UnComment the following line if you want the IR remote
#define IR
#define IRLib2
//-------------------------------------------------------

#include "ucglibConf.h"
#include <EEPROM.h>

#ifdef IR
#ifdef IRLib2
#include <IRLibDecodeBase.h>
// uncomment the needed protocols and check in the library that the receive is enabled
#include <IRLib_P01_NEC.h>
//#include <IRLib_P02_Sony.h>
//#include <IRLib_P03_RC5.h>
//#include <IRLib_P04_RC6.h>
//#include <IRLib_P05_Panasonic_Old.h>
//#include <IRLib_P06_JVC.h>
#include <IRLib_P07_NECx.h>
//#include <IRLib_P08_Samsung36.h>
//#include <IRLib_P09_GICable.h>
//#include <IRLib_P10_DirecTV.h>
//#include <IRLib_P11_RCMM.h>
//include additional protocols here
#include <IRLibCombo.h>
#include <IRLibRecvPCI.h>
#else
#include "IRremote.h"
#endif
#endif

#if defined(__AVR_ATmega328P__) 
#define SERIALX Serial
// the pin_playing is high when playing
#define PIN_PLAYING 5
#else
#define SERIALX Serial1
#define PIN_PLAYING 5
#endif

#define PIN_LED 6

// nams <--> num of line
#define STATIONNAME 0
#define STATION1  1
#define STATION2  2
#define IP        3
#define GENRE     2
#define TITLE1    3
#define TITLE11   4
#define TITLE2    5
#define TITLE21   6
#define VOLUME    7

// constants
const int  BAUD            = 28800;  // any standard serial value: 300 - 115200
const int  EEaddr          = 0;     // EEPROM address for storing WPM
const int  EEaddr1         = 2;     // EEPROM address for LCD address
const int  EEaddrIp        = 10;    // EEPROM address for the IP

#ifdef IR
char irStr[4];
// IR define and objects
#ifdef IRLib2
#define PIN_IRRECV  2
IRrecvPCI irrecv(PIN_IRRECV);//create instance of receiver using pin PIN_IRRECV
IRdecode irDecoder;   //create decoder
IRdecode results; //create decoder
#else
#define PIN_IRRECV	11
IRrecv irrecv(PIN_IRRECV); // The IR 
decode_results results; 
#endif
#endif

bool state = false; // start stop on Ok ir key
//-----------
#ifdef IR
// Character array pointers
   char  msg[]      = {"Karadio IR+clcd V1.0"}; //
#else
	 char  msg[]      = {"Karadio clcd V1.0"}; //
#endif
   char  msg1[]    = {"(c) KaraWin"}; //
   char  msg2[]    = {"https://hackaday.io/project/11570-wifi-webradio-with-esp8266-and-vs1053"};
      
// Karadio specific data
#define BUFLEN  170
#define LINES	8
char line[BUFLEN]; // receive buffer
char station[BUFLEN]; //received station
char title[BUFLEN];	// received title
char nameset[BUFLEN]; // the local name of the station
byte volume;
char nameNum[5]; // the number of the station

char* lline[LINES] ; // array of ptr of n lines 
uint8_t  iline[LINES] ; //array of index for scrolling
uint8_t  tline[LINES] ; // tempo at end or begin
uint8_t  mline[LINES] ; // mark to display

unsigned index = 0; // receive buffer index
unsigned loopcount = 0; // loop counter
char oip[20];

int16_t y ;		//Height between line
int16_t yy;		//Height of screen
int16_t x ;		//Width of the screen
int16_t z ;		// an internal offset for y

////////////////////////////////////////
//Setup all things, check for contrast adjust and show initial page.
// setup on esp reset
void setup2(void)
{
  lline[0] = msg;
  lline[1] = msg1;
  lline[2] = msg2;
  nameNum[0]=0;
  eepromReadStr(EEaddrIp, oip);
  lline[3] = (char*)"IP:";
  lline[4] = oip;
  drawFrame();
  digitalWrite(PIN_PLAYING, LOW);
}
//Setup all things, check for contrast adjust and show initial page.
void setup(void) {
   char  msg3[] = {"Karadio"};
   Serial.begin(9600);
   SERIALX.begin(BAUD);

  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_PLAYING, OUTPUT);
  digitalWrite(PIN_PLAYING, LOW);

#ifdef IR
  irrecv.enableIRIn(); // Start the IR receiver
 irStr[0] = 0;
#endif

#if defined(__AVR_ATmega32U4__)
// switch off the leds
  TXLED1;
  RXLED1;
#endif

// start the graphic library  
  ucg.begin(UCG_FONT_MODE_TRANSPARENT);
  ucg.clearScreen();
  ucg.setRotate90();
  if (ucg.getWidth() == 84)
  ucg.setFont(ucg_font_6x10_tf);
  else 
  ucg.setFont(ucg_font_6x13_tf);
  ucg.setFontPosTop();
// some constant data
  y = - ucg.getFontDescent()+ ucg.getFontAscent() +4; //interline
  yy = ucg.getHeight(); // screen height
  x = ucg.getWidth();   // screen width
  z = 0; 
  //banner
  ucg.setColor(0, 255, 0);
  for (int i = 0;i<3;i++)
  {
    if (i%2) ucg.setColor(0, 255, 0);
    else ucg.setColor(255, 255, 0);
    ucg.drawString(ucg.getWidth()/2 - (ucg.getStrWidth(msg3)/2), ucg.getHeight()/3,0, msg3);  
    delay(500);
    if (i%2) ucg.setScale2x2(); 
    else ucg.undoScale(); 
    ucg.clearScreen();
  }
  ucg.undoScale(); 
  lline[0] = msg;
  lline[1] = msg1;
  lline[2] = msg2;
  nameNum[0]=0;
  eepromReadStr(EEaddrIp, oip);
  lline[3] = (char*)"IP:";
  lline[4] = oip;
  drawFrame();
  delay(500);
}



////////////////////////////////////////
// Clear all buffers and indexes
void clearAll()
{
      title[0] = 0;
      station[0]=0;
	  for (int i=1;i<LINES;i++) {lline[i] = NULL;iline[i] = 0;tline[i] = 0;mline[i]=1;}
}
////////////////////////////////////////
void cleartitle()
{
     title[0] = 0;
     for (int i = 3;i<LINES-1;i++)  // clear lines
     {
       lline[i] = NULL;
	     iline[i] = 0;
	     tline[i] = 0; 
       mline[i] = 1;
     }  
}

////////////////////////////////////////
void removeUtf8(byte *characters)
{
  int index = 0;
  while (characters[index])
  {
    if ((characters[index] >= 0xc2)&&(characters[index] <= 0xc3)) // only 0 to FF ascii char
    {
      //      SERIALX.println((characters[index]));
      characters[index+1] = ((characters[index]<<6)&0xFF) | (characters[index+1] & 0x3F);
      int sind = index+1;
      while (characters[sind]) { characters[sind-1] = characters[sind];sind++;}
      characters[sind-1] = 0;
    }
    index++;
  }
}

////////////////////////////////////////
void eepromReadStr(int addr, char* str)
{
  byte rd;
  do {
    rd = EEPROM.read(addr++);
    *str = rd;
//    SERIALX.println(str[0],16);
    str++;
  } while (( rd != 0)&&( rd != 0xFF)); 
  *str = 0;
}

////////////////////////////////////////
void eepromWriteStr(int addr, char* str)
{
  byte rd;
  do {
    EEPROM.write( addr++,*str);
    rd = *str;
    str++;
  } while (( rd != 0)&&( rd != 0xFF)); 
  EEPROM.write( addr,0);
}

////////////////////////////////////////
void separator(char* from)
{
    char* interp;
//    len = strlen(from);
    while (from[strlen(from)-1] == ' ') from[strlen(from)-1] = 0; // avoid blank at end
    while ((from[0] == ' ') ){ strcpy( from,from+1); }
    interp=strstr(from," - ");
	if (from == nameset) {lline[0] = nameset;lline[1] = NULL;lline[2] = NULL;return;}
	if (interp != NULL)
	{
	  from[interp-from]= 0;
	  lline[(from==station)?STATION1:TITLE1] = from;
	  lline[(from==station)?STATION2:TITLE2] = interp+3;
    mline[(from==station)?STATION1:TITLE1]=1;
    mline[(from==station)?STATION2:TITLE2]=1;
	} else
	{
	  lline[(from==station)?STATION1:TITLE1] = from;
    mline[(from==station)?STATION1:TITLE1]=1;
	}

// 2 lines for Title
 if ((from == title)&&(ucg.getStrWidth(lline[TITLE1]) > x))
 {
    int idx = strlen(lline[TITLE1]);
    *(lline[TITLE1]+idx) = ' ';
    *(lline[TITLE1]+idx+1) = 0;
    while ((ucg.getStrWidth(lline[TITLE1]) > x)&&(idx !=0))
    {
      *(lline[TITLE1]+idx--)= ' ';
      while ((*(lline[TITLE1]+idx)!= ' ')&&(idx !=0)) idx--;
      if (idx != 0) *(lline[TITLE1]+idx)= 0;
    }
    lline[TITLE11] = lline[TITLE1]+idx+1;
    mline[TITLE11]=1; 
 }
 
 if ((from == title)&&(ucg.getStrWidth(lline[TITLE2]) > x))
 {
    int idx = strlen(lline[TITLE2]);
    *(lline[TITLE2]+idx) = ' ';
    *(lline[TITLE2]+idx+1) = 0;
    while ((ucg.getStrWidth(lline[TITLE2]) > x)&&(idx !=0))
    {
      *(lline[TITLE2]+idx--)= ' ';
      while ((*(lline[TITLE2]+idx)!= ' ')&&(idx !=0)) idx--;
      if (idx != 0) *(lline[TITLE2]+idx)= 0;
    }
    lline[TITLE21] = lline[TITLE2]+idx+1;
    mline[TITLE21]=1; 
 }

}
////////////////////////////////////////
// parse the karadio received line and do the job
void parse(char* line)
{
  char* ici;
//Serial.print("received: ");Serial.println(line); 
   removeUtf8((byte*)line);
   
 //////  reset of the esp
   if ((ici=strstr(line,"VS Version")) != NULL) 
   {
      clearAll();
      setup2();
   }
   else
 ////// Meta title   
   if ((ici=strstr(line,"META#: ")) != NULL)
   {     
     cleartitle(); 
     strcpy(title,ici+7);    
	   separator(title); 
   } else 
    ////// ICY4 Description
    if ((ici=strstr(line,"ICY4#: ")) != NULL)
    {
	    strcpy(title,ici+7);
	    if (lline[GENRE] == NULL)lline[GENRE] = title;
      markDraw(GENRE);  
    } else 
 ////// ICY0 station name
   if ((ici=strstr(line,"ICY0#: ")) != NULL)
   {
      clearAll();
	    if (strlen(ici+7) == 0) strcpy (station,nameset);
      else strcpy(station,ici+7);
	    separator(station);
   } else
 ////// STOPPED  
   if ((ici=strstr(line,"STOPPED")) != NULL)
   {
       digitalWrite(PIN_PLAYING, LOW);
	     cleartitle();
       strcpy(title,"STOPPED");
       lline[TITLE1] = title;
       mline[TITLE1] = 1;
   }    
 /////// Station Ip      
   else  
   if ((ici=strstr(line,"Station Ip: ")) != NULL) 
   {
       eepromReadStr(EEaddrIp, oip);
       if ( strcmp(oip,ici+12) != 0)
         eepromWriteStr(EEaddrIp,ici+12 ); 
   } else
 //////Nameset
   if ((ici=strstr(line,"MESET#: ")) != NULL)  
   {
     strcpy(nameset,ici+8);
	   ici = strstr(nameset," ");
	   strncpy(nameNum,nameset,ici-nameset+1);
	   nameNum[ici - nameset+1] = 0;
	   strcpy(nameset,nameset+strlen(nameNum));
     lline[STATIONNAME] = nameset;
     markDraw(STATIONNAME);          
   } else
 //////Playing
   if ((ici=strstr(line,"YING#")) != NULL)  
   {
	   digitalWrite(PIN_PLAYING, HIGH);
/*     if (strcmp(title,"STOPPED") == 0)
     {
		   title[0] = 0;
		   separator(title);
     }*/
   } else
 //////Volume
   if ((ici=strstr(line,"VOL#:")) != NULL)  
   {
      volume = atoi(ici+6);
      markDraw(VOLUME);  
   }
}

////////////////////////////////////////
// receive the esp8266 stream

void serial()
{
    char temp;
    while ((temp=SERIALX.read()) != -1)
    {
	    switch (temp)
	    {
        case '\\':break;
		    case '\n' : if (index == 0) break;
		    case '\r' :
				line[index] = 0; // end of string
				index = 0;
				parse(line);
				break;
		    default : // put the received char in line
				line[index++] = temp;
        if (index>BUFLEN-1) 
        {
         line[index] = 0;
         index = 0;
         parse(line);
        }       
	    }
    }
}

// Mark the lines to draw
void markDraw(int i)
{
  mline[i] = 1;
}
////////////////////////////////////////
// draw the full screen
void drawLines()
{
     for (int i=0;i<LINES;i++)
     {
      if (mline[i]) draw(i); 
      serial();
     }
}
////////////////////
void drawFrame()
{
    ucg.clearScreen();
    ucg.setColor(0,255,255,0);  
    ucg.setColor(1,0,255,255);  
    ucg.drawGradientLine(0,(4*y) - (y/2)-5,x,0);
    ucg.setColor(0,255,255,255);  
    ucg.drawBox(0,0,x-1,(x == 84)?10:13);  
    for (int i=0;i<LINES;i++) draw(i);
}
//////////////////////////
// set color of font per line
void setColor(int i)
{
        switch(i){
          case STATIONNAME: ucg.setColor(0,0,0); break;
          case STATION1: ucg.setColor(255,255,255); break;
          case STATION2: ucg.setColor(255,200,200);  break;
          case TITLE1:
          case TITLE11: ucg.setColor(255,255,0);  break;
          case TITLE2:
          case TITLE21: ucg.setColor(0,255,255); break; 
          case VOLUME:  ucg.setColor(200,200,255); break; 
          default:ucg.setColor(100,255,100);  
        }  
}
////////////////////
// draw one line
void draw(int i)
{
//  Serial.print("Draw ");Serial.print(i);Serial.print("  ");Serial.println(lline[i]);
     if ( mline[i]) mline[i] =0;
      if (i >=3) z = y/2 ; else z = 0;
			if (i == STATIONNAME) 
			{	
        ucg.setColor(255,255,255); 	
        ucg.drawBox(0,0,x,((x == 84)?10:13)-ucg.getFontDescent()); 	
        ucg.setColor(0,0,0);	
				if (nameNum[0] ==0)  ucg.drawString(1,0,0,lline[i]+iline[i]);
				else 
				{
					ucg.drawString(1,0,0,nameNum);
					ucg.drawString(ucg.getStrWidth(nameNum)-2,0,0,lline[i]+iline[i]);
				}
			} else
      if (i == VOLUME)
      {
        ucg.setColor(0,0,200);   
        ucg.drawBox(0,yy-4,x,4); 
        ucg.setColor(255,0,0); 
        ucg.drawBox(1,yy-3,((uint16_t)(x*volume)/255),2);                  
      }
			else 
			{
        ucg.setColor(0,0,0); 
        ucg.drawBox(0,y*i+z,x,((x == 84)?10:13)-ucg.getFontDescent()); 
        setColor(i);
			  ucg.drawString(0,y*i+z,0,lline[i]+iline[i]);        
			}
}

////////////////////////////////////////
// scroll each line if out of screen
void scroll()
{
int16_t len;

	for (int i = 0;i < LINES;i++)
	{  
	   serial();
	   if (tline[i]>0) 
	   {
	     if (tline[i] == 6) 
	     {
	      iline[i]= 0;
	      if (ucg.getStrWidth(lline[i]) > x)draw(i);
	     }
	     tline[i]--;		 
	   } 
	   else
	   {
		   if (i == 0)
			 len = ucg.getStrWidth(nameNum) + ucg.getStrWidth(lline[i]+iline[i]);
		   else
			 len = ucg.getStrWidth(lline[i]+iline[i]);
		   if (len > x)
      {
		    iline[i] += x/6;
        len = iline[i];
        while ((*(lline[i]+iline[i])!=' ')&&(*(lline[i]+iline[i])!='-')&&(iline[i]!= 0))iline[i]--;
        if (iline[i]==0) iline[i]=len;
		    draw(i);
      }
		   else 
			  {tline[i] = 8;}
	   }
	}
}

////////////////////////////
#ifdef IR
// a number of station in progress...
void nbStation(char nb)
{
  if (strlen(irStr)>=3) irStr[0] = 0;
  uint8_t id = strlen(irStr);
  irStr[id] = nb;
  irStr[id+1] = 0;
}
////////////////////////////
void translateIR() // takes action based on IR code received
//  KEYES Remote IR codes (NEC P01)
//  and Ocean Digital remote (NEC P07)
{
#ifdef IRLib2
  if (irrecv.getResults())
  {
    results.decode();    
#else
	if ((irrecv.decode(&results)))
  {
#endif  

//  Uncomment the following line to see the code of your remote control and report to the case the value
#ifdef IRLib2
//	    SERIALX.print("Protocol:");SERIALX.print(results.protocolNum);SERIALX.print("  value:");SERIALX.println(results.value,HEX);
#else
//      SERIALX.print("Protocol:");SERIALX.print(results.decode_type);SERIALX.print("  value:");SERIALX.println(results.value,HEX);
#endif
//      SERIALX.print("  value:");SERIALX.println(results.value,HEX);
		switch(results.value)
		{
			case 0xFF629D: 
			case 0x10EF48B7:	/*(" FORWARD");*/  irStr[0] = 0;SERIALX.print("cli.next\r"); break;

			case 0xFF22DD:
			case 0x10EFA857:
			case 0x10EF42BD: /*(" LEFT");*/  irStr[0] = 0;SERIALX.print("cli.vol-\r");  break;

			case 0xFF02FD:
			case 0x10EF7887:		/*(" -OK-");*/
			{  
//      state?SERIALX.print("cli.start\r"):SERIALX.print("cli.stop\r");
        if (strlen(irStr) >0)
        {
          SERIALX.print("cli.play(\"");SERIALX.print(irStr);SERIALX.print("\")\r");
          irStr[0] = 0;
        }
        else
        { 
          state?SERIALX.print("cli.start\r"):SERIALX.print("cli.stop\r");
/*				  if (state)
           SERIALX.print("cli.start\r");
			  	else
			  		SERIALX.print("cli.stop\r");
           */
        }
				state = !state;
        irStr[0] = 0;
				break;
			}
			case 0xFFC23D:
			case 0x10EF28D7:
			case 0x10EF827D: /*(" RIGHT");*/ irStr[0] = 0;SERIALX.print("cli.vol+\r");  break; // volume +
			case 0xFFA857:
			case 0x10EFC837:	/*(" REVERSE");*/ irStr[0] = 0;SERIALX.print("cli.prev\r"); break;
			case 0xFF6897:
			case 0x10EF807F: /*(" 1");*/ nbStation('1');   break;
			case 0xFF9867:
			case 0x10EF40BF: /*(" 2");*/ nbStation('2');   break;
			case 0xFFB04F:
			case 0x10EFC03F: /*(" 3");*/ nbStation('3');   break;
			case 0xFF30CF:
			case 0x10EF20DF: /*(" 4");*/ nbStation('4');   break;
			case 0xFF18E7:
			case 0x10EFA05F: /*(" 5");*/ nbStation('5');   break;
			case 0xFF7A85:
			case 0x10EF609F: /*(" 6");*/ nbStation('6');   break;
			case 0xFF10EF:
			case 0x10EFE01F: /*(" 7");*/ nbStation('7');   break;
			case 0xFF38C7:
			case 0x10EF10EF: /*(" 8");*/ nbStation('8');   break;
			case 0xFF5AA5:
			case 0x10EF906F: /*(" 9");*/ nbStation('9');   break;
			case 0xFF42BD:
			case 0x10EFE817: /*(" *");*/   irStr[0] = 0;SERIALX.print("cli.stop\r"); break;
			case 0xFF4AB5:
			case 0x10EF00FF: /*(" 0");*/ nbStation('0');   break;
			case 0xFF52AD:
			case 0x10EFB847: /*(" #");*/   irStr[0] = 0;SERIALX.print("cli.start\r"); break;
			case 0xFFFFFFFF: /*(" REPEAT");*/break;
			default:;
			/*SERIALX.println(" other button   ");*/
		}// End Case
;	
#ifdef IRLib2
    irrecv.enableIRIn();      //Restart receiver
#else
		irrecv.resume(); // receive the next value
#endif    
	}
} //END translateIR
#endif

////////////////////////////////////////
void loop(void) {
  static unsigned scrl = 0 ;
    drawLines();     
#ifdef IR
    if (loopcount++ == ((x==84)?0x900:0x1000)) // 
    {
		 translateIR();
#else
    if (loopcount++ == ((x==84)?0x1000:0x2000)) // 
    {
#endif
  	  loopcount = 0;
      scrl++;
  		if (scrl%2 == 0) 
  		  digitalWrite(PIN_LED, HIGH);	
  		scroll();	     
      if (scrl%4 == 0)  
        digitalWrite(PIN_LED, LOW);      
    }
}

