#include <Adafruit_MLX90640.h>

#define MINTEMP 18.0f
#define MAXTEMP 33.0f

Adafruit_MLX90640 mlx;
float frame[32*24]; // buffer for full frame of temperatures

const char ascii_LUT[] = " .:-=+*#%@";

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

void print_IR_frame() {
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

// allocate 100 points for blob sampling
uint8_t blob_coords_x[100];
uint8_t blob_coords_y[100];

void get_ir(int &lookx, int &looky) {
    if (mlx.getFrame(frame) != 0) {
        Serial.println("Get Frame Fail");
        return; // lookx and looky unchanged
    }
    float max = 0, min = frame[0];

    for(int t=0; t<32*24; t++) { // grab the max and min
        if (frame[t] > max) {
            max = frame[t];
        }
        if (frame[t] < min) {
            min = frame[t];
        }
    }

    if(max-min < 3) { // couldn't find anything hot that stands out
        Serial.println("Nothing to look at");
        lookx = 32;
        looky = 32;
        return;
    }

    float thresh = max - (max-min) * 0.1f; // cutoff the top 20% of the range

    int blob_idx = 0;

    for (uint8_t y=0; y<24; y++) { 
        for (uint8_t x=0; x<32; x++) {
            float t = frame[y*32 + x];
            if (t > thresh) {
                blob_coords_x[blob_idx] = x;
                blob_coords_y[blob_idx] = y;
                blob_idx++;
            }
        }
    }

    uint64_t newx = 0, newy = 0;

    // average the blob
    for (int i=0; i<blob_idx; i++) {
        newx += blob_coords_x[i];
        newy += blob_coords_y[i];
    }

    newx = newx / blob_idx;
    newy = newy / blob_idx;

    
    lookx = int(newx * 2); // map 0-32 to 0-64
    looky = (int)(newy*64.0f/24.0f); // map 0-24 to 0-64

    int magx = lookx - 32;
    int magy = looky - 32;

    if(magx * magx + magy * magx > 25*25) { // if the blob is too far away, don't look at it
        Serial.print("Too far: ");
        return;
    }

    Serial.print("x: "); Serial.print(lookx);
    Serial.print(" y: "); Serial.println(looky);    
}

/*
void show_IR_on_LEDS(){
  if (mlx.getFrame(frame) != 0) {
    Serial.println("Failed");
  } else {
    Serial.println("Success");
    //int colorTemp;
    uint8_t byte_frame[32*24];

    for(int i=0; i<32*24; i++){
      byte_frame[i] = (uint8_t)map(frame[i], MINTEMP, MAXTEMP, 0, 255);
    }

    dma_display->fillScreenRGB888(0, 0, 0);
    for(uint8_t i=0; i<64; i++){
      for(uint8_t j=0; j<48; j++){
        //currentColor = ColorFromPalette(currentPalette, byte_frame[(int)(i/2 * 32 + j * 24/64)]);
        //dma_display->drawPixelRGB888(i, j, currentColor.r, currentColor.g, currentColor.b);
        int index = (int)(j/2 * 32 + i/2);
        uint8_t t = byte_frame[index];

        // interpolate
        if(i%2 == 1) {
          if(i < 63) {
            t = (t + byte_frame[index+1])/2;
          }
        }
        if(j%2 == 1) {
          if(j < 47) {
            t = (t + byte_frame[index+32])/2;
          }
        }

        if(t < 128) {
          t = t*2;
          dma_display->drawPixelRGB888(i, j, 0, t, 255-t);
        } else {
          t = (t-128)*2;
          dma_display->drawPixelRGB888(i, j, t, 255-t, 0);
        }
      }
    }
    
  }
}

*/