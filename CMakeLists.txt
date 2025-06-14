cmake_minimum_required(VERSION 3.15)

project(locally LANGUAGES CXX)

# Estándar de C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Archivos fuente
set(SOURCES
    main.cpp
    src/log.cpp
    src/config.cpp
    src/utils.cpp
    src/websocket.cpp
    src/filewatcher.cpp
    src/apihandler.cpp
    src/httpfilehandler.cpp
    src/server.cpp
)

include_directories(include)

add_executable(locally ${SOURCES})

# ZLIB (MSYS2 UCRT)
set(ZLIB_ROOT "C:/msys64/ucrt64")
set(ZLIB_INCLUDE_DIR "${ZLIB_ROOT}/include")
set(ZLIB_LIBRARY "${ZLIB_ROOT}/lib/libz.a")

find_package(ZLIB REQUIRED)
target_link_libraries(locally C:/msys64/ucrt64/lib/libz.a)

# Librerías por plataforma
if(WIN32)
    target_link_libraries(locally ws2_32 wsock32 ssl crypto)
else()
    target_link_libraries(locally ssl crypto)
endif()
