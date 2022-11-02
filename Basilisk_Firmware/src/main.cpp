#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <Adafruit_MLX90640.h>
#include <FastLED.h>
#include "LUT.h"
#include <Wire.h>

Adafruit_MLX90640 mlx;
float frame[32*24]; // buffer for full frame of temperatures

#define PANEL_RES_X 64      // Number of pixels wide of each INDIVIDUAL panel module. 
#define PANEL_RES_Y 64     // Number of pixels tall of each INDIVIDUAL panel module.
#define PANEL_CHAIN 1      // Total number of panels chained one to another

#define PANE_WIDTH PANEL_RES_X * PANEL_CHAIN
#define PANE_HEIGHT PANEL_RES_Y

#define R1_PIN 25
#define G1_PIN 26
#define B1_PIN 27
#define R2_PIN 14
#define G2_PIN 12
#define B2_PIN 13
#define A_PIN 33
#define B_PIN 15
#define C_PIN 2
#define D_PIN 17
#define E_PIN 32
#define LAT_PIN 4
#define OE_PIN 23 //0
#define CLK_PIN 16

MatrixPanel_I2S_DMA *dma_display = nullptr;
HUB75_I2S_CFG::i2s_pins _pins={R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN, A_PIN, B_PIN, C_PIN, D_PIN, E_PIN, LAT_PIN, OE_PIN, CLK_PIN};

uint16_t time_counter = 0, cycles = 0, fps = 0;
unsigned long fps_timer;

CRGB currentColor;
//CRGBPalette16 palettes[] = {HeatColors_p, LavaColors_p, RainbowColors_p, RainbowStripeColors_p, CloudColors_p};
CRGBPalette16 currentPalette = HeatColors_p; //palettes[0];

// uncomment *one* of the below
//#define PRINT_TEMPERATURES
#define PRINT_ASCIIART

#define LED 2

const char ascii_LUT[] = " .:-=+*#%@";

void solid_colors() {
  Serial.println("Fill screen: RED");
  dma_display->fillScreenRGB888(255, 0, 0);
  delay(1000);

  Serial.println("Fill screen: GREEN");
  dma_display->fillScreenRGB888(0, 255, 0);
  delay(1000);

  Serial.println("Fill screen: BLUE");
  dma_display->fillScreenRGB888(0, 0, 255);
  delay(1000);

  Serial.println("Fill screen: Neutral White");
  dma_display->fillScreenRGB888(64, 64, 64);
  delay(1000);

  Serial.println("Fill screen: black");
  dma_display->fillScreenRGB888(0, 0, 0);
  delay(1000);
}

void plasma() {
  for (int x = 0; x < PANE_WIDTH; x++) {
    for (int y = 0; y <  PANE_HEIGHT; y++) {
      int16_t v = 0;
      uint8_t wibble = sin8(time_counter);
      v += sin16(x * wibble * 3 + time_counter);
      v += cos16(y * (128 - wibble)  + time_counter);
      v += sin16(y * x * cos8(-time_counter) / 8);

      currentColor = ColorFromPalette(currentPalette, (v >> 8) + 127); //, brightness, currentBlendType);
      dma_display->drawPixelRGB888(x, y, currentColor.r, currentColor.g, currentColor.b);
    }
  }

  ++time_counter;
  ++cycles;
  ++fps;

  /*if (cycles >= 1024) {
    time_counter = 0;
    cycles = 0;
    currentPalette = palettes[random(0,sizeof(palettes)/sizeof(palettes[0]))];
  }*/

  // print FPS rate every 5 seconds
  // Note: this is NOT a matrix refresh rate, it's the number of data frames being drawn to the DMA buffer per second
  if (fps_timer + 5000 < millis()){
    Serial.printf_P(PSTR("Effect fps: %d\n"), fps/5);
    fps_timer = millis();
    fps = 0;
  }
}

