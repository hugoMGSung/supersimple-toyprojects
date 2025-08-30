#include <Servo.h>  // 또는 ServoESP8266

Servo servo;

const int SERVO_PIN = D5; // GPIO14

void setup() {
  // 부팅 직후 서보가 튀지 않도록 약간 기다린 뒤 attach
  delay(1500);

  // MG945는 대개 1000~2000us가 0~180도 범위.
  // 여유를 위해 500~2500us로 attach하고 중앙으로 이동
  servo.attach(SERVO_PIN, 500, 2500);
  servo.write(90); // 중앙
}

void loop() {
  // 0 → 180 → 0 천천히 스윕
  for (int pos = 0; pos <= 180; pos += 2) {
    servo.write(pos);
    delay(15);
  }
  for (int pos = 180; pos >= 0; pos -= 2) {
    servo.write(pos);
    delay(15);
  }
}