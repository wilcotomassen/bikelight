#include <Adafruit_NeoPixel.h>

// Button pin (Digital 2)
#define BUTTON_PIN 2
#define BUTTON_PIN_INTERRUPT 0

// Led pin
#define LED_PIN 0
#define LED_COUNT 10

#define MODE_LIGHT          0
#define MODE_STROBE_BLUE    1
#define MODE_STROBE_ORANGE  2
#define MODE_STROBE_RED     3
#define MODE_KITT           4

Adafruit_NeoPixel leds = Adafruit_NeoPixel(LED_COUNT, LED_PIN, NEO_RGB + NEO_KHZ800);

// Color palette
const uint32_t C_OFF = leds.Color(0, 0, 0);
const uint32_t C_WHITE = leds.Color(255, 255, 255);
const uint32_t C_BLUE = leds.Color(0, 0, 255);
const uint32_t C_ORANGE = leds.Color(255, 70, 0);
const uint32_t C_RED = leds.Color(255, 0, 0);

// Mode variables
volatile boolean modeChangeTriggered;
uint8_t currentMode;

// Strobe variables
uint32_t primaryColor;
boolean strobeOn;
boolean shortStrobe;
uint8_t strobeCount;


void setup() {
  
  // Setup button pin and mode toggle interrupt
  pinMode(BUTTON_PIN, INPUT);
  attachInterrupt(BUTTON_PIN_INTERRUPT, modeChangeISR, FALLING);

  // Initialize leds
  leds.begin();
  leds.show();

  // Start in light mode
  enterMode(MODE_LIGHT);
  
}

void loop() {

  // Handle mode specifics (if any)
  switch(currentMode) {
    case MODE_STROBE_BLUE:
    case MODE_STROBE_ORANGE:
    case MODE_STROBE_RED:
      setLights(strobeOn ? primaryColor : C_OFF, strobeOn ? C_OFF : primaryColor);
      strobeOn = !strobeOn;
      delay(shortStrobe ? 150 : 500);
      strobeCount++;
      if (strobeCount >= 20) {
        shortStrobe = !shortStrobe;
        strobeCount = 0;
      }
      
      break;
    case MODE_KITT:
      for (int i = 0; i < LED_COUNT; i++) {
        for (int j = 0; j < LED_COUNT; j++) {
          leds.setPixelColor(j, i == j ?  primaryColor : C_OFF);
        }
        leds.show();
        delay(80);        
      }
      for (int i = LED_COUNT -2; i > 0; i--) {
        for (int j = 0; j < LED_COUNT; j++) {
          leds.setPixelColor(j, i == j ?  primaryColor : C_OFF);
        }
        leds.show();
        delay(80);        
      }
      break;
  }

  // Handle mode change
  if (modeChangeTriggered) {
    delay(300);
    gotoNextMode();
    modeChangeTriggered = false;    
  }

}

void setLights(uint32_t leftColor, uint32_t rightColor) {  
  for (uint8_t i = 0; i < LED_COUNT; i++) {
    leds.setPixelColor(i, i < (LED_COUNT / 2) ? leftColor : rightColor);
  }
  leds.show();
}

/**
 * Go to next mode (add one to currentMode), will
 * cycle through all modes if repeatedly called
 */
void gotoNextMode() {
  enterMode(currentMode + 1);
}

/**
 * Handle changing modes, this function will sanitize the mode
 * input to prevent entering invalid mode (default is MODE_LIGHT), so
 * continuously going one mode higher than current mode results in cycling 
 * through the modes
 */
void enterMode(uint8_t mode) { 

  // Prevent entering undefined mode
  if (mode < MODE_LIGHT || mode > MODE_KITT) {
    mode = MODE_LIGHT;
  }

  // Handle entering of mode
  currentMode = mode;
  shortStrobe = false;
  strobeCount = 0;
  switch(currentMode) {
    case MODE_LIGHT:
      setLights(C_WHITE, C_WHITE);
      break;
    case MODE_STROBE_BLUE:
      primaryColor = C_BLUE;
      break;
    case MODE_STROBE_ORANGE:
      primaryColor = C_ORANGE;
      break;    
    case MODE_STROBE_RED:
      primaryColor = C_RED;
      break; 
    case MODE_KITT:
      primaryColor = C_RED;
      break;       
  }
}

/**
 * ISR to handle mode button press, sets modeChangeTriggered flag 
 * to true
 */
void modeChangeISR() {
  modeChangeTriggered = true;
}

