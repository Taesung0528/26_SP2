#define PIN_LED 7

unsigned int period = 10000;  // PWM 주기 (us)
int duty = 0;                 // Duty ratio (0~100)

// PWM 주기 설정
void set_period(int new_period) {
  if (new_period >= 100 && new_period <= 10000) {
    period = new_period;
  }
}

// Duty 설정
void set_duty(int new_duty) {
  if (new_duty >= 0 && new_duty <= 100) {
    duty = new_duty;
  }
}

// PWM 한 주기 발생
void pwm() {
  unsigned long high_time;
  unsigned long low_time;

  high_time = (unsigned long)period * duty / 100;
  low_time = period - high_time;

  if (duty == 0) {
    digitalWrite(PIN_LED, LOW);
    delayMicroseconds(period);
  }
  else if (duty == 100) {
    digitalWrite(PIN_LED, HIGH);
    delayMicroseconds(period);
  }
  else {
    digitalWrite(PIN_LED, HIGH);
    delayMicroseconds(high_time);

    digitalWrite(PIN_LED, LOW);
    delayMicroseconds(low_time);
  }
}

void setup() {
  pinMode(PIN_LED, OUTPUT);

  // 주기 값 변경 직접 입력 필요
  set_period();  
  // set_period(10000);  // 10 ms
  // set_period(1000);  // 1 ms
  // set_period(100);   // 0.1 ms
}

void loop() {

  unsigned long start_time = millis();

  // 약 1초 동안 Triangle 패턴 반복
  while (millis() - start_time < 1000) {

    unsigned long elapsed = millis() - start_time;

    // 0 ~ 500ms : duty 0 → 100
    if (elapsed < 500) {
      set_duty(elapsed / 5);
    }

    // 500 ~ 1000ms : duty 100 → 0
    else {
      set_duty((1000 - elapsed) / 5);
    }

    pwm();
  }
}
