//Akashic Electronics, July 2020 -EL

//Libraries
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <PWMServo.h> 


// Connections
#define SERVO_PIN 10 // servo, jaw motion
#define PING_PIN 5 // ping for sonar sensor
#define ECHO_PIN 6 // echo for sonar sensor
#define MOSFETPin 9 // mostfet for servo, used as a relay
#define LED_DATA_L 32 //left eye
#define LED_DATA_R 33 //right eye

//Addressable LEDs
#include <WS2812Serial.h>
#define USE_WS2812SERIAL //this is crucial.  Paul S.'s DMA Serial library for using WS2812's without interrupts (on Teensy).  See documentation.
#include <FastLED.h>
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
#define NUM_LEDS    10
CRGB leds[NUM_LEDS];
#define BRIGHTNESS          20
#define FRAMES_PER_SECOND   20
#define COOLING  10
#define SPARKING 120
bool gReverseDirection = false;
uint8_t gHue = 0;



//Distance sensor
#define  SONAR_THRESHOLD 2200 // microseconds --- this triggers at about 1 foot 
// Speed of sound in air is about 343 m/s
// Quick calculators:
// const SONAR_THRESHOLD (148 * 6) // ~inches
// const SONAR_THRESHOLD (58 * 15) // ~cm

// Servo
PWMServo jaw;
#define JAW_OPEN 70 //range of servo motion
#define JAW_CLOSED 0
// Length of time for `loop()` to run.  1000/25 Hz (40ms) should be pretty fluid.  However this motor is limited to 200ms.
#define  SAMPLE_TIME 200 // ms
// Delay playback to compensate for the jaw servo's lag:
#define SERVO_DELAY 50 // ms

// Audio
// GUItool: begin automatically generated code
AudioPlaySdWav           playSdWav1;     //xy=329,371
AudioMixer4              mixer1;         //xy=548,386
AudioEffectDelay         delay1;         //xy=749,424
AudioOutputAnalog        dac1;           //xy=951,421
AudioAnalyzePeak         peak1;          //xy=973,360
AudioConnection          patchCord1(playSdWav1, 0, mixer1, 0);
AudioConnection          patchCord2(playSdWav1, 1, mixer1, 1);
AudioConnection          patchCord3(mixer1, delay1);
AudioConnection          patchCord4(delay1, 0, peak1, 0);
AudioConnection          patchCord5(delay1, 1, dac1, 0);
// GUItool: end automatically generated code

#define SDCARD_CS_PIN    BUILTIN_SDCARD
#define SDCARD_MOSI_PIN  11  // not actually used
#define SDCARD_SCK_PIN   13
// File array to play when sonar triggers; one will be selected randomly
char *  SAMPLE_FILE[] = {"01.wav", "02.wav", "03.wav", "04.wav", "05.wav", "06.wav", "07.wav"};
// playSdWav1 will use the built-in sd card slot, assumed to have a fat32 filesystem.

 

// `elapsedMillis` is a teensy type that automatically counts up
elapsedMillis sinceTest1;
elapsedMillis sinceTest2;
elapsedMillis sinceTest3;
elapsedMillis sinceTest4;


// Sonar Funtion:
int ping() {
  digitalWrite(PING_PIN, LOW);
  if (sinceTest1 >= 2) {
    sinceTest1 = sinceTest1 - 2; 
    digitalWrite(PING_PIN, HIGH); }
  if (sinceTest2 >= 10) {
    sinceTest2 = sinceTest2 - 10; 
    digitalWrite(PING_PIN, LOW); }
  if (pulseIn(ECHO_PIN, HIGH, SONAR_THRESHOLD) != 0) {
    return HIGH;
  }
  return LOW;
}


// Mosfet Function:
uint8_t mosfet_state = LOW;
void set_mosfet(uint8_t state) {
  if (state != mosfet_state) {
    Serial.print("Setting MOSFET to ");
    Serial.println(state == LOW ? "LOW" : "HIGH");
    mosfet_state = state;
    digitalWrite(MOSFETPin, mosfet_state);
    }
  }



