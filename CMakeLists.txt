cmake_minimum_required(VERSION 3.0)
project(node_printer)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fexec-charset=UTF-8")
set(CMAKE_CXX_STANDARD 14)
include_directories(C:/Users/ASUS/AppData/Local/node-gyp/Cache/16.16.0/include/node)

add_executable(node_printer main.cpp printer/printer.cpp getPrinterList.cpp)
