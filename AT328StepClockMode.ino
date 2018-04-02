#include <SPI.h>
#include "L6470.h"

//328 pro mini 3v3 8Mhz

#define SLAVE_SELECT_PIN 10  // Wire this to the CSN pin
#define MOSI             11  // Wire this to the SDI pin
#define MISO             12  // Wire this to the SDO pin
#define SCK              13  // Wire this to the CK pin
#define dSPIN_BUSYN       9  // Wire this to the BSYN line  
#define dSPIN_RESET       8  // Wire this to the RESET pin
#define Flag              7  // Wire this to the RESET pin

const int ledPin =  4; // Status Led Pin
int Spin = 2;
//int Enable = 3;


void setup()
{
  Serial.begin(9600);
  pinMode(SLAVE_SELECT_PIN, OUTPUT);
  pinMode(dSPIN_BUSYN, INPUT);
  pinMode(Flag, INPUT);
  pinMode(Spin, INPUT);
  //  pinMode(Enable, INPUT);
  pinMode(ledPin, OUTPUT);
  
  dSPIN_init();

  dSPIN_SetParam(dSPIN_STEP_MODE,
                 !dSPIN_SYNC_EN |
                 dSPIN_STEP_SEL_1_128 |
                 dSPIN_SYNC_SEL_64);
  dSPIN_SetParam(dSPIN_MAX_SPEED, MaxSpdCalc(1000));
  dSPIN_SetParam(dSPIN_FS_SPD, FSCalc(0x3FF));
  dSPIN_SetParam(dSPIN_ACC, 100);
  dSPIN_SetParam(dSPIN_DEC, 100);
  dSPIN_SetParam(dSPIN_OCD_TH, dSPIN_OCD_TH_2250mA);
  dSPIN_SetParam(dSPIN_CONFIG,
                 dSPIN_CONFIG_PWM_DIV_1 |
                 dSPIN_CONFIG_PWM_MUL_2 |
                 dSPIN_CONFIG_SR_180V_us |
                 dSPIN_CONFIG_OC_SD_DISABLE |
                 dSPIN_CONFIG_VS_COMP_DISABLE |
                 dSPIN_CONFIG_SW_HARD_STOP |
                 dSPIN_CONFIG_INT_16MHZ);

  dSPIN_SetParam(dSPIN_KVAL_RUN,  50);
  dSPIN_SetParam(dSPIN_KVAL_ACC, 50);
  dSPIN_SetParam(dSPIN_KVAL_DEC, 50);
  dSPIN_SetParam(dSPIN_KVAL_HOLD, 50);

  dSPIN_GetStatus();

  TCCR1A = 0;
  TCCR1B = B00000110;
  TCNT1 = 0;
}

void loop()
{
  if (Flag == LOW) {
    digitalWrite(ledPin, LOW);
    dSPIN_SoftStop();
  }
  else {
    digitalWrite(ledPin, HIGH);
  }
  Spin = digitalRead(2);
  if (Spin == HIGH)   {
    dSPIN_Step_Clock(FWD);
  }
  else if (Spin == LOW) {
    dSPIN_Step_Clock(REV);
  }
}



