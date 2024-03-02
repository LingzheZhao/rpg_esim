cmake_minimum_required(VERSION 2.8.3)
project(replica_converter)

find_package(catkin_simple REQUIRED)
find_package(OpenCV REQUIRED)
catkin_simple()

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O3")

set(HEADERS
    include/events_converter.hpp
    include/npy.hpp
)

set(SOURCES
    src/events_converter.cpp
)

cs_add_library(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# make the executable
cs_add_executable(test_read_npy src/test_read_npy.cpp)
cs_add_executable(test_events_converter src/test_events_converter.cpp)

# link the executable to the necessary libs
target_link_libraries(test_read_npy
    ${PROJECT_NAME}
    ${catkin_LIBRARIES}
    stdc++fs
)

target_link_libraries(test_events_converter
    ${PROJECT_NAME}
    ${catkin_LIBRARIES}
    ${OpenCV_LIBRARIES}
    stdc++fs
)

# target_compile_options(test_events_converter PRIVATE -fsanitize=address)
# target_link_options(test_events_converter PRIVATE -fsanitize=address)

cs_install()
