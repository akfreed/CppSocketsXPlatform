//============================================================================
// Copyright (c) 2018 Alexander Freed
//
// Contains the cpp code for TcpSocket
//============================================================================

#include "TcpSocket.h"

#include <assert.h>


// constructor connects to host:port
TcpSocket::TcpSocket(const char* host, const char* port)
    : m_socketId(INVALID_SOCKET)
    , m_hostInfoList(nullptr)
    , m_state(State::CLOSED)
    , m_dumpCount(0)
{
    ZeroMemory(&m_hostInfo, sizeof(m_hostInfo));
    Connect(host, port);
}


// default constructor
// Socket is not connected
TcpSocket::TcpSocket()
    : m_socketId(INVALID_SOCKET)
    , m_hostInfoList(nullptr)
    , m_state(State::CLOSED)
    , m_dumpCount(0)
{ 
    std::lock_guard<std::mutex> lock(m_socketLock);  // ensures proper memory fencing
    ZeroMemory(&m_hostInfo, sizeof(m_hostInfo));
}


// special private constructor used only by TcpListener.Accept(), which has a friend function
TcpSocket::TcpSocket(SOCKET fd)
    : m_socketId(fd)
    , m_hostInfoList(nullptr)
    , m_state(State::CONNECTED)
    , m_dumpCount(0)
{
    std::lock_guard<std::mutex> lock(m_socketLock);  // ensures proper memory fencing
    ZeroMemory(&m_hostInfo, sizeof(m_hostInfo));
}


// move constructor
// Cannot be noexcept because it a mutex can fail to lock.
// The copy operations are deleted, so standard containers will used move 
// operations, however the strong exception guarantee is lost.
TcpSocket::TcpSocket(TcpSocket&& other) noexcept(false)
    : TcpSocket()
{
    this->move(std::move(other));
}


// move assignment
// Cannot be noexcept because it a mutex can fail to lock.
// The copy operations are deleted, so standard containers will used move 
// operations, however the strong exception guarantee is lost.
TcpSocket& TcpSocket::operator=(TcpSocket&& other) noexcept(false)
{
    // move function handles cleaning up
    this->move(std::move(other));
    return *this;
}


// destructor
// Close() can throw, but in that case we pretty much have to abort anyways.
TcpSocket::~TcpSocket() noexcept
{
    Close();
}


//----------------------------------------------------------------------------

void TcpSocket::move(TcpSocket&& other) noexcept(false)
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

    m_hostInfo = std::move(other.m_hostInfo);
    ZeroMemory(&other.m_hostInfo, sizeof(other.m_hostInfo));

    m_hostInfoList = std::move(other.m_hostInfoList);
    other.m_hostInfoList = nullptr;

    m_state = std::move(other.m_state);
    other.m_state = State::CLOSED;

    m_dumpCount = std::move(other.m_dumpCount);
    other.m_dumpCount = 0;
}


// shutdown and close the socket
void TcpSocket::Close()
{
    std::unique_lock<std::mutex> lock(m_socketLock);
    close(lock);
}


// be sure to lock before calling!
void TcpSocket::close(std::unique_lock<std::mutex>& lock)
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

    case State::CONNECTED:
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
    freeaddrinfo(m_hostInfoList);
}


// connects to host:port
bool TcpSocket::Connect(const char* host, const char* port)
{
    std::lock_guard<std::mutex> lock(m_socketLock);
    if (m_state != State::CLOSED)
        return false;

    ZeroMemory(&m_hostInfo, sizeof(m_hostInfo));
    m_hostInfo.ai_family = AF_UNSPEC;  // can be IPv4 or IPv6
    m_hostInfo.ai_socktype = SOCK_STREAM;  // for TCP

                                           // get some info. Fill the struct linked list
    int error = getaddrinfo(host, port, &m_hostInfo, &m_hostInfoList);
    if (error != 0)
    {
        return false;
    }

    // get us a socket
    m_socketId = socket(m_hostInfoList->ai_family, m_hostInfoList->ai_socktype, m_hostInfoList->ai_protocol);
    if (m_socketId == INVALID_SOCKET)
    {
        freeaddrinfo(m_hostInfoList);
        m_hostInfoList = nullptr;
        return false;
    }

    // connect
    error = connect(m_socketId, m_hostInfoList->ai_addr, (int)m_hostInfoList->ai_addrlen);
    if (error == SOCKET_ERROR)
    {
        freeaddrinfo(m_hostInfoList);
        m_hostInfoList = nullptr;
        closesocket(m_socketId);
        return false;
    }

    m_state = State::CONNECTED;
    m_dumpCount = 0;
    return true;
}


