cmake_minimum_required(VERSION 3.10)

project(durak1)

find_package(SFML 2.6 COMPONENTS graphics window system CONFIG REQUIRED)

add_executable(durak1 main.cpp "inc/Card.h"  "inc/Game.h" "src/Game.cpp" "inc/Hand.h" "src/Hand.cpp" "inc/Deck.h" "src/Deck.cpp" "inc/AI.h" "src/AI.cpp" "inc/Actor.h" "src/Actor.cpp" "inc/Player.h" "src/Player.cpp")

target_link_libraries(durak1 PRIVATE sfml-graphics sfml-window sfml-system)

target_include_directories(durak1 PRIVATE inc)