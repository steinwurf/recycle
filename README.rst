recycle
=======

|Linux make-specs| |Windows make-specs| |MacOS make-specs| |Linux CMake| |Windows CMake| |MacOS CMake| |Raspberry Pi| |Valgrind| |No Assertions| |Clang Format| |Cppcheck|

.. |Linux make-specs| image:: https://github.com/steinwurf/recycle/actions/workflows/linux_mkspecs.yml/badge.svg
   :target: https://github.com/steinwurf/recycle/actions/workflows/linux_mkspecs.yml
   
.. |Windows make-specs| image:: https://github.com/steinwurf/recycle/actions/workflows/windows_mkspecs.yml/badge.svg
   :target: https://github.com/steinwurf/recycle/actions/workflows/windows_mkspecs.yml

.. |MacOS make-specs| image:: https://github.com/steinwurf/recycle/actions/workflows/macos_mkspecs.yml/badge.svg
   :target: https://github.com/steinwurf/recycle/actions/workflows/macos_mkspecs.yml
   
.. |Linux CMake| image:: https://github.com/steinwurf/recycle/actions/workflows/linux_cmake.yml/badge.svg
   :target: https://github.com/steinwurf/recycle/actions/workflows/linux_cmake.yml

.. |Windows CMake| image:: https://github.com/steinwurf/recycle/actions/workflows/windows_cmake.yml/badge.svg
   :target: https://github.com/steinwurf/recycle/actions/workflows/windows_cmake.yml
   
.. |MacOS CMake| image:: https://github.com/steinwurf/recycle/actions/workflows/macos_cmake.yml/badge.svg
   :target: https://github.com/steinwurf/recycle/actions/workflows/macos_cmake.yml
   
.. |Raspberry Pi| image:: https://github.com/steinwurf/recycle/actions/workflows/raspberry_pi.yml/badge.svg
   :target: https://github.com/steinwurf/recycle/actions/workflows/raspberry_pi.yml

.. |Clang Format| image:: https://github.com/steinwurf/recycle/actions/workflows/clang-format.yml/badge.svg
   :target: https://github.com/steinwurf/recycle/actions/workflows/clang-format.yml

.. |No Assertions| image:: https://github.com/steinwurf/recycle/actions/workflows/nodebug.yml/badge.svg
   :target: https://github.com/steinwurf/recycle/actions/workflows/nodebug.yml

.. |Valgrind| image:: https://github.com/steinwurf/recycle/actions/workflows/valgrind.yml/badge.svg
   :target: https://github.com/steinwurf/recycle/actions/workflows/valgrind.yml

.. |Cppcheck| image:: https://github.com/steinwurf/recycle/actions/workflows/cppcheck.yml/badge.svg
   :target: https://github.com/steinwurf/recycle/actions/workflows/cppcheck.yml

recycle is an implementation of a simple C++ resource pool.

.. contents:: Table of Contents:
   :local:

Usage
-----

The ``recycle`` project contains two types of resource pools:

1. The ``recycle::shared_pool`` is useful when managing expensive to
   construct objects. The life-time of the managed objects is controlled
   by using ``std::shared_ptr``. A custom deleter is used to reclaim
   objects in the pool when the last remaining ``std::shared_ptr`` owning
   the object is destroyed.

2. The ``recycle::unique_pool`` works the same way as the
   ``recycle::shared_pool`` but instead uses ``std::unique_ptr`` for
   managing the resources. Still we need a custom deleter - with
   ``std::unique_ptr`` this has to be part of the type. So the
   ``std::unique_ptr`` returned by ``recycle::unique_pool`` is
   of type ``recycle::unique_pool::pool_ptr``.

Besides the fact that ``recycle::shared_pool`` manages ``std::shared_ptr`` and
``recycle::unique_pool`` manages ``std::unique_ptr`` the API should be the
same. So in the following you can replace ``shared`` with ``unique`` to
swap the behavior.

Header-only
...........

