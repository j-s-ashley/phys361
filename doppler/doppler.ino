/*
--- start of header ---
doppler.ino
Takes analog input from microphone
Displays via LCD on Arduino

Last edit:
2025/05/08
by
Jordan Ashley @j-s-ashley

TODO: add microphone circuit info
  The circuit:
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * LCD VSS pin to ground
 * LCD VCC pin to 5V
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)

--- end of header ---
*/

#include <arduinoFFT.h>
#include <LiquidCrystal.h>

// Initialize the library by associating LCD interface pins
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// FFT setup
uint16_t FFTrefresh = 50;

#define MIC_PIN A0		// set arduino pin A0 as mic input
ArduinoFFT<double> FFT; 	// initialize FFT class object
const uint16_t samples = 128;	// set max samples held
double vReal[samples];
double vImag[samples];
const double samplingFrequency = 4000.0; // Hz

double lastPeak = 0.0;
const double alpha = 0.8; 	// smoothing factor
double peak = 0.0;

// Timing
unsigned long lastFFTTime = 0;

void setup() {
  lcd.begin(16, 2);
  lcd.print("Starting up...");
}

void loop() {								// hold FFT to refresh rate
  if (millis() - lastFFTTime > FFTrefresh) {
    runFFT();
    lastFFTTime = millis();
  }
}

void runFFT() {
  // Sample the microphone
  unsigned long startMicros = micros();					// timestamp in microseconds
  for (uint16_t i = 0; i < samples; i++) {				// limit samples taken
    // Sample every 1000000 / samplingFrequency seconds
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
  FFT.windowing(vReal, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);	// weigh samples
  FFT.compute(vReal, vImag, samples, FFT_FORWARD);			// run calculations
  FFT.complexToMagnitude(vReal, vImag, samples);			// compute magnitude

  // Check signal strength
  double maxAmplitude = 0;
  for (uint16_t i = 1; i < samples / 2; i++) {
    if (vReal[i] > maxAmplitude) maxAmplitude = vReal[i];
  }

  if (maxAmplitude < 50) {
    displayFrequency(0); 						// Too quiet
    return;
  }

  // Calculate dominant frequency
  double rawPeak = FFT.majorPeak(vReal, samples, samplingFrequency);
  if (rawPeak < 60 || rawPeak > 2000) rawPeak = 0;

  // Smooth
  if (abs(rawPeak - lastPeak) < 20) {
    peak = lastPeak;
  } else {
    peak = alpha * lastPeak + (1.0 - alpha) * rawPeak;
  }
  lastPeak = peak;

  displayFrequency((int)peak);						// send frequency to LCD
}

void displayFrequency(int freq) {
  lcd.clear();
  lcd.setCursor(0, 0);

  if (freq == 0) {
    lcd.print("Frequency: ----");
  } else {
    lcd.print("Frequency: ");
    lcd.print(freq);
    lcd.print(" Hz");
  }
}
