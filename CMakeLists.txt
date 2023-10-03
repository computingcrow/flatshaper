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
    ${FLATSHAPER_SOURCE_DIR}/main.cpp
    ${FLATSHAPER_SOURCE_DIR}/entity.cpp
    ${FLATSHAPER_SOURCE_DIR}/glutil.cpp
    ${FLATSHAPER_SOURCE_DIR}/plyutil.cpp

    ${FLATSHAPER_SOURCE_DIR}/systems/system_physics.cpp
    ${FLATSHAPER_SOURCE_DIR}/systems/render/system_render.cpp)

set(FLATSHAPER_INCLUDES
    ${FLATSHAPER_INCLUDE_DIR}/flatshaper/main.hpp
    ${FLATSHAPER_INCLUDE_DIR}/flatshaper/entity.hpp
    ${FLATSHAPER_INCLUDE_DIR}/flatshaper/glutil.hpp
    ${FLATSHAPER_INCLUDE_DIR}/flatshaper/plyutil.hpp

    ${FLATSHAPER_INCLUDE_DIR}/flatshaper/systems/system_physics.hpp
    ${FLATSHAPER_INCLUDE_DIR}/flatshaper/systems/render/system_render.hpp)

add_executable(flatshaper ${FLATSHAPER_SOURCES} ${FLATSHAPER_INCLUDES})
target_compile_features(flatshaper PRIVATE cxx_std_17)
target_include_directories(flatshaper PUBLIC ${FLATSHAPER_INCLUDE_DIR})
target_link_libraries(flatshaper PUBLIC PkgConfig::GLFW3)
target_link_libraries(flatshaper PUBLIC GLAD)
target_link_libraries(flatshaper PUBLIC PkgConfig::GLM)
target_link_libraries(flatshaper PUBLIC PkgConfig::DevIL)

if (DEFINED CMAKE_BUILD_TYPE AND ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    target_compile_definitions(flatshaper PRIVATE "FLATSHAPER_DEBUG_GL")
endif()

configure_file(assets/assets.csv assets/assets.csv COPYONLY)
configure_file(assets/shaders/VertexShader.glsl assets/shaders/VertexShader.glsl COPYONLY)
configure_file(assets/shaders/FragmentShader.glsl assets/shaders/FragmentShader.glsl COPYONLY)
configure_file(assets/levels/lv1/assets.csv assets/levels/lv1/assets.csv COPYONLY)
configure_file(assets/models/sprite.ply assets/models/sprite.ply COPYONLY)
configure_file(assets/models/sprite_ascii.ply assets/models/sprite_ascii.ply COPYONLY)
configure_file(assets/models/flat_diffuse.png assets/models/flat_diffuse.png COPYONLY)
configure_file(assets/models/neutral_normal.png assets/models/neutral_normal.png COPYONLY)
