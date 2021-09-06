// ==================================================================
// Copyright 2018 Alexander K. Freed
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

// Contains the cpp code for UdpSocket.h

#include "UdpSocket.h"

#include <utility>
#include <assert.h>


// This constructor uses any port
// a port of 0 means windows will pick a port for you
UdpSocket::UdpSocket(const char* host, unsigned short hostport)
    : UdpSocket(host, hostport, 0)
{
}


// This constructor sets the port and not the host
// The first client to connect will be locked in
UdpSocket::UdpSocket(unsigned short myport)
    : UdpSocket(nullptr, 0, myport)
{ 
}


// This constructor lets you specify the host and your own port
UdpSocket::UdpSocket(const char* host, unsigned short hostport, unsigned short myport)
    : m_socketId(INVALID_SOCKET)
    , m_theirInfoLen(sizeof(m_theirInfo))
    , m_theirInfoIsValid(false)
    , m_state(State::CLOSED)
    , m_dumpCount(0)
{
    std::lock_guard<std::mutex> lock(m_socketLock);
    ZeroMemory(&m_myInfo, sizeof(m_myInfo));

    m_socketId = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (m_socketId == INVALID_SOCKET)
        return;

    m_myInfo.sin_family = AF_INET;
    m_myInfo.sin_addr.s_addr = htonl(INADDR_ANY);
    m_myInfo.sin_port = htons(myport); 

    if (bind(m_socketId, (sockaddr*)&m_myInfo, sizeof(m_myInfo)) == SOCKET_ERROR)
    {
        closesocket(m_socketId);
        return;
    }

    ZeroMemory(&m_theirInfo, sizeof(m_theirInfo));

    // if host==nullptr or hostport==0, we will skip this
    if (host && hostport)
    {
        m_theirInfo.sin_family = AF_INET;
        m_theirInfo.sin_port = htons(hostport);
        inet_pton(AF_INET, host, &m_theirInfo.sin_addr);
        m_theirInfoIsValid = true;
    }

    m_state = State::OPEN;
}


// default constructor
UdpSocket::UdpSocket()
    : m_socketId(INVALID_SOCKET)
    , m_theirInfoLen(sizeof(m_theirInfo))
    , m_theirInfoIsValid(false)
    , m_state(State::CLOSED)
    , m_dumpCount(0)
{ 
    std::lock_guard<std::mutex> lock(m_socketLock);  // ensures proper memory fencing
    ZeroMemory(&m_myInfo, sizeof(m_myInfo));
    ZeroMemory(&m_theirInfo, sizeof(m_theirInfo));
}


// move constructor
// Cannot be noexcept because it a mutex can fail to lock.
// The copy operations are deleted, so standard containers will used move 
// operations, however the strong exception guarantee is lost.
UdpSocket::UdpSocket(UdpSocket&& other) noexcept(false)
    : UdpSocket()
{
    this->move(std::move(other));
}


// move assignment
// Cannot be noexcept because it a mutex can fail to lock.
// The copy operations are deleted, so standard containers will used move 
// operations, however the strong exception guarantee is lost.
UdpSocket& UdpSocket::operator=(UdpSocket&& other) noexcept(false)
{
    // move function handles cleaning up
    this->move(std::move(other));
    return *this;
}


// destructor
// Close() can throw, but in that case we pretty much have to abort anyways.
UdpSocket::~UdpSocket() noexcept
{
    Close();
}


//----------------------------------------------------------------------------

void UdpSocket::move(UdpSocket&& other) noexcept(false)
{
    std::lock(m_socketLock, other.m_socketLock);  // deadlock-proof
    std::unique_lock<std::mutex> lock(m_socketLock, std::adopt_lock);
    std::unique_lock<std::mutex> other_lock(other.m_socketLock, std::adopt_lock);

    // clean up
    close(lock);

    // If the other socket is reading, moving it probably create a logic bug.
    // If the other socket is shutting down, wait for it to finish.
    if (other.m_state == State::READING || other.m_state == State::SHUTTING_DOWN)
    {
        assert(other.m_state != State::READING);
        other.close(other_lock);
    }

    m_socketId = std::move(other.m_socketId);
    other.m_socketId = INVALID_SOCKET;

    m_myInfo = std::move(other.m_myInfo);
    ZeroMemory(&other.m_myInfo, sizeof(other.m_myInfo));

    m_theirInfo = std::move(other.m_theirInfo);
    ZeroMemory(&other.m_theirInfo, sizeof(other.m_theirInfo));

    m_theirInfoLen = std::move(other.m_theirInfoLen);
    other.m_theirInfoLen = sizeof(other.m_theirInfo);

    m_theirInfoIsValid = std::move(other.m_theirInfoIsValid);
    other.m_theirInfoIsValid = false;

    m_state = std::move(other.m_state);
    other.m_state = State::CLOSED;

    m_dumpCount = other.m_dumpCount;
    other.m_dumpCount = 0;
}


