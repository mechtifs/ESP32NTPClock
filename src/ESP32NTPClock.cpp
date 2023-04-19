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

ulong lastDigitTime = 0;
ulong lastUpdateTime = 0;

uint h;
uint m;
uint num[4];
uint digitState = 0;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

void displayDigit(uint8_t pin, int num) {
  for (int i = 0; i < 7; i++) {
    digitalWrite(segPins[i], (digits[num] << i) & 0b10000000);
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
  uint now = timeClient.getEpochTime() + 3600 * TIME_ZONE;
  lastUpdateTime = millis();
  h = now / 3600 % 24;
  m = now / 60 % 60;
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
    lastUpdateTime = now;
    m = (m + 1) % 60;
    num[2] = m / 10;
    num[3] = m % 10;
    if (!m) {
      h = (h + 1) % 24;
      if (h == SYNC_HOUR) {
        syncTime();
        return;
      }
      num[0] = h / 10;
      num[1] = h % 10;
    }
  }

  if (now - lastDigitTime > 4) {
    digitState = (digitState + 1) % 4;
    digitalWrite(digitPins[(digitState + 3) % 4], HIGH);
    digitalWrite(digitPins[digitState], LOW);
    displayDigit(digitPins[digitState], num[digitState]);
    lastDigitTime = now;
  }
}

