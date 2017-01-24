#include "u8glibConf.h"
/*

  KaradioU8glib.pde
  
*/

#include "u8glibConf.h"
#include <EEPROM.h>

#define PIN_LED 13
#define PIN_PLAYING 12

// constants
const int  BAUD            = 28800;  // any standard serial value: 300 - 115200
const int  EEaddr          = 0;     // EEPROM address for storing WPM
const int  EEaddr1         = 2;     // EEPROM address for LCD address
const int  EEaddrIp        = 10;    // EEPROM address for the IP

const byte ContrastPin     = 8;     // D8 low activates the Contrast adjustment

// Character array pointers
   char  msg[]       = {"Karadio lcd V1.3"}; //
   char  msg1[]      = {"== (c) KaraWin =="}; //
   char  msg2[]      = {"https://hackaday.io/project/11570-wifi-webradio-with-esp8266-and-vs1053"};
   char  msg3[]	 = {"Karadio"};
   
// Karadio specific data
#define BUFLEN  200
#define LINES	10
char line[BUFLEN]; // receive buffer
char station[BUFLEN]; //received station
char title[BUFLEN];	// received title
char nameset[BUFLEN]; // the local name of the station
char nameNum[5]; // the number of the station
char* lline[LINES] ; // array of ptr of n lines 
int  iline[LINES] ; //array of index for scrolling
char  tline[LINES] ;
char* ici;
unsigned index = 0;
unsigned loopcount = 0;
unsigned scrl = 0;
char oip[20];

int y ;		//Height of a line
int yy;		//Height of screen
int x ;		//Width
int z ;		// an internal offset for y

byte NOKIAcontrast;                 // LCD initialization contrast values B0 thru BF
char temp;


////////////////////////////////////////
void u8g_prepare(void) {
  if (u8g.getWidth() == 84)
	u8g.setFont(u8g_font_6x10);
  else 
	u8g.setFont(u8g_font_6x13);
  u8g.setFontRefHeightExtendedText();
  u8g.setDefaultForegroundColor();
  u8g.setFontPosTop();
}



////////////////////////////////////////
// Clear all buffers and indexes
void clearAll()
{
      title[0] = 0;
      station[0]=0;
	  for (int i=1;i<LINES;i++) {lline[i] = NULL;iline[i] = 0;tline[i] = 0;}
}
////////////////////////////////////////
void cleartitle()
{
     title[0] = 0;
     for (int i = 3;i<LINES;i++)  // clear lines
     {
       lline[i] = NULL;
	   iline[i] = 0;
	   tline[i] = 0; 
     }  
}


