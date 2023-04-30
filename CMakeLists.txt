cmake_minimum_required(VERSION 3.20)

###########################################################################################

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${CMAKE_SOURCE_DIR}/.git")
# Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/arduino-cmake")
    message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif()

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/arduino-cmake/cmake/Arduino-Toolchain.cmake")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

###########################################################################################

project(VacuumMeter VERSION 1.0.0)
option(DEBUG "Compile Debug serial" OFF)

if(DEBUG)
    add_compile_definitions("DEBUG")
endif()

arduino_cmake_project(${PROJECT_NAME} BOARD_NAME nano BOARD_CPU atmega328)

add_arduino_executable(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(src)

find_arduino_library(LIB_LiquidCrystal_I2C LiquidCrystal_I2C 3RD_PARTY)
find_arduino_library(LIB_Bounce2 Bounce2 3RD_PARTY)
find_arduino_library(LIB_Encoder Encoder 3RD_PARTY)
find_arduino_library(LIB_EncoderButton EncoderButton 3RD_PARTY)
find_arduino_library(LIB_LcdMenu LcdMenu 3RD_PARTY HEADER_ONLY)

link_arduino_library(LIB_EncoderButton LIB_Bounce2)
link_arduino_library(LIB_EncoderButton LIB_Encoder)

link_arduino_library(${PROJECT_NAME} LIB_LiquidCrystal_I2C)
link_arduino_library(${PROJECT_NAME} LIB_EncoderButton)
link_arduino_library(${PROJECT_NAME} LIB_LcdMenu HEADER_ONLY)

set_target_upload_port(${PROJECT_NAME} /dev/ttyUSB0)
