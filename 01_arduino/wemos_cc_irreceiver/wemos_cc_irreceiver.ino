#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

const uint16_t RECV_PIN = D6;     // VS1838 OUT → D6
IRrecv irrecv(RECV_PIN);
decode_results results;

char mapNecToDigit(uint64_t v) {
  switch (v) {
    /* 이전값
    case 0xFF6897: return '0';
    case 0xFF30CF: return '1';
    case 0xFF18E7: return '2';
    case 0xFF7A85: return '3';
    case 0xFF10EF: return '4';
    case 0xFF38C7: return '5';
    case 0xFF5AA5: return '6';
    case 0xFF42BD: return '7';
    case 0xFF4AB5: return '8';
    case 0xFF52AD: return '9';
    */
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
    case 0xFF02FD: return 'O';

    default:       return '\0';   // 매칭 없음
  }
}

char lastKey = 0;

void setup() {
  Serial.begin(115200);
  delay(200);
  irrecv.enableIRIn();            // 수신 시작
}

void loop() {
  if (!irrecv.decode(&results)) return;

  if (results.decode_type == NEC) {
    // 길게 누를 때 반복코드는 건너뛰기
    if (results.repeat ||
        results.value == 0xFFFFFFFFULL ||
        results.value == 0xFFFFFFFFFFFFFFFFULL) {
      // 필요하면 여기서 lastKey를 반복 출력해도 됨
    } else {
      char k = mapNecToDigit(results.value);
      if (k) {
        Serial.println(k);        // 예: '9'
        lastKey = k;
      } else {
        // 숫자키가 아니면 원래 코드도 참고용으로 출력
        Serial.print("UNK 0x");
        Serial.println(uint64ToString(results.value, 16));
      }
    }
  }

  irrecv.resume();                // 다음 수신 준비
}
