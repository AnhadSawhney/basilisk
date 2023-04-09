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

  mlx.setRefreshRate(MLX90640_32_HZ); // 64 hz doesnt work
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

#define BLOB_MAX_SIZE 50

// allocate 50 points for blob sampling
uint8_t blob_coords_x[BLOB_MAX_SIZE];
uint8_t blob_coords_y[BLOB_MAX_SIZE];

float get_ir(int &lookx, int &looky) {
    if (mlx.getFrame(frame) != 0) {
        Serial.println("Get Frame Fail");
        return 0; // lookx and looky unchanged
    }
    float max = 0, min = frame[0], avg = 0, t;

    for(int i=0; i<32*24; i++) { // grab the max and min
        t = frame[i];
        if (t > max) {
            max = t;
        }
        if (t < min) {
            min = t;
        }
        avg += t;
    }

    avg /= (32.0f*24.0f);

    //Serial.print("Average: "); Serial.print(avg);
    //Serial.println(" Min: "); Serial.print(min);
    //Serial.println(" Max: "); Serial.print(max);

    if(avg - min < 3) { // couldn't find anything hot that stands out
        Serial.println("Nothing to look at");
        lookx = 32;
        looky = 32;
        return 100;
    }

    float thresh = max - (max-min) * 0.2f; // cutoff the top 20% of the range
    Serial.print(" Thresh: "); Serial.print(thresh);

    int blob_idx = 0;

    for (uint8_t y=0; y<24; y++) { 
        for (uint8_t x=0; x<32; x++) {
            float t = frame[y*32 + x];
            if (t > thresh) {
                blob_coords_x[blob_idx] = x;
                blob_coords_y[blob_idx] = y;
                blob_idx++;
            }
            if(blob_idx >= BLOB_MAX_SIZE) break; // don't overflow the blob array
        }
    }

    Serial.print(" Blob size: "); Serial.print(blob_idx);

    uint64_t newx = 0, newy = 0;

    // average the blob
    for (int i=0; i<blob_idx; i++) {
        newx += blob_coords_x[i];
        newy += blob_coords_y[i];
    }

    newx = (int)((float)(newx) / (float)(blob_idx) * 2.0f);
    newy = (int)((float)(newy) / (float)(blob_idx) * 64.0f/24.0f);

    int magx = newx - 32;
    int magy = newy - 32;
    uint64_t mag = magx * magx + magy * magy;

    if(mag > 28*28) { // if the blob is too far away, don't look at it
        Serial.print("Too far: ");
        lookx = (float)(newx * mag) / 28.0f;
        looky = (float)(newy * mag) / 28.0f;
    } else {
        lookx = newx;
        looky = newy;
    }

    Serial.print(" x: "); Serial.print(newx);
    Serial.print(" y: "); Serial.println(newy);    
    return thresh;
}