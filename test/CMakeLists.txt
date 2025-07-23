# Test configuration for Dungeon Merc

# Find Google Test (optional)
find_package(GTest QUIET)

if(GTest_FOUND)
    # Add test executable
    add_executable(dungeon_merc_tests
        test_main.cpp
        # Add test files here as they are created
    )

    # Link libraries
    target_link_libraries(dungeon_merc_tests
        GTest::gtest
        GTest::gtest_main
    )

    # Include directories
    target_include_directories(dungeon_merc_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/include
    )

    # Add tests
    add_test(NAME DungeonMercTests COMMAND dungeon_merc_tests)

    message(STATUS "Google Test found - tests enabled")
else()
    message(STATUS "Google Test not found - tests disabled")
endif()

# Enable CTest integration
enable_testing()
