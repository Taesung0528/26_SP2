#include <Servo.h>

#define PIN_TRIG 12
#define PIN_ECHO 13
#define PIN_SERVO 10

#define SND_VEL 346.0
#define PULSE_DURATION 10
#define TIMEOUT 10000UL
#define SCALE (0.001 * 0.5 * SND_VEL)

// Calibrate these endpoints for the actual servo.
#define _DUTY_MIN 500
#define _DUTY_MAX 2400
#define _DUTY_NEU ((_DUTY_MIN + _DUTY_MAX) / 2)

// Experiment 1: 3.0 degrees/s, 180 degrees in 60 seconds.
// Experiment 2: 0.3 degrees/s, 90 degrees in 300 seconds.
#define _SERVO_SPEED 0.3
#define INTERVAL 20UL
#define _INTERVAL_DIST 100UL
#define _INTERVAL_SERIAL 100UL

Servo myservo;
unsigned long move_start_time;
unsigned long last_sampling_time;
unsigned long last_dist_time;
unsigned long last_serial_time;
unsigned long moving_time;
float target_angle;
float angle_curr = 0.0;
float dist_raw = 0.0;
int duty_curr = _DUTY_MIN;
bool finished = false;

float USS_measure(int TRIG, int ECHO);

void setup() {
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  digitalWrite(PIN_TRIG, LOW);
  Serial.begin(115200);

  // Explicit limits allow the configured 500 us endpoint.
  myservo.attach(PIN_SERVO, _DUTY_MIN, _DUTY_MAX);
  myservo.writeMicroseconds(_DUTY_MIN);
  delay(2000); // Allow the servo to reach the starting position.

  if (_SERVO_SPEED == 3.0) {
    target_angle = 180.0;
    moving_time = 60000UL;
  } else if (_SERVO_SPEED == 0.3) {
    target_angle = 90.0;
    moving_time = 300000UL;
  } else {
    Serial.println("Set _SERVO_SPEED to 3.0 or 0.3.");
    while (1) { }
  }

  Serial.print("Speed_deg_s:"); Serial.println(_SERVO_SPEED, 1);
  Serial.print("Target_deg:"); Serial.println(target_angle, 1);
  Serial.println("START");
  Serial.flush();
  move_start_time = millis();
  last_sampling_time = move_start_time;
  last_dist_time = move_start_time;
  last_serial_time = move_start_time;
}

void loop() {
  unsigned long now = millis();

  if (now - last_sampling_time >= INTERVAL) {
    last_sampling_time = now;
    unsigned long elapsed = now - move_start_time;

    // Calculate from elapsed time; do not truncate each small increment.
    if (elapsed >= moving_time) {
      angle_curr = target_angle;
      finished = true;
    } else {
      angle_curr = _SERVO_SPEED * (elapsed / 1000.0);
      if (angle_curr > target_angle) angle_curr = target_angle;
    }

    float duty_float = _DUTY_MIN
        + (_DUTY_MAX - _DUTY_MIN) * (angle_curr / 180.0);
    duty_curr = (int)(duty_float + 0.5); // Round only at output.
    myservo.writeMicroseconds(duty_curr);
  }

  // Sensor and serial tasks run less often than servo updates.
  now = millis();
  if (now - last_dist_time >= _INTERVAL_DIST) {
    last_dist_time = now;
    dist_raw = USS_measure(PIN_TRIG, PIN_ECHO);
  }

  now = millis();
  if (now - last_serial_time >= _INTERVAL_SERIAL) {
    last_serial_time = now;
    Serial.print("time_s:"); Serial.print((now - move_start_time) / 1000.0, 2);
    Serial.print(",angle_cmd:"); Serial.print(angle_curr, 2);
    Serial.print(",duty_curr:"); Serial.print(duty_curr);
    Serial.print(",dist_raw:"); Serial.print(dist_raw, 1);
    Serial.print(",done:"); Serial.println(finished ? 1 : 0);
  }
  // Hold the endpoint after completion. Reset to repeat the experiment.
}

float USS_measure(int TRIG, int ECHO) {
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);
  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE;
}
