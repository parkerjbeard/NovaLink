# CMakeLists.txt for NovaLink Unit Tests

# Collect all unit test source files
file(GLOB UNIT_TEST_SOURCES *.cpp)

# Create the NovaLinkUnitTests executable
add_executable(NovaLinkUnitTests ${UNIT_TEST_SOURCES})

# Link the NovaLink library and Google Test to the test executable
target_link_libraries(NovaLinkUnitTests PRIVATE 
    NovaLink 
    GTest::GTest
    GTest::Main
)

# Add include directories for tests
target_include_directories(NovaLinkUnitTests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests
)

# Discover and register tests
include(GoogleTest)
gtest_discover_tests(NovaLinkUnitTests)