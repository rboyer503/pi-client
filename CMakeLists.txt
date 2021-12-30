cmake_minimum_required (VERSION 2.8)

project (pi-client)

find_package (OpenCV REQUIRED)
find_package (Boost REQUIRED COMPONENTS thread date_time)

include_directories (${OpenCV_INCLUDE_DIRS})
include_directories (${Boost_INCLUDE_DIRS})

add_executable (pi-client
    pi-client.cpp
    SocketMgr.cpp)

target_link_libraries (pi-client
    ${OpenCV_LIBS}
    ${Boost_THREAD_LIBRARY}
    ${Boost_DATE_TIME_LIBRARY})
