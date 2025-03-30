cmake_minimum_required(VERSION 3.10)
project(Lazarus LANGUAGES CXX)

# Fix the output directory to access models and resources
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compilation options
# - USE_GUI compiles and activates GUI written with SFML library
# - DEV compiles and activates tests from test/
option(USE_GUI "Build GUI" OFF)
option(DEV "Build & run tests" OFF)

set(SOURCES main.cpp)

# Add subdirectories
# - Very important - src/engine and src/utilities should go before src/gui
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS src/engine/*.cpp src/engine/*.cxx src/engine/*.cc)
list(APPEND SOURCES ${ENGINE_SOURCES})

file(GLOB_RECURSE UTILITIES_SOURCES CONFIGURE_DEPENDS src/utilities/*.cpp src/utilities/*.cxx src/utilities/*.cc)
list(APPEND SOURCES ${UTILITIES_SOURCES})

# Compile GUI only if USE_GUI is set up
if(USE_GUI)
    file(GLOB_RECURSE GUI_SOURCES CONFIGURE_DEPENDS src/gui/*.cpp src/gui/*.cxx src/gui/*.cc)
    list(APPEND SOURCES ${GUI_SOURCES})
endif()

# Compile tests only if DEV is set up
if(DEV)
    file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS test/*.cpp test/*.cxx test/*.cc)
    list(APPEND SOURCES ${TEST_SOURCES})
endif()

add_executable(Lazarus ${SOURCES})

# Add header files
target_include_directories(Lazarus PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/engine
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utilities
)

if(USE_GUI)
    target_compile_definitions(Lazarus PRIVATE USE_GUI)
    target_include_directories(Lazarus PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/gui)

    # Link SFML library for GUI
    find_package(SFML 2.5.1 COMPONENTS graphics window system REQUIRED)
    target_link_libraries(Lazarus PRIVATE SFML::Graphics SFML::Window SFML::System)
endif()

if(DEV)
    target_compile_definitions(Lazarus PRIVATE DEV)
    target_include_directories(Lazarus PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/test)
endif()