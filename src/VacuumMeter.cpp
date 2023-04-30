#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <EncoderButton.h>

#include "VacuumMeter.h"
#include "custom_chars.h"
#include "conversion.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);
EncoderButton encoder(2, 3, 8);

uint8_t g_menu_state = 0;
uint8_t g_menu_option = 1;
uint16_t g_delta = 0;
uint16_t g_pression_atmo = 0;
bool setup_done = false;
volatile uint16_t g_vacuum_1;
volatile uint16_t g_vacuum_2;

void setup() {
    pinMode(BUTTON, INPUT_PULLUP);
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(9, OUTPUT);
    digitalWrite(9, LOW);
 	cli();
 	TCCR2A = (1<<WGM21)|(0<<WGM20);// Mode CTC
 	TIMSK2 = (1<<OCIE2A);// Local interruption OCIE2A
 	TCCR2B = (0<<WGM22)|(1<<CS22)|(1<<CS21);// Frequency 16Mhz/ 256 = 62500
 	OCR2A = 250;//250*125 = 31250 = 16Mhz/256/2
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
            g_menu_option = constrain(val, 1, 3);
        }
    });
    encoder.setLongClickHandler([](EncoderButton &e) {
        g_menu_state = 0;
    });
    encoder.setClickHandler([](EncoderButton &e) {
        g_menu_state = g_menu_option;
    });
    lcd.clear();
    setup_done = true;
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
    }
}

void synchronization() {
    static int pression_V1;
    static int pression_V2;

    pression_V1 = map(g_vacuum_1, VACCUM_AMIN, VACCUM_AMAX, VACCUM_VMIN, VACCUM_VMAX);
    pression_V2 = map(g_vacuum_2, VACCUM_AMIN, VACCUM_AMAX, VACCUM_VMIN, VACCUM_VMAX);
    pression_V2 = pression_V2 + g_delta;

    lcd.setCursor(0, 0);
    lcd.print("    ");
    lcd.setCursor(0, 0);
    lcd.print(pression_V1);
    lcd.setCursor(12, 0);
    align_right(pression_V2, 4);

    set_bar(constrain(((pression_V2 - pression_V1 + BAR_MAX) / 2), BAR_MIN, BAR_MAX));
}

void pression_diff() {
    static int pression_V1;

    pression_V1 = map(g_vacuum_1, VACCUM_AMIN, VACCUM_AMAX, VACCUM_VMIN, VACCUM_VMAX) - g_pression_atmo;

    lcd.setCursor(7, 0);
    lcd.print("    ");
    lcd.setCursor(7, 0);
    lcd.print(pression_V1);

    set_bar(constrain(map(pression_V1, VACCUM_DMIN, VACCUM_DMAX, BAR_MAX, BAR_MIN), BAR_MIN, BAR_MAX));
}

void pression_absolute() {
    static int pression_V1;

    pression_V1 = map(g_vacuum_1, VACCUM_AMIN, VACCUM_AMAX, VACCUM_VMIN, VACCUM_VMAX);

    lcd.setCursor(7, 0);
    lcd.print("    ");
    lcd.setCursor(7, 0);
    lcd.print(pression_V1);
    lcd.setCursor(0, 1);
    pression_V1 = floor(pression_V1 / c_MMHG);
    lcd.setCursor(7, 1);
    lcd.print("     ");
    lcd.setCursor(7, 1);
    lcd.print(pression_V1);
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

void updateLcd() {
    static uint8_t last_menu_state = g_menu_state;
    if (setup_done) {
        switch (g_menu_state) {
            case 0:
                show_menu();
                break;
            case 1:
                if (last_menu_state != g_menu_state) {
                    // lcd.clear();
                    last_menu_state = g_menu_state;
                    lcd.setCursor(0, 0);
                    lcd.print("      mbar      ");
                }
                synchronization();
                break;
            case 2:
                if (last_menu_state != g_menu_state) {
                    // lcd.clear();
                    last_menu_state = g_menu_state;
                    lcd.setCursor(0, 0);
                    lcd.print("Press.      mbar");
                }
                pression_diff();
                break;
            case 3:
                if (last_menu_state != g_menu_state) {
                    // lcd.clear();
                    last_menu_state = g_menu_state;
                    lcd.setCursor(0, 0);
                    lcd.print("P.abs.      mbar");
                    lcd.setCursor(0, 1);
                    lcd.print("            mmHg");
                }
                pression_absolute();
                break;
        }
    }
}

void calibrate() {
    g_pression_atmo = map(g_vacuum_1, VACCUM_AMIN, VACCUM_AMAX, VACCUM_VMIN, VACCUM_VMAX);
    g_delta = g_pression_atmo - map(g_vacuum_2, VACCUM_AMIN, VACCUM_AMAX, VACCUM_VMIN, VACCUM_VMAX);
}

ISR(TIMER2_COMPA_vect){
 	static int counter=0;
    uint32_t sum_a0 = 0, sum_a1 = 0;
    encoder.update();
    for (uint8_t i = 0; i < ADC_SAMPLES; ++i) {
        sum_a0 += analogRead(VACCUM_1);
        sum_a1 += analogRead(VACCUM_2);
    }
    g_vacuum_1 = sum_a0 / ADC_SAMPLES;
    g_vacuum_2 = sum_a1 / ADC_SAMPLES;

 	if (++counter >= 50) { // 50 * 4 ms = 200 ms
 			counter = 0;
 			updateLcd();
 	}
}
