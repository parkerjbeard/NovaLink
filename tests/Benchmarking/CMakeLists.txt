# Find Google Benchmark package
find_package(benchmark REQUIRED)

# Find Boost (assuming it's used in the project)
find_package(Boost REQUIRED COMPONENTS system)

# Function to create benchmark executable
function(add_benchmark_executable NAME SOURCE)
    add_executable(${NAME} ${SOURCE})
    target_link_libraries(${NAME} PRIVATE 
        NovaLink 
        benchmark::benchmark
        Boost::system
    )
    target_include_directories(${NAME} PRIVATE 
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/tests
    )
endfunction()

# Add individual benchmark executables
add_benchmark_executable(SCALPELBenchmark SCALPELBenchmark.cpp)
add_benchmark_executable(ManagementBenchmark ManagementBenchmark.cpp)
add_benchmark_executable(UtilsBenchmark UtilsBenchmark.cpp)
add_benchmark_executable(PhysicalLayerBenchmark PhysicalLayerBenchmark.cpp)

# Create a combined benchmark executable
add_executable(AllBenchmarks 
    BenchmarkMain.cpp
    SCALPELBenchmark.cpp
    ManagementBenchmark.cpp
    UtilsBenchmark.cpp
    PhysicalLayerBenchmark.cpp
)
target_link_libraries(AllBenchmarks PRIVATE 
    NovaLink 
    benchmark::benchmark
    Boost::system
)
target_include_directories(AllBenchmarks PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests
)
target_compile_definitions(AllBenchmarks PRIVATE COMBINED_BENCHMARK)

# Optionally, set compile options for benchmarks
target_compile_options(AllBenchmarks PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-O3 -march=native>
    $<$<CXX_COMPILER_ID:MSVC>:/O2>
)
