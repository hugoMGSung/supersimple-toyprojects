// 기본 LED Blink (보드 내장 LED는 D4 핀 = GPIO2에 연결됨)
void setup() {
  pinMode(D4, OUTPUT); // 보드 내장 LED 핀
}

void loop() {
  digitalWrite(D4, LOW);   // LED ON (ESP8266은 LOW가 켜짐)
  delay(1000);
  digitalWrite(D4, HIGH);  // LED OFF
  delay(1000);
}