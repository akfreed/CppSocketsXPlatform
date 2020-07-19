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

#include <cstring>      // memcpy and memset
#include <utility>      // move
#include <unistd.h>     // close
#include <arpa/inet.h>  // inet_pton
#include <sys/ioctl.h>  // ioctl
#include <assert.h>
#include <errno.h>


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
    memset(&m_myInfo, 0, sizeof(m_myInfo));

    m_socketId = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (m_socketId == INVALID_SOCKET)
        return;

    m_myInfo.sin_family = AF_INET;
    m_myInfo.sin_addr.s_addr = htonl(INADDR_ANY);
    m_myInfo.sin_port = htons(myport); 

    if (bind(m_socketId, (sockaddr*)&m_myInfo, sizeof(m_myInfo)) == SOCKET_ERROR)
    {
        ::close(m_socketId);
        return;
    }

    memset(&m_theirInfo, 0, sizeof(m_theirInfo));

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
    memset(&m_myInfo, 0, sizeof(m_myInfo));
    memset(&m_theirInfo, 0, sizeof(m_theirInfo));
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
    memset(&other.m_myInfo, 0, sizeof(other.m_myInfo));
    
    m_theirInfo = std::move(other.m_theirInfo);
    memset(&other.m_theirInfo, 0, sizeof(other.m_theirInfo));
    
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
        shutdown(m_socketId, SHUT_RDWR);
        break;  // fall out

    case State::READING:
        m_state = State::SHUTTING_DOWN;
        shutdown(m_socketId, SHUT_RDWR);
        // If there is a blocked read on a separate thread, the socket shutdown will unblock it. Wait for it to finish.
        while (m_state != State::CLOSED)
            m_readCancel.wait(lock);
        break;  // fall out
    }

    ::close(m_socketId);
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

// To keep compatibility with the Windows version, if this timeout is reached, 
// the socket will be closed. Thank you, Windows.
// Only use this feature as a robustness mechanism.
// (e.g. so you don't block forever if the connection is somehow silently lost.)
// Don't use this as a form of non-blocking read.
// A time of 0 will set it to infinitely block. No idea what this does in Windows.
// I recommend not using 0.
// after setting this, a timed out read will return -1
// you should check errno
bool UdpSocket::SetReadTimeout(unsigned milliseconds)
{
    std::lock_guard<std::mutex> lock(m_socketLock);
    if (m_state != State::OPEN)
        return false;
    
    timeval t;
    t.tv_sec = milliseconds / 1000;
    t.tv_usec = (milliseconds % 1000) * 1000;
    return setsockopt(m_socketId, SOL_SOCKET, SO_RCVTIMEO, (char*)&t, sizeof(timeval)) == 0;
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
        assert(false);  // todo: remove from final implementation
        return false;  // todo: this needs more testing
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
        if (errno == EHOSTUNREACH)
        {
            // I've never seen this happen on linux with sendto, but I guess I'll put it in anyways.
            // In UDP this means a write failed (ICMP Port Unreachable). Since packet drop in UDP is
            // not unknown, just pretend like nothing happened.
            return true;
        }
                
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
bool UdpSocket::postReadCheck(int amountRead, int maxlen, sockaddr_in&, int infoLen, std::unique_lock<std::mutex>& lock)
{
    if (m_state == State::SHUTTING_DOWN)
    {
        m_state = State::CLOSED;
        m_readCancel.notify_all();
        return false;
    }
    else if (amountRead == 0 && infoLen == 0 && maxlen > 0)
    {
        // In a multi-threaded environment, this probably means the socket was closed by another thread.
        // This shouldn't happen since this situation should be protected by the state machine and mutexes.
        assert(false);
        return false;
    }
    else if (amountRead == SOCKET_ERROR)
    {
        switch (errno)
        {
        case EAGAIN:           // Probably means that a timeout was reached. Socket is still good, but for 
            close(lock);       // consistency with the Windows version, we need to close the socket.
            return false;
            break;
            
        default:
            assert(false);  // todo: development only. Need to see what kind of errors we experience.
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
    socklen_t infoLen;
    int amountRead;
    bool success = false;

    while (!success)
    {
        infoLen = sizeof(info);
        memset(&info, 0, sizeof(info));

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

    int bytesAvailable;
    if (ioctl(m_socketId, FIONREAD, &bytesAvailable) != 0)
        return -1;
    
    return bytesAvailable;
}


// dumps all the datagrams from the buffer
bool UdpSocket::DumpReadBuffer()
{
    ++m_dumpCount;
    const int BUFFER_SIZE = 4096;
    char buffer[BUFFER_SIZE];

    sockaddr_in info;
    socklen_t infoLen = sizeof(info);
    memset(&info, 0, sizeof(info));
    int available;
    ssize_t amountRead;

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

