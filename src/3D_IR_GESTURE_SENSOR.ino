/* 
 *  Arduino 3D Gesture Recognizer based on IR Photodiodes
 *  
 *  Created by Nicolas Britos
 *  
 *  May, 2016
 *  
 *  V1.0
 *  
 *  iosnicoib@hotmail.com  
 */

#define IR_RX_TL A0                 // Top Left IR receiver photodiode on analog pin A0
#define IR_RX_TR A1                 // Top Right IR receiver photodiode on analog pin A1
#define IR_RX_BL A2                 // IR receiver photodiode on analog pin A2
#define IR_RX_BR A3                 // IR receiver photodiode on analog pin A3

#define IR_TX_TL 2                  // Top Left IR emitter LED on digital pin 2
#define IR_TX_TR 3                  // Top Right IR emitter LED on digital pin 3
#define IR_TX_BL 4                  // Bottom Left IR emitter LED on digital pin 4
#define IR_TX_BR 5                  // Bottom Right IR emitter LED on digital pin 5

bool readGesture = true;            // Used to process the gesture

int ambientIR_TL;                   // Stores the ambient IR value read from the TL IR receiver
int ambientIR_TR;                   // Stores the ambient IR value read from the TR IR receiver
int ambientIR_BL;                   // Stores the ambient IR value read from the BL IR receiver
int ambientIR_BR;                   // Stores the ambient IR value read from the BR IR receiver
int obstacleIR_TL;                  // Stores the raw object IR value read from the TL IR receiver
int obstacleIR_TR;                  // Stores the raw object IR value read from the TR IR receiver
int obstacleIR_BL;                  // Stores the raw object IR value read from the BL IR receiver
int obstacleIR_BR;                  // Stores the raw object IR value read from the BR IR receiver
int value_TL[10];                   // Stores the IR value read from the TL IR receiver
int value_TR[10];                   // Stores the IR value read from the TR IR receiver
int value_BL[10];                   // Stores the IR value read from the BL IR receiver
int value_BR[10];                   // Stores the IR value read from the BR IR receiver
int distance_TL;                    // Stores the mapped IR value read from the TL IR receiver
int distance_TR;                    // Stores the mapped IR value read from the TR IR receiver
int distance_BL;                    // Stores the mapped IR value read from the BL IR receiver
int distance_BR;                    // Stores the mapped IR value read from the BR IR receiver
int calibration_TL;                 // Stores the raw IR value read from the TL receiver when the Arduino is powered
int calibration_TR;                 // Stores the raw IR value read from the TR receiver when the Arduino is powered
int calibration_BL;                 // Stores the raw IR value read from the BL receiver when the Arduino is powered
int calibration_BR;                 // Stores the raw IR value read from the BR receiver when the Arduino is powered
int lastDistanceHold_TL;            // The next variables are used to detect a "hold gesture" action
int lastDistanceHold_TR;
int lastDistanceHold_BL;
int lastDistanceHold_BR;
int holdMillis = 300;               // Used in a timer to determine a "hold gesture" action

uint8_t distanceArray_TL[100];      // Stores the last 100 IR values from the TL IR receiver to determine the gesture later
uint8_t distanceArray_TR[100];      // Stores the last 100 IR values from the TR IR receiver to determine the gesture later
uint8_t distanceArray_BL[100];      // Stores the last 100 IR values from the BL IR receiver to determine the gesture later
uint8_t distanceArray_BR[100];      // Stores the last 100 IR values from the BR IR receiver to determine the gesture later
uint8_t distanceArrayCounter = 0;   // Used to access the previous array's content
uint8_t gesture = 0;                // Stores the gesture

uint32_t lastHoldMillis = 0;        // Used to control the "hold gesture" timer

void setup(){
  Serial.begin(115200);
  pinMode(IR_TX_TL, OUTPUT);
  pinMode(IR_TX_TR, OUTPUT);
  pinMode(IR_TX_BL, OUTPUT);
  pinMode(IR_TX_BR, OUTPUT);
  digitalWrite(IR_TX_TL, LOW);
  digitalWrite(IR_TX_TR, LOW);
  digitalWrite(IR_TX_BL, LOW);
  digitalWrite(IR_TX_BR, LOW);
  calibrateIR();                    // Store the current IR values to calibrate the sensors later
}

void loop(){
  readIR(5);                        // Read 5 times the IR values and store the average
  determineGesture();               // It determines the type of gesture made
  processHoldGesture();             // It checks if the hand is being hold
}

