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

import sys

import hooks.clang_tidy

import shim_utils


def main() -> int:
    """Filter out filenames from other system implementations and call pocc's clang-tidy hook with the modified args."""

    if sys.platform == "win32":
        exclude = "linux"
    elif sys.platform == "linux":
        exclude = "windows"
    else:
        raise NotImplementedError(f"Not implemented for system: {sys.platform}")

    filtered_args = shim_utils.filter_args(exclude, sys.argv)
    sys.argv = filtered_args
    return hooks.clang_tidy.main(filtered_args)


if __name__ == "__main__":
    sys.exit(main())
