/*
  This implementation specific to Arduino Mini-Pro 328 running at 3.3V
  and used with a NOKIA 5110 Craphic LCD.

  Built under Arduino 1.6.10
  Derived from https://create.arduino.cc/projecthub/rayburne/tiny-serial-terminal-151988
*/
#define version 1.0
#include <EEPROM.h>
#include "Defines.h"                // Nokia 5110 LCD pin usage as controlled by "ScrnFuncts.ino"

// constants
const int  BAUD            = 28800;  // any standard serial value: 300 - 115200
const int  EEaddr          = 0;     // EEPROM address for storing WPM
const int  EEaddr1         = 2;     // EEPROM address for LCD address
const int  EEaddrIp        = 10;    // EEPROM address for the IP
// Arduino 328P pins (not physical)

const byte ContrastPin     = 8;     // D8 low activates the Contrast adjustment

// global prog variables

byte nRow;                          // line count      (0-5 for NOKIA LCD)
byte nColumn;                       // character count (0-11 for NOKIA LCD)
byte NOKIAcontrast;                 // LCD initialization contrast values B0 thru BF
char temp;

// Character array pointers
   char  msg[]       = {"Karadio for nokia 84*48 (c) KaraWin  "}; //
   char  BlankLine[]  = {"            "};  // Nokia 12 x 6 (84 * 48)
//   char  anim = {"-\\|/"};


// Karadio specific data
#define BUFLEN  200
char line[BUFLEN]; // receive budder
char station[BUFLEN];
char title[BUFLEN];
char nameset[BUFLEN];
char* ici;
unsigned index = 0;
unsigned iStation = 0;
unsigned iTitle = 0;
unsigned loopcount = 0;
unsigned loopcount1 = 0;
byte loopflip = 0;
char arrow[] = {0x7e};

void setup(void)
  { 
    char oip[20];
    pinMode(13, OUTPUT);
    pinMode(ContrastPin, INPUT);
    digitalWrite(ContrastPin, HIGH); // activate internal pullup resistor

    Serial.begin(BAUD);
//    Serial.println(F("Karadio display nokia 84 * 48"));
    Serial.println(F("Free RAM available:")) ;
    Serial.print(freeRam());

    ReStart:  // Come back here if LCD contract is changed
    // Read the EEPROM to determine if display is using a custom contrast value
    NOKIAcontrast = EEPROM.read(EEaddr1);
    // Set the Nokia LCD Contrast to default or reset if EEPROM is corrupt or set to new value
    if (NOKIAcontrast  < 0xB0 || NOKIAcontrast > 0xBF) NOKIAcontrast = 0xB2;
    // LCD
    LcdInitialise();
    LcdClear();
    LcdString(msg);
    eepromReadStr(EEaddrIp, oip);
    displayStr(oip,4,2); 
    delay(2000);
    if (!digitalRead(ContrastPin)) {
        NOKIAcontrast++;
        if (NOKIAcontrast > 0xBF) NOKIAcontrast = 0xB0;
        EEPROM.write(EEaddr1, NOKIAcontrast);
        goto ReStart;
    }
    
    nRow = 0; nColumn = 0;
    gotoXY(nColumn, nRow);
//    LcdClear();  // nRow = 0; nColumn = 0 done by function LcdClear()
    
}

//scroll this at index istr at row on nline line
void scroll(char* ici,unsigned* istr, byte row, byte nline)
{
  unsigned len = strlen(ici+*istr);

   if ( len > ((nline)*LCD_NCOL)) // somethink to scroll
   {
     for (int i = 0;i<nline;i++)  // clear lines
     {
       gotoXY(0, row+i);
      LcdString(BlankLine); 
     }
     gotoXY(0, row);
     LcdnString(ici+*istr,nline*LCD_NCOL);
     *istr += (nline)*LCD_NCOL;  
   }else
   if ((len > 0))// &&(*istr >0))
   {
     if (*istr >0)
     for (int i = 0;i<nline;i++)  // clear lines
     {
       gotoXY(0, row+i);
      LcdString(BlankLine); 
     }
     gotoXY(0, row);
     LcdnString(ici+*istr,nline*LCD_NCOL); 
      *istr = 0;   
   } else
   {*istr = 0;}
}
// display a string  in row row, and on nline lines
void displayStr(char* ici, byte row, byte nline)
{   
   for (int i = 0;i<nline;i++)  // clear lines
   {
     gotoXY(0, row+i);
     LcdString(BlankLine); 
   }
   gotoXY(0, row);
   LcdnString(ici,nline*LCD_NCOL);
}
// Clear all buffers and indexes
void clearAll()
{
      iStation = 0;
      iTitle = 0;
      title[0] = 0;
      station[0]=0;
      LcdClear();  
      loopcount = 0;
      loopcount1 = -4;
}
void cleartitle()
{
     strcpy(title,arrow);
     title[1] = 0;
     iTitle = 0;
     for (int i = 3;i<6;i++)  // clear lines
     {
       gotoXY(0, i);
      LcdString(BlankLine); 
     }  
}

// cesure for next scroll
void cesure(char* from, int at)
{
     if (strlen(from) > at)
     {
        int i = 0;
        while (( from[at -i] != ' ')&& ( i< LCD_NCOL))
        {i++; }        
        if ((i < LCD_NCOL)&&(i!=1))
        {   
           char tmp[256];   
           strcpy(tmp,from+at-i+1);
           strcpy(from+at,tmp);
           for (i;i>0;i--)
             from[at-i] = ' '; 
        }  
     }
}

