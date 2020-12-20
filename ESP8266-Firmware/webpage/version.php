<?php header('Access-Control-Allow-Origin: *'); ?>
<!-- saved from url=(0037)http://karadio.karawin.fr/version.php -->
<!DOCTYPE html>
<html>
<body>

    <p><span class="label label-success">Release <span id="firmware_last">1.9 Rev 7 </span> - Built on 2020/12/20
	</span>
	</p>
	
    New:
	<ul id="ordered">
		<li> 1.9 R7: 
		<ul id="ordered">
		<li>New vs1053b patch Rev 2.9 .</li>
		<li></li>
	</ul></li>	
	<ul id="ordered">
		<li> 1.9 R6: 
		<ul id="ordered">
		<li>Parse playlist fix.</li>
		<li></li>
	</ul></li>
		<li> 1.9 R5: 
		<ul id="ordered">
		<li>json parsing correction for char " in a string</li>
		<li></li>
	</ul></li>
		<li> 1.9 R4: 
		<ul id="ordered">
		<li>LWIP layer tuned. Better errno 11 computing.</li>
		</ul>
		<li> 1.9 R3: 
		<ul id="ordered">
		<li>Instant play: new button "Add/Edit", and update fields on cli commands.</li>
		</ul>
		<li> 1.9 R2: 
		<ul id="ordered">
		<li>AP mode no working. A wrong lwip was the cause. Now 192.168.4.1 is working.</li>
		</ul>
		<li> 1.9 R1: 
		<ul id="ordered">
		<li>Keyboard on web page correction.</li>
		<li>cli.list correction.</li>
		</ul>
		<li> 1.9 R0: 
		<ul id="ordered">
		<li>Use of the RTOS SDK 2.0.0.</li>
		</ul>
    </ul>

    <div class="alert alert-danger">
        <h4 class="trn">Warning!</h4>
        <p class="trn">If you experiment some strange problems with karadio, please check if the adc (A0) pin is wired to Ground if you don't have a control panel.</p>
    </div>
	</p>
</body>
</html>