News for recycle
================

This file lists the major changes between versions. For a more detailed list of
every change, see the Git log.

Latest
------
* tbd

7.0.0
-----
* Major: Use waf-tools 5.
* Minor: Updated waf.

6.0.0
-----
* Major: Change cmake build to be object library based.

5.1.0
-----
* Minor: Added install step to CMake.

5.0.0
-----
* Major: Use std::size_t for size and positions.

4.1.2
-----
* Patch: Fix ``target_compile_features``.

4.1.1
-----
* Patch: Added ``target_compile_features`` to CMake script so that c++14 is
  used.

4.1.0
-----
* Minor: Added CMake build file.

4.0.0
-----
* Major: Rename resource_pool to shared_pool
* Minor: Adding unique_pool

3.0.0
-----
* Major: Upgrade to waf-tools 4
* Minor: Upgrade to gtest 4

2.0.0
-----
* Major: Upgrade to waf-tools 3
* Minor: Upgrade to gtest 3

1.2.0
-----
* Patch: Fix a memory leak caused by a circular dependency when using objects
  inheriting from ``std::enable_shared_from_this``.
* Minor: Added buildbot.py for coverage reports.
* Patch: Fixed comparison warnings in unit tests.

1.1.1
-----
* Patch: Fix version define.

1.1.0
-----
* Minor: Added version define.

1.0.1
-----
* Patch: Added test for no_locking_policy.hpp
* Patch: Fixed includes

1.0.0
-----
* Major: Initial release of the project.
