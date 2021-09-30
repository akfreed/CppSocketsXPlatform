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

#include <strapper/net/Endian.h>

#include <arpa/inet.h>
#include <endian.h>

#include <cstring>

namespace strapper { namespace net {

void EndianGloss(int32_t* i32)
{
    *i32 = htonl(*i32);
}

int32_t EndianGloss(int32_t i32)
{
    return htonl(i32);
}

//! Be careful not to assign anything to this double until it's back to host form.
void EndianGloss(double* d)
{
    static_assert(sizeof(*d) == 8, "Function not compatible with this compiler.");
    uint64_t u64;
    std::memcpy(&u64, d, sizeof(u64));
    u64 = htobe64(u64);
    std::memcpy(d, &u64, sizeof(u64));
}

} }