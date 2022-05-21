# ==================================================================
# Copyright 2022 Alexander K. Freed
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ==================================================================

import pytest

from shim_utils import *


def test_flags_and_files():
    args = ["script.py", "--flag1=hi", "-f", "--jabba", "file1", "file2", "linux", "windows"]
    first, second = split_args(args)
    assert first == ["script.py", "--flag1=hi", "-f", "--jabba"]
    assert second == ["file1", "file2", "linux", "windows"]
    assert first is not args
    assert second is not args


def test_no_flags():
    args = ["script.py", "file1", "file2", "linux", "windows"]
    first, second = split_args(args)
    assert first == ["script.py"]
    assert second == ["file1", "file2", "linux", "windows"]
    assert first is not args
    assert second is not args


def test_no_files():
    args = ["script.py", "--flag1=hi", "-f", "--jabba"]
    first, second = split_args(args)
    assert first == args
    assert second == []
    assert first is not args
    assert second is not args


def test_script_name_only():
    args = ["script.py"]
    first, second = split_args(args)
    assert first == args
    assert second == []
    assert first is not args
    assert second is not args


def test_empty_args():
    args = []
    first, second = split_args(args)
    assert first == []
    assert second == []
    assert first is not args
    assert second is not args


def filter_files_by_folder():
    result = filter_files_by_folder("windows", ["pocc-shim.py", "-f", R"alice\windows\bob", R"alice\linux\bob", R"alice\_windows\bob"])
    assert result ==                ["pocc-shim.py", "-f", R"alice\linux\bob", R"alice\_windows\bob"]

    result = filter_files_by_folder(  "linux", ["pocc-shim.py", "-f", R"alice/windows\bob", R"alice/linux\bob", R"alice/_windows\bob"])
    assert result ==                ["pocc-shim.py", "-f", R"alice/windows\bob", R"alice/_windows\bob"]

    result = filter_files_by_folder("windows", ["pocc-shim.py", "--two", "--flags", R"alice/_linux\bob", R"alice/windows_/bob", R".windows/alice"])
    assert result ==                ["pocc-shim.py", "--two", "--flags", R"alice/_linux\bob", R"alice/windows_/bob", R".windows/alice"]

    result = filter_files_by_folder(  "linux", ["pocc-shim.py", "--two", "--flags", R"alice\_linux\bob", R"alice\windows_\bob", R".windows\alice"])
    assert result ==                ["pocc-shim.py", "--two", "--flags", R"alice\_linux\bob", R"alice\windows_\bob", R".windows\alice"]

    result = filter_files_by_folder("windows", ["pocc-shim.py", R"alice\linux_\bob", R"windows/alice", R"linux\alice"])
    assert result ==                ["pocc-shim.py", R"alice\linux_\bob", R"linux\alice"]

    result = filter_files_by_folder(  "linux", ["pocc-shim.py", R"alice/linux_/bob", R"windows/alice", R"linux/alice"])
    assert result ==                ["pocc-shim.py", R"alice/linux_/bob", R"windows/alice"]

    result = filter_files_by_folder("windows", ["pocc-shim.py"])
    assert result ==                ["pocc-shim.py"]
    result = filter_files_by_folder(  "linux", ["pocc-shim.py"])
    assert result ==                ["pocc-shim.py"]

    result = filter_files_by_folder("windows", ["pocc-shim.py", "--fix"])
    assert result ==                ["pocc-shim.py", "--fix"]
    result = filter_files_by_folder(  "linux", ["pocc-shim.py", "--fix"])
    assert result ==                ["pocc-shim.py", "--fix"]

    result = filter_files_by_folder("windows", ["pocc-shim.py", "-p=build", "--fix", "--config-file=.clang-tidy", R"bob/windows", R".linux\alice", R"bob\linux"])
    assert result ==                ["pocc-shim.py", "-p=build", "--fix", "--config-file=.clang-tidy", R"bob/windows", R".linux\alice", R"bob\linux"]

    result = filter_files_by_folder(  "linux", ["pocc-shim.py", "-p=build", "--fix", "--config-file=.clang-tidy", R"bob\windows", R".linux/alice", R"bob\linux"])
    assert result ==                ["pocc-shim.py", "-p=build", "--fix", "--config-file=.clang-tidy", R"bob\windows", R".linux/alice", R"bob\linux"]


def filter_files_by_folder():
    result = filter_files_by_ext(".h", [R"alice/bob.h", R"bob.h/blah", R"alice\windows\bob.hpp", R"alice\windows\bob.h"])
    assert result ==                   [R"bob.h/blah", R"alice\windows\bob.hpp"]

    result = filter_files_by_ext(".cpp", [R"alice/bob.h", R"bob.h/blah", R"alice\windows\bob.hpp", R"alice\windows\bob.h", R"alice/windows\bob.cpp"])
    assert result ==                     [R"alice/bob.h", R"bob.h/blah", R"alice\windows\bob.hpp", R"alice\windows\bob.h"]

    result = filter_files_by_ext(".", [R"alice/bob.h", R"bob.h/blah", R"alice\windows\bob.hpp", R"alice\windows\bob.h"])
    assert result ==                  [R"alice/bob.h", R"bob.h/blah", R"alice\windows\bob.hpp", R"alice\windows\bob.h"]

    result = filter_files_by_ext("", [R"alice/bob.h", R"bob.h\blah", R"alice/windows/bob.hpp", R"alice\windows\bob.h"])
    assert result ==                  [R"alice/bob.h", R"alice/windows/bob.hpp", R"alice\windows\bob.h"]
