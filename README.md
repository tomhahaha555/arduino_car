# arduino_car
performs different tasks on grid paper

The arduino car is composed of a arduino mega 2560, 2 servo motors with wheels, 
various sensors(line	sensor,	color	sensor,	ultrasonic,	photo-resistor), a wireless charging setup, a OLED screen,
a ESP8266 wifi board and a power bank.

A simple gui is program to the oled screen so it serves as a debug tool and control interface.
Also, a web interface in hosted by the ESP8266, which allows user to control the car at a remote position.
It communicates with the board through a MQTT server.

Task 1:
go	to	a	particular	coordinate	with	obstacles on a 5x5 grid paper as instructed. The car traverse through the lines
as in the search algorithm. It would stop searching if the target point cannot be reached.

Task 2:
Two cars are put on the 5x5 grid paper to locate a spot where a tiny red paper has been put on. Two cars can communicate 
so that the first to find the red spot would inform the other to stop. Communication is through a MQTT server.

Task 3:
light tracking. Use a torch as the light source. The car would follow the light and adjust the safe distance.

Task 4:
traverse multiple points by a fastest route. e.g. (1,0), (3,4), (5,3) are instructed, then a fastest route will be calculated.

Task 5:
wireless charging. Charge the power bank.
