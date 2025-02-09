#include "DFRobot_GDL.h"
#include <Arduino.h>
#include <stdint.h>
#include <HX711.h> // https://github.com/bogde/HX711

#define DEFAULT_VREF    1100

#define TFT_DC  1 // Data/command
#define TFT_CS  7 // TFT Chip Select
#define TFT_RST 2 // Reset
#define TFT_BLK 10 // Backlight
#define TFT_SCK 4  // SPI clock
#define TFT_MOSI 6
#define TFT_MISO 5
#define TFT_SDCS 0  // SD card chip select
#define TFT_TCS 3
#define TFT_SCL 22 // I2C clock
#define TFT_SDA 21 // I2C data
#define adcPin A0
#define LOADCELL_DOUT_PIN 10
#define LOADCELL_SCK_PIN 3

DFRobot_ST7735_128x160_HW_SPI screen(/*dc=*/TFT_DC,/*cs=*/TFT_CS,/*rst=*/TFT_RST);
// https://www.circuitschools.com/weighing-scale-using-load-cell-and-hx711-amplifier-with-arduino/
HX711 loadcell;

int ADC_VALUE = 0;
double voltage_value = 0.0; 
float calibration_factor = 137326;
double lastReading = 0.0;
float slope = 2.48;// 92.07; // slope from linear fit
float intercept = 0.72;//-29.92; // intercept from linear fit

void setup() {
  // Serial1.begin(9600, SERIAL_8N1,/*rx =*/0,/*Tx =*/1);  should work but won't
  Serial.begin(9600, SERIAL_8N1); // this give my ain't shit
  screen.begin();
  pinMode(adcPin, INPUT); // set the analog reference to 3.3V
  splash();
  // LOAD CELL
  screen.setCursor(4, 4);
  screen.println("Initializing LoadCell...");
  loadcell.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  loadcell.set_scale();
  loadcell.tare();	//Reset the scale to 0
  long zero_factor = loadcell.read_average(); //Get a baseline reading
  screen.setCursor(4, 12);
  screen.print("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
  screen.println(zero_factor);
  screen.fillScreen(COLOR_RGB565_GREEN);
  screen.setCursor(4, 20);
  screen.println("LoadCell ready");

  screen.fillScreen(COLOR_RGB565_BLACK);
}

void loop() {
  screen.setTextSize(2);
  screen.setCursor(4, 4);
  screen.print("Readings");
  loadcell.set_scale(calibration_factor); //Adjust to this calibration factor
  screen.drawRect(/*x=*/2, /*y=*/2 , /*w=*/screen.width() - 8, /*h=*/46 + 8 + 8, /*color=*/COLOR_RGB565_DGREEN);
  screen.fillRect(/*x=*/4, /*y=*/24 , /*w=*/screen.width() - (8 + 4), /*h=*/46 -6, /*color=*/COLOR_RGB565_BLACK);
  delay(100);

  screen.setTextColor(COLOR_RGB565_WHITE);
  screen.setTextSize(1);

  sensor_reading();
  screen.setCursor(4, 24);
  screen.print("ADC value: ");
  screen.println(ADC_VALUE);
  screen.setCursor(4, 36);
  screen.print("ADC voltage: ");
  screen.println(voltage_value);
  
  screen.setCursor(4, 46);
  float reading = loadcell.get_units();
  screen.print("Weight: ");
  screen.print(reading, 4);
  screen.println(" Kgs");
  screen.setCursor(4, 54);
  screen.print("Water: ");
  float vol_water_cont = ((1.0 / voltage_value) * slope) + intercept; // calc of theta_v (vol. water content)
  screen.print(vol_water_cont);
  screen.println(" cm^3/cm^3"); // cm^3/cm^3
  // ESP_LOGI("LOOP", "calibration scheme version is %s", "Curve Fitting");
 
  if ((reading - lastReading) > 0.005) {
    lastReading = reading;
    delay(500);
  }
  else {
    delay(2000);
  }


}

void splash() {
  screen.fillScreen(COLOR_RGB565_BLACK);
  int color = 0xF00F;
  int i;
  int x = 0;
  int y = 0;
  int w = screen.width()-3;
  int h = screen.height()-3;
  for(i = 0 ; i <= 10; i+=2) {
    screen.drawRoundRect(/*x0=*/x, /*y0=*/y, /*w=*/w, /*h=*/h, /*radius=*/20, /*color=*/color);
    x+=5;
    y+=5;
    w-=10;
    h-=10;
    color+=0x0100;
    delay(50);
  }
  for(i = 0 ; i <= 10; i+=2) {
    screen.fillRoundRect(/*x0=*/x, /*y0=*/y, /*w=*/w, /*h=*/h, /*radius=*/10, /*color=*/color);
    x+=5;
    y+=5;
    w-=10;
    h-=10;
    color+=0x0500;
    delay(50);
  }
}

float sensor_reading() {
  ADC_VALUE = analogRead(adcPin);
  voltage_value = (ADC_VALUE * 3.3 ) / (4095);
  return voltage_value;
}