int separator(char* from)
{
     byte len;
     char tmp[256];
     char* interp;
     int ret = 1;
     len = strlen(from);
     while (from[strlen(from)-1] == ' ') from[strlen(from)-1] = 0; // avoid blank at end
     while ((from[0] == arrow[0])&& (from[1] == ' ') ){ strcpy( from+1,from+2); }
     interp=strstr(from," - ");
     while( (interp != NULL)&&((interp-from) > LCD_NCOL*ret)) {cesure(from,LCD_NCOL*ret);interp=strstr(from," - ");ret++;}     
//   Serial.println(from);
     if (interp != NULL) 
     {
        len = strlen(from);
        if (from != station)
        {
          strcpy(tmp,arrow);
          strcpy(tmp+1,interp+3);
        } else
        strcpy(tmp,interp+3);
        interp[0]=0;
//    Serial.println(tmp);        
        len -= strlen(tmp)+3;
        len %= LCD_NCOL;
        len = LCD_NCOL -len ;
        /*if (len != 12)*/for (byte i = 0; i<len;i++) interp[i] = ' ';
        strcpy(interp+len, tmp);
     }
     return ret;
}
     
// parse the karadio received line and do the job
void parse(char* line)
{
  int mul;
  removeUtf8((byte*)line);
//   Serial.println(line);
 ////// Meta
   if ((ici=strstr(line,"ETA#: ")) != NULL)
   {
     cleartitle(); 
     strcpy(title+1,ici+6);    
     mul = separator(title);
     while (strlen(title) > mul*LCD_NCOL)
       cesure(title,mul++*LCD_NCOL); 
     scroll(title,&iTitle,3,3) ; 
   } else  
 ////// ICY0
   if ((ici=strstr(line,"Y0#: ")) != NULL)
   {
      int len;
      clearAll();
      LcdCurrentLine(2);
      strcpy(station,arrow);
      station[1] = 0;
      for (int i = 0;i<2;i++)  // clear lines
      {
        gotoXY(0,i);
       LcdString(BlankLine); 
      }
      strcat(station,ici+5); 
      if (strlen(station) < 2) strcpy(station,nameset); 
      mul =  separator(station);
      while (strlen(station) > mul*LCD_NCOL)
        cesure(station,mul++*LCD_NCOL);     
 
      scroll(station,&iStation,0,2);
//      displayStr(station,0,2);
   } else
 ////// STOPPED  
   if ((ici=strstr(line,"STOPPED")) != NULL)
   {
       cleartitle();
       displayStr((char*)"  STOPPED   ",4, 1);
   }    
 /////// Station Ip      
   else  
   if ((ici=strstr(line,"Station Ip: ")) != NULL) 
   {
       char sip[20];
       char oip[20];
       eepromReadStr(EEaddrIp, oip);
       if ( strcmp(oip,ici+12) != 0)
         eepromWriteStr(EEaddrIp,ici+12 );
       displayStr(ici+12,4,2); 
   } else
 //////Nameset
   if ((ici=strstr(line,"ESET#: ")) != NULL)  
   {
      int len;
      strcpy(nameset,arrow);
      nameset[1] = 0;
      for (int i = 0;i<2;i++)  // clear lines
      {
          gotoXY(0,i);
         LcdString(BlankLine); 
      }
      strcat(nameset,ici+7); 
/*        mul =  separator(nameset);
        while (strlen(nameset) > mul*LCD_NCOL)
           cesure(nameset,mul++*LCD_NCOL);     
        scroll(nameset,&iStation,0,2);
*/               
   } else
 //////Playing
   if ((ici=strstr(line,"YING#")) != NULL)  
   {
        if (strlen(station) == 0)
        {
          mul =  separator(nameset);
          while (strlen(nameset) > mul*LCD_NCOL)
            cesure(nameset,mul++*LCD_NCOL);     
          scroll(nameset,&iStation,0,2);         
        }
   }
   for (int i = 0; i< BUFLEN;i++) line[i] = 0; // clear buffer line
}
void eepromReadStr(int addr, char* str)
{
  byte rd;
  do {
    rd = EEPROM.read(addr++);
    *str = rd;
//    Serial.println(str[0],16);
    str++;
  } while (( rd != 0)&&( rd != 0xFF)); 
}
void eepromWriteStr(int addr, char* str)
{
  byte rd;
  do {
    EEPROM.write( addr++,*str);
    rd = *str;
    str++;
  } while (( rd != 0)&&( rd != 0xFF)); 
}
void loop(void)
{
    char temp;
    if ((temp=Serial.read()) != -1)
    {
//      char temp = Serial.read();
//      Serial << temp;  // for diagnostic and Arduino term echo
      switch (temp)
        {
          case '\n' : break;
          case '\r' :
            line[index] = 0; // end of string
            index = 0;
            parse(line);          
           break;
          default : // put the received char in line
            if (index>BUFLEN) break; // small memory so small buffer
            line[index++] = temp;
        }
    }
    
    if (loopcount++ == -1)
    {
      if (loopcount1++ == 2)
      {
        digitalWrite(13, HIGH);
        if ((loopflip++%2) == 0)
          scroll(station,&iStation,0,2);
        else  
          scroll(title,&iTitle,3,3) ;   
       loopcount1 = 0; loopcount = 0;
        digitalWrite(13, LOW);
      }
    }
}

