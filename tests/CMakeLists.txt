# tests/CMakeLists.txt

# Collect all test source files (excluding unit tests and benchmarks)
file(GLOB_RECURSE TEST_SOURCES
    IntegrationTests/*.cpp
    StressTests/*.cpp
)

# Create the NovaLinkTests executable (excluding benchmark tests)
add_executable(NovaLinkTests ${TEST_SOURCES})

# Link the NovaLink library and Google Test to the test executable
target_link_libraries(NovaLinkTests PRIVATE 
    NovaLink 
    GTest::gtest
    GTest::gtest_main
)

# Add include directories for tests
target_include_directories(NovaLinkTests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests
)

# Discover tests
include(GoogleTest)
gtest_discover_tests(NovaLinkTests)

# Add subdirectories for different types of tests
add_subdirectory(UnitTests)
add_subdirectory(Benchmarking)

# Uncomment the following line when IntegrationTests are ready
# add_subdirectory(IntegrationTests)