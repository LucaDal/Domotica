#include <Wire.h>

#include <ESP8266HTTPClient.h>
HTTPClient http;
WiFiClient client;

#include "DFRobot_SHT20.h"
DFRobot_SHT20 sht20;

#include <EEPROM.h>

#include "DHT.h"
#define DHTTYPE DHT11
#define DHTPIN 3

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 32  // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT);


#define MOTOR 1
DHT dht(DHTPIN, DHTTYPE);

#include "RTClib.h"
char daysOfTheWeek[7][12] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
RTC_DS1307 rtc;

#include <ESP8266WiFi.h>
const char *ssid = "Vodafone-A82448034";
const char *password = "2piedinidimoira";

int umid_to_water = 0;
int ml_to_give = 0;
int millisec_to_adjust_water = 0;
int ora_a = 0;
int min_a = 0;

int min_sent_data;
bool data_sent = false;
int minToSetFleg;

String HA;
String TA;
int HT;
String TT;

bool annaffiato = true;
float millisWateringTime;
unsigned long timer = 0;
bool check = false;
bool checkAfterMilliliters = true;
bool waitToCheck = false;

unsigned long previousMillisData = 0;
const unsigned long intervalData = 4000;
unsigned long previousMillisDisplay = 0;
unsigned long intervalDisplay = 4000;
bool displayState = false;
bool motorState = false;
bool dataIsRead = false;


void setup() {

  // inizializzo i sensori:
  Wire.begin(2, 0);
  WiFi.begin(ssid, password);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  sht20.initSHT20();
  delay(100);
  sht20.checkSHT20();
  EEPROM.begin(512);


  //GPIO 1 (TX) swap the pin to a GPIO.
  pinMode(MOTOR, FUNCTION_3);
  //GPIO 3 (RX) swap the pin to a GPIO.
  pinMode(DHTPIN, FUNCTION_3);
  //GPIO 1 (TX) swap the pin to a TX.
  //pinMode(1, FUNCTION_0);
  //GPIO 3 (RX) swap the pin to a RX.
  //pinMode(3, FUNCTION_0);
  pinMode(MOTOR, OUTPUT);

  dht.begin();
  if (!rtc.begin()) {
    display.clearDisplay();
    printOnScreen("RTC error", 2, 0, 0);
    delay(2000);
  }

  int cont = 20;
  while ((WiFi.status() != WL_CONNECTED) || cont == 0) {
    display.clearDisplay();
    printOnScreen("Connection...", 2, 0, 0);
    delay(500);
    cont--;
  }
  if (WiFi.status() == WL_CONNECTED) {
    display.clearDisplay();
    printOnScreen("Connected", 2, 0, 0);
    delay(1000);
    askData();
  } else {
    display.clearDisplay();
    printOnScreen("Error with WiFi", 1, 0, 0);
    delay(2000);
  }
  //Leggo i parametri dal sito
  readData();
  millisWateringTime = ((ml_to_give * 200) / 5) + millisec_to_adjust_water;  // 20000/500 millesimi/ml -- ci ha messo 20 secondi per fare mezzo litro


  display.clearDisplay();
  printOnScreen((String)umid_to_water, 2, 0, 0);
  printOnScreen("%", 2, 30, 0);
  printOnScreen((String)ml_to_give, 2, 50, 0);
  printOnScreen((String)millisec_to_adjust_water, 2, 0, 16);
  printOnScreen(",", 2, 28, 16);
  printOnScreen((String)ora_a, 2, 38, 16);
  printOnScreen(":", 2, 66, 16);
  printOnScreen((String)min_a, 2, 70, 16);

  //Messaggio di benvenuto----------------

  delay(3000);
  display.clearDisplay();
  printOnScreen("Welcome", 2, 0, 0);
  display.clearDisplay();

  //inizializzo le variabili;-------------
  HA = (String)((int)dht.readHumidity());
  TA = (String)dht.readTemperature();
  HT = ((int)sht20.readHumidity());
  TT = (String)sht20.readTemperature();
  dataIsRead = true;
  delay(1000);
}


void printOnScreen(String text, int sizeText, int x, int y) {
  display.setTextSize(sizeText);
  display.setTextColor(WHITE);
  display.setCursor(x, y);
  display.println(text);
  display.display();
}


void sendDataToSite(String ora, String minn, String temp_aria, String temp_terreno, String umid_aria, String umid_terreno) {
  if ((WiFi.status() == WL_CONNECTED)) {
    Serial.print("[HTTPS] begin...\n");
    http.begin(client, "http://dalessandroluca.altervista.org/Projects/sentFromPlant.php?device_name=oJd4K&ora=" + ora + "&min=" + minn + "&temp_aria=" + temp_aria + "&temp_terreno=" + temp_terreno + "&umid_aria=" + umid_aria + "&umid_terreno=" + umid_terreno);
    http.GET();
    http.end();
  }
}


int getValureFromEEPROM(int *index) {
  char temp[] = "      ";
  for (int i = 0; i < 5; i++) {
    temp[i] = EEPROM.read(++(*index));
  }
  temp[5] = '\0';
  return atoi(temp);
}


void readData(void) {
  Serial.printf("leggo da EEPROM\n");
  int index = 0;
  umid_to_water = EEPROM.read(index);
  ml_to_give = getValureFromEEPROM(&index);
  millisec_to_adjust_water = getValureFromEEPROM(&index);
  ora_a = EEPROM.read(++index);
  min_a = EEPROM.read(++index);
}


