// /*
//  * The simplest operation using SHT3x
//  */

#include <SHT3x.h>
#include <Arduino.h>
#include <Wire.h>
#include "../include/ER_ML_4IO/IO_Define.h"
#include "ML_Ethernet.h"
#include "ModbusIP_ESP8266.h"
ModbusIP mb;
ML_Ethernet *eth = new ML_Ethernet;
SHT3x Sensor;
typedef enum
{
  IDLE_STATE,
  BTN_POWER,
  BTN_AC
} STATE;
STATE state_ac;
IPAddress local_ip = {192, 168, 31, 110};
IPAddress gateway = {192, 168, 31, 1};
IPAddress subnet = {255, 255, 255, 0};
IPAddress dns1 = {8, 8, 8, 8};
IPAddress dns2 = {8, 8, 4, 4};
bool statePowerAC = false;
unsigned long preTime = 0;
void updateAcState();
void setup()
{
  pinMode(Q1, OUTPUT);
  pinMode(Q2, OUTPUT);
  pinMode(Q3, OUTPUT);
  pinMode(Q4, OUTPUT);

  eth->init(local_ip, gateway, subnet, dns1, dns2);
  Serial.begin(115200);
  Sensor.Begin(32, 33);
  mb.server();
  mb.addCoil(0, 0, 10);
  mb.addHreg(0, 0, 10);
}
void loop()
{
  updateAcState();
  Sensor.UpdateData();
  Serial.print("Temperature: ");
  Serial.print(Sensor.GetTemperature());
  Serial.write("\xC2\xB0"); //The Degree symbol
  Serial.println("C");
  Serial.print("Humidity: ");
  Serial.print(Sensor.GetRelHumidity());
  Serial.println("%");
  mb.Hreg(0, Sensor.GetTemperature() * 10);
  mb.Hreg(1, Sensor.GetRelHumidity() * 10);
  digitalWrite(Q1, mb.Coil(0));
  // digitalWrite(Q2,mb.Coil(1));
  // digitalWrite(Q3,mb.Coil(2));
  // digitalWrite(Q4,mb.Coil(3));

  mb.task();
}
void updateAcState()
{
  static bool flag_power_on = false;
  static unsigned long preTime = millis();
  static bool stateAC = false;
  static bool preCoil1=false;
  static bool preCoil2=false;
  switch (state_ac)
  {
  case IDLE_STATE:
  {
    if (mb.Coil(1) !=preCoil1)
    {
      preCoil1=mb.Coil(1);
      preTime = millis();
      state_ac = BTN_POWER;
    }
 else   if (mb.Coil(2) == true)
    {
      mb.Coil(2, false);
      state_ac = BTN_AC;
      preTime = millis();
    }
    break;
  }
  case BTN_POWER:
  {
    digitalWrite(Q2, true);
    if (millis() - preTime > 500)
    {
      digitalWrite(Q2, false);
      state_ac = BTN_AC;
      preTime=millis();
    }
    break;
  }
  case BTN_AC:
  {
    digitalWrite(Q3, true);
    if (millis() - preTime > 500)
    {
      digitalWrite(Q3, false);
      state_ac = IDLE_STATE;
    }
    break;
  }
  }
}
/* TSL2591 Digital Light Sensor */
/* Dynamic Range: 600M:1 */
/* Maximum Lux: 88K */

