cmake_minimum_required(VERSION 3.10)
project(MyCProject)

set(CMAKE_C_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)

add_executable(MyCProject-ex src/main.c)


add_library(tui STATIC src/tui/tui.c src/tui/tui.h)
target_include_directories(tui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/tui)
target_compile_options(tui PRIVATE -Wall -Wextra)

add_executable(a1_hallo src/uebungen/a1_hallo.c)
target_link_libraries(a1_hallo PRIVATE tui)

add_executable(a2_uboot src/uebungen/a2_uboot.c)
target_link_libraries(a2_uboot PRIVATE tui)

add_executable(a2_uboot src/uebungen/a3_welt.c)
target_link_libraries(a3_welt PRIVATE tui)