The library itself is header-only so essentially to use it you just
have to clone the repository and setup the right include paths in the
project where you would like to use it.

The library uses C++14 features, so you need a relatively recent compiler
to use it.

Allocating Objects
------------------

There are two ways we can control how objects are allocated:

Using the Default Allocator
...........................

Example:

.. code-block:: cpp

   #include <recycle/shared_pool.hpp>
   #include <cassert>

   struct heavy_object
   {
      // ... some expensive resource
   };


   recycle::shared_pool<heavy_object> pool;

   // Initially the pool is empty
   assert(pool.unused_resources() == 0U);

   {
       auto o1 = pool.allocate();
   }

   // Heavy object is back in the pool
   assert(pool.unused_resources() == 1U);

In this case we use the default constructor of the
``recycle::shared_pool`` this will only work if the object in this
case ``heavy_object`` is default constructible (i.e. has a constructor
which takes no arguments). Internally the resource pool uses
``std::make_shared`` to allocate the object.

Using a Custom Allocator
........................

Example:

.. code-block:: cpp

   #include <recycle/shared_pool.hpp>
   #include <memory>

   struct heavy_object
   {
       heavy_object(std::size_t size);

      // ... some expensive resource
   };

   auto make = []()->std::shared_ptr<heavy_object>
        {
            return std::make_shared<heavy_object>(300000U);
        };

   recycle::shared_pool<heavy_object> pool(make);

   auto o1 = pool.allocate();

In this case we provide a custom allocator function which takes no
arguments and returns a ``std::shared_ptr``.

Recycling Objects
-----------------

When recycling objects it is sometimes necessary to ensure that
certain clean-up operations are performed before objects get stored in
the pool. This can be open file handles etc. which should be
closed. We cannot rely on the destructor for this when using a resource pool.

To support this the ``recycle::shared_pool`` support a custom
recycle function which will be called right before an object is about
to go back into the pool.

Example:

.. code-block:: cpp

   #include <recycle/shared_pool.hpp>
   #include <memory>

   struct heavy_object
   {
       heavy_object(std::size_t size);

      // ... some expensive resource
   };

   auto make = []()->std::shared_ptr<heavy_object>
        {
            return std::make_shared<heavy_object>(300000U);
        };

   auto recycle = [](std::shared_ptr<heavy_object> o)
        {
            o->close_sockets();
        };


   recycle::shared_pool<heavy_object> pool(make, recycle);

   {
       auto o1 = pool.allocate();

       // As we exit the scope here recycle will be called
       // with o1 as argument.
   }

Thread Safety
-------------

Since the free lunch is over we want to make sure that the resource
pool is thread safe.

This can be achieved by specifying a lock policy (we were inspired by the
flyweight library in Boost).

Example:

.. code-block:: cpp

   #include <recycle/shared_pool.hpp>
   #include <mutex>
   #include <thread>

   struct heavy_object
   {
      // ... some expensive resource
   };

   struct lock_policy
   {
       using mutex_type = std::mutex;
       using lock_type = std::lock_guard<mutex_type>;
   };

   recycle::shared_pool<heavy_object, lock_policy> pool;

   // Lambda the threads will execute captures a reference to the pool
   // so they will all operate on the same pool concurrently
   auto run = [&pool]()
   {
       auto a1 = pool.allocate();
   };

   const std::size_t number_threads = 8;
   std::thread t[number_threads];

   //Launch a group of threads
   for (std::size_t i = 0; i < number_threads; ++i)
   {
       t[i] = std::thread(run);
   }

   //Join the threads with the main thread
   for (std::size_t i = 0; i < number_threads; ++i)
   {
       t[i].join();
   }

Use as Dependency in CMake
--------------------------

To depend on this project when using the CMake build system, add the following
in your CMake build script:

.. code-block:: cmake

   add_subdirectory("/path/to/recycle" recycle)
   target_link_libraries(<my_target> steinwurf::recycle)

Where ``<my_target>`` is replaced by your target.
