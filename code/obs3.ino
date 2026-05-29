//OBSTACLE CHALLENGE CODE

#include <ESP32Servo.h>                                                   // LIBRARIES DEFINITION
#include <Wire.h>
#include <VL53L1X.h>
#include <Adafruit_TCS34725.h>
#include "DFRobot_HuskylensV2.h"

Servo servo;                                                              // LIBRARY VARIABLES DEFINITION
const uint8_t sensorCount = 3;                                            // NUMBER OF DISTANCE SENSORS                                                             // VARIABLE DEFINITION
VL53L1X sensors[sensorCount];
Adafruit_TCS34725 tcs = Adafruit_TCS34725();
HuskylensV2 huskylens;

const int motorA = 27;                                                     // MOTOR PINS
const int motorB = 2;  
const int ENA    = 32;   

const int buttonPin = 13;                                                  // START BUTTON PIN

const uint8_t xshutPins[sensorCount] = {23,5,14};                         // DISTANCE SENSORS XSHUT PINS


int buttonState = HIGH;                                                   //GLOBAL VARIABLES DEFINITION
int LapCount = 0;
long DelayLine = 0;
long MotorMillis = 0;
long PrevMillis = 0;
long PrevMillisF = 0;
int TravelSense = 1;
int CRearSeq=0;

void setup() {
  Serial.begin(115200);
  Wire.begin(16,17);                                                           //INITIALIZE I2C 0 ON DEFAULT PINS 21, 22
  Wire1.begin(21,22);                                                     //INITIALIZE I2C 1 ON DEFAULT PINS 16, 17 
  
  servo.attach(26);                                                       //INITIALIZE SERVO ON PIN 26
  servo.write(90);                                                        //0 POSITION SERVO  
  
  pinMode(buttonPin, INPUT_PULLUP);                                       //START BUTTON 
  
  pinMode(motorA, OUTPUT);                                                //MOTOR OUTPUTS
  pinMode(motorB, OUTPUT);
  digitalWrite(motorA, LOW);                                              //STOP MOTOR
  digitalWrite(motorB, LOW);
  
  ledcAttachChannel(ENA, 1000, 8, 2);                                     //PWM OUTPUT MOTOR 
  
 
  for (uint8_t i = 0; i < sensorCount; i++)                               //INITIALIZE DISTANCE SENSORS 
  {
    pinMode(xshutPins[i], OUTPUT);
    digitalWrite(xshutPins[i], LOW);
  }  
  for (uint8_t i = 0; i < sensorCount; i++)
  {    
    pinMode(xshutPins[i], INPUT);
    delay(10);
    sensors[i].setTimeout(500);
    if (!sensors[i].init())
    {
      Serial.print("Failed to detect and initialize sensor ");
      Serial.println(i);
      while (1);
    }    
    sensors[i].setAddress(0x3A + i);                                      //I2C ADDRESSS SENSORS    
    sensors[i].setDistanceMode(VL53L1X::Long);
    sensors[i].setMeasurementTimingBudget(33000);
    sensors[i].startContinuous(33);
  }

  if (tcs.begin(0x29,&Wire1)) {                                           //INITIALIZE COLOR SENSOR 
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1);
  }
  tcs.setIntegrationTime(TCS34725_INTEGRATIONTIME_2_4MS);
  tcs.setGain(TCS34725_GAIN_60X);

  Serial1.begin(115200, SERIAL_8N1, 18, 19);                               // INIT HUSKYLENS ON UART (RX=18, TX=19)
  huskylens.begin(Serial1);
  huskylens.switchAlgorithm(ALGORITHM_COLOR_RECOGNITION);
  
  int countButton = 0;                                                    //WAIT FOR START BUTTON
  while (countButton <= 10) {
    buttonState = digitalRead(buttonPin);
    if (buttonState == LOW) countButton++;
    delay(10);
  }  

  
  
}

