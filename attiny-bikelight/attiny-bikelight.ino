#include <Adafruit_NeoPixel.h>
#include <avr/sleep.h>

// Button pin (Digital 2)
#define BUTTON_PIN 2
#define BUTTON_PIN_INTERRUPT 0

// Led pin
#define LED_PIN 0
#define LED_COUNT 16

#define MAX_RUNTIME_MS 60 * 1000 // 1 min
//60*60*1000 // 1 hour

#define MODE_LIGHT          0
#define MODE_STROBE_BLUE    1
#define MODE_STROBE_ORANGE  2
#define MODE_OFF            3

Adafruit_NeoPixel leds = Adafruit_NeoPixel(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// Color palette
const uint32_t C_OFF = leds.Color(0, 0, 0);
const uint32_t C_WHITE = leds.Color(255, 255, 255);
const uint32_t C_BLUE = leds.Color(0, 0, 255);
const uint32_t C_ORANGE = leds.Color(255, 70, 0);

// Mode variables
volatile boolean modeChangeTriggered = false;
uint8_t currentMode;

// Strobe variables
uint32_t currentStrobeColor;
boolean strobeToggle;

// Sleep variables
uint64_t sleepTime;

void setup() {
  
  // Setup button pin and mode toggle interrupt
  pinMode(BUTTON_PIN, INPUT);
  attachInterrupt(BUTTON_PIN_INTERRUPT, modeChangeISR, FALLING);

  // Initialize leds
  leds.begin();
  leds.show();

  // Start in light mode
  currentMode = MODE_LIGHT;
  enterMode(currentMode);

  // Reset sleep timer
  resetSleepTimer();

}

void loop() {

  // Mode specifics (if any)
  switch(currentMode) {
    case MODE_STROBE_BLUE:
    case MODE_STROBE_ORANGE:
      setLights(strobeToggle ? currentStrobeColor : C_OFF, strobeToggle ? C_OFF : currentStrobeColor);
      strobeToggle = !strobeToggle;
      delay(500);
      break;
  }

  if (modeChangeTriggered) {
    delay(300);
    gotoNextMode();
    modeChangeTriggered = false;    
  }

  // Handle running time timeout
  if (millis() >= sleepTime) {
    currentMode = MODE_OFF;
    enterMode(currentMode);
  }

}

void setLights(uint32_t leftColor, uint32_t rightColor) {  
  for (uint8_t i = 0; i < LED_COUNT; i++) {
    leds.setPixelColor(i, i < (LED_COUNT / 2) ? leftColor : rightColor);
  }
  leds.show();
}

void gotoNextMode() {
  currentMode++;

  // Loop around after off
  if (currentMode > MODE_OFF) {
    currentMode = MODE_LIGHT;
  }

  // Handle entering of mode
  enterMode(currentMode);

  // Reset sleep timer
  resetSleepTimer();
  
}

void enterMode(uint8_t mode) {
   switch(mode) {
    case MODE_LIGHT:
      setLights(C_WHITE, C_WHITE);
      break;
    case MODE_STROBE_BLUE:
      currentStrobeColor = C_BLUE;
      break;
    case MODE_STROBE_ORANGE:
      currentStrobeColor = C_ORANGE;
      break;
    case MODE_OFF:
      setLights(C_OFF, C_OFF);
      enterSleepMode();
      break;
  }
}

void modeChangeISR() {
  modeChangeTriggered = true;
}

void enterSleepMode() {

  // Detach mode button interrupt
  detachInterrupt(BUTTON_PIN_INTERRUPT);

  // Entering sleep mode: enable sleep and wake-up interrupt
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  attachInterrupt(BUTTON_PIN_INTERRUPT, wakeUpInterrupt, LOW);
  sleep_mode();
  
  // >> Here we are in sleep mode

  // Exiting sleepmode: disable sleep and wake-up interrupt
  sleep_disable();
  detachInterrupt(BUTTON_PIN_INTERRUPT);
  
  // Set the mode to the first mode
  modeChangeTriggered = true;

  // Attach mode button interrupt
  attachInterrupt(BUTTON_PIN_INTERRUPT, modeChangeISR, FALLING);

  // Reset sleep timer
  resetSleepTimer();
  
}

void wakeUpInterrupt() {
  // No real need to do something here  
}

void resetSleepTimer() {
  sleepTime = millis() + MAX_RUNTIME_MS;
}

