cmake_minimum_required(VERSION 3.16)
project(211_331_Kuznetsov LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

find_package(Qt6 COMPONENTS Widgets REQUIRED QUIET)
if(NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Widgets REQUIRED)
    set(QT_PACKAGE Qt5)
else()
    set(QT_PACKAGE Qt6)
endif()

set(SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE ${QT_PACKAGE}::Widgets)

# Копируем папку data с JSON файлами в директорию сборки
file(COPY ${CMAKE_SOURCE_DIR}/data DESTINATION ${CMAKE_BINARY_DIR})