void MLX_Setup(TwoWire *w) {
  delay(100);

  Serial.println("Adafruit MLX90640 Simple Test");
  uint8_t address;
  bool found = false;
  for (address = 1; address < 127; address++ ) {
    if (! mlx.begin(address, w)) {
      Serial.print("MLX90640 not found! ");
      Serial.println(address, HEX);
    } else {
      Serial.print("Found Adafruit MLX90640 ");
      Serial.println(address, HEX);
      found = true;
      break;
    }
  }

  if (!found) {
    Serial.println("No MLX90640 found, check wiring?");
    while (1);
  }

  Serial.print("Serial number: ");
  Serial.print(mlx.serialNumber[0], HEX);
  Serial.print(mlx.serialNumber[1], HEX);
  Serial.println(mlx.serialNumber[2], HEX);
  
  //mlx.setMode(MLX90640_INTERLEAVED);
  mlx.setMode(MLX90640_CHESS);
  Serial.print("Current mode: ");
  if (mlx.getMode() == MLX90640_CHESS) {
    Serial.println("Chess");
  } else {
    Serial.println("Interleave");    
  }

  mlx.setResolution(MLX90640_ADC_16BIT);
  Serial.print("Current resolution: ");
  mlx90640_resolution_t res = mlx.getResolution();
  switch (res) {
    case MLX90640_ADC_16BIT: Serial.println("16 bit"); break;
    case MLX90640_ADC_17BIT: Serial.println("17 bit"); break;
    case MLX90640_ADC_18BIT: Serial.println("18 bit"); break;
    case MLX90640_ADC_19BIT: Serial.println("19 bit"); break;
  }

  mlx.setRefreshRate(MLX90640_32_HZ);
  Serial.print("Current frame rate: ");
  mlx90640_refreshrate_t rate = mlx.getRefreshRate();
  switch (rate) {
    case MLX90640_0_5_HZ: Serial.println("0.5 Hz"); break;
    case MLX90640_1_HZ: Serial.println("1 Hz"); break; 
    case MLX90640_2_HZ: Serial.println("2 Hz"); break;
    case MLX90640_4_HZ: Serial.println("4 Hz"); break;
    case MLX90640_8_HZ: Serial.println("8 Hz"); break;
    case MLX90640_16_HZ: Serial.println("16 Hz"); break;
    case MLX90640_32_HZ: Serial.println("32 Hz"); break;
    case MLX90640_64_HZ: Serial.println("64 Hz"); break;
  }

  if (mlx.getFrame(frame) != 0) {
    Serial.println("Failed");
    return;
  } else {
    Serial.println("Success");
  }

}

void print_IR_frame(TwoWire *w) {
  //w.begin(21, 22);
  if (mlx.getFrame(frame) != 0) {
    Serial.println("Failed");
    return;
  }
  
  Serial.println();
  Serial.println();
  for (uint8_t h=0; h<24; h++) {
    for (uint8_t w=0; w<32; w++) {
      float t = frame[h*32 + w];
#ifdef PRINT_TEMPERATURES
      Serial.print(t, 1);
      Serial.print(", ");
#endif
#ifdef PRINT_ASCIIART
      uint8_t idx = (uint8_t)map(t, 20, 40, 0, 10);
      Serial.print(ascii_LUT[idx]);
      //if(t>21) Serial.print("X");
      //else Serial.print(" ");
#endif
    }
    Serial.println();
  }
  //return 0;
}

uint8_t byte_frame[32*24];
//uint8_t LUT_24_64 = []

void show_IR_on_LEDS(){
  if (mlx.getFrame(frame) != 0) {
    Serial.println("Failed");
  } else {
    Serial.println("Success");
    //int colorTemp;
    for(int i=0; i<32*24; i++){
      byte_frame[i] = (uint8_t)map(frame[i], MINTEMP, MAXTEMP, 0, 255);
    }

    /*for (uint8_t h=0; h<24; h++) {
      for (uint8_t w=0; w<32; w++) {
        float t = frame[h*32 + w];
        // Serial.print(t, 1); Serial.print(", ");

        //t = min(t, MAXTEMP);
        //t = max(t, MINTEMP); 
            
        uint8_t colorIndex = map(t, MINTEMP, MAXTEMP, 0, 255);
        
        //colorIndex = constrain(colorIndex, 0, 255);

        currentColor = ColorFromPalette(currentPalette, colorIndex);

        dma_display->drawPixelRGB888(w, h, currentColor.r, currentColor.g, currentColor.b);
        //draw the pixels!           
      }
    }*/

    for(uint8_t i=0; i<64; i++){
      for(uint8_t j=0; j<64; j++){
        currentColor = ColorFromPalette(currentPalette, byte_frame[(int)(i/2 * 32 + j * 24/64)]);
        dma_display->drawPixelRGB888(i, j, currentColor.r, currentColor.g, currentColor.b);
      }
    }
    
  }
}

TwoWire w = TwoWire(0);

void setup() {

    // Module configuration
    HUB75_I2S_CFG mxconfig(
      PANEL_RES_X,   // module width
      PANEL_RES_Y,   // module height
      PANEL_CHAIN,    // Chain length
      _pins
    );

    mxconfig.driver = HUB75_I2S_CFG::SM5266P;
    //mxconfig.clkphase = 0;

    // Display Setup
    dma_display = new MatrixPanel_I2S_DMA(mxconfig);
    dma_display->begin();
    dma_display->setBrightness8(50); //0-255
    //dma_display->fillScreen(0xFF);
    dma_display->setLatBlanking(2);
    dma_display->clearScreen();

    //pinMode(LED, OUTPUT);

    //while (!Serial) delay(10);
    Serial.begin(115200);

    w = TwoWire(0);
    w.begin(21, 22, 800000);

    MLX_Setup(&w);

    //solid_colors();

    // Set current FastLED palette
    currentPalette = RainbowColors_p;
    Serial.println("Starting plasma effect...");
    fps_timer = millis();
}

void loop() {
  //digitalWrite(LED, HIGH); 
  //delay(10);             
  //digitalWrite(LED, LOW);  
  //delay(10);          
  //print_IR_frame(&w);
  //plasma();
  //solid_colors();
  show_IR_on_LEDS();
}