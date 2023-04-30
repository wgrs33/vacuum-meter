#include "VacuumMeter.h"

#include <Arduino.h>
#include <EncoderButton.h>
#include <ItemCommand.h>
#include <ItemList.h>
#include <ItemSubMenu.h>
#include <LcdMenu.h>
#include <LiquidCrystal_I2C.h>

#include "conversion.h"
#include "custom_chars.h"

enum LcdMode {
    MENU = 0,
    SYNCHRO = 1,
    VACUUM = 2,
    ABS = 3
};

LcdMode g_menu_state = LcdMode::MENU;
uint8_t g_units = 0;
int g_delta;
int g_pression_atmo;
bool g_program_setup = true;
volatile bool g_refresh_lcd = false;
volatile uint16_t g_vacuum_1 = 0;
volatile uint16_t g_vacuum_2 = 0;

LiquidCrystal_I2C lcd(0x27, 16, 2);
EncoderButton encoder(2, 3, 8);
LcdMenu menu(2, 16);

extern MenuItem* settingsMenu[];
String units[] = {"mBar", "PSI", "mmHg", "inHg"};

// clang-format off
MAIN_MENU(
    ITEM_COMMAND("Synchro", []() { g_menu_state = LcdMode::SYNCHRO; }),
    ITEM_COMMAND("Vacuum", []() { g_menu_state = LcdMode::VACUUM; }),
    ITEM_COMMAND("P.absolute", []() { g_menu_state = LcdMode::ABS; }),
    ITEM_SUBMENU("Settings", settingsMenu)
);

SUB_MENU(settingsMenu, mainMenu,
    ITEM_STRING_LIST("Units", units, 4, [](uint8_t pos) { g_units = pos; }),
    ITEM_COMMAND("Calibrate", []() {
        lcd.clear();
        lcd.print("Calibrating...");
        while(!calibrate()) {
            delay(200);
        }
        lcd.clear(); 
    })
);
// clang-format on

void setup() {
    // ADC setup
    cli();
    TCCR2A = (1 << WGM21) | (0 << WGM20);  // Mode CTC
    TIMSK2 = (1 << OCIE2A);                // Local interruption OCIE2A
    TCCR2B = (0 << WGM22) | (1 << CS22) |
             (1 << CS21);  // Frequency 16Mhz/ 256 = 62500
    OCR2A = 250;           // 250*125 = 31250 = 16Mhz/256/2
    sei();

    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.print("VacuumMeter");
    lcd.setCursor(0, 1);
    lcd.print("      by wgrs33");
    delay(2000);

    lcd.print("Calibrating...");
    while(!calibrate()) {
        delay(200);
    }
    lcd.clear();

    menu.setupLcdWithMenu(0x27, mainMenu);

    encoder.setEncoderHandler([](EncoderButton &e) {
        if (g_menu_state == LcdMode::MENU) {
            if (e.increment() < 0)
                menu.up();
            else if (e.increment() > 0)
                menu.down();
        }
    });
    encoder.setLongClickHandler([](EncoderButton &e) {
        g_menu_state = LcdMode::MENU;
        g_program_setup = true;
        lcd.createChar(0, upArrow);
        lcd.createChar(1, downArrow);
        menu.back();
    });
    encoder.setClickHandler([](EncoderButton &e) {
        if (g_menu_state == LcdMode::MENU) {
            menu.enter();
        }
    });

    // set custom characters
    lcd.createChar(2, chCase1);
    lcd.createChar(3, chCase2);
    lcd.createChar(4, chCase3);
    lcd.createChar(5, chCase4);
    lcd.createChar(6, chCase5);
    lcd.createChar(7, chEndEmpty);
}

void updateLcd() {
    switch (g_menu_state) {
        case LcdMode::SYNCHRO:
            synchronization();
            break;
        case LcdMode::VACUUM:
            pression_diff();
            break;
        case LcdMode::ABS:
            pression_absolute();
            break;
    }
}

