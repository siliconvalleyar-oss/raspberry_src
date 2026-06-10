cmake_minimum_required(VERSION 3.10)

project(OBD_APP VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Warnings
add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
)

# Optimización
set(CMAKE_CXX_FLAGS_RELEASE "-O2")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

# Directorios
include_directories(
    include
    headers
)

# 🔴 SALIDA BINARIO FUERA DE BUILD
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Fuentes
set(SOURCES
    src/main.cpp
    src/funciones.cpp
    src/comunicacion.cpp
    src/oled.cpp
    src/BluetoothManager.cpp
)

add_executable(obd_app ${SOURCES})

# Librerías
target_link_libraries(obd_app
    bcm2835
    SSD1306_OLED_RPI
)