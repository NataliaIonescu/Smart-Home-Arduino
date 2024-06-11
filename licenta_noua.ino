/*=========================LIBRARIES====================================================*/
#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
/*=======================END OF LIBRARIES===============================================*/

/*=======================DEFINES========================================================*/
#define whiteLed 13
#define whiteLedPeriod 5000
#define yellowLed 5
#define INA_Fan 7
#define INB_Fan 6
#define doorOpenAngle 120
#define doorCloseAngle 45
#define windowOpenAngle 120
#define windowCloseAngle 0
#define buzzer 3
#define pirMotionSensor 2
#define gasSensorA A0
#define gasSensorD 11
#define calibrationPeriod 22000
#define waterSensor A3
#define photocellSensor A1
#define soilMoistureSensor A2
#define redButton 4
#define blueButton 8
#define sensorReadPeriod 200
#define openDoorPeriod 4000
#define openMessPeriod 2000
#define errMessPeriod 1500
#define againMessPeriod 1500
#define btnPeriod 1000
/*======================END OF DEFINES==================================================*/

/*=======================GLOBAL VARIABLES===============================================*/
LiquidCrystal_I2C mylcd(0x27, 16, 2);

volatile int incomingValue = 0;

volatile bool whiteLedOnFlag = false;
unsigned long startWhiteLedTime = 0; 
unsigned long endWhiteLedTime = 0; 

Servo doorServo;
Servo windowServo;

bool flagDoorSwOn = false;

volatile int pirMotionValue;  
unsigned long lastPIRReadTime = 0;
unsigned long currentMillisPIR = 0;

unsigned long startGasTime = 0;

//Variables used to calculate the average of sensors' values
int gasMedian = 0;
int waterMedian = 0;
int photocellMedian = 0;
int soilMoistureMedian = 0;
int gasSum = 0;
int waterSum = 0;
int photocellSum = 0;
int soilMoistureSum = 0;

//Button states used to detect pushed buttons
unsigned long btnRedPressTime = 0;
String password = "";
volatile int btnRedState = 0;     
volatile int lastBtnRedState = 0; 
unsigned long btnRedStartPressed = 0;    
unsigned long btnRedEndPressed = 0;      

unsigned long startOpenTime = 0;    
unsigned long endOpenTime = 0;
volatile bool messageOpenFlag = false;

unsigned long startErrorTime = 0;    
unsigned long endErrorTime = 0;
volatile bool messageErrorFlag = false;

unsigned long startAgainTime = 0;    
unsigned long endAgainTime = 0;
volatile bool messageAgainFlag = false;

unsigned long startOpenDoorTime = 0;
unsigned long endOpenDoorTime = 0;
volatile bool openDoorFlag = false;

//Variables used to compute the average of sensors' values from 5 readings at 200ms interval
unsigned long lastSensorReadTime = 0;
unsigned long currentMillisSensor = 0;
unsigned int number_of_readings = 0;
/*===========================END OF GLOBAL VARIABLES==================================*/

