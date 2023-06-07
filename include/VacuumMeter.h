#pragma once

#include <Arduino.h>

constexpr int VACCUM_1 = A2;
constexpr int VACCUM_2 = A0;
constexpr float FILTER_CONSTANT = 0.9945;

constexpr int BAR_MIN = 0;
constexpr int BAR_MAX = 70;

constexpr unsigned int VACCUM_AMIN = 41U;
constexpr unsigned int VACCUM_AMAX = 982U;
constexpr unsigned int VACCUM_VMIN = 200U;
constexpr unsigned int VACCUM_VMAX = 2500U;

/// @brief Show synchronization status (2 strokes)
void synchronization();

/// @brief Set bar graph value
/// @param value value in the range [0 - 70]
void set_bar(int value);

/// @brief Align text to right
/// @param value Value to show
/// @param max max length
void align_right(int value, int max);

/// @brief Calibrate inputs
/// @details Both inputs must not be connected to any source.
/// Atmospheric pressure is stored.
/// Delta is calculated between two inputs.
/// @return True if calibration has finished
bool calibrate();

/// @brief Refresh LCD screen
void updateLcd();