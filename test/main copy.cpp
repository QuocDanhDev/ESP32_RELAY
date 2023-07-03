#include <SHT3x.h>
#include <Arduino.h>
#include <Wire.h>
#include "../include/ER_ML_4IO/IO_Define.h"
#include "ML_Ethernet.h"
#include "ModbusIP_ESP8266.h"
#include "WiFiUdp.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
#include <BH1750.h>
#include <cstring>
TwoWire I2C = TwoWire(0);
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);
BH1750 lightMeter;
WiFiUDP udp;
char packetBuffer[255];
ModbusIP mb;
ML_Ethernet *eth = new ML_Ethernet;
IPAddress local_ip = {192, 168, 31, 120};
IPAddress gateway = {192, 168, 31, 1};
IPAddress subnet = {255, 255, 255, 0};
IPAddress dns1 = {8, 8, 8, 8};
IPAddress dns2 = {8, 8, 4, 4};
unsigned long preTime = 0;
enum STATE
{
    IDLE_STATE,
    START_MESURE,
    STOP_MEASURE,
    SEND_MESURE
};
bool start =false;
STATE state;
void updateState();
void setup()
{
    Serial.begin(115200);
    eth->init(local_ip, gateway, subnet, dns1, dns2);
    I2C.begin(16, 4);
    lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23, &I2C);
    udp.begin(5000);
}
void loop()
{
    int packetSize = udp.parsePacket();
    // float lux = lightMeter.readLightLevel();
    if (packetSize)
    {
        int len = udp.read(packetBuffer, 255);

        if (len > 0)
        {
            packetBuffer[len] = 0;
            Serial.println(packetBuffer);
            
            if(strcmp(packetBuffer,"start\n")==0){
            start=true;
            }
            else if(strcmp(packetBuffer,"stop\n")==0){
            start=false;
            state=IDLE_STATE;
            }
        }
    }
    updateState();
}
void updateState()
{
    static unsigned long preTime=0;
    static unsigned long preTimeCount=0;
    switch (state)
    {
    case IDLE_STATE:
    {
        if(start){
            state=START_MESURE;
            Serial.println("Start measurement");
            preTime=millis();
            preTimeCount=millis();
        }
        break;
    }
    case START_MESURE:
    {
        if(millis()-preTime>=5){
                    float lux = lightMeter.readLightLevel();
            udp.println("C;"+(String)lux+";"+(String)(millis()-preTimeCount));
            preTime=millis();
        }
        break;
    }
    case STOP_MEASURE:
    {
        break;
    }
    case SEND_MESURE:
    {
        break;
    }
    }
}