#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <PWMServo.h> 

// Configuration
// Connections
#define SERVO_PIN 10 // Jaw servo
#define PING_PIN 5 // ping / echo for sonar distance sensor
#define ECHO_PIN 6
#define MOSFETPin 9 // mostfet for servo power control

#include <WS2812Serial.h>
#define USE_WS2812SERIAL
#include <FastLED.h>
#define LED_TYPE    WS2812
#define COLOR_ORDER RGB
#define NUM_LEDS    10
CRGB leds[NUM_LEDS];
#define BRIGHTNESS          20
#define FRAMES_PER_SECOND   20
#define COOLING  10
#define SPARKING 120
bool gReverseDirection = false;

// How long until the ping returns to consider it a "trigger"
#define  SONAR_THRESHOLD 900 // microseconds.
// Speed of sound in air is about 343 m/s, so this is about 15 cm (~6 in), or a 30cm round trip.
// Quick calculators:
// const SONAR_THRESHOLD (148 * 6) // ~inches
// const SONAR_THRESHOLD (58 * 15) // ~cm

// Servo range configuration
#define JAW_OPEN 70
#define JAW_CLOSED 0

// Length of time for `loop()` to run.  1000/25 Hz (40ms) should be pretty fluid
#define  SAMPLE_TIME 200 // ms

// Sound file to play when sonar triggers
#define  SAMPLE_FILE "02.wav"

// Delay playback to compensate for the jaw servo's lag.
#define SERVO_DELAY 50 // ms

// To play with this, import the below block into https://www.pjrc.com/teensy/gui/

// playSdWav1 will use the built-in sd card slot, assumed to have a fat32 filesystem.
// If you're using some other way to attach SD, or you'd like to use in-memory or
// whatever, there are other options.

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

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

PWMServo jaw;

// Entirely optional.  For stable sample management.  `elapsedMillis` is a teensy type that
// automatically counts up, so it's useful for making sure if you want 25Hz, you _get_ 25Hz.
elapsedMillis sinceTest1;
elapsedMillis sinceTest2;
elapsedMillis sinceTest3;
elapsedMillis sinceTest4;

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

  LEDS.addLeds<WS2812SERIAL, 32, RGB>(leds, NUM_LEDS);
  LEDS.addLeds<WS2812SERIAL, 33, RGB>(leds, NUM_LEDS);
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

void loop_state_waiting() {
  if (mosfet_state == HIGH) {
    Serial.println("playback ended; returning to sleep.");
    fadeToBlackBy( leds, NUM_LEDS, 30);
  }
  set_mosfet(LOW);
  if (ping() == HIGH) {
    Serial.println("sonar ping!  Starting sound/animation.");
    set_mosfet(HIGH);
    if (sinceTest3 >= 5) {
      sinceTest3 = sinceTest3 - 5; 
      playSdWav1.play(SAMPLE_FILE);  
  }
}
}

void loop_state_playing() {
  if (peak1.available()) {
    float level = peak1.readPeakToPeak();
    int new_servo_pos = (int) (level * (JAW_OPEN - JAW_CLOSED) / 2.0 + JAW_CLOSED);
    Serial.print("Level: ");
    Serial.print(level);
    Serial.print("; servo: ");
    Serial.println(new_servo_pos);
    jaw.write(new_servo_pos);
    Fire2012();
    FastLED.show(); // display this frame
    FastLED.delay(4000 / FRAMES_PER_SECOND); 
  }
}


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


void loop() {
  // resets the counter
  //timing = 0;
  if (sinceTest4 >= SAMPLE_TIME) {
      sinceTest4 = sinceTest4 - SAMPLE_TIME ; 
      
     if (!playSdWav1.isPlaying()) {
    loop_state_waiting();
     } else {  
    loop_state_playing();
     }
  }
}
  // Only delay for the remaining time in the sample.
  