//----------------------------------------------------------------------------

// number of times we have called DumpReadBuffer() on this socket
// used to figure out if a socket is deviant
int TcpSocket::GetDumpCount() const
{
    return m_dumpCount;
}


bool TcpSocket::IsConnected()
{
    std::lock_guard<std::mutex> lock(m_socketLock);  // mutex used more for the memory fence than anything else...
    return m_state != State::CLOSED;
}


//----------------------------------------------------------------------------

// If this timeout is reached, the socket will be closed. Thank you, Windows.
// Only use this feature as a robustness mechanism.
// (e.g. so you don't block forever if the connection is somehow silently lost.)
// Don't use this as a form of non-blocking read.
bool TcpSocket::SetReadTimeout(unsigned milliseconds)
{
    std::lock_guard<std::mutex> lock(m_socketLock);
    if (m_state != State::CONNECTED)
        return false;
    return setsockopt(m_socketId, SOL_SOCKET, SO_RCVTIMEO, (const char*)&milliseconds, sizeof(milliseconds)) == 0;
}


//----------------------------------------------------------------------------

// write a char to the stream
bool TcpSocket::Write(const char c)
{
    return Write(&c, 1);
}


// write a boolean to the stream
bool TcpSocket::Write(bool b)
{
    const char c = static_cast<char>(b);
    return Write(&c, 1);
}


// write a 32-bit integer to the stream
bool TcpSocket::Write(int32_t int32)
{
    static_assert(sizeof(int32_t) == 4, "Function not compatible with this architecture.");
    static_assert(sizeof(char) == 1, "Function not compatible with this architecture.");

    char buffer[sizeof(int32_t)];

    int32 = htonl(int32);  // convert to big endian
    memcpy(buffer, &int32, sizeof(int32));

    return Write(buffer, 4);
}


// write a double to the stream
bool TcpSocket::Write(double d)
{
    static_assert(sizeof(unsigned long long) == 8, "Function not compatible with this architecture.");
    static_assert(sizeof(double) == 8, "Function not compatible with this architecture.");
    static_assert(sizeof(char) == 1, "Function not compatible with this architecture.");

    char buffer[sizeof(double)];

    // convert to big endian
    unsigned long long in;
    memcpy(&in, &d, sizeof(d));
    
    for (int i = sizeof(double) - 1; i >= 0; --i)
    {
        buffer[i] = static_cast<char>(in & 0xFF);
        in >>= 8;
    }

    return Write(buffer, 8);
}


bool TcpSocket::Write(const char* buf, int len)
{
    std::unique_lock<std::mutex> lock(m_socketLock);
    if (m_state == State::CLOSED || m_state == State::SHUTTING_DOWN)
        return false;

    if (len <= 0)
    {
        assert(false);
        return true;
    }

    int amountWritten = send(m_socketId, buf, len, 0);

    if (amountWritten == SOCKET_ERROR)
    {
        close(lock);
        return false;
    }
    return true;
}


// send a char string
bool TcpSocket::WriteString(const char* str)
{
    int len = strlen(str);
    if (len > MAX_STRING_LEN)
        len = MAX_STRING_LEN;

    return Write(len) && Write(str, len);
}


//----------------------------------------------------------------------------

// read the next byte from the stream as a char and put into dest
bool TcpSocket::Read(char &dest)
{
    return Read(&dest, 1);
}


// read the next byte from the stream as a boolean and put into dest
bool TcpSocket::Read(bool &dest)
{
    char buf;
    bool result = Read(&buf, 1);
    if (result)
        dest = static_cast<bool>(buf);
    return result;
}


// read the next 4 bytes from the stream as a 32-bit integer and put into dest
bool TcpSocket::Read(int32_t &dest)
{
    static_assert(sizeof(int32_t) == 4, "Function not compatible with this architecture.");
    static_assert(sizeof(char) == 1, "Function not compatible with this architecture.");

    char buffer[sizeof(int32_t)];

    bool result = Read(buffer, 4);
    
    if (result)
    {
        memcpy(&dest, buffer, sizeof(dest));
        dest = ntohl(dest);  // convert to host endian
    }
    return result;
}


