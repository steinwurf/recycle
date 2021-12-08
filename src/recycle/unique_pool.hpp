// Copyright Steinwurf ApS 2014.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

/**
 * MODIFIED BY SWXTCH.IO 2021
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <type_traits>
#include <utility>

#include "recycle/mutex_lock_policy.hpp"

namespace recycle {
/// @brief The unique_pool uses unique_ptr to store value objects and recycle them.
///
/// The unique_pool is a useful construct if you have some expensive to create
/// objects where you would like to create a factory capable of recycling the objects.
///
/// Default locking policy support SPMC pools. Use the no_locking_policy if you only
/// need SPSC pools.
///
template <class Value, class LockingPolicy = mutex_lock_policy>
class unique_pool {
   public:
    /// Forward declare
    struct deleter;

   private:
    struct impl;

   public:
    /// The type managed
    using value_type = Value;

    /// The pointer to the resource
    using pool_ptr = std::unique_ptr<value_type, deleter>;
    using pool_ptrs = std::vector<pool_ptr>;

    /// The owning pointer to the resource
    using value_ptr = std::unique_ptr<value_type>;

    /// The allocate function type
    /// Should take no arguments and return an std::unique_ptr to the Value
    using allocate_function = std::function<value_ptr()>;

    /// The recycle function type
    /// If specified the recycle function will be called every time a
    /// resource gets recycled into the pool. This allows temporary
    /// resources, e.g., file handles to be closed when an object is longer
    /// used.
    using recycle_function = std::function<void(value_ptr&)>;

    /// The locking policy mutex type
    using mutex_type = typename LockingPolicy::mutex_type;

    /// The locking policy lock type
    using lock_type = typename LockingPolicy::lock_type;

   public:
    /// Default constructor, we only want this to be available
    /// i.e. the unique_pool to be default constructible if the
    /// value_type we build is default constructible.
    ///
    /// This means that we only want
    /// std::is_default_constructible<unique_pool<T>>::value to
    /// be true if the type T is default constructible.
    ///
    /// Unfortunately this does not work if we don't do the
    /// template magic seen below. What we do there is to use
    /// SFINAE to disable the default constructor for non default
    /// constructible types.
    ///
    /// It looks quite ugly and if somebody can fix in a simpler way
    /// please do :)
    template <class T = Value,
              typename std::enable_if<std::is_default_constructible<T>::value, uint8_t>::type = 0>
    unique_pool()
        : m_pool(std::make_shared<impl>(allocate_function(std::make_unique<value_type>))) {
    }

    /// Create a unique_pool using a specific allocate function.
    /// @param allocate Allocation function
    unique_pool(allocate_function allocate) : m_pool(std::make_shared<impl>(std::move(allocate))) {
    }

    /// Create a unique_pool using a specific allocate function and
    /// recycle function.
    /// @param allocate Allocation function
    /// @param recycle Recycle function
    unique_pool(allocate_function allocate, recycle_function recycle)
        : m_pool(std::make_shared<impl>(std::move(allocate), std::move(recycle))) {
    }

    /// Copy constructor
    unique_pool(const unique_pool& other) : m_pool(std::make_shared<impl>(*other.m_pool)) {
    }

    /// Move constructor
    unique_pool(unique_pool&& other) : m_pool(std::move(other.m_pool)) {
        assert(m_pool);
    }

    /// Copy assignment
    unique_pool& operator=(const unique_pool& other) {
        unique_pool tmp(other);
        std::swap(*this, tmp);
        return *this;
    }

    /// Move assignment
    unique_pool& operator=(unique_pool&& other) {
        m_pool = std::move(other.m_pool);
        return *this;
    }

    /// @returns the number of unused resources
    std::size_t unused_resources() const {
        assert(m_pool);
        return m_pool->unused_resources();
    }

    /// Frees all unused resources
    void free_unused() {
        assert(m_pool);
        m_pool->free_unused();
    }

    /// @return A resource from the pool.
    pool_ptr allocate() {
        assert(m_pool);
        return m_pool->allocate();
    }

    unique_pool& prefill(size_t count) {
        assert(m_pool);
        m_pool->prefill(count);
        return *this;
    }

    unique_pool& setFixedCapacity() {
        assert(m_pool);
        m_pool->setFixedCapacity(true);
        return *this;
    }

    size_t allocate_bulk(pool_ptrs& array, size_t count) {
        assert(m_pool);
        auto Count = m_pool->allocate_bulk(array, count);
        return Count;
    }

    void recycle_bulk(pool_ptrs& array) {
        assert(m_pool);
        m_pool->recycle_bulk(array);
    }

   private:
    /// The actual pool implementation. We use the
    /// enable_shared_from_this helper to make sure we can pass a
    /// "back-pointer" to the pooled objects. The idea behind this
    /// is that we need objects to be able to add themselves back
    /// into the pool once they go out of scope.
    struct impl : public std::enable_shared_from_this<impl> {
        /// @copydoc unique_pool::unique_pool(allocate_function)
        impl(allocate_function allocate) : m_allocate(std::move(allocate)), m_fixedCapacity(false) {
            assert(m_allocate);
        }

        /// @copydoc unique_pool::unique_pool(allocate_function,
        ///                                       recycle_function)
        impl(allocate_function allocate, recycle_function recycle)
            : m_allocate(std::move(allocate)),
              m_recycle(std::move(recycle)),
              m_fixedCapacity(false) {
            assert(m_allocate);
            assert(m_recycle);
        }

        /// Copy constructor
        impl(const impl& other)
            : std::enable_shared_from_this<impl>(other),
              m_allocate(other.m_allocate),
              m_recycle(other.m_recycle),
              m_fixedCapacity(false) {
            std::size_t size = other.unused_resources();
            for (std::size_t i = 0; i < size; ++i) {
                m_free_list.push_back(m_allocate());
            }
        }

        /// Move constructor
        impl(impl&& other)
            : std::enable_shared_from_this<impl>(other),
              m_allocate(std::move(other.m_allocate)),
              m_recycle(std::move(other.m_recycle)),
              m_free_list(std::move(other.m_free_list)),
              m_fixedCapacity(false) {
        }

        /// Copy assignment
        impl& operator=(const impl& other) {
            impl tmp(other);
            std::swap(*this, tmp);
            return *this;
        }

        /// Move assignment
        impl& operator=(impl&& other) {
            m_allocate = std::move(other.m_allocate);
            m_recycle = std::move(other.m_recycle);
            m_free_list = std::move(other.m_free_list);
            return *this;
        }

        /// Allocate a new value from the pool
        pool_ptr allocate() {
            value_ptr resource;
            {
                lock_type lock(m_mutex);
                if (m_free_list.size() > 0) {
                    resource = std::move(m_free_list.back());
                    m_free_list.pop_back();
                }
            }
            if (!resource) {
                if (m_fixedCapacity) {
                    std::cout << "allocate error: no more entries in pool" << std::endl;
                    return pool_ptr(nullptr, deleter());
                }
                assert(m_allocate);
                resource = m_allocate();
            }
            // Here we create a std::unique_ptr<T> with a naked
            // pointer to the resource and a custom deleter
            // object. The custom deleter object stores two
            // things:
            //
            //   1. A std::weak_ptr<T> to the pool (used when we
            //      need to put the resource back in the pool). If
            //      the pool dies before the resource then we can
            //      detect this with the weak_ptr and not try to
            //      access it.
            //
            //   2. A std::unique_ptr<T> that points to the actual
            //      resource and is the one actually keeping it alive.
            value_type* naked_ptr = resource.get();
            auto p = pool_ptr(naked_ptr, deleter(impl::shared_from_this(), std::move(resource)));
            return p;
        }

        /// Allocate a bunch of new values from the pool
        size_t allocate_bulk(pool_ptrs& Array, size_t count) {
            value_ptr resource;
            {
                lock_type lock(m_mutex);
                while (Array.size() < count) {
                    if (m_free_list.size() == 0) {
                        break;
                    }
                    value_ptr resource = std::move(m_free_list.back());
                    m_free_list.pop_back();
                    value_type* naked_ptr = resource.get();
                    auto p = pool_ptr(naked_ptr,
                                      deleter(impl::shared_from_this(), std::move(resource)));
                    Array.push_back(std::move(p));
                }
            }
            if ((Array.size() < count) && !m_fixedCapacity) {
                assert(m_allocate);
                while (Array.size() < count) {
                    resource = m_allocate();
                    value_type* naked_ptr = resource.get();
                    auto p = pool_ptr(naked_ptr,
                                      deleter(impl::shared_from_this(), std::move(resource)));
                    Array.push_back(std::move(p));
                }
            }
            return Array.size();
        }

        /// Pre-fill the pool with count instances of the resource
        void prefill(size_t count) {
            assert(m_allocate);
            lock_type lock(m_mutex);
            while (m_free_list.size() < count) {
                value_ptr resource = m_allocate();
                m_free_list.push_back(std::move(resource));
            }
        }

        void setFixedCapacity(bool value) {
            m_fixedCapacity = value;
        }

        /// @copydoc unique_pool::free_unused()
        void free_unused() {
            lock_type lock(m_mutex);
            m_free_list.clear();
        }

        /// @copydoc unique_pool::unused_resources()
        std::size_t unused_resources() const {
            lock_type lock(m_mutex);
            return m_free_list.size();
        }

        /// This function called when a resource should be added back into the pool
        void recycle(value_ptr resource) {
            if (m_recycle) {
                m_recycle(resource);
            }

            lock_type lock(m_mutex);
            m_free_list.push_back(std::move(resource));
        }

        /// Adds a bunch of values back to the pool
        void recycle_bulk(pool_ptrs& array) {
            lock_type lock(m_mutex);
            for (pool_ptr& p : array) {
                if (p) {
                    auto resource = p.get_deleter().release();
                    if (m_recycle) {
                        m_recycle(resource);
                    }
                    m_free_list.push_back(std::move(resource));
                }
            }
        }

       private:
        /// The allocator to use
        allocate_function m_allocate;

        /// The recycle function
        recycle_function m_recycle;

        /// Stores all the free resources
        std::list<value_ptr> m_free_list;

        /// Mutex used to coordinate access to the pool. We had to
        /// make it mutable as we have to lock in the
        /// unused_resources() function. Otherwise we can have a
        /// race condition on the size it returns. I.e. if one
        /// threads releases a resource into the free list while
        /// another tries to read its size.
        mutable mutex_type m_mutex;

        /// Limits the pool to only the entries that where prefilled.
        /// I.E., no additional allocations will be made.
        bool m_fixedCapacity;

    };  // end impl

   public:
    /// The custom deleter object used by the std::unique_ptr<T>
    /// to de-allocate the object if the pool goes out of
    /// scope. When a std::unique_ptr wants to de-allocate the
    /// object contained it will call the operator() define here.
    struct deleter {
        /// Constructor
        deleter() = default;

        /// @param pool A weak_ptr to the pool
        /// @param resource The owning unique_ptr
        deleter(const std::weak_ptr<impl>& pool, std::unique_ptr<Value> resource)
            : m_pool(pool), m_resource(std::move(resource)) {
            assert(!m_pool.expired());
            assert(m_resource);
        }

        /// Call operator called by std::unique_ptr<T> when de-allocating the object.
        void operator()(value_type*) {
            if (!m_resource) {
                return;
            }
            // Get a shared pointer to the the pool. If the pool is no longer
            // available then we will get a null reference.
            auto pool = m_pool.lock();
            if (pool) {
                pool->recycle(std::move(m_resource));
            }
        }

        std::unique_ptr<Value> release() {
            return std::move(m_resource);
        }
        // Pointer to the pool needed for recycling
        std::weak_ptr<impl> m_pool;

        // The resource object
        std::unique_ptr<Value> m_resource;
    };

   private:
    // The pool impl
    std::shared_ptr<impl> m_pool;
};

}  // namespace recycle
