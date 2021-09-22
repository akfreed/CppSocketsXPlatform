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

#pragma once

#include <strapper/net/SocketError.h>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <iostream>

namespace strapper { namespace net { namespace test {

class Timeout
{
public:
    Timeout() = default;

    template <typename Rep, typename Period>
    explicit Timeout(std::chrono::duration<Rep, Period> timeout)
    {
        m_thread = std::thread(&Timeout::WaitThread<Rep, Period>, this, timeout);
    }

    Timeout(const Timeout&) = delete;
    Timeout(Timeout&&) = delete;
    Timeout& operator=(const Timeout&) = delete;
    Timeout& operator=(Timeout&&) = delete;

    ~Timeout()
    {
        Release();
    }

    //! Cancel the timeout.
    void Release()
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_complete = true;
        }
        m_cv.notify_all();
        if (m_thread.joinable())
            m_thread.join();
    }

private:
    template <typename Rep, typename Period>
    void WaitThread(std::chrono::duration<Rep, Period> timeout)
    {
        try
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            bool complete = m_cv.wait_for(lock, timeout, [this]() { return m_complete; });
            // If complete is true, the cv did not time out.
            if (!complete)
            {
                std::cout << "Timeout exceeded." << std::endl;
                throw ProgramError("Timeout exceeded.");
            }
        }
        catch (...)
        {
            throw;
        }
    }

    std::mutex m_mutex;
    std::condition_variable m_cv;
    bool m_complete = false;
    std::thread m_thread;
};

} } }
