#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <Wire.h>
#include "SSD1306.h"


static int T, H, iMaxT, iMinT, iMaxH, iMinH;

static BLEUUID serviceUUID("0000181a-0000-1000-8000-00805f9b34fb"); //Service
//std::string VD_BLE_Name = "ATC_E5A275";
std::string service_data;
char Scanned_BLE_Name[32];
String Scanned_BLE_Address;
BLEScanResults foundDevices;
BLEScan *pBLEScan;
static BLEAddress *Server_BLE_Address;
SSD1306  display(0x3c, 5, 4);

void Write1Byte( uint8_t Addr ,  uint8_t Data )
{
    Wire1.beginTransmission(0x34);
    Wire1.write(Addr);
    Wire1.write(Data);
    Wire1.endTransmission();
}

uint8_t Read8bit( uint8_t Addr )
{
    Wire1.beginTransmission(0x34);
    Wire1.write(Addr);
    Wire1.endTransmission();
    Wire1.requestFrom(0x34, 1);
    return Wire1.read();
}


void lightSleep(uint64_t time_in_us)
{
  esp_sleep_enable_timer_wakeup(time_in_us);
  esp_light_sleep_start();
}

// Called for each device found during a BLE scan by the client
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice) {
//      Serial.printf("Scan Result: %s \n", advertisedDevice.toString().c_str());
      const char *szAddr = advertisedDevice.getAddress().toString().c_str();
      // Serial.printf("MAC: %s\n",szAddr);
      if (advertisedDevice.haveServiceData() && memcmp(szAddr, "a4:c1:38", 8) == 0) {
          Serial.printf("MAC: %s\n",szAddr);
          const char *s = service_data.c_str();
          int i, iLen = service_data.length();
          uint8_t *p = (uint8_t *)s; // unsigned data
          service_data = advertisedDevice.getServiceData();
          // Serial.println(p[6]);
          // Serial.println(p[7]);
          // Serial.println(p[8]);
          T = (p[6] << 8) + p[7];
          H = p[8];
          if (T > iMaxT) iMaxT = T;
          if (T < iMinT) iMinT = T;
          if (H > iMaxH) iMaxH = H;
          if (H < iMinH) iMinH = H;
//          Serial.printf("Temp = %d.%dC, Humidity = %d%%\n", T/10, T % 10, p[8]);
        }
    }
};

void ShowInfo(void)
{
char szTemp[64];
  sprintf(szTemp, "Temp %d.%01d, Humid %d%%", T/10, T % 10,  H);
  Serial.println(szTemp);
  // display.setCursor(0,0);  //over,down
  display.drawString(0,0, szTemp);
 
  sprintf(szTemp, "MaxT %d.%01d, MaxH %d%%", iMaxT/10, iMaxT % 10, iMaxH);
  // display.setCursor(0,10);  //over,down
  display.drawString(0,10,szTemp);
  
  sprintf(szTemp, "MinT %d.%01d, MinH %d%%", iMinT/10, iMinT % 10, iMinH);
  // display.setCursor(0,20);  //over,down
  display.drawString(0,20, szTemp);

  //Serial.print(szTemp);
  //display.setCursor(0,0);  //over,down
  //display.print(szTemp);
  //display.setCursor(0,10);  //over,down
  //display.print(szTemp);
  
  display.display();
  delay(1000);
  display.clear();
    
}


void setup() {
  // put your setup code here, to run once:

  display.init();
  display.flipScreenVertically();

  
  iMaxT = 0;
  iMinT = 1000;
  iMaxH = 0;
  iMinH = 99;
  Serial.begin(115200);
  Serial.println("About to start BLE");
  BLEDevice::init("ESP32BLE");
  pBLEScan = BLEDevice::getScan(); //create new scan
  Serial.println("getScan returned");
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks()); //Call the class that is defined above
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
}

void loop() {
  // put your main code here, to run repeatedly:

   foundDevices = pBLEScan->start(5, false); //Scan for 5 seconds to find the Fitness band
   pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
   pBLEScan->stop();
   ShowInfo();
   lightSleep(10000000); // wait 10 seconds, then start another scan
}