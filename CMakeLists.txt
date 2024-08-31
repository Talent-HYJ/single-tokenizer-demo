cmake_minimum_required(VERSION 3.10) 
# 设置项目名称和版本  
project(single VERSION 1.0)  
# 设置可执行文件的输出目录  
set(CMAKE_CXX_STANDARD 14)
set(CXX_STANDARD_REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")  

# 设置库文件的输出目录  
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib") 
option(SQLITE_ENABLE_FTS5         "enables full text searches version 5"         ON)
option(BUILD_SHELL "build SQLite3 shell application" ON)
# 查找SQLite3库
add_subdirectory(sqlite3)
add_library(single SHARED src/entry.cpp)
# 链接SQLite3库
target_link_libraries(single PRIVATE SQLite3)
