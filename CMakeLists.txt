cmake_minimum_required(VERSION 3.24)
project(MonkeyTyper)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt
    GIT_TAG 11.1.4
)

FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.0
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM
)

FetchContent_MakeAvailable(fmt SFML)

add_executable(MonkeyTyper 
    main.cpp
    Game.cpp
    components/Button.cpp
    components/Word.cpp
    Game.h
    enums/GameState.h
    enums/Difficulty.h
    enums/WordPackage.h
    components/Button.h
    components/Word.h)

target_link_libraries(MonkeyTyper PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
    fmt::fmt
)