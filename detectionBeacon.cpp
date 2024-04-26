#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEUtils.h>
#include <BLEAdvertisedDevice.h>
#include <WiFi.h>
#include <String>
#include "time.h"
#include  <ctime>
#include <iostream>
#include <HTTPClient.h>
// la classe Scnned Tag:
class SCANNEDTAG
{
  public:
    String  adressMac ;
    String MouvementTime ;
    String pointdepassagescan ;
    SCANNEDTAG(String m, String Mv, String pt)
    {
      adressMac = m ;
      MouvementTime = Mv ;
      pointdepassagescan = pt ;
    }
    String getAdressMac() {
      return adressMac;
    }
   String getMouvementTime() {
      return MouvementTime;
    }
    String getPointdepassagescan() {
      return pointdepassagescan;
    }
    void setAdressMac(String s) {
      adressMac = s;
    }
    void setMouvementTime(String t) {
      MouvementTime = t;
    }
    void setPointdepassagescan(String p) {
      pointdepassagescan = p;
    }
};


BLEScan* scan ;
String pointdepassagescan ;
String BoardAdress;
String chaine;
String MouvementTime;
String adressMac ;
boolean trouve = true ;
int Distance = -80 ;
int sizeScan = 0 ;
const int pinButton = 13;
String httpRequestData = "";
String jsonBeacon;
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600 ;
const char* serverName;
SCANNEDTAG** scanedTag;
// wifi:
void setupWiFi() {
  BoardAdress = WiFi.macAddress();
  const char* ssid = "DESKTOP-2RH2AMI 1983" ;
  const char* password ="994%kB41";
  WiFi.begin( ssid , password );
  while ( WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println(F("connexion au reseau WiFi en cours..."));
  }
  Serial.println(F("connexion au reseau WiFi etablie!"));
  Serial.println(F("Adresse IP :"));
  Serial.println(WiFi.localIP());
}


boolean loopWiFi() {
  if (WiFi.isConnected()) {
    Serial.println(F("Connexion WiFi toujours active!"));
    return true;
  } else {
    Serial.println(F("La connexion WiFi a été perdue!"));
    return false;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(F("webservice"));
    WiFiClient client ;
    HTTPClient http;
    http.begin(client, "localhost:8000");
    http.addHeader( "Content-type", "application/json");
    int httpResponseCode = http.POST(httpRequestData);
    Serial.println(F("httpResponseCode:-------------------------"));
    Serial.println(httpResponseCode);
    http.end();
    if (httpResponseCode == 201) {
      return true;
    } else {
      return false;
    }
  }
}
// TIME:
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASS";
String printLocalTime() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return "Failed to obtain time";
    }


    char buffer[80];
    strftime(buffer, sizeof(buffer), "%A, %B %d %Y %H:%M:%S", &timeinfo);
    return buffer;
}
void setupTIME()
{
  Serial.begin(115200);


  //connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(F("CONNECTED"));


  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);


  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}


/*
void loopTIME()
{
  delay(1000);
  printLocalTime();
}
*/
// projet:
void setup() {




  Serial.begin(9600) ; // initialisation la communication serie pour afficher les messages de detection du beacon
  pinMode(pinButton, OUTPUT);// configuration du pin led en sortie
  scanedTag = new SCANNEDTAG*[5];
  jsonBeacon = "";
  jsonBeacon = "{\"Mouvement\":[";
  Serial.println(F("debut scan wifi"));
  setupWiFi();
  Serial.println(F("Fin scan WIFI"));
  BLEDevice::init("");
  scan = BLEDevice::getScan(); // l objet scan est utilise pour effectuer l operation du balayage BLE
  scan->setActiveScan(true); // si l activation du scan =true on detecte les appareils BLE à proximité
  scan->setInterval(50);
  scan->setWindow(50);
}
void loop() // le programme s'execute dans une boucle infine
{
  int a;
  pointdepassagescan = BoardAdress ;
  scan->setActiveScan(true);
  BLEScanResults results = scan->start(3);
  configTime(gmtOffset_sec , daylightOffset_sec , ntpServer);
  //printLocalTime();
  for (int i = 0; i < results.getCount(); i++) {
     BLEAdvertisedDevice advertisedDevice = results.getDevice(i);
    std::string advertisedDeviceStr = advertisedDevice.toString();
    std::string deviceName = advertisedDevice.getName();
    String Devicename = deviceName.c_str();
    BLEAddress  macAd = advertisedDevice.getAddress();
    String  adressMac = macAd.toString().c_str();
 
    std::string str2 = macAd.toString().substr(0, 8);
      MouvementTime = printLocalTime().c_str();




    int rssi = advertisedDevice.getRSSI();
    if (str2 == "ac:23:3f") {
     
      digitalWrite(pinButton, HIGH); // alumer la LED
      //delay(8000); // la led va s'allumer pendant 8s apres elle s'et
      if (rssi > Distance) {
        if (sizeScan == 0) {
          scanedTag[sizeScan] = new SCANNEDTAG(adressMac,MouvementTime,pointdepassagescan);
          sizeScan++;
   
        }
        else {
                    a = 0;
                    //Serial.print(F("Le point de passage: "));
                 while ( a < sizeScan  && !adressMac.equals(scanedTag[a]->adressMac.c_str())) {
                     a++;
                  }
                  if (a < sizeScan) {
                    trouve = true;
                  }
                  else {
                    trouve = false;
                  }      
          if (trouve == false) {
            Serial.println(adressMac);
            scanedTag[sizeScan] = new SCANNEDTAG(adressMac, MouvementTime, pointdepassagescan);
            sizeScan++;
          }
        }
      }
    }
    for (int i=0; i<sizeScan;i++) {
       
      jsonBeacon = jsonBeacon+"{\"pointdepassagescan\":\"" + scanedTag[i]->getPointdepassagescan()+"\",\"Standard\":\"" + scanedTag[i]->getAdressMac() + "\",\"date\":\"" + scanedTag[i]->getMouvementTime() + "\"},";
    }
    if (sizeScan > 0){
      jsonBeacon = jsonBeacon.substring(0,jsonBeacon.length()-1);
      jsonBeacon = jsonBeacon + "]}";
      Serial.println(jsonBeacon);
         sizeScan=0;
         jsonBeacon = "";
            jsonBeacon = "{\"Mouvement\":[";
    }


    httpRequestData = jsonBeacon;


    /*if (loopWiFi()) {
      scan->clearResults();
      scan = BLEDevice::getScan();
      jsonBeacon = "";
      jsonBeacon = "{\"Mouvement\":[";
      sizeScan=0;
    }*/
   
  }
}
