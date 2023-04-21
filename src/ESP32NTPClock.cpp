#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>


const char *ssid = "ssid";
const char *password = "password";

const char TIME_ZONE = 8;
const char SYNC_HOUR = 4;

const char digitPins[] = {21, 19, 18, 5};
const char segPins[] = {32, 33, 25, 26, 27, 14, 12};
const char digits[] = {
  0b11111100, // 0
  0b01100000, // 1
  0b11011010, // 2
  0b11110010, // 3
  0b01100110, // 4
  0b10110110, // 5
  0b10111110, // 6
  0b11100000, // 7
  0b11111110, // 8
  0b11110110  // 9
};

ulong lastUpdateTime;
ulong lastDigitTime = 0;

uint num[4];
uint digitState = 0;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

void displayDigit(uint8_t pin, int num) {
  for (int i = 0; i < 7; i++) {
    digitalWrite(segPins[i], digits[num] << i & 0b10000000);
  }
}

void displayLoop() {
  digitalWrite(segPins[6], LOW);
  for (int i = 0; i < 6; i++) {
    digitalWrite(segPins[i], HIGH);
    delay(100);
    digitalWrite(segPins[i], LOW);
  }
}

void syncTime() {
  for (int i = 0; i < 4; i++) {
    digitalWrite(digitPins[i], LOW);
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    displayLoop();
  }

  timeClient.begin();
  while (!timeClient.update()) {
    displayLoop();
  }
  uint epochTime = timeClient.getEpochTime() + 3600 * TIME_ZONE;
  lastUpdateTime = millis() - epochTime % 60 * 1000;
  int h = epochTime / 3600 % 24;
  int m = epochTime / 60 % 60;
  num[0] = h / 10;
  num[1] = h % 10;
  num[2] = m / 10;
  num[3] = m % 10;

  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

void setup() {
  for (int i = 0; i < 4; i++) {
    pinMode(digitPins[i], OUTPUT);
  }
  for (int i = 0; i < 7; i++) {
    pinMode(segPins[i], OUTPUT);
  }

  syncTime();
}

void loop() {
  ulong now = millis();
  if (now - lastUpdateTime >= 60000) {
    lastUpdateTime += 60000;
    if (++num[3] == 10) {
      num[3] = 0;
      if (++num[2] == 6) {
        num[2] = 0;
        if (++num[1] == (num[0] == 2 ? 4 : 10)) {
          num[1] = 0;
          if (++num[0] == 3) {
            num[0] = 0;
          }
        }
        if (num[0] * 10 + num[1] == SYNC_HOUR) {
          syncTime();
        }
      }
    }
  }

  if (now - lastDigitTime) {
    digitalWrite(digitPins[digitState], HIGH);
    if (++digitState == 4) {
      digitState = 0;
    }
    digitalWrite(digitPins[digitState], LOW);
    displayDigit(digitPins[digitState], num[digitState]);
    lastDigitTime = now;
  }
}
