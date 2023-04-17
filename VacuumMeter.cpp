#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
    lcd.init();
    lcd.backlight();
    lcd.print("VacuumMeter");
    lcd.setCursor(0, 1);
    lcd.print("      by wgrs33");
    delay(2000);
    lcd.clear();
}

void loop() {
    delay(1000);
    int sensorValue = analogRead(A0);
    float voltage = sensorValue * (5.0 / 1023.0);
    lcd.setCursor(0, 0);
    lcd.print("A0 voltage:");
    lcd.setCursor(0, 1);
    lcd.print(voltage);
}
