#include "VacuumMeter.h"

#include <Arduino.h>
#include <EncoderButton.h>
#include <LiquidCrystal_I2C.h>

#include "custom_chars.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);
EncoderButton encoder(2, 3, 8);

uint8_t g_menu_option = 1;
uint16_t g_delta = 0;
uint16_t g_pressure_atmo = 0;
bool g_setup_done = false;
volatile uint8_t g_menu_state = 0;
volatile bool g_enter_function = true;
volatile uint16_t g_vacuum_1 = 0;
volatile uint16_t g_vacuum_2 = 0;

void setup() {
    pinMode(BUTTON, INPUT_PULLUP);
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(9, OUTPUT);
    digitalWrite(9, LOW);
    cli();
    TCCR2A = (1 << WGM21) | (0 << WGM20);  // Mode CTC
    TIMSK2 = (1 << OCIE2A);                // Local interruption OCIE2A
    TCCR2B = (0 << WGM22) | (1 << CS22) |
             (1 << CS21);  // Frequency 16Mhz/ 256 = 62500
    OCR2A = 250;           // 250*125 = 31250 = 16Mhz/256/2
    sei();

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

    calibrate();

    encoder.setEncoderHandler([](EncoderButton &e) {
        if (!g_menu_state) {
            auto val = g_menu_option + e.increment();
            g_menu_option = constrain(val, 1, 4);
        }
    });
    encoder.setLongClickHandler([](EncoderButton &e) { g_menu_state = 0; });
    encoder.setClickHandler([](EncoderButton &e) {
        g_enter_function = true;
        g_menu_state = g_menu_option;
    });
    g_setup_done = true;
}

void loop() {}

void show_menu() {
    lcd.setCursor(0, 0);
    lcd.print("Choose function ");
    switch (g_menu_option) {
        case 1:
            lcd.setCursor(0, 1);
            lcd.print("<   Synchro    >");
            break;
        case 2:
            lcd.setCursor(0, 1);
            lcd.print("<    Vacuum    >");
            break;
        case 3:
            lcd.setCursor(0, 1);
            lcd.print("<  P.absolute  >");
            break;
        case 4:
            lcd.setCursor(0, 1);
            lcd.print("<  Calibrate   >");
            break;
    }
}

void synchronization() {
    static int pressure_V1;
    static int pressure_V2;

    pressure_V1 =
        map(g_vacuum_1, VACCUM_AMIN, VACCUM_AMAX, VACCUM_VMIN, VACCUM_VMAX);
    pressure_V2 =
        map(g_vacuum_2, VACCUM_AMIN, VACCUM_AMAX, VACCUM_VMIN, VACCUM_VMAX);
    pressure_V2 = pressure_V2 + g_delta;

    lcd.setCursor(0, 0);
    lcd.print("    ");
    lcd.setCursor(0, 0);
    lcd.print(pressure_V1);
    lcd.setCursor(12, 0);
    align_right(pressure_V2, 4);

    set_bar(constrain(((pressure_V2 - pressure_V1 + BAR_MAX) / 2), BAR_MIN,
                      BAR_MAX));
}

void pressure_diff() {
    static int pressure_V1;

    pressure_V1 =
        map(g_vacuum_1, VACCUM_AMIN, VACCUM_AMAX, VACCUM_VMIN, VACCUM_VMAX) -
        g_pressure_atmo;

    lcd.setCursor(7, 0);
    lcd.print("    ");
    lcd.setCursor(7, 0);
    lcd.print(pressure_V1);

    set_bar(
        constrain(map(pressure_V1, VACCUM_DMIN, VACCUM_DMAX, BAR_MAX, BAR_MIN),
                  BAR_MIN, BAR_MAX));
}

void pressure_absolute() {
    static int pressure_V1;

    pressure_V1 =
        map(g_vacuum_1, VACCUM_AMIN, VACCUM_AMAX, VACCUM_VMIN, VACCUM_VMAX);

    lcd.setCursor(7, 0);
    lcd.print("    ");
    lcd.setCursor(7, 0);
    lcd.print(pressure_V1);
    lcd.setCursor(0, 1);
    pressure_V1 = (int)floor(pressure_V1 / c_MMHG);
    lcd.setCursor(7, 1);
    lcd.print("     ");
    lcd.setCursor(7, 1);
    lcd.print(pressure_V1);
}

void set_bar(int value) {
    static bool start = false;
    static bool end = false;

    if (value == BAR_MIN) {
        if (start) {
            lcd.createChar(0, chStartEmpty);
            start = false;
        }
    } else {
        if (!start) {
            lcd.createChar(0, chStartFull);
            start = true;
        }
    }

    if (value == BAR_MAX) {
        if (!end) {
            lcd.createChar(7, chEndFull);
            end = true;
        }
    } else {
        if (end) {
            lcd.createChar(7, chEndEmpty);
            end = false;
        }
    }

    lcd.setCursor(0, 1);
    lcd.write((byte)0);

    for (byte i = 1; i < 15; ++i) {
        if (value > 5) {
            lcd.write((byte)6);
        } else {
            switch (value) {
                case 0:
                    lcd.write((byte)1);
                    break;
                case 1:
                    lcd.write((byte)2);
                    break;
                case 2:
                    lcd.write((byte)3);
                    break;
                case 3:
                    lcd.write((byte)4);
                    break;
                case 4:
                    lcd.write((byte)5);
                    break;
                case 5:
                    lcd.write((byte)6);
                    break;
            }
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
    static uint8_t cnt = 0;
    if (g_setup_done) {
        switch (g_menu_state) {
            case 0:
                show_menu();
                break;
            case 1:
                if (g_enter_function) {
                    g_enter_function = false;
                    lcd.clear();
                    lcd.setCursor(6, 0);
                    lcd.print("mbar");
                }
                synchronization();
                break;
            case 2:
                if (g_enter_function) {
                    g_enter_function = false;
                    lcd.clear();
                    lcd.print("Press.      mbar");
                }
                pressure_diff();
                break;
            case 3:
                if (g_enter_function) {
                    g_enter_function = false;
                    lcd.clear();
                    lcd.print("P.abs.      mbar");
                    lcd.setCursor(12, 1);
                    lcd.print("mmHg");
                }
                pressure_absolute();
                break;
            case 4:
                if (g_enter_function) {
                    g_enter_function = false;
                    lcd.clear();
                    lcd.print("Calibrating...");
                    calibrate();
                }
                if (++cnt > 10) {
                    cnt = 0;
                    g_menu_state = 0;
                }
                break;
        }
    }
}

void calibrate() {
    g_pressure_atmo =
        map(g_vacuum_1, VACCUM_AMIN, VACCUM_AMAX, VACCUM_VMIN, VACCUM_VMAX);
    g_delta = g_pressure_atmo - map(g_vacuum_2, VACCUM_AMIN, VACCUM_AMAX,
                                    VACCUM_VMIN, VACCUM_VMAX);
}

ISR(TIMER2_COMPA_vect) {
    static int counter = 0;
    uint32_t sum_a0 = 0, sum_a1 = 0;
    encoder.update();
    for (uint8_t i = 0; i < ADC_SAMPLES; ++i) {
        sum_a0 += analogRead(VACCUM_1);
        sum_a1 += analogRead(VACCUM_2);
    }
    g_vacuum_1 = sum_a0 / ADC_SAMPLES;
    g_vacuum_2 = sum_a1 / ADC_SAMPLES;

    if (++counter >= 50) {  // 50 * 4 ms = 200 ms
        counter = 0;
        updateLcd();
    }
}
