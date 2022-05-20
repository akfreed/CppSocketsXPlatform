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

import re


def get_exclusion_regex(folder_to_exclude: str):
    return r"(\A|/|\\){}(/|\\)".format(folder_to_exclude)


def split_args(args: list):
    """Split args into two list.
    The first list containins the args comprised of the script name and subsequent flags.
    The second list contains the files.
    Two lists are always returned, even if they are empty.
    """
    script_and_flags = []
    files = []
    for i, arg in enumerate(args):
        if i == 0:
            # Ignore script name.
            continue
        if arg[:1] != '-':
            return args[:i], args[i:]
    return args[:], []


def filter_args(exclude_folder: str, args: list) -> list:
    """Remove the filepaths containing the exclusion, keeping the script name and flags."""
    exclude_re = get_exclusion_regex(exclude_folder)
    script_and_flags, files = split_args(args)
    filtered_files = [name for name in files if not re.search(exclude_re, name, flags=re.IGNORECASE)]
    return script_and_flags + filtered_files
