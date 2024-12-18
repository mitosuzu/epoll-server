cmake_minimum_required(VERSION 3.10)
project(MyServer)

set(SOURCES "./src/main.c"
            "./src/ThreadPool.c"
            "./src/server.c"
            "./src/dboperator.c"
            "./src/globalinfo.c"
            )
            
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

include_directories("${CMAKE_SOURCE_DIR}/include")

add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})
target_link_libraries(${CMAKE_PROJECT_NAME} sqlite3)