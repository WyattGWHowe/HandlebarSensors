// Pulse Sensor headers
#include "MAX30105.h"
#include "heartRate.h"

//Temperature Sensor Headers
//#include "SparkFun_MLX90632_Arduino_Library.h"

#include <MLX90614.h>

//ESp32 I2C header
#include "Wire.h"

MAX30105 pulseSensor;
MLX90614 tempSensor;

//Sensor On/Off
bool pulse = true;
bool temp = true;
bool GSR = true;

//Heart Rate Variables
const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;
float BPM;
int avgBeats;

//Temp Sensor Variables
long temperature;

//GSR Sensor Variables
const int GSR_PIN=A0;
//int GSR_sensorValue = 0;
int GSR_avg = 0;
int GSR_delay = 5;
int GSR_runAmt = 10;

void setup() {
  Serial.begin(9600);

  //I2C setup
  Wire.begin();

  //Setup pulse sensor to use the I2C connections
  if(pulse) pulseSensor.begin(Wire, I2C_SPEED_FAST);
  if(pulse) pulseSensor.setPulseAmplitudeRed(0x0A);

  //Setup temperature sensor to use I2C connections (done in the cpp file)
  if(temp) tempSensor.begin();
}

void PulseSensorReading(){
long irVal = pulseSensor.getIR(); //Get IR Value from MAX30105

  if(irVal > 7000){ //Check to see if finger is on sensor
    if(checkForBeat(irVal)) //See if beat is detected
    {
      long delta = millis() - lastBeat; //Measure diference between beats
      lastBeat = millis(); //Set last beat to this beat
      BPM = 60/(delta/1000.0); //Work out beats per minute

      if(BPM < 240 && BPM > 30){ //Ensure the BPM makes sense
        rates[rateSpot++] = (byte)BPM; //Insert beat in rate for avg
        rateSpot %= RATE_SIZE; // Ensure it doesnt go over 4
        avgBeats = 0; //Reset avgBeats

        //Loop through rate and set average
        for(byte x = 0; x < RATE_SIZE; x++){
          avgBeats += rates[x];
        }
        avgBeats /= RATE_SIZE;

      }
    }
  }
  else {
    avgBeats = 0; //No heart rate, no avg
  }
}

void TempSensorReading(){
  temperature = tempSensor.getObjectTemp();
}

void GSRSensorReading(){
  // Set base variables
  long amt = 0;
  long GSR_sensorValue = 0;

  //Loop through set amount and return average
  for(int i = 0; i < GSR_runAmt; i++){
    GSR_sensorValue = analogRead(GSR_PIN);
    amt += GSR_sensorValue;
    delay(GSR_delay); //This will hang the rest of the program - might need to be changed
  }
  GSR_avg = amt/GSR_runAmt;
}

void loop() {
  if(pulse) PulseSensorReading();
  if(temp) TempSensorReading();
  if(GSR) GSRSensorReading();

  if(pulse) Serial.println("HEART RATE - BPM: " + String(BPM) + " \n Average Beats: " + String(avgBeats));
  if(temp) Serial.println("TEMPERATURE - Temp: " + String(temperature));
  if(GSR) Serial.println("GSR - GSR Value: " + String(GSR_avg));

}
