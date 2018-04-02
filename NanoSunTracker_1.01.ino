//(C)2014 Helioeo.com. Two axis DC suntracking controller. by joy.

#include <Helios.h>
#include <DS1107H.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>
Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);
Helios helios;    // sun/earth positioning calculator
DS1107H clock;    // RTC
double dAzimuth;
double dElevation;

// analog input pins  ADXL335 Accelerometer sensor (Z for elevation, XY for Tilt)
// const unsigned int ACCX = A1;
// const unsigned int ACCY = A2;
const unsigned int ACCZ = A1;

// digital output pins
const unsigned int X_TRACKING_MOTOR_OK = 10;
const unsigned int Y_TRACKING_MOTOR_OK = 9;
const unsigned int X_TRACKING_MOTOR_EAST = 8;
const unsigned int X_TRACKING_MOTOR_WEST = 7;
const unsigned int Y_TRACKING_MOTOR_NORTH = 6;
const unsigned int Y_TRACKING_MOTOR_SOUTH = 5;

void setup()  {

  Serial.begin(9600);
  Wire.begin();
  if (!mag.begin())
    pinMode(ACCZ, INPUT);
  pinMode(X_TRACKING_MOTOR_OK, OUTPUT);
  pinMode(Y_TRACKING_MOTOR_OK, OUTPUT);
  pinMode(X_TRACKING_MOTOR_EAST, OUTPUT);
  pinMode(X_TRACKING_MOTOR_WEST, OUTPUT);
  pinMode(Y_TRACKING_MOTOR_NORTH, OUTPUT);
  pinMode(Y_TRACKING_MOTOR_SOUTH, OUTPUT);

  // Set clock UT= GMT(France) -1
  //  byte second=0;
  //  byte minute=35;
  //  byte hour=22;
  //  byte dayOfWeek=6;
  //  byte dayOfMonth=31;
  //  byte month=3;
  //  byte year=18;
  //  clock.setDate(second,minute,hour,dayOfWeek,dayOfMonth,month,year);
  delay(250);
}

