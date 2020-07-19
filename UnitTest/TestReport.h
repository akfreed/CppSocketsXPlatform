//============================================================================
// Copyright (c) 2018 Alexander Freed
// 
// Contains the definition for TestReport.
//============================================================================
#pragma once

#include <vector>


//============================================================================

struct TestReport
{
    std::string Name;
    std::string Description;
    bool Passed = false;
    bool FailedAssumptions = false;
    bool Skipped = false;
    std::string ResultNotes = "";
    std::vector<TestReport> SubTests;

    TestReport() { }
    TestReport(std::string name, std::string description)
        : Name(std::move(name))
        , Description(std::move(description))
    { }
};

