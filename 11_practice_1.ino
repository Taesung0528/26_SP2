#include <Servo.h>

// Arduino pin assignment
#define PIN_LED   9   // active-low: LOW = ON
#define PIN_TRIG  12
#define PIN_ECHO  13
#define PIN_SERVO 10

// Sonar parameters
#define SND_VEL        346.0
#define INTERVAL       25       // ms
#define PULSE_DURATION 10       // us
#define _DIST_MIN      180.0    // mm
#define _DIST_MAX      360.0    // mm

#define TIMEOUT ((INTERVAL / 2) * 1000.0)
#define SCALE   (0.001 * 0.5 * SND_VEL)

#define _EMA_ALPHA 0.3

// Calibrated servo pulse widths
#define _DUTY_MIN 500
#define _DUTY_MAX 2500
#define _DUTY_NEU ((_DUTY_MIN + _DUTY_MAX) / 2)

Servo myservo;

float dist_prev = (_DIST_MIN + _DIST_MAX) / 2.0;
float dist_ema  = (_DIST_MIN + _DIST_MAX) / 2.0;
bool ema_initialized = false;

unsigned long last_sampling_time = 0;

void setup() {
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, HIGH); // OFF until an in-range reading

  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  digitalWrite(PIN_TRIG, LOW);

  // Set calibrated limits for both pulse output and read().
  myservo.attach(PIN_SERVO, _DUTY_MIN, _DUTY_MAX);
  myservo.writeMicroseconds(_DUTY_NEU);

  Serial.begin(57600);
  last_sampling_time = millis();
}

void loop() {
  unsigned long now = millis();

  // Handles millis() rollover.
  if ((unsigned long)(now - last_sampling_time) < INTERVAL) {
    return;
  }
  last_sampling_time = now;

  float dist_raw = USS_measure(PIN_TRIG, PIN_ECHO);

  bool in_range = (dist_raw >= _DIST_MIN &&
                   dist_raw <= _DIST_MAX);

  // LED ON when the current measurement is within 18–36 cm.
  digitalWrite(PIN_LED, in_range ? LOW : HIGH);

  // Range filter: retain the previous valid measurement.
  float dist_filtered = dist_prev;

  if (in_range) {
    dist_filtered = dist_raw;
    dist_prev = dist_raw;
  }

  // Initialize EMA only after the first valid measurement.
  if (!ema_initialized) {
    if (in_range) {
      dist_ema = dist_filtered;
      ema_initialized = true;
    }
  } else {
    dist_ema = _EMA_ALPHA * dist_filtered
             + (1.0 - _EMA_ALPHA) * dist_ema;
  }

  // Distance -> continuous angle: 180–360 mm -> 0–180 degrees.
  float servo_angle =
      (dist_ema - _DIST_MIN) * 180.0 / (_DIST_MAX - _DIST_MIN);

  servo_angle = constrain(servo_angle, 0.0, 180.0);

  // Angle -> calibrated pulse width: 0–180 degrees -> 500–2500 us.
  int servoPulse = (int)(
      _DUTY_MIN
      + servo_angle * (_DUTY_MAX - _DUTY_MIN) / 180.0
      + 0.5
  );

  myservo.writeMicroseconds(servoPulse);

  // Serial Plotter format from the assignment; distance unit: mm.
  Serial.print("Min:");    Serial.print(_DIST_MIN);
  Serial.print(",dist:");  Serial.print(dist_raw);
  Serial.print(", ema:");  Serial.print(dist_ema);
  Serial.print(",Servo:"); Serial.print(myservo.read());
  Serial.print(",Max:");   Serial.print(_DIST_MAX);
  Serial.println("");
}

// Return distance in mm; 0 indicates an echo timeout.
float USS_measure(int TRIG, int ECHO) {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);

  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE;
}
