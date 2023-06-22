// Copyright Steinwurf ApS 2014.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#include <recycle/unique_pool.hpp>

#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <type_traits>

#include <gtest/gtest.h>

// Put tests classes in an anonymous namespace to avoid violations of
// ODF (one-definition-rule) in other translation units
namespace
{
// Default constructible dummy object
struct dummy_one
{
    dummy_one()
    {
        ++m_count;
    }

    ~dummy_one()
    {
        --m_count;
    }

    // Counter which will check how many object have been allocate
    // and deallocated
    static int32_t m_count;
};

int32_t dummy_one::m_count = 0;

std::unique_ptr<dummy_one> make_dummy_one()
{
    return std::make_unique<dummy_one>();
}

// Non Default constructible dummy object
struct dummy_two
{
    dummy_two(std::size_t)
    {
        ++m_count;
    }

    ~dummy_two()
    {
        --m_count;
    }

    static int32_t m_count;
};

int32_t dummy_two::m_count = 0;

std::unique_ptr<dummy_two> make_dummy_two(std::size_t v)
{
    return std::make_unique<dummy_two>(v);
}

// enable_shared_from_this dummy object
struct dummy_three : std::enable_shared_from_this<dummy_three>
{

    dummy_three()
    {
        ++m_count;
    }

    ~dummy_three()
    {
        --m_count;
    }

    // Counter which will check how many object have been allocate
    // and deallocated
    static int32_t m_count;
};

int32_t dummy_three::m_count = 0;
}

/// Test that our resource pool is a regular type. We are not
/// implementing equality or less than here, but maybe we could.
namespace
{
/// This code checks whether a type is regular or not. See the
/// Eric Niebler's talk from C++Now
/// 2014. http://youtu.be/zgOF4NrQllo
template <class T>
struct is_regular
    : std::integral_constant<bool, std::is_default_constructible<T>::value &&
                                       std::is_copy_constructible<T>::value &&
                                       std::is_move_constructible<T>::value &&
                                       std::is_copy_assignable<T>::value &&
                                       std::is_move_assignable<T>::value>
{
};
}

TEST(test_unique_pool, regular_type)
{
    EXPECT_TRUE(is_regular<recycle::unique_pool<dummy_one>>::value);
    EXPECT_FALSE(is_regular<recycle::unique_pool<dummy_two>>::value);
}

/// Test the basic API construct and free some objects
TEST(test_unique_pool, api)
{
    {
        recycle::unique_pool<dummy_one> pool;

        EXPECT_EQ(pool.unused_resources(), 0U);

        {
            auto d1 = pool.allocate();
            EXPECT_EQ(pool.unused_resources(), 0U);
        }

        EXPECT_EQ(pool.unused_resources(), 1U);

        auto d2 = pool.allocate();

        EXPECT_EQ(pool.unused_resources(), 0U);

        auto d3 = pool.allocate();

        EXPECT_EQ(pool.unused_resources(), 0U);
        EXPECT_EQ(dummy_one::m_count, 2);

        {
            auto d4 = pool.allocate();
            EXPECT_EQ(pool.unused_resources(), 0U);
        }

        EXPECT_EQ(pool.unused_resources(), 1U);

        pool.free_unused();

        EXPECT_EQ(pool.unused_resources(), 0U);
    }

    EXPECT_EQ(dummy_one::m_count, 0);
}

/// Test the pool works with std::bind
TEST(test_unique_pool, bind)
{
    {
        recycle::unique_pool<dummy_one> pool_one(std::bind(make_dummy_one));

        recycle::unique_pool<dummy_two> pool_two(std::bind(make_dummy_two, 4U));

        auto o1 = pool_one.allocate();
        auto o2 = pool_two.allocate();

        EXPECT_EQ(dummy_one::m_count, 1);
        EXPECT_EQ(dummy_two::m_count, 1);
    }

    EXPECT_EQ(dummy_one::m_count, 0);
    EXPECT_EQ(dummy_two::m_count, 0);
}

