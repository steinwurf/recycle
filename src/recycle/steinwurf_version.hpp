// Copyright Steinwurf ApS 2014.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#pragma once

#include <string>

namespace recycle
{
    /// Returns a string representing the Steinwurf version of this library.
    inline std::string steinwurf_version()
    {
        #ifdef STEINWURF_RECYCLE_VERSION
        return STEINWURF_RECYCLE_VERSION;
        #else
        return "unknown";
        #endif
    }
}
