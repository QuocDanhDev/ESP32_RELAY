#include <Arduino.h>
#include "ML_Ethernet.h"
#include <ModbusIP_ESP8266.h>

const int relayPins[] = {12, 14, 33, 16, 5, 4, 2, 15};
const int numRelays = sizeof(relayPins) / sizeof(relayPins[0]);

ML_Ethernet eth;
ModbusIP mb;
void setup()
{

  for (int i = 0; i < numRelays; i++)
  {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], HIGH);
  }

  Serial.begin(115200);

  // Khởi tạo Ethernet
  eth.init();
  mb.server();
  mb.addHreg(0, 0, 16);
  mb.addCoil(0, false, 10);
}

void loop()
{
  static unsigned long pretime = millis();
  if ((millis() - pretime >= 1000))
  {
    // bật tắt relay
    for (int i = 0; i < 8; i++)
    {
      if (mb.Coil(i) == 1)
      {
        digitalWrite(relayPins[i], LOW);
      }
      else
      {
        digitalWrite(relayPins[i], HIGH);
      }
    }
    if (mb.Coil(8) == 1)
    {
      for (int i = 0; i < numRelays; i++)
      {
        digitalWrite(relayPins[0], LOW);
        mb.Coil(i);
      }
    }
    else
    {
      for (int i = 0; i < numRelays; i++)
      {
        digitalWrite(relayPins[numRelays], HIGH);
        mb.Coil(i);
      }
    }
    // hẹn giờ relay
    for (int i = 0; i < 8; i++)
    {
      int value = mb.Hreg(i);
      static unsigned long startTime = millis();
      static unsigned long interval = 1000;

      if (value >= 1)
      {
        digitalWrite(relayPins[i], LOW);

        while (value > 1)
        {
          static unsigned long currentMillis = millis();
          if (currentMillis - startTime >= interval)
          {
            startTime = currentMillis;
            value--;
            mb.Hreg(i, value);
          }
        }
      }

      pretime = millis();
    }
    mb.task();
  }
}