# SirEdward
Sir Edward is an animatronic skull who lives in Long Beach, CA

The functionality is as follows:
A person in passing triggers the sonar proximity sensor.
The audio, servo, and lighting are all activated and perform, in synchrony, an animitronics sequence.
After the audio file is played, the power to the servo motor is cut (by way of a mosfet) and the circuit is on standby until the next trigger of the proximity sensor.

This project is controlled by a Teensy 3.5 mcu.  Custom PCB in use.  The code points to which pins are connected to which hardware.  
RevB is the first uploaded code, since this iteration of the code represents a sufficiently operational model of what was originally intended for creation.
Subsequent codes will include greater variation in the audio file selection and lighting animations plus some tweaks on the sonar detection.

PWMServo.h library is used rather than Servo.h library - This was to make the servo motion compatible with the Audio library from which its position is 
directly mapped.  PWMServo had less interupts.  

USE_WS2812SERIAL callout -- this is a special pathway created by Paul S. at Teensy where one can program addressable LEDs by way of the Teensy mcu direct memory access using serial communications ports.  This allows us to run simulataneously the servo and the ws2812's without cross-talk, an accomplishment in itself.

With any multithreading code of this type, there should be no delay() calls in the loop. Creative uses of millis() or some timing library strategy are required.  

I would like to thank Fordiman and members of the PJRC.com forum for volunteering to help me complete this project at critical points where I was stuck.  When we reach out we find the answers we needed.

EL, July 2020