void loop()  {

  Serial.println("");

  clock.getDate();
  helios.calcSunPos(clock.year, clock.month, clock.dayOfMonth,
                    clock.hour, clock.minute, clock.second, 5.62879, 44.31636);
  //Set the longitude and  the latitude of Etoile St Cyrice, France.
  //(5.62879,44.31636 is for  5°37'43.66"E,44°18'58.91"N).
  showTime(clock);
  dAzimuth = helios.dAzimuth;
  show("dAzimuth", dAzimuth, true);
  dElevation = helios.dElevation;
  show("dElevation", dElevation, true);

  sensors_event_t event;
  mag.getEvent(&event);
  float heading = atan2(event.magnetic.y, event.magnetic.x);
  float declinationAngle = 0.005;
  heading += declinationAngle;
  if (heading < 0)
    heading += 2 * PI;

  // Check for wrap due to addition of declination.
  if (heading > 2 * PI)
    heading -= 2 * PI;

  // Convert radians to degrees for readability.
  float headingDegrees = heading * 180 / M_PI;

  delay(250);

  float AzimutValue = (headingDegrees);
  Serial.print("AzimutValue=");
  Serial.println(AzimutValue, 2);
  float AzValue = (dAzimuth);

  float ElevationValue = (343.00 - analogRead(ACCZ)) * 90.00 / 72.00;
  Serial.print("ElevationValue=");
  Serial.println(ElevationValue, 2);
  float ElValue = sin(3.14159 * dElevation / 180.00) * 90.00;
  Serial.print("ElValue=");
  Serial.println(ElValue, 2);
  
  Serial.print("ACCZ=");
  Serial.println(analogRead(ACCZ));

  if (dElevation < + 15)  {
    Serial.println("NIGHT");
    //    digitalWrite(X_TRACKING_MOTOR_WEST, LOW);
    //    digitalWrite(X_TRACKING_MOTOR_EAST, LOW);
  }
  if (dElevation > + 15)  {
    Serial.println("DAY");
    //    digitalWrite(X_TRACKING_MOTOR_EAST, HIGH);
    //    digitalWrite(X_TRACKING_MOTOR_WEST, LOW);
  }
  if ( AzimutValue < AzValue + 2) {
    digitalWrite(X_TRACKING_MOTOR_OK, HIGH);
  }
  if ( AzimutValue > AzValue - 2) {
    digitalWrite(X_TRACKING_MOTOR_OK, HIGH);
  }
  if ( AzimutValue > AzValue + 2) {
    digitalWrite(X_TRACKING_MOTOR_OK, LOW);
  }
  if ( AzimutValue < AzValue - 2) {
    digitalWrite(X_TRACKING_MOTOR_OK, LOW);
  }
  if (ElevationValue < ElValue + 1.5)  {
    digitalWrite(Y_TRACKING_MOTOR_OK, HIGH);
  }
  if (ElevationValue > ElValue - 1.5)  {
    digitalWrite(Y_TRACKING_MOTOR_OK, HIGH);
  }
  if (ElevationValue > ElValue + 1.5)  {
    digitalWrite(Y_TRACKING_MOTOR_OK, LOW);
  }
  if (ElevationValue < ElValue - 1.5)  {
    digitalWrite(Y_TRACKING_MOTOR_OK, LOW);
  }
  if (ElevationValue < ElValue - 1.5)  {
    digitalWrite(Y_TRACKING_MOTOR_NORTH, LOW);
    digitalWrite(Y_TRACKING_MOTOR_SOUTH, HIGH);
    delay(250);
    digitalWrite(Y_TRACKING_MOTOR_NORTH, HIGH);
  }
  else if (ElevationValue >= ElValue - 1.5)
    if (ElevationValue <= ElValue + 1.5)  {
      digitalWrite(Y_TRACKING_MOTOR_NORTH, HIGH);
      digitalWrite(Y_TRACKING_MOTOR_SOUTH, HIGH);
    }
  if (ElevationValue > ElValue + 1.5)  {
    digitalWrite(Y_TRACKING_MOTOR_NORTH, HIGH);
    digitalWrite(Y_TRACKING_MOTOR_SOUTH, LOW);
    delay(250);
    digitalWrite(Y_TRACKING_MOTOR_SOUTH, HIGH);
  }
  if (AzimutValue < AzValue - 2)  {
    digitalWrite(X_TRACKING_MOTOR_EAST, LOW);
    digitalWrite(X_TRACKING_MOTOR_WEST, HIGH);
    delay(250);
    digitalWrite(X_TRACKING_MOTOR_EAST, HIGH);
  }
  else if (AzimutValue >= AzValue - 2)
    if (AzimutValue <= AzValue + 2)  {
      digitalWrite(X_TRACKING_MOTOR_EAST, HIGH);
      digitalWrite(X_TRACKING_MOTOR_WEST, HIGH);
    }
  if (AzimutValue > AzValue + 2)  {
    digitalWrite(X_TRACKING_MOTOR_EAST, HIGH);
    digitalWrite(X_TRACKING_MOTOR_WEST, LOW);
    delay(250);
    digitalWrite(X_TRACKING_MOTOR_WEST, HIGH);
  }
}
//Sun Position calculation (elevation, azimut)
void show(char nameStr[], double val, boolean newline) {
  Serial.print(nameStr);
  Serial.print("=");
  if (newline)
    Serial.println(val);
  else Serial.print(val);
}

void showTime(DS1107H timerChip) {

  //UTC Timer values
  Serial.print("Time:  ");
  Serial.print(timerChip.hour, DEC);
  Serial.print(":");
  Serial.print(timerChip.minute, DEC);
  Serial.print(":");
  Serial.print(timerChip.second, DEC);
  Serial.print("  ");
  Serial.print(timerChip.month, DEC);
  Serial.print("/");
  Serial.print(timerChip.dayOfMonth, DEC);
  Serial.print("/");
  Serial.print(timerChip.year, DEC);
  Serial.print("  jour de la semaine:");
  Serial.println(timerChip.dayOfWeek, DEC);
  Serial.print("latitude and longitude: ");
  Serial.println("44.31636,5.62879 (Etoile St Cyrice)");
  //  Serial.print("Declinaison magnetique: ");
  //  Serial.println("1deg42,06minE 01/04/2018");
  delay(2000);
}

