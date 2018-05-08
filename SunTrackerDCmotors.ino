/*(C)2018 JoyLab. www.joyjoy.fr. Two axis DC suntracking controller. Controleur héliostatique 2 axes. by joy.
Arduino Libraries*/
#include <Helios.h>
#include <DS1107H.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>
Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);
Helios helios;     // sun/earth positioning calculator
DS1107H clock;     // RTC horloge temps réel
double dAzimuth;   //  angle d'azimut calculé
double dElevation; //  angle d'élévation calculé
const int nEchantillons = 10; // nombre d'échantillons
int adc[nEchantillons];       // stockage des échantillons lus de l'acceléromètre ADX345 g: accélération de la pesanteur
int magneto[nEchantillons];   // stockage des échantillons lus de la boussole HMC5883 headingDegrees: angle par rapport au nord manétique
int indice = 0;               // indice de l'échantillon courant
float g = 0;                  //  accélération de la pesanteur
float gmoy = 0;               //  moyenne de l'accélération de la pesanteur
float angl = 0;               //  angle mesuré par rapport au nord magnétique corrigé (HMC5883)
float anglmoy = 0;            //  moyenne de l'angle mesuré (HMC5883)
int inputPin = A1;            //  lecture ADC ADX345 axe Z.
const unsigned int X_est = 4;    // pins commande moteur X gauche
const unsigned int X_ouest = 5;  // pins commande moteur X droit
const unsigned int Y_nord = 6;   // pins commande moteur Y haut
const unsigned int Y_sud = 7;    // pins commande moteur Y bas
void displaySensorDetails()  {
  sensor_t sensor;
  mag.getSensor(&sensor);
  delay(500);
}
void setup()  {
  Serial.begin(4800);
  Wire.begin();
  for (int i = 0; i < nEchantillons; i++) {
    adc[i] = 0;
  }
  if (!mag.begin()) {
    while (1);
  }
  displaySensorDetails();
  pinMode(A1, INPUT);
  pinMode(X_est, OUTPUT);
  pinMode(X_ouest, OUTPUT);
  pinMode(Y_nord, OUTPUT);
  pinMode(Y_sud, OUTPUT);
  byte second = 0;
  byte minute = 26;
  byte hour = 14;       // réglage d'horloge UT= GMT(France) -1.
  byte dayOfWeek = 1;
  byte dayOfMonth = 7;
  byte month = 5;
  byte year = 18;       // (supprimer // de clock.setDate. téléverser 20 s avant, remettre // et retéléverser de suite)
  // clock.setDate(second,minute,hour,dayOfWeek,dayOfMonth,month,year);
}
void loop()  {
  sensors_event_t event;
  mag.getEvent(&event);
  float heading = atan2(event.magnetic.y, event.magnetic.x);
  float declinationAngle = 0.004;                              // Angle de déclinaison magnétique
  heading += declinationAngle;
  if (heading < 0)
    heading += 2 * PI;
  if (heading > 2 * PI)
    heading -= 2 * PI;
  float headingDegrees = heading * 180 / M_PI;
  clock.getDate();             //latitude and longitude(5.62879,44.31636 = 44°18'58.91"N, 5°37'43.66"E).
  helios.calcSunPos(clock.year, clock.month, clock.dayOfMonth, clock.hour, clock.minute, clock.second, 5.62879, 44.31636);
  showTime(clock);
  dAzimuth = helios.dAzimuth;
  show("dAzimuth", dAzimuth, true);
  dElevation = helios.dElevation;
  show("dElevation", dElevation, true);
  g = g - adc[indice];
  angl = angl - magneto[indice];
  adc[indice] = analogRead(inputPin);
  magneto[indice] = headingDegrees;
  g = g + adc[indice];
  angl = angl + magneto[indice];
  indice++;
  if (indice >= nEchantillons) {
    indice = 0;
    gmoy = g / nEchantillons;
    anglmoy = angl / nEchantillons;
  }                           // 508: valeur adc (A1) à 0 degré. 610: valeur adc (A1) à 90 degré. 610 - 508 = 102 (échelle π/4 rd)
  float ElevationValue = (gmoy - 508.00) / 102.00 * 90.00;  // Valeur de l'accéléromètre Z entre 0 et + 90 degrés
  float ElValue = atan(3.14159 * dElevation / 180.00) * 90.00;  // Valeur calculée avec l'angle solaire. à comparer avec ElevationValue
  if (indice == 0)  {
    Serial.print("azimut : ");
    Serial.println(anglmoy);
    Serial.print("ElevationValue : ");
    Serial.println(ElevationValue);
    Serial.print("ElValue : ");
    Serial.println(ElValue);
  }                                      // fin de journée. remise à l'est (90 degrés est) pour le matin suivant
  if (dElevation < + 15)                                       // condition de la nuit
    if (indice == 0)  {
      Serial.println("Nuit");
      if (anglmoy > 90) {
        digitalWrite(X_est, HIGH);
        digitalWrite(X_ouest, LOW);
      }
      if (anglmoy < 90) {
        digitalWrite(X_est, HIGH);
        digitalWrite(X_ouest, HIGH);
      }
    }
  if (dElevation > + 15)                                       // condition du jour
    if (indice == 0)  {
      Serial.println("Jour");
    }                                   // asservissement moteur vertical élévation
  if (dElevation > + 15)                                       // conditions du jour
    if (ElevationValue < ElValue - 0.5)
      if (indice == 0)  {
        digitalWrite(Y_nord, LOW);
        digitalWrite(Y_sud, HIGH);
        delay(250);
        digitalWrite(Y_nord, HIGH);
      }
      else if (ElevationValue >= ElValue - 0.5)
        if (ElevationValue <= ElValue + 0.5)  {
          digitalWrite(Y_nord, HIGH);
          digitalWrite(Y_sud, HIGH);
        }
  if (dElevation > + 15)                                     // conditions du jour
    if (ElevationValue > ElValue + 0.5)
      if (indice == 0)  {
        digitalWrite(Y_nord, HIGH);
        digitalWrite(Y_sud, LOW);
        delay(250);
        digitalWrite(Y_sud, HIGH);
      }                               // asservissement moteur horizontal azimut
  if (dElevation > + 15)                                     // conditions du jour
    if (anglmoy < dAzimuth - 0.5)

      if (indice == 0)  {
        digitalWrite(X_est, LOW);
        digitalWrite(X_ouest, HIGH);
        delay(250);
        digitalWrite(X_est, HIGH);
      }
      else if (anglmoy >= dAzimuth - 0.5)
        if (anglmoy <= dAzimuth + 0.5)  {
          digitalWrite(X_est, HIGH);
          digitalWrite(X_ouest, HIGH);
        }
  if (dElevation > + 15)                                     // conditions du jour
    if (anglmoy > dAzimuth + 0.5)
      if (indice == 0)  {
        digitalWrite(X_est, HIGH);
        digitalWrite(X_ouest, LOW);
        delay(250);
        digitalWrite(X_ouest, HIGH);
      }
}
void show(char nameStr[], double val, boolean newline) {  // valeurs d'angles calculées par l'algorithme helios
  if (indice == 0) {
    Serial.print(nameStr);
    Serial.print(" = ");
    if (newline)
      Serial.println(val);
    else Serial.print(val);
  }
}
void showTime(DS1107H timerChip) {
  if (indice == 0) {                                   // valeurs de l'horloge en temps universel                         
    Serial.println("");    
    Serial.print("Temps Universel:  ");
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
    Serial.print("  jour de la semaine: ");
    Serial.println(timerChip.dayOfWeek, DEC);
  }  
  delay(100);  // Durée du loop. Durée d'action = Durée du loop x nEchantillons = 100ms x 10 = 1s
}

