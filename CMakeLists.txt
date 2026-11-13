cmake_minimum_required(VERSION 3.10)
project(MyCProject)

   # Level-Dateien in den Build-Ordner kopieren, damit das Programm sie dort findet
   file(COPY ${CMAKE_SOURCE_DIR}/levels DESTINATION ${CMAKE_BINARY_DIR})

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

add_executable(a3_welt src/uebungen/a3_welt.c)
target_link_libraries(a3_welt PRIVATE tui)

add_executable(a4_level src/uebungen/a4_level.c)
target_link_libraries(a4_level PRIVATE tui)

add_executable(a5a_fahrt src/uebungen/a5a_fahrt.c)
target_link_libraries(a5a_fahrt PRIVATE tui m)

add_executable(a5b_spiel src/uebungen/a5b_spiel.c)
target_link_libraries(a5b_spiel PRIVATE tui m)