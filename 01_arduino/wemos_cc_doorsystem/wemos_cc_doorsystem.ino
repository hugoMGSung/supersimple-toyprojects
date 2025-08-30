#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <Servo.h>

// ---------- 핀 설정 ----------
const uint16_t IR_RECV_PIN = D6;   // VS1838 OUT
const uint8_t  SERVO_PIN   = D5;   // 서보 Signal
const uint8_t  BUZZ_PIN    = D7;   // 부저 Signal

// ---------- 부저 설정 ----------
// 모듈 극성: Active-LOW면 1, Active-HIGH면 0
#define BUZZ_ACTIVE_LOW 1
// 부저 타입: Passive(톤 필요)=1, Active(내장발진 ON/OFF)=0
#define PASSIVE_BUZZER  1

// ---------- 서보 파라미터 ----------
const int SERVO_MIN_US = 500;
const int SERVO_MAX_US = 2500;
const int SERVO_CENTER = 90;
const int SERVO_RIGHT  = 180;      // 성공: 오른쪽 유지
const int SERVO_LEFT   = 0;        // 실패: 왼쪽 유지

IRrecv irrecv(IR_RECV_PIN);
decode_results results;
Servo servo;
int servoPos = SERVO_CENTER;

// ---------- 비밀번호/입력 버퍼 ----------
String secret;                       // 부팅 시 랜덤 6자리
String entered = "";
unsigned long lastKeyMs = 0;
const unsigned long CLEAR_TIMEOUT_MS = 8000;

// ---------- NEC 코드 → 키 매핑 (사용자 테이블) ----------
char mapNecToKey(uint64_t v) {
  switch (v) {
    case 0xFF6897: return '1';
    case 0xFF9867: return '2';
    case 0xFFB04F: return '3';
    case 0xFF30CF: return '4';
    case 0xFF18E7: return '5';
    case 0xFF7A85: return '6';
    case 0xFF10EF: return '7';
    case 0xFF38C7: return '8';
    case 0xFF5AA5: return '9';
    case 0xFF42BD: return '*';
    case 0xFF4AB5: return '0';
    case 0xFF52AD: return '#';
    case 0xFF629D: return 'U';
    case 0xFFA857: return 'D';
    case 0xFF22DD: return 'L';
    case 0xFFC23D: return 'R';
    case 0xFF02FD: return 'O'; // OK
    default:       return '\0';
  }
}

// ---------- 랜덤 6자리 생성 ----------
String makeCode(uint8_t n = 6) {
  String s; s.reserve(n);
  for (uint8_t i = 0; i < n; i++) s += char('0' + random(10));
  return s;
}

// ---------- 부저 유틸 (극성 자동 반영) ----------
inline void buzzOff() { digitalWrite(BUZZ_PIN, BUZZ_ACTIVE_LOW ? HIGH : LOW); }
inline void buzzOn()  { digitalWrite(BUZZ_PIN, BUZZ_ACTIVE_LOW ? LOW  : HIGH); }

#if PASSIVE_BUZZER
  void beep(unsigned freq=2000, unsigned dur=120) {
    buzzOff();
    tone(BUZZ_PIN, freq);
    delay(dur);
    noTone(BUZZ_PIN);
    buzzOff();
  }
  void beepClick()   { beep(1400, 30); }
  void beepSuccess() { beep(2000,120); delay(40); beep(2600,140); }
  void beepFail()    { beep(400,180); }
#else
  void beepOnce(unsigned dur=120) { buzzOn(); delay(dur); buzzOff(); }
  void beepClick()   { beepOnce(30); }
  void beepSuccess() { beepOnce(120); delay(40); beepOnce(140); }
  void beepFail()    { beepOnce(180); }
#endif

// ---------- 서보 스무스 이동 ----------
void moveServoTo(int target, int step=2, int dly=15) {
  target = constrain(target, 0, 180);
  if (target > servoPos) {
    for (int p = servoPos; p <= target; p += step) { servo.write(p); delay(dly); }
  } else {
    for (int p = servoPos; p >= target; p -= step) { servo.write(p); delay(dly); }
  }
  servoPos = target;
}

// ---------- 입력 버퍼 초기화 ----------
void resetInput(const char* reason) {
  if (entered.length()) {
    Serial.print("[CLEAR] "); Serial.print(reason);
    Serial.print(" (entered: "); Serial.print(entered); Serial.println(")");
  }
  entered = "";
}

void setup() {
  Serial.begin(115200);
  delay(200);

  // IR 시작
  irrecv.enableIRIn();

  // 부저: 부팅 직후 OFF 강제 (삐— 방지)
  pinMode(BUZZ_PIN, OUTPUT);
  buzzOff();
  noTone(BUZZ_PIN);

  // 서보 준비
  delay(1200);
  servo.attach(SERVO_PIN, SERVO_MIN_US, SERVO_MAX_US);
  servo.write(SERVO_CENTER);

  // 랜덤 시드 & 6자리 비밀번호 생성
  randomSeed(ESP.getChipId() ^ micros() ^ analogRead(A0));
  secret = makeCode(6);

  Serial.println("==== SMART LOCK (IR + SERVO + BUZZER) ====");
  Serial.print ("PASSCODE: "); Serial.println(secret);    // ← 시리얼 모니터로 확인
  Serial.println("Enter 6 digits then OK. PASS→Right(hold), FAIL→Left(hold).");
}

void loop() {
  // 입력 타임아웃(버퍼만 초기화, 각도 유지)
  if (entered.length() && millis() - lastKeyMs > CLEAR_TIMEOUT_MS) {
    resetInput("timeout");
  }

  if (!irrecv.decode(&results)) return;

  if (results.decode_type == NEC) {
    // 길게 누름(리핏) 무시
    if (results.repeat ||
        results.value == 0xFFFFFFFFULL ||
        results.value == 0xFFFFFFFFFFFFFFFFULL) {
      irrecv.resume();
      return;
    }

    char k = mapNecToKey(results.value);
    if (k == '\0') {
      Serial.print("UNK 0x"); Serial.println(uint64ToString(results.value, 16));
    } else if (k >= '0' && k <= '9') {
      lastKeyMs = millis();
      if (entered.length() < 6) entered += k;     // 6자리만 받기
      Serial.print("KEY: "); Serial.print(k);
      Serial.print("   buffer: "); Serial.println(entered);
      beepClick();
    } else if (k == '*') {
      lastKeyMs = millis();
      if (entered.length()) entered.remove(entered.length() - 1);
      Serial.print("BACKSPACE -> "); Serial.println(entered);
      beepFail();
    } else if (k == '#') {
      lastKeyMs = millis();
      resetInput("# pressed");   // 각도 유지
      beepClick();
      // 필요하면 여기서 현재 비번 다시 보여주기 가능:
      // Serial.print("PASSCODE: "); Serial.println(secret);
    } else if (k == 'O') { // OK
      Serial.print("OK pressed. code="); Serial.println(entered);

      if (entered.length() == 6 && entered == secret) {
        Serial.println("PASS! Servo → Right (HOLD 180°)");
        beepSuccess();
        moveServoTo(SERVO_RIGHT);   // 유지
      } else {
        Serial.print("FAIL! Expect "); Serial.println(secret);
        beepFail();
        moveServoTo(SERVO_LEFT);    // 유지
      }
      resetInput("OK handled");
    }
  }

  irrecv.resume();
}
