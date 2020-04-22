cmake_minimum_required(VERSION 3.15)

set(PROJECT_NAME Diogenes)
project(${PROJECT_NAME} LANGUAGES CXX)

# Some generic settings
set(CMAKE_INCLUDE_CURRENT_DIR ON)
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

# Some settings to build Qt
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

# Custom paths
set(prefix "${PROJECT_NAME}.app/Contents")
set(INSTALL_RUNTIME_DIR "${prefix}/MacOS")
set(INSTALL_CMAKE_DIR "${prefix}/Resources")
set(APPS "${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}.app")  # Note Mac specific extension .app
set(DIRS "${CMAKE_BINARY_DIR}")  # Directories to look for dependencies

# Find packages
if(NOT TARGET spdlog)
    find_package(spdlog REQUIRED)
endif()
find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(AWSSDK REQUIRED COMPONENTS ec2)


# add executable
add_executable(${PROJECT_NAME} MACOSX_BUNDLE
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

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

target_link_libraries(${PROJECT_NAME}
        PRIVATE Qt5::Widgets
        PRIVATE spdlog::spdlog
        ${AWSSDK_LINK_LIBRARIES}
        )

# based on code from CMake's QtDialog/CMakeLists.txt
macro(install_qt5_plugin _qt_plugin_name _qt_plugins_var _prefix)
    get_target_property(_qt_plugin_path "${_qt_plugin_name}" LOCATION)
    if(EXISTS "${_qt_plugin_path}")
        get_filename_component(_qt_plugin_file "${_qt_plugin_path}" NAME)
        get_filename_component(_qt_plugin_type "${_qt_plugin_path}" PATH)
        get_filename_component(_qt_plugin_type "${_qt_plugin_type}" NAME)
        set(_qt_plugin_dest "${_prefix}/PlugIns/${_qt_plugin_type}")
        install(FILES "${_qt_plugin_path}"
                DESTINATION "${_qt_plugin_dest}")
        set(${_qt_plugins_var}
                "${${_qt_plugins_var}};${CMAKE_INSTALL_PREFIX}/${_qt_plugin_dest}/${_qt_plugin_file}")
    else()
        message(FATAL_ERROR "QT plugin ${_qt_plugin_name} not found")
    endif()
endmacro()

install_qt5_plugin("Qt5::QCocoaIntegrationPlugin" QT_PLUGINS ${prefix})
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/qt.conf"
        "[Paths]\nPlugins = ${_qt_plugin_dir}\n")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/qt.conf"
        DESTINATION "${INSTALL_CMAKE_DIR}")

# Destination paths below are relative to ${CMAKE_INSTALL_PREFIX}
install(TARGETS ${PROJECT_NAME}
        BUNDLE DESTINATION . COMPONENT Runtime
        RUNTIME DESTINATION ${INSTALL_RUNTIME_DIR} COMPONENT Runtime
        )

# Path used for searching by FIND_XXX(), with appropriate suffixes added
if(CMAKE_PREFIX_PATH)
    foreach(dir ${CMAKE_PREFIX_PATH})
        list(APPEND DIRS "${dir}/bin" "${dir}/lib")
    endforeach()
endif()

# Append Qt's lib folder which is two levels above Qt5Widgets_DIR
list(APPEND DIRS "${Qt5Widgets_DIR}/../..")

# Print some info
message("PROJECT_NAME = ${PROJECT_NAME}")
message("CMAKE_INSTALL_PREFIX = ${CMAKE_INSTALL_PREFIX}")
message("AWSSDK_LINK_LIBRARIES = ${AWSSDK_LINK_LIBRARIES}")
message("INSTALL_RUNTIME_DIR = ${INSTALL_RUNTIME_DIR}")
message("CMAKE_BINARY_DIR = ${CMAKE_BINARY_DIR}")
message("Qt5Widgets_DIR = ${Qt5Widgets_DIR}")
message("APPS = ${APPS}")
message("DIRS = ${DIRS}")
message("QT_PLUGINS = ${QT_PLUGINS}")

include(InstallRequiredSystemLibraries)

install(CODE "include(BundleUtilities)
    fixup_bundle(\"${APPS}\" \"${QT_PLUGINS}\" \"${DIRS}\")")

set(CPACK_GENERATOR "DRAGNDROP")
include(CPack)
