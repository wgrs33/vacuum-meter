cmake_minimum_required(VERSION 3.8.2)

project(VacuumMeter)

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
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

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/arduino-cmake")
    message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif()

arduino_cmake_project(${PROJECT_NAME} BOARD_NAME nano BOARD_CPU atmega168)

add_arduino_executable(${PROJECT_NAME} ${PROJECT_NAME}.cpp)

find_arduino_library(liquid_Crystal LiquidCrystal_I2C 3RD_PARTY)
link_arduino_library(${PROJECT_NAME} liquid_Crystal)

set_target_upload_port(${PROJECT_NAME} /dev/ttyUSB0)