////////////////////////////////////////
void eepromReadStr(int addr, char* str)
{
  byte rd;
  do {
    rd = EEPROM.read(addr++);
    *str = rd;
//    Serial.println(str[0],16);
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
    byte len;
    char* interp;
    len = strlen(from);
    while (from[strlen(from)-1] == ' ') from[strlen(from)-1] = 0; // avoid blank at end
    while ((from[0] == ' ') ){ strcpy( from,from+1); }
    interp=strstr(from," - ");
	if (from == nameset) {lline[0] = nameset;lline[1] = NULL;lline[2] = NULL;return;}
	if (interp != NULL)
	{
	  from[interp-from]= 0;
	  lline[(from==station)?1:3] = from;
	  lline[(from==station)?2:4] = interp+3;
	} else
	{
	  lline[(from==station)?1:3] = from;
	}

}
////////////////////////////////////////
// parse the karadio received line and do the job
void parse(char* line)
{
  int mul;
  char* pn;
//   Serial.println(line);
   removeUtf8((byte*)line);
 //////  reset of the esp
   if ((ici=strstr(line,"VS Version")) != NULL) setup2();
   else
 ////// Meta title   
   if ((ici=strstr(line,"META#: ")) != NULL)
   {     
     cleartitle(); 
     strcpy(title,ici+7);    
	 separator(title); 
	 draw(0);	
   } else 
    ////// ICY4 Description
    if ((ici=strstr(line,"ICY4#: ")) != NULL)
    {
	    cleartitle();
	    strcpy(title,ici+7);
	    lline[3] = title;
	    draw(0);
    } else 
 ////// ICY0 station name
   if ((ici=strstr(line,"ICY0#: ")) != NULL)
   {
      int len;
      clearAll();
	  if (strlen(ici+7) == 0) strcpy (station,nameset);
      else strcpy(station,ici+7);
	  separator(station);
	  draw(0);
   } else
 ////// STOPPED  
   if ((ici=strstr(line,"STOPPED")) != NULL)
   {
       digitalWrite(PIN_PLAYING, LOW);
	   cleartitle();
       strcpy(title,"STOPPED");
	   separator(title);
	   draw(0);	
   }    
 /////// Station Ip      
   else  
   if ((ici=strstr(line,"Station Ip: ")) != NULL) 
   {
//   Serial.println(line);
       eepromReadStr(EEaddrIp, oip);
       if ( strcmp(oip,ici+12) != 0)
         eepromWriteStr(EEaddrIp,ici+12 ); 
   } else
 //////Nameset
   if ((ici=strstr(line,"MESET#: ")) != NULL)  
   {
      int len;
      strcpy(nameset,ici+8);
	  pn = strstr(nameset," ");
	  strncpy(nameNum,nameset,pn-nameset+1);
	  nameNum[pn-nameset+1] = 0;
	  strcpy(nameset,nameset+strlen(nameNum));
	  separator(nameset);
             
   } else
 //////Playing
   if ((ici=strstr(line,"YING#")) != NULL)  
   {
	 digitalWrite(PIN_PLAYING, HIGH);
     if (strcmp(title,"STOPPED") == 0)
     {
		 title[0] = 0;
		 separator(title);
	     draw(0);
     }
   }
}


////////////////////////////////////////
//Setup all things, check for contrast adjust and show initial page.
void setup2()
{
	
	clearAll();
	lline[0] = msg;
	lline[1] = msg1;
	lline[2] = msg2;
	nameNum[0]=0;
	eepromReadStr(EEaddrIp, oip);
	lline[3] = (char*)"IP:";
	lline[4] = oip;
	draw(0);
	digitalWrite(PIN_PLAYING, LOW);

}
void setup(void) {
   Serial.begin(BAUD);
   while (!Serial) {
	   ;
   }

    pinMode(PIN_LED, OUTPUT);
	pinMode(PIN_PLAYING, OUTPUT);
    pinMode(ContrastPin, INPUT);
    digitalWrite(ContrastPin, HIGH); // activate internal pullup resistor
	digitalWrite(PIN_PLAYING, LOW);
    
//    Serial.println(F("Free RAM available:")) ;
//    Serial.print(freeRam());

ReStart:  // Come back here if LCD contract is changed
    // Read the EEPROM to determine if display is using a custom contrast value
    NOKIAcontrast = EEPROM.read(EEaddr1);
    // Set the Nokia LCD Contrast to default or reset if EEPROM is corrupt or set to new value
    if (NOKIAcontrast  < 0xB0 || NOKIAcontrast > 0xCF) NOKIAcontrast = 0xB8;
	u8g.setContrast(2*(NOKIAcontrast-0x80));
	u8g_prepare();
	y = u8g.getFontLineSpacing();
	yy = u8g.getHeight();
	x = u8g.getWidth();
	z = 0; 
	clearAll();  

	for (int i = 0;i<5;i++)
	{
	u8g.firstPage();
	do {
//		if (!(i%2)) u8g.drawFrame(0,0,x/2-1,yy/2-1); 
//	    else u8g.drawFrame(0,0,x-1,yy-1); 
		 u8g.drawStr(u8g.getWidth()/2 - (u8g.getStrWidth(msg3)/2), u8g.getHeight()/3, msg3);
	} while( u8g.nextPage() );
	delay(500);
	if (i%2)u8g.setScale2x2(); 
	else u8g.undoScale(); 
	}
	lline[0] = msg;
	lline[1] = msg1;
	//	lline[2] = msg2;
	nameNum[0]=0;
	eepromReadStr(EEaddrIp, oip);
	lline[3] = (char*)"IP:";
	lline[4] = oip;
	draw(0);
	delay(2000);
	if (!digitalRead(ContrastPin)) {
		NOKIAcontrast+=1;
		if (NOKIAcontrast > 0xCF) NOKIAcontrast = 0xB0;
		EEPROM.write(EEaddr1,NOKIAcontrast) ;
		itoa(NOKIAcontrast,title,16);
		lline[2] = title;
		goto ReStart;
	}
	lline[2] = msg2;
	draw(0);

}

////////////////////////////////////////
// receive the esp8266 stream
void serial()
{
    char temp;
    while ((temp=Serial.read()) != -1)
    {
	    switch (temp)
	    {
		    case '\n' : if (index == 0) break;
		    case '\r' :
				line[index] = 0; // end of string
				index = 0;
				parse(line);
				break;
		    default : // put the received char in line
				if (index>BUFLEN-1) break; // small memory so small buffer
				line[index++] = temp;
	    }
    }

}

////////////////////////////////////////
// draw all lines
void draw(int xx)
{
	u8g.firstPage();
	do {
		u8g.drawHLine(0,(4*y) - (y/2)-5,x);
		u8g.drawBox(0,0,x-1,(u8g.getWidth() == 84)?9:12);
		for (int i = 0;i < LINES;i++)
		{
			serial();
			if (i == 0)u8g.setColorIndex(0);
			else u8g.setColorIndex(1);
			if (i >=3) z = y/2 -3; else z = -1;
			if ((lline[i] != NULL))
			if (i == 0) 
			{				
				if (nameNum[0] ==0)  u8g.drawStr(1,0,lline[i]+iline[i]);
				else 
				{
					u8g.drawStr(1,0,nameNum);
					u8g.drawStr(u8g.getStrPixelWidth(nameNum)-2,0,lline[i]+iline[i]);
				}
			}
			else u8g.drawStr(0,y*i+z,lline[i]+iline[i]);
		}
	} while( u8g.nextPage() );
}
////////////////////////////////////////
// scroll each line
void scroll()
{
unsigned len;
	for (int i = 0;i < LINES;i++)
	{
	   
	   if (tline[i]>0) 
	   {
	     if (tline[i] == 4) iline[i]= 0;
	     tline[i]--;		 
	   } 
	   else
	   {
		   if (i == 0)
			 len = u8g.getStrWidth(nameNum) + u8g.getStrWidth(lline[i]+iline[i]);
		   else
			 len = u8g.getStrWidth(lline[i]+iline[i]);
		   if (len > x) iline[i] += 1;
		  else 
			{tline[i] = 6;}
	   }
	}
	draw(0);
}

////////////////////////////////////////
void loop(void) {
	serial();
    if (loopcount++ == ((x==84)?0x3000:0x7000))
    {
	    loopcount = 0;
		if (++scrl%6 == 0) digitalWrite(PIN_LED, HIGH);	
		scroll();	
		digitalWrite(PIN_LED, LOW);
    }
}

