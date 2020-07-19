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

// Contains the definition for TestReport.

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