// read the next 8 bytes from the stream as a double and put into dest
bool TcpSocket::Read(double &dest)
{
    static_assert(sizeof(unsigned long long) == 8, "Function not compatible with this architecture.");
    static_assert(sizeof(double) == 8, "Function not compatible with this architecture.");
    static_assert(sizeof(char) == 1, "Function not compatible with this architecture.");

    char buffer[sizeof(double)];

    bool result = Read(buffer, 8);

    if (result)
    {
        // convert to host endian
        unsigned long long out = 0;
        for (unsigned i = 0; i < sizeof(double); ++i)
        {
            out <<= 8;
            out |= buffer[i] & 0xFF;
        }

        memcpy(&dest, &out, sizeof(dest));
    }
    return result;
}


bool TcpSocket::preReadSetup()
{
    std::lock_guard<std::mutex> lock(m_socketLock);
    // Sanity check.
    if (m_state == State::READING)
    {
        assert(false);
        return false;
    }
    if (m_state != State::CONNECTED)
        return false;
    m_state = State::READING;
    return true;
}


bool TcpSocket::postReadCheck(int amountRead, int len)
{
    std::unique_lock<std::mutex> lock(m_socketLock);
    if (m_state == State::SHUTTING_DOWN)
    {
        m_state = State::CLOSED;
        m_readCancel.notify_all();
        return false;
    }
    else if (amountRead == SOCKET_ERROR || amountRead < len)  // amount read can be less than len if a read timeout occurs, in which case we need to close the socket.
    {
        m_state = State::CONNECTED;
        close(lock);
        return false;
    }
    else
    {
        m_state = State::CONNECTED;
        return true;
    }
}


// reads len bytes into given char* buffer
bool TcpSocket::Read(char* buf, int len)
{
    // sanity check
    if (len < 1)
    {
        assert(false);
        return false;
    }

    if (!preReadSetup())
        return false;

    int amountRead = recv(m_socketId, buf, len, MSG_WAITALL);  // MSG_WAITALL blocks until all bytes are there
    
    return postReadCheck(amountRead, len);
}


// maxlen is the size of the buffer
// if successful, the string will always be null-terminated
int TcpSocket::ReadString(char* c, int maxlen)
{
    if (maxlen < 1)
    {
        assert(false);
        return 0;
    }

    int len;
    if (!Read(len))
        return -1;

    if (len < 1)
    {
        // other end is corrupted
        assert(false);  // todo: development only. Please remove from final version
        Close();
        return -1;
    }
    
    int diff = len - maxlen;

    if (len > maxlen)
    {
        // other end is not following the rules
        assert(false);  // todo: development only. Please remove from final version
        len = maxlen;
    }

    if (!Read(c, len))
        return -1;

    if (len < maxlen)
        c[len++] = '\0';
    else
        c[len - 1] = '\0';

    if (diff > 0)  // If we had cut off the incoming data to maxlen, we need to dump the remaining data
        DumpReadBuffer(diff);

    return len;
}


// returns the amount of bytes available in the stream
// guaranteed not to be bigger than the actual number
// you can read this many bytes without blocking
// may be smaller than the actual number of bytes available
// returns -1 on error
int TcpSocket::DataAvailable()
{
    std::lock_guard<std::mutex> lock(m_socketLock);
    if (m_state != State::CONNECTED)
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


// reads all the data in the read stream, throwing it away
bool TcpSocket::DumpReadBuffer()
{
    ++m_dumpCount;
    int available;
    const int BUFFER_SIZE = 4096;
    char buffer[BUFFER_SIZE];
    int amountRead;
    while ((available = DataAvailable()) > 0)
    {
        if (!preReadSetup())
            return false;

        amountRead = recv(m_socketId, buffer, BUFFER_SIZE, 0);

        if (!postReadCheck(amountRead, 1))
            return false;
    }

    if (available != 0)
    {
        Close();
        return false;
    }

    return true;
}


int TcpSocket::DumpReadBuffer(int amountToDump)
{
    if (amountToDump < 1)
    {
        assert(false);
        return 0;
    }

    ++m_dumpCount;

    const int BUFFER_SIZE = 4096;
    char buffer[BUFFER_SIZE];
    int remaining = amountToDump;

    while (remaining > BUFFER_SIZE)
    {
        if (!Read(buffer, BUFFER_SIZE))
            return -1;
        remaining -= BUFFER_SIZE;
    }
    if (!Read(buffer, remaining))
        return -1;

    return amountToDump;
}
