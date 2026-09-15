#define PIN_LED 7

void setup() {
  pinMode(PIN_LED, OUTPUT);
}

void loop() {

  // 1. 처음 1초 동안 LED 켜기
  digitalWrite(PIN_LED, LOW);
  delay(1000);

  // 2. 다음 1초 동안 LED 5회 깜빡이기
  for (int i = 0; i < 5; i++) {
    digitalWrite(PIN_LED, HIGH);
    delay(100);

    digitalWrite(PIN_LED, LOW);
    delay(100);
  }

  // 3. LED 끄기
  digitalWrite(PIN_LED, HIGH);

  // 4. 여기서 무한루프 → 더 이상 실행되지 않음
  while (1) {
  }
}
