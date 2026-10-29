#include <Servo.h>

// Arduino pin assignment
#define PIN_IR    A0
#define PIN_LED   9
#define PIN_SERVO 10

#define _DUTY_MIN 500   // 0 degrees
#define _DUTY_MAX 2400  // 180 degrees
#define _DUTY_NEU ((_DUTY_MIN + _DUTY_MAX) / 2)

#define _DIST_MIN 100.0f  // mm (10 cm)
#define _DIST_MAX 250.0f  // mm (25 cm)

#define EMA_ALPHA 0.20f
#define LOOP_INTERVAL 20UL  // milliseconds

Servo myservo;
unsigned long last_loop_time = 0;

// Start at the midpoint to match the initial 90-degree servo position.
float dist_prev = (_DIST_MIN + _DIST_MAX) / 2.0f;
float dist_ema = (_DIST_MIN + _DIST_MAX) / 2.0f;

void setup()
{
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  myservo.attach(PIN_SERVO);
  myservo.writeMicroseconds(_DUTY_NEU);

  Serial.begin(1000000);
  last_loop_time = millis();
}

void loop()
{
  unsigned long time_curr = millis();
  if (time_curr - last_loop_time < LOOP_INTERVAL)
    return;
  last_loop_time = time_curr;

  float a_value = analogRead(PIN_IR);
  float dist_raw = -1.0f;  // Invalid reading marker for the serial plotter.

  // Avoid division by zero and invalid sensor readings.
  if (a_value > 9.0f)
    dist_raw = (6762.0 / (a_value - 9.0) - 4.0) * 10.0 - 60.0;

  // Range filter: keep the last valid distance for out-of-range readings.
  bool in_range = (dist_raw >= _DIST_MIN && dist_raw <= _DIST_MAX);
  digitalWrite(PIN_LED, in_range ? HIGH : LOW);
  if (in_range)
    dist_prev = dist_raw;

  // EMA: larger alpha responds faster; smaller alpha smooths more.
  dist_ema = EMA_ALPHA * dist_prev + (1.0f - EMA_ALPHA) * dist_ema;
  dist_ema = constrain(dist_ema, _DIST_MIN, _DIST_MAX);

  // Linear conversion without map(): 100 mm -> 0 deg, 250 mm -> 180 deg.
  int duty = (int)(_DUTY_MIN
      + (dist_ema - _DIST_MIN) * (_DUTY_MAX - _DUTY_MIN)
      / (_DIST_MAX - _DIST_MIN) + 0.5f);
  duty = constrain(duty, _DUTY_MIN, _DUTY_MAX);
  myservo.writeMicroseconds(duty);

  // Comma-separated label:value pairs for the serial plotter.
  Serial.print("_DUTY_MIN:");  Serial.print(_DUTY_MIN);
  Serial.print(",_DIST_MIN:"); Serial.print(_DIST_MIN);
  Serial.print(",IR:");        Serial.print(a_value);
  Serial.print(",dist_raw:");  Serial.print(dist_raw);
  Serial.print(",ema:");       Serial.print(dist_ema);
  Serial.print(",servo:");     Serial.print(duty);
  Serial.print(",_DIST_MAX:"); Serial.print(_DIST_MAX);
  Serial.print(",_DUTY_MAX:"); Serial.print(_DUTY_MAX);
  Serial.println();
}