void setup(){

  //Set pins mode
  pinMode(whiteLed, OUTPUT);
  pinMode(yellowLed, OUTPUT);
  pinMode(INA_Fan, OUTPUT);
  pinMode(INB_Fan, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(pirMotionSensor, INPUT);
  pinMode(photocellSensor, INPUT);
  pinMode(soilMoistureSensor, INPUT);
  pinMode(gasSensorA, INPUT);
  pinMode(gasSensorD, INPUT);
  pinMode(waterSensor, INPUT);
  pinMode(redButton, INPUT);
  pinMode(blueButton, INPUT);

  Serial.begin(9600);

  //Initialize LCD
  mylcd.init();
  mylcd.backlight();
  mylcd.print("Enter password:");

  //Initialize servo motors
  doorServo.attach(9);
  windowServo.attach(10);
  doorServo.write(doorCloseAngle);
  windowServo.write(windowCloseAngle);

  // Start gas sensor calbration time
  startGasTime = millis();
}

void loop(){
  
  auto_sensor();
  
  if (Serial.available() > 0) 
  {
    incomingValue = Serial.read();
  }
  switch (incomingValue) 
  {
    // If Interior Light Switch is turned ON and no gas is detected
    case 'a':
      if(gasMedian <= 341 )
      {
        digitalWrite(yellowLed, HIGH); 
      }
      break;
    // If Interior Light Switch is turned OFF 
    case 'b':
      digitalWrite(yellowLed, LOW); 
      break;
    // If Ventilation Switch is turned ON
    case 'r':
      digitalWrite(INA_Fan, LOW);
      analogWrite(INB_Fan, 100); 
      break;
    // If Ventilation Switch is turned OFF
    case 's':
      digitalWrite(INA_Fan, LOW);
      analogWrite(INB_Fan, 0); 
      break;
    // If Door Switch is turned ON
    case 'l':    
    if(openDoorFlag == false) { 
      doorServo.write(doorOpenAngle);
      flagDoorSwOn = true; 
    }
      break;
    // If Door Switch is turned OFF
    case 'm':
      if (flagDoorSwOn == true){
        doorServo.write(doorCloseAngle);
        flagDoorSwOn = false;      
      }
      break;
    // If Window Switch is turned ON and it is not raining
    case 'n':
      if(waterMedian <= 341)
      {
        windowServo.write(windowOpenAngle);
      }
      break;
    // If Window Switch is turned OFF
    case 'o':
      windowServo.write(windowCloseAngle);
      break;
  }
}
// Function for sensors' functionalities
void auto_sensor(){
  // Read the sensor values and send the values to the application once per second
  currentMillisSensor = millis();
  if(currentMillisSensor - lastSensorReadTime > sensorReadPeriod) 
  {
    lastSensorReadTime = currentMillisSensor;

    gasSum +=  analogRead(gasSensorA);
    waterSum +=  analogRead(waterSensor);
    photocellSum += analogRead(photocellSensor);
    soilMoistureSum += analogRead(soilMoistureSensor);
    
    number_of_readings++;

    //if 5 readings have passed = 1 second
    if(number_of_readings == 5)
    {
      //Compute average of sensors' values
      gasMedian = gasSum / 5;
      waterMedian = waterSum / 5;
      photocellMedian = photocellSum / 5;
      soilMoistureMedian = soilMoistureSum / 5;

      Serial.print(gasMedian);
      Serial.print("|");
      Serial.print(waterMedian);
      Serial.print("|");
      Serial.print(photocellMedian);
      Serial.print("|");
      Serial.println(soilMoistureMedian);

      //reset variables
      gasSum = 0;
      waterSum = 0;
      photocellSum = 0;
      soilMoistureSum = 0;
      number_of_readings = 0;
    }
    
  }
  // After calibration time, if gas is detected, an alarm will sound and the interior light will turn off
  if (millis() - startGasTime < calibrationPeriod) 
  {
    noTone(buzzer);
  }
  else{
    if (gasMedian > 341 && gasMedian <= 600) 
    {
      digitalWrite(yellowLed, LOW);
      tone(buzzer, 500);
    } 
    else if (gasMedian > 600) 
    {
      tone(buzzer, 1000);
    }
    else 
    {
      noTone(buzzer);
    }
  }

  // If rain is detected, the window will close
  if (waterMedian > 341)
  {
    windowServo.write(windowCloseAngle); 
  } 

  // If it is dark and any movement is detected, the exterior light will turn on for 5 seconds
  if (photocellMedian > 682 ) 
  {
    currentMillisPIR = millis();
    if (currentMillisPIR - lastPIRReadTime > sensorReadPeriod)
    {
      lastPIRReadTime = currentMillisPIR;
      pirMotionValue = digitalRead(pirMotionSensor);
    
      if(pirMotionValue == HIGH && whiteLedOnFlag == false) 
      {
        digitalWrite(whiteLed, HIGH);
        startWhiteLedTime = millis();
        whiteLedOnFlag = true;
      }
    }
  }

  endWhiteLedTime = millis();
  if (endWhiteLedTime - startWhiteLedTime >= whiteLedPeriod) 
  {
    digitalWrite(whiteLed, LOW);
    whiteLedOnFlag = false;
  }

  door();
}

// Function for the functionality of opening the door using the password
void door(){

// If the Door Switch is turned OFF
  if(  flagDoorSwOn == false )
  {
    btnRedState = digitalRead(redButton); 
    if (btnRedState != lastBtnRedState) 
    { 
      if (btnRedState == LOW)  
      {
        btnRedStartPressed = millis();
      } 
      else 
      { 
        btnRedEndPressed = millis();
        if (btnRedEndPressed - btnRedStartPressed > 0 && btnRedEndPressed - btnRedStartPressed <= btnPeriod)
        {
          password += ".";
        }
        else if (btnRedEndPressed - btnRedStartPressed > btnPeriod && btnRedStartPressed != 0)
        {
          password += "-";
        }
        mylcd.setCursor(0, 1);
        mylcd.print(password);
      }
    }

    lastBtnRedState = btnRedState;        

    if (digitalRead(blueButton) == LOW) 
    {
      if (password == ".--.") 
      {
        mylcd.clear();
        mylcd.print("OPEN!");
        startOpenTime = millis();
        messageOpenFlag=true;
        doorServo.write(doorOpenAngle);
        startOpenDoorTime = millis();
        openDoorFlag = true;
      } else if (password != ".--." && password != "")
      {
        mylcd.clear();
        mylcd.print("ERROR!");
        messageErrorFlag = true;
        startErrorTime = millis();
        
      }
      password = ""; 
    
    }
    
    endOpenTime = millis();
    if(messageOpenFlag == true && endOpenTime - startOpenTime >= openMessPeriod)
    {
      mylcd.clear();
      mylcd.print("Enter password:");
      messageOpenFlag = false;
    }

    endErrorTime=millis();
    if(messageErrorFlag == true && endErrorTime - startErrorTime >= errMessPeriod)
    {
      mylcd.clear();
      mylcd.print("AGAIN!");
      startAgainTime = millis();
      messageErrorFlag = false;
      messageAgainFlag = true;
    }

    endAgainTime = millis();
    if(messageAgainFlag == true && endAgainTime - startAgainTime >= againMessPeriod)
    {
      mylcd.clear();
      mylcd.print("Enter password:");
      messageAgainFlag = false;
    }

    endOpenDoorTime = millis();
    if (openDoorFlag == true && endOpenDoorTime - startOpenDoorTime >= openDoorPeriod)
    {
      doorServo.write(doorCloseAngle);
      openDoorFlag = false;
    }
  }

}
