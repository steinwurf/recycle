// Copyright Steinwurf ApS 2014.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#include <recycle/no_locking_policy.hpp>

#include <gtest/gtest.h>

TEST(TestNoLockingPolicy, Empty)
{
    recycle::no_locking_policy policy;
    (void) policy;
}
