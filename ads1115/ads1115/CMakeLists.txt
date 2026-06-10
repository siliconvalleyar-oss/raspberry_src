cmake_minimum_required(VERSION 3.5)

# Define el proyecto
project(Adafruit_ADS1015_RPi)

# Define el estándar de C++
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Directorios de inclusi
include_directories(~/src/Adafruit_ADS1X15_RPi)

# Archivos objeto de la librer
set(LIBRARY_OBJECTS
    ~/src/Adafruit_ADS1X15_RPi/Adafruit_ADS1015.o
)

# Añadir la librería wiringPi
find_package(PkgConfig REQUIRED)
pkg_check_modules(WIRINGPI REQUIRED wiringPi)

include_directories(${WIRINGPI_INCLUDE_DIRS})
link_directories(${WIRINGPI_LIBRARY_DIRS})

# Archivos fuente para el ejecutable principal
set(MAIN_SOURCES
    ${CMAKE_SOURCE_DIR}/ads1115.cc  # Asegúrate de que tu archivo principal se llame main.cpp
)

# Crear un ejecutable para la aplicación principal
add_executable(ads1115_app ${MAIN_SOURCES} ${LIBRARY_OBJECTS})

# Enlazar el ejecutable con wiringPi
target_link_libraries(ads1115_app ${WIRINGPI_LIBRARIES})
