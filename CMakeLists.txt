cmake_minimum_required(VERSION 3.14)
project(TaskScheduler VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Library sources
set(LIB_SOURCES
    src/task.cpp
    src/task_queue.cpp
    src/thread_pool.cpp
    src/dependency_tracker.cpp
)

# Create static library
add_library(taskscheduler STATIC ${LIB_SOURCES})

target_include_directories(taskscheduler PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Link threads
find_package(Threads REQUIRED)
target_link_libraries(taskscheduler PUBLIC Threads::Threads)

# Enable testing
enable_testing()
add_subdirectory(tests)