void readIR(uint8_t times){
  for(uint8_t x = 0; x < times; x++){
    digitalWrite(IR_TX_TL,LOW);                   // We need to turn off the IR LEDs to read the ambient IR
    digitalWrite(IR_TX_TR,LOW);
    digitalWrite(IR_TX_BL,LOW);
    digitalWrite(IR_TX_BR,LOW);
    delay(2);                                     // Wait until the IR LEDs are completely turned off
    ambientIR_TL = analogRead(IR_RX_TL);          // Stores the ambient IR light
    ambientIR_TR = analogRead(IR_RX_TR);
    ambientIR_BL = analogRead(IR_RX_BL);
    ambientIR_BR = analogRead(IR_RX_BR);
    digitalWrite(IR_TX_TL,HIGH);                  // Turn on the IR LEDs to read the IR light reflected by the obstacle
    digitalWrite(IR_TX_TR,HIGH);
    digitalWrite(IR_TX_BL,HIGH);
    digitalWrite(IR_TX_BR,HIGH);
    delay(1);                                     // Wait until the IR LEDs are completely turned on
    obstacleIR_TL = analogRead(IR_RX_TL);         // Stores the IR light reflected by the obstacle
    obstacleIR_TR = analogRead(IR_RX_TR);
    obstacleIR_BL = analogRead(IR_RX_BL);
    obstacleIR_BR = analogRead(IR_RX_BR);
    value_TL[x] = ambientIR_TL - obstacleIR_TL;   // Store the IR value to later calculate the average
    value_TR[x] = ambientIR_TR - obstacleIR_TR;
    value_BL[x] = ambientIR_BL - obstacleIR_BL;
    value_BR[x] = ambientIR_BR - obstacleIR_BR;
  }
  for(uint8_t x = 0; x < times; x++){             // Calculate the average IR value per sensor
    distance_TL += value_TL[x];
    distance_TR += value_TR[x];
    distance_BL += value_BL[x];
    distance_BR += value_BR[x];
  }
  distance_TL = (distance_TL / times);            // Save the distance value
  distance_TR = (distance_TR / times);
  distance_BL = (distance_BL / times);
  distance_BR = (distance_BR / times);
  if(calibration_TL + 50 > distance_TL){          // Calibrate the sensors
    distance_TL = 0;
  }
  else{
    distance_TL -= calibration_TL + 50;
  }
  if(calibration_TR + 50 > distance_TR){
    distance_TR = 0;
  }
  else{
    distance_TR -= calibration_TR + 50;
  }
  if(calibration_BL + 50 > distance_BL){
    distance_BL = 0;
  }
  else{
    distance_BL -= calibration_BL + 50;
  }
  if(calibration_BR + 50 > distance_BR){
    distance_BR = 0;
  }
  else{
    distance_BR -= calibration_BR + 50;
  }
  // If any distance is greater than 0 then map it from a 0-1100 range to a 0-255 range (to be able to store it in a 1B variable) and store it in an array
  if(distance_TL != 0 || distance_TR != 0 || distance_BL != 0 || distance_BR != 0){
    distance_TR = map(constrain(distance_TR, 0, 1100), 0, 1100, 0, 255);
    distance_TL = map(constrain(distance_TL, 0, 1100), 0, 1100, 0, 255);
    distance_BR = map(constrain(distance_BR, 0, 1100), 0, 1100, 0, 255);
    distance_BL = map(constrain(distance_BL, 0, 1100), 0, 1100, 0, 255);
    if(distanceArrayCounter < 100){
      distanceArray_TR[distanceArrayCounter] = distance_TR;
      distanceArray_TL[distanceArrayCounter] = distance_TL;
      distanceArray_BR[distanceArrayCounter] = distance_BR;
      distanceArray_BL[distanceArrayCounter] = distance_BL;
      distanceArrayCounter += 1;
    }
    else{      
      distanceArray_TR[0] = distance_TR;
      distanceArray_TL[0] = distance_TL;
      distanceArray_BR[0] = distance_BR;
      distanceArray_BL[0] = distance_BL;
      distanceArrayCounter = 1;
    }
  }
  else if(distance_TL == 0 && distance_TR == 0 && distance_BL == 0 && distance_BR == 0){      // If not then reset the arrays
    readGesture = true;
    distanceArrayCounter = 0;
    distanceArray_TR[0] = 0;
    distanceArray_TL[0] = 0;
    distanceArray_BR[0] = 0;
    distanceArray_BL[0] = 0;
  }
}