// shutdown and close the socket
void UdpSocket::Close()
{
    std::unique_lock<std::mutex> lock(m_socketLock);
    close(lock);
}


// be sure to lock before calling!
void UdpSocket::close(std::unique_lock<std::mutex>& lock)
{
    switch (m_state)
    {
    case State::CLOSED:
        return;  // return!
        break;

    case State::SHUTTING_DOWN:
        while (m_state != State::CLOSED)
            m_readCancel.wait(lock);
        return;  // return!
        break;

    case State::OPEN:
        m_state = State::CLOSED;
        shutdown(m_socketId, SD_BOTH);
        break;  // fall out

    case State::READING:
        m_state = State::SHUTTING_DOWN;
        shutdown(m_socketId, SD_BOTH);
        // If there is a blocked read on a separate thread, the socket shutdown will unblock it. Wait for it to finish.
        while (m_state != State::CLOSED)
            m_readCancel.wait(lock);
        break;  // fall out
    }

    closesocket(m_socketId);
    m_socketId = INVALID_SOCKET;
    m_theirInfoIsValid = false;
}


//----------------------------------------------------------------------------

int UdpSocket::GetDumpCount() const
{
    return m_dumpCount;
}


bool UdpSocket::IsValid()
{
    std::lock_guard<std::mutex> lock(m_socketLock);  // mutex used more for the memory fence than anything else...
    return m_state != State::CLOSED;
}


//----------------------------------------------------------------------------

// If this timeout is reached, the socket will be closed. Thank you, Windows.
// Only use this feature as a robustness mechanism.
// (e.g. so you don't block forever if the connection is somehow silently lost.)
// Don't use this as a form of non-blocking read.
bool UdpSocket::SetReadTimeout(unsigned milliseconds)
{
    std::lock_guard<std::mutex> lock(m_socketLock);
    if (m_state != State::OPEN)
        return false;
    return setsockopt(m_socketId, SOL_SOCKET, SO_RCVTIMEO, (const char*)&milliseconds, sizeof(milliseconds)) == 0;
}


//----------------------------------------------------------------------------

// The lock is required to be locked before entry. It is not used by this function, but serves as a reminder that this data should be protected by a lock
bool UdpSocket::checkSenderInfo(const sockaddr_in& info, int len, std::unique_lock<std::mutex>&) const
{
    // check if the info is different from expected
    if (m_theirInfoLen != len ||
        m_theirInfo.sin_addr.s_addr != info.sin_addr.s_addr ||
        m_theirInfo.sin_port != info.sin_port ||
        m_theirInfo.sin_family != info.sin_family)
    {
        //assert(false);  // todo: remove from final implementation
        //return false;  // todo: re-instate after testing
    }
    return true;
}


// The lock is required to be locked before entry. It is not used by this function, but serves as a reminder that this data should be protected by a lock
void UdpSocket::setSenderInfo(sockaddr_in& info, int len, std::unique_lock<std::mutex>&)
{
    // if this is our first read, save the info
    memcpy(&m_theirInfo, &info, sizeof(m_theirInfo));
    m_theirInfoLen = len;
    m_theirInfoIsValid = true;
}


//----------------------------------------------------------------------------

bool UdpSocket::Write(const char* buffer, int len)
{
    std::unique_lock<std::mutex> lock(m_socketLock);
    if (m_state == State::CLOSED || m_state == State::SHUTTING_DOWN || !m_theirInfoIsValid || len < 1)
        return false;

    int amountWritten = sendto(m_socketId, buffer, len, 0, (sockaddr*)&m_theirInfo, m_theirInfoLen);

    if (amountWritten == SOCKET_ERROR)
    {
        close(lock);
        return false;
    }
    return true;
}


bool UdpSocket::preReadSetup()
{
    std::lock_guard<std::mutex> lock(m_socketLock);
    // Sanity check.
    if (m_state == State::READING)
    {
        assert(false);
        return false;
    }
    if (m_state != State::OPEN)
        return false;
    m_state = State::READING;
    return true;
}


