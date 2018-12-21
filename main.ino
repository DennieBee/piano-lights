#include <MIDI.h>
#include <FastLED.h>

#define LED 13
#define BUTTON_PIN 2

FASTLED_USING_NAMESPACE

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    12
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    88
CRGB leds[NUM_LEDS];

#define BRIGHTNESS         160
#define FRAMES_PER_SECOND  120

struct ledInfo
{
   byte hue;
   byte saturation;
   byte value;
   bool on;
   int timeAlive;
};

typedef struct ledInfo LedInfo;

LedInfo ledInfos[NUM_LEDS];
int buttonState = 0;
int colorMode = 0;
byte randomHue = 0;

MIDI_CREATE_DEFAULT_INSTANCE();

void setup() {
  randomSeed(42);
  
  pinMode(LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  digitalWrite(LED, LOW);

  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.setHandleNoteOn(MyHandleNoteOn);
  MIDI.setHandleNoteOff(MyHandleNoteOff);

  delay(3000); // 3 second delay for recovery
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

  for(int x = 0; x < NUM_LEDS; x++) {
    LedInfo info;
    info.hue = 0;
    info.saturation = 0;
    info.value = 0;
    info.timeAlive = 0;
    info.on = false;
    ledInfos[x] = info;
  }
}

void loop() {
  MIDI.read(); 

  
  if (buttonState == LOW) {
    buttonState = digitalRead(BUTTON_PIN);
    if (buttonState == HIGH) {
      if (colorMode == 5) {
        colorMode = 0;
      } else {
        colorMode++;
      }
    }
  } else {
    buttonState = digitalRead(BUTTON_PIN);
  }

  for(int x = 0; x < NUM_LEDS; x++) {
    if (ledInfos[x].timeAlive > 500) {
      ledInfos[x].on = false;
    }
    if (ledInfos[x].on) {
      ledInfos[x].timeAlive = ledInfos[x].timeAlive + 1;
      leds[x] = CHSV(ledInfos[x].hue, ledInfos[x].saturation, ledInfos[x].value);
    } else {
      if (ledInfos[x].value > 0) {
        ledInfos[x].value = ledInfos[x].value - 1;
      }
      leds[x] = CHSV(ledInfos[x].hue, ledInfos[x].saturation, ledInfos[x].value);
    }
  }
  FastLED.show();
}

void MyHandleNoteOn(byte channel, byte pitch, byte velocity) {
  digitalWrite(LED, HIGH);
  int p = pitch - 21;

  int numberOfLedsOn = 0;
  for (int x = 0; x < NUM_LEDS; x++) {
    if (ledInfos[x].on) numberOfLedsOn++;
  }
  
  if (p >= 0 && p <= NUM_LEDS && numberOfLedsOn <= 20) {
    ledInfos[p].on = true;
    ledInfos[p].value = 255;
    ledInfos[p].timeAlive = 0;
    if (colorMode == 0) {
      ledInfos[p].hue = 0;
      ledInfos[p].value = 180;
      ledInfos[p].saturation = 0;
    }
    if (colorMode == 1) {
      ledInfos[p].hue = 0;
      ledInfos[p].saturation = 255;
    }
    if (colorMode == 2) {
      ledInfos[p].hue = 80;
      ledInfos[p].saturation = 220;
    }
    if (colorMode == 3) {
      ledInfos[p].hue = 174;
      ledInfos[p].saturation = 255;
    }
    if (colorMode == 4) {
      ledInfos[p].hue = 210;
      ledInfos[p].saturation = 255;
    }
    if (colorMode == 5) {
      randomHue = (byte)(random(0, 255));
      ledInfos[p].hue = randomHue;
      ledInfos[p].saturation = 255;
    }
  }
}

void MyHandleNoteOff(byte channel, byte pitch, byte velocity) {
  digitalWrite(LED, LOW);
  int p = pitch - 21;
  if (p >= 0 && p <= NUM_LEDS) {
    ledInfos[p].on = false;
    ledInfos[p].timeAlive = 0;
  }
}

