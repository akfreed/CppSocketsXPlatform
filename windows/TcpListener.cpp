// ==================================================================
// Copyright 2018, 2021 Alexander K. Freed
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

// Contains the cpp code for TcpListener

#include <TcpListener.h>

#include <TcpSocket.h>
#include <NetworkError.h>

#include <cassert>

TcpListener::TcpListener(uint16_t port)
    : m_listener(port)
    , m_state(m_listener ? State::OPEN : State::CLOSED)
{ }

TcpListener::TcpListener(TcpListener&& other) noexcept
    : TcpListener()
{
    swap(*this, other);
}

TcpListener& TcpListener::operator=(TcpListener&& other) noexcept
{
    TcpListener temp(std::move(other));
    swap(*this, temp);
    return *this;
}

TcpListener::~TcpListener()
{
    Close();
}

void swap(TcpListener& left, TcpListener& right)
{
    using State = TcpListener::State;
    std::lock(left.m_lock, right.m_lock);  // deadlock-proof
    std::unique_lock<std::mutex> leftLock(left.m_lock, std::adopt_lock);
    std::unique_lock<std::mutex> rightLock(right.m_lock, std::adopt_lock);

    // If the listener is accpting, moving it probably create a logic bug.
    assert(left.m_state != State::ACCEPTING);
    assert(right.m_state != State::ACCEPTING);
    // If the listener is shutting down, wait for it to finish.
    if (left.m_state == State::SHUTTING_DOWN)
        left.m_acceptCancel.wait(leftLock, [&left]() { return left.m_state == State::CLOSED; });
    if (right.m_state == State::SHUTTING_DOWN)
        right.m_acceptCancel.wait(rightLock, [&right]() { return right.m_state == State::CLOSED; });

    using std::swap;
    swap(left.m_listener, right.m_listener);
    swap(left.m_state, right.m_state);
}

bool TcpListener::IsListening() const
{
    std::lock_guard<std::mutex> lock(m_lock);
    return m_state != State::CLOSED;
}

void TcpListener::Close() noexcept
{
    std::unique_lock<std::mutex> lock(m_lock);

    switch (m_state)
    {
    case State::CLOSED:
        break;

    case State::SHUTTING_DOWN:
        m_acceptCancel.wait(lock, [this]() { return m_state == State::CLOSED; });
        break;

    case State::OPEN:
        m_state = State::CLOSED;
        m_listener.Close();
        break;

    case State::ACCEPTING:
        m_state = State::SHUTTING_DOWN;
        m_listener.shutdown();
        m_acceptCancel.wait(lock, [this]() { return m_state == State::CLOSED; });
        break;
    }
}

TcpSocket TcpListener::Accept()
{
    {
        std::lock_guard<std::mutex> lock(m_lock);
        if (m_state == State::ACCEPTING || m_state == State::SHUTTING_DOWN)
            throw NetworkProgrammingError("Listener is already accepting.");
        if (m_state == State::CLOSED)
            throw NetworkConnectionError("Listener is closed.");
        m_state = State::ACCEPTING;
    }

    try
    {
        TcpSocketBase newClient = m_listener.Accept();

        std::unique_lock<std::mutex> lock(m_lock);
        if (m_state == State::SHUTTING_DOWN)
            throw NetworkConnectionError("Listener was closed from another thread.");

        m_state = State::OPEN;
        return TcpSocket::Attorney::accept(std::move(newClient));
    }
    catch (NetworkError const&)
    {
        std::unique_lock<std::mutex> lock(m_lock);
        m_state = State::CLOSED;
        m_listener.Close();
        m_acceptCancel.notify_all();
        throw;
    }
}

TcpListener::operator bool() const
{
    return IsListening();
}
