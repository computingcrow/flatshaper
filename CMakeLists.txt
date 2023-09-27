cmake_minimum_required(VERSION 3.20)
project(flatshaper LANGUAGES C CXX)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLM REQUIRED IMPORTED_TARGET glm>=0.9.9)
pkg_check_modules(GLFW3 REQUIRED IMPORTED_TARGET glfw3>=3.3.5)
pkg_check_modules(DevIL REQUIRED IMPORTED_TARGET IL)


#### GLAD library ####
set(GLAD_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(GLAD_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)

set(GLAD_SOURCES ${GLAD_SOURCE_DIR}/glad.c)
set(GLAD_INCLUDES
    ${GLAD_INCLUDE_DIR}/glad/glad.h
    ${GLAD_INCLUDE_DIR}/KHR/khrplatform.h)

add_library(GLAD ${GLAD_SOURCES} ${GLAD_INCLUDES})
target_compile_features(GLAD PRIVATE c_std_90)
target_include_directories(GLAD PUBLIC ${GLAD_INCLUDE_DIR})


#### flatshaper executable ####
set(FLATSHAPER_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(FLATSHAPER_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)

set(FLATSHAPER_SOURCES
    ${FLATSHAPER_SOURCE_DIR}/main.cpp)

set(FLATSHAPER_INCLUDES
    ${FLATSHAPER_INCLUDE_DIR}/flatshaper/main.hpp)

add_executable(flatshaper ${FLATSHAPER_SOURCES} ${FLATSHAPER_INCLUDES})
target_compile_features(flatshaper PRIVATE cxx_std_17)
target_include_directories(flatshaper PUBLIC ${FLATSHAPER_INCLUDE_DIR})
target_link_libraries(flatshaper PUBLIC PkgConfig::GLFW3)
target_link_libraries(flatshaper PUBLIC GLAD)
target_link_libraries(flatshaper PUBLIC PkgConfig::GLM)
target_link_libraries(flatshaper PUBLIC PkgConfig::DevIL)
