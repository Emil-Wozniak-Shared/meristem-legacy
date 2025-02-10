#include "DFRobot_GDL.h"
#include <Arduino.h>
#include <stdint.h>
#include <HX711.h> 

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
#define ADC_PIN A0
#define BTN_PIN 21
#define LOADCELL_DOUT_PIN 10
#define LOADCELL_SCK_PIN 3

DFRobot_ST7735_128x160_HW_SPI screen(/*dc=*/TFT_DC,/*cs=*/TFT_CS,/*rst=*/TFT_RST);
// https://www.circuitschools.com/weighing-scale-using-load-cell-and-hx711-amplifier-with-arduino/
HX711 loadcell;

const int GRAM_MULTIPLIER = 1000;
int ADC_VALUE = 0;
double voltage_value = 0.0; 
float calibration_factor = 137326;
double lastReading = 0.0;
float slope = 2.48;// 92.07; // slope from linear fit
float intercept = 0.72;//-29.92; // intercept from linear fit
int buttonState = 0;

void setup() {
  // Serial1.begin(9600, SERIAL_8N1,/*rx =*/0,/*Tx =*/1);  should work but won't
  Serial.begin(9600, SERIAL_8N1); // this give my ain't shit
  screen.begin();
  pinMode(ADC_PIN, INPUT); // set the analog reference to 3.3V
  pinMode(BTN_PIN, INPUT);
  splash();
  // LOAD CELL
  screen.setCursor(4, 4);
  screen.println("Initializing LoadCell...");
  loadcell.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  loadcell.set_scale();
  loadcell.tare();	//Reset the scale to 0
  long zero_factor = loadcell.read_average(); //Get a baseline reading
  screen.fillScreen(COLOR_RGB565_BLACK);
  screen.setCursor(4, 12);
  screen.print("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
  screen.println(zero_factor);
  screen.setCursor(4, 20);
  screen.println("LoadCell ready");
  delay(1000);
  // exit and setup screen for loop
  screen.fillScreen(COLOR_RGB565_BLACK);
  screen.setTextColor(COLOR_RGB565_WHITE); // slow operation
}

void loop() {
  setupScreen();
  displayADC();
  bool isNew = displayWeight();
  delay(isNew ? 500 : 2000);
  // ESP_LOGI("LOOP", "calibration scheme version is %s", "Curve Fitting");
}

void setupScreen() {
  screen.setTextSize(2);
  screen.setCursor(4, 4);
  screen.print("Readings");
  screen.setTextSize(1);
  screen.drawRect(/*x=*/2, /*y=*/2 , /*w=*/screen.width() - 8, /*h=*/46 + 8 + 8, /*color=*/COLOR_RGB565_DGREEN);
  screen.fillRect(/*x=*/4, /*y=*/24 , /*w=*/screen.width() - (8 + 4), /*h=*/46 -6, /*color=*/COLOR_RGB565_BLACK);
  delay(100);
}

void displayADC() {
  sensor_reading();
  screen.setCursor(4, 24);
  screen.print("ADC value: ");
  screen.println(ADC_VALUE);
  screen.setCursor(4, 36);
  screen.print("ADC voltage: ");
  screen.println(voltage_value);
}

bool displayWeight() {
  loadcell.set_scale(calibration_factor); //Adjust to this calibration factor
  float reading = loadcell.get_units(10) * GRAM_MULTIPLIER; 
  screen.setCursor(4, 46);
  screen.print("Weight: ");
  screen.print(reading, 6);
  screen.println(" g");
  screen.setCursor(4, 54);
  screen.print("Water: ");
  float vol_water_cont = ((1.0 / voltage_value) * slope) + intercept; // calc of theta_v (vol. water content)
  screen.print(vol_water_cont);
  screen.println(" cm^3/cm^3"); // cm^3/cm^3
  bool isNew = (reading - lastReading) > 0.005;
  if (isNew) {
      lastReading = reading;
  }
  return isNew;
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
  for(i = 0 ; i <= 10; i+=2 ) {
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
  ADC_VALUE = analogRead(ADC_PIN);
  voltage_value = (ADC_VALUE * 3.3 ) / (4095);
  return voltage_value;
}
