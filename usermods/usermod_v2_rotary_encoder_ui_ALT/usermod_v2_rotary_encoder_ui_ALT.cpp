
// Simplified Rotary Encoder UI usermod for WLED (Dig2Go GPIOs: 21, 22, 24)
#include "wled.h"

#ifndef ENCODER_DT_PIN
#define ENCODER_DT_PIN 21
#endif

#ifndef ENCODER_CLK_PIN
#define ENCODER_CLK_PIN 22
#endif

#ifndef ENCODER_SW_PIN
#define ENCODER_SW_PIN 24
#endif

#define ENCODER_MAX_DELAY_MS 8

class RotaryEncoderUIUsermod : public Usermod {
  private:
    int8_t pinA = ENCODER_DT_PIN;
    int8_t pinB = ENCODER_CLK_PIN;
    int8_t pinC = ENCODER_SW_PIN;

    unsigned long loopTime = 0;
    unsigned long buttonPressedTime = 0;
    unsigned long buttonWaitTime = 0;
    bool buttonPressedBefore = false;
    bool buttonLongPressed = false;

    uint8_t select_state = 0; // 0=Brightness, 1=Effect, 2=Speed, 3=Intensity, 4=Palette

    bool Enc_A = false, Enc_B = false, Enc_A_prev = false;
    bool initDone = false;

  public:
    void setup() override {
      pinMode(pinA, INPUT_PULLUP);
      pinMode(pinB, INPUT_PULLUP);
      pinMode(pinC, INPUT_PULLUP);
      Enc_A = digitalRead(pinA);
      Enc_B = digitalRead(pinB);
      Enc_A_prev = Enc_A;
      initDone = true;
    }

    void loop() override {
      if (!initDone) return;
      unsigned long currentTime = millis();
      if (strip.isUpdating() || (currentTime - loopTime) < ENCODER_MAX_DELAY_MS) return;

      // Button handling
      bool buttonPressed = !digitalRead(pinC);
      if (buttonPressed) {
        if (!buttonPressedBefore) buttonPressedTime = currentTime;
        buttonPressedBefore = true;
      } else if (buttonPressedBefore) {
        buttonPressedBefore = false;
        if (currentTime - buttonPressedTime > 50) {
          select_state = (select_state + 1) % 5;
        }
      }

      // Rotary handling
      Enc_A = digitalRead(pinA);
      Enc_B = digitalRead(pinB);
      if ((Enc_A) && (!Enc_A_prev)) {
        bool clockwise = (Enc_B == LOW);
        switch (select_state) {
          case 0: changeBrightness(clockwise); break;
          case 1: changeEffect(clockwise); break;
          case 2: changeEffectSpeed(clockwise); break;
          case 3: changeEffectIntensity(clockwise); break;
          case 4: changePalette(clockwise); break;
        }
      }
      Enc_A_prev = Enc_A;
      loopTime = currentTime;
    }

    void changeBrightness(bool up) {
      bri = constrain(bri + (up ? 5 : -5), 0, 255);
      stateUpdated(CALL_MODE_BUTTON);
    }

    void changeEffect(bool up) {
      effectCurrent = (effectCurrent + (up ? 1 : strip.getModeCount() - 1)) % strip.getModeCount();
      strip.setMode(effectCurrent);
      stateUpdated(CALL_MODE_BUTTON);
    }

    void changeEffectSpeed(bool up) {
      effectSpeed = constrain(effectSpeed + (up ? 5 : -5), 0, 255);
      stateUpdated(CALL_MODE_BUTTON);
    }

    void changeEffectIntensity(bool up) {
      effectIntensity = constrain(effectIntensity + (up ? 5 : -5), 0, 255);
      stateUpdated(CALL_MODE_BUTTON);
    }

    void changePalette(bool up) {
      effectPalette = (effectPalette + (up ? 1 : strip.getPaletteCount() - 1)) % strip.getPaletteCount();
      strip.setPalette(effectPalette);
      stateUpdated(CALL_MODE_BUTTON);
    }

    void addToConfig(JsonObject &root) override {}
    bool readFromConfig(JsonObject &root) override { return true; }

    uint16_t getId() override {
      return USERMOD_ID_ROTARY_ENC_UI;
    }
};

static RotaryEncoderUIUsermod usermod_rotary_enc_dig2go;
REGISTER_USERMOD(usermod_rotary_enc_dig2go);
