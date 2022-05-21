#!/usr/bin/env python

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

import os
import sys

import hooks.clang_tidy

import shim_utils

POCC_SHIM_SKIP_CLANG_TIDY = "POCC_SHIM_SKIP_CLANG_TIDY"


def main() -> int:
    """Filter out filenames from other system implementations and call pocc's clang-tidy hook with the modified args."""

    # Check the environment variable set by GitHub Actions (or other CI) to indicate not to run clang-tidy.
    # This allows the auto-formatting job to complete faster, since clang-tidy takes a good amount of time.
    should_skip = os.getenv(POCC_SHIM_SKIP_CLANG_TIDY, "")
    if should_skip.lower() == "true" or should_skip.lower() == "yes" or should_skip == "1":
        print(f"Environment variable POCC_SHIM_SKIP_CLANG_TIDY={should_skip}")
        print("Skipping clang-tidy.")
        return 0

    # Check for the existence of compile_commands.json. If missing, clang-tidy will give a lot of false positives.
    if not os.path.exists("compile_commands.json"):
        print(f"Current Working Directory: {os.getcwd()}")
        print("Could not find compile_commands.json in the root of the source directory.")
        print("Generating the project with CMake creates compile_commands.json.")
        if sys.platform == "win32":
            print("Windows: Run CMake with Ninja generator on a console with admin privileges.")
        elif sys.platform == "linux":
            print("Linux: Run CMake (with Make).")
        return 1

    # Filter out source folders that aren't for this system.
    if sys.platform == "win32":
        exclude = "linux"
    elif sys.platform == "linux":
        exclude = "windows"
    else:
        raise NotImplementedError(f"Not implemented for system: {sys.platform}")

    filtered_args = shim_utils.filter_files_by_folder(exclude, sys.argv)
    filtered_args = shim_utils.filter_files_by_ext(".h", filtered_args)

    # Call pocc's hook with the filtered arguments list.
    sys.argv = filtered_args  # pocc reads directly from sys.argv, so we must modify sys.argv. See https://github.com/pocc/pre-commit-hooks/issues/46
    hooks.clang_tidy.main(filtered_args)
    return 0


if __name__ == "__main__":
    sys.exit(main())