void setup() {
  Serial.begin(9600);
  Serial.println("initializing audio memory");
  
  randomSeed(analogRead(A0)); //for use with random function

  LEDS.addLeds<WS2812SERIAL, LED_DATA_L, COLOR_ORDER>(leds, NUM_LEDS);
  LEDS.addLeds<WS2812SERIAL, LED_DATA_R, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  
  // Set up audio memory.  You need at least 1 block for each 2.9 ms of delay, plus
  // a minimum of 8 blocks for the player. 
  AudioMemory((int) (ceil(SERVO_DELAY / 2.9) + 8));
  Serial.println("initializing pins");
  jaw.attach(SERVO_PIN);
  pinMode(PING_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(MOSFETPin, OUTPUT);
  delay1.delay(0, 0);    
  delay1.delay(1, SERVO_DELAY);
  delay1.disable(2);
  delay1.disable(3);
  delay1.disable(4);
  delay1.disable(5);
  delay1.disable(6);
  delay1.disable(7);
  
  mixer1.gain(0, 1.0);
  mixer1.gain(1, 1.0);
  mixer1.gain(2, 0);
  mixer1.gain(3, 0);
  
  Serial.println("initializing SD access");
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    // stop here, but print a message repetitively
    while (1) {
      Serial.println(SD.begin(SDCARD_CS_PIN));
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }
}

// Idling Function:
void loop_state_waiting() {
  if (mosfet_state == HIGH) {
    Serial.println("playback ended; returning to sleep.");
     
  }
  set_mosfet(LOW);
  if (ping() == HIGH) {
    Serial.println("sonar ping!  Starting sound/animation.");
    set_mosfet(HIGH);
    if (sinceTest3 >= 5) {
      sinceTest3 = sinceTest3 - 5; 
      byte index = random(0, 6); //call up one of the seven audio files
      playSdWav1.play(SAMPLE_FILE[index]);  
  }
}
}

// ANIMATION FUNCTION:
void loop_state_playing() {
  if (peak1.available()) {
    float level = peak1.readPeakToPeak();
    int new_servo_pos = (int) (level * (JAW_OPEN - JAW_CLOSED) / 1.8 + JAW_CLOSED);
    Serial.print("Level: ");
    Serial.print(level);
    Serial.print("; servo: ");
    Serial.println(new_servo_pos);
    jaw.write(new_servo_pos);
    Fire2012();
    //bpm();
    red();
    //juggle();
    //Eventually I can assign different LED codes to different audio tracks using if else statements.  TBD.
    FastLED.show();
    FastLED.delay(1200 / FRAMES_PER_SECOND);
     
  }
}


//------------------------------------------------LED_STUFF-------------------------------------------------
//FastLED Opensourced Function:
void Fire2012()
{
// Array of temperature readings at each simulation cell
  static byte heat[NUM_LEDS];

  // Step 1.  Cool down every cell a little
    for( int i = 0; i < NUM_LEDS; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= NUM_LEDS - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < SPARKING ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < NUM_LEDS; j++) {
      CRGB color = HeatColor( heat[j]);
      int pixelnumber;
      if( gReverseDirection ) {
        pixelnumber = (NUM_LEDS-1) - j;
      } else {
        pixelnumber = j;
      }
      leds[pixelnumber] = color;
    }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 10; i++) {
    leds[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 255, 255);
    dothue += 32;
  }
}


void bpm()
{
  uint8_t gHue = 0;
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 12;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}


void red()
{
  for( int i = 0; i < 10; i++) {
    leds[i] = CRGB::Maroon; //note the Color Order setting at the beggining ie RGB
}
}

//---------------------------------------END_LED_STUFF-------------------------------------------------


// Last but not least:
void loop() {
  // resets the counter
  //timing = 0;
  if (sinceTest4 >= SAMPLE_TIME) {
      sinceTest4 = sinceTest4 - SAMPLE_TIME ; 
      
     if (!playSdWav1.isPlaying()) {
    loop_state_waiting();
    fadeToBlackBy( leds, NUM_LEDS, 100);
    FastLED.show();
     } else {  
    loop_state_playing();
     }
  }
}

  
