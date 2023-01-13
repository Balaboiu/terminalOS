cmake_minimum_required(VERSION 3.23)
project(TerminalOS C)


set(CMAKE_C_STANDARD 99)


add_executable(TerminalOS ${SOURCES} main.c)
add_executable(cp _cp.c)
add_executable(tee _tee.c)
add_executable(dirname _dirname.c)

target_link_libraries(TerminalOS /lib/x86_64-linux-gnu/libreadline.so)