void calibrateIR(){
  for(uint8_t x = 0; x < 5; x++){
    digitalWrite(IR_TX_TL,LOW);
    digitalWrite(IR_TX_TR,LOW);
    digitalWrite(IR_TX_BL,LOW);
    digitalWrite(IR_TX_BR,LOW);
    delay(2);
    ambientIR_TL = analogRead(IR_RX_TL);
    ambientIR_TR = analogRead(IR_RX_TR);
    ambientIR_BL = analogRead(IR_RX_BL);
    ambientIR_BR = analogRead(IR_RX_BR);
    digitalWrite(IR_TX_TL,HIGH);
    digitalWrite(IR_TX_TR,HIGH);
    digitalWrite(IR_TX_BL,HIGH);
    digitalWrite(IR_TX_BR,HIGH);
    delay(1);
    obstacleIR_TL = analogRead(IR_RX_TL);
    obstacleIR_TR = analogRead(IR_RX_TR);
    obstacleIR_BL = analogRead(IR_RX_BL);
    obstacleIR_BR = analogRead(IR_RX_BR);
    value_TL[x] = ambientIR_TL - obstacleIR_TL;
    value_TR[x] = ambientIR_TR - obstacleIR_TR;
    value_BL[x] = ambientIR_BL - obstacleIR_BL;
    value_BR[x] = ambientIR_BR - obstacleIR_BR;
  }
  for(uint8_t x = 0; x < 5; x++){
    distance_TL += value_TL[x];
    distance_TR += value_TR[x];
    distance_BL += value_BL[x];
    distance_BR += value_BR[x];
  }
  calibration_TL = (distance_TL / 5);
  calibration_TR = (distance_TR / 5);
  calibration_BL = (distance_BL / 5);
  calibration_BR = (distance_BR / 5);
}

