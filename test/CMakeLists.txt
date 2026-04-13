include(CTest)
enable_testing()

include(Catch)

add_executable(unit_tests
#TODO
)

# Link libraries we are testing + Catch2
target_link_libraries(unit_tests PRIVATE #TODO

)

# Auto-discover test cases using Catch2's --list-tests command
catch_discover_tests(unit_tests)


