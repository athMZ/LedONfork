#ifndef NEW_EFFECTS_H
#define NEW_EFFECTS_H


void breathingEffect() {
  uint8_t bright = cubicwave8(beat8(10)); 
  
  fill_solid(leds, configured_leds, CHSV(h, s, bright));
}


void matrixRainEffect() {

  fadeToBlackBy(leds, configured_leds, 40);
  
  if (random8() < 40) {
    leds[random16(configured_leds)] = CRGB(0, 255, 0); // Green
  }
}


void policeStrobe() {
  EVERY_N_MILLISECONDS(100) {
    static int state = 0;
    state++;
    if (state > 3) state = 0;
    
    FastLED.clear();
    
    if (state == 0) {
       // Left Red
       fill_solid(leds, configured_leds/2, CRGB::Red);
    } 
    else if (state == 2) {
       // Right Blue
       fill_solid(leds + configured_leds/2, configured_leds/2, CRGB::Blue);
    }
  }
}

void colorWipeEffect() {
  static int pos = 0;
  static int colorIndex = 0;
  
  EVERY_N_MILLISECONDS(30) {
    leds[pos] = CHSV(h + (colorIndex * 30), s, 255);
    
    pos++;
    if (pos >= configured_leds) {
      pos = 0;
      colorIndex++;
    }
  }
}

#endif
