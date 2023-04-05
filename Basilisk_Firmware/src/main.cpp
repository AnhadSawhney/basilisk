#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <Adafruit_MLX90640.h>
#include <FastLED.h>
#include "images.h"
#include <Wire.h>
#include "IR.h"

#define PANEL_RES_X 64      // Number of pixels wide of each INDIVIDUAL panel module. 
#define PANEL_RES_Y 64     // Number of pixels tall of each INDIVIDUAL panel module.
#define PANEL_CHAIN 1      // Total number of panels chained one to another

#define PANE_WIDTH PANEL_RES_X * PANEL_CHAIN
#define PANE_HEIGHT PANEL_RES_Y

/* ================= PINOUT ================= */

#define STATUS_LED 19

#define R1_PIN 25
#define G1_PIN 26
#define B1_PIN 27
#define R2_PIN 14
#define G2_PIN 12
#define B2_PIN 13
#define A_PIN 33
#define B_PIN 5
#define C_PIN 2
#define D_PIN 17
#define E_PIN 32
#define LAT_PIN 4
#define OE_PIN 15 
#define CLK_PIN 16

/* ========================================== */

#define LOOK_LOWPASS 0.8f // 80% the old value, 20% the new one

MatrixPanel_I2S_DMA *dma_display = nullptr;
HUB75_I2S_CFG::i2s_pins _pins={R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN, A_PIN, B_PIN, C_PIN, D_PIN, E_PIN, LAT_PIN, OE_PIN, CLK_PIN};

uint16_t time_counter = 0, cycles = 0, fps = 0;
unsigned long fps_timer;

TwoWire w = TwoWire(0);

int lookx = 0, looky = 0;

void draw_eye(int lookx, int looky) {
  // start with green
  //dma_display->fillScreenRGB888(0, 255, 0); // don't refresh the screen it causes flashes because a display refresh happens before the eye is fully finished drawing

  for(int x = 0; x < 64; x++) {
    for(int y = 0; y < 64; y++) {
      if( ISMASKED(x,y) ) { // this is masked out in the skin image, its part of the eyeball
        // TODO: draw the textured eyeball here with the remapping function
        //continue;
        dma_display->drawPixelRGB888(x, y, 0, 255, 0); 
      } else {
        dma_display->drawPixelRGB888(x, y, skin[x+y*64][0], skin[x+y*64][1], skin[x+y*64][2]);
      }
    }
  }
}

void draw_pupil(int lookx, int looky) {
  int pupil_height = 49;
  for(int i = 0; i < pupil_height; i++) {
    int y = (int)(looky - pupil_height/2 + i);
    if(y < 0 || y > 63) {
      continue;
    }
    float offset = pupil_data[i][0];
    float alpha = (32.0f-lookx)/2.0f; // how strongly to warp
    offset += alpha*(1.0f-y/32.0f)*(float)(1.0f-y/32.0f);

    int width = pupil_data[i][1];
    for(int j = 0; j < width; j++) {
      int x = int(lookx + j + offset);
      if(x < 0 or x > 63) {
        continue;
      }
      if( ISMASKED(x,y) ) {
        dma_display->drawPixelRGB888(x,y,0,0,0);
      }
    }
  }

  //dma_display->drawPixelRGB888(lookx, looky, 255, 0, 0); // just for debug, show where the eye should be looking
}

void setup() {
    pinMode(STATUS_LED, OUTPUT);

    // Module configuration
    HUB75_I2S_CFG mxconfig(
      PANEL_RES_X,   // module width
      PANEL_RES_Y,   // module height
      PANEL_CHAIN,    // Chain length
      _pins
    );

    //mxconfig.driver = HUB75_I2S_CFG::SM5266P;
    mxconfig.driver = HUB75_I2S_CFG::FM6124;
    //mxconfig.clkphase = 0;

    // Display Setup
    dma_display = new MatrixPanel_I2S_DMA(mxconfig);
    dma_display->begin();
    dma_display->setBrightness8(75); //0-255
    //dma_display->fillScreen(0xFF);
    dma_display->setLatBlanking(2);
    dma_display->clearScreen();

    //while (!Serial) delay(10);
    Serial.begin(115200);

    w = TwoWire(0);
    w.begin(21, 22, 800000);
    w.setClock(1000000);

    MLX_Setup(&w);
    fps_timer = millis();
}

bool status = false;

void loop() {
  int newx, newy;
  get_ir(newx, newy);

  lookx = (int)(lookx * LOOK_LOWPASS + newx * (1.0f - LOOK_LOWPASS));
  looky = (int)(looky * LOOK_LOWPASS + newy * (1.0f - LOOK_LOWPASS));

  draw_eye(lookx, looky);
  draw_pupil(lookx, looky);

  if(status) {
    digitalWrite(STATUS_LED, 0);
    delayMicroseconds(10);
    digitalWrite(STATUS_LED, 1);
  } else {
    digitalWrite(STATUS_LED, 1);
  }
  //delay(33);
  status = !status;
}