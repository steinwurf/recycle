find_package(GTest REQUIRED)

add_executable(recycle-test)
target_sources(recycle-test PRIVATE test_no_locking_policy.cpp
                                    test_resource_pool.cpp test_unique_pool.cpp)
target_link_libraries(recycle-test PRIVATE steinwurf::recycle GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(recycle-test)
