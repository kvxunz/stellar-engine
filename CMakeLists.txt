cmake_minimum_required(VERSION 3.24)
project(stellar_engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Vulkan REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(glm REQUIRED)
find_program(GLSLANG glslangValidator REQUIRED)

# ---- shaders: GLSL -> SPIR-V at build time ----
set(SHADER_SRC ${CMAKE_SOURCE_DIR}/shaders/star.vert ${CMAKE_SOURCE_DIR}/shaders/star.frag)
set(SHADER_OUT_DIR ${CMAKE_BINARY_DIR}/shaders)
set(SPIRV_FILES "")
foreach(SRC ${SHADER_SRC})
  get_filename_component(NAME ${SRC} NAME)
  set(OUT ${SHADER_OUT_DIR}/${NAME}.spv)
  add_custom_command(
    OUTPUT ${OUT}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_OUT_DIR}
    COMMAND ${GLSLANG} -V --target-env vulkan1.3 ${SRC} -o ${OUT}
    DEPENDS ${SRC}
    COMMENT "Compiling ${NAME} -> SPIR-V")
  list(APPEND SPIRV_FILES ${OUT})
endforeach()
add_custom_target(shaders DEPENDS ${SPIRV_FILES})

add_executable(stellar
  src/main.cpp
  src/universe/galaxy.cpp
  src/render/renderer.cpp)
add_dependencies(stellar shaders)
target_include_directories(stellar PRIVATE src)
target_link_libraries(stellar PRIVATE Vulkan::Vulkan glfw glm::glm)
target_compile_definitions(stellar PRIVATE
  SHADER_DIR="${SHADER_OUT_DIR}"
  GLFW_INCLUDE_VULKAN
  GLM_FORCE_DEPTH_ZERO_TO_ONE
  GLM_FORCE_RADIANS)
if(MSVC)
  target_compile_options(stellar PRIVATE /W4)
else()
  target_compile_options(stellar PRIVATE -Wall -Wextra -Wpedantic -Wno-missing-field-initializers)
endif()
