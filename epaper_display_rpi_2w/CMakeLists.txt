cmake_minimum_required(VERSION 3.10)
project(EPaperProject)

# Configura el compilador a Clang
if (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(STATUS "Configuring Clang compiler")
    set(CMAKE_CXX_COMPILER "clang++")
    set(CMAKE_C_COMPILER "clang")
endif()

set(CMAKE_CXX_STANDARD 14)

# Añadir todos los archivos fuente
add_executable(app_epaper main.cc spi.cc epaper.cc)

# Configurar el enlace de bibliotecas, si necesitas alguna, como wiringPi
target_link_libraries(app_epaper)
