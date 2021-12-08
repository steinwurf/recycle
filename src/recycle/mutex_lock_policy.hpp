#pragma once
// Copyright Steinwurf ApS 2014.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#include <mutex>
#include <thread>

namespace recycle {
/// Defines the locking policies for the recycle::resource_pool.
///
/// Custom locking policies may be defined to create a thread-safe
/// resource pool for different threading libraries.
///
/// A valid locking policy defines two types, namely the mutex and
/// the lock.
///
/// The following small example illustrates the expected behavior:
///
///     using mutex = locking_policy::mutex_type;
///     using lock = locking_policy::lock_type;
///
///     {
///         mutex m; // creates mutex m in unlocked state
///         lock l(m); // associates and locks the mutex m with the lock l.
///
///         ... // when l's destructor runs it unlocks m
///     }
///
/// If you wanted to use std::thread then a suitable locking
/// policy could be:
///
///    struct lock_policy
///    {
///        using mutex_type = std::mutex;
///        using lock_type = std::lock_guard<mutex_type>;
///    };
///

/**
 * Fully locking policy that works for SPMC pools
 */
struct mutex_lock_policy {
    using mutex_type = std::mutex;
    using lock_type = std::lock_guard<mutex_type>;
};

}  // namespace recycle
