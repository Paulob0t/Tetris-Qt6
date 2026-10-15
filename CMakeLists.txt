cmake_minimum_required(VERSION 3.20)
project(TetrisQt6 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(TetrisQt6
    src/main.cpp
    src/Piece.h
    src/Piece.cpp
    src/TetrisBoard.h
    src/TetrisBoard.cpp
    src/PiecePreviewWidget.h
    src/MainWindow.h
    src/MainWindow.cpp
)

target_link_libraries(TetrisQt6 PRIVATE Qt6::Widgets)