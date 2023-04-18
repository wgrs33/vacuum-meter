#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <EncoderButton.h>

#include "VacuumMeter.h"
#include "custom_chars.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);
EncoderButton encoder(2, 3, 8);

constexpr int BUTTON = 8;
constexpr int VACCUM_1 = A1;
constexpr int VACCUM_2 = A0;

constexpr unsigned int BAR_MIN = 0;
constexpr unsigned int BAR_MAX = 70;

constexpr unsigned int VACCUM_AMIN = 41;
constexpr unsigned int VACCUM_AMAX = 962;
constexpr unsigned int VACCUM_VMIN = 150;
constexpr unsigned int VACCUM_VMAX = 1150;
constexpr int VACCUM_DMIN = -850;
constexpr int VACCUM_DMAX = 0;

constexpr double c_MMHG = 1.33322;
constexpr unsigned int ADC_SAMPLES = 5000;

int g_menu_state = 1;
int g_delta;
int g_pression_atmo;
int g_run_loop = true;
int g_clicked = false;

void setup() {
    pinMode(BUTTON, INPUT_PULLUP);

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

    g_pression_atmo = map(analogRead(VACCUM_1), VACCUM_AMIN, VACCUM_AMAX, VACCUM_VMIN, VACCUM_VMAX);
    g_delta = g_pression_atmo - map(analogRead(VACCUM_2), VACCUM_AMIN, VACCUM_AMAX, VACCUM_VMIN, VACCUM_VMAX);
    
    encoder.setEncoderHandler([&g_menu_state](EncoderButton &e) {
        g_menu_state += e.increment();
    });
    encoder.setLongClickHandler([&g_run_loop](EncoderButton &e) {
        g_run_loop = false;
    });
    encoder.setClickHandler([&g_clicked](EncoderButton &e) {
        g_clicked = true;
    });
}

void loop() {
    lcd.clear();
    lcd.print("Choose function");
    delay(200);

    while(g_run_loop) {
        encoder.update();
        switch (g_menu_state) {
            case 1:
                lcd.setCursor(0, 1);
                lcd.print("<   Synchro    >");
                break;
            case 2:
                lcd.setCursor(0, 1);
                lcd.print("<  Vacuum  >");
                break;
            case 3:
                lcd.setCursor(0, 1);
                lcd.print("<  P.absolute  >");
                break;
        }
    };

    g_menu_state = 1;
    switch (g_menu_state) {
        case 1:
            synchronization();
            break;
        case 2:
            pression_diff();
            break;
        case 3:
            pression_absolute();
            break;
    }
}

void synchronization() {
    long total_V1;
    long total_V2;
    int pression_V1;
    int pression_V2;

    lcd.clear();
    lcd.print("      mbar      ");

    while (digitalRead(BUTTON) == HIGH) {
        total_V1 = 0;
        total_V2 = 0;
        for (int i = 0; i < ADC_SAMPLES; ++i) {
            pression_V1 = analogRead(VACCUM_1);
            total_V1 += pression_V1;
            pression_V2 = analogRead(VACCUM_2);
            total_V2 += pression_V2;
            if (digitalRead(BUTTON) == LOW) {
                break;
            }
        }
        pression_V1 = total_V1 / ADC_SAMPLES;
        pression_V2 = total_V2 / ADC_SAMPLES;
        pression_V1 = map(pression_V1, VACCUM_AMIN, VACCUM_AMAX, VACCUM_VMIN, VACCUM_VMAX);
        pression_V2 = map(pression_V2, VACCUM_AMIN, VACCUM_AMAX, VACCUM_VMIN, VACCUM_VMAX);
        pression_V2 = pression_V2 + g_delta;

        lcd.setCursor(0, 0);
        lcd.print("    ");
        lcd.setCursor(0, 0);
        lcd.print(pression_V1);
        lcd.setCursor(12, 0);
        lcd.print("    ");
        lcd.setCursor(12, 0);
        align_right(pression_V2, 4);

        set_bar(constrain(((pression_V2 - pression_V1 + BAR_MAX) / 2), BAR_MIN, BAR_MAX));
    }
    delay(100);
    while (digitalRead(BUTTON) == LOW);
}

void pression_diff() {
    long total_V1;
    int pression_V1;

    lcd.clear();
    lcd.print("Press.      mbar");

    while (digitalRead(BUTTON) == HIGH) {
        total_V1 = 0;
        for (int i = 0; i < ADC_SAMPLES; ++i) {
            pression_V1 = analogRead(VACCUM_1);
            total_V1 += pression_V1;
            if (digitalRead(BUTTON) == LOW) {
                break;
            }
        }
        pression_V1 = total_V1 / ADC_SAMPLES;
        pression_V1 = map(pression_V1, VACCUM_AMIN, VACCUM_AMAX, VACCUM_VMIN, VACCUM_VMAX);
        pression_V1 = pression_V1 - g_pression_atmo;

        lcd.setCursor(7, 0);
        lcd.print("    ");
        lcd.setCursor(7, 0);
        lcd.print(pression_V1);

        set_bar(constrain(map(pression_V1, VACCUM_DMIN, VACCUM_DMAX, BAR_MAX, BAR_MIN), BAR_MIN, BAR_MAX));
    }
    delay(100);
    while (digitalRead(BUTTON) == LOW);
}

void pression_absolute() {
    long total_V1;
    int pression_V1;
    lcd.clear();
    lcd.print("P.abs.      mbar");
    lcd.setCursor(0, 1);
    lcd.print("            mmHg");

    while (digitalRead(BUTTON) == HIGH) {
        total_V1 = 0;
        for (int i = 0; i < ADC_SAMPLES; ++i) {
            pression_V1 = analogRead(VACCUM_1);
            total_V1 = total_V1 + pression_V1;
            if (digitalRead(BUTTON) == LOW) {
                break;
            }
        }
        pression_V1 = total_V1 / ADC_SAMPLES;
        pression_V1 = map(pression_V1, VACCUM_AMIN, VACCUM_AMAX, VACCUM_VMIN, VACCUM_VMAX);
        lcd.setCursor(7, 0);
        lcd.print("    ");
        lcd.setCursor(7, 0);
        lcd.print(pression_V1);
        lcd.setCursor(0, 1);
        pression_V1 = pression_V1 / c_MMHG;
        pression_V1 = floor(pression_V1);
        lcd.setCursor(7, 1);
        lcd.print("     ");
        lcd.setCursor(7, 1);
        lcd.print(pression_V1);
    }
    delay(100);
    while (digitalRead(BUTTON) == LOW);
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
    for(int i = (max_length - 1); i > 0; --i) {
        if (value < pow(10, i)) lcd.print(" ");
    }
    lcd.print(value);
}
