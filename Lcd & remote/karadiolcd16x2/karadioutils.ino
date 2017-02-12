


int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}
/*
void removeUtf8(byte *characters)
{
	int index = 0;
	while (characters[index])
	{
		if ((characters[index]) == 195)
		{
			characters[index] = 'x';
			//      Serial.println((characters[index]));
			characters[index+1] +=64;
			int sind = index+1;
			switch(characters[sind])
			{
				case 0xFF: case 0xFD:
				characters[sind] = 'y'; break;
				case 0xDF:
				characters[sind] = 'Y'; break;
				case 0xE7:
				characters[sind] = 'c'; break;
				case 0xC7:
				characters[sind] = 'C'; break;
				case 0xF2: case 0XF3: case 0xF4: case 0xF5: case 0xF6:
				characters[sind] = 'o'; break;
				case 0xD2: case 0XD3: case 0xD4: case 0xD5: case 0xD6:
				characters[sind] = 'O'; break;
				case 0xEC: case 0XED: case 0xEE: case 0xEF:
				characters[sind] = 'i'; break;
				case 0xCC: case 0XCD: case 0xCE: case 0xCF:
				characters[sind] = 'I'; break;
				case 0xF9: case 0XFA: case 0xFB: case 0xFC:
				characters[sind] = 'u'; break;
				case 0xD9: case 0XDA: case 0xDB: case 0xDC:
				characters[sind] = 'U'; break;
				case 0xE8: case 0XE9: case 0xEA: case 0xEB:
				characters[sind] = 'e'; break;
				case 0xC8: case 0XC9: case 0xCA: case 0xCB:
				characters[sind] = 'E'; break;
				case 0xE3: case 0XE0: case 0xE1: case 0xE2: case 0xE4: case 0XE5: case 0xE6:
				characters[sind] = 'a'; break;
				case 0xC3: case 0XC0: case 0xC1: case 0xC2: case 0xC4: case 0XC5: case 0xC6:
				characters[sind] = 'A'; break;
				default: ;
			} 
			while (characters[sind]) { characters[sind-1] = characters[sind];sind++;}
			characters[sind-1] = 0;

		}
		index++;
	}
}*/

void removeUtf8(byte *characters)
{
	int index = 0;
	while (characters[index])
	{
		if ((characters[index] >= 0xc2)&&(characters[index] <= 0xc3)) // only 0 to FF ascii char
		{
			//      Serial.println((characters[index]));
			characters[index+1] = ((characters[index]<<6)&0xFF) | (characters[index+1] & 0x3F);
			int sind = index+1;
			while (characters[sind]) { characters[sind-1] = characters[sind];sind++;}
			characters[sind-1] = 0;

		}
		index++;
	}
}

