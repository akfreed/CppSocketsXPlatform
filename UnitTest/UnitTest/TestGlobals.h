// ==================================================================
// Copyright 2018-2022 Alexander K. Freed
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

#pragma once

#include <cstdint>

namespace strapper { namespace net { namespace test {

struct TestGlobals
{
    static char constexpr localhost[] = "127.0.0.1";
    static uint16_t constexpr testPortA = 11111;
    static uint16_t constexpr testPortB = 11112;
};

}}}  // namespace strapper::net::test
