// KC1FSZ Bruce MacKinnon
// VSWR/Return Loss Meter
// 4-March-2018
//
#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

// Used for tracking the data samples
const unsigned long samplesPerSecond = 32;
const unsigned long msBetweenSamples = 1000UL / samplesPerSecond;
unsigned long lastSample = millis();

// Used for tracking the display update
unsigned long lastCycle = millis();
const unsigned long msBetweenCycles = 500;

unsigned int samplePtr = 0;
// This controls how large the window is 
const unsigned int sampleCount = samplesPerSecond * 2;
// This is where the samples are stored
int dataF[sampleCount];
int dataR[sampleCount];

float partVoltage = 3.3;

void setup() {
  Serial.begin(9600);
  delay(100);
  Serial.println("KC1FSZ Return Loss/VSWR Meter");  
  display.begin(SSD1306_SWITCHCAPVCC,SSD1306_I2C_ADDRESS);
  display.clearDisplay();
  display.display();
}

void loop() {

  // Data sample timer. 
  unsigned long now = millis();
  if (now - lastSample > msBetweenSamples) {
    lastSample = now;
    int f = analogRead(A0);
    int r = analogRead(A1);
    dataF[samplePtr] = f;
    dataR[samplePtr] = r;
    samplePtr++;
    if (samplePtr >= sampleCount) {
      samplePtr = 0;
    }
  }
  
  // Check to see if we should generate an output 
  if (now - lastCycle > msBetweenCycles) {
    lastCycle = now;

    // Average the voltage readings from across all of the data we have collected
    float avgF = 0;
    float avgR = 0;
    for (unsigned int i = 0; i < sampleCount; i++) {
      avgF += dataF[i];
      avgR += dataR[i];
    }

    // Update the display if there is any activity
    if (avgF > 16.0) {  
      avgF /= (float)sampleCount;
      // Move to the 3.3V scale
      avgF = (avgF / 1024.0) * partVoltage;
      // Add the diode drop
      //avgF += 0.3;      
      avgR /= (float)sampleCount;
      // Move to the 3.3V scale
      avgR = (avgR / 1024.0) * partVoltage;
      // Add the diode drop      
      //avgR += 0.3;      
      // Compute the reflection coefficient
      float gama = abs(avgR / avgF);
      // Compute the return loss (db)
      float rldb = 20.0 * log10(1.0 / gama);
      // Compute the VSWR
      float vswr = (1.0 + gama) / (1.0 - gama);
      // Forward power
      // This is based on the assumption that the bridge transformers are 12:1
      float pwrF = ((avgF * avgF) / 100.0) / 0.0069;
      
      Serial.print(avgF);
      Serial.print("\t");
      Serial.print(avgR);
      Serial.print("\t");
      Serial.print(gama);
      Serial.print("\t");
      Serial.print(rldb);
      Serial.print("\t");
      Serial.print(pwrF);
      Serial.print("\t");
      Serial.print(vswr);
      Serial.println();

      display.clearDisplay();
      display.setTextSize(0);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
      display.print("KC1FSZ VSWR Meter 1.0");

      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(0,14);
      display.print(abs(vswr));
      
      display.setTextSize(1);
      display.setCursor(70,14);
      display.print(rldb);

      display.setCursor(70,24);
      display.print(pwrF);
      display.print(" ");
      display.print(avgF);

      display.display();
    }
  }
}