/// Test that the pool works for non default constructable objects, if
/// we provide the allocator
TEST(test_unique_pool, non_default_constructable)
{
    {
        recycle::unique_pool<dummy_two> pool(std::bind(make_dummy_two, 4U));

        auto o1 = pool.allocate();
        auto o2 = pool.allocate();

        EXPECT_EQ(dummy_two::m_count, 2);
    }

    EXPECT_EQ(dummy_two::m_count, 0);

    {
        auto make = []() -> std::unique_ptr<dummy_two>
        { return std::make_unique<dummy_two>(3U); };

        recycle::unique_pool<dummy_two> pool(make);

        auto o1 = pool.allocate();
        auto o2 = pool.allocate();

        EXPECT_EQ(dummy_two::m_count, 2);
    }

    EXPECT_EQ(dummy_two::m_count, 0);
}

/// Test that the pool works for non constructable objects, even if
/// we do not provide the allocator
TEST(test_unique_pool, default_constructable)
{
    {
        recycle::unique_pool<dummy_one> pool;

        auto o1 = pool.allocate();
        auto o2 = pool.allocate();

        EXPECT_EQ(dummy_one::m_count, 2);
    }

    EXPECT_EQ(dummy_one::m_count, 0);
}

/// Test that everything works even if the pool dies before the
/// objects allocated
TEST(test_unique_pool, pool_die_before_object)
{
    {
        recycle::unique_pool<dummy_one>::pool_ptr d1;
        recycle::unique_pool<dummy_one>::pool_ptr d2;
        recycle::unique_pool<dummy_one>::pool_ptr d3;

        {
            recycle::unique_pool<dummy_one> pool;

            d1 = pool.allocate();
            d2 = pool.allocate();
            d3 = pool.allocate();

            EXPECT_EQ(dummy_one::m_count, 3);
        }

        EXPECT_EQ(dummy_one::m_count, 3);
    }

    EXPECT_EQ(dummy_one::m_count, 0);
}

/// Test that the recycle functionality works
TEST(test_unique_pool, recycle)
{
    std::size_t recycled = 0;

    auto recycle = [&recycled](std::unique_ptr<dummy_two>& o)
    {
        EXPECT_TRUE((bool)o);
        ++recycled;
    };

    auto make = []() -> std::unique_ptr<dummy_two>
    { return std::make_unique<dummy_two>(3U); };

    recycle::unique_pool<dummy_two> pool(make, recycle);

    auto o1 = pool.allocate();
    o1.reset();

    EXPECT_EQ(recycled, 1U);
}

/// Test that copying the shared_pool works as expected.
///
/// For a type to be regular then:
///
///     T a = b; assert(a == b);
///     T a; a = b; <-> T a = b;
///     T a = c; T b = c; a = d; assert(b == c);
///     T a = c; T b = c; zap(a); assert(b == c && a != b);
///
TEST(test_unique_pool, copy_constructor)
{
    recycle::unique_pool<dummy_one> pool;

    auto o1 = pool.allocate();
    auto o2 = pool.allocate();

    o1.reset();

    recycle::unique_pool<dummy_one> new_pool(pool);

    EXPECT_EQ(pool.unused_resources(), 1U);
    EXPECT_EQ(new_pool.unused_resources(), 1U);

    o2.reset();

    EXPECT_EQ(pool.unused_resources(), 2U);
    EXPECT_EQ(new_pool.unused_resources(), 1U);

    EXPECT_EQ(dummy_one::m_count, 3);

    pool.free_unused();
    new_pool.free_unused();

    EXPECT_EQ(dummy_one::m_count, 0);
}

/// Test copy assignment works
TEST(test_unique_pool, copy_assignment)
{
    recycle::unique_pool<dummy_one> pool;

    auto o1 = pool.allocate();
    auto o2 = pool.allocate();

    o1.reset();

    recycle::unique_pool<dummy_one> new_pool;
    new_pool = pool;

    EXPECT_EQ(dummy_one::m_count, 3);
    auto o3 = new_pool.allocate();
    EXPECT_EQ(dummy_one::m_count, 3);
}

