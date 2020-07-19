//============================================================================
// Copyright (c) 2018 Alexander Freed
//
// The WinsockContextManager is used for the Windows version of the API. It
// is just a dummy class in the Linux API.
//============================================================================
#pragma once

class WinsockContextManager
{
public:
    bool IsInitialized()
    {
        return true;
    }
};
