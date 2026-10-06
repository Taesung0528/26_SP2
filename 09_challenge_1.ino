// Arduino pin assignment
#define PIN_LED 9
#define PIN_TRIG 12
#define PIN_ECHO 13

#define SND_VEL 346.0
#define INTERVAL 25UL          // Sampling interval (ms)
#define PULSE_DURATION 10     // Trigger pulse (us)
#define _DIST_MIN 100         // Plot reference (mm)
#define _DIST_MAX 300         // Plot reference (mm)
#define TIMEOUT ((INTERVAL / 2) * 1000UL)
#define SCALE (0.001 * 0.5 * SND_VEL)
#define _EMA_ALPHA 0.5

// Change to 3, 10, or 30 for the required screenshots.
#define N 30
#if N < 1
#error "N must be at least 1"
#endif

unsigned long last_sampling_time;
float samples[N];
int next_index = 0;
int sample_count = 0;
float dist_ema;
bool ema_initialized = false;

float median_filter(float new_sample);
float USS_measure(int TRIG, int ECHO);

void setup() {
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  digitalWrite(PIN_TRIG, LOW);
  digitalWrite(PIN_LED, HIGH); // LED OFF (original active-low wiring)
  Serial.begin(57600);
  last_sampling_time = millis();
}

void loop() {
  unsigned long now = millis();
  if (now - last_sampling_time < INTERVAL)
    return;
  last_sampling_time = now;

  float dist_raw = USS_measure(PIN_TRIG, PIN_ECHO);

  // Keep all raw samples, including timeout (0) and out-of-range values.
  // No previous-valid-value replacement and no range clipping.
  float dist_median = median_filter(dist_raw);

  // Independent EMA for comparison only; median does not use EMA output.
  if (!ema_initialized) {
    dist_ema = dist_raw;
    ema_initialized = true;
  } else {
    dist_ema = _EMA_ALPHA * dist_raw + (1.0 - _EMA_ALPHA) * dist_ema;
  }

  Serial.print("Min:");    Serial.print(_DIST_MIN);
  Serial.print(",raw:");   Serial.print(dist_raw);
  Serial.print(",ema:");   Serial.print(dist_ema);
  Serial.print(",median:"); Serial.print(dist_median);
  Serial.print(",Max:");   Serial.print(_DIST_MAX);
  Serial.println("");

  // Preserve the original LED range indicator.
  if (dist_raw < _DIST_MIN || dist_raw > _DIST_MAX)
    digitalWrite(PIN_LED, HIGH);
  else
    digitalWrite(PIN_LED, LOW);
}

float median_filter(float new_sample) {
  // Circular buffer: overwrite only the oldest sample once full.
  samples[next_index] = new_sample;
  next_index = (next_index + 1) % N;
  if (sample_count < N)
    sample_count++;

  // Sort a copy to preserve the original buffer's time order.
  float sorted[N];
  for (int i = 0; i < sample_count; i++)
    sorted[i] = samples[i];

  // Insertion sort (sufficient for N <= 30).
  for (int i = 1; i < sample_count; i++) {
    float value = sorted[i];
    int j = i - 1;
    while (j >= 0 && sorted[j] > value) {
      sorted[j + 1] = sorted[j];
      j--;
    }
    sorted[j + 1] = value;
  }

  // During startup, use only samples actually measured.
  int middle = sample_count / 2;
  if (sample_count % 2 == 1)
    return sorted[middle];
  return (sorted[middle - 1] + sorted[middle]) / 2.0;
}

float USS_measure(int TRIG, int ECHO) {
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);
  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE;
}