void loop() {
    if (g_refresh_lcd) {
        g_refresh_lcd = false;
        updateLcd();
    }
}

void synchronization() {
    static int pression_V1;
    static int pression_V2;

    if (g_program_setup) {
        lcd.createChar(0, chStartEmpty);
        lcd.createChar(1, caseEmpty);
        lcd.clear();
        lcd.print("      ");
        lcd.print(units[g_units]);
        lcd.print("      ");
        g_program_setup = false;
    }

    pression_V1 =
        map(g_vacuum_1, VACCUM_AMIN, VACCUM_AMAX, VACCUM_VMIN, VACCUM_VMAX);
    pression_V2 =
        map(g_vacuum_2, VACCUM_AMIN, VACCUM_AMAX, VACCUM_VMIN, VACCUM_VMAX) +
        g_delta;

    lcd.setCursor(0, 0);
    lcd.print("    ");
    lcd.setCursor(0, 0);
    lcd.print(convert_value(pression_V1));
    lcd.setCursor(12, 0);
    lcd.print("    ");
    lcd.setCursor(12, 0);
    align_right(convert_value(pression_V2), 4);

    set_bar(map(((pressure_V2 - pressure_V1 + 140) / 2), 0, 140, BAR_MIN, BAR_MAX));
}

void pression_diff() {
    static int pression_V1;

    if (g_program_setup) {
        lcd.createChar(0, chStartEmpty);
        lcd.createChar(1, caseEmpty);
        lcd.clear();
        lcd.print("Press.      ");
        lcd.print(units[g_units]);
        g_program_setup = false;
    }

    pression_V1 = map(g_vacuum_1, VACCUM_AMIN, VACCUM_AMAX, VACCUM_VMIN, VACCUM_VMAX) - g_pression_atmo;

    lcd.setCursor(7, 0);
    lcd.print("    ");
    lcd.setCursor(7, 0);
    lcd.print(convert_value(pression_V1));

    set_bar(
        constrain(map(pression_V1, VACCUM_DMIN, VACCUM_DMAX, BAR_MAX, BAR_MIN),
                  BAR_MIN, BAR_MAX));
}

void pression_absolute() {
    static int pression_V1;

    if (g_program_setup) {
        lcd.createChar(0, chStartEmpty);
        lcd.createChar(1, caseEmpty);
        lcd.clear();
        lcd.print("P.abs.      ");
        lcd.print(units[g_units]);
        g_program_setup = false;
    }

    pression_V1 =
        map(g_vacuum_1, VACCUM_AMIN, VACCUM_AMAX, VACCUM_VMIN, VACCUM_VMAX);
    lcd.setCursor(7, 0);
    lcd.print("    ");
    lcd.setCursor(7, 0);
    lcd.print(convert_value(pression_V1));
    lcd.setCursor(0, 1);
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

bool calibrate() {
    static uint8_t cnt = 0;
    if (++cnt == 10) {
        g_pression_atmo = map(g_vacuum_1, VACCUM_AMIN, VACCUM_AMAX, VACCUM_VMIN, VACCUM_VMAX);
        g_delta = g_pression_atmo - map(g_vacuum_2, VACCUM_AMIN, VACCUM_AMAX, VACCUM_VMIN, VACCUM_VMAX);
        cnt = 0;
        return true;
    }
    return false;
}

int convert_value(int value) {
    int ret = -1;
    switch (g_units) {
        case 0:
            ret = value;
            break;
        case 1:
            ret = convert_kPa_PSI(value);
            break;
        case 2:
            ret = convert_kPa_mmHg(value);
            break;
        case 3:
            ret = convert_kPa_inHg(value);
            break;
        default:
            ret = value;
            break;
    }
    return ret;
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
        counter = 0;
        g_refresh_lcd = true;
    }
    encoder.update();
}
