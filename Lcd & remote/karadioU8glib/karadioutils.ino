
int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}


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

