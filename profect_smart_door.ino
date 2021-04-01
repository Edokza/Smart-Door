#include <ESP8266WiFi.h>
#include <time.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "SPI.h"
#include "MFRC522.h"
#include <Servo.h>
#include <ESP8266HTTPClient.h>

//set wifi
#define ssid "kkkk"
#define passw "11111111" 

//set pin ให้ RFID
#define SS_PIN D10
#define RST_PIN D9
#define SP_PIN D8

//set เวลาของหมุนมอเตอร์
#define TURN_TIME 1070

//ประกาศตัวแปร motor
Servo myservo;

//ประกาศตัวแปรlcd
LiquidCrystal_I2C lcd(0x27, 16, 2);

//ประกาศตัวแปรของRFID
MFRC522 rfid(SS_PIN, RST_PIN);

MFRC522::MIFARE_Key key;


// Config time
int timezone = 7;       // Zone +7 for Thailand
char ntp_server1[20] = "ntp.ku.ac.th";
char ntp_server2[20] = "fw.eng.ku.ac.th";
char ntp_server3[20] = "time.uni.net.th";
int dst = 0;
int  Sec = 0;

//ตัวแแปรstringเก็บค่าเวลา
String tmpNow;
String dateNow;
String dateTmpNow;
//ตัวแปรเก็บและค่าID
String strID = "";

//แปลงIDเก็บในstring
String NowString();
//แสดงผลlcd
void showlcd();
//แปลงIDเป็นstring
String uidToStr();
//เปิดประตู
void openDoor();
//แสดงผลlcd2
void showlcd2();
//put data ลง firebase
void updateLogs();

void setup() {
  Serial.begin(115200);
  
  /*---- Config WiFi ----*/
  WiFi.begin(ssid ,passw );

  /*---- Config Time ----*/
  configTime(timezone * 3600, dst, ntp_server1, ntp_server2, ntp_server3);
  while (!time(nullptr)) {
    Serial.print(".");
    delay(500);
    lcd.begin();

  // Turn on the blacklight and print a message.
  lcd.backlight();

  SPI.begin();
  rfid.PCD_Init();
  
  myservo.attach(D8);
  // Initially the servo must be stopped 
  myservo.write(90);
  
  }  


}

void loop() {

  NowString();
  showlcd();
  delay(1000);
  
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial())
    return;

  // Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  // Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }


  uidToStr();
  Serial.println(strID);

  if(strID == "4A:00:01:85" || "EA:5A:9F:1F"){
      Serial.println("Correct id card");
      updateLogs();
      showlcd2();
      openDoor();
    }
   else {
      Serial.println("Not correct id card");
    }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  
}

String NowString() {
  //reset ค่าตัวแปร
  tmpNow = ""; 
  dateNow = "";
  dateTmpNow = "";

  //ตัวแปรเก็บค่าเวลาและตัวชี้
  time_t now = time(nullptr);
  struct tm* newtime = localtime(&now);

  dateNow += String(newtime->tm_mday);
  dateNow += "-";
  dateNow += String(newtime->tm_mon + 1);
  dateNow += "-";
  dateNow += String(newtime->tm_year + 1900);
  
  tmpNow += String(newtime->tm_hour);
  tmpNow += ":";
  tmpNow += String(newtime->tm_min);
  tmpNow += ":";
  tmpNow += String(newtime->tm_sec);

  dateTmpNow = dateNow + " " + tmpNow;
  Serial.println(dateTmpNow);
  return dateTmpNow;

}


void showlcd(){
  //reset lcd
  lcd.clear();
  //set ตำแหน่งlcd แล้วแสดงผล
  lcd.setCursor(4,0);
  lcd.print(tmpNow);
  lcd.setCursor(1,1);
  lcd.print("Tag your card:");
  }

 String uidToStr(){
  //reset ค่าID
  strID = "";
  //loop เก็บค่าID
  for (byte i = 0; i < 4; i++) {
    strID +=
    (rfid.uid.uidByte[i] < 0x10 ? "0" : "") +
    String(rfid.uid.uidByte[i], HEX) +
    (i!=3 ? ":" : "");
  }
  strID.toUpperCase();
  return strID;
  }

void openDoor(){
  // Start turning clockwise
    myservo.write(80);
    // Go on turning for the right duration
    delay(TURN_TIME);
    // Stop turning
    myservo.write(90);
    delay(5000);
    myservo.write(100);
    // Go on turning for the right duration
    delay(TURN_TIME);
    // Stop turning
    myservo.write(90);
}

void showlcd2(){
  lcd.clear();
  lcd.setCursor(4,1);
  lcd.print(tmpNow);
  lcd.setCursor(2,0);
  lcd.print("Check in at:");
  }

void updateLogs(){
   HTTPClient http; //Declare object of class HTTPClient
   http.begin("http://bakamitai.herokuapp.com/data/write"); //Specify request destination
   http.addHeader("Content-Type", "application/json"); //Specify content-type header
   int httpCode = http.POST("{\"date\":\"" + dateNow + "\",\"time\":\"" + tmpNow + "\"}"); //Send the request
   String payload = http.getString(); //Get the response payload
   Serial.println(httpCode); //Print HTTP return code
   Serial.println(payload); //Print request response payload
   http.end(); //Close connection
}
