cmake_minimum_required(VERSION 3.16)
project(obd2_rpi VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ─── Opciones de compilación ────────────────────────────────────────────────
option(BUILD_TESTS    "Compilar tests"        OFF)
option(ENABLE_GM      "Soporte GM modo 22"    ON)
option(ENABLE_LOGGING "Logging CSV"           ON)
option(INSTALL_SERVICE "Instalar systemd"     OFF)

# ─── Flags de optimización ──────────────────────────────────────────────────
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Wextra")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# ─── Dependencias ───────────────────────────────────────────────────────────
find_package(Threads REQUIRED)

find_library(GPIOD_LIB gpiod REQUIRED)
find_path(GPIOD_INCLUDE gpiod.h REQUIRED)

include_directories(include ${GPIOD_INCLUDE})

# ─── Fuentes ────────────────────────────────────────────────────────────────
file(GLOB SOURCES src/*.cpp)

add_executable(obd2_rpi ${SOURCES})

target_link_libraries(obd2_rpi
    bluetooth
    Threads::Threads
    ${GPIOD_LIB}
)

# ─── Instalación ────────────────────────────────────────────────────────────
install(TARGETS obd2_rpi RUNTIME DESTINATION bin)
install(DIRECTORY config/ DESTINATION etc/obd2_rpi)
install(FILES obd2_rpi.service DESTINATION lib/systemd/system)

# ─── Tests ──────────────────────────────────────────────────────────────────
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

message(STATUS "Version       : ${PROJECT_VERSION}")
message(STATUS "Binario       : ${CMAKE_SOURCE_DIR}/bin/obd2_rpi")
message(STATUS "GM soporte    : ${ENABLE_GM}")
message(STATUS "Tests         : ${BUILD_TESTS}")
