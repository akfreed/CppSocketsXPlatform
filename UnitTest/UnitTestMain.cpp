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

#include <gtest/gtest.h>

#include <iostream>

namespace strapper { namespace net { namespace test {

void TerminateHandler()
{
    std::exception_ptr currentException = std::current_exception();
    std::cout << "std::terminate called." << std::endl;
    if (!currentException)
    {
        std::cout << "    No current exception." << std::endl;
    }
    else
    {
        try
        {
            std::rethrow_exception(currentException);
        }
        catch (std::exception const& e)
        {
            std::cout << "    what: " << e.what() << std::endl;
        }
        catch (...)
        {
            std::cout << "    Unknown exception." << std::endl;
        }
    }
    std::abort();
}

} } }

int main()
{
    try
    {
        std::set_terminate(strapper::net::test::TerminateHandler);
        testing::InitGoogleTest();
        return RUN_ALL_TESTS();
    }
    catch (std::exception const& e)
    {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cout << "Unknown Exception." << std::endl;
    }
}
