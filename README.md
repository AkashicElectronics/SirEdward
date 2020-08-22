# SirEdward
Sir Edward is an animatronic skull who lives in Long Beach, CA

The functionality is as follows:
A person in passing triggers the sonar proximity sensor.
The audio, servo, and lighting are all activated and perform, in synchrony, an animitronics sequence.
After the audio file is played, the power to the servo motor is cut (by way of a mosfet) and the circuit is on standby until the next trigger of the proximity sensor.

This project is controlled by a Teensy 3.5 mcu.  I made a custom PCB for it.  The code points to which pins are connected to which hardware.  
I am uploading code starting with revB of the Master Code, since this iteration of the code represents a sufficiently operational model of what was originally intended for creation.
Subsequent codes will include greater variation in the audio file selection and lighting animations plus some tweaks on the sonar detection

You will notice that PWMServo.h library is used rather than Servo.h library.  This was to make the servo motion compatible with the Audio library from which its position is 
directly mapped.  Long story short PWMServo had less interupts.  You will also notice that there is a USE_WS2812SERIAL callout -- this is a special pathway created by Paul S. 
at Teensy where one can program addressable LEDs by way of the Teensy mcu direct memory access using serial communications ports.
This allows us to run simulataneously the servo and the ws2812's without cross-talk, an accomplishment in itself.
Finally, as with any multithreading code of this type, there should be no delay() calls in the loop. Creative uses of millis() or some timing library strategy are required.  

I would like to thank Fordiman and members of the PJRC.com forum for volunteering to help me complete this project at critical points where I was stuck.  Your cosmic interdependent energy is 
its own reward.  

EL, July 2020