void determineGesture(){
  if(distanceArrayCounter > 2){       // If the arrays have more than 2 elements then determine the gesture
    uint8_t distancePost_TL = 0;      // The next "distancePost..." vars are the sum of it previous value plus 
    uint8_t distancePost_TR = 0;      // the last element minus the previous element of the array
    uint8_t distancePost_BL = 0;
    uint8_t distancePost_BR = 0;
    uint8_t distancePostNeg_TL = 0;
    uint8_t distancePostNeg_TR = 0;
    uint8_t distancePostNeg_BL = 0;
    uint8_t distancePostNeg_BR = 0;
    uint8_t expectedGesture = 0;      // Used later to determine the gesture made
    bool increasing_TL = false;       // Used later to determine the gesture made. If the IR sensor's value 
    bool decreasing_TL = false;       // increases then "increasing_xx" is true. Otherwise, if it decreasing 
    bool increasing_TR = false;       // then "decreasing_xx" is true
    bool decreasing_TR = false;
    bool increasing_BL = false;
    bool decreasing_BL = false;
    bool increasing_BR = false;
    bool decreasing_BR = false;
    // For every element in the distanceArray...
    for(uint8_t x = 1; x < distanceArrayCounter; x++){
      // Update the "distancePost..." vars values
      if(distanceArray_TL[x] - distanceArray_TL[x-1] > 0){
        distancePost_TL += distanceArray_TL[x] - distanceArray_TL[x-1];
      }
      else{
        distancePostNeg_TL += distanceArray_TL[x-1] - distanceArray_TL[x];
      }
      
      if(distanceArray_TR[x] - distanceArray_TR[x-1] > 0){
        distancePost_TR += distanceArray_TR[x] - distanceArray_TR[x-1];
      }
      else{
        distancePostNeg_TR += distanceArray_TR[x-1] - distanceArray_TR[x];
      }
      
      if(distanceArray_BL[x] - distanceArray_BL[x-1] > 0){
        distancePost_BL += distanceArray_BL[x] - distanceArray_BL[x-1];
      }
      else{
        distancePostNeg_BL += distanceArray_BL[x-1] - distanceArray_BL[x];
      }
      
      if(distanceArray_BR[x] - distanceArray_BR[x-1] > 0){
        distancePost_BR += distanceArray_BR[x] - distanceArray_BR[x-1];
      }
      else{
        distancePostNeg_BR += distanceArray_BR[x-1] - distanceArray_BR[x];
      }
      
      // Based on the "distancePost..." vars determine if the value is increasing or decreasing
      if(distancePost_TL > 30 && distancePostNeg_TL < 30){
        increasing_TL = true;
        decreasing_TL = false;
      }
      else if(distancePostNeg_TL > 30){
        increasing_TL = false;
        decreasing_TL = true;
      }
      
      if(distancePost_TR > 30 && distancePostNeg_TR < 30){
        increasing_TR = true;
        decreasing_TR = false;
      }
      else if(distancePostNeg_TR > 30){
        increasing_TR = false;
        decreasing_TR = true;
      }
      
      if(distancePost_BL > 30 && distancePostNeg_BL < 30){
        increasing_BL = true;
        decreasing_BL = false;
      }
      else if(distancePostNeg_BL > 30){
        increasing_BL = false;
        decreasing_BL = true;
      }
      
      if(distancePost_BR > 30 && distancePostNeg_BR < 30){
        increasing_BR = true;
        decreasing_BR = false;
      }
      else if(distancePostNeg_BR > 30){
        increasing_BR = false;
        decreasing_BR = true;
      }

      // Determine the type of gesture
      switch(expectedGesture){
        case 0:                   // If "expectedGesture" is 0 then determine the expected gesture 
                                  // by analyzing if the sensors' values are increasing or decreasing
          if(increasing_TL && increasing_BL && !increasing_TR && !increasing_BR){         // Left to right movement
            expectedGesture = 1;  // Expected an "east" gesture
          }
          else if(increasing_TR && increasing_BR && !increasing_TL && !increasing_BL){    // Right to left movement
            expectedGesture = 2;  // Expected a "west" gesture
          }
          else if(increasing_TR && increasing_TL && !increasing_BL && !increasing_BR){    // Top to bottom movement
            expectedGesture = 3;  // Expected a "south" gesture
          }
          else if(increasing_BR && increasing_BL && !increasing_TL && !increasing_TR){    // Bottom to top movement
            expectedGesture = 4;  // Expected a "north" gesture
          }
          // Check if the current "distance_xx" minus the 2nd element in the array is greater than 50 then it is a "down" movement.
          // The "increasing_xx" vars are not used because they are very precise and they didn´t detect the "down" movement most of the time.
          else if(distance_TL - distanceArray_TL[2] > 50 && distance_TR - distanceArray_TR[2] > 50 && distance_BL - distanceArray_BL[2] > 50 && distance_BR - distanceArray_BR[2] > 50){  // Up to down movementd
            expectedGesture = 5;  // Expected a "down" gesture
          }
          break;
        case 1:   
          // If "expectedGesture" is "east" then check if TR and BR sensors are increasing and TL and BL are decreasing. 
          // If this is true then save the current distance to the var "lastDistanceHold" to use it later to check a "hold 
          // gesture" action. Also, reset the "expectedGesture" var and set the "gesture" var to 1 ("east")
          if(increasing_TR && increasing_BR && decreasing_TL && decreasing_BL){
            lastDistanceHold_TR = distance_TR;
            lastDistanceHold_BR = distance_BR;
            lastDistanceHold_TL = distance_TL;
            lastDistanceHold_BL = distance_BL;
            if(readGesture){
              Serial.println("EAST");
              gesture = 1;
              holdMillis = 300;   // Change it to 300ms to add a small delay between the gesture "east" and the "hold gesture" action
              lastHoldMillis = millis();
            }
            expectedGesture = 0;
            increasing_TL = false;
            decreasing_TL = false;
            increasing_TR = false;
            decreasing_TR = false;
            increasing_BL = false;
            decreasing_BL = false;
            increasing_BR = false;
            decreasing_BR = false;
            readGesture = false;
          }
          break;
        case 2:
          if(increasing_TL && increasing_BL && decreasing_BR && decreasing_TR){
            if(readGesture){
              Serial.println("WEST");
              gesture = 2;
              holdMillis = 300;
              lastHoldMillis = millis();
            }
            expectedGesture = 0;
            lastDistanceHold_TR = distance_TR;
            lastDistanceHold_BR = distance_BR;
            lastDistanceHold_TL = distance_TL;
            lastDistanceHold_BL = distance_BL;
            increasing_TL = false;
            decreasing_TL = false;
            increasing_TR = false;
            decreasing_TR = false;
            increasing_BL = false;
            decreasing_BL = false;
            increasing_BR = false;
            decreasing_BR = false;
            readGesture = false;
          }
          break;
        case 3:
          if(increasing_BR && increasing_BL && decreasing_TL && decreasing_TR){
            if(readGesture){
              Serial.println("SOUTH");
              gesture = 3;
              holdMillis = 300;
              lastHoldMillis = millis();
            }
            expectedGesture = 0;
            lastDistanceHold_TR = distance_TR;
            lastDistanceHold_BR = distance_BR;
            lastDistanceHold_TL = distance_TL;
            lastDistanceHold_BL = distance_BL;
            increasing_TL = false;
            decreasing_TL = false;
            increasing_TR = false;
            decreasing_TR = false;
            increasing_BL = false;
            decreasing_BL = false;
            increasing_BR = false;
            decreasing_BR = false;
            readGesture = false;
          }
          break;
        case 4:
          if(increasing_TR && increasing_TL && decreasing_BL && decreasing_BR){
            if(readGesture){
              Serial.println("NORTH");
              gesture = 4;
              holdMillis = 300;
              lastHoldMillis = millis();
            }
            expectedGesture = 0;
            lastDistanceHold_TR = distance_TR;
            lastDistanceHold_BR = distance_BR;
            lastDistanceHold_TL = distance_TL;
            lastDistanceHold_BL = distance_BL;
            increasing_TL = false;
            decreasing_TL = false;
            increasing_TR = false;
            decreasing_TR = false;
            increasing_BL = false;
            decreasing_BL = false;
            increasing_BR = false;
            decreasing_BR = false;
            readGesture = false;
          }
          break;
        case 5:
          // The next if statement checks if 3 sensors are increasing its value and if it is true then store a "down" gesture
          if((increasing_BR && increasing_BL && increasing_TL) || (increasing_TR && increasing_BL && increasing_TL) || (increasing_TR && increasing_BR && increasing_TL) || (increasing_TR && increasing_BR && increasing_BL)){
            if(readGesture){
              Serial.println("DOWN");
              gesture = 5;
              holdMillis = 300;
              lastHoldMillis = millis();
            }
            expectedGesture = 0;
            increasing_TL = false;
            decreasing_TL = false;
            increasing_TR = false;
            decreasing_TR = false;
            increasing_BL = false;
            decreasing_BL = false;
            increasing_BR = false;
            decreasing_BR = false;
            readGesture = false;
          }
          break;
      } 
    }
  }
}

