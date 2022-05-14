// ==================================================================
// Copyright 2021 Alexander K. Freed
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ==================================================================

#include "SocketIncludes.h"
#include <strapper/net/Endian.h>

#include <cstring>

namespace strapper { namespace net {

void nton(int32_t* i32)
{
    *i32 = htonl(*i32);
}

int32_t nton(int32_t i32)
{
    return htonl(i32);
}

void nton(uint32_t* i32)
{
    *i32 = htonl(*i32);
}

uint32_t nton(uint32_t i32)
{
    return htonl(i32);
}

//! Be careful not to assign anything to this double until it's back to host form.
void nton(double* d)
{
    auto const ull = htond(*d);
    std::memcpy(d, &ull, sizeof(ull));
}

}}  // namespace strapper::net
