#include "VacuumMeter.h"

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

#include "custom_chars.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);

int g_delta = 0;
bool g_setup_done = false;
volatile uint16_t g_vacuum_1 = 0;
volatile uint16_t g_vacuum_2 = 0;
volatile bool g_refresh_lcd = false;

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    cli();
    TCCR2A = (1 << WGM21) | (0 << WGM20);  // Mode CTC
    TIMSK2 = (1 << OCIE2A);                // Local interruption OCIE2A
    TCCR2B = (0 << WGM22) | (1 << CS22) |
             (1 << CS21);  // Frequency 16Mhz/ 256 = 62500
    OCR2A = 250;           // 250*125 = 31250 = 16Mhz/256/2
    sei();

#if defined(DEBUG)
    Serial.begin(19200);
#endif

    lcd.init();
    lcd.createChar(0, chStartEmpty);
    lcd.createChar(1, caseEmpty);
    lcd.createChar(2, chCase1);
    lcd.createChar(3, chCase2);
    lcd.createChar(4, chCase3);
    lcd.createChar(5, chCase4);
    lcd.createChar(6, chCase5);
    lcd.createChar(7, chEndEmpty);

    lcd.backlight();
    lcd.clear();
    lcd.print("VacuumMeter");
    lcd.setCursor(0, 1);
    lcd.print("      by wgrs33");
    delay(2000);
    lcd.clear();

    lcd.print("Calibrating...");
    while(!calibrate()) {
        delay(100);
    }
    lcd.clear();

    g_setup_done = true;
}

void loop() {
    if (g_refresh_lcd) {
        g_refresh_lcd = false;
        updateLcd();
    }
}

void synchronization() {
    static int pressure_V1;
    static int pressure_V2;

    pressure_V1 =
        map(g_vacuum_1, VACCUM_AMIN, VACCUM_AMAX, VACCUM_VMIN, VACCUM_VMAX);
    pressure_V2 =
        map(g_vacuum_2, VACCUM_AMIN, VACCUM_AMAX, VACCUM_VMIN, VACCUM_VMAX) + g_delta;

    lcd.setCursor(0, 0);
    lcd.print("    ");
    lcd.setCursor(0, 0);
    lcd.print(pressure_V1);
    lcd.setCursor(12, 0);
    align_right(pressure_V2, 4);

    set_bar(map(((pressure_V2 - pressure_V1 + 140) / 2), 0, 140, BAR_MIN, BAR_MAX));
}

void set_bar(int value) {
    static bool start = false;
    static bool end = false;

    if (value == BAR_MIN && start) {
        lcd.createChar(0, chStartEmpty);
        start = false;
    } else if (!start) {
        lcd.createChar(0, chStartFull);
        start = true;
    }

    if (value == BAR_MAX && !end) {
        lcd.createChar(7, chEndFull);
        end = true;
    } else if (end) {
        lcd.createChar(7, chEndEmpty);
        end = false;
    }

    lcd.setCursor(0, 1);
    lcd.write((byte)0);

    for (byte i = 1; i < 15; ++i) {
        if (value >= 5) {
            lcd.write((byte)6);
        } else {
            lcd.write((byte)(value + 1));
        }
        value = constrain((value - 5), BAR_MIN, BAR_MAX);
    }
    lcd.write((byte)7);
}

void align_right(int value, int max_length) {
    for (int i = (max_length - 1); i > 0; --i) {
        if (value < pow(10, i)) lcd.print(" ");
    }
    lcd.print(value);
}

void updateLcd() {
    static bool g_enter_function = true;
    if (g_setup_done) {
        if (g_enter_function) {
            g_enter_function = false;
            lcd.clear();
            lcd.setCursor(6, 0);
            lcd.print("mbar");
        }
        synchronization();
    }
}

bool calibrate() {
    static uint8_t cnt = 0;
    if (++cnt == 20) {
        g_delta = map(g_vacuum_1, VACCUM_AMIN, VACCUM_AMAX, VACCUM_VMIN, VACCUM_VMAX) 
            - map(g_vacuum_2, VACCUM_AMIN, VACCUM_AMAX, VACCUM_VMIN, VACCUM_VMAX);
        cnt = 0;
        return true;
    }
    return false;
}

float filter_value(const int current_value, const float &old_value) {
    return ((FILTER_CONSTANT * old_value) + ((1 - FILTER_CONSTANT) * (float)current_value));
}

ISR(TIMER2_COMPA_vect) {
    static int counter = 0;
    static float filtered1 = 0.f, filtered2 = 0.f;

    filtered1 = filter_value(analogRead(VACCUM_1), filtered1);
    filtered2 = filter_value(analogRead(VACCUM_2), filtered2);

    if (++counter >= 50) {  // 50 * 4 ms = 200 ms
        g_vacuum_1 = constrain((uint16_t)(floor(filtered1)), VACCUM_AMIN, VACCUM_AMAX);
        g_vacuum_2 = constrain((uint16_t)(floor(filtered2)), VACCUM_AMIN, VACCUM_AMAX);
#if defined(DEBUG)
        Serial.print("f1: ");
        Serial.print(filtered1);
        Serial.print(" | f2: ");
        Serial.print(filtered2);
        Serial.print(" | g_vacuum_1: ");
        Serial.print(g_vacuum_1);
        Serial.print(" | g_vacuum_2: ");
        Serial.print(g_vacuum_2);
        auto pV1 = map(g_vacuum_1, VACCUM_AMIN, VACCUM_AMAX, VACCUM_VMIN, VACCUM_VMAX);
        auto pV2 = map(g_vacuum_2, VACCUM_AMIN, VACCUM_AMAX, VACCUM_VMIN, VACCUM_VMAX) + g_delta;
        Serial.print(" | pV1: ");
        Serial.print(pV1);
        Serial.print(" | pV2: ");
        Serial.println(pV2);
#endif
        counter = 0;
        g_refresh_lcd = true;
    }
}