void writeStringToEEPROM(int *index, String str, bool *flag) {

  for (unsigned int i = 0; i < 5 - str.length(); i++) {  //max value is 99999
    if ('0' != EEPROM.read(++(*index))) {
      EEPROM.write(*index, '0');
      *flag = true;
    }
  }
  for (unsigned int i = 0; i < str.length(); i++) {
    if (str.charAt(i) != EEPROM.read(++(*index))) {
      EEPROM.write(*index, str.charAt(i));
      *flag = true;
    }
  }
}


void updateData(int *umid, int *ml, int *sec, int *oraa, int *mina) {
  bool flag = false;
  int index = 0;
  if (*umid != EEPROM.read(index)) {
    EEPROM.write(index, *umid);
    Serial.print("scrivo :umid\n");
    flag = true;
  }
  writeStringToEEPROM(&index, (String)*ml, &flag);
  writeStringToEEPROM(&index, (String)*sec, &flag);
  if (*oraa != EEPROM.read(++index)) {
    EEPROM.write(index, *oraa);
    flag = true;
  }
  if (*mina != EEPROM.read(++index)) {
    EEPROM.write(index, *mina);
    flag = true;
  }
  if (flag) {
    //Serial.print("Scrivo su eprom...\n");
    EEPROM.commit();
  }
}


String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}


void askData(void) {
  if ((WiFi.status() == WL_CONNECTED)) {
    //Serial.print("[HTTPS] begin...\n");
    if (http.begin(client, "http://dalessandroluca.altervista.org/Projects/requestFromPlant.php?device_name=oJd4K")) {
      //Serial.print("[HTTP] GET...\n");
      int httpCode = http.GET();
      if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          int umid = getValue(payload, ';', 0).toInt();
          int ml = getValue(payload, ';', 1).toInt();
          int sec = getValue(payload, ';', 2).toInt();
          int oraa = getValue(payload, ';', 3).toInt();
          int mina = getValue(payload, ';', 4).toInt();
          //Serial.printf("letto: umid: %i,ml: %i,sec: %i,oraa: %i,mina: %i\n",umid,ml,sec,oraa,mina );
          updateData(&umid, &ml, &sec, &oraa, &mina);
        }
      }
    } else {
      display.clearDisplay();
      printOnScreen("Errore ricezzione dati", 1, 0, 0);
      delay(200);
    }
    http.end();
  }
}


void loop() {
  DateTime now = rtc.now();
  int hourInt = now.hour();
  String hours = String(hourInt);
  int minInt = now.minute();
  String minss = String(minInt);
  unsigned long currentMillis = millis();

  //READING---------------------------------------------------------------
  if (currentMillis - previousMillisData >= intervalData) {
    previousMillisData = currentMillis;
    if (!dataIsRead) {
      HA = (String)((int)dht.readHumidity());
      TA = (String)dht.readTemperature();
      HT = ((int)sht20.readHumidity());
      TT = (String)sht20.readTemperature();
      dataIsRead = true;
    } else {
      dataIsRead = false;
    }
  }


  //RIDEFINISCO L'OUTPUT--------------------------------------------------
  if (hourInt < 10) {
    hours = "0" + hours;
  }
  if (minInt < 10) {
    minss = "0" + minss;
  }
  if (HT > 100) {
    HT = 100;
  }


  // INVIO DATI------------------------------------------------------------
  if (data_sent == false && (minInt == 30 || minInt == 0)) {
    data_sent = true;
    minToSetFleg = minInt;
    if(TT.toInt() <= 100 && hourInt <= 24){
      sendDataToSite(hours, minss, TA, TT, HA, (String)HT);
      delay(50);
    }
  }
  if (minInt != minToSetFleg) {
    data_sent = false;
  }


  //IRRIGAZIONE -----------------------------------------------------------
  // if millilitri = 0 -> automatico
  if (!annaffiato) {
   // if ((hourInt >= ora_a) && (hourInt <= (ora_a + 10))) {
      if (currentMillis - timer >= millisWateringTime) {
        timer = currentMillis;
        if (!motorState) {
          digitalWrite(MOTOR, HIGH);
          motorState = true;
        } else {
          digitalWrite(MOTOR, LOW);
          motorState = false;
          waitToCheck = true;
          annaffiato = true;
          //mi server per il controllo sotto, altrimenti entra subito nel timer;
        }
      }
   // }
  }
  //controllo HT dopo tot tempo altrimenti torna falso dopo l'irrigazione a millilitri

  if (waitToCheck == false) {
    if (HT <= umid_to_water) {
      annaffiato = false;
    }
  } else {
    if (currentMillis - timer >= 120000) {  //due minuti  di attesa
      waitToCheck = false;
    }
  }

  //DISPLAY----------------------------------------------------------------
  if (currentMillis - previousMillisDisplay >= intervalDisplay) {
    previousMillisDisplay = currentMillis;
    if (!displayState) {
      //display.clearDisplay();
      printOnScreen(HA, 2, 0, 0);
      printOnScreen("%", 2, 33, 0);
      printOnScreen(TA, 2, 60, 0);
      printOnScreen((String)HT, 2, 0, 16);
      printOnScreen("%", 2, 33, 16);
      printOnScreen(TT, 2, 60, 16);
      displayState = true;
      intervalDisplay -= 3000; //display will last 1 second
      
    } else {
      display.clearDisplay();
      display.display();
      intervalDisplay += 3000; //black display will last 4 second
      displayState = false;
    }
  }
}