#include <ESP8266WiFi.h>

const char* ssid = "SK_WiFiGIGA3BA0_2.4G";
const char* password = "KAP23@0405";

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  Serial.print("WiFi 연결중");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi 연결 완료!");
  Serial.print("IP 주소: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // 네트워크 연결 후 로직 작성
}
