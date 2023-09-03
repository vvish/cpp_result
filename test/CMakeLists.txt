add_executable(
    unit-tests
    result_test.cpp
)

enable_testing()

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

set_target_properties(unit-tests PROPERTIES CXX_STANDARD 14)
target_link_libraries(unit-tests GTest::gtest_main)
target_compile_options(unit-tests PRIVATE -O0 --coverage -g)
target_link_options(unit-tests PRIVATE --coverage)

include(GoogleTest)

gtest_discover_tests(unit-tests)
