//Current code for the Doppler project. There are notes as I was having ChatrGPT help me with some coding questions I was having along the way. 


#include <arduinoFFT.h>

// === Pin Definitions ===
// Segment pins A-G: D6–D12
const uint8_t segmentPins[7] = {6, 7, 8, 9, 10, 11, 12}; // A–G
// Digit control pins (left to right): D2–D5
const uint8_t digitPins[4] = {2, 3, 4, 5}; // Digit 1–4

// Segment patterns for digits 0–9 (common cathode)
const uint8_t digitSegments[10][7] = {
  {1, 1, 1, 1, 1, 1, 0}, // 0
  {0, 1, 1, 0, 0, 0, 0}, // 1
  {1, 1, 0, 1, 1, 0, 1}, // 2
  {1, 1, 1, 1, 0, 0, 1}, // 3
  {0, 1, 1, 0, 0, 1, 1}, // 4
  {1, 0, 1, 1, 0, 1, 1}, // 5
  {1, 0, 1, 1, 1, 1, 1}, // 6
  {1, 1, 1, 0, 0, 0, 0}, // 7
  {1, 1, 1, 1, 1, 1, 1}, // 8
  {1, 1, 1, 1, 0, 1, 1}  // 9
};

// === FFT Setup ===
#define MIC_PIN A0
ArduinoFFT<double> FFT;
const uint16_t samples = 128;
double vReal[samples];
double vImag[samples];
const double samplingFrequency = 4000.0; // Hz

double lastPeak = 0.0;
const double alpha = 0.8; // smoothing factor
double peak = 0.0;

// === Display Refresh Variables ===
unsigned long lastDisplayTime = 0;
int currentDigit = 0;
int displayDigits[4] = {0, 0, 0, 0};

// === Timing ===
unsigned long lastFFTTime = 0;

void setup() {
  Serial.begin(9600);

  // Setup pins
  for (int i = 0; i < 7; i++) pinMode(segmentPins[i], OUTPUT);
  for (int i = 0; i < 4; i++) pinMode(digitPins[i], OUTPUT);
}

void loop() {
  // Refresh display at ~200 Hz
  if (millis() - lastDisplayTime > 5) {
    refreshDisplay();
    lastDisplayTime = millis();
  }

  // Run FFT every ~200ms
  if (millis() - lastFFTTime > 200) {
    runFFT();
    lastFFTTime = millis();
  }
}

