include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Unit tests
add_executable(unit_tests
    unit/task_test.cpp
    unit/task_queue_test.cpp
    unit/thread_pool_test.cpp
    unit/dependency_tracker_test.cpp
)

target_link_libraries(unit_tests
    taskscheduler
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(unit_tests)
