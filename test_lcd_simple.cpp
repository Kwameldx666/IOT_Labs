// Simple LCD test - copy this to src/main.cpp temporarily to test LCD alone
#include <Arduino.h>

#define LCD_RS 7
#define LCD_E  6
#define LCD_D4 5
#define LCD_D5 4
#define LCD_D6 3
#define LCD_D7 2

void lcdPulseEnable() {
  digitalWrite(LCD_E, LOW);
  delayMicroseconds(1);
  digitalWrite(LCD_E, HIGH);
  delayMicroseconds(1);
  digitalWrite(LCD_E, LOW);
  delayMicroseconds(100);
}

void lcdWrite4Bits(uint8_t value) {
  digitalWrite(LCD_D4, (value >> 0) & 0x01);
  digitalWrite(LCD_D5, (value >> 1) & 0x01);
  digitalWrite(LCD_D6, (value >> 2) & 0x01);
  digitalWrite(LCD_D7, (value >> 3) & 0x01);
  lcdPulseEnable();
}

void lcdSend(uint8_t value, uint8_t mode) {
  digitalWrite(LCD_RS, mode);
  lcdWrite4Bits(value >> 4);
  lcdWrite4Bits(value);
}

void lcdCommand(uint8_t cmd) { lcdSend(cmd, LOW); }
void lcdData(uint8_t data) { lcdSend(data, HIGH); }

void lcdInit() {
  pinMode(LCD_RS, OUTPUT);
  pinMode(LCD_E, OUTPUT);
  pinMode(LCD_D4, OUTPUT);
  pinMode(LCD_D5, OUTPUT);
  pinMode(LCD_D6, OUTPUT);
  pinMode(LCD_D7, OUTPUT);
  
  delay(100);
  digitalWrite(LCD_RS, LOW);
  digitalWrite(LCD_E, LOW);
  
  lcdWrite4Bits(0x03);
  delay(5);
  lcdWrite4Bits(0x03);
  delay(5);
  lcdWrite4Bits(0x03);
  delay(1);
  lcdWrite4Bits(0x02);
  delay(1);
  
  lcdCommand(0x28); // 4-bit, 2 lines, 5x8
  delay(1);
  lcdCommand(0x0C); // Display on, cursor off
  delay(1);
  lcdCommand(0x06); // Entry mode
  delay(1);
  lcdCommand(0x01); // Clear
  delay(3);
}

void lcdPrint(const char* str) {
  while (*str) lcdData(*str++);
}

void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.println("Starting LCD test...");
  
  lcdInit();
  Serial.println("LCD initialized");
  
  lcdCommand(0x80); // Line 0
  lcdPrint("Hello World!");
  Serial.println("Sent: Hello World!");
  
  lcdCommand(0xC0); // Line 1
  lcdPrint("Test 123");
  Serial.println("Sent: Test 123");
  
  Serial.println("LCD test complete!");
}

void loop() {
  static int counter = 0;
  delay(1000);
  
  lcdCommand(0xC0); // Line 1
  lcdPrint("Count: ");
  
  char buf[10];
  sprintf(buf, "%d   ", counter++);
  lcdPrint(buf);
  
  Serial.print("Counter: ");
  Serial.println(counter - 1);
}
