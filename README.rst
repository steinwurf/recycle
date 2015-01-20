=======
recycle
=======

recycle is an implementation of a simple c++ resource pool.

.. contents:: Table of Contents:
   :local:

Usage
-----

The ``recycle::resource_pool`` is useful when managing expensive to
construct objects. The life-time of the managed objects is controlled
by using ``std::shared_ptr``. A custom deleter is used to reclaim
objects in the pool when the last remaining ``std::shared_ptr`` owning
the object is destroyed.

Header-only
...........

The library itself is header-only so essentially to use it you just
have to clone the repository and setup the right include paths in the
project where you would like to use it.

The library uses c++11 features such as variadic templates, so you
need a relatively recent compiler to use it.

Allocating Objects
------------------

There are two ways we can control how objects are allocated:

Using the Default Allocator
...........................

Example:

::

   #include <recycle/resource_pool.hpp>
   #include <cassert>

   struct heavy_object
   {
      // ... some expensive resource
   };


   recycle::resource_pool<heavy_object> pool;

   // Initially the pool is empty
   assert(pool.unused_resources() == 0U);

   {
       auto o1 = pool.allocate();
   }

   // Heavy object is back in the pool
   assert(pool.unused_resources() == 1U);

In this case we use the default constructor of the
``recycle::resource_pool`` this will only work if the object in this
case ``heavy_object`` is default constructible (i.e. has a constructor
which takes no arguments). Internally the resource pool uses
``std::make_shared`` to allocate the object.

Using a Custom Allocator
........................

Example:

::

   #include <recycle/resource_pool.hpp>
   #include <memory>

   struct heavy_object
   {
       heavy_object(uint32_t size);

      // ... some expensive resource
   };

   auto make = []()->std::shared_ptr<heavy_object>
        {
            return std::make_shared<heavy_object>(300000U);
        };

   recycle::resource_pool<heavy_object> pool(make);

   auto o1 = pool.allocate();

In this case we provide a custom allocator function which takes no
arguments and returns a ``std::shared_ptr``.

Recycling Objects
-----------------

When recycling objects it is sometimes necessary to ensure that
certain clean-up operations are performed before objects get stored in
the pool. This can be open file handles etc. which should be
closed. We cannot rely on the destructor for this when using a resource pool.

To support this the ``recycle::resource_pool`` support a custom
recycle function which will be called right before an object is about
to go back into the pool.

Example:

::

   #include <recycle/resource_pool.hpp>
   #include <memory>

   struct heavy_object
   {
       heavy_object(uint32_t size);

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


   recycle::resource_pool<heavy_object> pool(make, recycle);

   {
       auto o1 = pool.allocate();

       // As we exit the scope here recycle will be called
       // with o1 as argument.
   }

Thread Safety
-------------

Since the free lunch is over we want to make sure that the resource
pool is thread safe.

This can be achieved by specifying a lock policy (we were inspired by
flyweight library in Boost).

Example:

::

   #include <recycle/resource_pool.hpp>
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

   recycle::resource_pool<heavy_object, lock_policy> pool;

   // Lambda the threads will execute captures a reference to the pool
   // so they will all operate on the same pool concurrently
   auto run = [&pool]()
   {
       auto a1 = pool.allocate();
   };

   const uint32_t number_threads = 8;
   std::thread t[number_threads];

   //Launch a group of threads
   for (uint32_t i = 0; i < number_threads; ++i)
   {
       t[i] = std::thread(run);
   }

   //Join the threads with the main thread
   for (uint32_t i = 0; i < number_threads; ++i)
   {
       t[i].join();
   }