void processHoldGesture(){
  // If the actual "distance_xx" minus the "lastDistanceHold_xx" (saved when the gesture was detected) 
  // is within a range (20) then make a "hold gesture" action
  if(-20 <= distance_TR - lastDistanceHold_TR && distance_TR - lastDistanceHold_TR <= 20 && -20 <= distance_TL - lastDistanceHold_TL && distance_TL - lastDistanceHold_TL <= 20 && -20 <= distance_BR - lastDistanceHold_BR && distance_BR - lastDistanceHold_BR <= 20 && -20 <= distance_BL - lastDistanceHold_BL && distance_BL - lastDistanceHold_BL <= 20){
    if(distance_TR != 0 || distance_TL != 0 || distance_BR != 0 || distance_BL != 0){
      switch(gesture){
        case 1:
          if(millis() - lastHoldMillis > holdMillis){
            holdMillis = 100;
            Serial.println("HOLD EAST");
            lastHoldMillis = millis();
          }
          break;
        case 2:
          if(millis() - lastHoldMillis > holdMillis){
            holdMillis = 100;
            Serial.println("HOLD WEST");
            lastHoldMillis = millis();
          }
          break;
        case 3:
          if(millis() - lastHoldMillis > holdMillis){
            holdMillis = 100;
            Serial.println("HOLD SOUTH");
            lastHoldMillis = millis();
          }
          break;
        case 4:
          if(millis() - lastHoldMillis > holdMillis){
            holdMillis = 100;
            Serial.println("HOLD NORTH");
            lastHoldMillis = millis();
          }
          break;
      }
    }
    else{
      // Set a high number for the "lastDistanceHold" vars to stop reading a "hold gesture" action
      lastDistanceHold_TL = 20000;
      lastDistanceHold_TR = 20000;
      lastDistanceHold_BR = 20000;
      lastDistanceHold_BL = 20000;
    }
  }
  else{
    lastDistanceHold_TL = 20000;
    lastDistanceHold_TR = 20000;
    lastDistanceHold_BL = 20000;
    lastDistanceHold_BR = 20000;
  }
}

