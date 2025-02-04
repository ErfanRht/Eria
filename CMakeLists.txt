cmake_minimum_required(VERSION 3.29)
project(IDE_Project)

set(CMAKE_CXX_STANDARD 20)
add_executable(IDE_Project main.cpp)
target_link_libraries(IDE_Project -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer)
