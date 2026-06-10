cmake_minimum_required(VERSION 3.0)
project(ALS31313)

# add flags
#set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -D_BSD_SOURCE -std=c11 -Wall -pedantic -Wextra")
set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -D_BSD_SOURCE -std=c11 -pedantic -lm")



# build
include_directories ("${PROJECT_SOURCE_DIR}/src")
add_executable (als31313 src/als31313.c)
target_link_libraries(als31313)

# install
install(TARGETS als31313 DESTINATION bin)
