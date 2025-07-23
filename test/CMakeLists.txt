# Test configuration for Dungeon Merc

# Find Google Test
find_package(GTest REQUIRED)

# Add test executable
add_executable(dungeon_merc_tests
    test_main.cpp
    # Add test files here as they are created
)

# Link libraries
target_link_libraries(dungeon_merc_tests
    GTest::gtest
    GTest::gtest_main
    dungeon_merc
)

# Include directories
target_include_directories(dungeon_merc_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# Add tests
add_test(NAME DungeonMercTests COMMAND dungeon_merc_tests)

# Enable CTest integration
enable_testing()
