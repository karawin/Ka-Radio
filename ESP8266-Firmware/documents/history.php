<?php header("Access-Control-Allow-Origin: *"); ?>
<html>
   <body>
   <BR/> 
 <a href="http://karadio.karawin.fr/Interface.txt"  target="_blank"><font color="#14A692">Click here for help on the uart and html interfaces</font></a>
  <BR/><BR/>
<font color="#14A692">PlayLists:</font><BR/>
My playlist: <a href="http://karadio.karawin.fr/WebStations.txt" download> WebStations.txt</a> updated on 2016/10/29<BR/>
<A HREF="mailto:jp@karawin.fr">Send me your playlist to add it here.</A> <br/> <br/>
<font color="red">History:</font><BR/>
<font color="#14A692">Release 1.1.0 </font>  <br/>
<font color="red">New features:</font><br/>
- Drag an drop on the stations list: rearrange the list by dragging a line to another one. Save the change when asked.<br/>
- Themes toggle between light blue and dark marron theme with a click on the logo.<br/>
- Sleep mode to stop the play after a delay.<br/>
- Functions on the web page are optimized. <br/>
- Many minor improvements on the code and the web page.<br/>
<BR/>
<font color="#14A692">Release 1.0.10 </font>  <br/>
<font color="red">New features:</font><br/>
- New uart interface command: cli.uart("x")<br/>
 With x the uart baudrate at the next reset and permanently.<br/>
List of valid baudrate: 1200,2400,4800,9600,14400,19200,28800,38400,57600,76880,115200,230400<br/>
The command will reply with<br/>
##CLI.UART= 115200# on the next reset.<br/>
<br/>
- New Station selection with number<br/>
- Stations management in one page<br/>
- abort button on station edition.<br/>
<font color="red">Corrected:</font><br/>
- The I2S interface was not working. Thanks  Discover1977 for the test
<BR/>
<BR/>
<font color="#14A692">Release 1.0.9 </font>  <br/>
<font color="red">New features:</font><br/>
- New html interface see  <a href="http://karadio.karawin.fr/Interface.txt"  target="_blank"><font color="#14A692">uart and html interfaces</font></a><br/>
- New I2S external DAC interface on the vs1053. The I2S is enabled by default to 48kHz. To change the speed permanently use the uart interface with the command 
	cli.i2s("x") with x =0 for 48kHz, x=1 for 96kHz and x=2 for 192kHz<br/>
	The i2s interface is available on the alientek vs1053 with i2s_sclk on SCLK (GPIO16), i2s_sdata on SDIN (GPIO07), i2s_mclk on MCLK (GPIO05), i2s_lrout on LRCK (GPIO04).<br/>
- New Autostart: If autoplay is checked on the web interface, the current station is started at power on even with no web browser.<br/>
<BR/><BR/>
<font color="#14A692">Release 1.0.8 </font>  <br/>
<font color="red">New features:</font><br/>
- Corrected: <br/>
Some chunked html stations was wrong.<BR/>
If no metadata, the station name is displayed in place<BR/>
A station not found is indicated in the name on the web interface<BR/>
<BR/><BR/>
<font color="#14A692">Release 1.0.7 </font>  <br/>
<font color="red">New features:</font><br/>
- Corrected: The webstation lists was wrong on empty station.<BR/>
<BR/><BR/><font color="#14A692">Release 1.0.6 </font>  <br/>
<font color="red">New features:</font><br/>
- New sdk patch from Espressif see <a href="http://bbs.espressif.com/viewtopic.php?f=46&t=2349" target="_blank">http://bbs.espressif.com/viewtopic.php?f=46&t=2349</a> <BR/>
- Now the maximum number of stations is 256<BR/>
- Detection of external ram is working. If your chip has a /Vhold in place of /VBAT, the pin 7 must be wired to VCC (pin8)<BR/>
- Save stations from... now working with the right filename for edge browser<BR/>
- The AP Password field is masked.<BR/>
- Increased spi clock for external ram<BR/>
<BR/><BR/>
    </body>
</html>


