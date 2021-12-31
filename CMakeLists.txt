cmake_minimum_required (VERSION 3.9)

project (pi-client)

find_package (OpenCV REQUIRED)
find_package (Boost REQUIRED COMPONENTS thread date_time)
find_package (OpenMP REQUIRED)

include_directories (${OpenCV_INCLUDE_DIRS})
include_directories (${Boost_INCLUDE_DIRS})

add_executable (pi-client
    pi-client.cpp
    SocketMgr.cpp)

target_link_libraries (pi-client
    ${OpenCV_LIBS}
    ${Boost_THREAD_LIBRARY}
    ${Boost_DATE_TIME_LIBRARY}
    OpenMP::OpenMP_CXX)
