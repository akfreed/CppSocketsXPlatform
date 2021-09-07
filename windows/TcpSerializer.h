// ==================================================================
// Copyright 2018-2021 Alexander K. Freed
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

// Contains the declaration TcpSocket
//
// This file should be included before windows.h
//
// The copy operations are deleted, so standard containers will used move 
// operations, however the strong exception guarantee is lost.

#pragma once

#include "TcpSocket.h"

class TcpSerializer
{
public:
    static constexpr int MAX_STRING_LEN = 4096;

    explicit TcpSerializer(TcpSocket&& socket);

    const TcpSocket& Socket() const;
    TcpSocket& Socket();

    bool Write(char c);
    bool Write(bool b);
    bool Write(int32_t int32);
    bool Write(double d);
    bool WriteString(const char* str);

    bool Read(char& dest);
    bool Read(bool& dest);
    bool Read(int32_t& dest);
    bool Read(double& dest);
    int ReadString(char* c, int maxlen);

private:
    TcpSocket m_socket;
};