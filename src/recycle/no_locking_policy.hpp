// Copyright Steinwurf ApS 2014.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#pragma once

namespace recycle
{
/// Defines the default non thread-safe locking policy for the
/// recycle::resource_pool.
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
struct no_locking_policy
{
    /// Define dummy mutex type
    struct no_mutex
    {
    };

    /// Define dummy lock type
    struct no_lock
    {
        no_lock(no_mutex&)
        {
        }
    };

    /// The locking policy mutex type
    using mutex_type = no_mutex;

    /// The locking policy lock type
    using lock_type = no_lock;
};
}
