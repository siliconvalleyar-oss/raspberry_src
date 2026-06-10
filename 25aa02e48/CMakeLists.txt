cmake_minimum_required(VERSION 3.5)

project(app_25AA)

set(CMAKE_CXX_STANDARD 17)

#find_library(BCM2835_LIB bcm2835)

add_executable(app_25aa main.cc)

target_link_libraries(app_25aa)

#target_link_libraries(app_25aa ${BCM2835_LIB})
