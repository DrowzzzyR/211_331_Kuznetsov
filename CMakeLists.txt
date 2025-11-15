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
    encryptionmanager.cpp
    encryptionmanager.h
    debugprotection.cpp
    debugprotection.h
)

add_executable(${PROJECT_NAME} ${SOURCES})

find_package(OpenSSL REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE ${QT_PACKAGE}::Widgets OpenSSL::Crypto)

# Копируем папку data с JSON и зашифрованными файлами в директорию сборки
# Используем configure_file для каждого файла, чтобы обновлять при изменении
file(GLOB DATA_FILES "${CMAKE_SOURCE_DIR}/data/*.json" "${CMAKE_SOURCE_DIR}/data/*.enc")
foreach(DATA_FILE ${DATA_FILES})
    get_filename_component(DATA_NAME ${DATA_FILE} NAME)
    configure_file(${DATA_FILE} ${CMAKE_BINARY_DIR}/data/${DATA_NAME} COPYONLY)
endforeach()

# Копируем ключ шифрования в build директорию (если существует)
if(EXISTS "${CMAKE_SOURCE_DIR}/config/encryption.key")
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/config)
    configure_file("${CMAKE_SOURCE_DIR}/config/encryption.key" ${CMAKE_BINARY_DIR}/config/encryption.key COPYONLY)
endif()