// lock should be locked before entry
bool UdpSocket::postReadCheck(int amountRead, int maxlen, sockaddr_in& info, int infoLen, std::unique_lock<std::mutex>& lock)
{
    if (m_state == State::SHUTTING_DOWN)
    {
        m_state = State::CLOSED;
        m_readCancel.notify_all();
        return false;
    }
    else if (amountRead == SOCKET_ERROR)
    {
        int error = WSAGetLastError();
        switch (error)
        {
        case WSAEINTR:       // blocking call was interrupted. In a multi-threaded environment, probably means the socket was closed by another thread
            assert(false);   // This shouldn't happen since this situation should be protected by the state machine and mutexes.
            return false;
            break;

        case WSAETIMEDOUT:   // timeout was reached. If this happens, the socket is in an invalid state and must be closed. (Thanks, Windows)
            m_state = State::OPEN;
            close(lock);
            return false;
            break;

        case WSAEMSGSIZE:    // buffer was not large enough
            amountRead = maxlen;
            break;  // fall out of error check back to normal return

        case WSAECONNRESET:  // In TCP, this is a hard reset. In UDP, it means a previous write failed (ICMP Port Unreachable).
            if (!m_theirInfoIsValid)  // expected behavior when reusing a socket. However, we don't allow socket reuse in this implementation.
            {
                assert(false);
                return false;
            }
            else if (checkSenderInfo(info, infoLen, lock))  // same sender. Up to calling code if they want to close
            {
                return false;
            }
            // else, different sender. fall out of error check back to normal return, which will try the read again
            break;

        default:
            assert(false);  // todo: development only. Need to see what kind of errors we experience.
            m_state = State::OPEN;
            close(lock);
            return false;
            break;
        }
    }

    m_state = State::OPEN;
    return true;
}


int UdpSocket::Read(char* dest, int maxlen)
{
    // sanity check
    if (maxlen < 1)
    {
        assert(false);
        return 0;
    }

    sockaddr_in info;
    int infoLen;
    int amountRead = 0;
    bool success = false;

    while (!success)
    {
        infoLen = sizeof(info);
        ZeroMemory(&info, sizeof(info));

        if (!preReadSetup())
            return -1;

        amountRead = recvfrom(m_socketId, dest, maxlen, 0, (sockaddr*)&info, &infoLen);

        {
            std::unique_lock<std::mutex> lock(m_socketLock);
            if (!postReadCheck(amountRead, maxlen, info, infoLen, lock))
                return SOCKET_ERROR;

            if (!m_theirInfoIsValid)  // first connection in is locked in for the life of the object
            {
                setSenderInfo(info, infoLen, lock);
                success = true;
            }
            else if (checkSenderInfo(info, infoLen, lock))  // If sender is different from expected, ignore the read and try again
                success = true;
        }
    }
    
    return amountRead;
}


//----------------------------------------------------------------------------

// returns the total amount of data in the buffer. 
// A call to Read will not necessarily return this much data, since the buffer may contain many datagrams
// returns -1 on error
int UdpSocket::DataAvailable()
{
    std::lock_guard<std::mutex> lock(m_socketLock);
    if (m_state != State::OPEN)
    {
        assert(false);
        return -1;
    }

    unsigned long bytesAvailable;
    if (ioctlsocket(m_socketId, FIONREAD, &bytesAvailable) != 0)
        return -1;

    if (bytesAvailable > INT_MAX)
        bytesAvailable = INT_MAX;

    return static_cast<int>(bytesAvailable);
}


// dumps all the datagrams from the buffer
bool UdpSocket::DumpReadBuffer()
{
    ++m_dumpCount;
    const int BUFFER_SIZE = 4096;
    char buffer[BUFFER_SIZE];

    sockaddr_in info;
    int infoLen = sizeof(info);
    ZeroMemory(&info, sizeof(info));
    int available;
    int amountRead;

    while ((available = DataAvailable()) > 0)
    {
        if (!preReadSetup())
            return false;

        amountRead = recvfrom(m_socketId, buffer, BUFFER_SIZE, 0, (sockaddr*)&info, &infoLen);

        {
            std::unique_lock<std::mutex> lock(m_socketLock);
            if (!postReadCheck(amountRead, BUFFER_SIZE, info, infoLen, lock))
                return false;
        }
    }

    if (available != 0)
    {
        Close();
        return false;
    }

    return true;
}

