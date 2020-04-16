cmake_minimum_required(VERSION 3.15)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)

project(broke-sage-notebooks LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

if(NOT TARGET spdlog)
    # Stand-alone build
    find_package(spdlog REQUIRED)
endif()
find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(AWSSDK REQUIRED COMPONENTS ec2)


add_executable(broke-sage-notebooks
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        awsutils.cpp
        awsutils.hpp
        launchdialog.cpp
        launchdialog.h
        launchdialog.ui
        loadingscreendialog.cpp
        loadingscreendialog.h
        loadingscreendialog.ui
        selectregiondialog.cpp
        selectregiondialog.h
        selectregiondialog.ui
        errordialog.cpp
        errordialog.hpp
        errordialog.ui
)


target_compile_features(broke-sage-notebooks PUBLIC cxx_std_11)
target_link_libraries(broke-sage-notebooks
        PRIVATE Qt5::Widgets
        PRIVATE spdlog::spdlog
        ${AWSSDK_LINK_LIBRARIES}
)
