The main purpose of “SMART HOME” project is to develop a home monitoring and control system capable to operate the interior lights, turn on the ventilation, close/open the doors etc. Monitoring and control are done using a mobile phone application.
In addition to operate the interior light, turning on/off the ventilation, closing/opening the door, I have added the following functionalities:
•	automatic opening and closing of the door, using a password; 
•	manual opening and closing of a window from the application;
•	switching on for five seconds of an exterior light, when it is night and movement is detected; 
•	automatic closing of the window when rain is detected; 
•	activating an alarm and turning off the interior light if gas is detected.
The system is composed of an Arduino UNO microcontroller and an Android application.
The microcontroller is placed inside the house and its role is to gather and process data from sensors, send it to the Android application, and to control the servo motors. 
The purpose of the Android application is to gather data from the microcontroller for the user to monitor the sensors, or to control the door, window, interior light, ventilation at the user's request.