void runFFT() {
  // Sample signal using micros() for accuracy
  unsigned long startMicros = micros();
  for (uint16_t i = 0; i < samples; i++) {
    while (micros() - startMicros < (1000000.0 / samplingFrequency) * i);
    vReal[i] = analogRead(MIC_PIN);
    vImag[i] = 0.0;
  }

  // Remove DC offset
  double mean = 0;
  for (uint16_t i = 0; i < samples; i++) mean += vReal[i];
  mean /= samples;
  for (uint16_t i = 0; i < samples; i++) vReal[i] -= mean;

  // Perform FFT
  FFT.windowing(vReal, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.compute(vReal, vImag, samples, FFT_FORWARD);
  FFT.complexToMagnitude(vReal, vImag, samples);

  // Amplitude threshold: reject if signal too weak
  double maxAmplitude = 0;
  for (uint16_t i = 1; i < samples / 2; i++) {
    if (vReal[i] > maxAmplitude) maxAmplitude = vReal[i];
  }
  if (maxAmplitude < 50) {
    peak = 0;
    updateDisplay(0);
    return;
  }

  // Find peak frequency
  double rawPeak = FFT.majorPeak(vReal, samples, samplingFrequency);

  // Filter out irrelevant frequencies
  if (rawPeak < 60 || rawPeak > 2000) rawPeak = 0;

  // Apply smoothing & stability
  if (abs(rawPeak - lastPeak) < 20) {
    peak = lastPeak;
  } else {
    peak = alpha * lastPeak + (1.0 - alpha) * rawPeak;
  }
  lastPeak = peak;

  int freq = (int)peak;
  Serial.print("Frequency: ");
  Serial.println(freq);

  updateDisplay(freq);
}

void updateDisplay(int freq) {
  displayDigits[0] = (freq / 1000) % 10;
  displayDigits[1] = (freq / 100) % 10;
  displayDigits[2] = (freq / 10) % 10;
  displayDigits[3] = freq % 10;
}

void refreshDisplay() {
  // Turn off all digits
  for (int i = 0; i < 4; i++) digitalWrite(digitPins[i], LOW);

  // Get value for current digit
  int value = displayDigits[currentDigit];

  // Set segments
  for (int s = 0; s < 7; s++) {
    digitalWrite(segmentPins[s], digitSegments[value][s]);
  }

  // Enable current digit
  digitalWrite(digitPins[currentDigit], HIGH);

  // Move to next digit
  currentDigit = (currentDigit + 1) % 4;
}


\\Newest Version 

#include <arduinoFFT.h>

// === Pin Definitions ===
// Segment pins A–G
const uint8_t segmentPins[7] = {6, 7, 8, 9, 10, 11, 12}; // A-G
// Digit control pins (from leftmost to rightmost)
const uint8_t digitPins[4] = {2, 3, 4, 5}; // Digit 1–4

// Segment patterns for digits 0–9 and dash (common cathode)
const uint8_t digitSegments[11][7] = {
  {1, 1, 1, 1, 1, 1, 0}, // 0
  {0, 1, 1, 0, 0, 0, 0}, // 1
  {1, 1, 0, 1, 1, 0, 1}, // 2
  {1, 1, 1, 1, 0, 0, 1}, // 3
  {0, 1, 1, 0, 0, 1, 1}, // 4
  {1, 0, 1, 1, 0, 1, 1}, // 5
  {1, 0, 1, 1, 1, 1, 1}, // 6
  {1, 1, 1, 0, 0, 0, 0}, // 7
  {1, 1, 1, 1, 1, 1, 1}, // 8
  {1, 1, 1, 1, 0, 1, 1}, // 9
  {0, 0, 0, 1, 0, 0, 0}  // Dash '-'
};

// === FFT Setup ===
#define MIC_PIN A0
ArduinoFFT<double> FFT;
const uint16_t samples = 128;
double vReal[samples];
double vImag[samples];
const double samplingFrequency = 4000.0;

double lastPeak = 0.0;
const double alpha = 0.8;
double peak = 0.0;

// === Display Variables ===
int displayDigits[4] = {10, 10, 10, 10}; // Start with '----'
int currentDigit = 0;
unsigned long lastDigitRefresh = 0;

// === Update Timing ===
unsigned long lastFFTTime = 0;

void setup() {
  Serial.begin(9600);
  for (int i = 0; i < 7; i++) pinMode(segmentPins[i], OUTPUT);
  for (int i = 0; i < 4; i++) pinMode(digitPins[i], OUTPUT);
}

void loop() {
  // Constant fast display multiplexing (~1 ms)
  if (millis() - lastDigitRefresh >= 1) {
    refreshDisplay();
    lastDigitRefresh = millis();
  }

  // Update reading every 500 ms
  if (millis() - lastFFTTime >= 500) {
    runFFT();
    lastFFTTime = millis();
  }
}

void runFFT() {
  unsigned long startMicros = micros();
  for (uint16_t i = 0; i < samples; i++) {
    while (micros() - startMicros < (1000000.0 / samplingFrequency) * i);
    vReal[i] = analogRead(MIC_PIN);
    vImag[i] = 0.0;
  }

  // Remove DC
  double mean = 0;
  for (uint16_t i = 0; i < samples; i++) mean += vReal[i];
  mean /= samples;
  for (uint16_t i = 0; i < samples; i++) vReal[i] -= mean;

  FFT.windowing(vReal, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.compute(vReal, vImag, samples, FFT_FORWARD);
  FFT.complexToMagnitude(vReal, vImag, samples);

  double maxAmplitude = 0;
  for (uint16_t i = 1; i < samples / 2; i++) {
    if (vReal[i] > maxAmplitude) maxAmplitude = vReal[i];
  }

  if (maxAmplitude < 50) {
    showDashes();
    Serial.println("Signal too weak");
    return;
  }

  double rawPeak = FFT.majorPeak(vReal, samples, samplingFrequency);
  if (rawPeak < 60 || rawPeak > 2000) {
    showDashes();
    Serial.println("Out of range");
    return;
  }

  // Smooth frequency
  if (abs(rawPeak - lastPeak) < 20) {
    peak = lastPeak;
  } else {
    peak = alpha * lastPeak + (1 - alpha) * rawPeak;
  }
  lastPeak = peak;

  int freq = (int)peak;
  Serial.print("Freq: ");
  Serial.println(freq);
  updateDisplay(freq);
}

void updateDisplay(int freq) {
  displayDigits[0] = (freq / 1000) % 10;
  displayDigits[1] = (freq / 100) % 10;
  displayDigits[2] = (freq / 10) % 10;
  displayDigits[3] = freq % 10;
}

void showDashes() {
  for (int i = 0; i < 4; i++) {
    displayDigits[i] = 10; // dash
  }
}

void refreshDisplay() {
  // Turn off all digits before switching
  for (int i = 0; i < 4; i++) digitalWrite(digitPins[i], LOW);

  int value = displayDigits[currentDigit];
  for (int s = 0; s < 7; s++) {
    digitalWrite(segmentPins[s], digitSegments[value][s]);
  }

  digitalWrite(digitPins[currentDigit], HIGH);
  currentDigit = (currentDigit + 1) % 4;
}
