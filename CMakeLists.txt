cmake_minimum_required(VERSION 3.21)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(RayTracer VERSION 1.0)

if(WIN32)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w -Wall -lopengl32 -lglfw3 -lglew32 -lgdi32 -fopenmp")

endif()

set(LIB_NAME ${PROJECT_NAME})

add_subdirectory(src)

target_include_directories(${LIB_NAME}_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_dependencies(${LIB_NAME}_run ${LIB_NAME}_lib)

target_link_libraries(${LIB_NAME}_run ${LIB_NAME}_lib)

