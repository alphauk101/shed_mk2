#include "Arduino.h"
#include "DigitLedDisplay.h"

#define DECODEMODE_ADDR 9
#define BRIGHTNESS_ADDR 10
#define SCANLIMIT_ADDR 11
#define SHUTDOWN_ADDR 12
#define DISPLAYTEST_ADDR 15


DigitLedDisplay::DigitLedDisplay(int dinPin, int csPin, int clkPin) {
  this->DIN_PIN = dinPin;
  this->CS_PIN = csPin;
  this->CLK_PIN = clkPin;

  pinMode(DIN_PIN, OUTPUT);
  pinMode(CS_PIN, OUTPUT);
  pinMode(CLK_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);
}

void DigitLedDisplay::setBright(int brightness) {
  if (brightness >= 0 && brightness < 16)
    this->write(BRIGHTNESS_ADDR, brightness);
}

void DigitLedDisplay::setDigitLimit(int limit) {
  this->_digitLimit = limit;

  this->write(DISPLAYTEST_ADDR, 0);
  this->write(SCANLIMIT_ADDR, limit - 1);

  // 0: Register Format
  // 255: Code B Font (0xff)
  this->write(DECODEMODE_ADDR, 0);
  this->clear();
  this->write(SHUTDOWN_ADDR, 1);
}


void DigitLedDisplay::on() {
  this->write(SHUTDOWN_ADDR, 0x01);
}

void DigitLedDisplay::off() {
  this->write(SHUTDOWN_ADDR, 0x00);
}

void DigitLedDisplay::clear() {
  for (int i = 1; i <= _digitLimit; i++) {
    this->write(i, B00000000);
  }
}

void DigitLedDisplay::table(byte address, int val, boolean period) {
  byte tableValue;
  tableValue = pgm_read_byte_near(charTable + val);
  if (period)  //If period required or the bit
    tableValue |= B10000000;
  this->write(address, tableValue);
}

void DigitLedDisplay::write(volatile byte address, volatile byte data) {
  delay(2); //Add a small delay as I believe the CS pin wasnt being changed quick enough.
  digitalWrite(CS_PIN, LOW);
  shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, address);
  shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, data);
  digitalWrite(CS_PIN, HIGH);
}

void DigitLedDisplay::printMinus(byte startDigit) {
  byte tableValue;
  tableValue = B00000001;
  this->write(startDigit, tableValue);
}

void DigitLedDisplay::printDigit(long number, byte startDigit, boolean period) {
  String figure = String(number);
  int figureLength = figure.length();

  int parseInt;
  char str[2];
  for (int i = 0; i < figure.length(); i++) {
    str[0] = figure[i];
    str[1] = '\0';
    parseInt = (int)strtol(str, NULL, 10);
    if (i == (figure.length() - 1)) {
      this->table(figureLength - i + startDigit, parseInt, period);
    } else {
      this->table(figureLength - i + startDigit, parseInt, false);
    }
  }
}