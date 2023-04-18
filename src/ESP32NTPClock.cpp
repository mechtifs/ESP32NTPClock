#include <WiFi.h>
#include <WiFiUdp.h>
#include <ESP32Time.h>
#include <NTPClient.h>


const char *ssid = "ssid";
const char *password = "password";

const int TIME_ZONE = 8;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
ESP32Time rtc(3600*TIME_ZONE);

uint8_t digitPins[] = {21, 19, 18, 5};
uint8_t segPins[] = {32, 33, 25, 26, 27, 14, 12};
uint8_t digit[][8] = {
  {1, 1, 1, 1, 1, 1, 0},
  {0, 1, 1, 0, 0, 0, 0},
  {1, 1, 0, 1, 1, 0, 1},
  {1, 1, 1, 1, 0, 0, 1},
  {0, 1, 1, 0, 0, 1, 1},
  {1, 0, 1, 1, 0, 1, 1},
  {1, 0, 1, 1, 1, 1, 1},
  {1, 1, 1, 0, 0, 0, 0},
  {1, 1, 1, 1, 1, 1, 1},
  {1, 1, 1, 1, 0, 1, 1}
};

void displayDigit(int pin, int num) {
  for (int i = 0; i < 7; i++) {
    digitalWrite(segPins[i], digit[num][i]);
  }
}

void displayNumber(int num[]) {
  digitalWrite(digitPins[3], HIGH);
  digitalWrite(digitPins[0], LOW);
  displayDigit(digitPins[0], num[0]);
  delay(5);
  digitalWrite(digitPins[0], HIGH);
  digitalWrite(digitPins[1], LOW);
  displayDigit(digitPins[1], num[1]);
  delay(5);
  digitalWrite(digitPins[1], HIGH);
  digitalWrite(digitPins[2], LOW);
  displayDigit(digitPins[2], num[2]);
  delay(5);
  digitalWrite(digitPins[2], HIGH);
  digitalWrite(digitPins[3], LOW);
  displayDigit(digitPins[3], num[3]);
  delay(5);
}

void displayNull() {
  for (int i = 0; i < 6; i++) {
    digitalWrite(segPins[i], LOW);
  }
  digitalWrite(segPins[6], HIGH);
}

void displayLoop() {
  digitalWrite(segPins[6], LOW);
  for (int j = 0; j < 6; j++) {
    digitalWrite(segPins[j], HIGH);
    delay(100);
    digitalWrite(segPins[j], LOW);
  }
}

void setup() {
  for (int i = 0; i < 7; i++) {
    pinMode(digitPins[i], OUTPUT);
  }
  for (int i = 0; i < 7; i++) {
    pinMode(segPins[i], OUTPUT);
  }
  for (int i = 0; i < 4; i++) {
    digitalWrite(digitPins[i], LOW);
  }

  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    displayLoop();
    Serial.print(".");
  }
  displayNull();

  timeClient.begin();
  do {
    timeClient.update();
  } while (timeClient.getEpochTime() == 0);
  rtc.setTime(timeClient.getEpochTime());

  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

void loop() {
  int m = rtc.getMinute();
  int h = rtc.getHour(true);
  int num[] = {h / 10, h % 10, m / 10, m % 10};
  displayNumber(num);
}
