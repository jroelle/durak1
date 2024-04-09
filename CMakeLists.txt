cmake_minimum_required(VERSION 3.10)

project(durak1)

set(SFML_STATIC_LIBRARIES TRUE)

find_package(SFML 2.6 COMPONENTS graphics window system CONFIG REQUIRED)

add_executable(durak1 main.cpp
					"inc/Bot.h"
					"src/Bot.cpp"
					"inc/Card.h"
					"src/Card.cpp"
					"inc/Color.h"
					"inc/Context.h"
					"src/Context.cpp"
					"inc/Deck.h"
					"src/Deck.cpp"
					"inc/Drawing.h"
					"src/Drawing.cpp"
					"inc/Event.hpp"
					"inc/Game.h"
					"src/Game.cpp"
					"inc/Hand.h"
					"src/Hand.cpp"
					"inc/IController.h"
					"inc/Player.h"
					"src/Player.cpp"
					"inc/PlayersGroup.h"
					"src/PlayersGroup.cpp"
					"inc/Round.h"
					"src/Round.cpp"
					"inc/UI.h"
					"src/UI.cpp"
					"inc/User.h"
					"src/User.cpp"
					"inc/Utility.hpp"
 "inc/Vector.h")

target_link_libraries(durak1 PRIVATE sfml-graphics sfml-window sfml-system)

target_include_directories(durak1 PRIVATE inc)

target_compile_features(durak1 PRIVATE cxx_std_20)