void loop() {
  uint16_t r, g, b, c, colorTemp;

  tcs.getRawData(&r, &g, &b, &c);
  colorTemp = tcs.calculateColorTemperature_dn40(r, g, b, c);

  //Serial.print("Color Temp: "); Serial.println(colorTemp, DEC);             //INFORMATION FOR COLOR SENSOR CALIBRATION
  if (colorTemp > 4100){                                                  //CALIBRATE BEFORE ROUND
    if ((PrevMillis + 3000)<=millis()){
      PrevMillis = millis();
      if (TravelSense >= 2) LapCount--;
      else LapCount++;
    }
  }  

  if (LapCount >= 12) {                                                   //STOP CAR IF NUMBER OF LAPS REACHED
    if ((MotorMillis + 3500) <= millis()) {
      TravelSense=0;    
    }
  } else {    
    MotorMillis = millis();
  }

  switch( TravelSense){                                                   //CAR GO FORWARD, REVERSE OR STOP
    case 0: digitalWrite(motorA, LOW);
            digitalWrite(motorB, LOW);
    break;
    case 1: digitalWrite(motorA, HIGH);
            digitalWrite(motorB, LOW);
    break;
    case 2: digitalWrite(motorA, LOW);
            digitalWrite(motorB, HIGH);
    break;
    
   }
  
  drive();                                                                //CAR MOVEMENT
}


void drive() {
  static signed int distanceRight,distanceLeft,distanceFront;
  static int CountCorrection=0;
  static signed int correction=0;
  huskylens.getResult(ALGORITHM_COLOR_RECOGNITION);
  if (huskylens.available(ALGORITHM_COLOR_RECOGNITION)) {                                              // OBTAIN DATA FROM HUSKYLENS
    //for (int i = 0; i < huskylens.countBlocks(); i++) {
      //Result *ColorID = huskylens.getCachedCenterResult(ALGORITHM_COLOR_RECOGNITION);
      Result *ColorID =huskylens.getCachedResultByIndex(ALGORITHM_COLOR_RECOGNITION,1-1);
       //Serial.println(ColorID->height);
      Serial.println(ColorID->ID);
      //serial.println(result.height);
      if (ColorID->ID == 1 || ColorID->ID == 3  ) {                              // ID1 = GREEN
        CountCorrection=20;                            
        correction=-35;
      }
      else if (ColorID->ID == 2 || ColorID->ID == 4 ) {                         // ID2 = RED
        CountCorrection=20;
        correction=35;
      }
    //}
  }

  
  /*if ((huskylens.available(ALGORITHM_COLOR_RECOGNITION))) {
    Serial.println((String("ID of the color block near the center: ") + String((RET_ITEM_NUM(huskylens.getCachedCenterResult(ALGORITHM_COLOR_RECOGNITION), Result, ID)))));
    Serial.println((String("Total number of detected color blocks: ") + String((huskylens.getCachedResultNum(ALGORITHM_COLOR_RECOGNITION)))));
    Serial.println((String("ID of the first detected color block: ") + String((RET_ITEM_NUM(huskylens.getCachedResultByIndex(ALGORITHM_COLOR_RECOGNITION, 1-1), Result, ID)))));
  */

  if (sensors[0].dataReady()){                                            //READ DISTANCE SENSORS
    distanceRight = sensors[0].read(false);
    if (distanceRight>1500) distanceRight=1500;
    }  

  if (sensors[1].dataReady()){
    distanceLeft = sensors[1].read(false);
    if (distanceLeft>1500) distanceLeft=1500;
    }  
  
  if (sensors[2].dataReady()){
    distanceFront = sensors[2].read(false);}
  
  if (distanceFront<220){                                                             //IF FRONT SENSOR DETECTS OBSTACLE INIT REAR SEQUENCE
    if ((PrevMillisF + 1500)<=millis()){
      PrevMillisF = millis();
      CRearSeq=70;
      }
    }
   
  if(CRearSeq>=60) TravelSense=0;                                                     //REAR SEQUENCE
  if(CRearSeq>0 && CRearSeq<60) TravelSense=2;
  if(CRearSeq==1) TravelSense=1;
  if(CRearSeq>0) CRearSeq--;

  signed int dServo = (distanceRight - distanceLeft )/20;                             //TURN ALGORITHM
  if (dServo > 40) dServo = 40;
  if (dServo < -40) dServo = -40;
  if (CountCorrection>0) {
    dServo=correction;
    CountCorrection--;
    } 
  if (TravelSense >=2) servo.write(-dServo + 85);
  else if (TravelSense ==1)servo.write(dServo + 85);
  
  if (dServo>=0) ledcWrite(ENA,80+dServo);                                           //ADJUST MOTOR SPEED WHEN TURNING
    else ledcWrite(ENA, 80-dServo);
}