/// Test move constructor
TEST(test_unique_pool, move_constructor)
{
    recycle::unique_pool<dummy_one> pool;

    auto o1 = pool.allocate();
    auto o2 = pool.allocate();

    o1.reset();

    recycle::unique_pool<dummy_one> new_pool(std::move(pool));

    o2.reset();
    EXPECT_EQ(new_pool.unused_resources(), 2U);
}

/// Test move assignment
TEST(test_unique_pool, move_assignment)
{
    recycle::unique_pool<dummy_one> pool;

    auto o1 = pool.allocate();
    auto o2 = pool.allocate();

    o1.reset();

    recycle::unique_pool<dummy_one> new_pool;
    new_pool = std::move(pool);

    o2.reset();

    EXPECT_EQ(new_pool.unused_resources(), 2U);
}

/// Test that copy assignment works when we copy from an object with
/// recycle functionality
TEST(test_unique_pool, copy_recycle)
{
    std::size_t recycled = 0;

    auto recycle = [&recycled](std::unique_ptr<dummy_two>& o)
    {
        EXPECT_TRUE((bool)o);
        ++recycled;
    };

    auto make = []() -> std::unique_ptr<dummy_two>
    { return std::make_unique<dummy_two>(3U); };

    recycle::unique_pool<dummy_two> pool(make, recycle);
    recycle::unique_pool<dummy_two> new_pool = pool;

    EXPECT_EQ(pool.unused_resources(), 0U);
    EXPECT_EQ(new_pool.unused_resources(), 0U);

    auto o1 = new_pool.allocate();

    EXPECT_EQ(dummy_two::m_count, 1);

    o1.reset();
    EXPECT_EQ(recycled, 1U);

    new_pool.free_unused();

    EXPECT_EQ(dummy_two::m_count, 0);
}

/// Test that we are thread safe
namespace
{
struct lock_policy
{
    using mutex_type = std::mutex;
    using lock_type = std::lock_guard<mutex_type>;
};
}

TEST(test_unique_pool, thread)
{
    std::size_t recycled = 0;

    auto recycle = [&recycled](std::unique_ptr<dummy_two>& o)
    {
        EXPECT_TRUE((bool)o);
        ++recycled;
    };

    auto make = []() -> std::unique_ptr<dummy_two>
    { return std::make_unique<dummy_two>(3U); };

    // The pool we will use
    using pool_type = recycle::unique_pool<dummy_two, lock_policy>;

    pool_type pool(make, recycle);

    // Lambda the threads will execute captures a reference to the pool
    // so they will all operate on the same pool concurrently
    auto run = [&pool]()
    {
        {
            auto a1 = pool.allocate();
        }

        auto a2 = pool.allocate();
        auto a3 = pool.allocate();

        {
            auto a4 = pool.allocate();
        }

        pool_type new_pool = pool;

        auto b1 = new_pool.allocate();
        auto b2 = new_pool.allocate();

        pool.free_unused();
    };

    const std::size_t number_threads = 8;
    std::thread t[number_threads];

    // Launch a group of threads
    for (std::size_t i = 0; i < number_threads; ++i)
    {
        t[i] = std::thread(run);
    }

    // Join the threads with the main thread
    for (std::size_t i = 0; i < number_threads; ++i)
    {
        t[i].join();
    }
}

/// Test that the pool works for enable_shared_from_this objects, even if
/// we do not provide the allocator
TEST(test_unique_pool, enable_shared_from_this)
{
    {
        recycle::unique_pool<dummy_three> pool;

        auto o1 = pool.allocate();

        EXPECT_EQ(dummy_three::m_count, 1);
    }

    EXPECT_EQ(dummy_three::m_count, 0);
}
